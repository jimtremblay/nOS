/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "stm32f4xx_conf.h"
#include "nOS.h"

#define TICKS_PER_SECOND            1000

#define THREAD_STACK_SIZE           128

nOS_Sem semA;
nOS_Sem semB;
nOS_Sem semC;
nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
nOS_Stack stackA[THREAD_STACK_SIZE];
nOS_Stack stackB[THREAD_STACK_SIZE];
nOS_Stack stackC[THREAD_STACK_SIZE];

NOS_ISR(SysTick_Handler)
{
    nOS_Tick();
}

void ThreadA (void *arg)
{
    uint16_t count;

    NOS_UNUSED(arg);

    count = nOS_TickCount() + 500;
    while (1) {
        //nOS_SemTake(&semA, NOS_WAIT_INFINITE);
        nOS_SleepUntil(count);
        count += 500;
    }
}

void ThreadB (void *arg)
{
    uint16_t count;

    NOS_UNUSED(arg);

    count = nOS_TickCount() + 500;
    while (1) {
        //nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        //nOS_SemGive(&semA);
        nOS_SleepUntil(count);
        count += 500;
    }
}

void ThreadC (void *arg)
{
    uint16_t count;

    NOS_UNUSED(arg);

    count = nOS_TickCount() + 500;
    while (1) {
        //nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        //nOS_SemGive(&semB);
        nOS_SleepUntil(count);
        count += 500;
    }
}

int main(void)
{
    // No interrupt should occurs before nOS_Init
    nOS_Init();

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, 0, stackA, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY);
    nOS_ThreadCreate(&threadB, ThreadB, 0, stackB, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY);
    nOS_ThreadCreate(&threadC, ThreadC, 0, stackC, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY);

    while (1) {
        nOS_SemGive(&semC);
    }
}
