/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* This is an internal function */
void SuspendThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (thread != &nOS_mainThread) {
        if (thread->state == NOS_READY) {
            RemoveThreadFromReadyList(thread);
        }
        thread->state |= NOS_SUSPENDED;
    }
}

/* This is an internal function */
void ResumeThread (void *payload, void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)payload;
    uint8_t     *sched = (uint8_t*)arg;

    if (thread->state & NOS_SUSPENDED) {
        thread->state &= ~NOS_SUSPENDED;
        if (thread->state == NOS_READY) {
            AppendThreadToReadyList(thread);
            if (thread->prio > nOS_runningThread->prio) {
                if (sched != NULL) {
                    *sched = 1;
                }
            }
        }
    }
}

/* This is an internal function */
void TickThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if ((thread->state & NOS_WAITING) && (thread->timeout > 0)) {
        thread->timeout--;
        if (thread->timeout == 0) {
            if ((thread->state & NOS_WAITING) == NOS_SLEEPING) {
                thread->error = NOS_OK;
            } else {
                nOS_ListRemove(&thread->event->waitingList, &thread->readyWaiting);
                thread->error = NOS_E_TIMEOUT;
            }
            thread->state &= ~NOS_WAITING;
            if (thread->state == NOS_READY) {
                AppendThreadToReadyList(thread);
            }
        }
    }
}

/* This is an internal function */
void SignalThread (nOS_Thread *thread)
{
    nOS_ListRemove(&thread->event->waitingList, &thread->readyWaiting);
    thread->error = NOS_OK;
    thread->state &= ~NOS_WAITING;
    if (thread->state == NOS_READY) {
        AppendThreadToReadyList(thread);
    }
}

/* This is an internal function */
void SetThreadPriority (nOS_Thread *thread, uint8_t prio)
{
    if (thread->prio != prio)
    {
        if (thread->state == NOS_READY)
        {
            RemoveThreadFromReadyList(thread);
        }
        thread->prio = prio;
        if (thread->state == NOS_READY)
        {
            AppendThreadToReadyList(thread);
        }
    }
}

nOS_Error nOS_ThreadCreate (nOS_Thread *thread, void(*func)(void*), void *arg,
                            stack_t *stack, size_t ssize, uint8_t prio, uint8_t state)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (func == NULL) {
        err = NOS_E_INV_VAL;
    } else if (stack == NULL) {
        err = NOS_E_INV_VAL;
    } else if (ssize == 0) {
        err = NOS_E_INV_VAL;
    } else if (prio > NOS_CONFIG_MAX_THREAD_PRIO) {
        err = NOS_E_INV_VAL;
    } else if ((state != NOS_READY) && (state != NOS_SUSPENDED)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        thread->prio = prio;
        thread->state = state;
        thread->timeout = 0;
        thread->event = NULL;
        thread->context = NULL;
        thread->error = NOS_OK;
        thread->full.payload = thread;
        thread->readyWaiting.payload = thread;
        nOS_ContextInit(thread, stack, ssize, func, arg);
        nOS_CriticalEnter();
        nOS_ListAppend(&nOS_fullList, &thread->full);
        if (state == NOS_READY) {
            AppendThreadToReadyList(thread);
            if (prio > nOS_runningThread->prio) {
                nOS_Sched();
            }
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadSuspend (nOS_Thread *thread)
{
    nOS_Error   err;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

    /* Main thread can't be suspended */
    if (thread == &nOS_mainThread) {
        err = NOS_E_IDLE;
    } else if (thread == nOS_runningThread) {
        /* Can't switch context if scheduler is locked */
        if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        } else {
            err = NOS_OK;
        }
    } else {
        err = NOS_OK;
    }

    if (err == NOS_OK) {
        nOS_CriticalEnter();
        SuspendThread(thread, NULL);
        nOS_CriticalLeave();
        if (thread == nOS_runningThread) {
            nOS_Sched();
        }
    }

    return err;
}

nOS_Error nOS_ThreadSuspendAll (void)
{
    nOS_Error   err;

    /* Can't suspend all threads from other threads when scheduler is locked */
    if ((nOS_lockNestingCounter > 0) && (nOS_runningThread != &nOS_mainThread)) {
        err = NOS_E_LOCKED;
    } else {
        nOS_CriticalEnter();
        nOS_ListWalk(&nOS_fullList, SuspendThread, NULL);
        nOS_CriticalLeave();
        if (nOS_runningThread != &nOS_mainThread) {
            nOS_Sched();
        }
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadResume (nOS_Thread *thread)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (thread == nOS_runningThread) {
        err = NOS_E_INV_VAL;
    } else if (thread == &nOS_mainThread) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        ResumeThread(thread, NULL);
        if (thread->prio > nOS_runningThread->prio) {
            nOS_Sched();
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadResumeAll (void)
{
    uint8_t sched = 0;

    nOS_CriticalEnter();
    nOS_ListWalk(&nOS_fullList, ResumeThread, &sched);
    nOS_CriticalLeave();

    if (sched) {
        nOS_Sched();
    }

    return NOS_OK;
}

nOS_Error nOS_ThreadSetPriority (nOS_Thread *thread, uint8_t prio)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (prio > NOS_CONFIG_MAX_THREAD_PRIO) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        if (thread == NULL) {
            thread = nOS_runningThread;
        }

        nOS_CriticalEnter();
        SetThreadPriority(thread, prio);
        if (thread->prio > nOS_runningThread->prio) {
            nOS_Sched();
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

uint8_t nOS_ThreadGetPriority (nOS_Thread *thread)
{
    uint8_t prio;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

    nOS_CriticalEnter();
    prio = thread->prio;
    nOS_CriticalLeave();

    return prio;
}

nOS_Thread* nOS_ThreadRunning(void)
{
    return nOS_runningThread;
}

#if defined(__cplusplus)
}
#endif
