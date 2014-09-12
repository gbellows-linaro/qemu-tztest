#include "tztest.h"

uint32_t nonsecure_dispatch_secure_usr_function(uint32_t (*)(uint32_t), uint32_t);
uint32_t nonsecure_dispatch_secure_svc_function(uint32_t (*)(uint32_t), uint32_t);
extern uint32_t _nsec_l1_page_table;
uint32_t *nsec_l1_page_table = &_nsec_l1_page_table;
extern uint32_t _ram_nsectext_start;
extern uint32_t _ram_nsecdata_start;
extern uint32_t _nsecstack_start;
extern uint32_t _nsectext_size;
extern uint32_t _nsecdata_size;
extern uint32_t _nsecstack_size;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile int _tztest_exception;
extern volatile int _tztest_exception_status;
extern volatile int _tztest_exception_addr;
volatile int *tztest_exception = &_tztest_exception;
volatile int *tztest_exception_addr = &_tztest_exception_addr;
volatile int *tztest_exception_status = &_tztest_exception_status;
uint32_t ram_nsectext_start = (uint32_t)&_ram_nsectext_start;
uint32_t ram_nsecdata_start = (uint32_t)&_ram_nsecdata_start;
uint32_t nsecstack_start = (uint32_t)&_nsecstack_start;
uint32_t nsectext_size = (uint32_t)&_nsectext_size;
uint32_t nsecdata_size = (uint32_t)&_nsecdata_size;
uint32_t nsecstack_size = (uint32_t)&_nsecstack_size;
tztest_smc_desc_t smc_desc;

void nonsecure_svc_handler(volatile uint32_t op,
                           volatile tztest_svc_desc_t *desc)
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
                nonsecure_dispatch_secure_usr_function(desc->dispatch.func,
                                                       desc->dispatch.arg);
            DEBUG_MSG("Returning from secure usr function, ret = 0x%x\n",
                      desc->dispatch.ret);
            break;
        case SVC_DISPATCH_SECURE_SVC:
            DEBUG_MSG("Dispatching secure svc function\n");
            desc->dispatch.ret =
                nonsecure_dispatch_secure_svc_function(desc->dispatch.func,
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

void nonsecure_undef_handler() {
    DEBUG_MSG("Undefined exception taken\n");
    *tztest_exception = CPSR_MODE_UND;
    *tztest_exception_status = 0;
}

void nonsecure_pabort_handler(int status, DEBUG_ARG int addr) {
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n", status, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

void nonsecure_dabort_handler(int status, DEBUG_ARG int addr) {
    DEBUG_MSG("Data Abort: %s\n", FAULT_STR(status & 0x1f));
    DEBUG_MSG("status = 0x%x\taddress = 0x%x\n",
              status & 0x1f, addr);
    *tztest_exception = CPSR_MODE_ABT;
    *tztest_exception_status = status & 0x1f;
}

uint32_t nonsecure_dispatch_secure_usr_function(uint32_t (*func)(uint32_t),
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

uint32_t nonsecure_dispatch_secure_svc_function(uint32_t (*func)(uint32_t),
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

void nonsecure_pagetable_init()
{
    pagetable_map_entry_t nsec_pagetable_entries[] = {
        {.va = (uint32_t)ram_nsectext_start, .pa = (uint32_t)ram_nsectext_start,
         .type = PAGE, .len = nsectext_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X |
                 NONSECURE },
        {.va = (uint32_t)ram_nsecdata_start, .pa = (uint32_t)ram_nsecdata_start,
         .type = PAGE, .len = nsecdata_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W |
                 NONSECURE },
        {.va = (uint32_t)nsecstack_start, .pa = (uint32_t)nsecstack_start,
         .type = PAGE, .len = nsecstack_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W |
                 NONSECURE },
        {.va = (uint32_t)nsec_l1_page_table, .pa = (uint32_t)nsec_l1_page_table,
         .type = SECTION, .len = 16*1024*1024,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W },
    };

    pagetable_init(nsec_l1_page_table);

    PT_ADD_ENTRIES(nsec_l1_page_table, nsec_pagetable_entries);

    pagetable_init_common(nsec_l1_page_table);
}
