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

#if (NOS_CONFIG_MEM_ENABLE > 0)
nOS_Error nOS_MemCreate (nOS_Mem *mem, void *buffer, nOS_MemSize bsize, nOS_MemCounter bmax)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_MemCounter  i;
    void            **blist;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (buffer == NULL) {
        err = NOS_E_NULL;
    } else if (bsize < sizeof(void**)) {
        err = NOS_E_INV_VAL;
    }
 #if (NOS_MEM_ALIGNMENT > 1)
  #if (NOS_MEM_POINTER_WIDTH == 8)
    else if ((uint64_t)buffer % NOS_MEM_ALIGNMENT != 0)
  #elif (NOS_MEM_POINTER_WIDTH == 4)
    else if ((uint32_t)buffer % NOS_MEM_ALIGNMENT != 0)
  #elif (NOS_MEM_POINTER_WIDTH == 2)
    else if ((uint16_t)buffer % NOS_MEM_ALIGNMENT != 0)
  #endif
    {
        err = NOS_E_INV_VAL;
    }
 #endif
    else if (bmax == 0) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (mem->e.type != NOS_EVENT_INVALID) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
#if (NOS_CONFIG_SAFE > 0)
            nOS_CreateEvent((nOS_Event*)mem, NOS_EVENT_MEM);
#else
            nOS_CreateEvent((nOS_Event*)mem);
#endif
            /* Initialize the single-link list */
            blist = NULL;
            for (i = 0; i < bmax-1; i++) {
                *(void**)buffer = blist;
                blist = (void**)buffer;
                buffer = (void*)((uint8_t*)buffer + bsize);
            }
            *(void**)buffer = blist;
            mem->blist  = (void**)buffer;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
            mem->buffer = buffer;
            mem->bsize  = bsize;
            mem->bcount = bmax;
            mem->bmax   = bmax;
#endif

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_MEM_DELETE_ENABLE > 0)
nOS_Error nOS_MemDelete (nOS_Mem *mem)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (mem->e.type != NOS_EVENT_MEM) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            mem->blist  = NULL;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
            mem->buffer = NULL;
            mem->bsize  = 0;
            mem->bcount = 0;
            mem->bmax   = 0;
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (nOS_DeleteEvent((nOS_Event*)mem)) {
                nOS_Schedule();
            }
#else
            nOS_DeleteEvent((nOS_Event*)mem);
#endif

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

void *nOS_MemAlloc(nOS_Mem *mem, nOS_TickCounter timeout)
{
    nOS_StatusReg   sr;
    void            *block = NULL;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        block = NULL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (mem->e.type != NOS_EVENT_MEM) {
            block = NULL;
        } else
#endif
        {
            if (mem->blist != NULL) {
                block = (void*)mem->blist;
                mem->blist = *(void***)block;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
                mem->bcount--;
#endif
            } else if (timeout == NOS_NO_WAIT) {
                /* Caller can't wait? Try again. */
                block = NULL;
            } else if (nOS_isrNestingCounter > 0) {
                /* Can't wait from ISR */
                block = NULL;
            }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
            else if (nOS_lockNestingCounter > 0) {
                /* Can't switch context when scheduler is locked */
                block = NULL;
            }
#endif
            else if (nOS_runningThread == &nOS_idleHandle) {
                /* Main thread can't wait */
                block = NULL;
            } else {
                nOS_runningThread->ext = (void*)&block;
                nOS_WaitForEvent((nOS_Event*)mem, NOS_THREAD_ALLOC_MEM, timeout);
            }
        }
        nOS_LeaveCritical(sr);
    }

    return block;
}

nOS_Error nOS_MemFree(nOS_Mem *mem, void *block)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_Thread      *thread;

#if (NOS_CONFIG_SAFE > 0)
    err = NOS_OK;
    if (mem == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (block == NULL) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (mem->e.type != NOS_EVENT_MEM) {
            err = NOS_E_INV_OBJ;
        } else
 #if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
        {
            err = nOS_MemSanityCheck(mem, block);
        }
        if (err == NOS_OK)
 #endif
#endif
        {
            thread = nOS_SendEvent((nOS_Event*)mem, NOS_OK);
            if (thread != NULL) {
                *(void**)thread->ext = block;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                    nOS_Schedule();
                }
#endif
            } else {
                *(void**)block = mem->blist;
                mem->blist = (void**)block;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
                mem->bcount++;
#endif
            }

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

bool nOS_MemIsAvailable (nOS_Mem *mem)
{
    nOS_StatusReg   sr;
    bool            avail;

#if (NOS_CONFIG_SAFE > 0)
    if (mem == NULL) {
        avail = false;
    } else if (mem->e.type != NOS_EVENT_MEM) {
        avail = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (mem->e.type != NOS_EVENT_MEM) {
            avail = false;
        } else
#endif
        {
            avail = (mem->blist != NULL);
        }
        nOS_LeaveCritical(sr);
    }

    return avail;
}

 #if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
nOS_Error nOS_MemSanityCheck (nOS_Mem *mem, void *block)
{
    nOS_Error   err;
    void        *p;

    if (block < mem->buffer) {
        /* Memory block pointer is out of range. */
        err = NOS_E_INV_VAL;
    } else if (block >= (void*)((uint8_t*)mem->buffer + (mem->bsize * mem->bmax))) {
        /* Memory block pointer is out of range. */
        err = NOS_E_INV_VAL;
    } else if ((nOS_MemSize)((uint8_t*)block - (uint8_t*)mem->buffer) % mem->bsize != 0) {
        /* Memory block pointer is not a multiple of block size. */
        err = NOS_E_INV_VAL;
    } else if (mem->bcount == mem->bmax) {
        /* All blocks are already free. */
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
 #endif
#endif  /* NOS_CONFIG_MEM_ENABLE */

#ifdef __cplusplus
}
#endif
