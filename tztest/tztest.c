#include "syscntl.h"
#include "interop.h"
#include "svc.h"
#include "libcflat.h"
#include "tztest_internal.h"
#include "tztest_el0.h"

tztest_t tztest[TZTEST_COUNT] =
{
    [TZTEST_SMC] = el0_check_smc,
    [TZTEST_REG_ACCESS] = el0_check_register_access,
#ifdef AARCH64
    [TZTEST_CPACR_TRAP] = el0_check_cpacr_trap,
    [TZTEST_WFX_TRAP] = el0_check_wfx_trap
#endif
};

void interop_test()
{
    op_test_t test;

    test.orig = test.val = 1024;
    test.fail = test.count = 0;
    printf("\nValidating interop communication between ELs... ");
    __svc(SVC_OP_TEST, (svc_op_desc_t *)&test);
    TEST_CONDITION(!test.fail && test.val == (test.orig >> test.count));
}

void run_test(tztest_func_id_t fid, uint32_t el)
{
    op_dispatch_t disp;

    tztest[fid](el);

    disp.func_id = fid;
    __svc(SVC_OP_DISPATCH, (svc_op_desc_t *)&disp);
}

void tztest_start()
{
    printf("Starting TZ test...\n");

    /* Test EL to EL communication */
    interop_test();

    run_test(TZTEST_SMC, 0);
    run_test(TZTEST_REG_ACCESS, 0);
#if AARCH64
    run_test(TZTEST_CPACR_TRAP, 0);
    run_test(TZTEST_WFX_TRAP, 0);
#endif

    printf("\nValidation complete.  Passed %d of %d tests.\n",
              syscntl->test_cntl->test_count - syscntl->test_cntl->fail_count,
              syscntl->test_cntl->test_count);
}

