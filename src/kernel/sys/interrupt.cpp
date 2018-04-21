#include <sys/interrupt.hpp>
#include <dev/pic.hpp>
#include <algo/memory.hpp>
#include <sys/debug.hpp>

typedef struct __attribute__((packed)) __attribute__((aligned(16))) {
    uint16_t limit;
    uint64_t base;
} idtr_t;

typedef struct __attribute__((packed)) __attribute__((aligned(16))) {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
} idt_entry_t;

static idtr_t m_idtr;
static idt_entry_t m_idt_entries[ISR_MAX_INTERRUPTS];

void isr_init(void) {
    memset(m_idt_entries, 0, sizeof(m_idt_entries));
    pic_remap(ISR_IRQ_BASE, ISR_IRQ_BASE + 8);

    m_idtr.limit = sizeof(m_idt_entries) - 1;
    m_idtr.base = reinterpret_cast<uint64_t>(m_idt_entries);
}

void isr_load_and_unmask(void) {
    __asm__ volatile ( "lidt %0\n\
                        in $0x70, %%al\n\
                        and $0x7f, %%al\n\
                        out %%al, $0x70" :: "m"(m_idtr));
}

void isr_set_handler(int interrupt, interrupt_handler handler, int rpl) {
    if (!handler) {
        memset(&m_idt_entries[interrupt], 0, sizeof(idt_entry_t));
        return;
    }
    
    debug::printf("m_idt at %a\n", m_idt_entries);
    debug::printf("setting interrupt %x\n", interrupt);
    debug::printf("pwning entry at %a\n", &m_idt_entries[interrupt]);
    debug::printf("entry size: %d\n", sizeof(idt_entry_t));
    m_idt_entries[interrupt].offset_1 = reinterpret_cast<uintptr_t>(handler) & 0xffff;
    m_idt_entries[interrupt].offset_2 = (reinterpret_cast<uintptr_t>(handler) >> 16) & 0xffff;
    m_idt_entries[interrupt].offset_3 = (reinterpret_cast<uintptr_t>(handler) >> 32) & 0xffffffff;
    m_idt_entries[interrupt].selector = 8;
    m_idt_entries[interrupt].zero = 0;
    m_idt_entries[interrupt].ist = 0;
    m_idt_entries[interrupt].type_attr = 0x8e | ((rpl & 3) << 5);
}

void isr_set_error_handler(int interrupt, interrupt_error_handler handler) {
    if (!handler) {
        memset(&m_idt_entries[interrupt], 0, sizeof(idt_entry_t));
        return;
    }
        
    m_idt_entries[interrupt].offset_1 = reinterpret_cast<uintptr_t>(handler) & 0xffff;
    m_idt_entries[interrupt].offset_2 = (reinterpret_cast<uintptr_t>(handler) >> 16) & 0xffff;
    m_idt_entries[interrupt].offset_3 = (reinterpret_cast<uintptr_t>(handler) >> 32) & 0xffffffff;
    m_idt_entries[interrupt].selector = 8;
    m_idt_entries[interrupt].zero = 0;
    m_idt_entries[interrupt].ist = 0;
    m_idt_entries[interrupt].type_attr = 0x8e;
}
