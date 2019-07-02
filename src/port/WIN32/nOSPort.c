/*
 * Copyright (c) 2014-2019 Jim Tremblay
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

HANDLE                  nOS_hCritical;
uint32_t                nOS_criticalNestingCounter;

static HANDLE           _hSchedRequest;
static nOS_Stack        _mainStack;

static DWORD WINAPI _Entry (LPVOID lpParameter)
{
    nOS_Thread *thread = (nOS_Thread*)lpParameter;

    while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);
    nOS_criticalNestingCounter = 0;
    ReleaseMutex(nOS_hCritical);

    /* Enter thread main loop */
    thread->stackPtr->entry(thread->stackPtr->arg);

    return 0;
}

static DWORD WINAPI _Scheduler (LPVOID lpParameter)
{
    NOS_UNUSED(lpParameter);

    while (!nOS_running) {
        Sleep(1);
    }

    while (true) {
        /* Wait until a thread requesting a context switch */
        while (WaitForSingleObject(_hSchedRequest, INFINITE) != WAIT_OBJECT_0);

        /* Enter critical section */
        while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);

        /* Reset context switching request event in critical section */
        ResetEvent(_hSchedRequest);

        /* Suspend currently running thread */
        SuspendThread(nOS_runningThread->stackPtr->handle);

        if (nOS_highPrioThread == NULL) {
            nOS_highPrioThread = &nOS_mainHandle;
        }
        nOS_runningThread = nOS_highPrioThread;

        /* Resume high prio thread */
        ResumeThread(nOS_highPrioThread->stackPtr->handle);

        /* Release sync object only if resumed thread is waiting in context switch */
        if (nOS_highPrioThread->stackPtr->sync) {
            nOS_highPrioThread->stackPtr->sync = false;
            ReleaseSemaphore(nOS_highPrioThread->stackPtr->hsync, 1, NULL);
        }

        /* Leave critical section */
        ReleaseMutex(nOS_hCritical);
    }

    return 0;
}

static DWORD WINAPI _SysTick (LPVOID lpParameter)
{
    nOS_Thread     *highPrioThread;

    NOS_UNUSED(lpParameter);

    while (!nOS_running) {
        Sleep(1);
    }

    while (true) {
        Sleep(1000/NOS_CONFIG_TICKS_PER_SECOND);

        /* Enter critical section */
        while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);
        nOS_criticalNestingCounter = 1;

        /* Simulate entry in interrupt */
        nOS_isrNestingCounter = 1;

        nOS_Tick(1);

#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            highPrioThread = nOS_FindHighPrioThread();
            if (highPrioThread != NULL) {
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0) || (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE > 0)
                nOS_highPrioThread = highPrioThread;
                if (nOS_runningThread != nOS_highPrioThread) {
                    SetEvent(_hSchedRequest);
                }
#endif
            } else if (nOS_runningThread != &nOS_mainHandle) {
                nOS_highPrioThread = &nOS_mainHandle;
                SetEvent(_hSchedRequest);
            }
        }

        /* Simulate exit of interrupt */
        nOS_isrNestingCounter = 0;

        /* Leave critical section */
        nOS_criticalNestingCounter = 0;
        ReleaseMutex(nOS_hCritical);
    }

    return 0;
}

void nOS_InitSpecific (void)
{
    nOS_criticalNestingCounter = 0;
    /* Create a mutex for critical section */
    nOS_hCritical = CreateMutex(NULL,               /* Default security descriptor */
                                false,              /* Initial state is unlocked */
                                NULL);              /* No name */

    nOS_mainHandle.stackPtr = &_mainStack;
    _mainStack.entry = NULL;
    _mainStack.arg = NULL;
    _mainStack.sync = false;
    _mainStack.hsync = CreateSemaphore(NULL,        /* Default security descriptor */
                                       0,           /* Initial count = 0 */
                                       1,           /* Maximum count = 1 */
                                       NULL);       /* No name */
    /* Convert pseudo handle of GetCurrentThread to real handle to be used by Scheduler */
    DuplicateHandle(GetCurrentProcess(),
                    GetCurrentThread(),
                    GetCurrentProcess(),
                    &_mainStack.handle,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
    _mainStack.id = GetCurrentThreadId();

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

void nOS_InitContext (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    NOS_UNUSED(ssize);

    thread->stackPtr = stack;
    stack->entry = entry;
    stack->arg = arg;
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

void nOS_SwitchContext (void)
{
    uint32_t crit;

    /* Backup thread's critical nesting counter */
    crit = nOS_criticalNestingCounter;

    nOS_runningThread->stackPtr->sync = true;
    SetEvent(_hSchedRequest);

    /* Leave critical section (allow Scheduler and SysTick to run) */
    ReleaseMutex(nOS_hCritical);

    /* Wait synchronization event from Scheduler */
    while(WaitForSingleObject(nOS_runningThread->stackPtr->hsync, INFINITE) != WAIT_OBJECT_0);

    /* Enter critical section */
    while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);

    /* Restore thread's critical nesting counter */
    nOS_criticalNestingCounter = crit;
}

int nOS_Print (const char *format, ...)
{
    va_list         args;
    nOS_StatusReg   sr;
    int             ret;

    va_start(args, format);

    nOS_EnterCritical(sr);
    ret = vprintf(format, args);
    nOS_LeaveCritical(sr);

    va_end(args);

    return ret;
}

#ifdef __cplusplus
}
#endif
