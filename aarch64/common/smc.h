#ifndef _SMC_H
#define _SMC_H

#define SMC_OP_NOOP 0
#define SMC_OP_DISPATCH_MONITOR 1
#define SMC_OP_YIELD 2
#define SMC_OP_DISPATCH_SECURE_USR 3
#define SMC_OP_DISPATCH_SECURE_SVC 4
#define SMC_OP_ALLOCATE_SECURE_MEMORY 4
#define SMC_OP_EXIT 5
#define SMC_OP_MAP 8

#ifndef __ASSEMBLY__
#include "interop.h"

extern uint32_t __smc(uint32_t, void *);

typedef union {
    op_dispatch_t dispatch;
    op_map_mem_t map;
} smc_op_desc_t;

extern smc_op_desc_t *smc_interop_buf;

#define SMC_NO_DESC(_op) __smc((_op), NULL)

#define SMC_EXIT()  SMC_NO_DESC(SMC_OP_EXIT)
#define SMC_YIELD()  SMC_NO_DESC(SMC_OP_YIELD)

#endif

#endif
