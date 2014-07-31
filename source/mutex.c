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

nOS_Error nOS_MutexCreate (nOS_Mutex *mutex, uint8_t type, uint8_t prio)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else if ((type != NOS_MUTEX_NORMAL) && (type != NOS_MUTEX_RECURSIVE)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        nOS_EventCreate((nOS_Event*)mutex);
        mutex->type = type;
        mutex->count = 0;
        mutex->owner = NULL;
        mutex->prio = prio;
        mutex->backup = 0;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

/* nOS_MutexLock
 * mutex: must be a valid mutex object
 * tout: NOS_NO_TIMEOUT = wait indefinitely
 *       != 0 = number of ticks to wait
 * Can be called from threads
 * NOS_OK: Succeed to lock mutex
 * NOS_E_LOCKED: Thread can't wait for mutex when scheduler is locked
 * NOS_E_ISR: Mutex can't be locked from ISR
 * NOS_E_IDLE: Idle thread can't wait for mutex
 * NOS_E_TIMEOUT: Mutex can't be taken in required time
 * NOS_E_OVERFLOW: Mutex have been locked too much times
 */
nOS_Error nOS_MutexLock (nOS_Mutex *mutex, uint16_t tout)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    /* Can't lock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        /* Mutex available? Reserve it for calling thread */
        if (mutex->owner == NULL) {
            mutex->count++;
            mutex->owner = nOS_runningThread;
            mutex->backup = nOS_runningThread->prio;
            if (mutex->prio != NOS_MUTEX_PRIO_INHERIT) {
                if (nOS_runningThread->prio < mutex->prio) {
                    SetThreadPriority(nOS_runningThread, mutex->prio);
                }
            }
            err = NOS_OK;
        /* Mutex owner relock it? */
        } else if (mutex->owner == nOS_runningThread) {
            if (mutex->type == NOS_MUTEX_RECURSIVE) {
                if (mutex->count < UINT8_MAX) {
                    mutex->count++;
                    err = NOS_OK;
                } else {
                    err = NOS_E_OVERFLOW;
                }
            /* Binary mutex */
            } else {
                err = NOS_E_OVERFLOW;
            }
        /* Calling thread can't wait? Try again. */
        } else if (tout == NOS_NO_WAIT) {
            /*
             * If current thread can ask to lock mutex,
             * maybe is prio is higher than mutex owner.
             */
            if (mutex->prio == NOS_MUTEX_PRIO_INHERIT) {
                if (mutex->owner->prio < nOS_runningThread->prio) {
                    SetThreadPriority(mutex->owner, nOS_runningThread->prio);
                }
            }
            err = NOS_E_AGAIN;
        /* Can't switch context when scheduler is locked */
        } else if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        /* Main thread can't wait */
        } else if (nOS_runningThread == &nOS_mainThread) {
            err = NOS_E_IDLE;
        /* Calling thread must wait on mutex. */
        } else {
            /*
             * If current thread can ask to lock mutex,
             * maybe is prio is higher than mutex owner.
             */
            if (mutex->prio == NOS_MUTEX_PRIO_INHERIT) {
                if (mutex->owner->prio < nOS_runningThread->prio) {
                    SetThreadPriority(mutex->owner, nOS_runningThread->prio);
                }
            }
            err = nOS_EventWait((nOS_Event*)mutex, NOS_LOCKING_MUTEX, tout);
        }
        nOS_CriticalLeave();
    }

    return err;
}

/* nOS_MutexUnlock
 * mutex: must be a valid mutex object
 * tout: NOS_NO_TIMEOUT = wait indefinitely
 *       != 0 = number of ticks to wait
 * Can be called from threads and idle
 * NOS_OK: Succeed to unlock mutex
 * NOS_E_ISR: Mutex can't be unlocked from ISR
 * NOS_E_OWNER: You can't unlock the mutex for the owner
 * NOS_E_UNDERFLOW: Mutex have been unlocked too much times
 */
nOS_Error nOS_MutexUnlock (nOS_Mutex *mutex)
{
    nOS_Thread      *thread;
    nOS_Error       err;

#if NOS_CONFIG_SAFE > 0
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    /* Can't unlock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        if (mutex->owner != NULL) {
            if (mutex->owner == nOS_runningThread) {
                mutex->count--;
                if (mutex->count == 0) {
                    SetThreadPriority(mutex->owner, mutex->backup);
                    thread = nOS_EventSignal((nOS_Event*)mutex);
                    if (thread != NULL) {
                        mutex->count++;
                        mutex->owner = thread;
                        mutex->backup = thread->prio;
                        if (mutex->prio != NOS_MUTEX_PRIO_INHERIT) {
                            SetThreadPriority(thread, mutex->prio);
                        }
                        if ((thread->state == NOS_READY) && (thread->prio > nOS_runningThread->prio)) {
                            nOS_Sched();
                        }
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
        nOS_CriticalLeave();
    }

    return err;
}

#if defined(__cplusplus)
}
#endif
