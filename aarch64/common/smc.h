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
#define SMC_OP_TEST 13

#ifndef __ASSEMBLY__
#include "interop.h"

extern uint32_t __smc(uint32_t, void *);

const char *smc_op_name[] = {
    [SMC_OP_NOOP] = "SMC_OP_NOOP",
    [SMC_OP_DISPATCH_MONITOR] = "SMC_DISPATCH_MONITOR",
    [SMC_OP_YIELD] = "SMC_OP_YIELD",
    [SMC_OP_EXIT] = "SMC_OP_EXIT",
    [SMC_OP_MAP] = "SMC_OP_MAP",
    [SMC_OP_TEST] = "SMC_OP_TEST"
};

typedef union {
    op_dispatch_t dispatch;
    op_map_mem_t map;
    op_test_t test;
} smc_op_desc_t;

extern smc_op_desc_t *smc_interop_buf;

#define SMC_EXIT()  __smc(SMC_OP_EXIT, NULL)
#define SMC_YIELD()  __smc(SMC_OP_YIELD, smc_interop_buf);

#endif

#endif
