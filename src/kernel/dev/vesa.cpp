#include <dev/vesa.hpp>
#include <sys/debug.hpp>
#include <sys/panic.hpp>
#include <sys/emu/emu16_64.hpp>

using namespace debug;

struct ivt_entry {
    uint16_t offset;
    uint16_t segment;
};

#define VESA_VIDEO_INTERRUPT 0x10

#define MMAP_BIOS_ZONE_START 0xa0000
#define MMAP_BIOS_ZONE_END 0xfffff

#define HANDLER_PTR_LOOKS_LIKE_VALID(ptr) (((ptr) >= MMAP_BIOS_ZONE_START) && ((ptr) <= MMAP_BIOS_ZONE_END))
#define RETURN_SEGMENT_MAGIC 0xdead
#define RETURN_OFFSET_MAGIC 0xbabe

#define REALMODE_SEGMENT_MULTIPLIER 0x10
#define CPU_SEGMENT_BOUNDARY 0x10000
#define CPU_REAL_TO_LINEAR(seg, ofs) ((seg) * REALMODE_SEGMENT_MULTIPLIER + (ofs))
#define CPU_LINEAR_TO_OFFSET(seg, ofs) ((ofs) - ((seg) * REALMODE_SEGMENT_MULTIPLIER))

ivt_entry entry;

//emu::Emulator16 emulator;

static void initMore(ivt_entry handler) {
/*    printf("vesa: setting mode 0x3, screen should wipe\n");
    
    // using conventional memory for stack
    emulator.ss = 0x8e0;
    emulator.esp = 0xfff0;

    // poking magic return pointer
    emulator.pokew(emulator.ss, emulator.esp, RETURN_OFFSET_MAGIC);
    emulator.pokew(emulator.ss, emulator.esp + 2, RETURN_SEGMENT_MAGIC);

    // setting IP on handler
    emulator.cs = handler.segment;
    emulator.eip = handler.offset;

    // setting command - modeset 3
    emulator.eax = 3;

    // executing instructions
    do {
        emulator.step();
    } while (((emulator.eip & 0xffff) != RETURN_OFFSET_MAGIC) || (emulator.cs != RETURN_SEGMENT_MAGIC));
*/}

void vesa::init() {
    printf("vesa: observing IVT\n");
    ivt_entry entry = *reinterpret_cast<ivt_entry*>(VESA_VIDEO_INTERRUPT * sizeof(ivt_entry));
    uintptr_t handlerPtr = CPU_REAL_TO_LINEAR(entry.segment, entry.offset);
    bool valid = HANDLER_PTR_LOOKS_LIKE_VALID(handlerPtr);
    printf("vesa: int 10h handler located in %04x:%04x, linear: %a, seems %s\n", entry.segment, entry.offset, handlerPtr, 
        valid ? "ok" : "not very valid");

    if (valid) {
        initMore(entry);
    }
}
