/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PORT_H
#define PORT_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4

#define NOS_PORT_SCHED_USE_32_BITS

#if !defined(NOS_CONFIG_ISR_STACK_SIZE)
 #define NOS_CONFIG_ISR_STACK_SIZE          128
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (default to 128)."
 #endif
#else
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #endif
#endif

__attribute__( ( always_inline ) ) static inline uint32_t GetMSP (void)
{
    uint32_t r;

    __asm volatile ("MRS %0, MSP" : "=r" (r));

    return r;
}

__attribute__( ( always_inline ) ) static inline void SetMSP (uint32_t r)
{
    __asm volatile ("MSR MSP, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t GetPSP (void)
{
    uint32_t r;

    __asm volatile ("MRS %0, PSP" : "=r" (r));

    return r;
}

__attribute__( ( always_inline ) ) static inline void SetPSP (uint32_t r)
{
    __asm volatile ("MSR PSP, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t GetCONTROL (void)
{
    uint32_t r;

    __asm volatile ("MRS %0, CONTROL" : "=r" (r));

    return r;
}

__attribute__( ( always_inline ) ) static inline void SetCONTROL (uint32_t r)
{
    __asm volatile ("MSR CONTROL, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t GetPRIMASK (void)
{
    uint32_t r;

    __asm volatile ("MRS %0, PRIMASK" : "=r" (r));

    return r;
}

__attribute__( ( always_inline ) ) static inline void SetPRIMASK (uint32_t r)
{
    __asm volatile ("MSR PRIMASK, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline void DSB (void)
{
    __asm volatile ("DSB");
}

__attribute__( ( always_inline ) ) static inline void ISB (void)
{
    __asm volatile ("ISB");
}

__attribute__( ( always_inline ) ) static inline void NOP (void)
{
    __asm volatile ("NOP");
}

__attribute__( ( always_inline ) ) static inline void DisableInterrupt (void)
{
    __asm volatile ("CPSID I");
}

__attribute__( ( always_inline ) ) static inline void EnableInterrupt (void)
{
    __asm volatile ("CPSIE I");
}

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint32_t    _primask = GetPRIMASK();                                        \
    DisableInterrupt();                                                         \
    DSB();                                                                      \
    ISB()

#define nOS_CriticalLeave()                                                     \
    SetPRIMASK(_primask);                                                       \
    DSB();                                                                      \
    ISB();                                                                      \
}

/*
 * Request a context switch and enable interrupts to allow PendSV interrupt.
 */
#define nOS_ContextSwitch()                                                     \
    *(volatile uint32_t *)0xe000ed04UL = 0x10000000UL;                          \
    EnableInterrupt();                                                          \
    DSB();                                                                      \
    ISB();                                                                      \
    NOP();                                                                      \
    DisableInterrupt();                                                         \
    DSB();                                                                      \
    ISB()


void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);

#define NOS_ISR(func)                                                           \
void func##_ISR(void);                                                          \
void func(void)                                                                 \
{                                                                               \
    nOS_IsrEnter();                                                             \
    func##_ISR();                                                               \
    nOS_IsrLeave();                                                             \
}                                                                               \
void func##_ISR(void)

#if defined(NOS_PRIVATE)
void        nOS_PortInit        (void);
#endif  /* NOS_PRIVATE */

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* PORT_H */
