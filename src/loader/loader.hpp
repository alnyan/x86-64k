#pragma once
#include <stdint.h>

#define LOADER_MAGIC 0x310AD007

struct LoaderData {
    uint32_t loaderMagic;
};
