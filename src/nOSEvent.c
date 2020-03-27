/*
 * Copyright (c) 2014-2019 Jim Tremblay
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

void nOS_CreateEvent (nOS_Event *event
#if (NOS_CONFIG_SAFE > 0)

                     ,nOS_EventType type
#endif
                     )
{
#if (NOS_CONFIG_SAFE > 0)
    event->type = type;
#endif
    nOS_InitList(&event->waitList);
}

void nOS_DeleteEvent (nOS_Event *event)
{
#if (NOS_CONFIG_SAFE > 0)
    event->type = NOS_EVENT_INVALID;
#endif
    nOS_BroadcastEvent(event, NOS_E_DELETED);
}

void nOS_BroadcastEvent (nOS_Event *event, nOS_Error err)
{
    while (nOS_SendEvent(event, err) != NULL);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    /* Verify if a highest prio thread is ready to run */
    nOS_Schedule();
#endif
}

nOS_Error nOS_WaitForEvent (nOS_Event *event,
                            nOS_ThreadState state
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                           ,nOS_TickCounter timeout
#endif
                           )
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        /* Can't wait from ISR */
        err = NOS_E_ISR;
    } else
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    if (nOS_lockNestingCounter > 0) {
        /* Can't switch context when scheduler is locked */
        err = NOS_E_LOCKED;
    } else
#endif
    {
        nOS_RemoveThreadFromReadyList((nOS_Thread*)nOS_runningThread);

        nOS_runningThread->state = (nOS_ThreadState)(nOS_runningThread->state | (state & NOS_THREAD_WAITING_MASK));
        nOS_runningThread->event = event;
        if (event != NULL) {
            nOS_AppendToList(&event->waitList, (nOS_Node*)&nOS_runningThread->readyWait);
        }

#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
        if ((timeout > 0) && (timeout < NOS_WAIT_INFINITE)) {
            nOS_runningThread->timeout = nOS_tickCounter + timeout;
            nOS_runningThread->state = (nOS_ThreadState)(nOS_runningThread->state | NOS_THREAD_WAIT_TIMEOUT);
            nOS_AppendToList((nOS_List*)&nOS_timeoutThreadsList, (nOS_Node*)&nOS_runningThread->tout);
        }
#endif

        nOS_Schedule();

        err = (nOS_Error)nOS_runningThread->error;
    }

    return err;
}

nOS_Error nOS_WaitOnHold (
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                          nOS_TickCounter timeout
#endif
                         )
{
    nOS_StatusReg   sr;
    nOS_Error       err;

    nOS_EnterCritical(sr);
    err = nOS_WaitForEvent(NULL,
                           NOS_THREAD_ON_HOLD
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                          ,timeout
#endif
                          );
    nOS_LeaveCritical(sr);

    return err;
}

nOS_Thread* nOS_SendEvent (nOS_Event *event, nOS_Error err)
{
    nOS_Thread  *thread;

    thread = (nOS_Thread*)nOS_GetHeadOfList(&event->waitList);
    if (thread != NULL) {
        nOS_WakeUpThread(thread, err);
    }

    return thread;
}

#ifdef __cplusplus
}
#endif
