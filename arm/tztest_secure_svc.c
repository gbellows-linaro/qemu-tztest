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

int dispatch_secure_usr(int, int);
void tztest_secure_svc_loop(int initial_r0, int initial_r1);
void *sec_allocate_secure_memory(int);
extern uint32_t sec_l1_page_table;
extern uint32_t _ram_sectext_start;
extern uint32_t _ram_secdata_start;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile int _tztest_exception;
extern volatile int _tztest_exception_status;
extern volatile int _tztest_test_count;
extern volatile int _tztest_fail_count;
volatile int *tztest_test_count = &_tztest_test_count;
volatile int *tztest_fail_count = &_tztest_fail_count;
volatile int *tztest_exception = &_tztest_exception;
volatile int *tztest_exception_status = &_tztest_exception_status;

pagetable_map_entry_t sec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_sectext_start, .pa = (uint32_t)&_ram_sectext_start,
     .size = SECTION_SIZE,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED |
             SECTION_P1_RO | SECTION_P0_RO | SECTION_SECTION },
    {.va = (uint32_t)&_ram_secdata_start, .pa = (uint32_t)&_ram_secdata_start,
     .size = SECTION_SIZE * 2,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED |
             SECTION_P1_RW | SECTION_P0_RW | SECTION_SECTION },
};

pagetable_map_entry_t nsec_pagetable_entries[] = {
    {.va = (uint32_t)&_ram_nsec_base, .pa = (uint32_t)&_ram_nsec_base,
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

pagetable_map_entry_t heap_pagetable_entries[] = {
    {.va = (uint32_t)&_shared_memory_heap_base,
     .pa = (uint32_t)&_shared_memory_heap_base,
     .size = SECTION_SIZE,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_UNCACHED |
             SECTION_P1_RW | SECTION_P0_RW | SECTION_SECTION },
};

void sec_svc_handler(volatile svc_op_t op, volatile tztest_svc_desc_t *desc)
{
    DEBUG_MSG("Entered\n");
    switch (op) {
        case SVC_READ_REG:
            desc->reg_read.val = tztest_read_register(desc->reg_read.reg);
            break;
        case SVC_CHECK_SECURE_STATE:
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

void sec_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    *tztest_exception = CPSR_MODE_UND;
    *tztest_exception_status = 0;
}

void sec_pabort_handler(int status, int addr) {
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n", status, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

void sec_dabort_handler(int status, int addr) {
    DEBUG_MSG("Data Abort: %s\n", FAULT_STR(status & 0x1f));
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n",
              status & 0x1f, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

int secure_test_var = 42;

void *sec_allocate_secure_memory(int len)
{
    DEBUG_MSG("Entered\n");
    DEBUG_MSG("received len = %d\n", len);
    DEBUG_MSG("Exiting\n");

    return &secure_test_var;
}

void check_init_mode()
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

void tztest_secure_svc_loop(int initial_op, int initial_data)
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
                    dispatch_secure_usr((int)data->dispatch.func,
                                        data->dispatch.arg);
                DEBUG_MSG("Returned from secure USR dispatch\n");
                break;
            case SMC_DISPATCH_SECURE_SVC:
                func = (uint32_t (*)())data->dispatch.func;
                DEBUG_MSG("Dispatching secure SVC function\n");
                data->dispatch.ret = func(data->dispatch.arg);
                DEBUG_MSG("Returned from secure SVC dispatch\n");
                break;
            case SMC_ALLOCATE_SECURE_MEMORY:
//                op = (int)sec_allocate_secure_memory(data);
                break;
        }
        DEBUG_MSG("Returning from op 0x%x data 0x%x\n", op, data);
        __smc(op, data);
        DEBUG_MSG("Handling smc op 0x%x\n", op);
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
    count = sizeof(nsec_pagetable_entries) / sizeof(nsec_pagetable_entries[0]);
    pagetable_add_sections(table, nsec_pagetable_entries, 1);
    pagetable_add_sections(table, heap_pagetable_entries, 1);
}

void tztest_secure_svc_init_monitor()
{
    struct sm_nsec_ctx *nsec_ctx;


    /* Set-up the non-secure context so that the switch to nonsecure state
     * resumes at initiallizing the nonsecure svc mode.
     */
    nsec_ctx = sm_get_nsec_ctx();
    nsec_ctx->mon_lr = (uint32_t)&_ram_nsec_base;
    nsec_ctx->mon_spsr = CPSR_MODE_SVC | CPSR_I;
}

void tztest_dispatch_monitor(tztest_smc_desc_t *desc)
{
    uint32_t (*func)() = desc->dispatch.func;
    DEBUG_MSG("Entered\n");
    func();
    DEBUG_MSG("Exiting\n");
}
