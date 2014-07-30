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

NOS_STACK(isrStack[NOS_CONFIG_ISR_STACK_SIZE]);
  
void nOS_PortInit(void)
{
    nOS_CriticalEnter();
    /* Copy msp to psp */
    __set_PSP(__get_MSP());
    /* Set msp to local isr stack */
    __set_MSP(((unsigned long)&isrStack[NOS_CONFIG_ISR_STACK_SIZE-1]) & 0xfffffff8);
    /* Set current stack to psp and priviledge mode */
    __set_CONTROL(__get_CONTROL() | 0x00000002UL);
    /* Set PendSV and SysTick to lowest priority */
    *(volatile uint32_t *)0xe000ed20 |= 0xffff0000;
    nOS_CriticalLeave();
}

void nOS_ContextInit(nOS_Thread *thread, stack_t *stack, size_t ssize, void(*func)(void*), void *arg)
{
    stack_t *tos = (stack_t*)((stack_t)(stack + (ssize - 1)) & 0xfffffff8);
    
	*(--tos) = 0x01000000;      /* xPSR */
    *(--tos) = (stack_t)func;   /* PC */
    *(--tos) = 0x00000000;      /* LR */
    tos     -= 4;               /* R12, R3, R2 and R1 */
    *(--tos) = (stack_t)arg;    /* R0 */
    *(--tos) = 0xfffffffd;      /* EXC_RETURN */
    tos     -= 8;               /* R11, R10, R9, R8, R7, R6, R5 and R4 */
    
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
        if (nOS_lockNestingCounter == 0) {
            nOS_highPrioThread = SchedHighPrio();
            if (nOS_runningThread != nOS_highPrioThread) {
                *(volatile uint32_t *)0xe000ed04 = 0x10000000;
            }
        }
    }
    nOS_CriticalLeave();
}

#if defined(__cplusplus)
}
#endif
