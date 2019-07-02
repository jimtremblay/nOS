/*
 * Copyright (c) 2014-2019 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <intrinsics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

#ifdef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
  #define NOS_MAX_UNSAFE_IPL                NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #else
  #undef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #endif
#endif

#ifndef NOS_MAX_UNSAFE_IPL
 typedef __istate_t                         nOS_StatusReg;
#else
 typedef __ilevel_t                         nOS_StatusReg;
#endif

#ifdef NOS_MAX_UNSAFE_IPL
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_level();                                           \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            __set_interrupt_level(NOS_MAX_UNSAFE_IPL);                          \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __set_interrupt_level(sr)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        __set_interrupt_level(0);                                               \
        __no_operation();                                                       \
        __set_interrupt_level(NOS_MAX_UNSAFE_IPL);                              \
    } while (0)
#else
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_state();                                           \
        __disable_interrupt();                                                  \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __set_interrupt_state(sr)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        __enable_interrupt();                                                   \
        __no_operation();                                                       \
        __disable_interrupt();                                                  \
    } while (0)
#endif

#define nOS_SwitchContext()                                     __asm("INT  #27")

void    nOS_EnterISR    (void);
void    nOS_LeaveISR    (void);

#define NOS_ISR(vect)                                                           \
void vect##_ISR_L2(void);                                                       \
_Pragma(_STRINGIFY(vector=vect))                                                \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    nOS_EnterISR();                                                             \
    vect##_ISR_L2();                                                            \
    __disable_interrupt();                                                      \
    nOS_LeaveISR();                                                             \
}                                                                               \
inline void vect##_ISR_L2(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
