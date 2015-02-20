#include "platform.h"
#include "common_svc.h"
#include "common_defs.h"
#include "common_mmu.h"
#include "el3_loader.h"
#include "string.h"

typedef struct state_buf {
    uintptr_t elr_el3;
    uintptr_t spsr_el3;
    uintptr_t sp_el0;
    uintptr_t sp_el1;
    uintptr_t elr_el1;
    uintptr_t esr_el1;
    uintptr_t x[27];        /* X4:X30 */
} state_buf;

state_buf sec_state;
state_buf nsec_state;

extern void monitor_switch(void);
extern void monitor_save_state(state_buf *);
extern void monitor_restore_state(state_buf *);

void monitor_dispatch(tztest_smc_desc_t *desc)
{
    uint32_t (*func)(uint32_t) = desc->dispatch.func;
    DEBUG_MSG("Entered\n");
    desc->dispatch.ret = func(desc->dispatch.arg);
    DEBUG_MSG("Exiting\n");
}

void el3_handle_exception(uint64_t ec, uint64_t iss)
{
    switch (ec) {
    case 0x17:      /* SMC from aarch64 */
        switch (iss) {
        case SMC_YIELD:
            DEBUG_MSG("took an SMC(SMC_YIELD) exception\n");
            monitor_switch();
            break;
        case SMC_DISPATCH_MONITOR:
            DEBUG_MSG("took an SMC(SMC_DSPATCH_MONITOR) exception\n");
            monitor_dispatch(NULL);
            break;
        case SMC_NOOP:
            DEBUG_MSG("took an SMC(SMC_NOOP) exception\n");
            break;
        default:
            printf("Unrecognized AArch64 SMC opcode: iss = %d\n", iss);
        }
        break;
    default:
        printf("Unhandled EL3 exception: EC = %d  ISS = %d\n", ec, iss);
        break;
    }
}

void el3_pagetable_init()
{
    uint64_t el3_l1_page_table = EL3_PGTBL_BASE;

    pagetable_map_entry_t el3_pagetable_entries[] = {
        {.va = EL3_RAM_TEXT, .pa = EL3_RAM_TEXT,
         .type = PAGE, .len = EL3_TEXT_SIZE,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X },
        {.va = EL3_RAM_DATA, .pa = EL3_RAM_DATA,
         .type = PAGE, .len = EL3_DATA_SIZE,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
        {.va = (uintptr_t)EL3_STACK_BASE, .pa = (uintptr_t)EL3_STACK_BASE,
         .type = PAGE, .len = EL3_STACK_SIZE,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
/*
        {.va = (uintptr_t)0xFFFF0000, .pa = (uintptr_t)ram_secvecs_start,
         .type = PAGE, .len = secvecs_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X }
        {.va = (uintptr_t)ram_sectext_start, .pa = (uintptr_t)ram_sectext_start,
         .type = PAGE, .len = sectext_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_X | P0_R | P0_X },
        {.va = (uintptr_t)ram_secdata_start, .pa = (uintptr_t)ram_secdata_start,
         .type = PAGE, .len = secdata_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
        {.va = (uintptr_t)secstack_start, .pa = (uintptr_t)secstack_start,
         .type = PAGE, .len = secstack_size,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W | P0_R | P0_W },
        {.va = (uintptr_t)sec_l1_page_table, .pa = (uintptr_t)sec_l1_page_table,
         .type = SECTION, .len = 16*1024*1024,
         .attr = SHARED | NOTGLOBAL | WBA_CACHED | P1_R | P1_W },
*/
    };

    pagetable_init((void *)el3_l1_page_table);

    PT_ADD_ENTRIES((void *)el3_l1_page_table, el3_pagetable_entries);
}
