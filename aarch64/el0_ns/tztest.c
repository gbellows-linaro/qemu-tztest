#include "libcflat.h"
#include "arm_builtins.h"
#include "svc.h"

#if 0
/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
volatile int *tztest_fail_count;
volatile int *tztest_test_count;

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
    alloc_mem_t alloc;
    alloc.type = type;
    alloc.len = len;
    alloc.addr = NULL;
    __svc(SVC_ALLOC, &alloc);

    return alloc.addr;
}

int main()
{
    printf("Starting TZ test ...\n");

//    P0_nonsecure_check_smc();
    void *va = alloc_mem(0, 0x2000);
    printf("Called alloc_mem: got addr = %x\n", va);

    va = alloc_mem(0, 0x1000);
    printf("Called alloc_mem: got addr = %x\n", va);

    __svc(SVC_EXIT, NULL);

    return 0;
}
