.global GdtFlush
GdtFlush:
	mov 4(%esp), %eax
	lgdt (%eax) // load the GDT with the gdt pointer
    mov $0x10, %ax // 0x10 is the offset in the gdt to our data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %gs
    mov %ax, %fs
    mov %ax, %ss
    jmp $0x08,$flush // 0x08 is the offset to our code segment: Far jump!
flush:
    ret
.global IdtLoad
IdtLoad:
	mov 4(%esp), %eax
	lidt (%eax)
	ret
