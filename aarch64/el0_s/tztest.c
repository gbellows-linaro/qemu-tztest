#include "libcflat.h"
#include "svc.h"
#include "syscntl.h"
#include "armv8_exception.h"
#include "arm_builtins.h"
#include "el0.h"
#include "debug.h"

sys_control_t *syscntl = NULL;

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */

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

#define TEST_EL1S_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, el1_excp[SEC])
#define TEST_EL1NS_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, el1_excp[NSEC])
#define TEST_EL3_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, el3_excp)

uint32_t P0_nonsecure_check_smc()
{
    printf("\nValidating non-secure P0 smc behavior:\n");
    printf("\tUnprivileged P0 smc call ... ");

    TEST_EL1NS_EXCEPTION(asm volatile("smc #0\n"), EC_UNKNOWN);

    return 0;
}

uint32_t P0_check_register_access(int state)
{
    char *state_str[2] = {"Secure", "Nonsecure"};

    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating %s P0 restricted register access:\n",
           (state == NSEC) ? "nonsecure" : "secure");

    printf("\t%s P0 SCR read ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(read_scr_el3(), EC_UNKNOWN);

    printf("\t%s P0 SCR write ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(write_scr_el3(0), EC_UNKNOWN);

    printf("\t%s P0 SDER read ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(read_sder32_el3(), EC_UNKNOWN);

    printf("\t%s P0 SDER write ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(write_sder32_el3(0), EC_UNKNOWN);

/*
    printf("\t%s P0 MVBAR read ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(read_mvbar(), EC_UNKNOWN);

    printf("\t%s P0 MVBAR write ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(write_mvbar(0), EC_UNKNOWN);

    printf("\t%s P0 NSACR write ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(write_nsacr(0), EC_UNKNOWN);
*/

    printf("\t%s P0 CPTR_EL3 read ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(read_cptr_el3(), EC_UNKNOWN);

    printf("\t%s P0 CPTR_EL3 write ... ", state_str[state]);
    TEST_EL1NS_EXCEPTION(write_cptr_el3(0), EC_UNKNOWN);

    return 0;
}

uint32_t P0_nonsecure_check_register_access()
{
//    validate_state(CPSR_MODE_USR, TZTEST_STATE_NONSECURE);

    P0_check_register_access(NSEC);

    return 0;
}

uint32_t P0_secure_check_register_access()
{
//    validate_state(CPSR_MODE_USR, TZTEST_STATE_SECURE);

    P0_check_register_access(SEC);

    return 0;
}
//SECURE_USR_FUNC(P0_secure_check_register_access);

void *alloc_mem(int type, size_t len)
{
    svc_op_desc_t op;
    op.alloc.type = type;
    op.alloc.len = len;
    op.alloc.addr = NULL;
    __svc(SVC_OP_ALLOC, &op);

    return op.alloc.addr;
}

void map_va(void *va, size_t len, int type)
{
    svc_op_desc_t op;
    op.map.va = va;
    op.map.len = len;
    op.map.type = type;

    __svc(SVC_OP_MAP, &op);
}

void el0_sec_loop()
{
    svc_op_desc_t desc;
    op_test_t *test = (op_test_t *)&desc;
    uint32_t op = SVC_OP_YIELD;

    DEBUG_MSG("In loop desc = %p  test = %p\n", &desc, test);

    while (op != SVC_OP_EXIT) {
        switch (op) {
        case SVC_OP_MAP:
            DEBUG_MSG("Doing a MAP desc = %p\n", &desc);
            op = SVC_OP_MAP;
            break;
        case SVC_OP_YIELD:
            DEBUG_MSG("Doing a YIELD desc = %p\n", &desc);
            break;
        case SVC_OP_TEST:
            DEBUG_MSG("Handling an svc(SVC_OP_TEST) = %p\n", &desc);
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
        DEBUG_MSG("Back from svc - op = %d &desc = %p\n", op, &desc);
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

//    P0_nonsecure_check_smc();
//    P0_nonsecure_check_register_access();

    /* Fetch the system-wide control structure */
//    __svc(SVC_OP_GET_MODE, &get_data);
//    uint64_t el = get_data.data;
//    printf("EL = 0x%lx\n", el);

    __svc(SVC_OP_EXIT, NULL);

    return 0;
}
