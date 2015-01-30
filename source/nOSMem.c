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

#if (NOS_CONFIG_MEM_ENABLE > 0)
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
static nOS_Error SanityCheck(nOS_Mem *mem, void *block)
{
    nOS_Error   err;
    void        *p;

    /* Memory block pointer is out of range? */
    if (block < (void*)mem->buffer) {
        err = NOS_E_INV_VAL;
    } else if (block >= (void*)(mem->buffer + (mem->bsize * mem->bmax))) {
        err = NOS_E_INV_VAL;
    /* Memory block pointer is multiple of block size? */
    } else if ((uint16_t)((uint8_t*)block - mem->buffer) % mem->bsize != 0) {
        err = NOS_E_INV_VAL;
    } else if (mem->bcount == mem->bmax) {
        err = NOS_E_OVERFLOW;
    } else {
        /* Memory block is already free? */
        p = (void*)mem->blist;
        while ((p != NULL) && (p != block)) {
            p = *(void**)p;
        }
        if (p == block) {
            err = NOS_E_OVERFLOW;
        } else {
            err = NOS_OK;
        }
    }

    return err;
}
#endif  /* NOS_CONFIG_MEM_SANITY_CHECK_ENABLE */

/*
 * Name        : nOS_MemCreate
 *
 * Description : Create a dynamic array of fixed block size of memory.
 *
 * Arguments   : mem    : Pointer to mem object.
 *               buffer : Pointer to array of memory.
 *               bsize  : Size of one block of memory.
 *               max    : Maximum number of blocks your buffer can hold.
 *
 * Return      : Error code.
 *               NOS_E_NULL    : Pointer to mem object is NULL.
 *                               OR
 *                               Pointer to buffer array is NULL.
 *               NOS_E_INV_VAL : Size of one block is too small.
 *                               OR
 *                               Buffer array is not aligned.
 *                               OR
 *                               Maximum number of blocks is 0.
 *               NOS_OK        : Mem created with success.
 *
 * Note        : Mem object must be created before using it, else
 *               behavior is undefined. MUST be called one time
 *               ONLY for each mem object.
 */
nOS_Error nOS_MemCreate (nOS_Mem *mem, void *buffer, size_t bsize, uint16_t bmax)
{
    nOS_Error   err;
    uint16_t    i;
    void        **blist;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        err = NOS_E_NULL;
    } else if (mem->e.type != NOS_EVENT_INVALID) {
        err = NOS_E_INV_OBJ;
    } else if (buffer == NULL) {
        err = NOS_E_NULL;
    } else if (bsize < sizeof(void**)) {
        err = NOS_E_INV_VAL;
    }
#if (NOS_MEM_ALIGNMENT > 1)
#if (NOS_MEM_ALIGNMENT >= 4)
    else if ((uint32_t)buffer % NOS_MEM_ALIGNMENT != 0) {
        err = NOS_E_INV_VAL;
    }
#else
    else if ((uint16_t)buffer % NOS_MEM_ALIGNMENT != 0) {
        err = NOS_E_INV_VAL;
    }
#endif
#endif
    else if (bmax == 0) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
#if (NOS_CONFIG_SAFE > 0)
        nOS_EventCreate((nOS_Event*)mem, NOS_EVENT_MEM);
#else
        nOS_EventCreate((nOS_Event*)mem);
#endif
        /* Initialize the single-link list */
        blist = NULL;
        for (i = 0; i < bmax-1; i++) {
            *(void**)buffer = blist;
            blist = (void**)buffer;
            buffer = ((uint8_t*)buffer + bsize);
        }
        *(void**)buffer = blist;
        mem->blist = (void**)buffer;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
        mem->buffer = buffer;
        mem->bsize = bsize;
        mem->bcount = bmax;
        mem->bmax = bmax;
#endif
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

#if (NOS_CONFIG_MEM_DELETE_ENABLE > 0)
nOS_Error nOS_MemDelete (nOS_Mem *mem)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        err = NOS_E_NULL;
    } else if (mem->e.type != NOS_EVENT_MEM) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        mem->blist = NULL;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
        mem->buffer = NULL;
        mem->bsize = 0;
        mem->bcount = 0;
        mem->bmax = 0;
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
        if (nOS_EventDelete((nOS_Event*)mem)) {
            nOS_Sched();
        }
#else
        nOS_EventDelete((nOS_Event*)mem);
#endif
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}
#endif

/*
 * Name        : nOS_MemAlloc
 *
 * Description : Wait on mem object for one block of memory. If no block available,
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
void *nOS_MemAlloc(nOS_Mem *mem, nOS_TickCounter tout)
{
    void    *block;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        block = NULL;
    } else if (mem->e.type != NOS_EVENT_MEM) {
        block = NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (mem->blist != NULL) {
            block = (void*)mem->blist;
            mem->blist = *(void**)block;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
            mem->bcount--;
#endif
        /* Caller can't wait? Try again. */
        } else if (tout == NOS_NO_WAIT) {
            block = NULL;
        } else if (nOS_isrNestingCounter > 0) {
            block = NULL;
        }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        else if (nOS_lockNestingCounter > 0) {
            block = NULL;
        }
#endif
        else if (nOS_runningThread == &nOS_idleHandle) {
            block = NULL;
        } else {
            nOS_runningThread->context = (void*)&block;
            if (nOS_EventWait((nOS_Event*)mem, NOS_THREAD_ALLOC_MEM, tout) != NOS_OK) {
                block = NULL;
            }
        }
        nOS_CriticalLeave();
    }

    return block;
}

/*
 * Name        : nOS_MemFree
 *
 * Description : Free a previously allocated block of memory.
 *
 * Arguments   : mem   : Pointer to mem object.
 *               block : Pointer to previously allocated block.
 *
 * Return      : Error code.
 *               NOS_E_NULL     : Pointer to flag object is NULL.
 *                                OR
 *                                Memory block pointer is NULL.
 *               NOS_E_INV_VAL  : Memory block pointer is out of range.
 *                                OR
 *                                Memory block pointer has been modified.
 *                                since allocation.
 *               NOS_E_OVERFLOW : Too much block has been freed (never happens
 *                                normally, sign of a corruption).
 *                                OR
 *                                Memory block is already free.
 *               NOS_OK         : Memory block has been freed with success.
 *
 * Note        : Do not use memory block after it is freed.
 */
nOS_Error nOS_MemFree(nOS_Mem *mem, void *block)
{
    nOS_Error   err;
    nOS_Thread  *thread;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        err = NOS_E_NULL;
    } else if (mem->e.type != NOS_EVENT_MEM) {
        err = NOS_E_INV_VAL;
    } else if (block == NULL) {
        err = NOS_E_NULL;
    } else
#endif
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
    {
        err = SanityCheck(mem, block);
    }

    if (err == NOS_OK)
#endif
    {
        nOS_CriticalEnter();
        thread = nOS_EventSignal((nOS_Event*)mem, NOS_OK);
        if (thread != NULL) {
            *(void**)thread->context = block;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                nOS_Sched();
            }
#endif
        } else {
            *(void**)block = mem->blist;
            mem->blist = (void**)block;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
            mem->bcount++;
#endif
        }
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

bool nOS_MemIsAvailable (nOS_Mem *mem)
{
    bool    avail;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        avail = false;
    } else if (mem->e.type != NOS_EVENT_MEM) {
        avail = false;
    } else
#endif
    {
        nOS_CriticalEnter();
        avail = (mem->blist != NULL);
        nOS_CriticalLeave();
    }

    return avail;
}
#endif  /* NOS_CONFIG_MEM_ENABLE */

#if defined(__cplusplus)
}
#endif
