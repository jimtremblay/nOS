/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

static pthread_mutex_t  _criticalSection;
volatile uint32_t       _criticalNestingCounter;
static pthread_cond_t   _schedCond;
volatile bool           _schedStarted;
volatile bool           _schedRequest;
static nOS_Stack        _idleStack;

static void* _Entry (void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)arg;

    pthread_mutex_lock(&_criticalSection);

    /* Signal to creator we're running */
    thread->stackPtr->started = true;
    pthread_cond_signal(&thread->stackPtr->cond);

    /* Wait to have the permission to run (given by scheduler) */
    while (!thread->stackPtr->running) {
        pthread_cond_wait(&thread->stackPtr->cond, &_criticalSection);
    }

    /* Initialize critical section counter */
    _criticalNestingCounter = thread->stackPtr->crit;

    pthread_mutex_unlock(&_criticalSection);

    thread->stackPtr->entry(thread->stackPtr->arg);

    return 0;
}

static void* _Scheduler (void *arg)
{
    pthread_mutex_lock(&_criticalSection);

    /* Signal to init we're running and ready to accept request */
    _schedStarted = true;
    pthread_cond_signal(&_schedCond);

    while (true) {
        /* Wait until a thread send a scheduling request */
        while (!_schedRequest) {
            pthread_cond_wait(&_schedCond, &_criticalSection);
        }
        _schedRequest = false;

        /* Find next high prio thread and give him permission to run */
        nOS_highPrioThread = nOS_FindHighPrioThread();
        nOS_runningThread = nOS_highPrioThread;
        nOS_highPrioThread->stackPtr->running = true;
        pthread_cond_signal(&nOS_highPrioThread->stackPtr->cond);
    }

    pthread_mutex_unlock(&_criticalSection);

    return 0;
}

static void* _SysTick (void *arg)
{
    while (true) {
        usleep(1000000/NOS_CONFIG_TICKS_PER_SECOND);

        pthread_mutex_lock(&_criticalSection);
        _criticalNestingCounter = 1;

        /* Simulate entry in interrupt */
        nOS_isrNestingCounter = 1;

        nOS_Tick();

        /* Simulate exit of interrupt */
        nOS_isrNestingCounter = 0;

        _criticalNestingCounter = 0;
        pthread_mutex_unlock(&_criticalSection);
    }

    return 0;
}

void nOS_InitSpecific(void)
{
    pthread_t pthread;

    _criticalNestingCounter = 0;
    pthread_mutex_init(&_criticalSection, NULL);

    pthread_cond_init (&_schedCond, NULL);

    nOS_idleHandle.stackPtr = &_idleStack;
    _idleStack.entry = NULL;
    _idleStack.arg = NULL;
    _idleStack.crit = 0;
    _idleStack.started = true;
    _idleStack.running = true;
    pthread_cond_init(&_idleStack.cond, NULL);

    pthread_mutex_lock(&_criticalSection);

    _schedStarted = false;
    _schedRequest = false;
    pthread_create(&pthread, NULL, _Scheduler, NULL);

    /* Wait until scheduler is running and waiting for request */
    while (!_schedStarted) {
        pthread_cond_wait(&_schedCond, &_criticalSection);
    }

    /* Create a SysTick thread to allow sleep/timeout */
    pthread_create(&pthread, NULL, _SysTick, NULL);

    pthread_mutex_unlock(&_criticalSection);
}

/* Called outside of critical section */
void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    pthread_mutex_lock(&_criticalSection);

    thread->stackPtr = stack;
    stack->entry = entry;
    stack->arg = arg;
    stack->crit = 0;
    stack->started = false;
    stack->running = false;
    pthread_cond_init(&stack->cond, NULL);
    pthread_create(&stack->handle, NULL, _Entry, thread);

    /* Wait until thread is started and waiting to run */
    while (!stack->started) {
        pthread_cond_wait(&stack->cond, &_criticalSection);
    }

    pthread_mutex_unlock(&_criticalSection);
}

/* Called from critical section */
void nOS_SwitchContext(void)
{
    nOS_Stack *stack = nOS_runningThread->stackPtr;

    /* Backup critical nesting counter and stop running */
    stack->crit = _criticalNestingCounter;
    stack->running = false;

    /* Send scheduling request */
    _schedRequest = true;
    pthread_cond_signal(&_schedCond);

    /* Wait until we have permission to run */
    while (!stack->running) {
        pthread_cond_wait(&stack->cond, &_criticalSection);
    }

    /* Restore critical nesting counter */
    _criticalNestingCounter = stack->crit;
}

void nOS_EnterCritical(void)
{
    if (_criticalNestingCounter == 0) {
        /* Lock mutex only one time */
        pthread_mutex_lock(&_criticalSection);
    }
    _criticalNestingCounter++;
}

void nOS_LeaveCritical(void)
{
    _criticalNestingCounter--;
    if (_criticalNestingCounter == 0) {
        /* Unlock mutex when nesting counter reach zero */
        pthread_mutex_unlock(&_criticalSection);
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
