/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

static bool                 running;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
#if defined(NOS_PORT_SCHED_USE_32_BITS)
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 32)
static uint32_t             readyPrio;
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
static uint32_t             readyGroup;
static uint32_t             readyPrio[((NOS_CONFIG_HIGHEST_THREAD_PRIO+31)/32)];
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#elif defined(NOS_PORT_SCHED_USE_16_BITS)
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
static uint16_t             readyPrio;
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
static uint16_t             readyGroup;
static uint16_t             readyPrio[((NOS_CONFIG_HIGHEST_THREAD_PRIO+15)/16)];
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#else   /* NOS_PORT_SCHED_USE_8_BITS */
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 8)
static uint8_t              readyPrio;
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
static uint16_t             readyPrio;
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 64)
static uint8_t              readyGroup;
static uint8_t              readyPrio[((NOS_CONFIG_HIGHEST_THREAD_PRIO+7)/8)];
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
static uint16_t             readyGroup;
static uint16_t             readyPrio[((NOS_CONFIG_HIGHEST_THREAD_PRIO+15)/16)];
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#endif  /* NOS_PORT_SCHED_USE_32_BITS */
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO > 0 */

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
static nOS_Event            sleepEvent;
#endif

static nOS_TickCounter      tickCounter;

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
static void TickSleeper(void *payload, void *arg)
{
    nOS_Thread          *thread = (nOS_Thread*)payload;
    nOS_SleepContext    *ctx    = (nOS_SleepContext*)thread->context;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (tickCounter == ctx->tick) {
        SignalThread(thread, NOS_OK);
    }
}

static void SleepTick(void)
{
    nOS_CriticalEnter();
    nOS_ListWalk(&sleepEvent.waitList, TickSleeper, NULL);
    nOS_CriticalLeave();
}
#endif

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
#if defined(NOS_PORT_SCHED_USE_32_BITS)
#if defined(NOS_PORT_HAVE_CLZ)
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 32)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    prio;

    prio = (31 - nOS_PortCLZ(readyPrio));

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    group;
    uint32_t    prio;

    group   = (31 - nOS_PortCLZ(readyGroup));
    prio    = (31 - nOS_PortCLZ(readyPrio[group]));

    return (nOS_Thread*)nOS_readyList[(group << 5) | prio].head->payload;
}
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#else   /* !defined(NOS_PORT_HAVE_CLZ) */
static const uint8_t tableDeBruijn[32] =
{
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
};
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 32)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    prio;

    prio = readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio |= prio >> 16;
    prio = (uint32_t)tableDeBruijn[(uint32_t)(prio * 0x07c4acddUL) >> 27];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
nOS_Thread* SchedHighPrio(void)
{
    uint32_t    group;
    uint32_t    prio;

    group = readyGroup;
    group |= group >> 1;
    group |= group >> 2;
    group |= group >> 4;
    group |= group >> 8;
    group |= group >> 16;
    group = (uint32_t)tableDeBruijn[(uint32_t)(group * 0x07c4acddUL) >> 27];

    prio = readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio |= prio >> 16;
    prio = (uint32_t)tableDeBruijn[(uint32_t)(prio * 0x07c4acddUL) >> 27];

    return (nOS_Thread*)nOS_readyList[(group << 5) | prio].head->payload;
}
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#endif  /* NOS_PORT_HAVE_CLZ */
#elif defined(NOS_PORT_SCHED_USE_16_BITS)
static const uint16_t tableDeBruijn[16] =
{
    0, 7, 1, 13, 8, 10, 2, 14, 6, 12, 9, 5, 11, 4, 3, 15
};
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
nOS_Thread* SchedHighPrio(void)
{
    uint16_t    prio;

    prio = readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
nOS_Thread* SchedHighPrio(void)
{
    uint16_t    group;
    uint16_t    prio;

    group = readyGroup;
    group |= group >> 1; // first round down to one less than a power of 2
    group |= group >> 2;
    group |= group >> 4;
    group |= group >> 8;
    group = tableDeBruijn[(uint16_t)(group * 0xf2d) >> 12];

    prio = readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[(group << 4) | prio].head->payload;
}
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#else   /* NOS_PORT_SCHED_USE_8_BITS */
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 8)
static const uint8_t tableDeBruijn[8] =
{
    0, 5, 1, 6, 4, 3, 2, 7
};

nOS_Thread* SchedHighPrio(void)
{
    uint8_t prio;

    prio = readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio = tableDeBruijn[(uint8_t)(prio * 0x1d) >> 5];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
static const uint16_t tableDeBruijn[16] =
{
    0, 7, 1, 13, 8, 10, 2, 14, 6, 12, 9, 5, 11, 4, 3, 15
};

nOS_Thread* SchedHighPrio(void)
{
    uint16_t    prio;

    prio = readyPrio;
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 64)
static const uint8_t tableDeBruijn[8] =
{
    0, 5, 1, 6, 4, 3, 2, 7
};

nOS_Thread* SchedHighPrio(void)
{
    uint8_t     group;
    uint8_t     prio;

    group = readyGroup;
    group |= group >> 1; // first round down to one less than a power of 2
    group |= group >> 2;
    group |= group >> 4;
    group = tableDeBruijn[(uint8_t)(group * 0x1d) >> 5];

    prio = readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio = tableDeBruijn[(uint8_t)(prio * 0x1d) >> 5];

    return (nOS_Thread*)nOS_readyList[(group << 3) | prio].head->payload;
}
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
static const uint16_t tableDeBruijn[16] =
{
    0, 7, 1, 13, 8, 10, 2, 14, 6, 12, 9, 5, 11, 4, 3, 15
};

nOS_Thread* SchedHighPrio(void)
{
    uint16_t    group;
    uint16_t    prio;

    group = readyGroup;
    group |= group >> 1; // first round down to one less than a power of 2
    group |= group >> 2;
    group |= group >> 4;
    group |= group >> 8;
    group = tableDeBruijn[(uint16_t)(group * 0xf2d) >> 12];

    prio = readyPrio[group];
    prio |= prio >> 1; // first round down to one less than a power of 2
    prio |= prio >> 2;
    prio |= prio >> 4;
    prio |= prio >> 8;
    prio = tableDeBruijn[(uint16_t)(prio * 0xf2d) >> 12];

    return (nOS_Thread*)nOS_readyList[(group << 4) | prio].head->payload;
}
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
#endif  /* NOS_PORT_SCHED_USE_32_BITS */

#if defined(NOS_PORT_SCHED_USE_32_BITS)
void AppendThreadToReadyList (nOS_Thread *thread)
{
    /* we use 32 bits variables for maximum performance */
    uint32_t    prio = (uint32_t)thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 32)
    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio |= (0x00000001UL << prio);
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint32_t    group = (prio >> 5UL) & 0x00000007UL;

    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio[group] |= (0x00000001UL << (prio & 0x0000001fUL));
    readyGroup |= (0x00000001UL << group);
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
void RemoveThreadFromReadyList (nOS_Thread *thread)
{
    /* we use 32 bits variables for maximum performance */
    uint32_t    prio = (uint32_t)thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 32)
    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio &=~ (0x00000001UL << prio);
    }
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint32_t    group = (prio >> 5UL) & 0x00000007UL;

    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio[group] &=~ (0x00000001UL << (prio & 0x0000001fUL));
        if (readyPrio[group] == 0x00000000UL) {
            readyGroup &=~ (0x00000001UL << group);
        }
    }
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
#elif defined(NOS_PORT_SCHED_USE_16_BITS)
void AppendThreadToReadyList (nOS_Thread *thread)
{
    /* we use 16 bits variables for maximum performance */
    uint16_t    prio = (uint16_t)thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio |= (0x0001 << prio);
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint16_t    group = (prio >> 4) & 0x000F;

    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio[group] |= (0x0001 << (prio & 0x0f));
    readyGroup |= (0x0001 << group);
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
void RemoveThreadFromReadyList (nOS_Thread *thread)
{
    /* we use 16 bits variables for maximum performance */
    uint16_t    prio = (uint16_t)thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio &=~ (0x0001 << prio);
    }
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint16_t    group = (prio >> 4) & 0x000F;

    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio[group] &=~ (0x0001 << (prio & 0x0f));
        if (readyPrio[group] == 0x0000) {
            readyGroup &=~ (0x0001 << group);
        }
    }
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
#else   /* NOS_PORT_SCHED_USE_32_BITS */
void AppendThreadToReadyList (nOS_Thread *thread)
{
    uint8_t prio = thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 8)
    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio |= (0x01 << prio);
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio |= (0x0001 << prio);
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 64)
    uint8_t     group = (prio >> 3) & 0x07;

    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio[group] |= (0x01 << (prio & 0x07));
    readyGroup |= (0x01 << group);
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint8_t     group = (prio >> 4) & 0x0F;

    nOS_ListAppend(&nOS_readyList[prio], &thread->readyWait);
    readyPrio[group] |= (0x0001 << (prio & 0x0f));
    readyGroup |= (0x0001 << group);
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
void RemoveThreadFromReadyList (nOS_Thread *thread)
{
    uint8_t prio = thread->prio;

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO < 8)
    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio &=~ (0x01 << prio);
    }
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 16)
    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio &=~ (0x0001 << prio);
    }
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 64)
    uint8_t     group = (prio >> 3) & 0x07;

    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio[group] &=~ (0x01 << (prio & 0x07));
        if (readyPrio[group] == 0x00) {
            readyGroup &=~ (0x01 << group);
        }
    }
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 256)
    uint8_t     group = (prio >> 4) & 0x0F;

    nOS_ListRemove(&nOS_readyList[prio], &thread->readyWait);
    if (nOS_readyList[prio].head == NULL) {
        readyPrio[group] &=~ (0x0001 << (prio & 0x0f));
        if (readyPrio[group] == 0x0000) {
            readyGroup &=~ (0x0001 << group);
        }
    }
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO */
}
#endif  /* NOS_PORT_SCHED_USE_32_BITS */
#endif  /* NOS_CONFIG_HIGHEST_THREAD_PRIO > 0 */

nOS_Error nOS_Init(void)
{
    /* Block context switching until initialization is completed */
    running = false;

    nOS_isrNestingCounter = 0;
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    nOS_lockNestingCounter = 0;
#endif

#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    nOS_mainHandle.prio = NOS_THREAD_PRIO_IDLE;
#endif
    nOS_mainHandle.state = NOS_THREAD_READY;
    nOS_mainHandle.error = NOS_OK;
    nOS_mainHandle.timeout = 0;
    nOS_mainHandle.event = NULL;
    nOS_mainHandle.context = NULL;
    nOS_mainHandle.full.payload = &nOS_mainHandle;
    nOS_mainHandle.readyWait.payload = &nOS_mainHandle;

    nOS_ListAppend(&nOS_fullList, &nOS_mainHandle.full);
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    AppendThreadToReadyList(&nOS_mainHandle);
#else
    nOS_ListAppend(&nOS_readyList, &nOS_mainHandle.readyWait);
#endif
    nOS_runningThread = &nOS_mainHandle;
    nOS_highPrioThread = &nOS_mainHandle;

    tickCounter = 0;

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
#if (NOS_CONFIG_SAFE > 0)
    nOS_EventCreate(&sleepEvent, NOS_EVENT_BASE);
#else
    nOS_EventCreate(&sleepEvent);
#endif
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerInit();
#endif

#if (NOS_CONFIG_TIME_ENABLE > 0)
    nOS_TimeInit();
#endif

    nOS_PortInit();

    /* Context switching is possible after this point */
    running = true;

    return NOS_OK;
}

nOS_Error nOS_Sched(void)
{
    nOS_Error   err;

    /* Switch only if initialization is completed */
    if (!running) {
        err = NOS_E_INIT;
    } else if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* switch only from thread without scheduler locked */
    else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    }
#endif
    else {
        nOS_CriticalEnter();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        /* Recheck if current running thread is the highest prio thread */
        nOS_highPrioThread = SchedHighPrio();
#else
        nOS_highPrioThread = nOS_ListHead(&nOS_readyList);
#endif
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
    }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    }
#endif
    else {
        nOS_CriticalEnter();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
        nOS_ListRotate(&nOS_readyList[nOS_runningThread->prio]);
#else
        nOS_ListRotate(&nOS_readyList);
#endif
        nOS_CriticalLeave();
        nOS_Sched();
        err = NOS_OK;
    }

    return err;
}

void nOS_Tick(void)
{
    nOS_CriticalEnter();
    tickCounter++;
    nOS_CriticalLeave();

    nOS_ThreadTick();

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
    SleepTick();
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerTick();
#endif

#if (NOS_CONFIG_TIME_ENABLE > 0)
    nOS_TimeTick();
#endif
}

nOS_TickCounter nOS_TickCount(void)
{
    nOS_TickCounter   tickcnt;

    nOS_CriticalEnter();
    tickcnt = tickCounter;
    nOS_CriticalLeave();

    return tickcnt;
}

#if (NOS_CONFIG_SLEEP_ENABLE > 0)
nOS_Error nOS_Sleep (nOS_TickCounter ticks)
{
    nOS_Error   err;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* Can't switch context when scheduler is locked */
    else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    }
#endif
    else if (nOS_runningThread == &nOS_mainHandle) {
        err = NOS_E_IDLE;
    } else if (ticks == 0) {
        nOS_Yield();
        err = NOS_OK;
    } else {
        nOS_CriticalEnter();
        err = nOS_EventWait(NULL, NOS_THREAD_SLEEPING, ticks);
        nOS_CriticalLeave();
    }

    return err;
}
#endif  /* NOS_CONFIG_SLEEP_ENABLE */

#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
nOS_Error nOS_SleepUntil (nOS_TickCounter tick)
{
    nOS_Error           err;
    nOS_SleepContext    ctx;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* Can't switch context when scheduler is locked */
    else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    }
#endif
    else if (nOS_runningThread == &nOS_mainHandle) {
        err = NOS_E_IDLE;
    } else {
        nOS_CriticalEnter();
        if (tick == tickCounter) {
            nOS_Yield();
            err = NOS_OK;
        } else {
            ctx.tick = tick;
            nOS_runningThread->context = &ctx;
            err = nOS_EventWait(&sleepEvent, NOS_THREAD_SLEEPING, NOS_WAIT_INFINITE);
        }
        nOS_CriticalLeave();
    }

    return err;
}
#endif  /* NOS_CONFIG_SLEEP_UNTIL_ENABLE */

#if defined(__cplusplus)
}
#endif
