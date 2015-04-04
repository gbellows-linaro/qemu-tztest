#include "common_svc.h"
#include "common_mmu.h"
#include "common_defs.h"
#include "platform.h"

pagetable_map_entry_t uart_pagetable_entries[] = {
    {.va = UART0_BASE, .pa = UART0_BASE, .type = PAGE, .len = 0x1000,
     .attr = SHARED | NOTGLOBAL | UNCACHED | P1_R | P1_W | P0_R | P0_W |
             NONSECURE },
};

void pagetable_init_common(uint32_t *table)
{
    PT_ADD_ENTRIES(table, uart_pagetable_entries);
}

#ifdef DEBUG
void validate_state(uint32_t mode, uint32_t state)
{
    tztest_svc_desc_t desc;

    assert((_read_cpsr() & CPSR_MODE_MASK) == mode);

    CLEAR_SVC_DESC(desc);
    __svc(SVC_GET_SECURE_STATE, &desc);
    assert(desc.secure_state.state == state);
}
#else
void validate_state(__attribute__((unused)) uint32_t mode,
                    __attribute__((unused)) uint32_t state) {}
#endif

