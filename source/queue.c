/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

static void Read (nOS_Queue *queue, uint8_t *buffer)
{
    memcpy(buffer, &queue->buffer[queue->r * queue->bsize], queue->bsize);
    queue->r = (queue->r + queue->bsize) % queue->max;
    queue->count--;
}

static void Write (nOS_Queue *queue, uint8_t *buffer)
{
    memcpy(&queue->buffer[queue->w * queue->bsize], buffer, queue->bsize);
    queue->w = (queue->w + queue->bsize) % queue->max;
    queue->count++;
}

nOS_Error nOS_QueueCreate (nOS_Queue *queue, void *buffer, uint16_t bsize, uint16_t max)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (queue == NULL) {
        err = NOS_E_NULL;
    } else if (buffer == NULL) {
        err = NOS_E_NULL;
    } else if (bsize == 0) {
        err = NOS_E_INV_VAL;
    } else if (max == 0) {
        err = NOS_E_INV_VAL;
    } else
#endif
    {
        nOS_CriticalEnter();
        nOS_EventCreate((nOS_Event*)queue);
        queue->buffer = (uint8_t*)buffer;
        queue->bsize = bsize;
        queue->max = max;
        queue->count = 0;
        queue->r = 0;
        queue->w = 0;
        nOS_CriticalLeave();
        err = NOS_OK;
    }

    return err;
}

nOS_Error nOS_QueueRead (nOS_Queue *queue, void *buffer, uint16_t tout)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (queue == NULL) {
        err = NOS_E_NULL;
    } else if (buffer == NULL) {
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
        if (queue->count > 0) {
            Read(queue, (uint8_t*)buffer);
            err = NOS_OK;
        } else if (tout > 0) {
            nOS_runningThread->context = buffer;
            err = nOS_EventWait((nOS_Event*)queue, NOS_READING_QUEUE, tout);
        } else {
            err = NOS_E_EMPTY;
        }
        nOS_CriticalLeave();
    }

    return err;
}

nOS_Error nOS_QueueWrite (nOS_Queue *queue, void *buffer)
{
    nOS_Error   err;

#if NOS_CONFIG_SAFE > 0
    if (queue == NULL) {
        err = NOS_E_NULL;
    } else if (buffer == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        nOS_CriticalEnter();
        if (queue->count < queue->max) {
            Write(queue, (uint8_t*)buffer);
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
