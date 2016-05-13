        name  nOSPortASM

        public  __set_cpu_sp
        public  __get_cpu_sp

        section .near.bss:DATA

PCL             DS8    1
PCH             DS8    1

        section .near_func.text:CODE

pop_pc          macro
                pop PCH
                pop PCL
                endm

push_pc         macro
                push  PCL
                push  PCH
                endm

__set_cpu_sp:
                pop_pc
                ldw SP, X
                push_pc
                ret

__get_cpu_sp:
                ldw X, SP
                addw X, #$2
                ret

                end