#ifndef _ARMV8_VMSA_H
#define _ARMV8_VMSA_H

typedef union {
    struct {
        uint64_t type : 2;
        uint64_t attridx : 3;
        uint64_t ns : 1;
        uint64_t ap2_1 : 2;
        uint64_t sh1_0 : 2;
        uint64_t af : 1;
        uint64_t ng : 1;
        uint64_t pa : 36;
        uint64_t res0 : 4;
        uint64_t contig : 1;
        uint64_t pxn : 1;
        uint64_t xn : 1;
    };
    uint64_t raw;
} armv8_4k_pg_pte_t;

typedef union {
    struct {
        uint64_t type : 2;
        uint64_t ignored0 : 10;
        uint64_t pa : 36;
        uint64_t res0 : 4;
        uint64_t ignored1 : 7;
        uint64_t pxn : 1;
        uint64_t xn : 1;
        uint64_t ap : 2;
        uint64_t ns : 1;
    };
    uint64_t raw;
} armv8_4k_tbl_pte_t;

#endif
