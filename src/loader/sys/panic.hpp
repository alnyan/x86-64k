#pragma once
#include <sys/debug.hpp>

#define panic_msg(msg, ...) { debug::printf("%s:%d: ", __FILE__, __LINE__); debug::printf(msg, ##__VA_ARGS__); panic(); }

[[noreturn]] void panic();
