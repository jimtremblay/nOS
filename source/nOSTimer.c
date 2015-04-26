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
 #ifdef NOS_SIMULATED_STACK
  static nOS_Stack  _timerStack;
 #else
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
    nOS_StatusReg       sr;
    nOS_Timer           *timer = (nOS_Timer *)payload;
    nOS_TimerCallback   callback = NULL;

    nOS_EnterCritical(sr);
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
            callback = timer->callback;
            arg      = timer->arg;
        }
    }
    nOS_LeaveCritical(sr);

    if (callback != NULL) {
        callback(timer, arg);
    }
}

void nOS_InitTimer(void)
{
    nOS_InitList(&_timerList);
    nOS_SemCreate(&_timerSem, 0, NOS_SEM_COUNT_MAX);
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    nOS_ThreadCreate(&_timerHandle,
                     _ThreadTimer,
                     NULL
 #ifdef NOS_SIMULATED_STACK
                     ,&_timerStack
 #else
                     ,_timerStack
 #endif
                     ,NOS_CONFIG_TIMER_THREAD_STACK_SIZE
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
#endif
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
                     ) == NOS_OK)
    {
        nOS_WalkInList(&_timerList, _TickTimer, NULL);
    }
}

nOS_Error nOS_TimerCreate (nOS_Timer *timer, nOS_TimerCallback callback, void *arg, nOS_TimerCounter reload, nOS_TimerMode mode)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state != NOS_TIMER_DELETED) {
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
            nOS_AppendToList(&_timerList, &timer->node);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_TIMER_DELETE_ENABLE > 0)
nOS_Error nOS_TimerDelete (nOS_Timer *timer)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->state = NOS_TIMER_DELETED;
            nOS_RemoveFromList(&_timerList, &timer->node);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

nOS_Error nOS_TimerStart (nOS_Timer *timer)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->count = timer->reload;
            timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerStop (nOS_Timer *timer)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_RUNNING);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerRestart (nOS_Timer *timer, nOS_TimerCounter reload)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->reload = reload;
            timer->count  = reload;
            timer->state  = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerPause (nOS_Timer *timer)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_PAUSED);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerResume (nOS_Timer *timer)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_PAUSED);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerSetReload (nOS_Timer *timer, nOS_TimerCounter reload)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->reload = reload;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerSetCallback (nOS_Timer *timer, nOS_TimerCallback callback, void *arg)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->callback = callback;
            timer->arg = arg;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerSetMode (nOS_Timer *timer, nOS_TimerMode mode)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    } else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            timer->state = (nOS_TimerState)(((nOS_TimerMode)timer->state &~ NOS_TIMER_MODE) | mode);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

bool nOS_TimerIsRunning (nOS_Timer *timer)
{
    nOS_StatusReg   sr;
    bool            running;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        running = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (timer->state == NOS_TIMER_DELETED) {
            running = false;
        } else
#endif
        {
            running = (timer->state & NOS_TIMER_RUNNING) == NOS_TIMER_RUNNING;
        }
        nOS_LeaveCritical(sr);
    }

    return running;
}
#endif  /* NOS_CONFIG_TIMER_ENABLE */

#ifdef __cplusplus
}
#endif
