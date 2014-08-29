#include "tztest.h"

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */

uint32_t mon_noop() { return 0; }

/* Function for dispatching an empty SMC call.  Inteded fo use from P0 or P1
 * mode.
 */
static inline uint32_t smc_noop()
{
    asm volatile(".arch_extension sec\n"
                 "push {lr}\n"
                 "mov r0, #0\n"
                 "smc #0\n"
                 "pop {lr}\n");
    return 0;
}

uint32_t P0_nonsecure_check_smc()
{
    validate_state(CPSR_MODE_USR, TZTEST_STATE_NONSECURE);
    printf("\nValidating non-secure P0 smc behavior:\n");
    printf("\tUnprivileged P0 smc call ... ");
    TEST_EXCEPTION(smc_noop(), CPSR_MODE_UND);

    return 0;
}

uint32_t P0_check_register_access(int state)
{
    char *state_str[2] = {"Secure", "Nonsecure"};

    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating %s P0 restricted register access:\n",
           (state == TZTEST_STATE_NONSECURE) ? "nonsecure" : "secure");

    printf("\t%s P0 SCR read ... ", state_str[state]);
    TEST_EXCEPTION(_read_scr(), CPSR_MODE_UND);

    printf("\t%s P0 SCR write ... ", state_str[state]);
    TEST_EXCEPTION(_write_scr(0), CPSR_MODE_UND);

    printf("\t%s P0 SDER read ... ", state_str[state]);
    TEST_EXCEPTION(_read_sder(), CPSR_MODE_UND);

    printf("\t%s P0 SDER write ... ", state_str[state]);
    TEST_EXCEPTION(_write_sder(0), CPSR_MODE_UND);

    printf("\t%s P0 MVBAR read ... ", state_str[state]);
    TEST_EXCEPTION(_read_mvbar(), CPSR_MODE_UND);

    printf("\t%s P0 MVBAR write ... ", state_str[state]);
    TEST_EXCEPTION(_write_mvbar(0), CPSR_MODE_UND);

    printf("\t%s P0 NSACR write ... ", state_str[state]);
    TEST_EXCEPTION(_write_nsacr(0), CPSR_MODE_UND);

    return 0;
}

uint32_t P0_nonsecure_check_register_access()
{
    validate_state(CPSR_MODE_USR, TZTEST_STATE_NONSECURE);

    P0_check_register_access(TZTEST_STATE_NONSECURE);

    return 0;
}

uint32_t P0_secure_check_register_access()
{
    validate_state(CPSR_MODE_USR, TZTEST_STATE_SECURE);

    P0_check_register_access(TZTEST_STATE_SECURE);

    return 0;
}
SECURE_USR_FUNC(P0_secure_check_register_access);

uint32_t P0_nonsecure_check_memory_access()
{
    validate_state(CPSR_MODE_USR, TZTEST_STATE_NONSECURE);

    printf("\nValidating non-secure P0 restricted memory access:\n");

    DEBUG_MSG("Writing %p\n", &_shared_memory_heap_base);
    *(uint32_t *)&_shared_memory_heap_base = 42;

    return 0;
}

uint32_t P1_nonsecure_check_mask_bits()
{
    uint32_t ret = 0;

    validate_state(CPSR_MODE_SVC, TZTEST_STATE_NONSECURE);

    uint32_t cpsr = _read_cpsr();

    /* Test: SCR.FW/AW protects access to CPSR.F/Aa when nonsecure
     *       pg. B1-1151  table B1-2
     */
    printf("\nValidating nonsecure accessiblilty of CPSR:\n");

    /* It is safe to assume that we are in nonsecure state as we validated
     * this on entry.  Set the SCR FW and AW bits and preserve the NS bit.
     * This SCR setting should allow the CPSR to be written from nonsecure
     * state.
     */
    DISPATCH_MONITOR(_write_scr, (SCR_AW | SCR_FW | SCR_NS), ret);

    printf("\tChecking CPSR.F with SCR.FW enabled... ");
    TEST_FUNCTION(_write_cpsr(cpsr | (1 << 6)),
                  ((cpsr | (1 << 6)) == _read_cpsr()));

    /* Restore CPSR to its original value before next test. */
    _write_cpsr(cpsr);

    printf("\tChecking CPSR.A with SCR.AW enabled... ");
    TEST_FUNCTION(_write_cpsr(cpsr | (1 << 8)),
                  (cpsr | (1 << 8)) == _read_cpsr());

    /* Restore CPSR to its original value before next test. */
    _write_cpsr(cpsr);

    /* Switch SCR back to what we likely started with and retry the CPSR
     * writes.  At this point the setting of these bits should be blocked due
     * to the AW and FW bits being clear.
     */
    DISPATCH_MONITOR(_write_scr, SCR_NS, ret);

    printf("\tChecking CPSR.F with SCR.FW disabled... ");
    TEST_FUNCTION(_write_cpsr(cpsr | (1 << 6)),
                  (cpsr == _read_cpsr()));

    /* Restore CPSR to its original value before next test. */
    _write_cpsr(cpsr);

    printf("\tChecking CPSR.A with SCR.AW disabled... ");
    TEST_FUNCTION(_write_cpsr(cpsr | (1 << 8)),
                  (cpsr == _read_cpsr()));

    /* Restore CPSR to its original value */
    _write_cpsr(cpsr);

    return ret;
}

uint32_t MON_check_state()
{
    printf("\nValidating monitor mode:\n");

    uint32_t cpsr = _read_cpsr();

    printf("\tChecking monitor mode... ");
    TEST_CONDITION(CPSR_MODE_MON == (cpsr & CPSR_MODE_MASK));

    // Test: Check that CPSR.A/I.F are set to 1 on exception to mon mode
    //      pg. B1-1182
    printf("\tChecking monitor mode CPSR.F value... ");
    TEST_CONDITION(CPSR_F == (CPSR_F & cpsr));

    printf("\tChecking monitor mode CPSR.I value... ");
    TEST_CONDITION(CPSR_I == (CPSR_I & cpsr));

    printf("\tChecking monitor mode CPSR.A value... ");
    TEST_CONDITION(CPSR_A == (CPSR_A & cpsr));

    return 0;
}

uint32_t MON_check_exceptions()
{
    printf("\nValidating monitor mode exception:\n");

    uint32_t scr = _read_scr();

    /* B1-1211: SMC exceptions from monitor mode cause transition to secure
     *          state.
     * Test: Check that an exception from mon mode, NS cleared to 0
     *       pg. B1-1170
     */
    /* Set our starting security state to secure */
    _write_scr(scr & ~SCR_NS);
    printf("\tChecking state after secure monitor... ");
    TEST_FUNCTION(smc_noop(), !SCR_NS == ((_read_scr() & SCR_NS)));

    /* Set our security state to nonsecure */
    _write_scr(scr | SCR_NS);
    printf("\tChecking state after nonsecure monitor... ");
    TEST_FUNCTION(smc_noop(), !SCR_NS == ((_read_scr() & SCR_NS)));

    /* Restore the original SCR */
    _write_scr(scr);

    return 0;
}

uint32_t P1_nonsecure_novirt_behavior()
{
    int ret = 0;

    validate_state(CPSR_MODE_SVC, TZTEST_STATE_NONSECURE);

    printf("\nValidating non-virtualized behavior:\n");

    DISPATCH_MONITOR(_write_scr, (SCR_SCD | SCR_NS), ret);

    // Check that SCR.SIF/SCD are not restrictive when EL2 not present
    //  pg. B1-1158
    printf("\tChecking SCR.SCD has no effect... ");
    TEST_EXCEPTION(smc_noop(), 0);

    /* Restore SCR to just the nonsecure state */
    DISPATCH_MONITOR(_write_scr, SCR_NS, ret);

    return ret;
}

uint32_t MON_check_banked_regs()
{
    uint32_t scr = _read_scr();
    uint32_t val = 0;

    printf("\nValidating monitor banked register access:\n");

    VERIFY_REGISTER_CUSTOM(csselr, 0xF, TZTEST_SVAL, TZTEST_NSVAL);
    /* Modifying SCTLR is highly disruptive, so the test is heavily restricted
     * to avoid complications.  We only flip the V-bit for comparison which is
     * safe unless we take an exception which should be low risk.
     */
    val = _read_sctlr();
    VERIFY_REGISTER_CUSTOM(sctlr, (1 << 13), val, (val | (1 << 13)));

    /* ACTLR is banked but not supported on Vexpress */

    /* For testing purposes we switch the secure world to the nonsecure page
     * table, so any translations can still be made.  Since we are only working
     * out of the non-secure mappings we should be safe.  This assumption could
     * be wrong.
     */
    VERIFY_REGISTER_CUSTOM(ttbr0, 0xFFF00000,
                           (uint32_t)nsec_l1_page_table, TZTEST_NSVAL);
    VERIFY_REGISTER(ttbr1);

    /* Modifying TTBCR is highly disruptive, so the test is heavily restricted
     * to avoid complications.  We only use the PD1-bit for comparison as we
     * are not using ttbr1 at this time.
     */
    val = _read_ttbcr();

    VERIFY_REGISTER_CUSTOM(ttbcr, (1 << 5), val, (val | (1 << 5)));
    /* Leave the bottom 4 bits alone as they will disrupt address translation
     * otherwise.
     */
    VERIFY_REGISTER_CUSTOM(dacr, 0xFFFFFFF0, 0x55555555, 0xAAAAAAA5);

    VERIFY_REGISTER(dfsr);
    VERIFY_REGISTER(ifsr);
    VERIFY_REGISTER(dfar);
    VERIFY_REGISTER(ifar);
    VERIFY_REGISTER_CUSTOM(par, 0xFFFFF000, TZTEST_SVAL, TZTEST_NSVAL);
    VERIFY_REGISTER(prrr);
    VERIFY_REGISTER(nmrr);

    /* The bottome 5 bits are SBZ */
    VERIFY_REGISTER_CUSTOM(vbar, 0xFFFFFFE0, TZTEST_SVAL, TZTEST_NSVAL);

    VERIFY_REGISTER(fcseidr);
    VERIFY_REGISTER(contextidr);
    VERIFY_REGISTER(tpidrurw);
    VERIFY_REGISTER(tpidruro);
    VERIFY_REGISTER(tpidrprw);

    /* Restore the SCR to it's original value */
    _write_scr(scr);

    return 0;
}

#ifdef DEBUG
uint32_t tztest_div_by_2(uint32_t arg) {return arg/2;}
SECURE_USR_FUNC(tztest_div_by_2);

#define TZTEST_HANDSHAKE_FUNC(_name, _remote)       \
    uint32_t _name(uint32_t arg) {                  \
        uint32_t ret = 0;                           \
        _remote(tztest_div_by_2, arg, ret);         \
        assert(arg/2 == ret);                       \
        return ret;                                 \
    }

TZTEST_HANDSHAKE_FUNC(hs_with_ns_svc, DISPATCH_NONSECURE_SVC)
TZTEST_HANDSHAKE_FUNC(hs_with_s_svc, DISPATCH_SECURE_SVC)
TZTEST_HANDSHAKE_FUNC(hs_with_s_usr, DISPATCH_SECURE_USR)
TZTEST_HANDSHAKE_FUNC(hs_with_mon, DISPATCH_MONITOR)

#define CHECK_HANDSHAKE(_func)                                      \
            _func;                                            \
            if (val/2 != ret) {                                     \
                DEBUG_MSG("Handshake %s FAILED (0x%x != 0x%x)\n",   \
                          #_func, val/2, ret);                      \
                assert(val/2 == ret);                               \
            }                                                       \

void test_handshake()
{
    uint32_t ret = 0;
    uint32_t val = TZTEST_SVAL;

    /* NS_USR -> NS_SVC */
    CHECK_HANDSHAKE(ret = hs_with_ns_svc(val));

    /* NS_USR -> NS_SVC -> S_SVC */
    CHECK_HANDSHAKE(ret = hs_with_s_svc(val));

    /* NS_USR -> NS_SVC -> S_SVC -> S_USR */
    CHECK_HANDSHAKE(ret = hs_with_s_usr(val));

    /* NS_USR -> NS_SVC -> MON */
    CHECK_HANDSHAKE(ret = hs_with_mon(val));

    /* NS_SVC -> S_SVC */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_s_svc, val, ret));

    /* NS_SVC -> S_USR */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_s_usr, val, ret));

    /* NS_SVC -> MON */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_mon, val, ret));

    /* S_SVC -> MON */
    CHECK_HANDSHAKE(DISPATCH_SECURE_SVC(hs_with_mon, val, ret));
}
#endif

uint32_t tztest_nonsecure_usr_main()
{
    uint32_t ret = 0;

    DEBUG_MSG("Entered\n");
    *tztest_test_count = 0;
    *tztest_fail_count = 0;

    P0_nonsecure_check_smc();
    P0_nonsecure_check_register_access();

#ifdef DEBUG
    test_handshake();
#endif

    DISPATCH_SECURE_USR(P0_secure_check_register_access, 0, ret);

    DISPATCH_MONITOR(MON_check_state, 0, ret);
    DISPATCH_MONITOR(MON_check_exceptions, 0, ret);
    DISPATCH_MONITOR(MON_check_banked_regs, 0, ret);

    DISPATCH_NONSECURE_SVC(P1_nonsecure_check_mask_bits, 0, ret);

    DISPATCH_NONSECURE_SVC(P1_nonsecure_novirt_behavior, 0, ret);

    printf("\nValidation complete.  Passed %d of %d tests\n",
           *tztest_test_count-*tztest_fail_count, *tztest_test_count);

    DEBUG_MSG("Exiting\n");

    return ret;
}

#ifdef MMU_ENABLED
    // Test: Access to secure memory only allowed from secure mode
    //      pg. B1-1156
    // Test: Check that monitor mode has access to secure resource despite NS
    //      pg. B1-1140
#endif
#ifdef VIRT_ENABLED
    // Test: PL2 and secure not combinable
    //      pg. B1-1157
    //      "hyp mode is only available when NS=1"
    // Test: An axception cannot be taken fromsecure mode to nonsecure (2->1)
    //      pg  B1-1138
#endif
#ifdef SIMD_ENABLED
    // Test: Check that vector ops are undefined if CPACR/NSACR
#endif
#ifdef IRQ_ENABLED
    // Test: Check that CPSR.M unpredictable values don't cause entry to sec
    //      pg. B1-1150
    //      NS state -> mon mode
    //      NSACR.RFR=1 + NS state -> FIQ
    // Test: Check that using SCR aborts/irqs/fiqs are routed to mon mode
    //      pg. B1-1158
    // Test: Check that using SCR aborts/irqs/fiqs can be masked
    //      pg. B1-1158
    // Test: Check that irq/fiq interrupts routed to mon mode use mvbar entry
    //      pg. B1-1168
    // Test: Check that IRQ exceptions are routed properly
    //      figure: B1-8
    // Test: Check that FIQ exceptions are routed properly
    //      figure: B1-9
#endif
#ifdef NONTZ_TEST
    // Test: Check that mon mode is only avail if sec ext. present
    //      pg. B1-1157
    // Test: Check that smc is only aval if sec. ext. present
    //      pg. B1-1157
#endif
#ifdef FRAMEWORK_TESTED
    // Test: Check that distinct vec tables possible for mon/sPL1/nsPL1
    //      pg. B1-1165
    // Test: SCR.SIF does not disallow secure exec of nonsecure memory when the
    //       virt extesnion is not present.
#endif
    // Test?: Check that code residing in secure memory can't be exec from ns
    //      pg. B1-1147
    // Test: Check that the SCTLR.V bit is banked allowing mix of vectors
    //      pg. B1-1158
    // Test: Check that the low exception vecotr base address is banked
    //      pg. B1-1158
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
