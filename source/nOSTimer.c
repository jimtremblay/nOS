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
 static void    _Thread     (void *arg);
#endif
static void     _Tick       (void *payload, void *arg);

static nOS_List     _activeList;
static nOS_List     _triggeredList;
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
 static nOS_Thread  _thread;
 #ifdef NOS_SIMULATED_STACK
  static nOS_Stack  _stack;
 #else
  static nOS_Stack  _stack[NOS_CONFIG_TIMER_THREAD_STACK_SIZE];
 #endif
#endif

#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
static void _Thread (void *arg)
{
    nOS_StatusReg   sr;

    NOS_UNUSED(arg);

    while (true) {
        nOS_TimerProcess();

        nOS_EnterCritical(sr);
        if (nOS_GetHeadOfList(&_triggeredList) == NULL) {
            nOS_WaitForEvent(NULL, NOS_THREAD_ON_HOLD, 0);
        }
        nOS_LeaveCritical(sr);
    }
}
#endif

/* Called from critical section */
static void _Tick (void *payload, void *arg)
{
    nOS_Timer   *timer      = (nOS_Timer *)payload;
    bool        *overflow   = (bool*)arg;

    if (timer->count > 0) {
        timer->count--;
    }

    if (timer->count == 0) {
        if (((nOS_TimerMode)timer->state & NOS_TIMER_MODE) == NOS_TIMER_FREE_RUNNING) {
            /* Free running timer */
            timer->count = timer->reload;
        } else {
            /* One-shot timer */
            timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_RUNNING);
            nOS_RemoveFromList(&_activeList, &timer->node);
        }
        if (timer->overflow == 0) {
            nOS_AppendToList(&_triggeredList, &timer->trig);
        }
        timer->overflow++;
        *overflow = true;
    }
}

void nOS_InitTimer(void)
{
    nOS_InitList(&_activeList);
    nOS_InitList(&_triggeredList);
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    nOS_ThreadCreate(&_thread,
                     _Thread,
                     NULL
 #ifdef NOS_SIMULATED_STACK
                     ,&_stack
 #else
                     ,_stack
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
    nOS_StatusReg   sr;
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    bool            overflow = false;
#endif

    nOS_EnterCritical(sr);
    nOS_WalkInList(&_activeList, _Tick, &overflow);
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    if (overflow && (_thread.state == (NOS_THREAD_READY | NOS_THREAD_ON_HOLD))) {
        nOS_WakeUpThread(&_thread, NOS_OK);
    }
#endif
    nOS_LeaveCritical(sr);
}

void nOS_TimerProcess (void)
{
    nOS_StatusReg       sr;
    nOS_Timer           *timer;
    nOS_TimerCallback   callback = NULL;
    void                *arg;

    nOS_EnterCritical(sr);
    timer = nOS_GetHeadOfList(&_triggeredList);
    if (timer != NULL) {
        timer->overflow--;
        if (timer->overflow == 0) {
            nOS_RemoveFromList(&_triggeredList, &timer->trig);
        } else {
            nOS_RotateList(&_triggeredList);
        }

        /* Call callback function outside of critical section */
        callback = timer->callback;
        arg      = timer->arg;
    }
    nOS_LeaveCritical(sr);

    if (callback != NULL) {
        callback(timer, arg);
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
            timer->count        = 0;
            timer->reload       = reload;
            timer->state        = (nOS_TimerState)(NOS_TIMER_CREATED | (nOS_TimerState)(mode & NOS_TIMER_MODE));
            timer->overflow     = 0;
            timer->callback     = callback;
            timer->arg          = arg;
            timer->node.payload = (void *)timer;
            timer->trig.payload = (void *)timer;

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
            if ((timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING) {
                nOS_RemoveFromList(&_activeList, &timer->node);
            }
            timer->state = NOS_TIMER_DELETED;
            if (timer->overflow > 0) {
                timer->overflow = 0;
                nOS_RemoveFromList(&_triggeredList, &timer->trig);
            }

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
            if ( !(timer->state & NOS_TIMER_RUNNING) ) {
                timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
                nOS_AppendToList(&_activeList, &timer->node);
            }
            timer->count = timer->reload;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerStop (nOS_Timer *timer, bool instant)
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
            if ((timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING) {
                nOS_RemoveFromList(&_activeList, &timer->node);
            }
            timer->state = (nOS_TimerState)(timer->state &~ (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED));
            if ((timer->overflow > 0) && instant) {
                timer->overflow = 0;
                nOS_RemoveFromList(&_triggeredList, &timer->trig);
            }

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
            if ( !(timer->state & NOS_TIMER_RUNNING) ) {
                timer->state  = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
                nOS_AppendToList(&_activeList, &timer->node);
            }
            timer->reload = reload;
            timer->count  = reload;

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
        if ((timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING) {
            timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_PAUSED);
            nOS_RemoveFromList(&_activeList, &timer->node);
        }
        err = NOS_OK;
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_TimerContinue (nOS_Timer *timer)
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
        if (timer->state & NOS_TIMER_PAUSED) {
            timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_PAUSED);
            nOS_AppendToList(&_activeList, &timer->node);
        }
        err = NOS_OK;
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
            running = (timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING;
        }
        nOS_LeaveCritical(sr);
    }

    return running;
}
#endif  /* NOS_CONFIG_TIMER_ENABLE */

#ifdef __cplusplus
}
#endif
