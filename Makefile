all: clean boot kernel linker

clean:
	rm -rf *.o
	rm -rf *.bin

boot:
	i686-elf-as boot.s -o boot.o
	i686-elf-as gdt_flush.s -o gdt_flush.o

kernel:
	i686-elf-gcc -g -c kernel.c -o kernel.o -std=gnu99 -ffreestanding # -O2 -Wall -Wextra

linker:
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o gdt_flush.o -lgcc

run:
	qemu-system-i386 -kernel myos.bin
