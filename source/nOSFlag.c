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

#if (NOS_CONFIG_FLAG_ENABLE > 0)
static void _TestFlag (void *payload, void *arg)
{
    nOS_Thread      *thread  = (nOS_Thread*)payload;
    nOS_Flag        *flag    = (nOS_Flag*)thread->event;
    nOS_FlagContext *ctx = (nOS_FlagContext*)thread->ext;
    nOS_FlagResult  *res = (nOS_FlagResult*)arg;
    nOS_FlagBits    r;

    /* Verify flags from object with wanted flags from waiting thread. */
    r = flag->flags & ctx->flags;
    if (((ctx->opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != ctx->flags)) {
        r = NOS_FLAG_NONE;
    }
    /* If conditions are met, wake up the thread and give it the result. */
    if (r != NOS_FLAG_NONE) {
        nOS_SignalThread(thread, NOS_OK);
        *ctx->rflags = r;
        /* Accumulate awoken flags if waiting thread want to clear it when awoken. */
        if (ctx->opt & NOS_FLAG_CLEAR_ON_EXIT) {
            res->rflags |= r;
        }
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (thread->prio > nOS_runningThread->prio) {
            /* Indicate that we need a preemptive scheduling. */
            res->sched = true;
        }
#endif
    }
}

nOS_Error nOS_FlagCreate (nOS_Flag *flag, nOS_FlagBits flags)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (flag->e.type != NOS_EVENT_INVALID) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        nOS_CreateEvent((nOS_Event*)flag, NOS_EVENT_FLAG);
#else
        nOS_CreateEvent((nOS_Event*)flag);
#endif
        flag->flags = flags;
        nOS_LeaveCritical(sr);
        err = NOS_OK;
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
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
        flag->flags = NOS_FLAG_NONE;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (nOS_DeleteEvent((nOS_Event*)flag)) {
            nOS_Schedule();
        }
#else
        nOS_DeleteEvent((nOS_Event*)flag);
#endif
        nOS_LeaveCritical(sr);
        err = NOS_OK;
    }

    return err;
}
#endif

nOS_Error nOS_FlagWait (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits *res,
                        nOS_FlagOption opt, nOS_TickCounter tout)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_FlagContext ctx;
    nOS_FlagBits    r;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
        r = flag->flags & flags;
        /* If thread is waiting for ALL flags, then clear result if NOT ALL flags set. */
        if (((opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != flags)) {
            r = NOS_FLAG_NONE;
        }
        /* If result is not cleared, then condition is met for waiting thread. */
        if (r != NOS_FLAG_NONE) {
            if (opt & NOS_FLAG_CLEAR_ON_EXIT) {
                /* Clear all flags that have awoken the waiting threads. */
                flag->flags &=~ r;
            }
            err = NOS_OK;
        } else if (tout == NOS_NO_WAIT) {
            /* Caller can't wait? Try again. */
            err = NOS_E_AGAIN;
        } else if (nOS_isrNestingCounter > 0) {
            /* Can't wait from ISR */
            err = NOS_E_ISR;
        }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        else if (nOS_lockNestingCounter > 0) {
            /* Can't switch context when scheduler is locked */
            err = NOS_E_LOCKED;
        }
#endif
        else if (nOS_runningThread == &nOS_idleHandle) {
            /* Main thread can't wait */
            err = NOS_E_IDLE;
        } else {
            /* Calling thread must wait on flag. */
            ctx.flags   = flags;
            ctx.opt     = opt;
            ctx.rflags  = &r;
            nOS_runningThread->ext = &ctx;
            err = nOS_WaitForEvent((nOS_Event*)flag, NOS_THREAD_WAITING_FLAG, tout);
        }
        nOS_LeaveCritical(sr);

        /* Return awoken flags if succeed to wait on flag object. */
        if (err == NOS_OK) {
            if (res != NULL) {
                *res = r;
            }
        }
    }

    return err;
}

nOS_Error nOS_FlagSend (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits mask)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_FlagResult  res;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        res.rflags = NOS_FLAG_NONE;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        res.sched = false;
#endif
        nOS_EnterCritical(sr);
        flag->flags ^= ((flag->flags ^ flags) & mask);
        nOS_WalkInList(&flag->e.waitList, _TestFlag, &res);
        /* Clear all flags that have awoken the waiting threads. */
        flag->flags &=~ res.rflags;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        /* Schedule only if one of awoken thread has an higher priority. */
        if (res.sched) {
            nOS_Schedule();
        }
#endif
        nOS_LeaveCritical(sr);
        err = NOS_OK;
    }

    return err;
}
#endif  /* NOS_CONFIG_FLAG_ENABLE */

#ifdef __cplusplus
}
#endif
