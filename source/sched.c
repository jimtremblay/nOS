/*
 * nOS v0.1
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define NOS_GLOBALS
#define NOS_PRIVATE
#include "nOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#if NOS_HIGHEST_PRIO < 8
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
    prio = tableDeBruijn[(uint8_t)(prio * 0x1D) >> 5];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif NOS_HIGHEST_PRIO < 16
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
#elif NOS_HIGHEST_PRIO < 64
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
    group = tableDeBruijn[(uint8_t)(group * 0x1D) >> 5];

    prio = nOS_readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio = tableDeBruijn[(uint8_t)(prio * 0x1D) >> 5];

    return (nOS_Thread*)nOS_readyList[(group << 3) | prio].head->payload;
}
#elif NOS_HIGHEST_PRIO < 256
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
#endif

void AppendThreadToReadyList (nOS_Thread *thread)
{
    #if NOS_HIGHEST_PRIO < 8
    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio |= (0x01 << thread->prio);
    #elif NOS_HIGHEST_PRIO < 16
    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio |= (0x0001 << thread->prio);
    #elif NOS_HIGHEST_PRIO < 64
    uint8_t     group = (thread->prio >> 3) & 0x07;

    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio[group] |= (0x01 << (thread->prio & 0x07));
    nOS_readyGroup |= (0x01 << group);
    #elif NOS_HIGHEST_PRIO < 256
    uint8_t     group = (thread->prio >> 4) & 0x0F;

    nOS_ListAppend(&nOS_readyList[thread->prio], &thread->readyWaiting);
    nOS_readyPrio[group] |= (0x0001 << (thread->prio & 0x0F));
    nOS_readyGroup |= (0x0001 << group);
    #endif
}

void RemoveThreadFromReadyList (nOS_Thread *thread)
{
    #if NOS_HIGHEST_PRIO < 8
    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio &= ~(0x01 << thread->prio);
    }
    #elif NOS_HIGHEST_PRIO < 16
    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio &= ~(0x0001 << thread->prio);
    }
    #elif NOS_HIGHEST_PRIO < 64
    uint8_t     group = (thread->prio >> 3) & 0x07;

    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio[group] &= ~(0x01 << (thread->prio & 0x07));
        if (nOS_readyPrio[group] == 0x00) {
            nOS_readyGroup &= ~(0x01 << group);
        }
    }
    #elif NOS_HIGHEST_PRIO < 256
    uint8_t     group = (thread->prio >> 4) & 0x0F;

    nOS_ListRemove(&nOS_readyList[thread->prio], &thread->readyWaiting);
    if (nOS_readyList[thread->prio].head == NULL) {
        nOS_readyPrio[group] &= ~(0x0001 << (thread->prio & 0x0F));
        if (nOS_readyPrio[group] == 0x0000) {
            nOS_readyGroup &= ~(0x0001 << group);
        }
    }
    #endif
}

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

int8_t nOS_EventWait (nOS_Event *event, uint8_t state, uint16_t tout)
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

int8_t nOS_Init(void)
{
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

    return NOS_OK;
}

int8_t nOS_Sched(void)
{
    int8_t  err;

    /* Recheck if current running thread is the highest prio thread */
    nOS_CriticalEnter();
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else {
        /* switch only from thread without scheduler locked */
        nOS_highPrioThread = SchedHighPrio();
        if (nOS_runningThread != nOS_highPrioThread) {
            nOS_ContextSwitch();
        }
        err = NOS_OK;
    }
    nOS_CriticalLeave();

    return err;
}

int8_t nOS_SchedLock(void)
{
    int8_t err;

    nOS_CriticalEnter();
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter < UINT8_MAX) {
        nOS_lockNestingCounter++;
        err = NOS_OK;
    } else {
        err = NOS_E_OVERFLOW;
    }
    nOS_CriticalLeave();

    return err;
}

int8_t nOS_SchedUnlock(void)
{
    int8_t err;

    nOS_CriticalEnter();
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        nOS_lockNestingCounter--;
        if (nOS_lockNestingCounter == 0) {
            nOS_Sched();
        }
        err = NOS_OK;
    } else {
        err = NOS_E_UNDERFLOW;
    }
    nOS_CriticalLeave();

    return err;
}

int8_t nOS_Yield(void)
{
    int8_t err;

    nOS_CriticalEnter();
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else {
        nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
        nOS_Sched();
        err = NOS_OK;
    }
    nOS_CriticalLeave();

    return err;
}

void nOS_Tick(void)
{
    nOS_CriticalEnter();
    nOS_ListWalk(&nOS_fullList, TickThread, NULL);
    nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
    nOS_CriticalLeave();
}

int8_t nOS_Sleep (uint16_t dly)
{
    int8_t          err;

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

#ifdef __cplusplus
}
#endif
