/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#define NOS_GLOBALS
#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(NOS_PORT_SCHED_USE_32_BITS)
#if defined(NOS_PORT_HAVE_CLZ)
#if (NOS_CONFIG_MAX_THREAD_PRIO < 32)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    prio;

    prio = (31 - nOS_PortCLZ(nOS_readyPrio));

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    group;
    uint32_t    prio;

    group   = (31 - nOS_PortCLZ(nOS_readyGroup));
    prio    = (31 - nOS_PortCLZ(nOS_readyPrio[group]));

    return (nOS_Thread*)nOS_readyList[(group << 5) | prio].head->payload;
}
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
#else   /* NOS_PORT_HAVE_CLZ */
#if (NOS_CONFIG_MAX_THREAD_PRIO < 32)
static const uint8_t tableDeBruijn[32] =
{
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
};

nOS_Thread* SchedHighPrio(void)
{
    uint32_t    prio;

    prio = nOS_readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio |= prio >> 16;
    prio = (uint32_t)tableDeBruijn[(uint32_t)(prio * 0x07c4acddUL) >> 27];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
static const uint8_t tableDeBruijn[32] =
{
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
};

nOS_Thread* SchedHighPrio(void)
{
    uint32_t    group;
    uint32_t    prio;

    group = nOS_readyGroup;
    group |= group >> 1;
    group |= group >> 2;
    group |= group >> 4;
    group |= group >> 8;
    group |= group >> 16;
    group = (uint32_t)tableDeBruijn[(uint32_t)(group * 0x07c4acddUL) >> 27];

    prio = nOS_readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio |= prio >> 16;
    prio = (uint32_t)tableDeBruijn[(uint32_t)(prio * 0x07c4acddUL) >> 27];

    return (nOS_Thread*)nOS_readyList[(group << 5) | prio].head->payload;
}
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
#endif  /* NOS_PORT_HAVE_CLZ */
#else   /* NOS_PORT_SCHED_USE_32_BITS */
#if (NOS_CONFIG_MAX_THREAD_PRIO < 8)
static const uint8_t tableDeBruijn[8] =
{
    0, 5, 1, 6, 4, 3, 2, 7
};

nOS_Thread* SchedHighPrio(void)
{
    uint8_t prio;

    prio = nOS_readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio = tableDeBruijn[(uint8_t)(prio * 0x1d) >> 5];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 16)
const uint16_t tableDeBruijn[16] =
{
    0, 7, 1, 13, 8, 10, 2, 14, 6, 12, 9, 5, 11, 4, 3, 15
};

nOS_Thread* SchedHighPrio(void)
{
    uint16_t    prio;

    prio = nOS_readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 64)
const uint8_t tableDeBruijn[8] =
{
    0, 5, 1, 6, 4, 3, 2, 7
};

nOS_Thread* SchedHighPrio(void)
{
    uint8_t     group;
    uint8_t     prio;

    group = nOS_readyGroup;
    group |= group >> 1; // first round down to one less than a power of 2
    group |= group >> 2;
    group |= group >> 4;
    group = tableDeBruijn[(uint8_t)(group * 0x1d) >> 5];

    prio = nOS_readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio = tableDeBruijn[(uint8_t)(prio * 0x1d) >> 5];

    return (nOS_Thread*)nOS_readyList[(group << 3) | prio].head->payload;
}
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
const uint16_t tableDeBruijn[16] =
{
    0, 7, 1, 13, 8, 10, 2, 14, 6, 12, 9, 5, 11, 4, 3, 15
};

nOS_Thread* SchedHighPrio(void)
{
    uint16_t    group;
    uint16_t    prio;

    group = nOS_readyGroup;
    group |= group >> 1; // first round down to one less than a power of 2
    group |= group >> 2;
    group |= group >> 4;
    group |= group >> 8;
    group = tableDeBruijn[(uint16_t)(group * 0xf2d) >> 12];

    prio = nOS_readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[(group << 4) | prio].head->payload;
}
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
#endif  /* NOS_PORT_SCHED_USE_32_BITS */

#if defined(NOS_PORT_SCHED_USE_32_BITS)
void AppendThreadToReadyList (nOS_Thread *thread)
{
    /* we use 32 bits variables for maximum performance */
    uint32_t    prio = (uint32_t)thread->prio;

#if (NOS_CONFIG_MAX_THREAD_PRIO < 32)
    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWaiting);
    nOS_readyPrio |= (0x00000001UL << prio);
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
    uint32_t    group = (prio >> 5UL) & 0x00000007UL;

    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWaiting);
    nOS_readyPrio[group] |= (0x00000001UL << (prio & 0x0000001fUL));
    nOS_readyGroup |= (0x00000001UL << group);
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
}

void RemoveThreadFromReadyList (nOS_Thread *thread)
{
    /* we use 32 bits variables for maximum performance */
    uint32_t    prio = (uint32_t)thread->prio;

#if (NOS_CONFIG_MAX_THREAD_PRIO < 32)
    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWaiting);
    if (nOS_readyList[prio].head == NULL) {
        nOS_readyPrio &= ~(0x00000001UL << prio);
    }
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
    uint32_t    group = (prio >> 5UL) & 0x00000007UL;

    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWaiting);
    if (nOS_readyList[prio].head == NULL) {
        nOS_readyPrio[group] &= ~(0x00000001UL << (prio & 0x0000001fUL));
        if (nOS_readyPrio[group] == 0x00000000UL) {
            nOS_readyGroup &= ~(0x00000001UL << group);
        }
    }
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
}
#else   /* NOS_PORT_SCHED_USE_32_BITS */
void AppendThreadToReadyList (nOS_Thread *thread)
{
#if (NOS_CONFIG_MAX_THREAD_PRIO < 8)
    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio |= (0x01 << thread->prio);
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 16)
    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio |= (0x0001 << thread->prio);
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 64)
    uint8_t     group = (thread->prio >> 3) & 0x07;

    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio[group] |= (0x01 << (thread->prio & 0x07));
    nOS_readyGroup |= (0x01 << group);
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
    uint8_t     group = (thread->prio >> 4) & 0x0F;

    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio[group] |= (0x0001 << (thread->prio & 0x0f));
    nOS_readyGroup |= (0x0001 << group);
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
}

void RemoveThreadFromReadyList (nOS_Thread *thread)
{
#if (NOS_CONFIG_MAX_THREAD_PRIO < 8)
    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio &= ~(0x01 << thread->prio);
    }
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 16)
    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio &= ~(0x0001 << thread->prio);
    }
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 64)
    uint8_t     group = (thread->prio >> 3) & 0x07;

    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio[group] &= ~(0x01 << (thread->prio & 0x07));
        if (nOS_readyPrio[group] == 0x00) {
            nOS_readyGroup &= ~(0x01 << group);
        }
    }
#elif (NOS_CONFIG_MAX_THREAD_PRIO < 256)
    uint8_t     group = (thread->prio >> 4) & 0x0F;

    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio[group] &= ~(0x0001 << (thread->prio & 0x0f));
        if (nOS_readyPrio[group] == 0x0000) {
            nOS_readyGroup &= ~(0x0001 << group);
        }
    }
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
}
#endif  /* NOS_PORT_SCHED_USE_32_BITS */

void nOS_ListInit (nOS_List *list)
{
    list->head = NULL;
    list->tail = NULL;
}

void* nOS_ListHead (nOS_List *list)
{
    if (list->head != NULL) {
        return list->head->payload;
    } else {
        return NULL;
    }
}

void nOS_ListAppend (nOS_List *list, nOS_Node *node)
{
    node->prev = list->tail;
    node->next = NULL;
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    list->tail = node;
    if (list->head == NULL) {
        list->head = node;
    }
}

void nOS_ListRemove (nOS_List *list, nOS_Node *node)
{
    if (list->head == node) {
        list->head = node->next;
    }
    if (list->tail == node) {
        list->tail = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    node->prev = NULL;
    node->next = NULL;
}

void nOS_ListRotate (nOS_List *list)
{
    if (list->head->next != NULL) {
        list->head->prev = list->tail;
        list->tail->next = list->head;
        list->head = list->head->next;
        list->tail = list->tail->next;
        list->head->prev = NULL;
        list->tail->next = NULL;
    }
}

/* Return 0 to continue to walk into list
 * Return non-zero values to return from walk with last iterator payload
 */
void nOS_ListWalk (nOS_List *list, void(*callback)(void*,void*), void *arg)
{
    nOS_Node    *it = list->head;
    nOS_Node    *next;

    while (it != NULL) {
        next = it->next;
        callback(it->payload, arg);
        it = next;
    }
}

void nOS_EventCreate (nOS_Event *event)
{
    nOS_ListInit(&event->waitingList);
}

nOS_Error nOS_EventWait (nOS_Event *event, uint8_t state, uint16_t tout)
{
    RemoveThreadFromReadyList(nOS_runningThread);
    nOS_runningThread->state = state;
    nOS_runningThread->event = event;
    nOS_runningThread->timeout = (tout == NOS_WAIT_INFINITE) ? 0 : tout;
    if (event != NULL) {
        nOS_ListAppend(&event->waitingList, &nOS_runningThread->readyWaiting);
    }

    nOS_Sched();

    return nOS_runningThread->error;
}

nOS_Thread* nOS_EventSignal (nOS_Event *event)
{
    nOS_Thread  *thread;

    thread = (nOS_Thread*)nOS_ListHead(&event->waitingList);
    if (thread != NULL) {
        SignalThread(thread);
    }

    return thread;
}

nOS_Error nOS_Init(void)
{
    nOS_PortInit();

    nOS_mainThread.prio = NOS_PRIO_IDLE;
    nOS_mainThread.state = NOS_READY;
    nOS_mainThread.error = NOS_OK;
    nOS_mainThread.timeout = 0;
    nOS_mainThread.event = NULL;
    nOS_mainThread.context = NULL;
    nOS_mainThread.full.payload = &nOS_mainThread;
    nOS_mainThread.readyWaiting.payload = &nOS_mainThread;

    nOS_CriticalEnter();
    nOS_ListAppend(&nOS_fullList, &nOS_mainThread.full);
    AppendThreadToReadyList(&nOS_mainThread);
    nOS_runningThread = &nOS_mainThread;
    nOS_highPrioThread = &nOS_mainThread;
    nOS_CriticalLeave();

#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerInit();
#endif

    return NOS_OK;
}

nOS_Error nOS_Sched(void)
{
    nOS_Error   err;

    /* Recheck if current running thread is the highest prio thread */

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else {
        nOS_CriticalEnter();
        /* switch only from thread without scheduler locked */
        nOS_highPrioThread = SchedHighPrio();
        if (nOS_runningThread != nOS_highPrioThread) {
            nOS_ContextSwitch();
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
nOS_Error nOS_SchedLock(void)
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        if (nOS_lockNestingCounter < UINT8_MAX) {
            nOS_lockNestingCounter++;
            err = NOS_OK;
        } else {
            err = NOS_E_OVERFLOW;
        }
        nOS_CriticalLeave();
    }

    return err;
}

nOS_Error nOS_SchedUnlock(void)
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else {
        nOS_CriticalEnter();
        if (nOS_lockNestingCounter > 0) {
            nOS_lockNestingCounter--;
            if (nOS_lockNestingCounter == 0) {
                nOS_Sched();
            }
            err = NOS_OK;
        } else {
            err = NOS_E_UNDERFLOW;
        }
        nOS_CriticalLeave();
    }

    return err;
}
#endif  /* NOS_CONFIG_SCHED_LOCK_ENABLE */

nOS_Error nOS_Yield(void)
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else {
        nOS_CriticalEnter();
        nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
        nOS_CriticalLeave();
        nOS_Sched();
        err = NOS_OK;
    }

    return err;
}

void nOS_Tick(void)
{
    nOS_CriticalEnter();
    nOS_ListWalk(&nOS_fullList, TickThread, NULL);
    nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
    nOS_CriticalLeave();

#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerTick();
#endif
}

#if (NOS_CONFIG_SLEEP_ENABLE > 0)
nOS_Error nOS_Sleep (uint16_t dly)
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else if (dly == 0) {
        nOS_Yield();
        err = NOS_OK;
    } else if (nOS_runningThread == &nOS_mainThread) {
        err = NOS_E_IDLE;
    } else {
        nOS_CriticalEnter();
        err = nOS_EventWait(NULL, NOS_SLEEPING, dly);
        nOS_CriticalLeave();
    }

    return err;
}
#endif  /* NOS_CONFIG_SLEEP_ENABLE */

#if defined(__cplusplus)
}
#endif
