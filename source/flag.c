/*
 * nOS v0.1
 * Copyright (c) 2014 Jim Tremblay
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

int8_t nOS_FlagCreate (nOS_Flag *flag, unsigned int flags)
{
    int8_t  err;

#if NOS_SAFE > 0
    if (flag == NULL) {
        err = NOS_E_FULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        nOS_EventCreate((nOS_Event*)flag);
        flag->flags = flags;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

int8_t nOS_FlagWait (nOS_Flag *flag, uint8_t opt, unsigned int flags, unsigned int *res, uint16_t tout)
{
    int8_t          err;
    nOS_FlagContext ctx;
    unsigned int    r;

#if NOS_SAFE > 0
    if (flag == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    } else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    } else if ((nOS_runningThread == &nOS_mainThread) && (tout > 0)) {
        err = NOS_E_IDLE;
    } else {
        nOS_CriticalEnter();
        r = flag->flags & flags;
        if (((opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != flags)) {
            r = NOS_FLAG_NONE;
        }
        if (r != NOS_FLAG_NONE) {
            err = NOS_OK;
        } else if (tout > 0) {
            ctx.flags   = flags;
            ctx.opt     = opt;
            ctx.rflags  = &r;
            nOS_runningThread->context = &ctx;
            err = nOS_EventWait((nOS_Event*)flag, NOS_WAITING_FLAG, tout);
        } else {
            err = NOS_E_AGAIN;
        }
        nOS_CriticalLeave();

        if (err == NOS_OK) {
            if (res != NULL) {
                *res = r;
            }
        }
    }

    return err;
}

static void TestFlag (void *payload, void *arg)
{
    nOS_Thread      *thread = (nOS_Thread*)payload;
    nOS_Flag        *flag = (nOS_Flag*)thread->event;
    nOS_FlagContext *ctx = (nOS_FlagContext*)thread->context;
    nOS_FlagResult  *res = (nOS_FlagResult*)arg;
    unsigned int    r;

    r = flag->flags & ctx->flags;
    if (((ctx->opt & NOS_FLAG_WAIT) == NOS_FLAG_WAIT_ALL) && (r != ctx->flags)) {
        r = NOS_FLAG_NONE;
    }
    if (r != NOS_FLAG_NONE) {
        SignalThread(thread);
        *ctx->rflags = r;
        if (ctx->opt & NOS_FLAG_CLEAR_ON_EXIT) {
            res->rflags |= r;
        }
        if (thread->prio > nOS_runningThread->prio) {
            res->sched = 1;
        }
    }
}

int8_t nOS_FlagSet (nOS_Flag *flag, unsigned int flags, unsigned int mask)
{
    int8_t          err;
    nOS_FlagResult  res;

#if NOS_SAFE > 0
    if (flag == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        res.rflags = NOS_FLAG_NONE;
        res.sched = 0;
        nOS_CriticalEnter();
        flag->flags = flag->flags ^ ((flag->flags ^ flags) & mask);
        nOS_ListWalk(&flag->e.waitingList, TestFlag, &res);
        flag->flags &= ~res.rflags;
        nOS_CriticalLeave();
        if (res.sched != 0) {
            nOS_Sched();
        }
        err = NOS_OK;
    }

    return err;
}

#ifdef __cplusplus
}
#endif
