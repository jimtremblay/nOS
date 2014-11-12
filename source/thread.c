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

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
/* This is an internal function */
void SuspendThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (thread != &nOS_mainThread) {
        if (thread->state == NOS_THREAD_READY) {
            RemoveThreadFromReadyList(thread);
        }
        thread->state |= NOS_THREAD_SUSPENDED;
    }
}

/* This is an internal function */
void ResumeThread (void *payload, void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)payload;
    bool        *sched =  (bool*)arg;

    if (thread->state & NOS_THREAD_SUSPENDED) {
        thread->state &=~ NOS_THREAD_SUSPENDED;
        if (thread->state == NOS_THREAD_READY) {
            AppendThreadToReadyList(thread);
            if (thread->prio > nOS_runningThread->prio) {
                if (sched != NULL) {
                    *sched = true;
                }
            }
        }
    }
}
#endif  /* NOS_CONFIG_THREAD_SUSPEND_ENABLE */

#if (NOS_CONFIG_THREAD_DELETE_ENABLE > 0)
/* This is an internal function */
void DeleteThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (thread->state == NOS_THREAD_READY) {
        RemoveThreadFromReadyList(thread);
    } else if (thread->state & NOS_THREAD_WAITING) {
        nOS_ListRemove(&thread->event->waitingList, &thread->readyWaiting);
    }
    thread->state   = NOS_THREAD_STOPPED;
    thread->event   = NULL;
    thread->context = NULL;
    thread->timeout = 0;
    thread->error   = NOS_OK;
}
#endif  /* NOS_CONFIG_THREAD_DELETE_ENABLE */

/* This is an internal function */
void TickThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (thread->timeout > 0) {
        thread->timeout--;
        if (thread->timeout == 0) {
            if (thread->state & NOS_THREAD_SLEEPING) {
                thread->state &=~ NOS_THREAD_SLEEPING;
                thread->error = NOS_OK;
            } else if (thread->state & NOS_THREAD_WAITING) {
                nOS_ListRemove(&thread->event->waitingList, &thread->readyWaiting);
                thread->state &=~ NOS_THREAD_WAITING;
                thread->error = NOS_E_TIMEOUT;
            }
            if (thread->state == NOS_THREAD_READY) {
                AppendThreadToReadyList(thread);
            }
        }
    }
}

/* This is an internal function */
void SignalThread (nOS_Thread *thread, nOS_Error err)
{
    nOS_ListRemove(&thread->event->waitingList, &thread->readyWaiting);
    thread->error = err;
    thread->state &=~ NOS_THREAD_WAITING;
    thread->timeout = 0;
    if (thread->state == NOS_THREAD_READY) {
        AppendThreadToReadyList(thread);
    }
}

#if (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0) || (NOS_CONFIG_MUTEX_ENABLE > 0)
/* This is an internal function */
void SetThreadPriority (nOS_Thread *thread, uint8_t prio)
{
    if (thread->prio != prio)
    {
        if (thread->state == NOS_THREAD_READY)
        {
            RemoveThreadFromReadyList(thread);
        }
        thread->prio = prio;
        if (thread->state == NOS_THREAD_READY)
        {
            AppendThreadToReadyList(thread);
        }
    }
}
#endif

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
nOS_Error nOS_ThreadCreate (nOS_Thread *thread, void(*func)(void*), void *arg,
                            nOS_Stack *stack, size_t ssize, uint8_t prio, uint8_t state)
#else
nOS_Error nOS_ThreadCreate (nOS_Thread *thread, void(*func)(void*), void *arg,
                            nOS_Stack *stack, size_t ssize, uint8_t prio)
#endif
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (thread == &nOS_mainThread) {
        err = NOS_E_INV_VAL;
    } else if (func == NULL) {
        err = NOS_E_INV_VAL;
    } else if (stack == NULL) {
        err = NOS_E_INV_VAL;
    } else if (ssize == 0) {
        err = NOS_E_INV_VAL;
    } else if (prio > NOS_CONFIG_HIGHEST_THREAD_PRIO) {
        err = NOS_E_INV_VAL;
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
    } else if ((state != NOS_THREAD_READY) && (state != NOS_THREAD_SUSPENDED)) {
        err = NOS_E_INV_VAL;
#endif
    } else if (thread->state != NOS_THREAD_STOPPED) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        thread->prio = prio;
        thread->state = NOS_THREAD_READY;
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
        if (state == NOS_THREAD_SUSPENDED) {
            thread->state |= NOS_THREAD_SUSPENDED;
        }
#endif
        thread->timeout = 0;
        thread->event = NULL;
        thread->context = NULL;
        thread->error = NOS_OK;
        thread->full.payload = thread;
        thread->readyWaiting.payload = thread;
        nOS_ContextInit(thread, stack, ssize, func, arg);
        nOS_CriticalEnter();
        nOS_ListAppend(&nOS_fullList, &thread->full);
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
        if (thread->state == NOS_THREAD_READY)
#endif
        {
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

#if (NOS_CONFIG_THREAD_DELETE_ENABLE > 0)
nOS_Error nOS_ThreadDelete (nOS_Thread *thread)
{
    nOS_Error   err;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    /* Main thread can't be deleted */
    if (thread == &nOS_mainThread) {
        err = NOS_E_IDLE;
    } else if (thread == nOS_runningThread) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        /* Can't switch context if scheduler is locked */
        if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        } else
#endif
        {
            err = NOS_OK;
        }
    } else if (thread->state == NOS_THREAD_STOPPED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        err = NOS_OK;
    }

    if (err == NOS_OK) {
        nOS_CriticalEnter();
        DeleteThread(thread, NULL);
        nOS_CriticalLeave();
        if (thread == nOS_runningThread) {
            nOS_Sched();
        }
    }

    return err;
}
#endif

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
nOS_Error nOS_ThreadSuspend (nOS_Thread *thread)
{
    nOS_Error   err;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    /* Main thread can't be suspended */
    if (thread == &nOS_mainThread) {
        err = NOS_E_IDLE;
    } else if (thread == nOS_runningThread) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        /* Can't switch context if scheduler is locked */
        if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        } else
#endif
        {
            err = NOS_OK;
        }
    } else if (thread->state == NOS_THREAD_STOPPED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
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

#if (NOS_CONFIG_SAFE > 0)
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* Can't suspend all threads from other threads when scheduler is locked */
    if ((nOS_lockNestingCounter > 0) && (nOS_runningThread != &nOS_mainThread)) {
        err = NOS_E_LOCKED;
    } else
#endif
#endif
    {
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

#if (NOS_CONFIG_SAFE > 0)
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (thread == nOS_runningThread) {
        err = NOS_E_INV_VAL;
    } else if (thread == &nOS_mainThread) {
        err = NOS_E_INV_VAL;
    } else if (thread->state == NOS_THREAD_STOPPED) {
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
    bool sched = false;

    nOS_CriticalEnter();
    nOS_ListWalk(&nOS_fullList, ResumeThread, &sched);
    nOS_CriticalLeave();

    if (sched) {
        nOS_Sched();
    }

    return NOS_OK;
}
#endif  /* NOS_CONFIG_THREAD_SUSPEND_ENABLE */

#if (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0)
nOS_Error nOS_ThreadSetPriority (nOS_Thread *thread, uint8_t prio)
{
    nOS_Error   err;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    if (prio > NOS_CONFIG_HIGHEST_THREAD_PRIO) {
        err = NOS_E_INV_VAL;
    } else if (thread->state == NOS_THREAD_STOPPED) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
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

int16_t nOS_ThreadGetPriority (nOS_Thread *thread)
{
    int16_t prio;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    if (thread->state == NOS_THREAD_STOPPED) {
        prio = -1;
    } else
#endif
    {
        nOS_CriticalEnter();
        prio = thread->prio;
        nOS_CriticalLeave();
    }

    return prio;
}
#endif  /* NOS_CONFIG_THREAD_SET_PRIO_ENABLE */

nOS_Thread* nOS_ThreadRunning(void)
{
    return nOS_runningThread;
}

#if defined(__cplusplus)
}
#endif
