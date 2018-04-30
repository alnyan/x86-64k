#include <sys/gdt.hpp>
#include <stdint.h>
#include <cstring>

typedef struct __attribute__ ((packed)) gdt_tss {
    uint32_t rsvd1;
    void* rsp0;
    uint64_t unused0[3];
    uint64_t ist1;
    uint32_t unused1[18];
    uint16_t iopb;
    uint16_t unused2;
} gdt_tss_t;

typedef uint64_t gdt_entry_t;
typedef struct {
    uint16_t limit;
    uint64_t addr;
}__attribute__((packed)) gdtr_t;

static gdt_entry_t s_gdt[7];
static gdtr_t s_gdtr;
static gdt_tss_t s_tss;

extern "C" void gdt_load(gdtr_t *gdtr);
extern "C" void tr_load(void);

void gdt::init() {
    // TODO change to bitfields
    s_gdt[0] = 0x000100000000FFFF;
    s_gdt[1] = 0x00AF9A0000000000;
    s_gdt[2] = 0x0000920000000000;
    s_gdt[3] = 0x00AFFA0000000000;
    s_gdt[4] = 0x0000F20000000000;
    s_gdt[5] = 0x0080890000000068 | ((reinterpret_cast<uint64_t>(&s_tss) & 0xffffffull) << 16) | ((reinterpret_cast<uint64_t>(&s_tss) & 0xff000000ull) << 32);
    s_gdt[6] = reinterpret_cast<uint64_t>(&s_tss) >> 32; // not sure

    s_gdtr.addr = reinterpret_cast<uint64_t>(s_gdt);
    s_gdtr.limit = sizeof(s_gdt) - 1;
    
    gdt_load(&s_gdtr);

    memset(&s_tss, 0, sizeof(s_tss));
    s_tss.iopb = sizeof(s_tss);
    tr_load();
}

void gdt::setTaskswitchStack(void *stackPtr) {
    // TODO check stack pointer
    s_tss.rsp0 = stackPtr;
}

void *gdt::taskswitchStack() { return s_tss.rsp0; }