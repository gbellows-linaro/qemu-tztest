#include "el0_common.h"

void interop_test()
{
    op_test_t test;

    test.orig = test.val = 1024;
    test.fail = test.count = 0;
    __svc(SVC_OP_TEST, (svc_op_desc_t *)&test);

    printf("\nValidating interop communication between ELs... ");
    TEST_CONDITION(!test.fail && test.val == (test.orig >> test.count));
}

int main()
{
    svc_op_desc_t desc;

    printf("Starting TZ test ...\n");

    /* Fetch the system-wide control structure */
    __svc(SVC_OP_GET_SYSCNTL, &desc);
    syscntl = ((sys_control_t *)desc.get.data);

    /* Allocate and globally map test control descriptor */
    syscntl->test_cntl = (test_control_t*)alloc_mem(0, 0x1000);
    map_va(syscntl->test_cntl, 0x1000, OP_MAP_ALL);

    /* Test EL to EL communication */
    interop_test();

    /* If we didn't get a valid control structure then something has already
     * gone drastically wrong.
     */
    if (!syscntl) {
        DEBUG_MSG("Failed to acquire system control structure\n");
        __svc(SVC_OP_EXIT, &desc);
    }

    P0_nonsecure_check_smc();
    P0_check_register_access();
    P0_check_trap_to_EL3();

    printf("\nValidation complete.  Passed %d of %d tests\n",
           syscntl->test_cntl->test_count - syscntl->test_cntl->fail_count,
           syscntl->test_cntl->test_count);

    __svc(SVC_OP_EXIT, NULL);

    return 0;
}
