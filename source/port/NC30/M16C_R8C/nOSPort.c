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

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint16_t)(stack + ssize) & 0xFFFE);
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFFFF;
    }
#endif

    /* FLG = 0x0C0 = interrupts enabled, USP stack pointer */
    *(--tos) = (((uint32_t)entry >> 8) & 0x0F00) | 0x00C0;
    *(--tos) = (nOS_Stack)entry;

#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0xEEEE;         /* FB */
    *(--tos) = 0xCCCC;         /* SB */
    *(--tos) = 0xBBBB;         /* A1 */
    *(--tos) = 0xAAAA;         /* A0 */
    *(--tos) = 0x3333;         /* R3 */
    *(--tos) = 0x2222;         /* R2 */
#else
        tos -= 6;              /* FB, SB, A1, A0, R3, R2 */
#endif
    *(--tos) = (nOS_Stack)arg; /* R1 */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x1111;         /* R0 */
#else
        tos -= 1;              /* R0 */
#endif

    thread->stackPtr = tos;
}

void nOS_EnterIsr (void)
{
    nOS_EnterCritical();
    nOS_isrNestingCounter++;
    nOS_LeaveCritical();
}

bool nOS_LeaveIsr (void)
{
    bool    swctx = false;

    nOS_EnterCritical();
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = nOS_FindHighPrioThread();
            if (nOS_runningThread != nOS_highPrioThread) {
                swctx = true;
            }
        }
    }
    nOS_LeaveCritical();

    return swctx;
}

#ifdef __cplusplus
}
#endif
