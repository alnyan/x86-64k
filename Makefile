LOADER_SOURCES_S=src/loader/mem/pm_s.s src/loader/mem/pmpae_s.s
LOADER_SOURCES_CXX=$(shell find src/loader -name *.cpp -type f)
LOADER_OBJECTS=$(LOADER_SOURCES_S:src/loader/%.s=build/loader/%.o) $(LOADER_SOURCES_CXX:src/loader/%.cpp=build/loader/%.o)
LOADER_HEADERS=$(shell find src/loader -name *.hpp -type f)

LOADER_CXXFLAGS=-ffreestanding \
			    -ggdb \
				-Isrc/loader \
				-fno-rtti \
				-fno-exceptions \
				-fno-builtin \
				-Wall \
				-Wextra \
				-Werror \
				-Wold-style-cast \
				-Wno-unused-variable \
				-Wno-unused-but-set-variable \
				-Wno-unused-parameter \
				-Wimplicit-fallthrough \
				-std=c++1z

LOADER_ASFLAGS=--32

LOADER_LDFLAGS=-nostdlib \
			   -O0 \
			   -ggdb \
			   -Tloader.lds \
			   -ffreestanding

KERNEL_CXXFLAGS=$(LOADER_CXXFLAGS)
KERNEL_LDFLAGS=-nostdlib \
			   -O0 \
			   -ggdb \
			   -Tkernel.lds \
			   -ffreestanding

LD32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++
AS32=$(TOOLCHAIN32)/bin/$(TARGET32)-as
CXX32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++

LD64=$(TOOLCHAIN64)/bin/$(TARGET64)-g++
AS64=$(TOOLCHAIN64)/bin/$(TARGET64)-as
CXX64=$(TOOLCHAIN64)/bin/$(TARGET64)-g++

LOADER_CRTBEGIN=$(shell $(CXX32) -print-file-name=crtbegin.o)
LOADER_CRTEND=$(shell $(CXX32) -print-file-name=crtend.o)
LOADER_LIBGCC=$(shell $(CXX32) --print-libgcc-file-name)

KERNEL_SOURCES_CXX=$(shell find src/kernel -name *.cpp -type f)
KERNEL_OBJECTS=$(KERNEL_SOURCES_CXX:src/kernel/%.cpp=build/kernel/%.o)

DIRS=build/iso/boot/grub build/loader/boot $(shell dirname $(LOADER_OBJECTS)) $(shell dirname $(KERNEL_OBJECTS))

all: build/image.iso

mkdirs:
	@mkdir -p $(DIRS)

build/loader.elf: mkdirs build/loader/boot/multiboot.o build/loader/crti.o build/loader/crtn.o $(LOADER_OBJECTS)
	$(LD32) -o $@ $(LOADER_LDFLAGS) \
	    build/loader/boot/multiboot.o \
		build/loader/crti.o \
		$(LOADER_CRTBEGIN) \
		$(LOADER_OBJECTS) \
		$(LOADER_LIBGCC) \
		$(LOADER_CRTEND) \
		build/loader/crtn.o

build/kernel.elf: mkdirs $(KERNEL_OBJECTS)
	$(LD64) -o $@ $(KERNEL_LDFLAGS) $(KERNEL_OBJECTS)

build/loader/%.o: src/loader/%.s
	$(AS32) -o $@ $(LOADER_ASFLAGS) $<

build/loader/%.o: src/loader/%.cpp $(LOADER_HEADERS)
	$(CXX32) -c -o $@ $(LOADER_CXXFLAGS) $<

build/kernel/%.o: src/kernel/%.cpp $(KERNEL_HEADERS)
	$(CXX64) -c -o $@ $(KERNEL_CXXFLAGS) $<

build/image.iso: build/loader.elf build/kernel.elf src/grub/grub.cfg
	cp build/loader.elf build/iso/boot/loader.elf
	cp src/grub/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ build/iso

qemu-run: build/image.iso
	qemu-system-i386 -cdrom build/image.iso -m 512 -enable-kvm -serial stdio

qemu-gdb: build/image.iso
	qemu-system-i386 -cdrom build/image.iso -m 512 -S -s -serial stdio

clean:
	rm -rf build
