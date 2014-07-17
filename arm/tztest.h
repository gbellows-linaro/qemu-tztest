#ifndef _TZTEST_H
#define _TZTEST_H

enum {
    USR = 0x10,
    FIQ,
    IRQ,
    SVC,
    MON = 0x16,
    ABT,
    HYP = 0x1a,
    UND,
    SYS = 0x1f
} processor_mode;

enum {
    SECURE = 0,
    NONSECURE
} secure_state;

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

#define DEBUG 1
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

#define TEST_EXCP_COND(fn, excp, op)        \
    do {                                    \
        fn;                                 \
        if (exception op excp) {            \
            exception = 0;                  \
            printf("PASSED\n");             \
        } else {                            \
            printf("FAILED\n");             \
            fail_count++;                   \
        }                                   \
        test_count++;                       \
    } while (0)

#endif
