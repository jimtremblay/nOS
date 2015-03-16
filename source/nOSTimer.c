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

#if (NOS_CONFIG_TIMER_ENABLE > 0)
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
 static void    _ThreadTimer    (void *arg);
#endif
static void     _TickTimer      (void *payload, void *arg);

static nOS_List     _timerList;
static nOS_Sem      _timerSem;
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
 static nOS_Thread  _timerHandle;
 #ifndef NOS_SIMULATED_STACK
  static nOS_Stack  _timerStack[NOS_CONFIG_TIMER_THREAD_STACK_SIZE];
 #endif
#endif

#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
static void _ThreadTimer (void *arg)
{
    NOS_UNUSED(arg);

    while (true) {
        nOS_TimerProcess();
    }
}
#endif

static void _TickTimer (void *payload, void *arg)
{
    nOS_Timer   *timer = (nOS_Timer *)payload;
    bool        call = false;

    NOS_UNUSED(arg);

    nOS_EnterCritical();
    if ((timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING) {
        if (timer->count > 0) {
            timer->count--;
        }

        if (timer->count == 0) {
            if (((nOS_TimerMode)timer->state & NOS_TIMER_MODE) == NOS_TIMER_FREE_RUNNING) {
                timer->count = timer->reload;
            /* One-shot timer */
            } else {
                timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_RUNNING);
            }
            /* Call callback function outside of critical section */
            call = true;
        }
    }
    nOS_LeaveCritical();

    if (call) {
        if (timer->callback != NULL) {
            timer->callback(timer, timer->arg);
        }
    }
}

void nOS_TimerInit(void)
{
    nOS_InitList(&_timerList);
    nOS_SemCreate(&_timerSem, 0, NOS_SEM_COUNT_MAX);
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    nOS_ThreadCreate(&_timerHandle,
                     _ThreadTimer,
                     NULL
#ifndef NOS_SIMULATED_STACK
                     ,_timerStack
#else
                     ,NOS_CONFIG_TIMER_THREAD_STACK_SIZE
#endif
#ifdef NOS_USE_SEPARATE_CALL_STACK
                     ,NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                     ,NOS_CONFIG_TIMER_THREAD_PRIO
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                     ,NOS_THREAD_READY
#endif
#if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                     ,"nOS_Timer"
#endif
                     );
#endif  /* NOS_CONFIG_TIMER_THREAD_ENABLE */
}

void nOS_TimerTick (void)
{
    nOS_SemGive(&_timerSem);
}

void nOS_TimerProcess (void)
{
    if (nOS_SemTake (&_timerSem,
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
                     NOS_WAIT_INFINITE
#else
                     NOS_NO_WAIT
#endif
                     ) == NOS_OK) {
        nOS_WalkInList(&_timerList, _TickTimer, NULL);
    }
}

nOS_Error nOS_TimerCreate (nOS_Timer *timer, nOS_TimerCallback callback, void *arg, nOS_TimerCounter reload, nOS_TimerMode mode)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else if (timer->state != NOS_TIMER_DELETED) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        timer->count = 0;
        timer->reload = reload;
        timer->state = (nOS_TimerState)(NOS_TIMER_CREATED | (nOS_TimerState)(mode & NOS_TIMER_MODE));
        timer->callback = callback;
        timer->arg = arg;
        timer->node.payload = (void *)timer;
        nOS_EnterCritical();
        nOS_AppendToList(&_timerList, &timer->node);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

#if (NOS_CONFIG_TIMER_DELETE_ENABLE > 0)
nOS_Error nOS_TimerDelete (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->state = NOS_TIMER_DELETED;
        nOS_RemoveFromList(&_timerList, &timer->node);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}
#endif

nOS_Error nOS_TimerStart (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->count = timer->reload;
        timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
        nOS_LeaveCritical();
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
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_RUNNING);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerRestart (nOS_Timer *timer, nOS_TimerCounter reload)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->reload = reload;
        timer->count  = reload;
        timer->state  = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerPause (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_PAUSED);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerResume (nOS_Timer *timer)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_PAUSED);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetReload (nOS_Timer *timer, nOS_TimerCounter reload)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->reload = reload;
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetCallback (nOS_Timer *timer, nOS_TimerCallback callback, void *arg)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->callback = callback;
        timer->arg = arg;
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_TimerSetMode (nOS_Timer *timer, nOS_TimerMode mode)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_NULL;
    } else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else if (timer->state == NOS_TIMER_DELETED) {
        err = NOS_E_DELETED;
    } else
#endif
    {
        nOS_EnterCritical();
        timer->state = (nOS_TimerState)(((nOS_TimerMode)timer->state &~ NOS_TIMER_MODE) | mode);
        nOS_LeaveCritical();
        err = NOS_OK;
    }

    return err;
}

bool nOS_TimerIsRunning (nOS_Timer *timer)
{
    bool running;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        running = false;
    } else if (timer->state == NOS_TIMER_DELETED) {
        running = false;
    } else
#endif
    {
        nOS_EnterCritical();
        running = (timer->state & NOS_TIMER_RUNNING) == NOS_TIMER_RUNNING;
        nOS_LeaveCritical();
    }

    return running;
}
#endif  /* NOS_CONFIG_TIMER_ENABLE */

#ifdef __cplusplus
}
#endif
