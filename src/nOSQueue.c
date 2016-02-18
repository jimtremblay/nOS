/*
 * Copyright (c) 2014-2016 Jim Tremblay
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
static void _Write (nOS_Queue *queue, void *block)
{
    memcpy(&queue->buffer[(size_t)queue->w * (size_t)queue->bsize], block, queue->bsize);
    queue->w = (queue->w + 1) % queue->bmax;
    queue->bcount++;
}

static void _Read (nOS_Queue *queue, void *block)
{
    memcpy(block, &queue->buffer[(size_t)queue->r * (size_t)queue->bsize], queue->bsize);
    queue->r = (queue->r + 1) % queue->bmax;
    queue->bcount--;
}

nOS_Error nOS_QueueCreate (nOS_Queue *queue, void *buffer, uint8_t bsize, nOS_QueueCounter bmax)
{
    nOS_Error       err;
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (bsize == 0) {
        err = NOS_E_INV_VAL;
    } else if ((buffer != NULL) && (bmax == 0)) {
        err = NOS_E_INV_VAL;
    } else if ((buffer == NULL) && (bmax > 0)) {
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
#if (NOS_CONFIG_SAFE > 0)
            nOS_CreateEvent((nOS_Event*)queue, NOS_EVENT_QUEUE);
#else
            nOS_CreateEvent((nOS_Event*)queue);
#endif
            queue->buffer = (uint8_t*)buffer;
            queue->bsize  = bsize;
            queue->bcount = 0;
            queue->bmax   = bmax;
            queue->r      = 0;
            queue->w      = 0;

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
            queue->buffer = NULL;
            queue->bsize  = 0;
            queue->bcount = 0;
            queue->bmax   = 0;
            queue->r      = 0;
            queue->w      = 0;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
            if (nOS_DeleteEvent((nOS_Event*)queue)) {
                nOS_Schedule();
            }
#else
            nOS_DeleteEvent((nOS_Event*)queue);
#endif

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
    } else if (block == NULL) {
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
        {
            /* No chance a thread waiting to read from queue if count is higher than 0 */
            if (queue->bcount > 0) {
                _Read(queue, block);
                /* Check if thread waiting to write in queue */
                thread = nOS_SendEvent((nOS_Event*)queue, NOS_OK);
                if (thread != NULL) {
                    /* Write thread's block in queue */
                    _Write(queue, thread->ext);
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                    if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                        nOS_Schedule();
                    }
#endif
                }
                err = NOS_OK;
            } else if (timeout == NOS_NO_WAIT) {
                err = NOS_E_EMPTY;
            } else if (nOS_isrNestingCounter > 0) {
                err = NOS_E_ISR;
            }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
            else if (nOS_lockNestingCounter > 0) {
                err = NOS_E_LOCKED;
            }
#endif
            else if (nOS_runningThread == &nOS_idleHandle) {
                err = NOS_E_IDLE;
            }
            else {
                nOS_runningThread->ext = block;
                err = nOS_WaitForEvent((nOS_Event*)queue, NOS_THREAD_READING_QUEUE, timeout);
            }
        }
        nOS_LeaveCritical(sr);
    }

    return err;
}

nOS_Error nOS_QueueWrite (nOS_Queue *queue, void *block, nOS_TickCounter timeout)
{
    nOS_Error       err;
    nOS_StatusReg   sr;
    nOS_Thread      *thread;

#if (NOS_CONFIG_SAFE > 0)
    if (queue == NULL) {
        err = NOS_E_INV_OBJ;
    } else if (block == NULL) {
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
        {
            /* If count equal 0, there are chances some threads can wait to read from queue */
            if (queue->bcount == 0) {
                /* Check if thread waiting to read from queue */
                thread = nOS_SendEvent((nOS_Event*)queue, NOS_OK);
                if (thread != NULL) {
                    /* Direct copy between thread's buffers */
                    memcpy(thread->ext, block, queue->bsize);
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                    if ((thread->state == NOS_THREAD_READY) && (thread->prio > nOS_runningThread->prio)) {
                        nOS_Schedule();
                    }
#endif
                    err = NOS_OK;
                } else if (queue->buffer != NULL) {
                    /* No thread waiting to read from queue, then store it */
                    _Write(queue, block);
                    err = NOS_OK;
                } else {
                    /* No thread waiting to consume message, inform producer */
                    err = NOS_E_NO_CONSUMER;
                }
            } else if (queue->bcount < queue->bmax) {
                /* No chance a thread waiting to read from queue if count is higher than 0 */
                _Write(queue, block);
                err = NOS_OK;
            } else if (timeout == NOS_NO_WAIT) {
                err = NOS_E_FULL;
            } else if (nOS_isrNestingCounter > 0) {
                err = NOS_E_ISR;
            }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
            else if (nOS_lockNestingCounter > 0) {
                err = NOS_E_LOCKED;
            }
#endif
            else if (nOS_runningThread == &nOS_idleHandle) {
                /* Main threadv can't wait. */
                err = NOS_E_IDLE;
            } else {
                nOS_runningThread->ext = block;
                err = nOS_WaitForEvent((nOS_Event*)queue, NOS_THREAD_WRITING_QUEUE, timeout);
            }
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
            if (queue->buffer != NULL) {
                empty = (queue->bcount == 0);
            } else {
                empty = true;
            }
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
            if (queue->buffer != NULL) {
                full = (queue->bcount == queue->bmax);
            } else {
                full = false;
            }
        }
        nOS_LeaveCritical(sr);
    }

    return full;
}
#endif  /* NOS_CONFIG_QUEUE_ENABLE */

#ifdef __cplusplus
}
#endif
