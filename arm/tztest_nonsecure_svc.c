#include "tztest.h"

uint32_t nsec_dispatch_secure_usr_function(uint32_t (*)(uint32_t), uint32_t);
uint32_t nsec_dispatch_secure_svc_function(uint32_t (*)(uint32_t), uint32_t);
extern uint32_t nsec_l1_page_table;
extern uint32_t _ram_nsectext_start;
extern uint32_t _ram_nsecdata_start;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile int _tztest_test_count;
extern volatile int _tztest_fail_count;
volatile int *tztest_test_count = &_tztest_test_count;
volatile int *tztest_fail_count = &_tztest_fail_count;
extern volatile int _tztest_exception;
extern volatile int _tztest_exception_status;
volatile int *tztest_exception = &_tztest_exception;
volatile int *tztest_exception_status = &_tztest_exception_status;

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

pagetable_map_entry_t heap_pagetable_entries[] = {
    {.va = (uint32_t)&_shared_memory_heap_base,
     .pa = (uint32_t)&_shared_memory_heap_base,
     .size = SECTION_SIZE,
     .attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_UNCACHED |
             SECTION_P1_RW | SECTION_P0_RW | SECTION_SECTION },
};

void nsec_svc_handler(volatile uint32_t op, volatile tztest_svc_desc_t *desc)
{
    int ret = 0;
    switch (op) {
        case SVC_DISPATCH_MONITOR:
            op = SMC_DISPATCH_MONITOR;
            __smc(op, desc);
            break;
        case SVC_DISPATCH_SECURE_USR:
            DEBUG_MSG("Dispatching secure usr function\n");
            desc->dispatch.ret =
                nsec_dispatch_secure_usr_function(desc->dispatch.func,
                                                  desc->dispatch.arg);
            DEBUG_MSG("Returning from secure usr function, ret = 0x%x\n",
                      desc->dispatch.ret);
            break;
        case SVC_DISPATCH_SECURE_SVC:
            DEBUG_MSG("Dispatching secure svc function\n");
            desc->dispatch.ret =
                nsec_dispatch_secure_svc_function(desc->dispatch.func,
                                                  desc->dispatch.arg);
            DEBUG_MSG("Returning from secure svc function, ret = 0x%x\n",
                      desc->dispatch.ret);
            break;
        case SVC_DISPATCH_NONSECURE_SVC:
            DEBUG_MSG("Dispatching nonsecure svc function\n");
            desc->dispatch.ret = desc->dispatch.func(desc->dispatch.arg);
            DEBUG_MSG("Returning from nonsecure svc function, ret = 0x%x\n",
                      desc->dispatch.ret);
            break;
        case SVC_GET_SECURE_STATE:
            /* This SVC handler is only accessible from the nonsecure vector
             * table, so unless something went drastically wrong with the
             * tables, it should be safe to assume we are in a nonsecure state.
             */
            desc->secure_state.state = TZTEST_STATE_NONSECURE;
            break;
        case SVC_EXIT:
            op = SMC_EXIT;
            __smc(op, ret);
            break;
    }
}

void nsec_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    *tztest_exception = CPSR_MODE_UND;
    *tztest_exception_status = 0;
}

void nsec_pabort_handler(int status, DEBUG_ARG int addr) {
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n", status, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

void nsec_dabort_handler(int status, DEBUG_ARG int addr) {
    DEBUG_MSG("Data Abort: %s\n", FAULT_STR(status & 0x1f));
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n",
              status & 0x1f, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

tztest_smc_desc_t smc_desc;
uint32_t nsec_dispatch_secure_usr_function(uint32_t (*func)(uint32_t),
                                           uint32_t arg)
{
    volatile int r0 = SMC_DISPATCH_SECURE_USR;
    tztest_smc_desc_t *desc_p = &smc_desc;

    smc_desc.dispatch.func = func;
    smc_desc.dispatch.arg = arg;
    DEBUG_MSG("Entered\n");
    __smc(r0, desc_p);
    DEBUG_MSG("Exiting, func = 0x%x\n", smc_desc.dispatch.func);
    return smc_desc.dispatch.ret;
}

uint32_t nsec_dispatch_secure_svc_function(uint32_t (*func)(uint32_t),
                                           uint32_t arg)
{
    volatile int op = SMC_DISPATCH_SECURE_SVC;
    tztest_smc_desc_t desc, *desc_p = &desc;

    desc.dispatch.func = func;
    desc.dispatch.arg = arg;
    DEBUG_MSG("Entered op = %x\n", op);
    __smc(op, desc_p);
    DEBUG_MSG("Exiting\n");
    return desc.dispatch.ret;
}

void tztest_nonsecure_pagetable_init()
{
    uint32_t *table = &nsec_l1_page_table;
    uint32_t count;

    pagetable_init(table);

    pagetable_add_sections(table, mmio_pagetable_entries, 1);
    count = sizeof(nsec_pagetable_entries) / sizeof(nsec_pagetable_entries[0]);
    pagetable_add_sections(table, nsec_pagetable_entries,  count);
    pagetable_add_sections(table, heap_pagetable_entries,  1);
}
