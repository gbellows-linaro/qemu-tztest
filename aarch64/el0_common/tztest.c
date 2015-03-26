#include "libcflat.h"
#include "svc.h"
#include "syscntl.h"
#include "armv8_exception.h"
#include "arm_builtins.h"
#include "el0.h"
#include "debug.h"
#include "el0_common.h"

sys_control_t *syscntl = NULL;

#define TEST_HEAD(_str, ...) \
    printf("\nValidating %s EL%d " _str ":\n", SEC_STATE_STR, el, ##__VA_ARGS__)

#define TEST_MSG(_str, ...) \
    printf("\tEL%d (%s): " _str "... ", el, SEC_STATE_STR, ##__VA_ARGS__)

uint32_t check_smc(uint32_t el)
{
    TEST_HEAD("smc behavior");

    TEST_MSG("SMC call");
    TEST_EL1_EXCEPTION(asm volatile("smc #0\n"), EC_UNKNOWN);

    return 0;
}

uint32_t check_register_access(uint32_t el)
{
    /* Set things to non-secure P1 and attempt accesses */
    TEST_HEAD("restricted register access");

    TEST_MSG("SCR read");
    TEST_EL1_EXCEPTION(read_scr_el3(), EC_UNKNOWN);

    TEST_MSG("SCR write");
    TEST_EL1_EXCEPTION(write_scr_el3(0), EC_UNKNOWN);

    TEST_MSG("SDER read");
    TEST_EL1_EXCEPTION(read_sder32_el3(), EC_UNKNOWN);

    TEST_MSG("SDER write");
    TEST_EL1_EXCEPTION(write_sder32_el3(0), EC_UNKNOWN);

/*
    TEST_MSG("MVBAR read");
    TEST_EL1_EXCEPTION(read_mvbar(), EC_UNKNOWN);

    TEST_MSG("MVBAR write");
    TEST_EL1_EXCEPTION(write_mvbar(0), EC_UNKNOWN);

    TEST_MSG("NSACR write");
    TEST_EL1_EXCEPTION(write_nsacr(0), EC_UNKNOWN);
*/

    TEST_MSG("CPTR_EL3 read");
    TEST_EL1_EXCEPTION(read_cptr_el3(), EC_UNKNOWN);

    TEST_MSG("CPTR_EL3 write");
    TEST_EL1_EXCEPTION(write_cptr_el3(0), EC_UNKNOWN);

    return 0;
}

uint32_t check_cpacr_trap(uint32_t el)
{
    uint64_t cptr_el3, cpacr;

    TEST_HEAD("CPACR trapping");

    /* Get the current CPTR so we can restore it later */
    SVC_GET_REG(CPTR_EL3, 3, cptr_el3);

    /* Disable CPACR access */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TCPAC);

    /* Try to read CPACR */
    TEST_MSG("Read of disabled CPACR");
    TEST_EL3_EXCEPTION(SVC_GET_REG(CPACR_EL1, 1, cpacr), EC_SYSINSN);

    /* Try to write CPACR */
    TEST_MSG("Write of disabled CPACR");
    TEST_EL3_EXCEPTION(SVC_SET_REG(CPACR_EL1, 1, cpacr), EC_SYSINSN);

#ifdef FP_TEST
    /* Disable FP access */
    TEST_MSG("Read of disabled FP reg");
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_EL3_EXCEPTION(asm volatile("fcmp s0, #0.0\n"), EC_SIMD);
#endif

    /* Restore the original CPTR */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3);

    return 0;
}

uint32_t check_wfx_trap(uint32_t el)
{
    uint64_t sctlr, scr_el3;

    TEST_HEAD("WFx traps");

    /* Get the current SCR so we can restore it later */
    SVC_GET_REG(SCR_EL3, 3, scr_el3);

    /* Get the current SCTLR so we can restore it later */
    SVC_GET_REG(SCTLR_EL1, 1, sctlr);

    /* Clear SCTLR.nTWE to cause WFE instructions to trap to EL1 */
    SVC_SET_REG(SCTLR_EL1, 1, sctlr & ~SCTLR_nTWE);
    TEST_MSG("Execution of WFE trapped to EL1");
    TEST_EL1_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* SCTLR.nTWE left as trapping to check precedence */

    /* Trap WFE instructions to EL3.  This should work even though SCTLR.nTWE
     * is clear
     */
    SVC_SET_REG(SCR_EL3, 3, scr_el3 | SCR_WFE);
    TEST_MSG("Execution of trapped WFE (SCTLR.nTWE clear)",
           SEC_STATE_STR);
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Restore SCTLR */
    SVC_SET_REG(SCTLR_EL1, 1, sctlr);

    /* This should trap to EL3 with SCTLR.nTWE set */
    TEST_MSG("Execution of trapped WFE (SCTLR.nTWE set)",
           SEC_STATE_STR);
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Restore SCR */
    SVC_SET_REG(SCR_EL3, 3, scr_el3);

    /* Clear SCTLR.nTWI to cause WFI instructions to trap to EL1 */
    SVC_SET_REG(SCTLR_EL1, 1, sctlr & ~SCTLR_nTWI);
    TEST_MSG("Execution of WFI trapped to EL1");
    TEST_EL1_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* SCTLR.nTWI left as trapping to check precedence */

    /* Trap WFI instructions to EL3.  This should work even though SCTLR.nTWE
     * is clear
     */
    SVC_SET_REG(SCR_EL3, 3, scr_el3 | SCR_WFI);

    TEST_MSG("Execution of trapped WFI (SCTLR.nTWI clear)",
           SEC_STATE_STR);
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Restore SCTLR */
    SVC_SET_REG(SCTLR_EL1, 1, sctlr);

    TEST_MSG("Execution of trapped WFI (SCTLR.nTWI set)",
           SEC_STATE_STR);
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Restore SCR */
    SVC_SET_REG(SCR_EL3, 3, scr_el3);

    return 0;
}

void tztest_init()
{
    tztest[TZTEST_SMC] = check_smc;
    tztest[TZTEST_REG_ACCESS] = check_register_access;
    tztest[TZTEST_CPACR_TRAP] = check_cpacr_trap;
    tztest[TZTEST_WFX_TRAP] = check_wfx_trap;
}

