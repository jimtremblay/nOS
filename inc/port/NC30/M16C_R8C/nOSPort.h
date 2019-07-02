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

typedef uint16_t                            nOS_Stack;
typedef uint16_t                            nOS_StatusReg;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   2
#define NOS_MEM_POINTER_WIDTH               2

#define NOS_16_BITS_SCHEDULER

#ifdef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #if (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO < 0) || (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 7)
  #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value: must be set between 0 and 7 inclusively. (0 disable zero interrupt latency feature)"
 #elif (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 0)
  #define NOS_MAX_UNSAFE_IPL                NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #else
  #undef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #endif
#endif

uint16_t    _GetIPL     (void);
void        _SetIPL     (uint16_t ipl);
uint16_t    _GetI       (void);
void        _SetI       (uint16_t i);

#define _NOP()                                                      asm("NOP")
#define _EI()                                                       asm("FSET I")
#define _DI()                                                       asm("FCLR I")

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
        _NOP();                                                                 \
    } while (0)
#endif

#define nOS_SwitchContext()                                         asm ("INT #32")

void    nOS_EnterISR    (void);
bool    nOS_LeaveISR    (void);

#define NOS_ISR(func)                                                           \
void func(void);                                                                \
void func##_L2(void);                                                           \
void func(void)                                                                 \
{                                                                               \
    asm (                                                                       \
        /* Push all registers on ISTACK */                                      \
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Increment interrupts nested counter */                               \
        "JSR.A  _nOS_EnterISR               \n"                                 \
                                                                                \
        /* Call user ISR function */                                            \
        "JSR.A  _"#func"_L2                 \n"                                 \
                                                                                \
        /* Ensure interrupts are disabled */                                    \
        "FCLR   I                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Decrement interrupts nested counter */                               \
        /* return true if we need to switch context, otherwise false */         \
        "JSR.A  _nOS_LeaveISR               \n"                                 \
                                                                                \
        /* Do we need to switch context from ISR ? */                           \
        "CMP.B  #0, R0L                     \n"                                 \
        "JEQ    _Leave"#func"               \n"                                 \
                                                                                \
        /* YES, we need to switch context */                                    \
                                                                                \
        /* Pop all thread registers */                                          \
        /* nOS_SwitchContextFromIsr will push it all on USTACK */               \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Switch to thread stack */                                            \
        "FSET   U                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Call nOS_SwitchContextFromIsr */                                     \
        "INT    #33                         \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* NO, We DON'T need to switch context */                               \
                                                                                \
        "_Leave"#func":                     \n"                                 \
        /* Pop all registers from ISTACK */                                     \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Return from interrupt */                                             \
        "REIT                               \n"                                 \
        "NOP                                \n"                                 \
    );                                                                          \
}                                                                               \
void func##_L2(void)

/* Unused function */
#define nOS_InitSpecific()

#ifdef NOS_PRIVATE
 void   nOS_InitContext                 (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

void    nOS_SwitchContextHandler        (void);
void    nOS_SwitchContextFromIsrHandler (void);

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
