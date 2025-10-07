all: clean boot kernel linker run

clean:
	rm -rf *.o
	rm -rf *.bin

boot:
	i686-elf-as boot.s -o boot.o
	i686-elf-as gdt_flush.s -o gdt_flush.o

kernel:
	i686-elf-gcc -c -m32 kernel.c -o kernel.o -ffreestanding -std=gnu99

 # -Wall -Wextra -g -O2 

linker:
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -nostdlib boot.o kernel.o gdt_flush.o -lgcc

run:
	qemu-system-i386 -kernel myos.bin
