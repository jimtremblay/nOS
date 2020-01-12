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

#if (NOS_CONFIG_FLAG_ENABLE > 0)
static inline nOS_FlagBits _GetHighestFlag (nOS_FlagBits flags)
{
    nOS_FlagBits    n;

    if (flags) {
        n = flags;
        n |= (n >> 1);
        n |= (n >> 2);
        n |= (n >> 4);
#if (NOS_CONFIG_FLAG_NB_BITS > 8)
        n |= (n >> 8);
 #if (NOS_CONFIG_FLAG_NB_BITS > 16)
        n |= (n >> 16);
  #if (NOS_CONFIG_FLAG_NB_BITS > 32)
        n |= (n >> 32);
  #endif
 #endif
#endif
        return ((n >> 1) + 1);
    }
    return (nOS_FlagBits)0;
}

static void _TestFlag (void *payload, void *arg)
{
    nOS_Thread      *thread  = (nOS_Thread*)payload;
    nOS_Flag        *flag    = (nOS_Flag*)thread->event;
    nOS_FlagContext *ctx     = (nOS_FlagContext*)thread->ext;
    nOS_FlagBits    *res     = &((nOS_FlagBits*)arg)[0];
    nOS_FlagBits    *excl    = &((nOS_FlagBits*)arg)[1];
    nOS_FlagBits    r;

    /* Verify flags from object with wanted flags from waiting thread. */
    r = (flag->flags & ctx->flags) &~ *excl;
    if (((ctx->opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != ctx->flags)) {
        r = NOS_FLAG_NONE;
    }
    if (((ctx->opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ONE) && (r != NOS_FLAG_NONE)) {
        r = _GetHighestFlag(r);
    }
    /* If conditions are met, wake up the thread and give it the result. */
    if (r != NOS_FLAG_NONE) {
        *ctx->rflags = r;
        /* Ensure exclusive flags will be cleared. */
        *res |= (r & flag->excl);
        /* Accumulate awoken flags if waiting thread want to clear it when awoken. */
        if (ctx->opt & NOS_FLAG_CLEAR_ON_EXIT) {
            *res |= r;
        }
        /* Accumulate exclusive flags to not resend it to other thread. */
        *excl |= (r & flag->excl);
        nOS_WakeUpThread(thread, NOS_OK);
    }
}

nOS_Error nOS_FlagCreate (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits excl)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (flag->e.type != NOS_EVENT_INVALID) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            nOS_CreateEvent((nOS_Event*)flag
#if (NOS_CONFIG_SAFE > 0)
                           ,NOS_EVENT_FLAG
#endif
                           );
            flag->flags = flags;
            flag->excl  = excl;

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_FLAG_DELETE_ENABLE > 0)
nOS_Error nOS_FlagDelete (nOS_Flag *flag)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (flag->e.type != NOS_EVENT_FLAG) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            flag->flags = NOS_FLAG_NONE;
            nOS_DeleteEvent((nOS_Event*)flag);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

nOS_Error nOS_FlagWait (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits *res,
                        nOS_FlagOption opt, nOS_TickCounter timeout)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_FlagContext ctx;
    nOS_FlagBits    r;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (flag->e.type != NOS_EVENT_FLAG) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            r = flag->flags & flags;
            /* If thread is waiting for ALL flags, then clear result if NOT ALL flags set. */
            if (((opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != flags)) {
                r = NOS_FLAG_NONE;
            }
            /* If thread is waiting for ONE flag, then get highest bit set. */
            if (((opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ONE) && (r != NOS_FLAG_NONE)) {
                r = _GetHighestFlag(r);
            }

            /* If result is not cleared, then condition is met for waiting thread. */
            if (r != NOS_FLAG_NONE) {
                /* CLear all exclusive flags sent to the thread. */
                flag->flags &=~ (r & flag->excl);
                if (opt & NOS_FLAG_CLEAR_ON_EXIT) {
                    /* Clear all flags that have awoken the waiting threads. */
                    flag->flags &=~ r;
                }
                err = NOS_OK;
            }
            else if (timeout == NOS_NO_WAIT) {
                /* Caller can't wait? Try again. */
                err = NOS_E_AGAIN;
            }
            else {
                /* Calling thread must wait on flag. */
                ctx.flags   = flags;
                ctx.opt     = opt;
                ctx.rflags  = &r;
                nOS_runningThread->ext = &ctx;
                err = nOS_WaitForEvent((nOS_Event*)flag,
                                       NOS_THREAD_WAITING_FLAG
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0)
                                      ,timeout
#elif (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                                      ,NOS_WAIT_INFINITE
#endif
                                      );
            }
        }
        nOS_LeaveCritical(sr);

        /* Return awoken flags if succeed to wait on flag object. */
        if (err == NOS_OK && res != NULL) {
            *res = r;
        }
    }

    return err;
}

nOS_Error nOS_FlagSend (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits mask)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_FlagBits    arg[2];

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (flag->e.type != NOS_EVENT_FLAG) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            flag->flags ^= ((flag->flags ^ flags) & mask);
            arg[0] = NOS_FLAG_NONE;
            arg[1] = NOS_FLAG_NONE;
            nOS_WalkInList(&flag->e.waitList, _TestFlag, (void*)&arg);
            /* Clear all flags that have awoken the waiting threads. */
            flag->flags &=~ arg[0];

#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            nOS_Schedule();
#endif

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif  /* NOS_CONFIG_FLAG_ENABLE */

#ifdef __cplusplus
}
#endif
