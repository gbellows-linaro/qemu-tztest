#ifndef _TZTEST_H
#define _TZTEST_H

#include <arm32.h>

#define STDOUT 1

typedef enum {
    SMC_SET_SECURE_STATE = 0,
    SMC_TEST,
    SMC_CATCH,
    SMC_NOOP
} smc_op_t;

typedef enum {
    SVC_SMC_TEST = 0,
    SVC_TEST,
    SVC_CATCH,
} svc_op_t;

typedef struct {
    int processor_mode;
    int secure_state;
    void (*func)();
    int result;
} test_desc_t;

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("[DEBUG] %s: " _str, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUG_MSG(_str, ...)
#endif  

#define MODE_STR(_mode)             \
    ((_mode == MON) ? "MON" :        \
     (_mode == SVC) ? "SVC" :        \
     (_mode == SYS) ? "SYS" :        \
     (_mode == USR) ? "USR" : "Unknown")

void smc_handler(smc_op_t, int) __attribute__ ((interrupt ("SWI")));
void svc_handler(svc_op_t, int) __attribute__ ((interrupt ("SWI")));
void undef_handler() __attribute__ ((interrupt ("UNDEF")));

#define TEST_EXCP_COND(_fn, _prefix, _excp, _op)    \
    do {                                            \
        _fn;                                        \
        if (_prefix##_exception _op _excp) {        \
            _prefix##_exception = 0;                \
            printf("PASSED\n");                     \
        } else {                                    \
            printf("FAILED\n");                     \
            _prefix##_fail_count++;                 \
        }                                           \
        _prefix##_test_count++;                     \
    } while (0)

#endif
