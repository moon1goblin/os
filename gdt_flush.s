.global gdt_flush
gdt_flush:
	mov %eax, 4(%esp)
    lgdt (%eax) // load the GDT with the gdt pointer
    mov $0x10, %eax // 0x10 is the offset in the gdt to our data segment
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %gs
    mov %eax, %fs
    mov %eax, %ss
    jmp $0x08,$flush2 // 0x08 is the offset to our code segment: Far jump!
flush2:
    ret
