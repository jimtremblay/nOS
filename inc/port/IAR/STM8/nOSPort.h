/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <intrinsics.h>

extern void     __set_cpu_sp    (int sp);
extern int      __get_cpu_sp    (void);

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t                 nOS_Stack;
typedef uint8_t                 nOS_StatusReg;

#define NOS_UNUSED(v)           (void)v

#define NOS_MEM_ALIGNMENT       1

#if   (__DATA_MODEL__ == __SMALL_DATA_MODEL__)
 #error "nOSConfig.h: Small data model is not supported: must use Medium or Large."
#elif (__DATA_MODEL__ == __MEDIUM_DATA_MODEL__)
 #define SET_CPU_SP_ON_ISR_ENTRY() __set_cpu_sp((int)nOS_EnterIsr((nOS_Stack*)__get_cpu_sp()))
 #define SET_CPU_SP_ON_ISR_EXIT()  __set_cpu_sp((int)nOS_LeaveIsr((nOS_Stack*)__get_cpu_sp()))
#elif (__DATA_MODEL__ == __LARGE_DATA_MODEL__)
 #define SET_CPU_SP_ON_ISR_ENTRY() __set_cpu_sp((int)((long)nOS_EnterIsr((nOS_Stack*)__get_cpu_sp())))
 #define SET_CPU_SP_ON_ISR_EXIT()  __set_cpu_sp((int)((long)nOS_LeaveIsr((nOS_Stack*)__get_cpu_sp())))
#endif

#ifdef NOS_CONFIG_ISR_STACK_SIZE
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
 #endif
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_state();                                           \
        __disable_interrupt();                                                  \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __set_interrupt_state(sr)

#define PUSH_CONTEXT()                                                          \
    __asm (                                                                     \
        "pushw  Y       \n"                                                     \
        "pushw  X       \n"                                                     \
        "push   A       \n"                                                     \
        "push   CC      \n"                                                     \
        "push   0x00    \n"                                                     \
        "push   0x01    \n"                                                     \
        "push   0x02    \n"                                                     \
        "push   0x03    \n"                                                     \
        "push   0x04    \n"                                                     \
        "push   0x05    \n"                                                     \
        "push   0x06    \n"                                                     \
        "push   0x07    \n"                                                     \
        "push   0x08    \n"                                                     \
        "push   0x09    \n"                                                     \
        "push   0x0a    \n"                                                     \
        "push   0x0b    \n"                                                     \
        "push   0x0c    \n"                                                     \
        "push   0x0d    \n"                                                     \
        "push   0x0e    \n"                                                     \
        "push   0x0f    \n"                                                     \
    )

#define POP_CONTEXT()                                                           \
    __asm (                                                                     \
        "pop    0x0f    \n"                                                     \
        "pop    0x0e    \n"                                                     \
        "pop    0x0d    \n"                                                     \
        "pop    0x0c    \n"                                                     \
        "pop    0x0b    \n"                                                     \
        "pop    0x0a    \n"                                                     \
        "pop    0x09    \n"                                                     \
        "pop    0x08    \n"                                                     \
        "pop    0x07    \n"                                                     \
        "pop    0x06    \n"                                                     \
        "pop    0x05    \n"                                                     \
        "pop    0x04    \n"                                                     \
        "pop    0x03    \n"                                                     \
        "pop    0x02    \n"                                                     \
        "pop    0x01    \n"                                                     \
        "pop    0x00    \n"                                                     \
        "pop    CC      \n"                                                     \
        "pop    A       \n"                                                     \
        "popw   X       \n"                                                     \
        "popw   Y       \n"                                                     \
    )

nOS_Stack*      nOS_EnterIsr        (nOS_Stack *sp);
nOS_Stack*      nOS_LeaveIsr        (nOS_Stack *sp);

#define NOS_ISR(vect)                                                           \
__task void vect##_ISR_L2(void);                                                \
__task void vect##_ISR_L3(void);                                                \
_Pragma(_STRINGIFY(vector=vect))                                                \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    vect##_ISR_L2();                                                            \
}                                                                               \
__task void vect##_ISR_L2(void)                                                 \
{                                                                               \
    PUSH_CONTEXT();                                                             \
    SET_CPU_SP_ON_ISR_ENTRY();                                                  \
    vect##_ISR_L3();                                                            \
    __disable_interrupt();                                                      \
    SET_CPU_SP_ON_ISR_EXIT();                                                   \
    POP_CONTEXT();                                                              \
}                                                                               \
__task void vect##_ISR_L3(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific         (void);
 void   nOS_InitContext          (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
 /* Declare this function as __task; we don't need the compiler to push registers on the stack since we do it manually */
 __task void   nOS_SwitchContext (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
