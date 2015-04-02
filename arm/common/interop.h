#ifndef _INTEROP_H
#define _INTEROP_H
#include <stdint.h>
#include <stddef.h>

#define OP_MAP_NSEC_EL0 0
#define OP_MAP_SEC_EL0  1
#define OP_MAP_NSEC_EL1 2
#define OP_MAP_SEC_EL1  4
#define OP_MAP_EL3      8
#define OP_MAP_ALL \
            (OP_MAP_SEC_EL0 | OP_MAP_NSEC_EL1 | OP_MAP_SEC_EL1 | OP_MAP_EL3)

typedef struct {
    uint32_t type;
    size_t len;
    void *addr;
} op_alloc_mem_t;

typedef struct {
    uint32_t type;
    size_t len;
    void *va;
    void *pa;
} op_map_mem_t;

typedef struct {
    uintptr_t func_id;
    uintptr_t arg;
    uintptr_t ret;
} op_dispatch_t;

typedef enum {
    CURRENTEL = 1,
    CPTR_EL3,
    CPACR,
    SCR,
    SCTLR,
} op_reg_key_t;

typedef struct {
    uint64_t key;
    uint64_t data;
    uint64_t el;
} op_data_t;

typedef struct {
    uint64_t orig;
    uint64_t val;
    uint64_t count;
    uint64_t fail;
} op_test_t;
#endif
