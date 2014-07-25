/*
 * Copyright (c) 2014 Jim Tremblay
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

nOS_Error nOS_MemCreate (nOS_Mem *mem, void *buffer, size_t bsize, uint16_t max)
{
    nOS_Error   err;
    uint16_t    i;
    void        **list;

#if NOS_CONFIG_SAFE > 0
    if (mem == NULL) {
        err = NOS_E_NULL;
    } else if (buffer == NULL) {
        err = NOS_E_NULL;
    } else if (bsize < sizeof(void**)) {
        err = NOS_E_INV_VAL;
    } else if (max == 0) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        nOS_EventCreate((nOS_Event*)mem);
        list = NULL;
        for (i = 0; i < max-1; i++) {
            *(void**)buffer = list;
            list = (void**)buffer;
            buffer = ((uint8_t*)buffer + bsize);
        }
        *(void**)buffer = list;
        mem->list = (void**)buffer;
        mem->count = max;
        mem->max = max;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

void *nOS_MemAlloc(nOS_Mem *mem, uint16_t tout)
{
    int8_t  err;
    void    *block;

#if NOS_CONFIG_SAFE > 0
    if (mem == NULL) {
        block = NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (mem->count > 0) {
            block = (void*)mem->list;
            mem->list = *(void**)block;
            mem->count--;
        } else if (tout > 0) {
            nOS_runningThread->context = (void*)&block;
            err = nOS_EventWait((nOS_Event*)mem, NOS_MEM_ALLOC, tout);
            if (err != NOS_OK) {
                block = NULL;
            }
        } else {
            block = NULL;
        }
        nOS_CriticalLeave();
    }

    return block;
}

nOS_Error nOS_MemFree(nOS_Mem *mem, void *block)
{
    nOS_Error   err;
    nOS_Thread  *thread;

#if NOS_CONFIG_SAFE > 0
    if (mem == NULL) {
        err = NOS_E_NULL;
    } else if (block == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (mem->count < mem->max) {
            thread = nOS_EventSignal((nOS_Event*)mem);
            if (thread != NULL) {
                *(void**)thread->context = block;
                if ((thread->state == NOS_READY) && (thread->prio > nOS_runningThread->prio)) {
                    nOS_Sched();
                }
            } else {
                *(void**)block = mem->list;
                mem->list = (void**)block;
                mem->count++;
            }
            err = NOS_OK;
        } else {
            err = NOS_E_FULL;
        }
        nOS_CriticalLeave();
    }

    return err;
}

#if defined(__cplusplus)
}
#endif
