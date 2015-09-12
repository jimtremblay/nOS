#include "nOS.h"

nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
nOS_Sem semA;
nOS_Sem semB;
nOS_Sem semC;
nOS_Stack stackA;
nOS_Stack stackB;
nOS_Stack stackC;

void ThreadA (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
    }
}

void ThreadB (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
    }
}

void ThreadC (void *arg)
{
    while(1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
    }
}

int main(int argc, char *argv[])
{
    nOS_Init();

    nOS_ThreadSetName(NULL, "main");

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, NULL, &stackA, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY, "ThreadA");
    nOS_ThreadCreate(&threadB, ThreadB, NULL, &stackB, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY, "ThreadB");
    nOS_ThreadCreate(&threadC, ThreadC, NULL, &stackC, 0, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY, "ThreadC");

    nOS_Start(NULL);

    while (1) {
        nOS_Print("%s running\n", nOS_ThreadGetName(NULL));
        nOS_SemGive(&semC);
    }
}
