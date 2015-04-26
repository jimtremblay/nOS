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

#if (NOS_CONFIG_SEM_ENABLE > 0)
nOS_Error nOS_SemCreate (nOS_Sem *sem, nOS_SemCounter count, nOS_SemCounter max)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (count > max) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (sem->e.type != NOS_EVENT_INVALID) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
#if (NOS_CONFIG_SAFE > 0)
            nOS_CreateEvent((nOS_Event*)sem, NOS_EVENT_SEM);
#else
            nOS_CreateEvent((nOS_Event*)sem);
#endif
            sem->count = count;
            sem->max = max;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_SEM_DELETE_ENABLE > 0)
nOS_Error nOS_SemDelete (nOS_Sem *sem)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (sem->e.type != NOS_EVENT_SEM) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            sem->count = 0;
            sem->max = 0;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (nOS_DeleteEvent((nOS_Event*)sem)) {
                nOS_Schedule();
            }
#else
            nOS_DeleteEvent((nOS_Event*)sem);
#endif
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

nOS_Error nOS_SemTake (nOS_Sem *sem, nOS_TickCounter tout)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (sem->e.type != NOS_EVENT_SEM) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            if (sem->count > 0) {
                /* Sem available. */
                sem->count--;
                err = NOS_OK;
            } else if (tout == NOS_NO_WAIT) {
                /* Calling thread can't wait. */
                err = NOS_E_AGAIN;
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
            else if (nOS_runningThread == &nOS_idleHandle) {
                /* Main thread can't wait */
                err = NOS_E_IDLE;
            } else {
                /* Calling thread must wait on sem. */
                err = nOS_WaitForEvent((nOS_Event*)sem, NOS_THREAD_TAKING_SEM, tout);
            }
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_SemGive (nOS_Sem *sem)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_Thread      *thread;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (sem->e.type != NOS_EVENT_SEM) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            thread = nOS_SendEvent((nOS_Event*)sem, NOS_OK);
            if (thread != NULL) {
    #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                    nOS_Schedule();
                }
    #endif

                err = NOS_OK;
            } else if (sem->count < sem->max) {
                sem->count++;

                err = NOS_OK;
            } else if (sem->max > 0) {
                err = NOS_E_OVERFLOW;
            } else {
                /* No thread waiting to consume sem, inform producer */
                err = NOS_E_NO_CONSUMER;
            }
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

bool nOS_SemIsAvailable (nOS_Sem *sem)
{
    nOS_StatusReg   sr;
    bool            avail;

#if (NOS_CONFIG_SAFE > 0)
    if (sem == NULL) {
        avail = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (sem->e.type != NOS_EVENT_SEM) {
            avail = false;
        } else
#endif
        {
            avail = (sem->count > 0);
        }
        nOS_LeaveCritical(sr);
    }

    return avail;
}
#endif  /* NOS_CONFIG_SEM_ENABLE */

#ifdef __cplusplus
}
#endif
