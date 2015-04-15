#include "libcflat.h"
#include "svc.h"
#include "smc.h"
#include "syscntl.h"
#include "arm_builtins.h"
#include "exception.h"
#include "state.h"
#include "debug.h"
#include "tztest_internal.h"

#ifdef AARCH64
uint32_t el1_check_cpacr_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t cptr_el3, cpacr;

    TEST_HEAD("CPACR trapping");

    /* Get the current CPTR so we can restore it later */
    SMC_GET_REG(CPTR_EL3, 3, cptr_el3);

    /* Disable CPACR access */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TCPAC);

    /* Try to read CPACR */
    TEST_MSG("Read of disabled CPACR");
    TEST_EL3_EXCEPTION(cpacr = READ_CPACR(), EC_SYSINSN);

    /* Try to write CPACR */
    TEST_MSG("Write of disabled CPACR");
    TEST_EL3_EXCEPTION(WRITE_CPACR(cpacr), EC_SYSINSN);

#ifdef FP_TEST
    /* Disable FP access */
    TEST_MSG("Read of disabled FP reg");
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_EL3_EXCEPTION(asm volatile("fcmp s0, #0.0\n"), EC_SIMD);
#endif

    /* Restore the original CPTR */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3);

    return 0;
}

uint32_t el1_check_wfx_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t sctlr, scr;

    TEST_HEAD("WFx traps");

    /* Get the current SCR so we can restore it later */
    SMC_GET_REG(SCR, 3, scr);

    /* Get the current SCTLR so we can restore it later */
    sctlr = READ_SCTLR();

    /* Clearing SCTLR.nTWE normally traps WFE to EL1 but we are already there */
    WRITE_SCTLR(sctlr & ~SCTLR_nTWE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE clear)");
    TEST_NO_EXCEPTION(asm volatile("wfe\n"));

    /* SCTLR.nTWE left as trapping to check precedence */

    /* Trap WFE instructions to EL3.  This should work even though SCTLR.nTWE
     * is clear
     */
    SMC_SET_REG(SCR, 3, scr | SCR_WFE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE set)");
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Restore SCTLR */
    WRITE_SCTLR(sctlr);

    /* This should trap to EL3 with SCTLR.nTWE set */
    TEST_MSG("WFE (SCTLR.nTWE set, SCR.WFE set)");
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Restore SCR */
    SMC_SET_REG(SCR, 3, scr);

    /* We cannot test the effect of WFI in EL1 mode like we did with WFE as it
     * causes a hang.  It is assumed that since the exception is not trapped we
     * actually execute the instruction.
     * For this reason we don't bother to test the SCTLR bit effect or
     * precedence.
     */

    /* Trap WFI instructions to EL3  */
    SMC_SET_REG(SCR, 3, scr | SCR_WFI);

    TEST_MSG("WFI (SCR.WFI set)");
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Restore SCR */
    SMC_SET_REG(SCR, 3, scr);

    return 0;
}
#endif

