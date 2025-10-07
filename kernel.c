#include <stddef.h>
#include <stdint.h>

// gdt
// ---------------------------------------
struct gdt_entry {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_t {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr_t gdt_ptr;

void set_gdt_gate(uint32_t ind, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[ind].base_low = base & 0xFFFF;
	gdt[ind].base_middle = (base >> 16) & 0xFF;
	gdt[ind].base_high = (base >> 24) & 0xFF;
	gdt[ind].limit = limit & 0xFFFF; 
	gdt[ind].flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);
	gdt[ind].access = access;
}

extern void gdt_flush(struct gdt_ptr_t*);

// void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
//     /* Setup the descriptor base address */
//     gdt[num].base_low = (base & 0xFFFF);
//     gdt[num].base_middle = (base >> 16) & 0xFF;
//     gdt[num].base_high = (base >> 24) & 0xFF;
//
//     /* Setup the descriptor limits */
//     gdt[num].limit_low = (limit & 0xFFFF);
//     gdt[num].granularity = ((limit >> 16) & 0x0F);
//
//     /* Finally, set up the granularity and access flags */
//     gdt[num].granularity |= (gran & 0xF0);
//     gdt[num].access = access;
// }

// ---------------------------------------

volatile uint16_t* vga_buf = (uint16_t*)0xB8000;
int term_color = 0x0F; // Black background, White foreground

void kernel_main() {
	// gdt init
	gdt_ptr.limit = sizeof(struct gdt_entry) * 3 - 1;
	gdt_ptr.base = (uint32_t)&gdt; // stfu im on 32 bit
	set_gdt_gate(0, 0, 0, 0, 0); 					  // NULL segment
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
	// set_gdt_gate(3, 0, 0xFFFFFFFF, 0b11111010, 0xCF); // user code segment
	// set_gdt_gate(4, 0, 0xFFFFFFFF, 0b10010010, 0xCF); // user data segment
	gdt_flush(&gdt_ptr);

	vga_buf[0] = ((uint16_t)term_color << 8) | 'W';
}
