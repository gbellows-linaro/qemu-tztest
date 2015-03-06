#ifndef _INTEROP_H
#define _INTEROP_H
#include <stdint.h>
#include <stddef.h>

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
    uintptr_t (*func)(uintptr_t);
    uintptr_t arg;
    uintptr_t ret;
} op_dispatch_t;

#endif
