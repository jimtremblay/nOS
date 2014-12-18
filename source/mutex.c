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

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
/*
 * Name        : nOS_MutexCreate
 *
 * Description : Initialize a mutex object. A mutex can only be locked from thread
 *               and unlocked from mutex owner. Any attempt to unlock a mutex that
 *               not belong to the caller thread will failed.
 *
 * Arguments   : mutex : Pointer to mutex object.
 *               type  : Type of mutex to create.
 *                       NOS_MUTEX_NORMAL    : Standard mutex (like binary
 *                                             semaphore).
 *                       NOS_MUTEX_RECURSIVE : Mutex that can be locked
 *                                             recursively.
 *               prio  : NOS_MUTEX_PRIO_INHERIT : Mutex owner inherit higher prio
 *                                                from other threads that try to
 *                                                lock this mutex.
 *                       prio > 0               : Mutex owner increase its prio
 *                                                to this value when it lock the
 *                                                mutex (ceiling protocol).
 *
 * Return      : Error code.
 *               NOS_E_NULL    : Pointer to mutex object is NULL.
 *               NOS_E_INV_VAL : Type of mutex is not valid.
 *               NOS_OK        : Mutex initialized with success.
 *
 * Note        : Mutex object must be created before using it, else
 *               behaviour is undefined and must be called one time
 *               ONLY for each mutex object.
 */
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
nOS_Error nOS_MutexCreate (nOS_Mutex *mutex, uint8_t prio, uint8_t type)
#else
nOS_Error nOS_MutexCreate (nOS_Mutex *mutex, uint8_t prio)
#endif
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else if (mutex->e.type != NOS_EVENT_INVALID) {
        err = NOS_E_INV_OBJ;
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
    } else if ((type != NOS_MUTEX_NORMAL) && (type != NOS_MUTEX_RECURSIVE)) {
        err = NOS_E_INV_VAL;
#endif
    } else
#endif
    {
        nOS_CriticalEnter();
#if (NOS_CONFIG_SAFE > 0)
        nOS_EventCreate((nOS_Event*)mutex, NOS_EVENT_MUTEX);
#else
        nOS_EventCreate((nOS_Event*)mutex);
#endif
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
        mutex->type = type;
        mutex->count = 0;
#endif
        mutex->owner = NULL;
        mutex->prio = prio;
        mutex->backup = 0;
        nOS_CriticalLeave();
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
        err = NOS_E_NULL;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
        mutex->count = 0;
#endif
        mutex->owner = NULL;
        if (nOS_EventDelete((nOS_Event*)mutex)) {
            nOS_Sched();
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}
#endif

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

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_VAL;
    } else
#endif
    /* Can't lock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        /* Mutex available? Reserve it for calling thread */
        if (mutex->owner == NULL) {
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
            mutex->count++;
#endif
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
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
            if (mutex->type == NOS_MUTEX_RECURSIVE) {
                if (mutex->count < UINT8_MAX) {
                    mutex->count++;
                    err = NOS_OK;
                } else {
                    err = NOS_E_OVERFLOW;
                }
            /* Binary mutex */
            } else
#endif
            {
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
        }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        /* Can't switch context when scheduler is locked */
        else if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        }
#endif
        /* Main thread can't wait */
        else if (nOS_runningThread == &nOS_mainThread) {
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
            err = nOS_EventWait((nOS_Event*)mutex, NOS_THREAD_LOCKING_MUTEX, tout);
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

#if (NOS_CONFIG_SAFE > 0)
    if (mutex == NULL) {
        err = NOS_E_NULL;
    } else if (mutex->e.type != NOS_EVENT_MUTEX) {
        err = NOS_E_INV_VAL;
    } else
#endif
    /* Can't unlock mutex from ISR */
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        if (mutex->owner != NULL) {
            if (mutex->owner == nOS_runningThread) {
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
                mutex->count--;
                if (mutex->count == 0)
#endif
                {
                    SetThreadPriority(mutex->owner, mutex->backup);
                    thread = nOS_EventSignal((nOS_Event*)mutex, NOS_OK);
                    if (thread != NULL) {
#if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
                        mutex->count++;
#endif
                        mutex->owner = thread;
                        mutex->backup = thread->prio;
                        if (mutex->prio != NOS_MUTEX_PRIO_INHERIT) {
                            SetThreadPriority(thread, mutex->prio);
                        }
                        if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
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
        nOS_CriticalEnter();
        locked = (mutex->owner != NULL);
        nOS_CriticalLeave();
    }

    return locked;
}

nOS_Thread* nOS_MutexOwner (nOS_Mutex *mutex)
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
        nOS_CriticalEnter();

        owner = mutex->owner;
        nOS_CriticalLeave();
    }

    return owner;
}
#endif  /* NOS_CONFIG_MUTEX_ENABLE */

#if defined(__cplusplus)
}
#endif
