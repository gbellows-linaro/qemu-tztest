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
    SVC_DISPATCH_SECURE_USR = 0,
    SVC_DISPATCH_SECURE_SVC,
} svc_op_t;

typedef struct {
    union {
        struct {
            void (*func)();
        } secure_dispatch;
    };
} tztest_svc_desc_t;

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

void smc_handler(smc_op_t, int) __attribute__ ((interrupt ("SWI")));
void svc_handler(svc_op_t, int) __attribute__ ((interrupt ("SWI")));
void undef_handler() __attribute__ ((interrupt ("UNDEF")));
extern volatile int tztest_exception;
extern volatile int tztest_exception_status;
extern volatile int tztest_fail_count;
extern volatile int tztest_test_count;

#define TEST_EXCP_COND(_fn, _excp, _op)             \
    do {                                            \
        _fn;                                        \
        if (tztest_exception _op _excp) {           \
            printf("PASSED\n");                     \
        } else {                                    \
            printf("FAILED\n");                     \
            tztest_fail_count++;                    \
        }                                           \
        tztest_exception = 0;                       \
        tztest_test_count++;                        \
    } while (0)

#define TEST_EXCP_COND_AND_STATUS(_fn, _prefix, _excp, _stat, _op)  \
    do {                                                            \
        _fn;                                                        \
        if (_prefix##_exception _op _excp &&                        \
            _prefix##_exception_status _op _excp) {                 \
            printf("PASSED\n");                                     \
        } else {                                                    \
            printf("FAILED\n");                                     \
            _prefix##_fail_count++;                                 \
        }                                                           \
        _prefix##_exception = 0;                                    \
        _prefix##_exception_status = 0;                             \
        _prefix##_test_count++;                                     \
    } while (0)

#endif
