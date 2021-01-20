cflags-obj=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -Werror -nostdlib -Iinclude -Iinclude/klibc -Iinclude/driver -Wl,--build-id=none -no-pie -mno-80387 -mgeneral-regs-only 

qemu=/home/tjz/software/qemu/build/./qemu-system-i386  
qemu=sudo qemu-system-i386
target=build/kernel.bin
build=build/

# These will be in the form of `./kernel/vga/vga.c ./kernel/mm/kmalloc.c ./kernel/mm/virt-mm.c'
c-src=$(shell find . -name '*.c')
asm-src=$(shell find ./kernel/asm -name '*.s')

# These will be in the form of `build/vga.o build/kernel.o build/phy-mm.o ...'
c-obj=$(addprefix $(build), $(subst .c,.o, $(notdir $(c-src))))	 
asm-obj=$(build)boot.o

# Collection of all objects
obj=$(asm-obj) $(c-obj)

# set search path for %.c and %.s wildcards to the subdirs of sources
vpath %.c $(dir $(c-src))
vpath %.s $(dir $(asm-src))

run: kernel
	echo $(c-src)
	$(qemu) -cdrom build/os2.iso -m 256 -netdev tap,id=n1 -device e1000,netdev=n1 -object filter-dump,id=id,netdev=n1,file=./build/dump.pcap 


debug: kernel
	$(qemu) -s -S -kernel $(target) -m 256 

kernel: clean $(obj) 
	i686-linux-gnu-gcc -T linker.ld -o $(target) $(cflags-obj) $(obj) 
	mkdir -p build/iso/boot/grub
	echo "menuentry \"os2\" {\n\
		multiboot /boot/kernel.bin\n\
	}" >> build/iso/boot/grub/grub.cfg
	cp $(target) build/iso/boot
	grub-mkrescue -o build/os2.iso build/iso
	# qemu-system-i386 -kernel $(target)

linux: 
	$(qemu) -cdrom /home/tjz/Downloads/Core-current.iso

target:  kernel
	sudo mkdir -p /mnt/efi
	sudo mount -a /dev/sdb1 /mnt/efi
	sudo mv $(target) /mnt/efi/EFI

build/%.o : %.c
	i686-linux-gnu-gcc $< $(cflags-obj) -c -o $@

build/%.o : %.s
	i686-linux-gnu-gcc -x assembler-with-cpp $< $(cflags-obj) -c -o $@

linux-malloc-test:
	make -C kernel/mm linux-malloc-test

clean: 
	rm -rf build
	mkdir build
	make -C ./kernel/mm clean
