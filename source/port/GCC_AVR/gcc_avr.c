/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define NOS_PRIVATE
#include "nOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(NOS_CONFIG_ISR_STACK_SIZE)
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #else
  static stack_t isrStack[NOS_CONFIG_ISR_STACK_SIZE];
 #endif
#endif

void nOS_ContextInit(nOS_Thread *thread, stack_t *stack, size_t ssize, void(*func)(void*), void *arg)
{
    /* Stack grow from high to low address */
    stack_t *tos = stack + (ssize - 1);

    /* Simulate a call to thread function */
    *tos-- = (uint8_t)((uint16_t)func);
    *tos-- = (uint8_t)((uint16_t)func >> 8);
#ifdef __AVR_3_BYTE_PC__
    *tos-- = 0x00;                                  /* Always set high part of address to 0 */
#endif

    /* Simulate a call of nOS_PushContext */
     tos  -= 1;                                     /* R0 */
    *tos-- = 0x80;                                  /* SREG: Interrupts enabled */
#ifdef __AVR_HAVE_RAMPZ__
     tos  -= 1;                                     /* RAMPZ */
#endif
#ifdef __AVR_3_BYTE_PC__
     tos  -= 1;                                     /* EIND */
#endif
    *tos-- = 0;                                     /* R1 always 0 */
     tos  -= 22;                                    /* R2 to R23 */
    *tos-- = (uint8_t)((uint16_t)arg);              /* R24: arg LSB */
    *tos-- = (uint8_t)((uint16_t)arg >> 8);         /* R25: arg MSB */
     tos  -= 6;                                     /* R26 to R31 */

    thread->stackPtr = tos;
}

/* Absolutely need a naked function because function call push the return address on the stack */
void nOS_ContextSwitch(void)
{
    nOS_ContextPush();
    nOS_runningThread->stackPtr = (uint8_t*)SP;
    nOS_runningThread = nOS_highPrioThread;
    SP = (int)nOS_runningThread->stackPtr;
    nOS_ContextPop();
    asm volatile("ret");
}

stack_t *nOS_IsrEnter (stack_t *sp)
{
    if (nOS_isrNestingCounter == 0) {
        nOS_runningThread->stackPtr = sp;
        #if defined(NOS_CONFIG_ISR_STACK_SIZE)
        sp = &isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
        #else
        sp = nOS_mainThread.stackPtr;
        #endif
    }
    nOS_isrNestingCounter++;

    return sp;
}

stack_t *nOS_IsrLeave (stack_t *sp)
{
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
        if (nOS_lockNestingCounter == 0) {
            nOS_highPrioThread = SchedHighPrio();
            nOS_runningThread = nOS_highPrioThread;
        }
        sp = nOS_runningThread->stackPtr;
    }

    return sp;
}

#ifdef __cplusplus
}
#endif
