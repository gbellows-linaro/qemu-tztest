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
#define SVC_OP_DISPATCH 14

#ifndef __ASSEMBLY__
#include "interop.h"

extern const char *svc_op_name[];

typedef union {
    op_alloc_mem_t alloc;
    op_map_mem_t map;
    op_data_t get;
    op_data_t set;
    op_test_t test;
    op_dispatch_t disp;
} svc_op_desc_t;

extern uint32_t __svc(uint32_t, const svc_op_desc_t *);

#define SVC_GET_REG(__reg, __el, __val)     \
    do {                                \
        svc_op_desc_t desc;             \
        desc.get.key = (__reg);         \
        desc.get.el = (__el);           \
        __svc(SVC_OP_GET_REG, &desc);   \
        (__val) = desc.get.data;        \
    } while (0)

#define SVC_SET_REG(__reg, __el, __val)     \
    do {                                \
        svc_op_desc_t desc;             \
        desc.get.key = (__reg);         \
        desc.get.el = (__el);           \
        desc.get.data = (__val);        \
        __svc(SVC_OP_SET_REG, &desc);   \
    } while (0)

#endif

#endif
