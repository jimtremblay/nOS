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

#if (NOS_CONFIG_QUEUE_ENABLE > 0)
static void _Append (nOS_Queue *queue, void *block)
{
    memcpy(&queue->buffer[(size_t)queue->w * (size_t)queue->bsize], block, queue->bsize);
    queue->w = (queue->w + 1) % queue->bmax;
    queue->bcount++;
}

static void _Prepend (nOS_Queue *queue, void *block)
{
    queue->w = queue->w > 0 ? queue->w-1 : queue->bmax;
    memcpy(&queue->buffer[(size_t)queue->w * (size_t)queue->bsize], block, queue->bsize);
    queue->bcount++;
}

static void _Read (nOS_Queue *queue, void *block)
{
    memcpy(block, &queue->buffer[(size_t)queue->r * (size_t)queue->bsize], queue->bsize);
    queue->r = (queue->r + 1) % queue->bmax;
    queue->bcount--;
}

static void _Flush (nOS_Queue *queue)
{
    queue->r = 0;
    queue->w = 0;
    queue->bcount = 0;
}

static nOS_Error _WriteQueue (nOS_Queue *queue, void *block, nOS_TickCounter timeout, bool front)
{
    nOS_Error           err;
    nOS_StatusReg       sr;
    nOS_Thread          *thread;
    nOS_QueueContext    ctx;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (block == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        /* If count equal 0, there are chances some threads can wait to read from queue */
        if (queue->bcount == 0) {
            /* Check if thread waiting to read from queue */
            thread = nOS_SendEvent((nOS_Event*)queue, NOS_OK);
            if (thread != NULL) {
                /* Direct copy between thread's buffers */
                memcpy(thread->ext, block, queue->bsize);
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                /* Verify if a highest prio thread is ready to run */
                nOS_Schedule();
#endif
                err = NOS_OK;
            }
            else if (queue->buffer != NULL) {
                /* No thread waiting to read from queue, then store it */
                _Append(queue, block);
                err = NOS_OK;
            }
            else {
                /* No thread waiting to consume message, inform producer */
                err = NOS_E_NO_CONSUMER;
            }
        }
        else if (queue->bcount < queue->bmax) {
            /* No chance a thread waiting to read from queue if count is higher than 0 */
            if (front) {
                _Prepend(queue, block);
            } else {
                _Append(queue, block);
            }
            err = NOS_OK;
        }
        else if (timeout == NOS_NO_WAIT) {
            err = NOS_E_FULL;
        }
        else {
            ctx.block = block;
            ctx.front = front;
            nOS_runningThread->ext = &ctx;
            err = nOS_WaitForEvent((nOS_Event*)queue,
                                   NOS_THREAD_WRITING_QUEUE
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0)
                                  ,timeout
#elif (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                                  ,NOS_WAIT_INFINITE
#endif
                                  );
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

/* Can create queue with block count at 0 to use it as pipe object (no buffer needed). */
nOS_Error nOS_QueueCreate (nOS_Queue *queue, void *buffer, uint8_t bsize, nOS_QueueCounter bmax)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (bsize == 0) {
        err = NOS_E_INV_VAL;
    }
    else if ((buffer != NULL) && (bmax == 0)) {
        err = NOS_E_INV_VAL;
    }
    else if ((buffer == NULL) && (bmax > 0)) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_INVALID) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            nOS_CreateEvent((nOS_Event*)queue
#if (NOS_CONFIG_SAFE > 0)
                           ,NOS_EVENT_QUEUE
#endif
                           );
            queue->buffer = (uint8_t*)buffer;
            queue->bsize  = bsize;
            queue->bmax   = bmax;
            _Flush(queue);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

#if (NOS_CONFIG_QUEUE_DELETE_ENABLE > 0)
nOS_Error nOS_QueueDelete (nOS_Queue *queue)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            _Flush(queue);
            queue->buffer = NULL;
            queue->bsize  = 0;
            queue->bmax   = 0;
            nOS_DeleteEvent((nOS_Event*)queue);

            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}
#endif

nOS_Error nOS_QueueRead (nOS_Queue *queue, void *block, nOS_TickCounter timeout)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_Thread      *thread;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (block == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        /* No chance a thread waiting to read from queue if count is higher than 0 */
        if (queue->bcount > 0) {
            _Read(queue, block);
            /* Check if thread waiting to write in queue */
            thread = nOS_SendEvent((nOS_Event*)queue, NOS_OK);
            if (thread != NULL) {
                /* Write thread's block in queue */
                if (((nOS_QueueContext*)thread->ext)->front) {
                    _Prepend(queue, ((nOS_QueueContext*)thread->ext)->block);
                } else {
                    _Append(queue, ((nOS_QueueContext*)thread->ext)->block);
                }
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                /* Verify if a highest prio thread is ready to run */
                nOS_Schedule();
#endif
            }
            err = NOS_OK;
        }
        else if (timeout == NOS_NO_WAIT) {
            err = NOS_E_EMPTY;
        }
        else {
            nOS_runningThread->ext = block;
            err = nOS_WaitForEvent((nOS_Event*)queue,
                                   NOS_THREAD_READING_QUEUE
#if (NOS_CONFIG_WAITING_TIMEOUT_ENABLE > 0)
                                  ,timeout
#elif (NOS_CONFIG_SLEEP_ENABLE > 0) || (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
                                  ,NOS_WAIT_INFINITE
#endif
                                  );
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_QueuePeek (nOS_Queue *queue, void *block)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    }
    else if (block == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        if (queue->bcount > 0) {
            memcpy(block, &queue->buffer[(size_t)queue->r * (size_t)queue->bsize], queue->bsize);
            err = NOS_OK;
        }
        else {
            err = NOS_E_EMPTY;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_QueueWrite (nOS_Queue *queue, void *block, nOS_TickCounter timeout)
{
    return _WriteQueue(queue, block, timeout, false);
}

nOS_Error nOS_QueueWriteInFront (nOS_Queue *queue, void *block, nOS_TickCounter timeout)
{
    return _WriteQueue(queue, block, timeout, true);
}

nOS_Error nOS_QueueFlush (nOS_Queue *queue, nOS_QueueCallback callback)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            err = NOS_E_INV_OBJ;
        } else
#endif
        {
            /* If blocks are stored in queue ... */
            if (queue->bcount > 0) {
                if (callback != NULL) {
                    /* ... call user's callback for every stored block */
                    while (queue->bcount > 0) {
                        callback(queue, &queue->buffer[(size_t)queue->r * (size_t)queue->bsize]);
                        queue->r = (queue->r + 1) % queue->bmax;
                        queue->bcount--;
                    }
                }
                else {
                    _Flush(queue);
                }
                /* maybe some threads are waiting to write in queue */
                nOS_BroadcastEvent((nOS_Event*)queue, NOS_E_FLUSHED);
            }
            err = NOS_OK;
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

bool nOS_QueueIsEmpty (nOS_Queue *queue)
{
    nOS_StatusReg   sr;
    bool            empty;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        empty = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            empty = false;
        } else
#endif
        {
            empty = queue->buffer != NULL ? queue->bcount == 0 : true;
        }
        nOS_LeaveCritical(sr);
    }

    return empty;
}

bool nOS_QueueIsFull (nOS_Queue *queue)
{
    nOS_StatusReg   sr;
    bool            full;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        full = false;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            full = false;
        } else
#endif
        {
            full = queue->buffer != NULL ?
                        queue->bcount == queue->bmax :
                        nOS_GetHeadOfList(&queue->e.waitList) != NULL ?  /* A thread can be ready to consume message */
                            false :
                            true;
        }
        nOS_LeaveCritical(sr);
    }

    return full;
}

nOS_QueueCounter nOS_QueueGetCount (nOS_Queue *queue)
{
    nOS_StatusReg       sr;
    nOS_QueueCounter    bcount;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        bcount = 0;
    } else
#endif
    {
        nOS_EnterCritical(sr);
#if (NOS_CONFIG_SAFE > 0)
        if (queue->e.type != NOS_EVENT_QUEUE) {
            bcount = 0;
        } else
#endif
        {
            bcount = queue->bcount;
        }
        nOS_LeaveCritical(sr);
    }

    return bcount;
}
#endif  /* NOS_CONFIG_QUEUE_ENABLE */

#ifdef __cplusplus
}
#endif
