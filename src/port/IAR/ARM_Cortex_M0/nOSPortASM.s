/*
 * Copyright (c) 2014-2017 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

    RSEG    CODE:CODE(2)
    thumb

    EXTERN nOS_runningThread
    EXTERN nOS_highPrioThread

    PUBLIC PendSV_Handler

PendSV_Handler:
    /* Disable interrupts */
    CPSID       I
    ISB

    /* Save PSP before doing anything, PendSV_Handler already running on MSP */
    MRS         R0,        PSP

    /* Get the location of nOS_runningThread */
    LDR         R3,         =nOS_runningThread
    LDR         R2,         [R3]

    /* Make space for the remaining registers */
    SUBS        R0,         R0,                 #32

    /* Save PSP to nOS_Thread object of current running thread */
    STR         R0,         [R2]

    /* Push low registers on thread stack */
    STMIA       R0!,        {R4-R7}

    /* Copy high registers to low registers */
    MOV         R4,         R8
    MOV         R5,         R9
    MOV         R6,         R10
    MOV         R7,         R11
    /* Push high registers on thread stack */
    STMIA       R0!,        {R4-R7}

    /* Get the location of nOS_highPrioThread */
    LDR         R1,         =nOS_highPrioThread
    LDR         R2,         [R1]

    /* Copy nOS_highPrioThread to nOS_runningThread */
    STR         R2,         [R3]

    /* Restore PSP from nOS_Thread object of high prio thread */
    LDR         R0,         [R2]

    /* Move to the high registers */
    ADDS        R0,         R0,                 #16

    /* Pop high registers from thread stack */
    LDMIA       R0!,        {R4-R7}
    /* Copy low registers to high registers */
    MOV         R11,        R7
    MOV         R10,        R6
    MOV         R9,         R5
    MOV         R8,         R4

    /* Restore PSP to high prio thread stack */
    MSR         PSP,        R0

    /* Go back for the low registers */
    SUBS        R0,         R0,                 #32

    /* Pop low registers from thread stack */
    LDMIA       R0!,        {R4-R7}

    /* Enable interrupts */
    CPSIE       I
    ISB

    /* Return */
    BX          LR

    END
