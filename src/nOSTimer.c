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

#if (NOS_CONFIG_TIMER_ENABLE > 0)
typedef struct _TickContext
{
    nOS_TickCounter ticks;
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
    bool            triggered;
#endif
} _TickContext;

#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
 #if (NOS_CONFIG_THREAD_JOIN_ENABLE > 0)
  static int _Thread (void *arg);
 #else
  static void _Thread (void *arg);
 #endif
#endif
static  void    _Tick       (void *payload, void *arg);

#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
 static nOS_List                    _activeList[NOS_CONFIG_TIMER_HIGHEST_PRIO+1];
 static nOS_List                    _triggeredList[NOS_CONFIG_TIMER_HIGHEST_PRIO+1];
#else
 static nOS_List                    _activeList;
 static nOS_List                    _triggeredList;
#endif
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
 #if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
  static nOS_Thread                 _thread[NOS_CONFIG_TIMER_HIGHEST_PRIO+1];
  #define _GetTimerThread(p)        (&_thread[p])
  static NOS_CONST uint8_t          _prio[] = {NOS_CONFIG_TIMER_THREAD_PRIO};
  NOS_STATIC_ASSERT(NOS_ROW_COUNT(_prio)==(NOS_CONFIG_TIMER_HIGHEST_PRIO+1),Priority_count_mismatch_in_list_NOS_CONFIG_TIMER_THREAD_PRIO);
 #else
  static nOS_Thread                 _thread;
  #define _GetTimerThread(p)        (&_thread)
 #endif
 #ifdef NOS_SIMULATED_STACK
  #if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
   static nOS_Stack                 _stack[NOS_CONFIG_TIMER_HIGHEST_PRIO+1];
  #else
   static nOS_Stack                 _stack;
  #endif
 #else
  #if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
   static nOS_Stack                 _stack[NOS_CONFIG_TIMER_HIGHEST_PRIO+1][NOS_CONFIG_TIMER_THREAD_STACK_SIZE]
   #ifdef NOS_CONFIG_TIMER_THREAD_STACK_SECTION
        __attribute__ ( ( section(NOS_CONFIG_TIMER_THREAD_STACK_SECTION) ) )
   #endif
   ;
  #else
   static nOS_Stack                 _stack[NOS_CONFIG_TIMER_THREAD_STACK_SIZE]
   #ifdef NOS_CONFIG_TIMER_THREAD_STACK_SECTION
        __attribute__ ( ( section(NOS_CONFIG_TIMER_THREAD_STACK_SECTION) ) )
   #endif
   ;
  #endif
 #endif
#elif defined(NOS_CONFIG_TIMER_USER_THREAD_HANDLE)
 #if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
  static NOS_CONST nOS_Thread*      _handle[] = {NOS_CONFIG_TIMER_USER_THREAD_HANDLE};
  #define _GetTimerThread(p)        (_handle[p])
  NOS_STATIC_ASSERT(NOS_ROW_COUNT(_handle)==(NOS_CONFIG_TIMER_HIGHEST_PRIO+1),Handle_count_mismatch_in_list_NOS_CONFIG_TIMER_USER_THREAD_HANDLE);
 #else
  #define _GetTimerThread(p)        (NOS_CONFIG_TIMER_USER_THREAD_HANDLE)
 #endif
#endif
static nOS_TimerCounter             _tickCounter;

#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
 #define _GetActiveList(p)              (&_activeList[(p)])
 #define _AppendToActiveList(t)         nOS_AppendToList(&_activeList[(t)->prio], &(t)->node)
 #define _RemoveFromActiveList(t)       nOS_RemoveFromList(&_activeList[(t)->prio], &(t)->node)
 #define _AppendToTriggeredList(t)      nOS_AppendToList(&_triggeredList[(t)->prio], &(t)->trig)
 #define _RemoveFromTriggeredList(t)    nOS_RemoveFromList(&_triggeredList[(t)->prio], &(t)->trig)
 #define _RotateTriggeredList(t)        nOS_RotateList(&_triggeredList[(t)->prio])
#else
 #define _GetActiveList(p)              (&_activeList)
 #define _AppendToActiveList(t)         nOS_AppendToList(&_activeList, &(t)->node)
 #define _RemoveFromActiveList(t)       nOS_RemoveFromList(&_activeList, &(t)->node)
 #define _AppendToTriggeredList(t)      nOS_AppendToList(&_triggeredList, &(t)->trig)
 #define _RemoveFromTriggeredList(t)    nOS_RemoveFromList(&_triggeredList, &(t)->trig)
 #define _RotateTriggeredList(t)        nOS_RotateList(&_triggeredList)
#endif

#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
#if (NOS_CONFIG_THREAD_JOIN_ENABLE > 0)
static int _Thread (void *arg)
#else
static void _Thread (void *arg)
#endif
{
    nOS_StatusReg   sr;
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
    unsigned int    prio = (unsigned int)arg;
#else
    NOS_UNUSED(arg);
#endif

    while (true) {
        nOS_TimerProcess(
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
            prio
#endif
        );
        nOS_EnterCritical(sr);
        if (!nOS_TimerAnyTriggered(
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
            prio
#endif
        )) {
            nOS_WaitOnHold(
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0) || (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                NOS_WAIT_INFINITE
#endif
            );
        }
        nOS_LeaveCritical(sr);
#if (NOS_CONFIG_THREAD_JOIN_ENABLE > 0)
        if (false) break; /* Remove "statement is unreachable" warning */
    }
    return 0;
#else
    }
#endif
}
#endif

/* Called from critical section */
static void _Tick (void *payload, void *arg)
{
    nOS_Timer           *timer  = (nOS_Timer *)payload;
    _TickContext        *ctx    = (_TickContext *)arg;
    nOS_TimerCounter    overflow;

    if ((timer->count - _tickCounter) <= ctx->ticks) {
        overflow = 1;
        if (((nOS_TimerMode)timer->state & NOS_TIMER_MODE) == NOS_TIMER_FREE_RUNNING) {
            /* Free running timer */
            overflow += ((ctx->ticks - (timer->count - _tickCounter)) / timer->reload);
            timer->count += (overflow * timer->reload);
        }
        else {
            /* One-shot timer */
            timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_RUNNING);
            _RemoveFromActiveList(timer);
        }
        if (timer->overflow == 0) {
            _AppendToTriggeredList(timer);
        }
        if ((NOS_TIMER_COUNT_MAX - timer->overflow) < overflow) {
            timer->overflow = NOS_TIMER_COUNT_MAX;
        } else {
            timer->overflow += overflow;
        }
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
        ctx->triggered = true;
#endif
    }
}

void nOS_InitTimer(void)
{
    _tickCounter = 0;
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
    {
        unsigned int i;
        for (i = 0; i <= NOS_CONFIG_TIMER_HIGHEST_PRIO; i++) {
            nOS_InitList(&_activeList[i]);
            nOS_InitList(&_triggeredList[i]);
 #if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
            if (_prio[i] <= NOS_CONFIG_HIGHEST_THREAD_PRIO) {
                nOS_ThreadCreate(&_thread[i],
                                 _Thread,
                                 (void*)i
  #ifdef NOS_SIMULATED_STACK
                                ,&_stack[i]
  #else
                                ,_stack[i]
  #endif
                                ,NOS_CONFIG_TIMER_THREAD_STACK_SIZE
  #ifdef NOS_USE_SEPARATE_CALL_STACK
                                ,NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
  #endif
                                ,_prio[i]
  #if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                                ,NOS_THREAD_READY
  #endif
  #if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                                ,"nOS_Timer #" NOS_STR(i+1)
  #endif
                );
            }
 #endif
        }
    }
#else
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
#endif
}

void nOS_TimerTick (nOS_TickCounter ticks)
{
    nOS_StatusReg   sr;
    _TickContext    ctx;

    ctx.ticks = ticks;

    nOS_EnterCritical(sr);
    {
        unsigned int i;
        for (i = 0; i <= NOS_CONFIG_TIMER_HIGHEST_PRIO; i++) {
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
            ctx.triggered = false;
#endif
            nOS_WalkInList(_GetActiveList(i), _Tick, &ctx);
#if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0) || defined(NOS_CONFIG_TIMER_USER_THREAD_HANDLE)
            if (ctx.triggered && (_GetTimerThread(i)->state == (NOS_THREAD_READY | NOS_THREAD_ON_HOLD))) {
                nOS_WakeUpThread(_GetTimerThread(i), NOS_OK);
            }
#endif
        }
    }
    _tickCounter += ticks;
    nOS_LeaveCritical(sr);
}

void nOS_TimerProcess (
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
    uint8_t prio
#else
    void
#endif
)
{
    nOS_StatusReg       sr;
    nOS_Timer           *timer;
    nOS_TimerCallback   callback = NULL;
    void                *arg;

    nOS_EnterCritical(sr);
    timer = (nOS_Timer*)nOS_GetHeadOfList(
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
        &_triggeredList[prio]
#else
        &_triggeredList
#endif
    );
    if (timer != NULL) {
        timer->overflow--;
        if (timer->overflow == 0) {
            _RemoveFromTriggeredList(timer);
        }
        else {
            _RotateTriggeredList(timer);
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

nOS_Error nOS_TimerCreate (nOS_Timer *timer,
                           nOS_TimerCallback callback,
                           void *arg,
                           nOS_TimerCounter reload,
                           nOS_TimerMode mode
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
                          ,uint8_t prio
#endif
                          )
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
        err = NOS_E_INV_VAL;
    } else
 #if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
    if (prio > NOS_CONFIG_TIMER_HIGHEST_PRIO) {
        err = NOS_E_INV_PRIO;
    } else
 #endif
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
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
            timer->prio         = prio;
#endif
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
                _RemoveFromActiveList(timer);
            }
            timer->state = NOS_TIMER_DELETED;
            if (timer->overflow > 0) {
                timer->overflow = 0;
                _RemoveFromTriggeredList(timer);
            }
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif  /* NOS_CONFIG_TIMER_DELETE_ENABLE */

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
        }
        else if (timer->reload == 0) {
            err = NOS_E_INV_VAL;
        } else
#endif
        {
            if ( !(timer->state & NOS_TIMER_RUNNING) ) {
                timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
                _AppendToActiveList(timer);
            }
            timer->count = _tickCounter + timer->reload;

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
                _RemoveFromActiveList(timer);
            }
            timer->state = (nOS_TimerState)(timer->state &~ (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED));
            if ((timer->overflow > 0) && instant) {
                timer->overflow = 0;
                _RemoveFromTriggeredList(timer);
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
    }
    else if (reload == 0) {
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
            if ( !(timer->state & NOS_TIMER_RUNNING) ) {
                timer->state  = (nOS_TimerState)(timer->state | NOS_TIMER_RUNNING);
                _AppendToActiveList(timer);
            }
            timer->reload = reload;
            timer->count  = _tickCounter + reload;

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
            if ((timer->state & (NOS_TIMER_RUNNING | NOS_TIMER_PAUSED)) == NOS_TIMER_RUNNING) {
                timer->state = (nOS_TimerState)(timer->state | NOS_TIMER_PAUSED);
                _RemoveFromActiveList(timer);
            }
            err = NOS_OK;
        }
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
        {
            if (timer->state & NOS_TIMER_PAUSED) {
                timer->state = (nOS_TimerState)(timer->state &~ NOS_TIMER_PAUSED);
                _AppendToActiveList(timer);
            }
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
    }
    else if (reload == 0) {
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
            timer->arg      = arg;

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
    }
    else if ((mode != NOS_TIMER_FREE_RUNNING) && (mode != NOS_TIMER_ONE_SHOT)) {
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

#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
nOS_Error nOS_TimerSetPrio (nOS_Timer *timer, uint8_t prio)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (timer == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (prio > NOS_CONFIG_TIMER_HIGHEST_PRIO) {
        err = NOS_E_INV_PRIO;
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
            _RemoveFromActiveList(timer);
            if (timer->overflow > 0) {
                _RemoveFromTriggeredList(timer);
            }
            timer->prio = prio;
            if (timer->overflow > 0) {
                _AppendToTriggeredList(timer);
            }
            _AppendToActiveList(timer);
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

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

bool nOS_TimerAnyTriggered(
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
    uint8_t prio
#else
    void
#endif
)
{
    nOS_StatusReg   sr;
    bool            triggered;

    nOS_EnterCritical(sr);
    triggered = (nOS_GetHeadOfList(
#if (NOS_CONFIG_TIMER_HIGHEST_PRIO > 0)
                &_triggeredList[prio]
#else
                &_triggeredList
#endif
    ) != NULL);
    nOS_LeaveCritical(sr);

    return triggered;
}
#endif  /* NOS_CONFIG_TIMER_ENABLE */

#ifdef __cplusplus
}
#endif
