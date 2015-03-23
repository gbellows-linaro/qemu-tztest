#include "el0_common.h"

void el0_sec_loop()
{
    svc_op_desc_t desc;
    op_test_t *test = (op_test_t *)&desc;
    uint32_t op = SVC_OP_YIELD;

    DEBUG_MSG("Starting loop - desc = %p  test = %p\n", &desc, test);

    while (op != SVC_OP_EXIT) {
        switch (op) {
        case SVC_OP_MAP:
            DEBUG_MSG("Handling a SVC_OP_MAP - desc = %p\n", &desc);
            op = SVC_OP_MAP;
            break;
        case SVC_OP_YIELD:
            DEBUG_MSG("Handling a SVC_OP_YIELD - desc = %p\n", &desc);
            break;
        case SVC_OP_TEST:
            DEBUG_MSG("Handling a SVC_OP_TEST - desc =  %p\n", &desc);
            if (test->val != test->orig >> test->count) {
                test->fail++;
            }
            test->val >>= 1;
            test->count++;
            break;
        case 0:
            op = SVC_OP_YIELD;
            break;
        default:
            DEBUG_MSG("Unrecognized SVC opcode %d.  Exiting ...\n", op);
            op = SVC_OP_EXIT;
            break;
        }

        DEBUG_MSG("Calling svc(%d, %p)\n", op, &desc);
        op = __svc(op, &desc);
        DEBUG_MSG("Returned from svc - op = %d &desc = %p\n", op, &desc);
    }

    __svc(SVC_OP_EXIT, NULL);
}

int main()
{
    svc_op_desc_t desc;

    printf("Starting secure-side EL0  ...\n");

    /* Fetch the system-wide control structure */
    __svc(SVC_OP_GET_SYSCNTL, &desc);
    syscntl = (sys_control_t *)desc.get.data;

    /* If we didn't get a valid control structure then something has already
     * gone drastically wrong.
     */
    if (!syscntl) {
        printf("Failed to acquire system control structure\n");
        __svc(SVC_OP_EXIT, NULL);
    }

    el0_sec_loop();

    return 0;
}
