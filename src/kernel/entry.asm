extern KernelMain
extern MemoryCheckout

global _start
_start:
    call MemoryCheckout
    call KernelMain
    mov ax, 3
    jmp $

global TestAssembly
TestAssembly:
    ; 立即数寻址
    mov ax, 0x22
    ; 直接寻址
    mov ax, [1234h]
    ; 构造一个值到 0x16c3
    mov [0x16c3], 0x22
    ; 变址寻址
    mov ax, [si+1000h]
    ret