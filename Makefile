# all: clean boot kernel linker run
all: clean kernel run

clean:
	rm -rf *.o
	rm -rf *.bin

kernel:
	i686-elf-as boot.s -o boot.o
	i686-elf-as dtload.s -o dtload.o
	i686-elf-as isr.s -o isr.o -msyntax=intel

	i686-elf-gcc -c -m32 kernel.c -o kernel.o -ffreestanding -std=gnu99

 # -Wall -Wextra -g -O2 

# boot:

# linker:
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -nostdlib kernel.o boot.o dtload.o isr.o -lgcc

run:
	qemu-system-i386 -kernel myos.bin
