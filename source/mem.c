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

/*
 * Name        : nOS_MemCreate
 *
 * Description : Create a dynamic array of fixed block size.
 *
 * Arguments   : mem    : Pointer to mem object.
 *               buffer : Pointer to array of memory.
 *               bsize  : Size of ONE block of memory.
 *               max    : Maximum number of blocks your buffer can hold.
 *
 * Return      : Error code.
 *               NOS_E_NULL    : Pointer to mem object is NULL.
 *                               OR
 *                               Pointer to buffer array is NULL.
 *               NOS_E_INV_VAL : Size of ONE block is too small.
 *                               OR
 *                               Maximum number of blocks is 0.
 *               NOS_OK        : Mem created with success.
 *
 * Note        : Mem object must be created before using it, else
 *               behaviour is undefined and must be called one time
 *               ONLY for each mem object.
 */
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
        /* Initialize the single-link list */
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

/*
 * Name        : nOS_MemAlloc
 *
 * Description : Wait on mem object for ONE block of memory. If no block available,
 *               calling thread will be placed in object's waiting list for number
 *               of ticks specified by tout. If a block of memory is freed before
 *               end of timeout, thread will be awoken and pointer to memory block
 *               will be returned.
 *
 * Arguments   : mem   : Pointer to mem object.
 *               tout  : Timeout value
 *                       NOS_NO_WAIT      : No waiting.
 *                       NOS_WAIT_INIFINE : Never timeout.
 *                       0 > tout < 65535 : Number of ticks to wait on mem object.
 *
 * Return      : Pointer to allocated block of memory.
 *               == NULL : No block available.
 *               != NULL : Pointer to newly allocated block of memory.
 *
 * Note        : Caller is responsible to free the block when memory is no longer needed.
 */
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
        /* Caller can't wait? Try again. */
        } else if (tout == NOS_NO_WAIT) {
            block = NULL;
        } else if (nOS_isrNestingCounter > 0) {
            block = NULL;
        } else if (nOS_lockNestingCounter > 0) {
            block = NULL;
        } else if (nOS_runningThread == &nOS_mainThread) {
            block = NULL;
        } else {
            nOS_runningThread->context = (void*)&block;
            err = nOS_EventWait((nOS_Event*)mem, NOS_MEM_ALLOC, tout);
            if (err != NOS_OK) {
                block = NULL;
            }
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
        thread = nOS_EventSignal((nOS_Event*)mem);
        if (thread != NULL) {
            *(void**)thread->context = block;
            if ((thread->state == NOS_READY) && (thread->prio > nOS_runningThread->prio)) {
                nOS_Sched();
            }
            err = NOS_OK;
        } else if (mem->count < mem->max) {
            *(void**)block = mem->list;
            mem->list = (void**)block;
            mem->count++;
            err = NOS_OK;
        } else {
            err = NOS_E_OVERFLOW;
        }
        nOS_CriticalLeave();
    }

    return err;
}

#if defined(__cplusplus)
}
#endif
