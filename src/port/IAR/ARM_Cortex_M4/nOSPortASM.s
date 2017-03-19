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
    MRS         R0,         PSP

    /* Get the location of nOS_runningThread */
    LDR         R3,         =nOS_runningThread
    LDR         R2,         [R3]

#ifdef __ARMVFP__
    /* Is thread using FPU? Yes, push high VFP registers to stack */
    TST         LR,         #0x10
    IT          EQ
    VSTMDBEQ    R0!,        {S16-S31}
#endif

    /* Push remaining registers on thread stack */
    STMDB       R0!,        {R4-R11, LR}

    /* Save PSP to nOS_Thread object of current running thread */
    STR         R0,         [R2]

    /* Get the location of nOS_highPrioThread */
    LDR         R1,         =nOS_highPrioThread
    LDR         R2,         [R1]

    /* Copy nOS_highPrioThread to nOS_runningThread */
    STR         R2,         [R3]

    /* Restore PSP from nOS_Thread object of high prio thread */
    LDR         R0,         [R2]

    /* Pop registers from thread stack */
    LDMIA       R0!,        {R4-R11, LR}

#ifdef __ARMVFP__
    /* Is thread using FPU? Yes, pop high VFP registers from stack */
    TST         LR,         #0x10
    IT          EQ
    VLDMIAEQ    R0!,        {S16-S31}
#endif

    /* Restore PSP to high prio thread stack */
    MSR         PSP,        R0

    /* Enable interrupts */
    CPSIE       I
    ISB

    /* Return */
    BX          LR

    END
