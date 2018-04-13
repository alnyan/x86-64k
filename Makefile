LOADER_SOURCES_S=$(shell find src/loader -name *.s -type f)
LOADER_SOURCES_CXX=$(shell find src/loader -name *.cpp -type f)
LOADER_OBJECTS=$(LOADER_SOURCES_S:src/loader/%.s=build/loader/%.o) $(LOADER_SOURCES_CXX:src/loader/%.cpp=build/loader/%.o)

LOADER_CXXFLAGS=-ffreestanding \
			  -ggdb

LOADER_ASFLAGS=--32

LOADER_LDFLAGS=-nostdlib \
			   -O0 \
			   -ggdb \
			   -Tloader.lds

LD32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++
AS32=$(TOOLCHAIN32)/bin/$(TARGET32)-as
CXX32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++

DIRS=build/iso/boot/grub $(shell dirname $(LOADER_OBJECTS))

all: build/image.iso

mkdirs:
	@mkdir -p $(DIRS)

build/loader.elf: mkdirs $(LOADER_OBJECTS)
	$(LD32) -o $@ $(LOADER_LDFLAGS) $(LOADER_OBJECTS)

build/loader/%.o: src/loader/%.s
	$(AS32) -o $@ $(LOADER_ASFLAGS) $<

build/loader/%.o: src/loader/%.cpp
	$(CXX32) -c -o $@ $(LOADER_CXXFLAGS) $<

build/image.iso: build/loader.elf src/grub/grub.cfg
	cp build/loader.elf build/iso/boot/loader.elf
	cp src/grub/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ build/iso

qemu-run: build/image.iso
	qemu-system-x86_64 -cdrom build/image.iso -m 512 -enable-kvm
