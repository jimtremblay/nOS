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

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
nOS_Error nOS_MutexCreate (nOS_Mutex *mutex,
                           nOS_MutexType type
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                           ,uint8_t prio
#endif
                           )
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (mutex->e.type != NOS_EVENT_INVALID) {
        err = NOS_E_INV_OBJ;
    } else if ((type != NOS_MUTEX_NORMAL) && (type != NOS_MUTEX_RECURSIVE)) {
        err = NOS_E_INV_VAL;
    }
    else
#endif
    {
        nOS_EnterCritical();
#if (NOS_CONFIG_SAFE > 0)
        nOS_CreateEvent((nOS_Event*)mutex, NOS_EVENT_MUTEX);
#else
        nOS_CreateEvent((nOS_Event*)mutex);
#endif
        mutex->owner = NULL;
        mutex->type = type;
        mutex->count = 0;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        mutex->prio = prio;
        mutex->backup = 0;
#endif
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

#if (NOS_CONFIG_MUTEX_DELETE_ENABLE > 0)
nOS_Error nOS_MutexDelete (nOS_Mutex *mutex)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical();
        mutex->owner = NULL;
        mutex->count = 0;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (nOS_DeleteEvent((nOS_Event*)mutex)) {
            nOS_Schedule();
        }
#else
        nOS_DeleteEvent((nOS_Event*)mutex);
#endif
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}
#endif

nOS_Error nOS_MutexLock (nOS_Mutex *mutex, nOS_TickCounter tout)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    /* Can't lock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_EnterCritical();
        /* Mutex available? Reserve it for calling thread */
        if (mutex->owner == NULL) {
            mutex->count++;
            mutex->owner = nOS_runningThread;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            mutex->backup = nOS_runningThread->prio;
            if (mutex->prio != NOS_MUTEX_PRIO_INHERIT) {
                if (nOS_runningThread->prio < mutex->prio) {
                    nOS_SetThreadPrio(nOS_runningThread, mutex->prio);
                }
            }
#endif
            err = NOS_OK;
        } else if (mutex->owner == nOS_runningThread) {
            /* Mutex owner relock it? */
            if (mutex->type == NOS_MUTEX_RECURSIVE) {
                if (mutex->count < NOS_MUTEX_COUNT_MAX) {
                    mutex->count++;
                    err = NOS_OK;
                } else {
                    err = NOS_E_OVERFLOW;
                }
            } else {
                /* Binary mutex: can't relock */
                err = NOS_E_OVERFLOW;
            }
        } else if (tout == NOS_NO_WAIT) {
            /* Calling thread can't wait? Try again. */
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            /*
             * If current thread can ask to lock mutex,
             * maybe is prio is higher than mutex owner.
             */
            if (mutex->prio == NOS_MUTEX_PRIO_INHERIT) {
                if (mutex->owner->prio < nOS_runningThread->prio) {
                    nOS_SetThreadPrio(mutex->owner, nOS_runningThread->prio);
                }
            }
#endif
            err = NOS_E_AGAIN;
        }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        /* Can't switch context when scheduler is locked */
        else if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        }
#endif
        else if (nOS_runningThread == &nOS_idleHandle) {
            /* Main thread can't wait */
            err = NOS_E_IDLE;
        } else {
            /* Calling thread must wait on mutex. */
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            /* If current thread can ask to lock mutex, maybe running
             * thread prio is higher than mutex owner prio. */
            if (mutex->prio == NOS_MUTEX_PRIO_INHERIT) {
                if (mutex->owner->prio < nOS_runningThread->prio) {
                    nOS_SetThreadPrio(mutex->owner, nOS_runningThread->prio);
                }
            }
#endif
            err = nOS_WaitForEvent((nOS_Event*)mutex, NOS_THREAD_LOCKING_MUTEX, tout);
        }
        nOS_LeaveCritical();
    }

    return err;
}

nOS_Error nOS_MutexUnlock (nOS_Mutex *mutex)
{
    nOS_Thread      *thread;
    nOS_Error       err;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    /* Can't unlock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_EnterCritical();
        if (mutex->owner != NULL) {
            if (mutex->owner == nOS_runningThread) {
                mutex->count--;
                if (mutex->count == 0) {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                    nOS_SetThreadPrio(mutex->owner, mutex->backup);
#endif
                    thread = nOS_SignalEvent((nOS_Event*)mutex, NOS_OK);
                    if (thread != NULL) {
                        mutex->count++;
                        mutex->owner = thread;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                        mutex->backup = thread->prio;
                        if (mutex->prio != NOS_MUTEX_PRIO_INHERIT) {
                            nOS_SetThreadPrio(thread, mutex->prio);
                        }
 #if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                        if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                            nOS_Schedule();
                        }
 #endif
#endif
                    } else {
                        mutex->owner = NULL;
                    }
                }
                err = NOS_OK;
            } else {
                err = NOS_E_OWNER;
            }
        } else {
            err = NOS_E_UNDERFLOW;
        }
        nOS_LeaveCritical();
    }

    return err;
}

bool nOS_MutexIsLocked (nOS_Mutex *mutex)
{
    bool    locked;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        locked = false;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        locked = false;
    } else
#endif
    {
        nOS_EnterCritical();
        locked = (mutex->owner != NULL);
        nOS_LeaveCritical();
    }

    return locked;
}

nOS_Thread* nOS_MutexGetOwner (nOS_Mutex *mutex)
{
    nOS_Thread *owner;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        owner = NULL;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        owner = NULL;
    } else
#endif
    {
        nOS_EnterCritical();
        owner = mutex->owner;
        nOS_LeaveCritical();
    }

    return owner;
}
#endif  /* NOS_CONFIG_MUTEX_ENABLE */

#ifdef __cplusplus
}
#endif
