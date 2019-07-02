/*
 * Copyright (c) 2014-2019 Jim Tremblay
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

typedef uint32_t                        nOS_Stack;
typedef uint32_t                        nOS_StatusReg;

#define NOS_UNUSED(v)                   (void)v

#define NOS_MEM_ALIGNMENT               4
#define NOS_MEM_POINTER_WIDTH           4

#define NOS_32_BITS_SCHEDULER

#ifdef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
  #define NOS_MAX_UNSAFE_IPL            (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << 24)
 #else
  #undef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #endif
#endif

__attribute__( ( always_inline ) ) static inline uint32_t _GetIPL(void)
{
    uint32_t ipl;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "AND    #0x0F000000,    %0      \n"
    : "=r" (ipl) );
    return ipl;
}

__attribute__( ( always_inline ) ) static inline void _SetIPL(uint32_t ipl)
{
    uint32_t psw;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "AND    #0xF0FFFFFF,    %0      \n"
        "OR     %1,             %0      \n"
        "MVTC   %0,             PSW     \n"
        "NOP                            \n"
    : "=r" (psw), "=r" (ipl) );
}

__attribute__( ( always_inline ) ) static inline uint32_t _GetI(void)
{
    uint32_t i;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "AND    #0x00010000,    %0      \n"
    : "=r" (i) );
    return i;
}

__attribute__( ( always_inline ) ) static inline void _SetI(uint32_t i)
{
    uint32_t psw;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "AND    #0xFFFEFFFF,    %0      \n"
        "OR     %1,             %0      \n"
        "MVTC   %0,             PSW     \n"
        "NOP                            \n"
    : "=r" (psw), "=r" (i) );
}

__attribute( ( always_inline ) ) static inline void _EI(void)
{
    __asm volatile(
        "SETPSW I                       \n"
        "NOP                            \n"
    );
}

__attribute( ( always_inline ) ) static inline void _DI(void)
{
    __asm volatile(
        "CLRPSW I                       \n"
        "NOP                            \n"
    );
}

__attribute( ( always_inline ) ) static inline void _NOP(void)
{
    __asm volatile("NOP");
}

#ifdef NOS_MAX_UNSAFE_IPL
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetIPL();                                                         \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            _SetIPL(NOS_MAX_UNSAFE_IPL);                                        \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    _SetIPL(sr)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        _SetIPL(0);                                                             \
        _NOP();                                                                 \
        _SetIPL(NOS_MAX_UNSAFE_IPL);                                            \
    } while (0)
#else
#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetI();                                                           \
        _DI();                                                                  \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    _SetI(sr)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        _EI();                                                                  \
        _NOP();                                                                 \
        _DI();                                                                  \
    } while (0)
#endif

#define nOS_SwitchContext()                                     __asm("INT  #27")

void    nOS_EnterISR    (void);
void    nOS_LeaveISR    (void);

#define NOS_ISR(vect)                                                           \
void vect(void) __attribute__ ( ( interrupt ) );                                \
void vect##_ISR(void);                                                          \
void vect(void)                                                                 \
{                                                                               \
    nOS_EnterISR();                                                             \
    vect##_ISR();                                                               \
    _DI();                                                                      \
    nOS_LeaveISR();                                                             \
}                                                                               \
__attribute__ ( ( always_inline ) ) inline void vect##_ISR(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
