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

#if (NOS_CONFIG_SIGNAL_ENABLE > 0)
#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
 static void    _Thread     (void *arg);
#endif

static nOS_List     _list;
#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
 static nOS_Thread  _thread;
 #ifdef NOS_SIMULATED_STACK
  static nOS_Stack  _stack;
 #else
  static nOS_Stack  _stack[NOS_CONFIG_SIGNAL_THREAD_STACK_SIZE];
 #endif
#endif

#if (NOS_CONFIG_SIGNAL_THREAD_ENABLE > 0)
static void _Thread (void *arg)
{
    nOS_StatusReg   sr;

    NOS_UNUSED(arg);

    while (1) {
        nOS_SignalProcess();

        nOS_EnterCritical(sr);
        if (nOS_GetHeadOfList(&_list) == NULL) {
            nOS_WaitForEvent(NULL, NOS_THREAD_ON_HOLD, NOS_WAIT_INFINITE);
        }
        nOS_LeaveCritical(sr);
    }
}
#endif

void nOS_InitSignal (void)
{
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
}

void nOS_SignalProcess (void)
{
    nOS_StatusReg       sr;
    nOS_Signal          *signal  = NULL;
    nOS_SignalCallback  callback = NULL;
    void                *arg     = NULL;

    nOS_EnterCritical(sr);
    signal = (nOS_Signal *)nOS_GetHeadOfList(&_list);
    if (signal != NULL) {
        if (signal->state & NOS_SIGNAL_RAISED) {
            signal->state = (nOS_SignalState)(signal->state &~ NOS_SIGNAL_RAISED);
            nOS_RemoveFromList(&_list, &signal->node);

            callback = signal->callback;
            arg      = signal->arg;
        }
    }
    nOS_LeaveCritical(sr);

    if (callback != NULL) {
        callback(signal, arg);
    }
}

nOS_Error nOS_SignalCreate (nOS_Signal *signal, nOS_SignalCallback callback)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (callback == NULL) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state != NOS_SIGNAL_DELETED) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            signal->state        = NOS_SIGNAL_CREATED;
            signal->callback     = callback;
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
            if (signal->state & NOS_SIGNAL_RAISED) {
                nOS_RemoveFromList(&_list, &signal->node);
            }
            signal->state           = NOS_SIGNAL_DELETED;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

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
        {
            if (signal->state & NOS_SIGNAL_RAISED) {
                err = NOS_E_OVERFLOW;
            } else {
                signal->state = (nOS_SignalState)(signal->state | NOS_SIGNAL_RAISED);
                signal->arg   = arg;
                nOS_AppendToList(&_list, &signal->node);

                if (_thread.state == (NOS_THREAD_READY | NOS_THREAD_ON_HOLD)) {
                    nOS_WakeUpThread(&_thread, NOS_OK);
                }

                err = NOS_OK;
            }
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
    } else if (callback == NULL) {
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

bool nOS_SignalIsRaised (nOS_Signal *signal)
{
    nOS_StatusReg   sr;
    bool            raised;

#if (NOS_CONFIG_SAFE > 0)
    if (signal == NULL) {
        raised = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (signal->state == NOS_SIGNAL_DELETED) {
            raised = false;
        } else
#endif
        {
            raised = (signal->state & NOS_SIGNAL_RAISED) == NOS_SIGNAL_RAISED;
        }
        nOS_LeaveCritical(sr);
    }

    return raised;
}
#endif  /* NOS_CONFIG_SIGNAL_ENABLE */

#ifdef __cplusplus
}
#endif
