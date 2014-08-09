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

#if (NOS_CONFIG_SEM_ENABLE > 0)
nOS_Error nOS_SemCreate (nOS_Sem *sem, uint16_t count, uint16_t max)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_NULL;
    } else if (max == 0) {
        err = NOS_E_INV_VAL;
    } else if (count > max) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        nOS_EventCreate((nOS_Event*)sem);
        sem->count = count;
        sem->max = max;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

/* nOS_SemTake
 * sem: must be a valid semaphore object
 * tout: NOS_NO_TIMEOUT = wait indefinitely
 *       != 0 = number of ticks to wait
 * Can be called from threads
 * NOS_OK: Succeed to take semaphore
 * NOS_E_LOCKED: Thread can't wait for semaphore when scheduler is locked
 * NOS_E_ISR: Thread can't wait for semaphore from ISR
 * NOS_E_IDLE: Idle thread can't wait for semaphore
 * NOS_E_TIMEOUT: Semaphore can't be taken in required time
 */
nOS_Error nOS_SemTake (nOS_Sem *sem, uint16_t tout)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        /* Sem available? Take it */
        if (sem->count > 0) {
            sem->count--;
            err = NOS_OK;
        /* Calling thread can't wait? Try again */
        } else if (tout == NOS_NO_WAIT) {
            err = NOS_E_AGAIN;
        /* Can't wait from ISR */
        } else if (nOS_isrNestingCounter > 0) {
            err = NOS_E_ISR;
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
        /* Calling thread must wait on sem. */
        } else {
            err = nOS_EventWait((nOS_Event*)sem, NOS_TAKING_SEM, tout);
        }
        nOS_CriticalLeave();
    }

    return err;
}

/* nOS_SemGive
 * sem: must be a valid semaphore object
 * Can be called from threads, idle and ISR
 * NOS_OK: Succeed to give semaphore
 * NOS_E_OVERFLOW: Too much semaphore given, limit exceeded
 */
nOS_Error nOS_SemGive (nOS_Sem *sem)
{
    nOS_Thread  *thread;
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        thread = nOS_EventSignal((nOS_Event*)sem);
        if (thread != NULL) {
            if ((thread->state == NOS_READY) && (thread->prio > nOS_runningThread->prio)) {
                nOS_Sched();
            }
            err = NOS_OK;
        } else if (sem->count < sem->max) {
            sem->count++;
            err = NOS_OK;
        } else {
            err = NOS_E_OVERFLOW;
        }
        nOS_CriticalLeave();
    }

    return err;
}
#endif  /* NOS_CONFIG_SEM_ENABLE */

#if defined(__cplusplus)
}
#endif
