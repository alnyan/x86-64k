#pragma once
#include <sys/debug.hpp>

#define panic_msg(msg) { debug::printf("%s:%d: %s\n", __FILE__, __LINE__, msg); panic(); }

[[noreturn]] void panic();
