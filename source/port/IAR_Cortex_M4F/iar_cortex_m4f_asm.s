/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
 
#if defined(NOS_USE_CONFIG_FILE)
#include "nOSConfig.h"
#else
#define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO      5
#endif

/* __NVIC_PRIO_BITS defined from CMSIS if used */
#if defined(__NVIC_PRIO_BITS)
 #define NOS_NVIC_PRIO_BITS                 __NVIC_PRIO_BITS
#else
 #define NOS_NVIC_PRIO_BITS                 4
#endif

#define NOS_PORT_MAX_UNSAFE_BASEPRI         (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))

    RSEG    CODE:CODE(2)
    thumb
    
    EXTERN nOS_runningThread
    EXTERN nOS_highPrioThread
    
    PUBLIC PendSV_Handler
    
PendSV_Handler:
    /* Set interrupt mask to disable interrupts that use nOS API */
    MOV         R0,         #NOS_PORT_MAX_UNSAFE_BASEPRI
    MSR         BASEPRI,    R0
    ISB

    /* Save PSP before doing anything, PendSV_Handler already running on MSP */
    MRS         R12,        PSP
    ISB
    
    /* Get the location of nOS_runningThread */
    LDR         R3,         =nOS_runningThread
    LDR         R2,         [R3]
    
#if defined(__ARMVFP__)
    /* Is the thread using the VFP ? Yes, push high VFP registers */
    TST         LR,         #0x10
    IT          EQ
    VSTMDBEQ    R12!,       {S16-S31}
#endif
    
    /* Push remaining registers on thread stack */
	STMDB       R12!,       {R4-R11, LR}
    
    /* Save psp to nOS_Thread object of current running thread */
    STR         R12,        [R2]
    
    /* Copy nOS_highPrioThread to nOS_runningThread */
    LDR         R1,         =nOS_highPrioThread
    LDR         R0,         [R1]
    STR         R0,         [R3]
    
    /* Restore psp from nOS_Thread object of high prio thread */
    LDR         R2,         [R1]
    LDR         R12,        [R2]
    
    /* Pop registers from thread stack */
    LDMIA       R12!,       {R4-R11, LR}
    
#if defined(__ARMVFP__)
    /* Is the thread using the VFP ? Yes, pop high VFP registers */
    TST         LR,         #0x10
    IT          EQ
    VLDMIAEQ    R12!,       {S16-S31}
#endif
    
    /* Restore psp to high prio thread stack */
    MSR         PSP,        R12
    ISB
    
    /* Clear interrupt mask to re-enable interrupts */
    MOV         R0,         #0
    MSR         BASEPRI,    R0
    ISB

	BX          LR
    
    /* Not needed in this file */
    /* END */
