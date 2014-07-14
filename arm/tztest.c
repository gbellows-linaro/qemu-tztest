#include "libcflat.h"
#include "tztest_builtins.h"
#include "tztest.h"

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
volatile int exception = 0;
volatile int fail_count = 0;
volatile int test_count = 0;

void smc_handler(smc_op_t op, int data) 
{
    test_desc_t *test;
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
        case SMC_TEST:
            test = (test_desc_t *)data;
            test->func(data);
            break;
        case SMC_CATCH:
            exception = MON;
            break;
        default:
            break;
    }
}

void svc_handler(svc_op_t op, int data) {
    test_desc_t *test;
    int cpsr = 0;
    switch(op) {
        case SVC_SMC:
            __smc(op, data);
            break;
        case SVC_TEST:
            test = (test_desc_t *)data;
            if (MON == test->processor_mode || USR == test->processor_mode) {
                break;
            }
            cpsr = _read_cpsr();
            cpsr &= ~0x1f;
            cpsr != test->processor_mode;
            _write_cpsr(cpsr);
            __smc(SMC_SET_SECURE_STATE, test->secure_state);
            test->func(data);
            break;
        case SVC_CATCH:
            exception = SVC;
            break;
        default:
            break;
    }
}

void undef_handler() {
    exception = UND;
}

void unused_handler() {
    exception = -1;
}

void P0_nonsecure_tests(test_desc_t *data) 
{
    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating non-secure P0 inaccessibility:\n");

    printf("\tUnprivileged smc call ... ");
    TEST_EXCP_COND(__smc(SMC_SET_SECURE_STATE, SECURE), UND, ==);
}

/* Validate restricted registers are inaccessible */
void P1_nonsecure_tests(test_desc_t *data) {
    /* Set ourselves in secure P1 and read the values we intend to write below.
     * The intent is to write-back the same value in case the write goes
     * through.
     */
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

void P1_smc_test(test_desc_t *data)
{
    /* Test that calling smc from a given P1 processor mode takes us to monitor
     * mode.
     */
    printf("\nValidating smc call from P1:\n");

    printf("\tCalling smc from %s ... ", MODE_STR(data->processor_mode));
    TEST_EXCP_COND(_smc(SMC_CATCH, 0), MON, ==);
}

void check_init_mode() 
{
    printf("\nValidating startup state:\n");

    printf("Security extension supported...");
    int idpfr1 = 0;
    /* Read the ID_PFR1 CP register and check that it is marked for support of
     * the security extension.
     */
    __mrc(15, 0, idpfr1, 0, 1, 1);
    if (0x10 != (idpfr1 & 0xf0)) {
        printf("/tFAILED\n");
        DEBUG_MSG("current IDPFR1 (%d) != expected IDPFR1 (%d)\n", 
                  (idpfr1 & 0xf0), 0x10);
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;

    printf("\tInitial processor mode... ");
    if (SVC != (_read_cpsr() & 0x1f)) {
        printf("FAILED\n");
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
    test_desc_t test;

    /* Make sure we are starting in secure state */
#if 0
    if (0 != (_read_scr() & 0x1)) {
        printf("Not in secure state!\n");
        goto exit;
    }
#endif

    P0_nonsecure_tests(NULL);
    
    test.processor_mode = SYS;
    test.secure_state = NONSECURE;
    test.func = P1_nonsecure_tests;
    __svc(SVC_TEST, &test);
 
    test.processor_mode = SYS;
    test.secure_state = SECURE;
    test.func = P1_smc_test;
    __svc(SVC_TEST, &test);
    test.processor_mode = SVC;
    __svc(SVC_TEST, &test);
 
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
