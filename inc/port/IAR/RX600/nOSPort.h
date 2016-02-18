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

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t                            nOS_Stack;
typedef __ilevel_t                          nOS_StatusReg;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#else
 #define NOS_MAX_UNSAFE_IPL                 NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_level();                                           \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            __set_interrupt_level(NOS_MAX_UNSAFE_IPL);                          \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __set_interrupt_level(sr)

#define nOS_SwitchContext()                                     __asm("INT  #27")

void    nOS_EnterIsr    (void);
void    nOS_LeaveIsr    (void);

#define NOS_ISR(vect)                                                           \
void vect##_ISR_L2(void);                                                       \
_Pragma(_STRINGIFY(vector=vect))                                                \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    nOS_EnterIsr();                                                             \
    vect##_ISR_L2();                                                            \
    __disable_interrupt();                                                      \
    nOS_LeaveIsr();                                                             \
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
