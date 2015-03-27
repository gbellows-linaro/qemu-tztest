#ifndef __EL1_COMMON_H
#define __EL1_COMMON_H

#include "libcflat.h"
#include "platform.h"
#include "svc.h"
#include "smc.h"
#include "string.h"
#include "el1.h"
#include "armv8_exception.h"
#include "armv8_vmsa.h"
#include "arm_builtins.h"
#include "debug.h"
#include "syscntl.h"

extern void el1_init_el0();
extern void *el1_load_el0(char *base, char *start_va);
extern uint64_t el1_allocate_pa();
extern void el1_map_pa(uintptr_t vaddr, uintptr_t paddr);
extern void el1_map_va(uintptr_t addr);
extern int el1_unmap_va(uint64_t addr);
extern void *el1_heap_allocate(size_t len);
extern void el1_alloc_mem(op_alloc_mem_t *alloc);
extern void *el1_lookup_pa(void *va);
extern void el1_map_secure(op_map_mem_t *map);

#endif
