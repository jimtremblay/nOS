/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static DWORD WINAPI _Entry (LPVOID lpParameter);
static DWORD WINAPI _Scheduler (LPVOID lpParameter);
static DWORD WINAPI _SysTick (LPVOID lpParameter);

static HANDLE           _hCritical;
static uint32_t         _criticalNestingCounter;
static HANDLE           _hSchedRequest;
static nOS_Stack        _idleStack;

static DWORD WINAPI _Entry (LPVOID lpParameter)
{
    nOS_Thread *thread = (nOS_Thread*)lpParameter;

    /* Enter thread main loop */
    thread->stackPtr->entry(thread->stackPtr->arg);

    return 0;
}

static DWORD WINAPI _Scheduler (LPVOID lpParameter)
{
    while (true) {
        /* Wait until a thread requesting a context switch */
        while (WaitForSingleObject(_hSchedRequest, INFINITE) != WAIT_OBJECT_0);

        /* Enter critical section */
        while (WaitForSingleObject(_hCritical, INFINITE) != WAIT_OBJECT_0);

        /* Reset context switching request event in critical section */
        ResetEvent(_hSchedRequest);

        /* If a high prio thread is waiting,
         * suspend running thread and resume high prio thread */
        nOS_highPrioThread = nOS_FindHighPrioThread();
        if (nOS_runningThread != nOS_highPrioThread) {
            /* If a thread is currently running, suspend it */
            if (nOS_runningThread != NULL) {
                SuspendThread(nOS_runningThread->stackPtr->handle);
            }

            nOS_runningThread = nOS_highPrioThread;

            /* If a thread need to be schedule, resume it */
            if (nOS_highPrioThread != NULL) {
                ResumeThread(nOS_highPrioThread->stackPtr->handle);

                /* Release sync object only if resumed thread is waiting in context switch */
                if (nOS_highPrioThread->stackPtr->sync) {
                    nOS_highPrioThread->stackPtr->sync = false;
                    ReleaseSemaphore(nOS_highPrioThread->stackPtr->hsync, 1, NULL);
                }
            }
        }

        /* Leave critical section */
        ReleaseMutex(_hCritical);
    }

    return 0;
}

static DWORD WINAPI _SysTick (LPVOID lpParameter)
{
    uint32_t crit;

    while (true) {
        Sleep(1000/NOS_CONFIG_TICKS_PER_SECOND);

        /* Enter critical section */
        while (WaitForSingleObject(_hCritical, INFINITE) != WAIT_OBJECT_0);

		/* Backup critical nesting counter to restore it at the end of SysTick */
        crit = _criticalNestingCounter;
        _criticalNestingCounter = 1;

		/* Simulate entry in interrupt */
		nOS_isrNestingCounter = 1;

        nOS_Tick();
#if (NOS_CONFIG_TIMER_ENABLE > 0)
        nOS_TimerTick();
#endif
#if (NOS_CONFIG_TIME_ENABLE > 0)
        nOS_TimeTick();
#endif

		/* Simulate exit of interrupt */
		nOS_isrNestingCounter = 0;

        if (nOS_runningThread != nOS_FindHighPrioThread()) {
            SetEvent(_hSchedRequest);
        }

        _criticalNestingCounter = crit;

        /* Leave critical section */
        ReleaseMutex(_hCritical);
    }

    return 0;
}

void nOS_InitSpecific(void)
{
    _criticalNestingCounter = 0;
    /* Create a mutex for critical section */
    _hCritical = CreateMutex(NULL,                  /* Default security descriptor */
                             false,                 /* Initial state is unlocked */
                             NULL);                 /* No name */

    nOS_idleHandle.stackPtr = &_idleStack;
    _idleStack.entry = NULL;
    _idleStack.arg = NULL;
    _idleStack.crit = 0;
    _idleStack.sync = false;
    _idleStack.hsync = CreateSemaphore(NULL,        /* Default security descriptor */
                                       0,           /* Initial count = 0 */
                                       1,           /* Maximum count = 1 */
                                       NULL);       /* No name */
    /* Convert pseudo handle of GetCurrentThread to real handle to be used by Scheduler */
    DuplicateHandle(GetCurrentProcess(),
                    GetCurrentThread(),
                    GetCurrentProcess(),
                    &_idleStack.handle,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
    _idleStack.id = GetCurrentThreadId();

    /* Create an event for context switching request */
    _hSchedRequest = CreateEvent(NULL,              /* Default security descriptor */
                                 TRUE,              /* Manual reset */
                                 FALSE,             /* Initial state is non-signaled */
                                 NULL);             /* No name */

    CreateThread(NULL,                              /* Default security descriptor */
                 0,                                 /* Default stack size */
                 _Scheduler,                        /* Start address of the thread */
                 NULL,                              /* No argument */
                 0,                                 /* Thread run immediately after creation */
                 NULL);                             /* Don't get thread identifier */
    CreateThread(NULL,                              /* Default security descriptor */
                 0,                                 /* Default stack size */
                 _SysTick,                          /* Start address of the thread */
                 NULL,                              /* No argument */
                 0,                                 /* Thread run immediately after creation */
                 NULL);                             /* Don't get thread identifier */
}

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    thread->stackPtr = stack;
    stack->entry = entry;
    stack->arg = arg;
    stack->crit = 0;
    stack->sync = false;
    /* Create a semaphore for context switching synchronization */
    stack->hsync = CreateSemaphore(NULL,            /* Default security descriptor */
                                   0,               /* Initial count = 0 */
                                   1,               /* Maximum count = 1 */
                                   NULL);           /* No name */
    stack->handle = CreateThread(NULL,              /* Default security descriptor */
                                 0,                 /* Default stack size */
                                 _Entry,            /* Start address of the thread */
                                 (LPVOID)thread,    /* Thread object as argument */
                                 CREATE_SUSPENDED,  /* Thread created in suspended state */
                                 &stack->id);       /* Store thread identifier in thread pseudo stack */
}

void nOS_SwitchContext(void)
{
    nOS_Stack *stack = nOS_runningThread->stackPtr;

    stack->sync = true;
    stack->crit = _criticalNestingCounter;
    SetEvent(_hSchedRequest);
    /* Leave critical section (allow Scheduler and SysTick to run) */
    ReleaseMutex(_hCritical);

    /* Wait synchronization event from Scheduler */
    while(WaitForSingleObject(stack->hsync, INFINITE) != WAIT_OBJECT_0);

    /* Enter critical section */
    while(WaitForSingleObject(_hCritical, INFINITE) != WAIT_OBJECT_0);
	_criticalNestingCounter = stack->crit;
}

void nOS_EnterCritical(void)
{
    if (nOS_running) {
        /* Enter critical section */
        while(WaitForSingleObject(_hCritical, INFINITE) != WAIT_OBJECT_0);
        if (_criticalNestingCounter > 0) {
            /* Keep only one level of Windows critical to be able
			 * to leave critical section when switching context */
            ReleaseMutex(_hCritical);
        }
        _criticalNestingCounter++;
    }
}

void nOS_LeaveCritical(void)
{
    if (nOS_running) {
        _criticalNestingCounter--;
        if (_criticalNestingCounter == 0) {
            /* Leave critical section */
            ReleaseMutex(_hCritical);
        }
    }
}

int nOS_Print(const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);

    nOS_EnterCritical();
    ret = vprintf(format, args);
    nOS_LeaveCritical();

    va_end(args);

    return ret;
}

#ifdef __cplusplus
}
#endif
