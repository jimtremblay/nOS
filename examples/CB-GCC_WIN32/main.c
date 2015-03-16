#include "nOS.h"

//#define EX_SEM

nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
#ifdef EX_SEM
 nOS_Sem semA;
 nOS_Sem semB;
 nOS_Sem semC;
#endif

void ThreadA (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
#ifdef EX_SEM
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
#else
        nOS_Sleep(NOS_CONFIG_TICKS_PER_SECOND);
#endif
    }
}

void ThreadB (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
#ifdef EX_SEM
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
#else
        nOS_Sleep(NOS_CONFIG_TICKS_PER_SECOND);
#endif
    }
}

void ThreadC (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
#ifdef EX_SEM
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
#else
        nOS_Sleep(NOS_CONFIG_TICKS_PER_SECOND);
#endif
    }
}

int main(int argc, char *argv[])
{
    nOS_Init();

    nOS_ThreadSetName(NULL, "main");

#ifdef EX_SEM
    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);
#endif

    nOS_ThreadCreate(&threadA, ThreadA, NULL, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY, "ThreadA");
    nOS_ThreadCreate(&threadB, ThreadB, NULL, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY, "ThreadB");
    nOS_ThreadCreate(&threadC, ThreadC, NULL, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY, "ThreadC");

    while (1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
#ifdef EX_SEM
        nOS_SemGive(&semC);
#else
        nOS_Sleep(NOS_CONFIG_TICKS_PER_SECOND);
#endif
    }
}
