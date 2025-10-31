#include <stddef.h>
#include <stdint.h>

#define GDT_SEG_LIMIT	0xFFFFFFFF
#define GDT_CODE_ACCESS	0x9A
#define GDT_DATA_ACCESS	0x92
#define GDT_GRAN 		0xCF

// #define PIC1			0x20		/* IO base address for master PIC */
// #define PIC2			0xA0		/* IO base address for slave PIC */
// #define PIC1_COMMAND	PIC1
// #define PIC1_DATA		(PIC1+1)
// #define PIC2_COMMAND	PIC2
// #define PIC2_DATA		(PIC2+1)
//
// #define ICW1_ICW4		0x01		/* Indicates that ICW4 will be present */
// #define ICW1_SINGLE		0x02		/* Single (cascade) mode */
// #define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
// #define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
// #define ICW1_INIT		0x10		/* Initialization - required! */
//
// #define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
// #define ICW4_AUTO		0x02		/* Auto (normal) EOI */
// #define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
// #define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
// #define ICW4_SFNM		0x10		/* Special fully nested (not) */
//
// #define CASCADE_IRQ 	2

// ---------------------------------------
// gdt

typedef struct {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;
	gdt_entry_t* base;
} __attribute__((packed)) gdt_ptr_t;

gdt_entry_t gdt[3];
gdt_ptr_t gdt_ptr;

void SetGdtGate(uint32_t ind, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[ind].base_low = base & 0xFFFF;
	gdt[ind].base_middle = (base >> 16) & 0xFF;
	gdt[ind].base_high = (base >> 24) & 0xFF;
	gdt[ind].limit = limit & 0xFFFF; 
	gdt[ind].flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);
	gdt[ind].access = access;
}

extern void GdtLoad(gdt_ptr_t*);

// ---------------------------------------
// idt

typedef struct {
	uint16_t isr_address_low; 	// the lower 16 bits of the isr's address
	uint16_t kernel_cs;			// the gdt segment selector that the CPU will load into CS before calling the ISR
	uint8_t reserved;     	  	// set to zero
	uint8_t flags;  		  	// type and attributes
	uint16_t isr_address_high;	// the higher 16 bits of the isr's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	idt_entry_t* base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt[256];
idt_ptr_t idt_ptr;

void SetIdtGate(uint32_t ind, uint32_t base, uint8_t flags) {
	idt[ind].isr_address_low = base << 16;
	idt[ind].isr_address_high = base >> 16;
	idt[ind].kernel_cs = 0x08;
	idt[ind].reserved = 0;
	idt[ind].flags = flags;
}

extern void IdtLoad(idt_ptr_t*);

typedef struct {
	uint32_t eip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed)) interrupt_frame_t;

// __attribute__((interrupt)) void interrupt_handler(interrupt_frame_t* frame) {
//
// }

// ---------------------------------------
// utils

void* memset(void* ptr, int value, size_t num) {
	uint8_t* pbyte = (uint8_t*)ptr;
	for (size_t i = 0; i < num; ++i, ++pbyte) {
		*pbyte = value;
	}
	return ptr;
}

// send value to an io location
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}
// ---------------------------------------
// isrs

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

const char *exception_messages[] = {
  "Division By Zero"
  , "Debug"
  , "Non Maskable Interrupt"
  , "Breakpoint"
  , "Into Detected Overflow"
  , "Out of Bounds"
  , "Invalid Opcode"
  , "No Coprocessor"
  , "Double Fault"
  , "Coprocessor Segment Overrun"
  , "Bad TSS"
  , "Segment Not Present"
  , "Stack Fault"
  , "General Protection Fault"
  , "Page Fault"
  , "Unknown Interrupt"
  , "Coprocessor Fault"
  , "Alignment Check (486+)"
  , "Machine Check (Pentium/586+)"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
  , "Reserved"
};

/* This defines what the stack looks like after an ISR was running */
typedef struct {
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_num, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} regs;

void isr_handler(regs *r)
{
    /* Is this a fault whose number is from 0 to 31? */
    if (r->int_num < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
		// TODO: output stuff to a serial port
        // puts(exception_messages[r->int_no]);
        // puts(" Exception. System Halted!\n");
        for (;;);
    }
}

// ---------------------------------------

volatile uint16_t* vga_buf = (uint16_t*)0xB8000;
int term_color = 0x0F; // Black background, White foreground

void KernelMain() {
	// gdt init
	gdt_ptr.limit = sizeof(gdt_entry_t) * 3 - 1;
	gdt_ptr.base = gdt;

	SetGdtGate(0, 0, 0, 0, 0); 					// NULL segment
	SetGdtGate(1, 0, GDT_SEG_LIMIT, GDT_CODE_ACCESS, GDT_GRAN);	// kernel code segment
	SetGdtGate(2, 0, GDT_SEG_LIMIT, GDT_DATA_ACCESS, GDT_GRAN);	// kernel data segment
	// SetGdtSegment(3, 0, GDT_SEG_LIMIT, GDT_CODE_ACCESS, GDT_GRAN);	// user code segment
	// SetGdtSegment(4, 0, GDT_SEG_LIMIT, GDT_DATA_ACCESS, GDT_GRAN);	// user data segment
	GdtLoad(&gdt_ptr);
	//-----------------------------------------------------------

	// idt init
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = idt;

	// TODO: do i need to do this?
    memset(&idt, 0, sizeof(idt_entry_t) * 256);
	IdtLoad(&idt_ptr);
	SetIdtGate(0,  (uint32_t)isr0,  0x8E);
	SetIdtGate(1,  (uint32_t)isr1,  0x8E);
	SetIdtGate(2,  (uint32_t)isr2,  0x8E);
	SetIdtGate(3,  (uint32_t)isr3,  0x8E);
	SetIdtGate(4,  (uint32_t)isr4,  0x8E);
	SetIdtGate(5,  (uint32_t)isr5,  0x8E);
	SetIdtGate(6,  (uint32_t)isr6,  0x8E);
	SetIdtGate(7,  (uint32_t)isr7,  0x8E);
	SetIdtGate(8,  (uint32_t)isr8,  0x8E);
	SetIdtGate(9,  (uint32_t)isr9,  0x8E);
	SetIdtGate(10, (uint32_t)isr10, 0x8E);
	SetIdtGate(11, (uint32_t)isr11, 0x8E);
	SetIdtGate(12, (uint32_t)isr12, 0x8E);
	SetIdtGate(13, (uint32_t)isr13, 0x8E);
	SetIdtGate(14, (uint32_t)isr14, 0x8E);
	SetIdtGate(15, (uint32_t)isr15, 0x8E);
	SetIdtGate(16, (uint32_t)isr16, 0x8E);
	SetIdtGate(17, (uint32_t)isr17, 0x8E);
	SetIdtGate(18, (uint32_t)isr18, 0x8E);
	SetIdtGate(19, (uint32_t)isr19, 0x8E);
	SetIdtGate(20, (uint32_t)isr20, 0x8E);
	SetIdtGate(21, (uint32_t)isr21, 0x8E);
	SetIdtGate(22, (uint32_t)isr22, 0x8E);
	SetIdtGate(23, (uint32_t)isr23, 0x8E);
	SetIdtGate(24, (uint32_t)isr24, 0x8E);
	SetIdtGate(25, (uint32_t)isr25, 0x8E);
	SetIdtGate(26, (uint32_t)isr26, 0x8E);
	SetIdtGate(27, (uint32_t)isr27, 0x8E);
	SetIdtGate(28, (uint32_t)isr28, 0x8E);
	SetIdtGate(29, (uint32_t)isr29, 0x8E);
	SetIdtGate(30, (uint32_t)isr30, 0x8E);
	SetIdtGate(31, (uint32_t)isr31, 0x8E);

	// // pic remap
	// outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	// outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	// outb(PIC1_DATA, 0x20);                 		// ICW2: Master PIC vector offset
	// outb(PIC2_DATA, 0x28);                 		// ICW2: Slave PIC vector offset
	// outb(PIC1_DATA, 1 << CASCADE_IRQ);        	// ICW3: tell Master PIC that there is a slave PIC at IRQ2
	// outb(PIC2_DATA, 2);                       	// ICW3: tell Slave PIC its cascade identity (0000 0010)
	//
	// outb(PIC1_DATA, ICW4_8086);               	// ICW4: have the PICs use 8086 mode (and not 8080 mode)
	// outb(PIC2_DATA, ICW4_8086);
	//
	// outb(PIC1_DATA, 0); 						// Unmask both PICs
	// outb(PIC2_DATA, 0);
	//-----------------------------------------------------------

	vga_buf[0] = ((uint16_t)term_color << 8) | 'W';
}
