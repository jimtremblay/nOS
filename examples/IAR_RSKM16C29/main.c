
#include "iom16c29.h"
#include "nOS.h"

nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
nOS_Stack stackA[64];
nOS_Stack stackB[64];
nOS_Stack stackC[64];
nOS_Sem semA;
nOS_Sem semB;
nOS_Sem semC;

NOS_ISR(TMRA0)
{
    nOS_Tick();
}

void ThreadA (void *arg)
{
	while (1) {
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
	}
}

void ThreadB (void *arg)
{
	while (1) {
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
	}
}

void ThreadC (void *arg)
{
	while (1) {
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
	}
}

void TimerA0_Init(void)
{
    ta0mr = 0x80;
	ta0 = 0xFFFFU;
	cpsrf = 0x08;
    ta0ic = NOS_CONFIG_MAX_UNSAFE_ISR_PRIO;
    ir_ta0ic = 0;
    
    ta0s = 1;

    asm("FSET I");
    asm("NOP");
}

int main (void)
{
    nOS_Init();
    
    nOS_ThreadSetName(NULL, "main");
    
    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, NULL, stackA, 64, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY, "ThreadA");
    nOS_ThreadCreate(&threadB, ThreadB, NULL, stackB, 64, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY, "ThreadB");
    nOS_ThreadCreate(&threadC, ThreadC, NULL, stackC, 64, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY, "ThreadC");

    nOS_Start(TimerA0_Init);

    while (1){
        nOS_SemGive(&semC);
    }
}
