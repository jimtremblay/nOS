/*
 * Copyright (c) 2014-2016 Jim Tremblay
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

#if (NOS_CONFIG_BARRIER_ENABLE > 0)
nOS_Error nOS_BarrierCreate (nOS_Barrier *barrier, uint8_t max)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (barrier == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (max == 0) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (barrier->e.type != NOS_EVENT_INVALID) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
#if (NOS_CONFIG_SAFE > 0)
            nOS_CreateEvent((nOS_Event*)barrier, NOS_EVENT_BARRIER);
#else
            nOS_CreateEvent((nOS_Event*)barrier);
#endif
            barrier->count = max;
            barrier->max   = max;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_BARRIER_DELETE_ENABLE > 0)
nOS_Error nOS_BarrierDelete (nOS_Barrier *barrier)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (barrier == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (barrier->e.type != NOS_EVENT_BARRIER) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            barrier->count = 0;
            barrier->max   = 0;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (nOS_DeleteEvent((nOS_Event*)barrier)) {
                nOS_Schedule();
            }
#else
            nOS_DeleteEvent((nOS_Event*)barrier);
#endif
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

nOS_Error nOS_BarrierWait (nOS_Barrier *barrier)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (barrier == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (barrier->e.type != NOS_EVENT_BARRIER) {
            /* Not a barrier event object */
            err = NOS_E_INV_OBJ;
        } else if (nOS_isrNestingCounter > 0) {
            /* Can't wait from ISR */
            err = NOS_E_ISR;
        }
 #if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        else if (nOS_lockNestingCounter > 0) {
            /* Can't switch context when scheduler is locked */
            err = NOS_E_LOCKED;
        }
 #endif
#endif
        else {
            barrier->count--;
            if (barrier->count == 0) {
                barrier->count = barrier->max;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                /* Wake up all threads waiting on barrier */
                if (nOS_BroadcastEvent((nOS_Event*)barrier, NOS_OK)) {
                    nOS_Schedule();
                }
#else
                nOS_BroadcastEvent((nOS_Event*)barrier, NOS_OK);
#endif
                err = NOS_OK;
            }
#if (NOS_CONFIG_SAFE > 0)
            else if (nOS_runningThread == &nOS_idleHandle) {
                barrier->count++;
                /* Main thread can't wait */
                err = NOS_E_IDLE;
            }
#endif
            else {
                /* Calling thread must wait for other threads. */
                err = nOS_WaitForEvent((nOS_Event*)barrier,
                                       NOS_THREAD_ON_BARRIER
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                                       ,NOS_WAIT_INFINITE
#endif
                                       );
            }
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#endif  /* NOS_CONFIG_BARRIER_ENABLE */

#ifdef __cplusplus
}
#endif
