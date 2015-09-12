#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "nOS.h"

// Using the 20MHz clock provided by the PIC24FJ256GB106
_CONFIG2(IESO_OFF & PLL_96MHZ_ON & PLLDIV_DIV5 & FNOSC_PRIPLL & POSCMOD_HS)   // Primary HS OSC with PLL, USBPLL /3
_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)        // JTAG off, watchdog timer off

#define THREAD_STACK_SIZE       128

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
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
    }
}

void ThreadB(void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
    }
}

void ThreadC(void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
    }
}

static void TimerInit(void)
{
    T2CON = 0;
    TMR2 = 0;

    PR2 = ( uint16_t ) 50000;

    IPC1bits.T2IP = NOS_CONFIG_MAX_UNSAFE_ISR_PRIO;

    IFS0bits.T2IF = 0;

    IEC0bits.T2IE = 1;

    T2CONbits.TON = 1;
}

NOS_ISR(_T2Interrupt)
{
    /* Clear the timer interrupt. */
    IFS0bits.T2IF = 0;

    nOS_Tick();
#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerTick();
#endif
#if (NOS_CONFIG_TIME_ENABLE > 0)
    nOS_TimeTick();
#endif
}

/*
 *
 */
int main(int argc, char** argv)
{
    nOS_Init();

    nOS_ThreadSetName(NULL, "main");

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, (void*)300, threadAStack, THREAD_STACK_SIZE, 5, NOS_THREAD_READY, "ThreadA");
    nOS_ThreadCreate(&threadB, ThreadB, (void*)200, threadBStack, THREAD_STACK_SIZE, 4, NOS_THREAD_READY, "ThreadB");
    nOS_ThreadCreate(&threadC, ThreadC, (void*)100, threadCStack, THREAD_STACK_SIZE, 3, NOS_THREAD_READY, "ThreadC");

    nOS_Start(TimerInit);

    while (1) {
        nOS_SemGive(&semC);
    }
}
