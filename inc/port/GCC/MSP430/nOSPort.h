/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <msp430.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NOS_UNUSED(v)               (void)v

#if (__MSP430X_LARGE__ > 0)
 typedef uint32_t                   nOS_Stack;
#else
 typedef uint16_t                   nOS_Stack;
#endif
#define NOS_16_BITS_SCHEDULER
#define NOS_MEM_ALIGNMENT           __SIZEOF_POINTER__
#define NOS_MEM_POINTER_WIDTH       __SIZEOF_POINTER__
typedef uint16_t                    nOS_StatusReg;

#ifdef NOS_CONFIG_ISR_STACK_SIZE
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
 #endif
#endif

#if (__MSP430X_LARGE__ > 0)
 #define    PUSH_X              "pushx.a"
 #define    POP_X               "popx.a"
 #define    PUSHM_X             "pushm.a"
 #define    POPM_X              "popm.a"
 #define    MOV_X               "movx.a"
#else
 #define    PUSH_X              "push.w"
 #define    POP_X               "pop.w"
 #define    PUSHM_X             "pushm.w"
 #define    POPM_X              "popm.w"
 #define    MOV_X               "mov.w"
#endif

#if (__MSP430X_LARGE__ > 0)
 #define    CALL_X              "calla"
 #define    RET_X               "reta"
#else
 #define    CALL_X              "call"
 #define    RET_X               "ret"
#endif

#define PUSH_SR                 "push.w     sr"
#define POP_SR                  "pop.w      sr"

#if (__MSP430X__ > 0)
 #define PUSH_CONTEXT            PUSHM_X"   #12,    r15"
 #define POP_CONTEXT             POPM_X"    #12,    r15"
#else
 #define PUSH_CONTEXT           "push.w     r15             \n"                 \
                                "push.w     r14             \n"                 \
                                "push.w     r13             \n"                 \
                                "push.w     r12             \n"                 \
                                "push.w     r11             \n"                 \
                                "push.w     r10             \n"                 \
                                "push.w     r9              \n"                 \
                                "push.w     r8              \n"                 \
                                "push.w     r7              \n"                 \
                                "push.w     r6              \n"                 \
                                "push.w     r5              \n"                 \
                                "push.w     r4"
 #define POP_CONTEXT            "pop.w      r4              \n"                 \
                                "pop.w      r5              \n"                 \
                                "pop.w      r6              \n"                 \
                                "pop.w      r7              \n"                 \
                                "pop.w      r8              \n"                 \
                                "pop.w      r9              \n"                 \
                                "pop.w      r10             \n"                 \
                                "pop.w      r11             \n"                 \
                                "pop.w      r12             \n"                 \
                                "pop.w      r13             \n"                 \
                                "pop.w      r14             \n"                 \
                                "pop.w      r15"
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_state();                                           \
        __disable_interrupt();                                                  \
        __no_operation();                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        __set_interrupt_state(sr);                                              \
        __no_operation();                                                       \
    } while (0)

nOS_Stack*  nOS_EnterIsr        (nOS_Stack *sp);
nOS_Stack*  nOS_LeaveIsr        (nOS_Stack *sp);

#define NOS_ISR(vect)                                                           \
void vect##_ISR_L2(void) __attribute__ ((naked));                               \
inline void vect##_ISR_L3(void) __attribute__ ((always_inline));                \
void __attribute__ ((interrupt(vect), naked)) vect##_ISR(void)                  \
{                                                                               \
    asm volatile (                                                              \
         CALL_X"    #"NOS_STR(vect##_ISR_L2)"   \n"                             \
        "reti                                   \n"                             \
    );                                                                          \
}                                                                               \
void vect##_ISR_L2(void)                                                        \
{                                                                               \
    asm volatile (                                                              \
         PUSH_SR"                               \n"                             \
        "                                       \n"                             \
        /* Push all registers to running thread stack */                        \
         PUSH_CONTEXT"                          \n"                             \
        "                                       \n"                             \
        /* Switch to isr stack if isr nesting counter is zero */                \
        "mov.w      sp,                 r12     \n"                             \
        CALL_X"     #nOS_EnterIsr               \n"                             \
        "mov.w      r12,                sp      \n"                             \
    );                                                                          \
    vect##_ISR_L3();                                                            \
    __disable_interrupt();                                                      \
    __no_operation();                                                           \
    asm volatile (                                                              \
        /* Switch to high prio thread stack if isr nesting counter reach zero */\
        "mov.w      sp,                 r12     \n"                             \
        CALL_X"     #nOS_LeaveIsr               \n"                             \
        "mov.w      r12,                sp      \n"                             \
        "                                       \n"                             \
        /* Pop all registers from high prio thread stack */                     \
         POP_CONTEXT"                           \n"                             \
        "                                       \n"                             \
         POP_SR"                                \n"                             \
        "                                       \n"                             \
         RET_X"                                 \n"                             \
    );                                                                          \
}                                                                               \
inline void vect##_ISR_L3(void)

/* Unused function for this port */
#define     nOS_InitSpecific()

#define     nOS_SwitchContext()         asm volatile (CALL_X" #nOS_SwitchContextHandler")
void        nOS_SwitchContextHandler    (void) __attribute__ ((naked));

#ifdef NOS_PRIVATE
 void       nOS_InitContext             (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
