#include <sys/isr.hpp>
#include <sys/interrupt.hpp>
#include <sys/debug.hpp>
#include <dev/pic.hpp>
//#include <dev/ps2kbd.h>
//#include <sys/mt/mt.h>

/*static void __attribute__ ((interrupt)) kbd_handler(interrupt_frame* _) {
	char ch = ps2_read_char_if_available();
	if (!PS2_IS_BAD_CHAR(ch)) {
		ps2_notify_char_pressed(ch);
	}

	DO_EOI_IF_NEEDED(1);
}*/

/*static void __attribute__ ((interrupt)) gp_fault_handler(interrupt_frame* _, uint32_t errorCode) {
	dprintf("gp error code: %d\n", errorCode);
	dpanic("gp fault");
}*/

static void __attribute__ ((interrupt)) test_handler(interrupt_frame_t* _) {
	debug::dprintf("c8 handled!\n");
}

extern "C" void test_handler_ring3(void) {
	debug::dprintf("c9 handled!\n");
}

extern "C" void test_handler_ring3_s(interrupt_frame_t* _);

void isr_setup_handlers() {
    isr_init();
    
	isr_set_handler(0xc8, test_handler, 3);
	isr_set_handler(0xc9, test_handler_ring3_s, 3);

    //isr_set_handler(ISR_IRQ_BASE, mt_irq0_handler, 0);
    //isr_set_handler(ISR_IRQ_BASE + 1, kbd_handler, 0);

    //isr_set_error_handler(0xd, gp_fault_handler);
    
    isr_load_and_unmask();
}
