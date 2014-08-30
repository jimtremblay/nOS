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

static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];

void nOS_PortInit(void)
{
    nOS_CriticalEnter();
    /* Copy msp to psp */
    __set_PSP(__get_MSP());
    /* Set msp to local isr stack */
    __set_MSP(((unsigned long)&isrStack[NOS_CONFIG_ISR_STACK_SIZE-1]) & 0xfffffff8UL);
    /* Set current stack to psp and priviledge mode */
    __set_CONTROL(__get_CONTROL() | 0x00000002UL);
    /* Set PendSV exception to lowest priority */
    *(volatile uint32_t *)0xe000ed20UL |= 0x00ff0000UL;
    nOS_CriticalLeave();
}

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint32_t)(stack + (ssize - 1)));
    
    /* Just to know if the thread has overflow his stack */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0xffffffffUL;
    *tos-- = 0xffffffffUL;
#endif
    
    /* 
     * We need to be aligned to 4 bytes boundary before multiple push
     * to be alligned to 8 bytes boundary at the end.
     */
    if (((uint32_t)tos & 0x00000007UL) == 0x00000000UL) {
        tos--;
    }

#if defined(__ARMVFP__)
    *tos-- = 0x00000000UL;      /* FPSCR */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x15151515UL;      /* S15 */
    *tos-- = 0x14141414UL;      /* S14 */
    *tos-- = 0x13131313UL;      /* S13 */
    *tos-- = 0x12121212UL;      /* S12 */
    *tos-- = 0x11111111UL;      /* S11 */
    *tos-- = 0x10101010UL;      /* S10 */
    *tos-- = 0x09090909UL;      /* S9 */
    *tos-- = 0x08080808UL;      /* S8 */
    *tos-- = 0x07070707UL;      /* S7 */
    *tos-- = 0x06060606UL;      /* S6 */
    *tos-- = 0x05050505UL;      /* S5 */
    *tos-- = 0x04040404UL;      /* S4 */
    *tos-- = 0x03030303UL;      /* S3 */
    *tos-- = 0x02020202UL;      /* S2 */
    *tos-- = 0x01010101UL;      /* S1 */
    *tos-- = 0x00000000UL;      /* S0 */
#else
    tos   -= 16;                /* S15 to S0 */
#endif
#endif
    *tos-- = 0x01000000UL;      /* xPSR */
    *tos-- = (nOS_Stack)func;   /* PC */
    *tos-- = 0x00000000UL;      /* LR */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x12121212UL;      /* R12 */
    *tos-- = 0x03030303UL;      /* R3 */
    *tos-- = 0x02020202UL;      /* R2 */
    *tos-- = 0x01010101UL;      /* R1 */
#else
    tos     -= 4;               /* R12, R3, R2 and R1 */
#endif
    *tos-- = (nOS_Stack)arg;    /* R0 */
#if defined(__ARMVFP__)
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x31313131UL;      /* S31 */
    *tos-- = 0x30303030UL;      /* S30 */
    *tos-- = 0x29292929UL;      /* S29 */
    *tos-- = 0x28282828UL;      /* S28 */
    *tos-- = 0x27272727UL;      /* S27 */
    *tos-- = 0x26262626UL;      /* S26 */
    *tos-- = 0x25252525UL;      /* S25 */
    *tos-- = 0x24242424UL;      /* S24 */
    *tos-- = 0x23232323UL;      /* S23 */
    *tos-- = 0x22222222UL;      /* S22 */
    *tos-- = 0x21212121UL;      /* S21 */
    *tos-- = 0x20202020UL;      /* S20 */
    *tos-- = 0x19191919UL;      /* S19 */
    *tos-- = 0x18181818UL;      /* S18 */
    *tos-- = 0x17171717UL;      /* S17 */
    *tos-- = 0x16161616UL;      /* S16 */
#else
    tos   -= 16;                /* S31 to S16 */
#endif
#endif
#if defined(__ARMVFP__)
    *tos-- = 0xffffffedUL;      /* EXC_RETURN (Thread mode, use FP state from PSP, Thread use PSP */
#else
    *tos-- = 0xfffffffdUL;      /* EXC_RETURN (Thread mode, don't use FP state, Thread use PSP */
#endif
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x11111111UL;      /* R11 */
    *tos-- = 0x10101010UL;      /* R10 */
    *tos-- = 0x09090909UL;      /* R9 */
    *tos-- = 0x08080808UL;      /* R8 */
    *tos-- = 0x07070707UL;      /* R7 */
    *tos-- = 0x06060606UL;      /* R6 */
    *tos-- = 0x05050505UL;      /* R5 */
    *tos   = 0x04040404UL;      /* R4 */
#else
    tos     -= 8;               /* R11, R10, R9, R8, R7, R6, R5 and R4 */
#endif

    thread->stackPtr = tos;
}

void nOS_IsrEnter (void)
{
    nOS_CriticalEnter();
    nOS_isrNestingCounter++;
    nOS_CriticalLeave();
}

void nOS_IsrLeave (void)
{
    nOS_CriticalEnter();
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = SchedHighPrio();
            if (nOS_runningThread != nOS_highPrioThread) {
                *(volatile uint32_t *)0xe000ed04UL = 0x10000000UL;
            }
        }
    }
    nOS_CriticalLeave();
}

#if defined(__cplusplus)
}
#endif
