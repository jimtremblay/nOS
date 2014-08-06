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

void PendSV_Handler(void) __attribute__( ( naked ) );

NOS_STACK(isrStack[NOS_CONFIG_ISR_STACK_SIZE]);

void nOS_PortInit(void)
{
    nOS_CriticalEnter();
    /* Copy msp to psp */
    SetPSP(GetMSP());
    /* Set msp to local isr stack */
    SetMSP(((uint32_t)&isrStack[NOS_CONFIG_ISR_STACK_SIZE-1]) & 0xfffffff8UL);
    /* Set current stack to psp and priviledge mode */
    SetCONTROL(GetCONTROL() | 0x00000002UL);
    /* Set PendSV and SysTick to lowest priority */
    *(volatile uint32_t *)0xe000ed20UL |= 0xffff0000UL;
    nOS_CriticalLeave();
}

void nOS_ContextInit(nOS_Thread *thread, stack_t *stack, size_t ssize, void(*func)(void*), void *arg)
{
    stack_t *tos = (stack_t*)((stack_t)(stack + (ssize - 1)) & 0xfffffff8UL);

    *(--tos) = 0x01000000UL;    /* xPSR */
    *(--tos) = (stack_t)func;   /* PC */
    *(--tos) = 0x00000000UL;    /* LR */
    tos     -= 4;               /* R12, R3, R2 and R1 */
    *(--tos) = (stack_t)arg;    /* R0 */
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
                *(volatile uint32_t *)0xe000ed04UL = 0x10000000UL;
            }
        }
    }
    nOS_CriticalLeave();
}

void PendSV_Handler(void)
{
    __asm volatile (
        "CPSID      I                               \n" /* Disable interrupts */
        "ISB                                        \n"
        "                                           \n"
        "MRS        R0,         PSP                 \n" /* Save PSP before doing anything, PendSV_Handler already running on MSP */
        "ISB                                        \n"
        "                                           \n"
        "LDR        R3,         runningThread       \n" /* Get the location of nOS_runningThread */
        "LDR        R2,         [R3]                \n"
        "                                           \n"
        "SUB        R0,         R0,             #32 \n" /* Make space for the remaining registers. */
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
        "LDR        R1,         highPrioThread      \n" /* Copy nOS_highPrioThread to nOS_runningThread */
        "LDR        R0,         [R1]                \n"
        "STR        R0,         [R3]                \n"
        "                                           \n"
        "LDR        R2,         [R1]                \n" /* Restore PSP from nOS_Thread object of high prio thread */
        "LDR        R0,         [R2]                \n"
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
        "CPSIE      I                               \n" /* Enable interrupts */
        "ISB                                        \n"
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
