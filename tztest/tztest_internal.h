#ifndef _TZTEST_INTERNAL_H
#define _TZTEST_INTERNAL_H

#include "tztest.h"

typedef enum {
    TZTEST_SMC = 0,
    TZTEST_REG_ACCESS,
    TZTEST_CPACR_TRAP,
    TZTEST_WFX_TRAP,
    TZTEST_COUNT
} tztest_func_id_t;

#define TEST_HEAD(_str, ...) \
    printf("\nValidating %s EL%d " _str ":\n", sec_state_str, el, ##__VA_ARGS__)

#define TEST_MSG(_str, ...) \
    printf("\tEL%d (%s): " _str "... ", el, sec_state_str, ##__VA_ARGS__)

#define INC_TEST_COUNT()    (syscntl->test_cntl->test_count += 1)
#define INC_FAIL_COUNT()    (syscntl->test_cntl->fail_count += 1)

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

#define TEST_EXCEPTION(_fn, _excp, _el)                 \
    do {                                                \
        syscntl->_el.ec = 0;                            \
        syscntl->excp_action = EXCP_ACTION_SKIP;        \
        syscntl->excp_log = true;                       \
        _fn;                                            \
        TEST_CONDITION(syscntl->_el.taken &&            \
                       syscntl->_el.ec == (_excp));     \
        syscntl->_el.taken = 0;                         \
        syscntl->excp_action = 0;                       \
        syscntl->_el.ec = 0;                            \
    } while (0)

#define TEST_EL1_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, el1_excp[SEC_STATE])
#define TEST_EL3_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, el3_excp)

#endif

