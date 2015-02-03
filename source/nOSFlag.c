/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

#if (NOS_CONFIG_FLAG_ENABLE > 0)
/* Internal function */
static void TestFlag (void *payload, void *arg)
{
    nOS_Thread      *thread = (nOS_Thread*)payload;
    nOS_Flag        *flag = (nOS_Flag*)thread->event;
    nOS_FlagContext *ctx = (nOS_FlagContext*)thread->context;
    nOS_FlagResult  *res = (nOS_FlagResult*)arg;
    nOS_FlagBits    r;

    /* Verify flags from object with wanted flags from waiting thread. */
    r = flag->flags & ctx->flags;
    if (((ctx->opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != ctx->flags)) {
        r = NOS_FLAG_NONE;
    }
    /* If conditions are met, wake up the thread and give it the result. */
    if (r != NOS_FLAG_NONE) {
        SignalThread(thread, NOS_OK);
        *ctx->rflags = r;
        /* Accumulate awoken flags if waiting thread want to clear it when awoken. */
        if (ctx->opt & NOS_FLAG_CLEAR_ON_EXIT) {
            res->rflags |= r;
        }
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        /* Indicate that we need a preemptive scheduling. */
        if (thread->prio > nOS_runningThread->prio) {
            res->sched = true;
        }
#endif
    }
}

/*
 * Name        : nOS_FlagCreate
 *
 * Description : Initialize a flag event object with given flags.
 *
 * Arguments   : flag  : Pointer to flag object.
 *               flags : Initial values.
 *
 * Return      : Error code.
 *               NOS_E_NULL : Pointer to flag object is NULL.
 *               NOS_OK     : Flag initialized with success.
 *
 * Note        : Flag object must be created before using it, else
 *               behaviour is undefined and must be called one time
 *               ONLY for each flag object.
 */
nOS_Error nOS_FlagCreate (nOS_Flag *flag, nOS_FlagBits flags)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_FULL;
    } else if (flag->e.type != NOS_EVENT_INVALID) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_CriticalEnter();
#if (NOS_CONFIG_SAFE > 0)
        nOS_EventCreate((nOS_Event*)flag, NOS_EVENT_FLAG);
#else
        nOS_EventCreate((nOS_Event*)flag);
#endif
        flag->flags = flags;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

#if (NOS_CONFIG_FLAG_DELETE_ENABLE > 0)
nOS_Error nOS_FlagDelete (nOS_Flag *flag)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_NULL;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_CriticalEnter();
        flag->flags = NOS_FLAG_NONE;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (nOS_EventDelete((nOS_Event*)flag)) {
            nOS_Sched();
        }
#else
        nOS_EventDelete((nOS_Event*)flag);
#endif
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}
#endif

/*
 * Name        : nOS_FlagWait
 *
 * Description : Wait on flag object for given flags. If flags are NOT set, calling
 *               thread will be placed in object's waiting list for number of ticks
 *               specified by tout. If flags are set before end of timeout, res
 *               will contain flags that have awoken the thread. If caller specify
 *               NOS_FLAG_CLEAR_ON_EXIT, ONLY awoken flags will be cleared.
 *
 * Arguments   : flag  : Pointer to flag object.
 *               flags : All flags to wait.
 *               res   : Pointer where to store awoken flags if needed. Only valid if
 *                       returned error code is NOS_OK. Otherwise, res is unchanged.
 *               opt   : Waiting options
 *                       NOS_FLAG_WAIT_ALL      : Wait for all flags to be set.
 *                       NOS_FLAG_WAIT_ANY      : Wait for any flags to be set.
 *                       NOS_FLAG_CLEAR_ON_EXIT : Clear woken flags.
 *               tout  : Timeout value
 *                       NOS_NO_WAIT       : No waiting.
 *                       NOS_WAIT_INFINITE : Never timeout.
 *                       0 > tout < 65535  : Number of ticks to wait on flag object.
 *
 * Return      : Error code.
 *               NOS_E_NULL    : Pointer to flag object is NULL.
 *               NOS_E_ISR     : Called from interrupt.
 *               NOS_E_LOCKED  : Called with scheduler locked.
 *               NOS_E_IDLE    : Called from idle thread.
 *               NOS_E_AGAIN   : Flags NOT in wanted state and tout == 0.
 *               NOS_E_TIMEOUT : Flags NOT in wanted state after tout ticks.
 *               NOS_OK        : Flags are in wanted state.
 *
 * Note        : Safe to be called from threads ONLY with scheduler unlocked.
 */
nOS_Error nOS_FlagWait (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits *res,
                        uint8_t opt, nOS_TickCounter tout)
{
    nOS_Error       err;
    nOS_FlagContext ctx;
    nOS_FlagBits    r;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_NULL;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_CriticalEnter();
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
        /* Caller can't wait? Try again. */
        } else if (tout == NOS_NO_WAIT) {
            err = NOS_E_AGAIN;
        /* Can't wait from ISR */
        } else if (nOS_isrNestingCounter > 0) {
            err = NOS_E_ISR;
        }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        /* Can't switch context when scheduler is locked */
        else if (nOS_lockNestingCounter > 0) {
            err = NOS_E_LOCKED;
        }
#endif
        /* Main thread can't wait */
        else if (nOS_runningThread == &nOS_idleHandle) {
            err = NOS_E_IDLE;
        /* Calling thread must wait on flag. */
        } else {
            ctx.flags   = flags;
            ctx.opt     = opt;
            ctx.rflags  = &r;
            nOS_runningThread->context = &ctx;
            err = nOS_EventWait((nOS_Event*)flag, NOS_THREAD_WAITING_FLAG, tout);
        }
        nOS_CriticalLeave();

        /* Return awoken flags if succeed to wait on flag object. */
        if (err == NOS_OK) {
            if (res != NULL) {
                *res = r;
            }
        }
    }

    return err;
}

/*
 * Name        : nOS_FlagSend
 *
 * Description : Set/Clear given flags on flag object. Many flags can be set and clear
 *               at the same time atomically. Can clear flags that has just been set
 *               if waiting threads as requested NOS_FLAG_CLEAR_ON_EXIT.
 *
 * Arguments   : flag  : Pointer to flag object.
 *               flags : All flags value to set or clear depending on mask.
 *               mask  : Mask containing which flags to affect. If corresponding bit
 *                       in flags is 0, this bit will be cleared. If corresponding
 *                       bit in flags is 1, this bit will be set.
 *
 * Return      : Error code.
 *               NOS_E_NULL    : Pointer to flag object is NULL.
 *               NOS_OK        : Flags are set/clear successfully.
 *
 * Note        : Safe to be called from threads, idle and ISR.
 */
nOS_Error nOS_FlagSend (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits mask)
{
    nOS_Error       err;
    nOS_FlagResult  res;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        err = NOS_E_NULL;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        res.rflags = NOS_FLAG_NONE;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        res.sched = false;
#endif
        nOS_CriticalEnter();
        flag->flags ^= ((flag->flags ^ flags) & mask);
        nOS_ListWalk(&flag->e.waitList, TestFlag, &res);
        /* Clear all flags that have awoken the waiting threads. */
        flag->flags &=~ res.rflags;
        nOS_CriticalLeave();
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        /* Schedule only if one of awoken thread has an higher priority. */
        if (res.sched) {
            nOS_Sched();
        }
#endif
        err = NOS_OK;
    }

    return err;
}

nOS_FlagBits nOS_FlagTest (nOS_Flag *flag, nOS_FlagBits flags, bool all)
{
    nOS_FlagBits    res;

#if (NOS_CONFIG_SAFE > 0)
    if (flag == NULL) {
        res = NOS_FLAG_NONE;
    } else if (flag->e.type != NOS_EVENT_FLAG) {
        res = NOS_FLAG_NONE;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (all) {
            res = (flag->flags & flags) == flags ? flags : NOS_FLAG_NONE;
        } else {
            res = (flag->flags & flags);
        }
        nOS_CriticalLeave();
    }

    return res;
}
#endif  /* NOS_CONFIG_FLAG_ENABLE */

#if defined(__cplusplus)
}
#endif
