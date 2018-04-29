#pragma once
#include <sys/types.h>

extern "C" void *memset(void *block, int value, size_t count);
extern "C" void *memcpy(void *dst, const void *src, size_t count);
