SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00

; Clear screen
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f
    int 0x10

; Get cursor location
    mov ah, 3
    mov bh, 0
    int 0x10

; Print string
    mov ax, message
    mov bp, ax
    mov cx, 5
    mov ax, 0x1301
    mov bx, 0x2
    int 0x10

; While(1)
    jmp $

    message db "1 MBR YWL"
    times 510-($-$$) db 0
    db 0x55, 0xaa
