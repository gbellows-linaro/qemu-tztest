#include "libcflat.h"
#include "tztest_builtins.h"
#include "tztest.h"

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
volatile int smc_count = 0; 
volatile int svc_count = 0;
volatile int undef_count = 0;
volatile int unused_count = 0;
volatile int exception = 0;
volatile int fail_count = 0;
volatile int test_count = 0;

void smc_handler(smc_op_t op, int data) 
{
    int smc = 0;
    switch (op) {
        case SMC_SET_SECURE_STATE:
            smc = _read_scr();
            DEBUG_MSG("Switching secure state from %s to %s\n", 
                      ((smc & 0x1) == SECURE) ? "SECURE" : "NONSECURE", 
                      (data == SECURE) ? "SECURE" : "NONSECURE");
            smc &= ~0x1;
            smc |= data;
            _write_scr(smc);
            break;
        default:
            break;
    }
}

void svc_handler(svc_op_t op, int data) {
    switch(op) {
        case SMC:
            __smc(op, data);
            break;
        default:
            break;
    }
    svc_count++;
    exception = SVC;
}

void undef_handler() {
    undef_count++;
    exception = UND;
}

void unused_handler() {
    unused_count++;
    exception = -1;
}

/* Validate restricted registers are inaccessible */
void test1() {
    /* Set ourselves in secure P1 and read the values we intend to write below.
     * The intent is to write-back the same value in case the write goes
     * through.
     */
    __cps(SYS);
    __smc(SMC_SET_SECURE_STATE, SECURE);
    int scr = _read_scr();
    int sder = _read_sder();
    int nsacr = _read_nsacr();
    int mvbar = _read_mvbar();

    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating non-secure P1 inaccessibility:\n");
    __smc(SMC_SET_SECURE_STATE, NONSECURE);

    printf("\tNon-secure read of SCR ... ");
    TEST_EXCP_COND(_read_scr(), UND, ==);

    printf("\tNon-secure write of SCR ... ");
    TEST_EXCP_COND(_write_scr(scr), UND, ==);

    printf("\tNon-secure read of SDER ... ");
    TEST_EXCP_COND(_read_sder(), UND, ==);

    printf("\tNon-secure write of SDER ... ");
    TEST_EXCP_COND(_write_sder(sder), UND, ==);

    printf("\tNon-secure read of MVBAR ... ");
    TEST_EXCP_COND(_read_mvbar(), UND, ==);

    printf("\tNon-secure write of MVBAR ... ");
    TEST_EXCP_COND(_write_mvbar(mvbar), UND, ==);

    printf("\tNon-secure write of NSACR ... ");
    TEST_EXCP_COND(_write_nsacr(nsacr), UND, ==);
}

void check_init_mode() 
{
    printf("\nValidating startup state:\n");

    printf("\tInitial processor mode... ");
    if (SVC != (_read_cpsr() & 0x1f)) {
        printf("Failed\n");
        DEBUG_MSG("current CPSR (%d) != expected CPSR (%d)\n", 
                  (_read_cpsr() & 0x1f), SVC);
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;

    printf("\tInitial security state... ");
    if (SECURE != (_read_scr() & 0x1)) {
        printf("Failed\n");
        DEBUG_MSG("current SCR.NS (%d) != expected SCR.NS (%d)\n", 
                  (_read_cpsr() & 0x1), SECURE);
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;
}

int tztest_start() 
{
    /* Make sure we are starting in secure state */
    if (0 != (_read_scr() & 0x1)) {
        printf("Not in secure state!\n");
        goto exit;
    }

    test1();
 
    // Check: Test if security extensions are enabled before testing
    // Test: switching to MON from other modes
    // Test: Call smc from unpriv mode
    // Test: Access to secure memory only allowed from secure mode
    //      pg. B1-1156
    // Test: PL2 and secure not combinable
    //      pg. B1-1157
    //      "hyp mode is only available when NS=1"
    // Test: An axception cannot be taken fromsecure mode to nonsecure (2->1)
    //      pg  B1-1138
    // Test: Check that vector ops are undefined if CPACR/NSACR
    // Test: Check that monitor mode has access to secure resource despite NS
    //      pg. B1-1140
    // Test: Check that access to banked regs from mon mode depend on NS
    // Test: Check that smc takes an exception to monitor mode + secure
    // Test: Check that code residing in secure memory can't be exec from ns
    //      pg. B1-1147
    // Test: Check that CPSR.M unpredictable values don't cause entry to sec
    //      pg. B1-1150
    //      NS state -> mon mode
    //      NSACR.RFR=1 + NS state -> FIQ
    // Test: SCR.FW/AW protects access to CPSR.F/Aa when nonsecure
    //      pg. B1-1151
    //      table B1-2
    // Test: Check that non-banked (shared) regs match between sec and ns
    //      pg. B1-1157
    // Test?: Should e check that SC.NS is deprecated in any mode but mon?
    //      pg. B1-1157
    // Test: Check that mon mode is only avail if sec ext. present
    //      pg. B1-1157
    // Test: Check that smc is only aval if sec. ext. present
    //      pg. B1-1157
    // Test: Check that the SCTLR.V bit is banked allowing mix of vectors
    //      pg. B1-1158
    // Test: Check that the low exception vecotr base address is banked
    //      pg. B1-1158
    // Test: Check that using SCR aborts/irqs/fiqs are routed to mon mode
    //      pg. B1-1158
    // Test: Check that using SCR aborts/irqs/fiqs can be masked
    //      pg. B1-1158
    // Test: Check that SCR.SIF/SCD are not restrictive when EL2 not present
    //      pg. B1-1158
    // Test: Check that distinct vec tables possible for mon/sPL1/nsPL1
    //      pg. B1-1165
    // Test: Check that unused vec tables are not used
    //      table B1-3
    // Test: Check that irq/fiq interrupts routed to mon mode use mvbar entry
    //      pg. B1-1168
    // Test: Check that an exception from mon mode, NS cleared to 0
    //      pg. B1-1170
    // Test: Check that an exception taken from sec state is taken to sec state
    //      in the default mode for the excp.
    //      pg. B1-1173
    // Test: Check that an exception taken from ns state is taken to ns state
    //      in the default mode for the excp.
    //      pg. B1-1173
    // Test: Check that undef exceptions are routed properly
    //      figure: B1-3
    // Test: Check that svc exceptions are routed properly
    //      figure: B1-4
    // Test: Check that prefetch abort exceptions are routed properly
    //      figure: B1-6
    // Test: Check that data abort exceptions are routed properly
    //      figure: B1-7
    // Test: Check that IRQ exceptions are routed properly
    //      figure: B1-8
    // Test: Check that FIQ exceptions are routed properly
    //      figure: B1-9
    // Test: Check that CPSR.A/I.F are set to 1 on exception to mon mode
    //      pg. B1-1182
    // Test: Check that on reset if sec et present, starts in sec state
    //      pg. B1-1204
    // Note: Unaligned access can cause abort in PMSA
    //
    
    printf("\nPassed %d or %d tests\n", test_count-fail_count, test_count);

exit:
    return 0;
}