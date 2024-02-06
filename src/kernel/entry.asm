extern KernelMain

global _start
_start:
    call KernelMain
    jmp $