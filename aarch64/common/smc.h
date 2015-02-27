#ifndef _SMC_H
#define _SMC_H

#define SMC_NOOP 0
#define SMC_DISPATCH_MONITOR 1
#define SMC_YIELD 2
#define SMC_DISPATCH_SECURE_USR 3
#define SMC_DISPATCH_SECURE_SVC 4
#define SMC_ALLOCATE_SECURE_MEMORY 4
#define SMC_EXIT 5

typedef struct {
    uint32_t (*func)(uint32_t);
    uint32_t arg;
    uint32_t ret;
} tztest_dispatch_t;

typedef struct {
    union {
        tztest_dispatch_t dispatch;
    };
} tztest_smc_desc_t;

#endif
