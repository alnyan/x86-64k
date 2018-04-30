#pragma once

#ifndef NO_STDCXX
#include <cstdint>
#else
#include <stdint.h>
#endif

#define ALIGNED(alignment) __attribute__((__aligned__(alignment)))
#define ALIGNED_STRUCT(alignment) struct __attribute__((__aligned__(alignment)))
#define PACKED_STRUCT struct __attribute__((__packed__))
#define PACKED_AND_ALIGNED_STRUCT(alignment) struct __attribute__((__packed__)) __attribute__((__aligned__(alignment)))

using bit_t = uint8_t;

enum page_struct_flags_t : uint64_t {
    PTSE_FLAG_PRESENT       = 1ull << 0,
    PTSE_FLAG_RW            = 1ull << 1,
    PTSE_FLAG_RING3         = 1ull << 2,
    PTSE_FLAG_WRITE_THROUGH = 1ull << 3,
    PTSE_FLAG_CACHE_DISABLE = 1ull << 4,
    PTSE_FLAG_ACCESSED      = 1ull << 5,
    PTSE_FLAG_DIRTY         = 1ull << 6,
    PTSE_FLAG_PAGE_PAT      = 1ull << 7,
    PTSE_FLAG_SIZE          = 1ull << 7,
    PTSE_FLAG_GLOBAL        = 1ull << 8,
    PTSE_FLAG_PD_PDPT_PAT   = 1ull << 12,
    PTSE_FLAG_NO_EXECUTE    = 1ull << 63
};

const int PTSE_SIZEOF = 0x1000;
const int PTSE_PAGE_SIZE = 0x1000;
const int PTSE_ENTRIES32 = PTSE_SIZEOF / sizeof(uint32_t);
const int PTSE_ENTRIES64 = PTSE_SIZEOF / sizeof(uint64_t);
const int PTSE_PDPT32_ENTRIES = 4;
const int PTSE_PDPT32_ALIGNMENT = 0x20;
#define PTSE_ALIGNED ALIGNED(PTSE_SIZEOF)
#define PTSE_STRUCT ALIGNED_STRUCT(PTSE_SIZEOF)

union page_table_entry32_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        bit_t dirty : 1;
        bit_t zero : 1;
        bit_t global : 1;
        uint8_t ignored : 3;
    } flags;
    uint32_t address;
};

union page_table_entry64_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        bit_t dirty : 1;
        bit_t pat : 1;
        bit_t global : 1;
        uint8_t ignored : 3;
        uint64_t addressAndReserved: 40;
        uint16_t ignored2 : 11;
        bit_t noExecute: 1;
    } flags;
    uint64_t address;
};

union page_directory_entry32_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        bit_t dirty : 1;
        bit_t size : 1;
        bit_t global : 1;
        uint8_t ignored : 3;
        bit_t pat : 1;
    } flags;
    uint32_t address;
};

union page_directory_entry64_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        bit_t dirty : 1;
        bit_t size : 1;
        bit_t global : 1;
        uint8_t ignored : 3;
        bit_t pat : 1;
        uint64_t addressAndReserved: 39;
        uint16_t ignored2 : 11;
        bit_t noExecute: 1;
    } flags;
    uint64_t address;
};

union pdpt_entry_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        bit_t dirty : 1;
        bit_t size : 1;
        bit_t global : 1;
        uint8_t ignored1 : 3;
        bit_t pat : 1;
        uint64_t addressAndReserved: 39;
        uint16_t ignored2 : 11;
        bit_t noExecute: 1;
    } flags;
    uint64_t address;
};

union pml4_entry_t {
    struct __attribute__ ((packed))  {
        bit_t present : 1;
        bit_t rw : 1;
        bit_t ring3 : 1;
        bit_t writeThrough : 1;
        bit_t noCache : 1;
        bit_t accessed : 1;
        uint8_t reserved : 3;
        uint8_t ignored1 : 3;
        uint64_t addressAndReserved: 40;
        uint16_t ignored2 : 11;
        bit_t noExecute: 1;
    } flags;
    uint64_t address;
};

/*PTSE_STRUCT page_directory32_t {
    page_table_entry32_t entries[PTSE_SIZEOF / sizeof(page_table_entry32_t)];
};

PTSE_STRUCT page_directory64_t {
    page_table_entry64_t entries[PTSE_SIZEOF / sizeof(page_table_entry64_t)];
};

ALIGNED_STRUCT(PTSE_PDPT32_ALIGNMENT) pdpt32_t {
    page_directory64_t entries[PTSE_PDPT32_ENTRIES];
};

PTSE_STRUCT pdpt64_t {
    page_directory64_t entries[PTSE_SIZEOF / sizeof(page_directory64_t)];
};

PTSE_STRUCT pml4_t {
    pdpt64_t entries[PTSE_SIZEOF / sizeof(pdpt64_t)];
};*/