#include <stddef.h>
#include <stdint.h>

volatile uint16_t* vga_buf = (uint16_t*)0xB8000;
int term_color = 0x0F; // Black background, White foreground

void kernel_main() {
	vga_buf[0] = ((uint16_t)term_color << 8) | 'W';
}
