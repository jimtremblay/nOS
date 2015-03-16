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

/* Always called from critical section */
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

/* Always called from critical section */
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
        thread = nOS_SignalEvent(event, NOS_E_DELETED);
        if (thread != NULL) {
#ifdef NOS_PSEUDO_SCHEDULER
            if (nOS_runningThread == NULL) {
                sched = true;
            } else
#endif
            if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                sched = true;
            }
        }
    } while (thread != NULL);
#else
    while (nOS_SignalEvent(event, NOS_E_DELETED) != NULL);
#endif
#if (NOS_CONFIG_SAFE > 0)
    event->type = NOS_EVENT_INVALID;
#endif

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    return sched;
#endif
}

/* Always called from critical section */
nOS_Error nOS_WaitForEvent (nOS_Event *event, nOS_ThreadState state, nOS_TickCounter tout)
{
    nOS_RemoveThreadFromReadyList(nOS_runningThread);
    nOS_runningThread->state = (nOS_ThreadState)(nOS_runningThread->state | (state & NOS_THREAD_STATE_WAIT_MASK));
    nOS_runningThread->event = event;
    nOS_runningThread->timeout = (tout == NOS_WAIT_INFINITE) ? 0 : tout;
    if (event != NULL) {
        nOS_AppendToList(&event->waitList, &nOS_runningThread->readyWait);
    }

    nOS_Schedule();

    return nOS_runningThread->error;
}

/* Always called from critical section */
nOS_Thread* nOS_SignalEvent (nOS_Event *event, nOS_Error err)
{
    nOS_Thread  *thread;

    thread = (nOS_Thread*)nOS_GetHeadOfList(&event->waitList);
    if (thread != NULL) {
        nOS_SignalThread(thread, err);
    }

    return thread;
}

#ifdef __cplusplus
}
#endif
