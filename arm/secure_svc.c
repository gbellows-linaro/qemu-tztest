#include "tztest.h"

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
extern int _ram_nsec_base;

int secure_dispatch_usr(int, int);
void secure_svc_loop(int initial_r0, int initial_r1);
void *sec_allocate_secure_memory(int);
extern uint32_t _sec_l1_page_table;
uint32_t *sec_l1_page_table = &_sec_l1_page_table;
extern uint32_t _ram_secvecs_start;
extern uint32_t _ram_sectext_start;
extern uint32_t _ram_secdata_start;
extern uint32_t _secstack_start;
extern uint32_t _secvecs_size;
extern uint32_t _sectext_size;
extern uint32_t _secdata_size;
extern uint32_t _secstack_size;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile int _tztest_exception;
extern volatile int _tztest_exception_addr;
extern volatile int _tztest_exception_status;
volatile int *tztest_exception = &_tztest_exception;
volatile int *tztest_exception_addr = &_tztest_exception_addr;
volatile int *tztest_exception_status = &_tztest_exception_status;
uint32_t ram_secvecs_start = (uint32_t)&_ram_secvecs_start;
uint32_t ram_sectext_start = (uint32_t)&_ram_sectext_start;
uint32_t ram_secdata_start = (uint32_t)&_ram_secdata_start;
uint32_t secstack_start = (uint32_t)&_secstack_start;
uint32_t secvecs_size = (uint32_t)&_secvecs_size;
uint32_t sectext_size = (uint32_t)&_sectext_size;
uint32_t secdata_size = (uint32_t)&_secdata_size;
uint32_t secstack_size = (uint32_t)&_secstack_size;

pagetable_map_entry_t nsec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_nsec_base, .pa = (uint32_t)&_ram_nsec_base,
     .type = SECTION, .len = SECTION_SIZE * 2,
     .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P1_X | P0_R |
             P0_W | P0_X | NONSECURE },
};

pagetable_map_entry_t sysreg_pagetable_entries[] = {
    {.va = SYSREG_BASE, .pa = SYSREG_BASE,
     .type = PAGE, .len = 0x1000,
     .attr = SHARED | NOTGLOBAL | UNCACHED | P1_R | P1_W | P0_R | P0_W |
             NONSECURE },
};

void secure_svc_handler(volatile uint32_t op, volatile tztest_svc_desc_t *desc)
{
    DEBUG_MSG("Entered\n");
    switch (op) {
        case SVC_DISPATCH_MONITOR:
            op = SMC_DISPATCH_MONITOR;
            __smc(op, desc);
            break;
        case SVC_GET_SECURE_STATE:
            /* This SVC handler is only accessible from the secure vector
             * table, so unless something went drastically wrong with the
             * tables, it should be safe to assume we are in a nonsecure state.
             */
            desc->secure_state.state = TZTEST_STATE_SECURE;
            break;
    }
    DEBUG_MSG("Exiting\n");
    return;
}

void secure_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    *tztest_exception = CPSR_MODE_UND;
    *tztest_exception_status = 0;
}

void secure_pabort_handler(int status, int addr) {
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n", status, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_addr = addr;
    *tztest_exception_status = status & 0x1f;
}

void secure_dabort_handler(int status, int addr) {
    DEBUG_MSG("Data Abort: %s\n", FAULT_STR(status & 0x1f));
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n",
              status & 0x1f, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_addr = addr;
    *tztest_exception_status = status & 0x1f;
}

void secure_check_init()
{
    printf("\nValidating startup state:\n");

    printf("\tChecking for security extension ...");
    int idpfr1 = 0;
    /* Read the ID_PFR1 CP register and check that it is marked for support of
     * the security extension.
     */
    __mrc(15, 0, idpfr1, 0, 1, 1);
    if (0x10 != (idpfr1 & 0xf0)) {
        printf("FAILED\n");
        DEBUG_MSG("current IDPFR1 (%d) != expected IDPFR1 (%d)\n",
                  (idpfr1 & 0xf0), 0x10);
        assert(0x10 == (idpfr1 & 0xf0));
    } else {
        printf("PASSED\n");
    }

    printf("\tChecking initial processor mode... ");
    if (CPSR_MODE_SVC != (_read_cpsr() & 0x1f)) {
        printf("FAILED\n");
        DEBUG_MSG("current CPSR (%d) != expected CPSR (%d)\n",
                  (_read_cpsr() & 0x1f), CPSR_MODE_SVC);
        assert(CPSR_MODE_SVC == (_read_cpsr() & 0x1f));
    } else {
        printf("PASSED\n");
    }

    // Test: Check that on reset if sec et present, starts in sec state
    //      pg. B1-1204
    printf("\tChecking initial security state... ");
    if (0 != (_read_scr() & SCR_NS)) {
        printf("Failed\n");
        DEBUG_MSG("current SCR.NS (%d) != expected SCR.NS (%d)\n",
                  (_read_cpsr() & SCR_NS), 0);
        assert(0 == (_read_scr() & SCR_NS));
    } else {
        printf("PASSED\n");
    }
}

void secure_svc_loop(int initial_op, int initial_data)
{
    volatile int op = initial_op;
    tztest_smc_desc_t *data = (tztest_smc_desc_t *)initial_data;
    uint32_t (*func)(uint32_t);

    DEBUG_MSG("Initial call\n");

    while (SMC_EXIT != op) {
        switch (op) {
            case SMC_DISPATCH_SECURE_USR:
                DEBUG_MSG("Dispatching secure USR function\n");
                data->dispatch.ret =
                    secure_dispatch_usr((int)data->dispatch.func,
                                        data->dispatch.arg);
                DEBUG_MSG("Returned from secure USR dispatch\n");
                break;
            case SMC_DISPATCH_SECURE_SVC:
                func = (uint32_t (*)())data->dispatch.func;
                DEBUG_MSG("Dispatching secure SVC function\n");
                data->dispatch.ret = func(data->dispatch.arg);
                DEBUG_MSG("Returned from secure SVC dispatch\n");
                break;
        }
        op = SMC_YIELD;
        __smc(op, data);
        DEBUG_MSG("Handling smc op 0x%x\n", op);
    }

    DEBUG_MSG("Exiting\n");
}

void secure_pagetable_init()
{
    pagetable_map_entry_t sec_pagetable_entries[] = {
        {.va = (uint32_t)0xFFFF0000, .pa = (uint32_t)ram_secvecs_start,
         .type = PAGE, .len = secvecs_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X },
        {.va = (uint32_t)ram_sectext_start, .pa = (uint32_t)ram_sectext_start,
         .type = PAGE, .len = sectext_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X },
        {.va = (uint32_t)ram_secdata_start, .pa = (uint32_t)ram_secdata_start,
         .type = PAGE, .len = secdata_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
        {.va = (uint32_t)secstack_start, .pa = (uint32_t)secstack_start,
         .type = PAGE, .len = secstack_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
        {.va = (uint32_t)sec_l1_page_table, .pa = (uint32_t)sec_l1_page_table,
         .type = SECTION, .len = 16*1024*1024,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W },
    };

    pagetable_init(sec_l1_page_table);

    PT_ADD_ENTRIES(sec_l1_page_table, sysreg_pagetable_entries);
    PT_ADD_ENTRIES(sec_l1_page_table, sec_pagetable_entries);
    PT_ADD_ENTRIES(sec_l1_page_table, nsec_pagetable_entries);

    pagetable_init_common(sec_l1_page_table);
}
