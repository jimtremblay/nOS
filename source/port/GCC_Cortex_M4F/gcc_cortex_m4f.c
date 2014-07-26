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

static stack_t isrStack[NOS_CONFIG_ISR_STACK_SIZE] __attribute__ ((aligned (8)));
  
void nOS_PortInit(void)
{
    nOS_CriticalEnter();
    /* Copy msp to psp */
    __asm volatile (
    	"MRS	R0,		MSP							\n"
    	"MSR	PSP,	R0							\n"
    );
    /* Set msp to local isr stack */
    __asm volatile (
    	"MSR	MSP,	%0							\n"
    	:
    	: "r" (((unsigned long)&isrStack[NOS_CONFIG_ISR_STACK_SIZE-1]) & 0xfffffff8)
    );
    /* Set current stack to psp and priviledge mode */
    __asm volatile (
    	"MRS	R0,			CONTROL					\n"
    	"ORRS	R0,			R0,			#2			\n"
    	"MSR	CONTROL,	R1						\n"
    );
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

void PendSV_Handler(void)
{
    /* Set interrupt mask to disable interrupts that use nOS API */
	__asm volatile (
		"MOV		R0,			%0					\n"
		"MSR		BASEPRI,	R0					\n"
		:
		: "I" (NOS_PORT_MAX_UNSAFE_BASEPRI)
	);

    /* Save PSP before doing anything, PendSV_Handler already running on MSP */
	__asm volatile (
		"MRS		R12,		PSP					\n"
		"ISB										\n"
	);

    /* Get the location of nOS_runningThread */
    __asm volatile (
    	"LDR		R3,			=runningThread		\n"
    	"LDR		R2,     	[R3]				\n"
    	"											\n"
    	".align 2									\n"
    	"runningThread: .word nOS_runningThread		\n"
    );

#if defined(__ARM_PCS_VFP)
    /* Is the thread using the VFP ? Yes, push high VFP registers */
    __asm volatile (
    	"TST		LR,			#0x10				\n"
    	"IT			EQ								\n"
    	"VSTMDBEQ	R12!,		{S16-S31}			\n"
    );
#endif

    /* Push remaining registers on thread stack */
	__asm volatile (
		"STMDB		R12!,		{R4-R11, LR}		\n"
	);

    /* Save psp to nOS_Thread object of current running thread */
	__asm volatile (
		"STR		R12,		[R2]				\n"
	);

    /* Copy nOS_highPrioThread to nOS_runningThread */
	__asm volatile (
		"LDR		R1,			=highPrioThread		\n"
		"LDR		R0,			[R1]				\n"
		"STR		R0,			[R3]				\n"
		"											\n"
		".align 2									\n"
		"highPrioThread: .word nOS_highPrioThread	\n"
	);

    /* Restore psp from nOS_Thread object of high prio thread */
	__asm volatile (
		"LDR		R2,			[R1]				\n"
		"LDR		R12,		[R2]				\n"
	);

    /* Pop registers from thread stack */
	__asm volatile (
		"LDMIA		R12!,		{R4-R11, LR}		\n"
	);

#if defined(__ARM_PCS_VFP)
    /* Is the thread using the VFP ? Yes, pop high VFP registers */
	__asm volatile (
		"TST		LR,			#0x10				\n"
		"IT			EQ								\n"
		"VLDMIAEQ	R12!,		{S16-S31}			\n"
	);
#endif

    /* Restore psp to high prio thread stack */
	__asm volatile (
		"MSR		PSP,		R12					\n"
	);

    /* Clear interrupt mask to re-enable interrupts */
	__asm volatile (
		"MOV		R0,			#0					\n"
		"MSR		BASEPRI,	R0					\n"
	);

    /* Return */
	__asm volatile (
		"BX			LR								\n"
	);
}

#if defined(__cplusplus)
}
#endif
