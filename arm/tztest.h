#ifndef _TZTEST_H
#define _TZTEST_H

#include <arm32.h>
#include "libcflat.h"

#define STDOUT 1
#define TZTEST_STATE_SECURE !SCR_NS
#define TZTEST_STATE_NONSECURE SCR_NS

typedef enum {
    SMC_DISPATCH_MONITOR = 0x32000000,
    SMC_DISPATCH_SECURE_USR,
    SMC_DISPATCH_SECURE_SVC,
    SMC_ALLOCATE_SECURE_MEMORY,
    SMC_EXIT = -1
} smc_op_t;

typedef enum {
    SVC_RETURN_FROM_SECURE_USR = 0,
    SVC_DISPATCH_MONITOR,
    SVC_DISPATCH_SECURE_USR,
    SVC_DISPATCH_SECURE_SVC,
    SVC_DISPATCH_NONSECURE_SVC,
    SVC_READ_REG,
    SVC_CHECK_SECURE_STATE,
    SVC_EXIT = -1
} svc_op_t;

typedef enum {
    TZTEST_REG_CPSR = 1,
    TZTEST_REG_SCR
} tztest_reg_t;

typedef struct {
    union {
        struct {
            uint32_t (*func)();
            uint32_t ret;
        } dispatch;
        struct {
            uint32_t reg;
            uint32_t val;
        } reg_read;
        struct {
            uint32_t state;
        } secure_state;
    };
} tztest_svc_desc_t;

#define CLEAR_SVC_DESC(_desc) memset(&(_desc), sizeof(tztest_svc_desc_t), 0)

typedef struct {
    union {
        struct {
            uint32_t (*func)();
            uint32_t ret;
        } dispatch;
    };
} tztest_smc_desc_t;

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("\n[DEBUG] %s: " _str, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUG_MSG(_str, ...)
#endif

#define MODE_STR(_mode)             \
    ((_mode == MON) ? "MON" :        \
     (_mode == SVC) ? "SVC" :        \
     (_mode == SYS) ? "SYS" :        \
     (_mode == USR) ? "USR" : "Unknown")


#define FAULT_STR(_s) \
    ((0x01 == (_s)) ? "Alignment fault" :                   \
     (0x02 == (_s)) ? "Debug event" :                       \
     (0x03 == (_s)) ? "Access flag fault" :                 \
     (0x04 == (_s)) ? "Instruction cache maintenance" :     \
     (0x05 == (_s)) ? "Translation fault" :                 \
     (0x06 == (_s)) ? "Access flag fault" :                 \
     (0x07 == (_s)) ? "Translation fault" :                 \
     (0x08 == (_s)) ? "Synchronous external abort" :        \
     (0x09 == (_s)) ? "Domain fault" :                      \
     (0x0b == (_s)) ? "Domain fault" :                      \
     (0x0c == (_s)) ? "External abort on table walk" :      \
     (0x0d == (_s)) ? "Permisison fault" :                  \
     (0x0e == (_s)) ? "Synchronous external abort" :        \
     (0x0f == (_s)) ? "Permisison fault" :                  \
     (0x10 == (_s)) ? "TLB conflict abort" :                \
     (0x1c == (_s)) ? "External parity err on table walk" : \
     (0x1e == (_s)) ? "External parity err on table walk" : \
     "Unknown")

extern volatile int *tztest_exception;
extern volatile int *tztest_exception_status;
extern volatile int *tztest_fail_count;
extern volatile int *tztest_test_count;
extern int tztest_read_register(tztest_reg_t);
extern int tztest_get_saved_cpsr();
extern void validate_state(uint32_t, uint32_t);

#define INC_TEST_COUNT()    (*tztest_test_count += 1)
#define INC_FAIL_COUNT()    (*tztest_fail_count += 1)

#define TEST_CONDITION(_cond)                           \
    do {                                                \
        if (!(_cond)) {                                 \
            printf("FAILED\n");                         \
            INC_FAIL_COUNT();                           \
        } else {                                        \
            printf("PASSED\n");                         \
        }                                               \
        INC_TEST_COUNT();                               \
    } while(0)

#define TEST_FUNCTION(_fn, _cond)                       \
    do {                                                \
        _fn;                                            \
        TEST_CONDITION(_cond);                          \
    } while(0)

#define TEST_EXCEPTION(_fn, _excp)                      \
    do {                                                \
        TEST_FUNCTION(_fn, *tztest_exception == (_excp));   \
        *tztest_exception = 0;                          \
    } while (0)

#endif
