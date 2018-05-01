#include <sys/lodebug.hpp>
#include <algo/itoa.hpp>
#include <cstring>
#include <dev/io.hpp>

#define BOCHS_CON_PORT 0xe9

devices::CharDevice* s_debug_devs[debug::MAX_DEBUG_DEVICES];

static void broadcast(char ch) {
    for (unsigned i = 0; i < debug::MAX_DEBUG_DEVICES; i++) {
        if (s_debug_devs[i] != nullptr) s_debug_devs[i]->putchar(ch);
    }
#ifndef NO_BOCHS_BROADCAST
    io::outb(BOCHS_CON_PORT, ch);
#endif
}

void debug::init() {
    memset(s_debug_devs, 0, sizeof(s_debug_devs));
}

void debug::regOutDev(devices::CharDevice *dev) {
    for (unsigned i = 0; i < debug::MAX_DEBUG_DEVICES; i++) {
        if (s_debug_devs[i] == nullptr) {
            s_debug_devs[i] = dev;
            return;
        }
    }
}

void debug::dputchar(char ch) {
    broadcast(ch);
}

void debug::dhalt() {
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}