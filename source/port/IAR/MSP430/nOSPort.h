/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <stdint.h>
#include <intrinsics.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if (__CORE__ == __430X__)
#if (__DATA_MODEL__ == __DATA_MODEL_SMALL__)
typedef uint16_t                nOS_Stack;
#define NOS_PORT_SCHED_USE_16_BITS
#define NOS_MEM_ALIGNMENT       2
#else
typedef uint32_t                nOS_Stack;
#define NOS_PORT_SCHED_USE_32_BITS
#define NOS_MEM_ALIGNMENT       4
#endif
#else
typedef uint16_t                nOS_Stack;
#define NOS_PORT_SCHED_USE_16_BITS
#define NOS_MEM_ALIGNMENT       2
#endif

#define NOS_UNUSED(v)           (void)v

#if (__DATA_MODEL__ == __DATA_MODEL_SMALL__)
#define     PUSH_X              "PUSH.W"
#define     POP_X               "POP.W"
#define     PUSHM_X             "PUSHM.W"
#define     POPM_X              "POPM.W"
#define     MOV_X               "MOV.W"
#endif

#if (__DATA_MODEL__ == __DATA_MODEL_MEDIUM__)
#define     PUSH_X              "PUSHX.A"
#define     POP_X               "POPX.A"
#define     PUSHM_X             "PUSHM.A"
#define     POPM_X              "POPM.A"
#define     MOV_X               "MOV.W"
#endif

#if (__DATA_MODEL__ == __DATA_MODEL_LARGE__)
#define     PUSH_X              "PUSHX.A"
#define     POP_X               "POPX.A"
#define     PUSHM_X             "PUSHM.A"
#define     POPM_X              "POPM.A"
#define     MOV_X               "MOVX.A"
#endif

#if (__CORE__ == __430X__)
#if (__CODE_MODEL__ == __CODE_MODEL_SMALL__)
#define     CALL_X              "CALL"
#define     RET_X               "RET"
#endif
#if (__CODE_MODEL__ == __CODE_MODEL_LARGE__)
#define     CALL_X              "CALLA"
#define     RET_X               "RETA"
#endif
#else
#define     CALL_X              "CALL"
#define     RET_X               "RET"
#endif

#define PUSH_SR                 "PUSH.W     SR              \n"
#define POP_SR                  "POP.W      SR              \n"
#if (__CORE__ == __430X__)
#define     PUSH_CONTEXT        PUSHM_X"    #12,    R15     \n"
#define     POP_CONTEXT         POPM_X"     #12,    R15     \n"
#else
#define     PUSH_CONTEXT                                                        \
    "PUSH   R15             \n"                                                 \
    "PUSH   R14             \n"                                                 \
    "PUSH   R13             \n"                                                 \
    "PUSH   R12             \n"                                                 \
    "PUSH   R11             \n"                                                 \
    "PUSH   R10             \n"                                                 \
    "PUSH   R9              \n"                                                 \
    "PUSH   R8              \n"                                                 \
    "PUSH   R7              \n"                                                 \
    "PUSH   R6              \n"                                                 \
    "PUSH   R5              \n"                                                 \
    "PUSH   R4              \n"
#define     POP_CONTEXT                                                         \
    "POP    R4              \n"                                                 \
    "POP    R5              \n"                                                 \
    "POP    R6              \n"                                                 \
    "POP    R7              \n"                                                 \
    "POP    R8              \n"                                                 \
    "POP    R9              \n"                                                 \
    "POP    R10             \n"                                                 \
    "POP    R11             \n"                                                 \
    "POP    R12             \n"                                                 \
    "POP    R13             \n"                                                 \
    "POP    R14             \n"                                                 \
    "POP    R15             \n"
#endif

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    __istate_t _sr = __get_interrupt_state();                                   \
    __disable_interrupt()

#define nOS_CriticalLeave()                                                     \
    __set_interrupt_state(_sr);                                                 \
}

#define NOS_ISR(vect)                                                           \
__task void vect##_ISR_L2(void);                                                \
void vect##_ISR_L3(void);                                                       \
_Pragma(_STRINGIFY(vector=##vect))                                              \
__interrupt __raw void vect##_ISR(void)                                         \
{                                                                               \
    vect##_ISR_L2();                                                            \
}                                                                               \
_Pragma("required=nOS_IsrEnter")                                                \
_Pragma("required=nOS_IsrLeave")                                                \
__task void vect##_ISR_L2(void)                                                 \
{                                                                               \
    __asm (                                                                     \
        PUSH_SR                                                                 \
        PUSH_CONTEXT                                                            \
        MOV_X"  SP,     R12                     \n"                             \
        CALL_X" #nOS_IsrEnter                   \n"                             \
        MOV_X"  R12,    SP                      \n"                             \
        CALL_X" #"_STRINGIFY(vect##_ISR_L3)"    \n"                             \
        "DINT                                   \n"                             \
        "NOP                                    \n"                             \
        MOV_X"  SP,     R12                     \n"                             \
        CALL_X" #nOS_IsrLeave                   \n"                             \
        MOV_X"  R12,    SP                      \n"                             \
        POP_CONTEXT                                                             \
        POP_SR                                                                  \
        RET_X                                                                   \
    );                                                                          \
}                                                                               \
void vect##_ISR_L3(void)

/* Unused function for this port */
#define nOS_PortInit()

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
__task void nOS_ContextSwitch   (void);

nOS_Stack*  nOS_IsrEnter        (nOS_Stack *sp);
nOS_Stack*  nOS_IsrLeave        (nOS_Stack *sp);

#if defined(__cplusplus)
}
#endif

#endif /* NOSPORT_H */
