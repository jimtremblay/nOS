/*
 * nOS v0.1
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "nOS.h"

#define THREAD_STACK_SIZE       128

static void Timer2Init(void);

void ThreadA(void *arg);
void ThreadB(void *arg);
void ThreadC(void *arg);

nOS_Sem semA;
nOS_Sem semB;
nOS_Sem semC;
nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
nOS_Stack threadAStack[THREAD_STACK_SIZE];
nOS_Stack threadBStack[THREAD_STACK_SIZE];
nOS_Stack threadCStack[THREAD_STACK_SIZE];

void ThreadA(void *arg)
{
    volatile uint32_t cntr = 0;

    (void)arg;

    while(1)
    {
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
        cntr++;
    }
}

void ThreadB(void *arg)
{
    volatile uint32_t cntr = 0;
    cntr |= 0x80;

    (void)arg;

    while(1)
    {
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
        cntr++;
    }
}

void ThreadC(void *arg)
{
    volatile uint32_t cntr = 0;

    (void)arg;

    while(1)
    {
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
        cntr++;
    }
}

NOS_ISR(TIMER2_OVF_vect)
{
    nOS_Tick();
}

static void Timer2Init(void)
{
    TCNT2=200;
    TCCR2B = 0b00000010;
    TIMSK2 = (1 << TOIE2);
}

int main (void)
{
    volatile uint32_t cntr = 0;

    nOS_Init();

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
    nOS_ThreadCreate(&threadA, ThreadA, (void*)300, threadAStack, THREAD_STACK_SIZE, 5, NOS_THREAD_SUSPENDED);
    nOS_ThreadCreate(&threadB, ThreadB, (void*)200, threadBStack, THREAD_STACK_SIZE, 4, NOS_THREAD_SUSPENDED);
    nOS_ThreadCreate(&threadC, ThreadC, (void*)100, threadCStack, THREAD_STACK_SIZE, 3, NOS_THREAD_SUSPENDED);
#else
    nOS_ThreadCreate(&threadA, ThreadA, (void*)300, threadAStack, THREAD_STACK_SIZE, 5);
    nOS_ThreadCreate(&threadB, ThreadB, (void*)200, threadBStack, THREAD_STACK_SIZE, 4);
    nOS_ThreadCreate(&threadC, ThreadC, (void*)100, threadCStack, THREAD_STACK_SIZE, 3);
#endif

    Timer2Init();

    /* enable all interrupts */
    sei();

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
    nOS_ThreadResumeAll();
#endif

    while (1)
    {
        nOS_SemGive(&semC);
        cntr++;
    }
}
