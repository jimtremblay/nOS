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

void PendSV_Handler(void) __attribute__( ( naked ) );

static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];

void nOS_PortInit(void)
{
#if (NOS_CONFIG_DEBUG > 0)
    uint32_t i;

    for (i = 0; i < NOS_CONFIG_ISR_STACK_SIZE; i++) {
        isrStack[i] = 0xffffffffUL;
    }
#endif

    /* Copy MSP to PSP */
    SetPSP(GetMSP());
    /* Set MSP to local ISR stack */
    SetMSP((uint32_t)&isrStack[NOS_CONFIG_ISR_STACK_SIZE] & 0xfffffff8UL);
    /* Set current stack to PSP and priviledge mode */
    SetCONTROL(GetCONTROL() | 0x00000002UL);
    /* Set PendSV exception to lowest priority */
    *(volatile uint32_t *)0xe000ed20UL |= 0x00ff0000UL;
}

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint32_t)(stack + ssize) & 0xfffffff8UL);
#if (NOS_CONFIG_DEBUG > 0)
    uint32_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xffffffffUL;
    }
#endif

    *(--tos) = 0x01000000UL;    /* xPSR */
    *(--tos) = (nOS_Stack)func; /* PC */
    *(--tos) = 0x00000000UL;    /* LR */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x12121212UL;    /* R12 */
    *(--tos) = 0x03030303UL;    /* R3 */
    *(--tos) = 0x02020202UL;    /* R2 */
    *(--tos) = 0x01010101UL;    /* R1 */
#else
        tos -= 4;               /* R12, R3, R2 and R1 */
#endif
    *(--tos) = (nOS_Stack)arg;  /* R0 */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x11111111UL;    /* R11 */
    *(--tos) = 0x10101010UL;    /* R10 */
    *(--tos) = 0x09090909UL;    /* R9 */
    *(--tos) = 0x08080808UL;    /* R8 */
    *(--tos) = 0x07070707UL;    /* R7 */
    *(--tos) = 0x06060606UL;    /* R6 */
    *(--tos) = 0x05050505UL;    /* R5 */
    *(--tos) = 0x04040404UL;    /* R4 */
#else
        tos -= 8;               /* R11, R10, R9, R8, R7, R6, R5 and R4 */
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

void PendSV_Handler(void)
{
    __asm volatile (
        "MRS        R0,         PSP                 \n" /* Save PSP before doing anything, PendSV_Handler already running on MSP */
        "ISB                                        \n"
        "                                           \n"
        "LDR        R3,         runningThread       \n" /* Get the location of nOS_runningThread */
        "LDR        R2,         [R3]                \n"
        "                                           \n"
        "SUB        R0,         R0,             #32 \n" /* Make space for the remaining registers */
        "                                           \n"
        "STR        R0,         [R2]                \n" /* Save PSP to nOS_Thread object of current running thread */
        "                                           \n"
        "STMIA      R0!,        {R4-R7}             \n" /* Push low registers on thread stack */
        "                                           \n"
        "MOV        R4,         R8                  \n" /* Copy high registers to low registers */
        "MOV        R5,         R9                  \n"
        "MOV        R6,         R10                 \n"
        "MOV        R7,         R11                 \n"
        "STMIA      R0!,        {R4-R7}             \n" /* Push high registers on thread stack */
        "                                           \n"
        "LDR        R1,         highPrioThread      \n" /* Get the location of nOS_highPrioThread */
        "LDR        R2,         [R1]                \n"
        "                                           \n"
        "STR        R2,         [R3]                \n" /* Copy nOS_highPrioThread to nOS_runningThread */
        "                                           \n"
        "LDR        R0,         [R2]                \n" /* Restore PSP from nOS_Thread object of high prio thread */
        "                                           \n"
        "ADD        R0,         R0,             #16 \n" /* Move to the high registers */
        "                                           \n"
        "LDMIA      R0!,        {R4-R7}             \n" /* Pop high registers from thread stack */
        "MOV        R11,        R7                  \n" /* Copy low registers to high registers */
        "MOV        R10,        R6                  \n"
        "MOV        R9,         R5                  \n"
        "MOV        R8,         R4                  \n"
        "                                           \n"
        "MSR        PSP,        R0                  \n" /* Restore PSP to high prio thread stack */
        "ISB                                        \n"
        "                                           \n"
        "SUB        R0,         R0,             #32 \n" /* Go back for the low registers */
        "                                           \n"
        "LDMIA      R0!,        {R4-R7}             \n" /* Pop low registers from thread stack */
        "                                           \n"
        "BX         LR                              \n" /* Return */
        "NOP                                        \n"
        "                                           \n"
        ".align 2                                   \n"
        "runningThread: .word nOS_runningThread     \n"
        "highPrioThread: .word nOS_highPrioThread   \n"
    );
}

#if defined(__cplusplus)
}
#endif
