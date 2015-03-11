#include "libcflat.h"
#include "arm_builtins.h"
#include "svc.h"

typedef struct {
    volatile int fail_count;
    volatile int test_count;
    volatile int exception_ec;
    volatile int exception_iss;
} test_control_t;

test_control_t *tztest_cntl;
svc_op_desc_t svc_exit = {.op = SVC_EXIT};
svc_op_desc_t svc_yield = {.op = SVC_YIELD};

#if 0
/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */

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
#ifdef DEBUG
void validate_state(uint32_t mode, uint32_t state)
{
    tztest_svc_desc_t desc;

    assert((_read_cpsr() & CPSR_MODE_MASK) == mode);

    CLEAR_SVC_DESC(desc);
    __svc(SVC_GET_SECURE_STATE, &desc);
    assert(desc.secure_state.state == state);
}
#else
void validate_state(__attribute__((unused)) uint32_t mode,
                    __attribute__((unused)) uint32_t state) {}
#endif

uint32_t P0_nonsecure_check_smc()
{
    validate_state(CPSR_MODE_USR, TZTEST_STATE_NONSECURE);
    printf("\nValidating non-secure P0 smc behavior:\n");
    printf("\tUnprivileged P0 smc call ... ");
    TEST_EXCEPTION(smc_noop(), CPSR_MODE_UND);

    return 0;
}
#endif

void *alloc_mem(int type, size_t len)
{
    svc_op_desc_t op;
    op.op = SVC_ALLOC;
    op.alloc.type = type;
    op.alloc.len = len;
    op.alloc.addr = NULL;
    __svc(&op);

    return op.alloc.addr;
}

void map_va(void *va, size_t len, int type)
{
    svc_op_desc_t op;
    op.op = SVC_MAP;
    op.map.va = va;
    op.map.len = len;
    op.map.type = type;

    __svc(&op);
}

int main()
{
    printf("Starting TZ test ...\n");

    tztest_cntl = (test_control_t*)alloc_mem(0, 0x1000);
    map_va(tztest_cntl, 0x1000, OP_MAP_ALL);
    printf("Called alloc_mem: got addr = %x\n", tztest_cntl);

    __svc(&svc_yield);
    __svc(&svc_exit);

    return 0;
}
