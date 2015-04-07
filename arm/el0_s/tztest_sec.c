#include "el0_common.h"

const char *sec_state_str;
tztest_t tztest[TZTEST_COUNT];

void el0_sec_loop()
{
    svc_op_desc_t _desc , *desc = &_desc;
    uint32_t op = SVC_OP_YIELD;

    DEBUG_MSG("Starting loop - desc = %p\n", desc);

    while (op != SVC_OP_EXIT) {
        switch (op) {
        case SVC_OP_MAP:
            DEBUG_MSG("Handling a SVC_OP_MAP - desc = %p\n", desc);
            op = SVC_OP_MAP;
            break;
        case SVC_OP_YIELD:
            DEBUG_MSG("Handling a SVC_OP_YIELD - desc = %p\n", desc);
            break;
        case SVC_OP_TEST:
            DEBUG_MSG("Handling a SVC_OP_TEST - desc =  %p\n", desc);
            if (desc->test.val != desc->test.orig >> desc->test.count) {
                desc->test.fail++;
            }
            desc->test.val >>= 1;
            desc->test.count++;
            op = SVC_OP_YIELD;
            break;
        case SVC_OP_DISPATCH:
            tztest[desc->disp.func_id](0);
            op = SVC_OP_YIELD;
            break;
        case 0:
            op = SVC_OP_YIELD;
            break;
        default:
            DEBUG_MSG("Unrecognized SVC opcode %d.  Exiting ...\n", op);
            op = SVC_OP_EXIT;
            break;
        }

        DEBUG_MSG("Calling svc(%d, %p)\n", op, desc);
        op = __svc(op, desc);
        DEBUG_MSG("Returned from svc - op = %d &desc = %p\n", op, desc);
    }

    __svc(SVC_OP_EXIT, NULL);
}

int main()
{
    svc_op_desc_t desc;

    /* ISSUE: For some reason, static initialization of the global security
     * state string fails.  The pointer ends up being NULL in some cases, but
     * not in others.  This likely has something to do with the position
     * independence of the EL0 code.  The below workaround works fine.
     */
    const char *str = "secure";
    sec_state_str = str;

    printf("EL0 (%s) started...\n", sec_state_str);

    tztest_init();

    /* Fetch the system-wide control structure */
    __svc(SVC_OP_GET_SYSCNTL, &desc);
    syscntl = (sys_control_t *)desc.get.data;

    /* If we didn't get a valid control structure then something has already
     * gone drastically wrong.
     */
    if (!syscntl) {
        DEBUG_MSG("Failed to acquire system control structure\n");
        __svc(SVC_OP_EXIT, NULL);
    }

    el0_sec_loop();

    return 0;
}
