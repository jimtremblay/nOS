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

#if (NOS_CONFIG_TIMER_ENABLE > 0)
static void ThreadTimer (void *arg);
static void TickTimer (void *payload, void *arg);

static nOS_List     timerList;
static nOS_Sem      timerSem;
static nOS_Thread   timerThread;
static nOS_Stack    timerStack[NOS_CONFIG_TIMER_THREAD_STACK_SIZE];

static void ThreadTimer (void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        if (nOS_SemTake (&timerSem, NOS_WAIT_INFINITE) == NOS_OK) {
            nOS_ListWalk(&timerList, TickTimer, NULL);
        }
    }
}

static void TickTimer (void *payload, void *arg)
{
    nOS_Timer   *timer = (nOS_Timer *)payload;
    uint8_t     cb = 0;

    NOS_UNUSED(arg);

    nOS_CriticalEnter();
    if (timer->state & NOS_TIMER_RUNNING) {
        if (timer->count > 0) {
            timer->count--;
            if (timer->count == 0) {
                if (timer->state & NOS_TIMER_FREE_RUNNING) {
                    timer->count = timer->reload;
                /* One-shot timer */
                } else {
                    timer->state &=~ NOS_TIMER_RUNNING;
                }
                /* Call callback function outside of critical section */
                cb = 1;
            }
        }
    }
    nOS_CriticalLeave();

    if (cb) {
        if (timer->callback != NULL) {
            timer->callback(timer->arg);
        }
    }
}

void nOS_TimerInit(void)
{
    nOS_ListInit(&timerList);
    nOS_SemCreate(&timerSem, 0, NOS_SEM_COUNT_MAX);
    nOS_ThreadCreate(&timerThread,
                     ThreadTimer,
                     NULL,
                     timerStack,
                     NOS_CONFIG_TIMER_THREAD_STACK_SIZE,
                     NOS_CONFIG_TIMER_THREAD_PRIO
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                     ,NOS_READY
#endif
                     );

}

void nOS_TimerTick(void)
{
    nOS_SemGive(&timerSem);
}

nOS_Error nOS_TimerCreate (nOS_Timer *timer, void(*callback)(void*), void *arg, nOS_TimerCount reload, uint8_t opt)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if ((opt != NOS_TIMER_FREE_RUNNING) && (opt != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        timer->count = 0;
        timer->reload = reload;
        timer->state = opt;
        timer->callback = callback;
        timer->arg = arg;
        timer->node.payload = (void *)timer;
        nOS_CriticalEnter();
        nOS_ListAppend(&timerList, &timer->node);
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerStart (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->count  = timer->reload;
        timer->state |= NOS_TIMER_RUNNING;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerRestart (nOS_Timer *timer, nOS_TimerCount reload)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->reload = reload;
        timer->count  = reload;
        timer->state |= NOS_TIMER_RUNNING;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerStop (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->state &=~ NOS_TIMER_RUNNING;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetCallback (nOS_Timer *timer, void(*callback)(void*), void *arg)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->callback = callback;
        timer->arg = arg;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetReloadValue (nOS_Timer *timer, nOS_TimerCount reload)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->reload = reload;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetMode (nOS_Timer *timer, uint8_t opt)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if ((opt != NOS_TIMER_FREE_RUNNING) && (opt != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        timer->state = (timer->state &~ NOS_TIMER_OPT) | opt;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

uint8_t nOS_TimerIsRunning (nOS_Timer *timer)
{
    uint8_t running = 0;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        running = 0;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (timer->state & NOS_TIMER_RUNNING) {
            running = 1;
        }
        nOS_CriticalLeave();
    }

    return running;
}
#endif  /* NOS_CONFIG_TIMER_ENABLE */

#if defined(__cplusplus)
}
#endif
