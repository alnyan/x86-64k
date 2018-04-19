
all: build/image.iso

mkdirs:
	@mkdir -p build/iso/boot/grub

build/loader.elf: mkdirs
	@BUILD_DIR=$(realpath build) make -C src/loader

build/kernel.elf: mkdirs
	@BUILD_DIR=$(realpath build) make -C src/kernel

build/image.iso: build/loader.elf build/kernel.elf src/grub/grub.cfg
	cp build/loader.elf build/iso/boot/loader.elf
	cp build/kernel.elf build/iso/boot/kernel.elf
	cp src/grub/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ build/iso

qemu-run: build/image.iso
	qemu-system-x86_64 -cdrom build/image.iso -m 512 -cpu qemu64,-vmx -serial stdio

qemu-gdb: build/image.iso
	qemu-system-x86_64 -cdrom build/image.iso -m 512 -cpu qemu64,-vmx -S -s -serial stdio

clean:
	rm -rf build

docs: mkdirs
	@BUILD_DIR=$(realpath build) make -C src/kernel docs

