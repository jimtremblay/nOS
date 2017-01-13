/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

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

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#endif

#if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
#define NOS_MAX_UNSAFE_BASEPRI              (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))
#else
#undef NOS_NVIC_PRIO_BITS
#endif

__attribute__( ( always_inline ) ) static inline uint32_t _GetMSP (void)
{
    register uint32_t r;
    __asm volatile ("MRS %0, MSP" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetMSP (uint32_t r)
{
    __asm volatile ("MSR MSP, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t _GetPSP (void)
{
    register uint32_t r;
    __asm volatile ("MRS %0, PSP" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetPSP (uint32_t r)
{
    __asm volatile ("MSR PSP, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t _GetCONTROL (void)
{
    register uint32_t r;
    __asm volatile ("MRS %0, CONTROL" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetCONTROL (uint32_t r)
{
    __asm volatile ("MSR CONTROL, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t _GetBASEPRI (void)
{
    register uint32_t r;
    __asm volatile ("MRS %0, BASEPRI" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetBASEPRI (uint32_t r)
{
    __asm volatile ("MSR BASEPRI, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline uint32_t _GetPRIMASK (void)
{
    uint32_t r;
    __asm volatile ("MRS %0, PRIMASK" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetPRIMASK (uint32_t r)
{
    __asm volatile ("MSR PRIMASK, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline void _DSB (void)
{
    __asm volatile ("DSB");
}

__attribute__( ( always_inline ) ) static inline void _ISB (void)
{
    __asm volatile ("ISB");
}

__attribute__( ( always_inline ) ) static inline void _NOP (void)
{
    __asm volatile ("NOP");
}

__attribute__( ( always_inline ) ) static inline void _DI (void)
{
    __asm volatile ("CPSID I");
}

__attribute__( ( always_inline ) ) static inline void _EI (void)
{
    __asm volatile ("CPSIE I");
}

__attribute__( ( always_inline ) ) static inline uint32_t _CLZ(uint32_t n)
{
    register uint32_t r;
    __asm volatile (
        "CLZ    %0, %1"
        : "=r" (r)
        : "r" (n)
    );
    return r;
}

#if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetBASEPRI();                                                     \
        if (sr == 0 || sr > NOS_MAX_UNSAFE_BASEPRI) {                           \
            _SetBASEPRI(NOS_MAX_UNSAFE_BASEPRI);                                \
            _DSB();                                                             \
            _ISB();                                                             \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        _SetBASEPRI(sr);                                                        \
        _DSB();                                                                 \
        _ISB();                                                                 \
    } while (0)
#else
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetPRIMASK();                                                     \
        _DI();                                                                  \
        _DSB();                                                                 \
        _ISB();                                                                 \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        _SetPRIMASK(sr);                                                        \
        _DSB();                                                                 \
        _ISB();                                                                 \
    } while (0)
#endif

void    nOS_EnterIsr    (void);
void    nOS_LeaveIsr    (void);

#define NOS_ISR(func)                                                           \
void func##_ISR(void) __attribute__ ( ( always_inline ) );                      \
void func(void)                                                                 \
{                                                                               \
    nOS_EnterIsr();                                                             \
    func##_ISR();                                                               \
    nOS_LeaveIsr();                                                             \
}                                                                               \
inline void func##_ISR(void)

#ifdef NOS_PRIVATE
 void       nOS_InitSpecific        (void);
 void       nOS_InitContext         (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
 void       nOS_SwitchContext       (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
