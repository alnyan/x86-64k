#pragma once
#include <stdint.h>

#define LOADER_MAGIC 0x310AD007

struct LoaderData {
    uint32_t loaderMagic;
    uint32_t loaderPagingBase;
    uint32_t loaderPagingSize;
    uint32_t loaderPagingTracking;
    uint32_t multibootInfo;
    uint8_t checksum;
} __attribute__((packed));
