/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

    if (thread != &nOS_idleHandle) {
        if (thread->state == NOS_THREAD_READY) {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            RemoveThreadFromReadyList(thread);
#else
            nOS_ListRemove(&nOS_readyList, &thread->readyWait);
#endif
        }
        thread->state |= NOS_THREAD_SUSPENDED;
    }
}

/* This is an internal function */
void ResumeThread (void *payload, void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)payload;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool        *sched =  (bool*)arg;
#else
    NOS_UNUSED(arg);
#endif

    if (thread->state & NOS_THREAD_SUSPENDED) {
        thread->state &=~ NOS_THREAD_SUSPENDED;
        if (thread->state == NOS_THREAD_READY) {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            AppendThreadToReadyList(thread);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (thread->prio > nOS_runningThread->prio) {
                if (sched != NULL) {
                    *sched = true;
                }
            }
#endif
#else
            nOS_ListAppend(&nOS_readyList, &thread->readyWait);
#endif
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
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        RemoveThreadFromReadyList(thread);
#else
        nOS_ListRemove(&nOS_readyList, &thread->readyWait);
#endif
    } else if (thread->state & NOS_THREAD_WAITING_EVENT) {
        nOS_ListRemove(&thread->event->waitList, &thread->readyWait);
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

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
    if (thread->state & NOS_THREAD_SLEEPING_UNTIL) {
        if (nOS_tickCounter == thread->timeout) {
            SignalThread(thread, NOS_OK);
        }
    } else
#endif
    if (thread->state & (NOS_THREAD_WAITING_EVENT
#if (NOS_CONFIG_SLEEP_ENABLE > 0)
                         | NOS_THREAD_SLEEPING
#endif
                         )
    ) {
        if (thread->timeout > 0) {
            thread->timeout--;
            if (thread->timeout == 0) {
#if (NOS_CONFIG_SLEEP_ENABLE > 0)
                if (thread->state & NOS_THREAD_SLEEPING) {
                    SignalThread(thread, NOS_OK);
                } else
#endif
                {
                    SignalThread(thread, NOS_E_TIMEOUT);
                }
            }
        }
    }
}

/* This is an internal function */
void SignalThread (nOS_Thread *thread, nOS_Error err)
{
    if (thread->event != NULL) {
        nOS_ListRemove(&thread->event->waitList, &thread->readyWait);
    }
    thread->error = err;
    thread->state &=~ (NOS_THREAD_WAITING_EVENT
#if (NOS_CONFIG_SLEEP_ENABLE > 0)
                       | NOS_THREAD_SLEEPING
#endif
#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                       | NOS_THREAD_SLEEPING_UNTIL
#endif
                       );
    thread->timeout = 0;
    if (thread->state == NOS_THREAD_READY) {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        AppendThreadToReadyList(thread);
#else
        nOS_ListAppend(&nOS_readyList, &thread->readyWait);
#endif
    }
}

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
#if (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0) || (NOS_CONFIG_MUTEX_ENABLE > 0)
/* This is an internal function */
void ChangeThreadPrio (nOS_Thread *thread, uint8_t prio)
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
#endif

nOS_Error nOS_ThreadCreate (nOS_Thread *thread,
                            nOS_ThreadEntry entry,
                            void *arg,
                            nOS_Stack *stack,
                            size_t ssize
#if defined(__ICCAVR__)
                            ,size_t cssize
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                            ,uint8_t prio
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                            ,uint8_t state
#endif
                            )
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (thread == &nOS_idleHandle) {
        err = NOS_E_INV_VAL;
    } else if (entry == NULL) {
        err = NOS_E_INV_VAL;
    } else if (stack == NULL) {
        err = NOS_E_INV_VAL;
    } else if (ssize == 0) {
        err = NOS_E_INV_VAL;
    }
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    else if (prio > NOS_CONFIG_HIGHEST_THREAD_PRIO) {
        err = NOS_E_INV_VAL;
    }
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
    else if ((state != NOS_THREAD_READY) && (state != NOS_THREAD_SUSPENDED)) {
        err = NOS_E_INV_VAL;
    }
#endif
    else if (thread->state != NOS_THREAD_STOPPED) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        thread->prio = prio;
#endif
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
        thread->main.payload = thread;
        thread->readyWait.payload = thread;
        nOS_ContextInit(thread, stack, ssize
#if defined(__ICCAVR__)
                        ,cssize
#endif
                        ,entry, arg);
        nOS_CriticalEnter();
        nOS_ListAppend(&nOS_mainList, &thread->main);
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
        if (thread->state == NOS_THREAD_READY)
#endif
        {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            AppendThreadToReadyList(thread);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (prio > nOS_runningThread->prio) {
                nOS_Sched();
            }
#endif
#else
            nOS_ListAppend(&nOS_readyList, &thread->readyWait);
#endif
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
    if (thread == &nOS_idleHandle) {
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

void nOS_ThreadTick(void)
{
    nOS_CriticalEnter();
    nOS_ListWalk(&nOS_mainList, TickThread, NULL);
#if (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE > 0)
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
#else
    nOS_ListRotate(&nOS_readyList);
#endif
#endif
    nOS_CriticalLeave();
}

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
nOS_Error nOS_ThreadSuspend (nOS_Thread *thread)
{
    nOS_Error   err;

    if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    /* Main thread can't be suspended */
    if (thread == &nOS_idleHandle) {
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
    } else if ((thread->state & NOS_THREAD_SUSPENDED) == NOS_THREAD_SUSPENDED) {
        err = NOS_E_INV_STATE;
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
    /* Can't suspend all threads from any thread (except idle) when scheduler is locked */
    if ((nOS_lockNestingCounter > 0) && (nOS_runningThread != &nOS_idleHandle)) {
        err = NOS_E_LOCKED;
    } else
#endif
#endif
    {
        nOS_CriticalEnter();
        nOS_ListWalk(&nOS_mainList, SuspendThread, NULL);
        nOS_CriticalLeave();
        if (nOS_runningThread != &nOS_idleHandle) {
            nOS_Sched();
        }
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadResume (nOS_Thread *thread)
{
    nOS_Error   err;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool        sched = false;
#endif

#if (NOS_CONFIG_SAFE > 0)
    if (thread == NULL) {
        err = NOS_E_NULL;
    } else if (thread == nOS_runningThread) {
        err = NOS_E_INV_VAL;
    } else if (thread == &nOS_idleHandle) {
        err = NOS_E_IDLE;
    } else if (thread->state == NOS_THREAD_STOPPED) {
        err = NOS_E_NOT_CREATED;
    } else if ((thread->state & NOS_THREAD_SUSPENDED) != NOS_THREAD_SUSPENDED) {
        err = NOS_E_INV_STATE;
    } else
#endif
    {
        nOS_CriticalEnter();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        ResumeThread(thread, &sched);
#else
        ResumeThread(thread, NULL);
#endif
        nOS_CriticalLeave();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (sched) {
            nOS_Sched();
        }
#endif
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadResumeAll (void)
{
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool sched = false;
#endif

    nOS_CriticalEnter();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    nOS_ListWalk(&nOS_mainList, ResumeThread, &sched);
#else
    nOS_ListWalk(&nOS_mainList, ResumeThread, NULL);
#endif
    nOS_CriticalLeave();

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    if (sched) {
        nOS_Sched();
    }
#endif

    return NOS_OK;
}
#endif  /* NOS_CONFIG_THREAD_SUSPEND_ENABLE */

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0)
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
        err = NOS_E_NOT_CREATED;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (prio != thread->prio) {
            ChangeThreadPrio(thread, prio);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (prio > nOS_runningThread->prio) {
                nOS_Sched();
            }
#endif
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
#endif  /* (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && NOS_CONFIG_THREAD_SET_PRIO_ENABLE */

nOS_Thread* nOS_ThreadRunning(void)
{
    return nOS_runningThread;
}

#if defined(__cplusplus)
}
#endif
