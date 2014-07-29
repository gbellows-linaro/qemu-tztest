#include "libcflat.h"
#include "tztest_builtins.h"
#include "tztest.h"
#include "tztest_mmu.h"
#include "sm.h"
#include "arm32.h"
#include "platform.h"

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
volatile int nsec_exception = 0;
volatile int nsec_fail_count = 0;
volatile int nsec_test_count = 0;

int tztest_nonsecure_svc_main();
int tztest_nonsecure_smc_test();
extern uint32_t nsec_l1_page_table;
extern uint32_t _ram_nsectext_start;
extern uint32_t _ram_nsecdata_start;

pagetable_map_entry_t nsec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_nsectext_start, .pa = (uint32_t)&_ram_nsectext_start,
     .size = SECTION_SIZE,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
             SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE |
             SECTION_SECTION }, 
    {.va = (uint32_t)&_ram_nsecdata_start, .pa = (uint32_t)&_ram_nsecdata_start,
     .size = SECTION_SIZE * 2,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
             SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE |
             SECTION_SECTION }, 
};

pagetable_map_entry_t mmio_pagetable_entries[] = {
    {.va = UART0_BASE, .pa = UART0_BASE, .size = SECTION_SIZE,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_UNCACHED | 
             SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE |
             SECTION_SECTION },
};

void nsec_svc_handler(volatile svc_op_t op, volatile int data) {
    volatile int r0, r1, r2, r3;

    switch (op) {
        case 0:
            r0 = 0;
            r1 = data;
            __smc(r0, r1, r2, r3);
            break;
        case 1:
            tztest_nonsecure_smc_test();
            break;
    }
    return;
}

void nsec_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    nsec_exception = CPSR_MODE_UND;
}

void nsec_pabort_handler(int status, int addr) {
    DEBUG_MSG("status = %d\taddress = %d\n", status, addr);
    nsec_exception = CPSR_MODE_ABT;
}

void nsec_dabort_handler(int status, int addr) {
    DEBUG_MSG("status = %d\taddress = %d\n", status, addr);
    nsec_exception = CPSR_MODE_ABT;
}

int tztest_nonsecure_smc_test() 
{
    volatile int r0, r1, r2, r3;
    DEBUG_MSG("Starting\n");
    r0 = 1; r1 = 88;
    __smc(r0, r1, r2, r3);
    r0 = 1; r1 = 42;
    __smc(r0, r1, r2, r3);
    r0 = 1; r1 = 40;
    __smc(r0, r1, r2, r3);
    DEBUG_MSG("Complete\n");
}

void tztest_nonsecure_pagetable_init()
{
    uint32_t *table = &nsec_l1_page_table;
    uint32_t count;

    pagetable_init(table);

    pagetable_add_sections(table, mmio_pagetable_entries, 1);
    count = sizeof(nsec_pagetable_entries) / sizeof(nsec_pagetable_entries[0]);
    pagetable_add_sections(table, nsec_pagetable_entries,  count);
}

int tztest_nonsecure_svc_main() 
{
    test_desc_t test;

#ifdef MMU_ENABLED
    // Test: Access to secure memory only allowed from secure mode
    //      pg. B1-1156
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
    
    printf("\nPassed %d or %d tests\n", 
           nsec_test_count - nsec_fail_count, nsec_test_count);

exit:
    return 0;
}

#if 0
void MON_smc_check(test_desc_t *data)
{
    /* Check that the secure state is as expected.  When called from anywhere
     * besides MON mode, the secure state should be the same as the
     * origination.  If smc is called from monitor mode then the secure state
     * should now be SECURE.
     */
    printf("\tChecking that smc results in switch to secure state ... ");
    if (data->secure_state != (_read_scr() & 0x1)) {
        printf("FAILED\n");
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;

    printf("\tChecking that smc results in switch to monitor mode ... ");
    if (MON != (_read_cpsr() & 0x1f)) {
        printf("FAILED\n");
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;
}

void P1_smc_test(test_desc_t *data)
{
    /* Test that calling smc from a given P1 processor mode takes us to monitor
     * mode.
     */
    __smc(SMC_SET_SECURE_STATE, SECURE);
    printf("\nValidating smc call from %s mode:\n", 
           MODE_STR(data->processor_mode));

    printf("\tChecking that smc causes SMC exception ... ");
    TEST_EXCP_COND(__smc(SMC_CATCH, 0), MON, ==);

    test_desc_t test;
    test.processor_mode = MON;
    test.secure_state = SECURE;
    test.func = MON_smc_check;
    __smc(SMC_TEST, &test);
}

void MON_check_banked_access(test_desc_t *data)
{
    int scr;
    int vbar[2];

    /* Make sure we are in secure state */
    __smc(SMC_NOOP, 0);

#if 0
    scr = _read_scr();

    DEBUG_MSG("SCR = %x\n", scr); 
    printf("\nValidating distinct banked register values:\n");

    printf("\tChecking VBAR banks... ");
    vbar[SECURE] = _read_vbar();
    DEBUG_MSG("vbar[SECURE] = %x\n", vbar[SECURE]);
    _write_scr(scr | NONSECURE);
    DEBUG_MSG("SCR = %x\n", _read_scr()); 
    vbar[NONSECURE] = _read_vbar();
    DEBUG_MSG("vbar[NONSECURE] = %x\n", vbar[NONSECURE]);
    if (vbar[SECURE] == vbar[NONSECURE]) {
        printf("FAILED\n");
            DEBUG_MSG("vbar[SECURE] (%x) == vbar[NONSECURE] (%x)\n",
                      vbar[SECURE], vbar[NONSECURE]);
        fail_count++;
    } else {
        printf("PASSED\n");
    }
    test_count++;

    _write_scr(scr);
#endif
}
#endif
