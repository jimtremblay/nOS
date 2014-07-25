/*
 * Copyright (c) 2014 Jim Tremblay
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
    /* Save psp before doing anything, handler already running on msp */
    mrs r12, psp
    isb
    
    /* Get the location of nOS_runningThread */
    ldr r3, =nOS_runningThread
    ldr r2, [r3]
    
#if defined(__ARMVFP__)
    /* Is the thread using the VFP ? Yes, push high VFP registers */
    tst lr, #0x10
    it eq
    vstmdbeq r12!, {s16-s31}
#endif
    
    /* Push remaining registers on thread stack */
	stmdb r12!, {r4-r11, lr}
    
    /* Save psp to nOS_Thread object of current running thread */
    str r12, [r2]
    
    /* Copy nOS_highPrioThread to nOS_runningThread */
    ldr r1, =nOS_highPrioThread
    ldr r0, [r1]
    str r0, [r3]
    
    /* Restore psp from nOS_Thread object of high prio thread */
    ldr r2, [r1]
    ldr r12, [r2]
    
    /* Pop registers from thread stack */
    ldmia r12!, {r4-r11, lr}
    
#if defined(__ARMVFP__)
    /* Is the thread using the VFP ? Yes, pop high VFP registers */
    tst lr, #0x10
    it eq
    vldmiaeq r12!, {s16-s31}
#endif
    
    /* Restore psp to high prio thread stack */
    msr psp, r12

	bx lr
    
    /* Not needed in this file */
    /* END */
