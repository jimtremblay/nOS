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

#if (NOS_CONFIG_SAFE > 0)
void nOS_CreateEvent (nOS_Event *event, nOS_EventType type)
#else
void nOS_CreateEvent (nOS_Event *event)
#endif
{
#if (NOS_CONFIG_SAFE > 0)
    event->type = type;
#endif
    nOS_InitList(&event->waitList);
}

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
bool nOS_DeleteEvent (nOS_Event *event)
#else
void nOS_DeleteEvent (nOS_Event *event)
#endif
{
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    nOS_Thread  *thread;
    bool        sched = false;

    do {
        thread = nOS_SendEvent(event, NOS_E_DELETED);
        if (thread != NULL) {
            if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                sched = true;
            }
        }
    } while (thread != NULL);
#else
    while (nOS_SendEvent(event, NOS_E_DELETED) != NULL);
#endif
#if (NOS_CONFIG_SAFE > 0)
    event->type = NOS_EVENT_INVALID;
#endif

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    return sched;
#endif
}

nOS_Error nOS_WaitForEvent (nOS_Event *event,
                            nOS_ThreadState state
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                           ,nOS_TickCounter timeout
#endif
                           )
{
    nOS_RemoveThreadFromReadyList(nOS_runningThread);
    nOS_runningThread->state = (nOS_ThreadState)(nOS_runningThread->state | (state & NOS_THREAD_WAITING_MASK));
    nOS_runningThread->event = event;
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
    if ((timeout > 0) && (timeout < NOS_WAIT_INFINITE)) {
        nOS_runningThread->timeout = nOS_tickCounter + timeout;
        nOS_runningThread->state = (nOS_ThreadState)(nOS_runningThread->state | NOS_THREAD_WAIT_TIMEOUT);
        nOS_AppendToList(&nOS_timeoutThreadsList, &nOS_runningThread->tout);
    }
#endif
    if (event != NULL) {
        nOS_AppendToList(&event->waitList, &nOS_runningThread->readyWait);
    }

    nOS_Schedule();

    return nOS_runningThread->error;
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
