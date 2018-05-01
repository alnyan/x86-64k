#pragma once
#include <sys/debug.hpp>
#include <dev/chardev.hpp>

namespace debug {
    const unsigned MAX_DEBUG_DEVICES = 2;

    void init();
    void regOutDev(devices::CharDevice *dev);
    bool unregOutDev(devices::CharDevice *dev); 
}
