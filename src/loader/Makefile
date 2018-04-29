LOADER_SOURCES_S=mem/pm_s.s mem/pmpae_s.s
LOADER_SOURCES_CXX=$(shell find . -name "*.cpp" -printf "%P\n" -type f)
LOADER_OBJECTS=$(LOADER_SOURCES_S:%.s=$(BUILD_DIR)/loader/%.o) $(LOADER_SOURCES_CXX:%.cpp=$(BUILD_DIR)/loader/%.o)
LOADER_HEADERS=$(shell find . -name "*.hpp" -printf "%P\n" -type f)

LOADER_CXXFLAGS=-ffreestanding \
			    -ggdb \
				-I. \
				-I../common \
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

LD32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++
AS32=$(TOOLCHAIN32)/bin/$(TARGET32)-as
CXX32=$(TOOLCHAIN32)/bin/$(TARGET32)-g++

LOADER_CRTBEGIN=$(shell $(CXX32) -print-file-name=crtbegin.o)
LOADER_CRTEND=$(shell $(CXX32) -print-file-name=crtend.o)
LOADER_LIBGCC=$(shell $(CXX32) --print-libgcc-file-name)

DIRS=$(shell dirname $(LOADER_OBJECTS)) $(BUILD_DIR)/loader/boot

$(BUILD_DIR)/loader.elf: mkdirs $(BUILD_DIR)/loader/boot/multiboot.o $(BUILD_DIR)/loader/crti.o $(BUILD_DIR)/loader/crtn.o $(LOADER_OBJECTS)
	$(LD32) -o $@ $(LOADER_LDFLAGS) \
	    $(BUILD_DIR)/loader/boot/multiboot.o \
		$(BUILD_DIR)/loader/crti.o \
		$(LOADER_CRTBEGIN) \
		$(LOADER_OBJECTS) \
		$(LOADER_LIBGCC) \
		$(LOADER_CRTEND) \
		$(BUILD_DIR)/loader/crtn.o

$(BUILD_DIR)/loader/%.o: %.s
	$(AS32) -o $@ $(LOADER_ASFLAGS) $<

$(BUILD_DIR)/loader/%.o: %.cpp $(LOADER_HEADERS)
	$(CXX32) -c -o $@ $(LOADER_CXXFLAGS) $<

mkdirs:
	@mkdir -p $(DIRS)
