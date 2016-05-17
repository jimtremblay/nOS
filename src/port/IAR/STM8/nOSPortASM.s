        name  nOSPortASM

        public  __set_cpu_sp
        public  __get_cpu_sp

        section .near.bss:DATA

PCL             DS8    1
PCH             DS8    1
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
PCE             DS8    1
#endif

        section .near_func.text:CODE

pop_pc          macro
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
                pop PCE
#endif
                pop PCH
                pop PCL
                endm

push_pc         macro
                push  PCL
                push  PCH
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
                push PCE
#endif
                endm

__set_cpu_sp:
                pop_pc
                ldw SP, X
                push_pc
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
                retf
#else
                ret
#endif

__get_cpu_sp:
                ldw X, SP
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
                addw X, #$3
                retf
#else
                addw X, #$2
                ret
#endif

                end
