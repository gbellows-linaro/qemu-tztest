#ifndef _SMC_H
#define _SMC_H

#define SMC_NOOP 0
#define SMC_DISPATCH_MONITOR 1
#define SMC_YIELD 2
#define SMC_DISPATCH_SECURE_USR 3
#define SMC_DISPATCH_SECURE_SVC 4
#define SMC_ALLOCATE_SECURE_MEMORY 4
#define SMC_EXIT 5
#define SMC_MAP 8

#ifndef __ASSEMBLY__
#include "interop.h"

typedef union {
    op_dispatch_t dispatch;
    op_map_mem_t map;
} smc_op_desc_t;

typedef struct {
     void *interop_buf_pa;
     void *interop_buf_va;
} smc_control_t;

#endif

#endif
