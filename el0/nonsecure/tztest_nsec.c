#include "el0_common.h"
#include "tztest.h"

tztest_t tztest[TZTEST_COUNT];
const char *sec_state_str;

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

int main()
{
    svc_op_desc_t desc;

    /* ISSUE: For some reason, static initialization of the global security
     * state string fails.  The pointer ends up being NULL in some cases, but
     * not in others.  This likely has something to do with the position
     * independence of the EL0 code.  The below workaround works fine.
     */
    const char *str = "non-secure";
    sec_state_str = str;

    printf("EL0 (%s) started...\n", sec_state_str);

    tztest_init();

    /* Fetch the system-wide control structure */
    __svc(SVC_OP_GET_SYSCNTL, &desc);
    syscntl = ((sys_control_t *)desc.get.data);

    /* Allocate and globally map test control descriptor */
    syscntl->test_cntl = (test_control_t*)alloc_mem(0, 0x1000);
    map_va(syscntl->test_cntl, 0x1000, OP_MAP_ALL);

    printf("Starting TZ test...\n");

    /* Test EL to EL communication */
    interop_test();

    /* If we didn't get a valid control structure then something has already
     * gone drastically wrong.
     */
    if (!syscntl) {
        DEBUG_MSG("Failed to acquire system control structure\n");
        __svc(SVC_OP_EXIT, &desc);
    }

    run_test(TZTEST_SMC, 0);
    run_test(TZTEST_REG_ACCESS, 0);
#if AARCH64
    run_test(TZTEST_CPACR_TRAP, 0);
    run_test(TZTEST_WFX_TRAP, 0);
#endif

    printf("\nValidation complete.  Passed %d of %d tests.\n",
              syscntl->test_cntl->test_count - syscntl->test_cntl->fail_count,
              syscntl->test_cntl->test_count);

    __svc(SVC_OP_EXIT, NULL);

    return 0;
}
