#include <stddef.h>
#include <stdint.h>

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
	uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

gdt_entry_t gdt[3];
gdt_ptr_t gdt_ptr;

void SetGdtSegment(uint32_t ind, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[ind].base_low = base & 0xFFFF;
	gdt[ind].base_middle = (base >> 16) & 0xFF;
	gdt[ind].base_high = (base >> 24) & 0xFF;
	gdt[ind].limit = limit & 0xFFFF; 
	gdt[ind].flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);
	gdt[ind].access = access;
}

extern void GdtFlush(gdt_ptr_t*);

void GdtInit() {
	gdt_ptr.limit = sizeof(gdt_entry_t) * 3 - 1;
	gdt_ptr.base = (uint32_t)&gdt; // stfu im on 32 bit
	SetGdtSegment(0, 0, 0, 0, 0); 					// NULL segment
	SetGdtSegment(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// kernel code segment
	SetGdtSegment(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	// kernel data segment
	// SetGdtSegment(3, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// user code segment
	// SetGdtSegment(4, 0, 0xFFFFFFFF, 0x92, 0xCF);	// user data segment
	GdtFlush(&gdt_ptr);
}

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
	uint32_t base;
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

void* memset(void* ptr, int value, size_t num) {
	uint8_t* pbyte = (uint8_t*)ptr;
	for (size_t i = 0; i < num; ++i, ++pbyte) {
		*pbyte = value;
	}
	return ptr;
}

void IdtInit() {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = (uint32_t)&idt; // stfu lsp
    memset(&idt, 0, sizeof(idt_entry_t) * 256);
	IdtLoad(&idt_ptr);
}

// ---------------------------------------

volatile uint16_t* vga_buf = (uint16_t*)0xB8000;
int term_color = 0x0F; // Black background, White foreground

void kernel_main() {
	GdtInit();
	IdtInit();

	vga_buf[0] = ((uint16_t)term_color << 8) | 'W';
}
