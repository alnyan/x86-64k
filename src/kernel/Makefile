KERNEL_ALL_SOURCES_S=$(shell find . -name "*.s" -printf "%P\n" -type f)
KERNEL_SOURCES_S=$(filter-out crti.s crtn.s,$(KERNEL_ALL_SOURCES_S))
KERNEL_SOURCES_CXX=$(shell find . -name "*.cpp" -printf "%P\n" -type f)
KERNEL_SOURCES_CXX_NOFLOAT=$(shell find . -name "*.cxx" -printf "%P\n" -type f)
KERNEL_HEADERS=$(shell find . -name "*.hpp" -printf "%P\n" -type f)
KERNEL_OBJECTS=$(KERNEL_SOURCES_S:%.s=$(BUILD_DIR)/kernel/%.o) $(KERNEL_SOURCES_CXX:%.cpp=$(BUILD_DIR)/kernel/%.o) $(KERNEL_SOURCES_CXX_NOFLOAT:%.cxx=$(BUILD_DIR)/kernel/%.o)

KERNEL_CXXFLAGS=-ffreestanding \
				-ggdb \
				-I. \
				-fno-rtti \
				-fno-exceptions \
				-Wall \
				-Wextra \
				-Werror \
				-Wold-style-cast \
				-Wno-unused-variable \
				-Wno-unused-but-set-variable \
				-Wno-unused-parameter \
				-Wimplicit-fallthrough \
				-std=c++1z \
				-mcmodel=kernel \
				-mno-red-zone \
				-static \
				-z max-page-size=0x1000

KERNEL_CXXFLAGS_NOFLOAT= -msoft-float \
	-mno-mmx \
	-mno-sse \
	-mno-sse2

KERNEL_ASFLAGS=--64

KERNEL_LDFLAGS=-nostdlib \
			   -O0 \
			   -ggdb \
                           -Tkernel.lds \
			   -ffreestanding \
			   -mcmodel=kernel \
			   -z max-page-size=0x1000

LD64=$(TOOLCHAIN64)/bin/$(TARGET64)-g++
AS64=$(TOOLCHAIN64)/bin/$(TARGET64)-as
CXX64=$(TOOLCHAIN64)/bin/$(TARGET64)-g++

KERNEL_CRTBEGIN=$(shell $(CXX64) -print-file-name=crtbegin.o)
KERNEL_CRTEND=$(shell $(CXX64) -print-file-name=crtend.o)

DIRS=$(shell dirname $(KERNEL_OBJECTS)) $(BUILD_DIR)/docs/kernel

all: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: mkdirs $(BUILD_DIR)/kernel/crti.o $(BUILD_DIR)/kernel/crtn.o $(KERNEL_OBJECTS)
	$(LD64) -o $@ $(KERNEL_LDFLAGS) \
		$(BUILD_DIR)/kernel/crti.o \
		$(KERNEL_CRTBEGIN) \
		$(KERNEL_OBJECTS) \
		$(KERNEL_CRTEND) \
		-lc \
		-lg \
		-lm \
		-lnosys \
		-lgcc \
		-lsupc++ \
		-lstdc++ \
		$(BUILD_DIR)/kernel/crtn.o


$(BUILD_DIR)/kernel/%.o: %.s
	$(AS64) -o $@ $(KERNEL_ASFLAGS) $<

$(BUILD_DIR)/kernel/%.o: %.cpp $(KERNEL_HEADERS)
	$(CXX64) -c -o $@ $(KERNEL_CXXFLAGS) $<

$(BUILD_DIR)/kernel/%.o: %.cxx $(KERNEL_HEADERS)
	$(CXX64) -c -o $@ $(KERNEL_CXXFLAGS) $(KERNEL_CXXFLAGS_NOFLOAT) $<

docs: $(KERNEL_SOURCES_CXX) $(KERNEL_HEADERS) mkdirs
	doxygen doxygen.conf

mkdirs:
	@mkdir -p $(DIRS)
