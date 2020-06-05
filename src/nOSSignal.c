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

#if (NOS_CONFIG_SIGNAL_ENABLE > 0)
#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
 #if (NOS_CONFIG_THREAD_JOIN_ENABLE > 0)
  static int _Thread (void *arg);
 #else
  static void _Thread (void *arg);
 #endif
#endif

#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
 static nOS_List                    _list[NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1];
#else
 static nOS_List                    _list;
#endif
#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
 #if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
  static nOS_Thread                 _thread[NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1];
  #define _GetSignalThread(s)       (&_thread[(s)->prio])
  static NOS_CONST uint8_t          _prio[] = {NOS_CONFIG_SIGNAL_THREAD_PRIO};
  NOS_STATIC_ASSERT(NOS_ROW_COUNT(_prio)==(NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1),Priority_count_mismatch_in_list_NOS_CONFIG_SIGNAL_THREAD_PRIO);
 #else
  static nOS_Thread                 _thread;
  #define _GetSignalThread(s)       (&_thread)
 #endif
 #ifdef NOS_SIMULATED_STACK
  #if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
   static nOS_Stack                 _stack[NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1];
  #else
   static nOS_Stack                 _stack;
  #endif
 #else
  #if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
   static nOS_Stack                 _stack[NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1][NOS_CONFIG_SIGNAL_THREAD_STACK_SIZE]
   #ifdef NOS_CONFIG_SIGNAL_THREAD_STACK_SECTION
        __attribute__ ( ( section(NOS_CONFIG_SIGNAL_THREAD_STACK_SECTION) ) )
   #endif
   ;
  #else
   static nOS_Stack                 _stack[NOS_CONFIG_SIGNAL_THREAD_STACK_SIZE]
   #ifdef NOS_CONFIG_SIGNAL_THREAD_STACK_SECTION
        __attribute__ ( ( section(NOS_CONFIG_SIGNAL_THREAD_STACK_SECTION) ) )
   #endif
   ;
  #endif
 #endif
#elif defined(NOS_CONFIG_SIGNAL_USER_THREAD_HANDLE)
 #if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
  static NOS_CONST nOS_Thread*      _handle[] = {NOS_CONFIG_SIGNAL_USER_THREAD_HANDLE};
  #define _GetSignalThread(s)       (_handle[(s)->prio])
  NOS_STATIC_ASSERT(NOS_ROW_COUNT(_handle)==(NOS_CONFIG_SIGNAL_HIGHEST_PRIO+1),Handle_count_mismatch_in_list_NOS_CONFIG_SIGNAL_USER_THREAD_HANDLE);
 #else
  #define _GetSignalThread(s)       (NOS_CONFIG_SIGNAL_USER_THREAD_HANDLE)
 #endif
#endif
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
 #define _AppendToList(s)                   nOS_AppendToList(&_list[(s)->prio], &(s)->node)
 #define _RemoveFromList(s)                 nOS_RemoveFromList(&_list[(s)->prio], &(s)->node)
 #define _RotateList(s)                     nOS_RotateList(&_list[(s)->prio])
#else
 #define _AppendToList(s)                   nOS_AppendToList(&_list, &(s)->node)
 #define _RemoveFromList(s)                 nOS_RemoveFromList(&_list, &(s)->node)
 #define _RotateList(s)                     nOS_RotateList(&_list)
#endif

#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
#if (NOS_CONFIG_THREAD_JOIN_ENABLE > 0)
static int _Thread (void *arg)
#else
static void _Thread (void *arg)
#endif
{
    nOS_StatusReg   sr;
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
    unsigned int    prio = (unsigned int)arg;
#else
    NOS_UNUSED(arg);
#endif

    while (1) {
        nOS_SignalProcess(
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
            prio
#endif
        );
        nOS_EnterCritical(sr);
        if (!nOS_SignalAnyRaised(
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
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
#endif  /* NOS_CONFIG_SIGNAL_THREAD_ENABLE */

void nOS_InitSignal (void)
{
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
    {
        unsigned int i;
        for (i = 0; i <= NOS_CONFIG_SIGNAL_HIGHEST_PRIO; i++) {
            nOS_InitList(&_list[i]);
 #if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
            if (_prio[i] <= NOS_CONFIG_HIGHEST_THREAD_PRIO) {
                nOS_ThreadCreate(&_thread[i],
                                 _Thread,
                                 (void*)i
  #ifdef NOS_SIMULATED_STACK
                                ,&_stack[i]
  #else
                                ,_stack[i]
  #endif
                                ,NOS_CONFIG_SIGNAL_THREAD_STACK_SIZE
  #ifdef NOS_USE_SEPARATE_CALL_STACK
                                ,NOS_CONFIG_SIGNAL_THREAD_CALL_STACK_SIZE
  #endif
                                ,_prio[i]
  #if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                                ,NOS_THREAD_READY
  #endif
  #if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                                ,"nOS_Signal #" NOS_STR(i+1)
  #endif
                );
            }
        }
 #endif
    }
#else
    nOS_InitList(&_list);
 #if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
    nOS_ThreadCreate(&_thread,
                     _Thread,
                     NULL
  #ifdef NOS_SIMULATED_STACK
                    ,&_stack
  #else
                    ,_stack
  #endif
                    ,NOS_CONFIG_SIGNAL_THREAD_STACK_SIZE
  #ifdef NOS_USE_SEPARATE_CALL_STACK
                    ,NOS_CONFIG_SIGNAL_THREAD_CALL_STACK_SIZE
  #endif
  #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                    ,NOS_CONFIG_SIGNAL_THREAD_PRIO
  #endif
  #if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                    ,NOS_THREAD_READY
  #endif
  #if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                    ,"nOS_Signal"
  #endif
    );
 #endif
#endif
}

void nOS_SignalProcess (
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
    uint8_t prio
#else
    void
#endif
)
{
    nOS_StatusReg       sr;
    nOS_Signal          *signal  = NULL;
    nOS_SignalCallback  callback = NULL;
    void                *arg     = NULL;

    nOS_EnterCritical(sr);
    signal = (nOS_Signal *)nOS_GetHeadOfList(
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
        &_list[prio]
#else
        &_list
#endif
    );
    if (signal != NULL) {
        if (signal->count > 0) {
            signal->count--;
            if (signal->count == 0) {
                _RemoveFromList(signal);
            } else {
                _RotateList(signal);
            }

            callback = signal->callback;
            if (signal->buffer != NULL) {
                arg = signal->buffer[signal->r];
                signal->r = (signal->r + 1) % signal->max;
            } else if (signal->max == 1) {
                arg = (void*)signal->buffer;
            }
        }
    }
    nOS_LeaveCritical(sr);

    if (callback != NULL) {
        callback(signal, arg);
    }
}

nOS_Error nOS_SignalCreate (nOS_Signal *signal,
                            nOS_SignalCallback callback,
                            void **buffer,
                            nOS_SignalCounter max
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
                            ,uint8_t prio
#endif
                            )
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (callback == NULL) {
        err = NOS_E_INV_VAL;
    }
    else if (max == 0) {
        err = NOS_E_INV_VAL;
    } else
 #if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
    if (prio > NOS_CONFIG_SIGNAL_HIGHEST_PRIO) {
        err = NOS_E_INV_PRIO;
    } else
 #endif
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state != NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
#if (NOS_CONFIG_SAFE > 0)
            signal->state        = NOS_SIGNAL_CREATED;
#endif
            signal->callback     = callback;
            signal->buffer       = buffer;
            signal->count        = 0;
            signal->max          = max;
            signal->r            = 0;
            signal->w            = 0;
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
            signal->prio         = prio;
#endif
            signal->node.payload = (void *)signal;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_SIGNAL_DELETE_ENABLE > 0)
nOS_Error nOS_SignalDelete (nOS_Signal *signal)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            if (signal->count > 0) {
                _RemoveFromList(signal);
            }
#if (NOS_CONFIG_SAFE > 0)
            signal->state       = NOS_SIGNAL_DELETED;
#endif
            signal->callback    = NULL;
            signal->buffer      = NULL;
            signal->count       = 0;
            signal->max         = 0;
            signal->r           = 0;
            signal->w           = 0;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif /* NOS_CONFIG_SIGNAL_DELETE_ENABLE */

nOS_Error nOS_SignalSend (nOS_Signal *signal, void *arg)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        if (signal->count == signal->max) {
            err = NOS_E_OVERFLOW;
        } else {
            signal->count++;
            if (signal->buffer != NULL) {
                signal->buffer[signal->w] = arg;
                signal->w = (signal->w + 1) % signal->max;
            } else if (signal->max == 1) {
                signal->buffer = (void**)arg;
            }
            if (signal->count == 1) {
                _AppendToList(signal);
            }

#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0) || defined(NOS_CONFIG_SIGNAL_USER_THREAD_HANDLE)
            if (_GetSignalThread(signal)->state == (NOS_THREAD_READY | NOS_THREAD_ON_HOLD)) {
                nOS_WakeUpThread((nOS_Thread*)_GetSignalThread(signal), NOS_OK);
 #if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                /* Verify if have wake up the highest prio thread */
                nOS_Schedule();
 #endif
            }
#endif
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_SignalSetCallback (nOS_Signal *signal, nOS_SignalCallback callback)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (callback == NULL) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            signal->callback = callback;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
nOS_Error nOS_SignalSetPrio (nOS_Signal *signal, uint8_t prio)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (prio > NOS_CONFIG_SIGNAL_HIGHEST_PRIO) {
        err = NOS_E_INV_PRIO;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            if (signal->count > 0) {
                _RemoveFromList(signal);
            }
            signal->prio = prio;
            if (signal->count > 0) {
                _AppendToList(signal);
            }

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif  /* NOS_CONFIG_SIGNAL_HIGHEST_PRIO */

nOS_SignalCounter nOS_SignalGetCount (nOS_Signal *signal)
{
    nOS_StatusReg       sr;
    nOS_SignalCounter   count;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        count = 0;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            count = 0;
        } else
#endif
        {
            count = signal->count;
        }
        nOS_LeaveCritical(sr);
    }

    return count;
}

bool nOS_SignalAnyRaised (
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
    uint8_t prio
#else
    void
#endif
)
{
    nOS_StatusReg   sr;
    bool            raised;

    nOS_EnterCritical(sr);
    raised = (nOS_GetHeadOfList(
#if (NOS_CONFIG_SIGNAL_HIGHEST_PRIO > 0)
                &_list[prio]
#else
                &_list
#endif
    ) != NULL);
    nOS_LeaveCritical(sr);

    return raised;
}
#endif  /* NOS_CONFIG_SIGNAL_ENABLE */

#ifdef __cplusplus
}
#endif
