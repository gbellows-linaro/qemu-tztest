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
extern int _ram_nsec_base;
volatile int sec_exception = 0;
volatile int sec_fail_count = 0;
volatile int sec_test_count = 0;

void dispatch_secure_usr(int);
void tztest_secure_svc_loop(int initial_r0, int initial_r1);
extern uint32_t sec_l1_page_table;
extern uint32_t _ram_sec_base;
extern uint32_t _ram_sectext_start;
extern uint32_t _ram_secdata_start;

pagetable_map_entry_t sec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_sectext_start, .pa = (uint32_t)&_ram_sectext_start, 
     .size = 0x100000,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
             SECTION_P1_RO | SECTION_P0_RO },
    {.va = (uint32_t)&_ram_secdata_start, .pa = (uint32_t)&_ram_secdata_start, 
     .size = 0x200000,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
             SECTION_P1_RW | SECTION_P0_RW },
};

pagetable_map_entry_t nsec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_nsec_base, .pa = (uint32_t)&_ram_nsec_base, 
     .size = 0x200000,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
             SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE}, 
};

pagetable_map_entry_t mmio_pagetable_entries[] = {
    {.va = UART0_BASE, .pa = UART0_BASE, .size = 0x100000,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_UNCACHED | 
             SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE },
};

void sec_svc_handler(svc_op_t op, int data) {
    DEBUG_MSG("Entered\n");
    switch (op) {
        case 1:
            break;
    }
    DEBUG_MSG("Exiting\n");
    return;
}

void sec_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    sec_exception = CPSR_MODE_UND;
}

void sec_pabort_handler(int status, int addr) {
    DEBUG_MSG("status = %x\taddress = %x\n", status, addr);
    sec_exception = CPSR_MODE_ABT;
}

void sec_dabort_handler(int status, int addr) {
    DEBUG_MSG("status = %x\taddress = %x\n", status, addr);
    sec_exception = CPSR_MODE_ABT;
}

void check_init_mode() 
{
    printf("\nValidating startup state:\n");

    printf("\tSecurity extension supported...");
    int idpfr1 = 0;
    /* Read the ID_PFR1 CP register and check that it is marked for support of
     * the security extension.
     */
    __mrc(15, 0, idpfr1, 0, 1, 1);
    if (0x10 != (idpfr1 & 0xf0)) {
        printf("FAILED\n");
        DEBUG_MSG("current IDPFR1 (%d) != expected IDPFR1 (%d)\n", 
                  (idpfr1 & 0xf0), 0x10);
        sec_fail_count++;
    } else {
        printf("PASSED\n");
    }
    sec_test_count++;

    printf("\tInitial processor mode... ");
    if (CPSR_MODE_SVC != (_read_cpsr() & 0x1f)) {
        printf("FAILED\n");
        DEBUG_MSG("current CPSR (%d) != expected CPSR (%d)\n", 
                  (_read_cpsr() & 0x1f), CPSR_MODE_SVC);
        sec_fail_count++;
    } else {
        printf("PASSED\n");
    }
    sec_test_count++;

    printf("\tInitial security state... ");
    if (0 != (_read_scr() & SCR_NS)) {
        printf("Failed\n");
        DEBUG_MSG("current SCR.NS (%d) != expected SCR.NS (%d)\n", 
                  (_read_cpsr() & SCR_NS), 0);
        sec_fail_count++;
    } else {
        printf("PASSED\n");
    }
    sec_test_count++;
}

void tztest_secure_svc_loop(int initial_r0, int initial_r1)
{
    volatile int r0 = initial_r0, r1 = initial_r1;
    int r2, r3;
    static int loopcnt = 0;
    void (*func)();

    while (-1 < r0) {
        switch (r0) {
            case 0:
                dispatch_secure_usr(r1);
                DEBUG_MSG("Returned from secure USR\n");
                r0 = 0;
                break;
            case 1:
                DEBUG_MSG("Pass %d  r1 = %d\n", loopcnt, r1);
                break;
        }
        loopcnt++;
        __smc(r0, r1, r2, r3);
    } 

    DEBUG_MSG("Exiting\n");
}

void tztest_secure_pagetable_init()
{
    uint32_t *table = &sec_l1_page_table;
    uint32_t count;

    pagetable_init(table);

    pagetable_add_sections(table, mmio_pagetable_entries, 1);
    count = sizeof(sec_pagetable_entries) / sizeof(sec_pagetable_entries[0]);
    pagetable_add_sections(table, sec_pagetable_entries, count);
    pagetable_add_sections(table, nsec_pagetable_entries, 1);
}

void tztest_secure_svc_init_monitor()
{
    struct sm_nsec_ctx *nsec_ctx;
    struct sm_sec_ctx *sec_ctx;

    check_init_mode();

    sm_init(0);

    /* Set-up the non-secure context so that the switch to nonsecure state
     * resumes at initiallizing the nonsecure svc mode.
     */
    nsec_ctx = sm_get_nsec_ctx();
    nsec_ctx->mon_lr = (uint32_t)&_ram_nsec_base;
    nsec_ctx->mon_spsr = CPSR_MODE_SVC | CPSR_I;
}

