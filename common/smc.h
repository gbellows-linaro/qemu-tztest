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
#define SMC_OP_GET_REG  11
#define SMC_OP_SET_REG  12
#define SMC_OP_TEST 13
#define SMC_OP_DISPATCH 14

#ifndef __ASSEMBLY__
#include "interop.h"

extern uint32_t __smc(uint32_t, void *);

typedef union {
    op_dispatch_t dispatch;
    op_map_mem_t map;
    op_test_t test;
    op_data_t get;
    op_data_t set;
} smc_op_desc_t;

extern smc_op_desc_t *smc_interop_buf;

#define SMC_EXIT()  __smc(SMC_OP_EXIT, NULL)
#define SMC_YIELD()  __smc(SMC_OP_YIELD, smc_interop_buf);

#define SMC_GET_REG(__reg, __el, __val)     \
    do {                                \
        smc_op_desc_t *desc = smc_interop_buf;             \
        desc->get.key = (__reg);         \
        desc->get.el = (__el);           \
        __smc(SMC_OP_GET_REG, desc);   \
        (__val) = desc->get.data;        \
    } while (0)

#define SMC_SET_REG(__reg, __el, __val)     \
    do {                                \
        smc_op_desc_t *desc = smc_interop_buf;             \
        desc->get.key = (__reg);         \
        desc->get.el = (__el);           \
        desc->get.data = (__val);        \
        __smc(SMC_OP_SET_REG, desc);   \
    } while (0)

#endif

#endif
