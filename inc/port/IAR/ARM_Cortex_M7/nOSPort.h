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
typedef uint32_t                            nOS_StatusReg;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER
#define NOS_USE_CLZ

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

/* __NVIC_PRIO_BITS defined from CMSIS if used */
#ifdef NOS_CONFIG_NVIC_PRIO_BITS
 #define NOS_NVIC_PRIO_BITS                 NOS_CONFIG_NVIC_PRIO_BITS
#elif defined(__NVIC_PRIO_BITS)
 #define NOS_NVIC_PRIO_BITS                 __NVIC_PRIO_BITS
#else
 #define NOS_NVIC_PRIO_BITS                 4
#endif

#ifdef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
  #define NOS_MAX_UNSAFE_BASEPRI            (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))
 #else
  #undef NOS_NVIC_PRIO_BITS
  #undef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #endif
#endif

#define _CLZ(n)                             __CLZ(n)

#ifdef NOS_MAX_UNSAFE_BASEPRI
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_BASEPRI();                                                   \
        if (sr == 0 || sr > NOS_MAX_UNSAFE_BASEPRI) {                           \
            __set_BASEPRI(NOS_MAX_UNSAFE_BASEPRI);                              \
            __DSB();                                                            \
            __ISB();                                                            \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        __set_BASEPRI(sr);                                                      \
        __DSB();                                                                \
        __ISB();                                                                \
    } while (0)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        __set_BASEPRI(0);                                                       \
        __DSB();                                                                \
        __ISB();                                                                \
        __no_operation();                                                       \
        __set_BASEPRI(NOS_MAX_UNSAFE_BASEPRI);                                  \
        __DSB();                                                                \
        __ISB();                                                                \
    } while (0)
#else
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_PRIMASK();                                                   \
        __disable_interrupt();                                                  \
        __DSB();                                                                \
        __ISB();                                                                \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        __set_PRIMASK(sr);                                                      \
        __DSB();                                                                \
        __ISB();                                                                \
    } while (0)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        __enable_interrupt();                                                   \
        __DSB();                                                                \
        __ISB();                                                                \
                                                                                \
        __no_operation();                                                       \
                                                                                \
        __disable_interrupt();                                                  \
        __DSB();                                                                \
        __ISB();                                                                \
    } while (0)
#endif

void    nOS_EnterISR    (void);
void    nOS_LeaveISR    (void);

#define NOS_ISR(func)                                                           \
void func##_ISR(void);                                                          \
void func(void)                                                                 \
{                                                                               \
    nOS_EnterISR();                                                             \
    func##_ISR();                                                               \
    nOS_LeaveISR();                                                             \
}                                                                               \
void func##_ISR(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
 void   nOS_SwitchContext   (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
