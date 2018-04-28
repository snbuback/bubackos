#include <stdint.h>

typedef struct tss_entry {
    uint32_t _reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t _reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t _reserved2;
    uint32_t _reserved3;
    uint32_t iomap;
} __attribute__ ((packed)) tss_entry_t;
