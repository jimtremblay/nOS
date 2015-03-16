/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
/* This is an internal function */
static void _SuspendThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (thread != &nOS_idleHandle) {
        if (thread->state == NOS_THREAD_READY) {
            nOS_RemoveThreadFromReadyList(thread);
        }
        thread->state = (nOS_ThreadState)(thread->state | NOS_THREAD_SUSPENDED);
    }
}

/* This is an internal function */
static void _ResumeThread (void *payload, void *arg)
{
    nOS_Thread  *thread = (nOS_Thread*)payload;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool        *sched =  (bool*)arg;
#else
    NOS_UNUSED(arg);
#endif

    if (thread->state & NOS_THREAD_SUSPENDED) {
        thread->state = (nOS_ThreadState)(thread->state &~ NOS_THREAD_SUSPENDED);
        if (thread->state == NOS_THREAD_READY) {
            nOS_AppendThreadToReadyList(thread);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (thread->prio > nOS_runningThread->prio) {
                if (sched != NULL) {
                    *sched = true;
                }
            }
#endif
        }
    }
}
#endif

/* This is an internal function */
void nOS_TickThread (void *payload, void *arg)
{
    nOS_Thread *thread = (nOS_Thread*)payload;

    /* Avoid warning */
    NOS_UNUSED(arg);

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
    if (thread->state & NOS_THREAD_SLEEPING_UNTIL) {
        if (nOS_tickCounter == thread->timeout) {
            nOS_SignalThread(thread, NOS_OK);
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
                    nOS_SignalThread(thread, NOS_OK);
                } else
#endif
                {
                    nOS_SignalThread(thread, NOS_E_TIMEOUT);
                }
            }
        }
    }
}

/* This is an internal function */
void nOS_SignalThread (nOS_Thread *thread, nOS_Error err)
{
    if (thread->event != NULL) {
        nOS_RemoveFromList(&thread->event->waitList, &thread->readyWait);
    }
    thread->error = err;
    thread->state = (nOS_ThreadState)(thread->state &~ NOS_THREAD_STATE_WAIT_MASK);
    thread->timeout = 0;
    if (thread->state == NOS_THREAD_READY) {
        nOS_AppendThreadToReadyList(thread);
    }
}

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
 #if (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0) || (NOS_CONFIG_MUTEX_ENABLE > 0)
/* This is an internal function */
void nOS_SetThreadPrio (nOS_Thread *thread, uint8_t prio)
{
    if (thread->prio != prio)
    {
        if (thread->state == NOS_THREAD_READY)
        {
            nOS_RemoveThreadFromReadyList(thread);
        }
        thread->prio = prio;
        if (thread->state == NOS_THREAD_READY)
        {
            nOS_AppendThreadToReadyList(thread);
        }
    }
}
 #endif
#endif

nOS_Error nOS_ThreadCreate (nOS_Thread *thread,
                            nOS_ThreadEntry entry,
                            void *arg
#ifndef NOS_SIMULATED_STACK
                            ,nOS_Stack *stack
#endif
                            ,size_t ssize
#ifdef NOS_USE_SEPARATE_CALL_STACK
                            ,size_t cssize
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                            ,uint8_t prio
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                            ,nOS_ThreadState state
#endif
#if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                            ,const char *name
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
    }
#ifndef NOS_SIMULATED_STACK
    else if (stack == NULL) {
        err = NOS_E_INV_VAL;
    } else if (ssize == 0) {
        err = NOS_E_INV_VAL;
    }
#endif
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
            thread->state = (nOS_ThreadState)(thread->state | NOS_THREAD_SUSPENDED);
        }
#endif
        thread->timeout = 0;
        thread->event = NULL;
        thread->ext = NULL;
#if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
        thread->name = name;
#endif
        thread->error = NOS_OK;
        thread->node.payload = thread;
        thread->readyWait.payload = thread;
        nOS_InitContext(thread
#ifndef NOS_SIMULATED_STACK
                        ,stack
#endif
                        ,ssize
#ifdef NOS_USE_SEPARATE_CALL_STACK
                        ,cssize
#endif
                        ,entry, arg);
        nOS_EnterCritical();
        nOS_AppendToList(&nOS_mainList, &thread->node);
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
        if (thread->state == NOS_THREAD_READY)
#endif
        {
            nOS_AppendThreadToReadyList(thread);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
 #ifdef NOS_PSEUDO_SCHEDULER
            if (nOS_runningThread == NULL) {
                nOS_Schedule();
            } else
 #endif
            if (prio > nOS_runningThread->prio) {
                nOS_Schedule();
            }
#endif
        }
        nOS_LeaveCritical();
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
        nOS_EnterCritical();
        if (thread->state == NOS_THREAD_READY) {
            nOS_RemoveThreadFromReadyList(thread);
        } else if (thread->state & NOS_THREAD_WAITING_EVENT) {
            nOS_RemoveFromList(&thread->event->waitList, &thread->readyWait);
        }
        thread->state   = NOS_THREAD_STOPPED;
        thread->event   = NULL;
        thread->ext     = NULL;
        thread->timeout = 0;
        thread->error   = NOS_OK;
        if (thread == nOS_runningThread) {
            /* Will never return */
            nOS_Schedule();
        }
        nOS_LeaveCritical();
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
 #ifndef NOS_PSEUDO_SCHEDULER
    /* Main thread can't be suspended */
    if (thread == &nOS_idleHandle) {
        err = NOS_E_IDLE;
    } else
 #endif
    if (thread == nOS_runningThread) {
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
        nOS_EnterCritical();
        _SuspendThread(thread, NULL);
        if (thread == nOS_runningThread) {
            nOS_Schedule();
        }
        nOS_LeaveCritical();
    }

    return err;
}

nOS_Error nOS_ThreadSuspendAll (void)
{
    nOS_Error   err;

#ifndef NOS_PSEUDO_SCHEDULER
 #if (NOS_CONFIG_SAFE > 0)
  #if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* Can't suspend all threads from any thread (except idle) when scheduler is locked */
    if ((nOS_lockNestingCounter > 0) && (nOS_runningThread != &nOS_idleHandle)) {
        err = NOS_E_LOCKED;
    } else
  #endif
 #endif
#endif
    {
        nOS_EnterCritical();
        nOS_WalkInList(&nOS_mainList, _SuspendThread, NULL);
        if (nOS_runningThread != &nOS_idleHandle) {
            nOS_Schedule();
        }
        nOS_LeaveCritical();
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
    }
#ifndef NOS_PSEUDO_SCHEDULER
    else if (thread == &nOS_idleHandle) {
        err = NOS_E_IDLE;
    }
#endif
    else if (thread->state == NOS_THREAD_STOPPED) {
        err = NOS_E_NOT_CREATED;
    } else if ((thread->state & NOS_THREAD_SUSPENDED) != NOS_THREAD_SUSPENDED) {
        err = NOS_E_INV_STATE;
    } else
#endif
    {
        nOS_EnterCritical();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        _ResumeThread(thread, &sched);
#else
        _ResumeThread(thread, NULL);
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (sched) {
            nOS_Schedule();
        }
#endif
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_ThreadResumeAll (void)
{
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool sched = false;
#endif

    nOS_EnterCritical();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    nOS_WalkInList(&nOS_mainList, _ResumeThread, &sched);
#else
    nOS_WalkInList(&nOS_mainList, _ResumeThread, NULL);
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    if (sched) {
        nOS_Schedule();
    }
#endif
    nOS_LeaveCritical();

    return NOS_OK;
}
#endif

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
        nOS_EnterCritical();
        if (prio != thread->prio) {
            nOS_SetThreadPrio(thread, prio);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
 #ifdef NOS_PSEUDO_SCHEDULER
            if (nOS_runningThread == NULL) {
                nOS_Schedule();
            } else
 #endif
            if (prio > nOS_runningThread->prio) {
                nOS_Schedule();
            }
#endif
        }
        nOS_LeaveCritical();
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
        nOS_EnterCritical();
        prio = thread->prio;
        nOS_LeaveCritical();
    }

    return prio;
}
#endif

#if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
const char* nOS_ThreadGetName (nOS_Thread *thread)
{
	const char *name = NULL;

	if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    if (thread->state != NOS_THREAD_STOPPED)
#endif
	{
		nOS_EnterCritical();
		name = thread->name;
		nOS_LeaveCritical();
	}

	return name;
}

void nOS_ThreadSetName (nOS_Thread *thread, const char *name)
{
	if (thread == NULL) {
        thread = nOS_runningThread;
    }

#if (NOS_CONFIG_SAFE > 0)
    if ((thread->state != NOS_THREAD_STOPPED) &&
        (name != NULL))
#endif
	{
		nOS_EnterCritical();
		thread->name = name;
		nOS_LeaveCritical();
	}
}
#endif

#ifdef __cplusplus
}
#endif
