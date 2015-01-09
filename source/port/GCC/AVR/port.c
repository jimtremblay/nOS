/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(NOS_CONFIG_ISR_STACK_SIZE)
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #else
  static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];
 #endif
#endif

void nOS_PortInit(void)
{
#if defined(NOS_CONFIG_ISR_STACK_SIZE) && (NOS_CONFIG_DEBUG > 0)
    uint16_t i;

    for (i = 0; i < NOS_CONFIG_ISR_STACK_SIZE; i++) {
        isrStack[i] = 0xff;
    }
#endif
}

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack *tos = stack + (ssize - 1);
#if (NOS_CONFIG_DEBUG > 0)
    uint16_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xff;
    }
#endif

    /* Simulate a call to thread function */
    *tos-- = (nOS_Stack)((uint16_t)entry);
    *tos-- = (nOS_Stack)((uint16_t)entry >> 8);
#if defined(__AVR_3_BYTE_PC__)
    *tos-- = 0x00;                                  /* Always set high part of address to 0 */
#endif

    /* Simulate a call of nOS_PushContext */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x01;                                  /* R0 */
#else
     tos  -= 1;                                     /* R0 */
#endif
    *tos-- = 0x80;                                  /* SREG: Interrupts enabled */
#if defined(__AVR_HAVE_RAMPZ__)
     tos  -= 1;                                     /* RAMPZ */
#endif
#if defined(__AVR_3_BYTE_PC__)
     tos  -= 1;                                     /* EIND */
#endif
    *tos-- = 0x00;                                  /* R1 always 0 */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x02;                                  /* R2 */
    *tos-- = 0x03;                                  /* R3 */
    *tos-- = 0x04;                                  /* R4 */
    *tos-- = 0x05;                                  /* R5 */
    *tos-- = 0x06;                                  /* R6 */
    *tos-- = 0x07;                                  /* R7 */
    *tos-- = 0x08;                                  /* R8 */
    *tos-- = 0x09;                                  /* R9 */
    *tos-- = 0x10;                                  /* R10 */
    *tos-- = 0x11;                                  /* R11 */
    *tos-- = 0x12;                                  /* R12 */
    *tos-- = 0x13;                                  /* R13 */
    *tos-- = 0x14;                                  /* R14 */
    *tos-- = 0x15;                                  /* R15 */
    *tos-- = 0x16;                                  /* R16 */
    *tos-- = 0x17;                                  /* R17 */
    *tos-- = 0x18;                                  /* R18 */
    *tos-- = 0x19;                                  /* R19 */
    *tos-- = 0x20;                                  /* R20 */
    *tos-- = 0x21;                                  /* R21 */
    *tos-- = 0x22;                                  /* R22 */
    *tos-- = 0x23;                                  /* R23 */
#else
     tos  -= 22;                                    /* R2 to R23 */
#endif
    *tos-- = (nOS_Stack)((uint16_t)arg);            /* R24: arg LSB */
    *tos-- = (nOS_Stack)((uint16_t)arg >> 8);       /* R25: arg MSB */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x26;                                  /* R26 */
    *tos-- = 0x27;                                  /* R27 */
    *tos-- = 0x28;                                  /* R28 */
    *tos-- = 0x29;                                  /* R29 */
    *tos-- = 0x30;                                  /* R30 */
    *tos-- = 0x31;                                  /* R31 */
#else
     tos  -= 6;                                     /* R26 to R31 */
#endif

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

nOS_Stack *nOS_IsrEnter (nOS_Stack *sp)
{
    /* Interrupts already disabled when entering in ISR */
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

nOS_Stack *nOS_IsrLeave (nOS_Stack *sp)
{
    /* Interrupts already disabled before leaving ISR */
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = SchedHighPrio();
            nOS_runningThread = nOS_highPrioThread;
        }
        sp = nOS_runningThread->stackPtr;
    }

    return sp;
}

#if defined(__cplusplus)
}
#endif
