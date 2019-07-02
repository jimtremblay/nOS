/*
 * Copyright (c) 2014-2019 Jim Tremblay
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

volatile uint32_t       nOS_criticalNestingCounter;
pthread_mutex_t         nOS_criticalSection;

static pthread_cond_t   _schedCond;
volatile bool           _schedStarted;
volatile bool           _schedRequest;
static nOS_Stack        _mainStack;

static void* _Entry (void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)arg;

    pthread_mutex_lock(&nOS_criticalSection);

    /* Signal to creator we're running */
    thread->stackPtr->started = true;
    pthread_cond_signal(&thread->stackPtr->cond);

    /* Wait to have the permission to run (given by scheduler) */
    while (!thread->stackPtr->running) {
        pthread_cond_wait(&thread->stackPtr->cond, &nOS_criticalSection);
    }

    /* Initialize critical section counter */
    nOS_criticalNestingCounter = thread->stackPtr->crit;

    pthread_mutex_unlock(&nOS_criticalSection);

    thread->stackPtr->entry(thread->stackPtr->arg);

    return 0;
}

static void* _Scheduler (void *arg)
{
    pthread_mutex_lock(&nOS_criticalSection);

    /* Signal to init we're running and ready to accept request */
    _schedStarted = true;
    pthread_cond_signal(&_schedCond);

    /* Wait until scheduler is started */
    pthread_mutex_unlock(&nOS_criticalSection);
    while (!nOS_running) {
        usleep(1000);
    }
    pthread_mutex_lock(&nOS_criticalSection);

    while (true) {
        /* Wait until a thread send a scheduling request */
        while (!_schedRequest) {
            pthread_cond_wait(&_schedCond, &nOS_criticalSection);
        }
        _schedRequest = false;

        /* Find next high prio thread and give him permission to run */
        nOS_highPrioThread = nOS_FindHighPrioThread();
        if (nOS_highPrioThread == NULL) {
            nOS_highPrioThread = &nOS_mainHandle;
        }
        nOS_runningThread = nOS_highPrioThread;
        nOS_highPrioThread->stackPtr->running = true;
        pthread_cond_signal(&nOS_highPrioThread->stackPtr->cond);
    }

    pthread_mutex_unlock(&nOS_criticalSection);

    return 0;
}

static void* _SysTick (void *arg)
{
    while (!nOS_running) {
        usleep(1000);
    }

    while (true) {
        usleep(1000000/NOS_CONFIG_TICKS_PER_SECOND);

        pthread_mutex_lock(&nOS_criticalSection);
        nOS_criticalNestingCounter = 1;

        /* Simulate entry in interrupt */
        nOS_isrNestingCounter = 1;

        nOS_Tick(1);

        /* Simulate exit of interrupt */
        nOS_isrNestingCounter = 0;

        nOS_criticalNestingCounter = 0;
        pthread_mutex_unlock(&nOS_criticalSection);
    }

    return 0;
}

void nOS_InitSpecific (void)
{
    pthread_t pthread;
    pthread_mutexattr_t attr;

    nOS_criticalNestingCounter = 0;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&nOS_criticalSection, &attr);

    pthread_cond_init (&_schedCond, NULL);

    nOS_mainHandle.stackPtr = &_mainStack;
    _mainStack.entry = NULL;
    _mainStack.arg = NULL;
    _mainStack.crit = 0;
    _mainStack.started = true;
    _mainStack.running = true;
    pthread_cond_init(&_mainStack.cond, NULL);

    pthread_mutex_lock(&nOS_criticalSection);

    _schedStarted = false;
    _schedRequest = false;
    pthread_create(&pthread, NULL, _Scheduler, NULL);

    /* Wait until scheduler is running and waiting for request */
    while (!_schedStarted) {
        pthread_cond_wait(&_schedCond, &nOS_criticalSection);
    }

    /* Create a SysTick thread to allow sleep/timeout */
    pthread_create(&pthread, NULL, _SysTick, NULL);

    pthread_mutex_unlock(&nOS_criticalSection);
}

/* Called from critical section */
void nOS_InitContext (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
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
        pthread_cond_wait(&stack->cond, &nOS_criticalSection);
    }
}

/* Called from critical section */
void nOS_SwitchContext (void)
{
    nOS_Stack *stack = nOS_runningThread->stackPtr;

    /* Backup critical nesting counter and stop running */
    stack->crit = nOS_criticalNestingCounter;
    stack->running = false;

    /* Send scheduling request */
    _schedRequest = true;
    pthread_cond_signal(&_schedCond);

    /* Wait until we have permission to run */
    while (!stack->running) {
        pthread_cond_wait(&stack->cond, &nOS_criticalSection);
    }

    /* Restore critical nesting counter */
    nOS_criticalNestingCounter = stack->crit;
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
