/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

    PUBLIC ___interrupt_27
    
    EXTERN _nOS_runningThread
    EXTERN _nOS_highPrioThread

    RSEG CODE:CODE(4)
    
___interrupt_27:
    /* Push R15 on ISP (we will use it) */
    PUSH.L      R15
    
    /* Get running thread stack and adjust it to contains PSW, PC and R15 */
    MVFC        USP,                    R15
    SUB         #12,                    R15
    
    /* Set USP to adjusted value */
    MVTC        R15,                    USP
    
    /* Moved pushed registers from ISP to running thread stack */
    MOV.L       [SP],                   [R15]
    MOV.L       4[SP],                  4[R15]
    MOV.L       8[SP],                  8[R15]
    
    /* Adjust ISP (Remove R15, PC and PSW from the stack) */
    ADD         #12,                    SP

    /* At this point, we can continue on USP */
    SETPSW  U

    /* Push all remaining registers to running thread stack */
    PUSHM       R1-R14

    /* Push floating-point status register to running thread stack */
    PUSHC       FPSW

    /* Push accumulator register to running thread stack */
    MVFACHI     R15
    MVFACMI     R14
    SHLL        #16, R14
    PUSHM       R14-R15

    /* Save SP in nOS_runningThread object */
    MOV.L       #_nOS_runningThread,    R15
    MOV.L       [R15],                  R14
    MOV.L       SP,                     [R14]
    
    /* nOS_runningThread = nOS_highPrioThread */
    MOV.L       #_nOS_highPrioThread,   R14
    MOV.L       [R14],                  [R15]
    
    /* Restore SP from nOS_highPrioThread object */
    MOV.L       [R14],                   R15
    MOV.L       [R15],                   SP
    
    /* Pop accumulator register from high prio thread stack */
    POPM       R14-R15
    MVTACLO    R14
    MVTACHI    R15
    
    /* Pop floating-point status register from high prio thread stack */
    POPC       FPSW
    
    /* Pop all registers from high prio thread stack */
    POPM       R1-R15

    /* Return from interrupt (will pop PC and PSW) */
    RTE
    NOP
    NOP

    END
