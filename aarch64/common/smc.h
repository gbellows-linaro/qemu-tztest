#ifndef _SMC_H
#define _SMC_H

#define SMC_NOOP 0
#define SMC_DISPATCH_MONITOR 1
#define SMC_YIELD 2
#define SMC_DISPATCH_SECURE_USR 3
#define SMC_DISPATCH_SECURE_SVC 4
#define SMC_ALLOCATE_SECURE_MEMORY 4
#define SMC_EXIT 5

#ifndef __ASSEMBLY__
typedef struct {
    uintptr_t (*func)(uintptr_t);
    uintptr_t arg;
    uintptr_t ret;
} dispatch_t;

typedef union {
    dispatch_t dispatch;
} smc_op_desc_t;
#endif

#endif
