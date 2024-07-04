GPU equ 0xb800

mov ax, 3
int 0x10

mov ax, GPU
mov es, ax

mov si, message
call print

// TODO1：读磁盘加载到内存中，自己来定磁盘号和内存地址

jmp $

print:
    push ax
    mov ah, 0xe
    .write:
        mov al, [si]
        cmp al, 0
        jz .done
        int 0x10
        inc si
        jmp .write
    .done:
        pop ax
        ret

message:
    db "Hello this is middle task", 10, 13, 0 
message_end:

// TODO2：补充读磁盘函数