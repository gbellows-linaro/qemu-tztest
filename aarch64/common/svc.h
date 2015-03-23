#ifndef _SVC_H
#define _SVC_H

#define SVC_OP_EXIT 6
#define SVC_OP_ALLOC 7
#define SVC_OP_MAP 8
#define SVC_OP_YIELD 9
#define SVC_OP_GET_SYSCNTL 10
#define SVC_OP_GET_REG 11
#define SVC_OP_SET_REG 12
#define SVC_OP_TEST 13

#ifndef __ASSEMBLY__
#include "interop.h"

const char *svc_op_name[] = {
    [SVC_OP_EXIT] = "SVC_OP_EXIT",
    [SVC_OP_ALLOC] = "SVC_OP_ALLOC",
    [SVC_OP_MAP] = "SVC_OP_MAP",
    [SVC_OP_YIELD] = "SVC_OP_YIELD",
    [SVC_OP_GET_SYSCNTL] = "SVC_OP_GET_SYSCNTL",
    [SVC_OP_GET_REG] = "SVC_OP_GET_REG",
    [SVC_OP_SET_REG] = "SVC_OP_SET_REG",
    [SVC_OP_TEST] = "SVC_OP_TEST"
};

typedef union {
    op_alloc_mem_t alloc;
    op_map_mem_t map;
    op_data_t get;
    op_data_t set;
    op_test_t test;
} svc_op_desc_t;

extern uint32_t __svc(uint32_t, const svc_op_desc_t *);

#endif

#endif
