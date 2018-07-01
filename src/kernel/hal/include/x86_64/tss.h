#include <stdint.h>

typedef struct tss_entry {
    uint32_t _reserved0;
    uint32_t rsp0_0_31;
    uint32_t rsp0_32_63;
    uint32_t rsp1_0_31;
    uint32_t rsp1_32_63;
    uint32_t rsp2_0_31;
    uint32_t rsp2_32_63;
    uint64_t _reserved1;
    uint32_t ist1_0_31;
    uint32_t ist1_32_63;
    uint32_t ist2_0_31;
    uint32_t ist2_32_63;
    uint32_t ist3_0_31;
    uint32_t ist3_32_63;
    uint32_t ist4_0_31;
    uint32_t ist4_32_63;
    uint32_t ist5_0_31;
    uint32_t ist5_32_63;
    uint32_t ist6_0_31;
    uint32_t ist6_32_63;
    uint32_t ist7_0_31;
    uint32_t ist7_32_63;
    uint64_t _reserved2;
    uint32_t _reserved3;
    uint32_t iomap;
} __attribute__ ((packed)) tss_entry_t;
