#include "libcflat.h"
#include "svc.h"
#include "syscntl.h"
#include "armv8_exception.h"
#include "arm_builtins.h"
#include "el0.h"
#include "debug.h"
#include "el0_common.h"

sys_control_t *syscntl = NULL;

uint32_t P0_nonsecure_check_smc()
{
    printf("\nValidating %s P0 smc behavior:\n", SEC_STATE_STR);
    printf("\tUnprivileged P0 smc call ... ");

    TEST_EL1NS_EXCEPTION(asm volatile("smc #0\n"), EC_UNKNOWN);

    return 0;
}

uint32_t P0_check_register_access()
{
    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating %s P0 restricted register access:\n", SEC_STATE_STR);

    printf("\t%s P0 SCR read ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(read_scr_el3(), EC_UNKNOWN);

    printf("\t%s P0 SCR write ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(write_scr_el3(0), EC_UNKNOWN);

    printf("\t%s P0 SDER read ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(read_sder32_el3(), EC_UNKNOWN);

    printf("\t%s P0 SDER write ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(write_sder32_el3(0), EC_UNKNOWN);

/*
    printf("\t%s P0 MVBAR read ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(read_mvbar(), EC_UNKNOWN);

    printf("\t%s P0 MVBAR write ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(write_mvbar(0), EC_UNKNOWN);

    printf("\t%s P0 NSACR write ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(write_nsacr(0), EC_UNKNOWN);
*/

    printf("\t%s P0 CPTR_EL3 read ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(read_cptr_el3(), EC_UNKNOWN);

    printf("\t%s P0 CPTR_EL3 write ... ", SEC_STATE_STR);
    TEST_EL1NS_EXCEPTION(write_cptr_el3(0), EC_UNKNOWN);

    return 0;
}

uint32_t P0_check_trap_to_EL3()
{
    uint32_t cptr_el3;
    svc_op_desc_t desc;

    printf("\nValidating %s P1 trap to EL3:\n", SEC_STATE_STR);

    /* Get the current CPTR so we can restore it later */
    desc.get.key = CPTR_EL3;
    desc.get.el = 3;
    __svc(SVC_OP_GET_REG, &desc);

    /* Disable CPACR access */
    cptr_el3 = desc.get.data;
    desc.set.data =  desc.get.data | (1 << 31);
    __svc(SVC_OP_SET_REG, &desc);

    /* Try to read CPACR */
    desc.get.key = CPACR_EL1;
    desc.get.el = 1;
    printf("\t%s P1 read of disabled CPACR... ", SEC_STATE_STR);
    TEST_EL3_EXCEPTION(__svc(SVC_OP_GET_REG, &desc), EC_SYSINSN);

    /* Try to write CPACR
     * Note: data still set to above get value in case we succeeded.
     */
    printf("\t%s P1 write of disabled CPACR... ", SEC_STATE_STR);
    TEST_EL3_EXCEPTION(__svc(SVC_OP_SET_REG, &desc), EC_SYSINSN);

    /* Restore the original CPTR */
    desc.get.key = CPTR_EL3;
    desc.get.el = 3;
    desc.set.data = cptr_el3;
    __svc(SVC_OP_SET_REG, &desc);

    return 0;
}

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

