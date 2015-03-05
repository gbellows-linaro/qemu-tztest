#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "el3_loader.h"
#include "string.h"
#include "libcflat.h"
#include "smc.h"
#include "svc.h"
#include "debug.h"
#include "armv8_exception.h"
#include "armv8_vmsa.h"
#include "el3_monitor.h"
#include "arm_builtins.h"

state_buf sec_state;
state_buf nsec_state;

void el3_dispatch(tztest_smc_desc_t *desc)
{
    uint32_t (*func)(uint32_t) = desc->dispatch.func;
    DEBUG_MSG("Entered\n");
    desc->dispatch.ret = func(desc->dispatch.arg);
    DEBUG_MSG("Exiting\n");
}


void el3_shutdown() {
    uintptr_t *sysreg_cfgctrl = (uintptr_t *)(SYSREG_BASE + SYSREG_CFGCTRL);

    printf("Shutting down\n");

    *sysreg_cfgctrl = SYS_SHUTDOWN;

    while(1);   /* Shutdown does not work on all machines */
}

uint64_t el3_next_pa = 0;
uint64_t el3_allocate_pa()
{
    uint64_t next = el3_next_pa;
    el3_next_pa += 0x1000;
    return next;
}

void el3_map_va(uintptr_t addr)
{
    uint64_t pa = EL3_PGTBL_BASE;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uint64_t off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = el3_allocate_pa();
            *pte = pa;
            *pte |= PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    *pte |= PTE_ACCESS;
}

extern void monitor_init(void);

int el3_unmap_va(uint64_t addr)
{
    uint64_t pa = EL3_PGTBL_BASE;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uint64_t off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            /* This is not a valid page, return an error */
            return -1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* Clear the page descriptor */
    *pte = 0;

    return 0;
}

void el3_handle_exception(uint64_t ec, uint64_t iss, uint64_t addr)
{
    armv8_data_abort_iss_t dai = {.raw = iss};
    switch (ec) {
    case EC_SMC64:      /* SMC from aarch64 */
        switch (iss) {
        case SMC_YIELD:
            DEBUG_MSG("took an SMC(SMC_YIELD) exception\n");
            monitor_switch();
            break;
        case SMC_DISPATCH_MONITOR:
            DEBUG_MSG("took an SMC(SMC_DSPATCH_MONITOR) exception\n");
            el3_dispatch(NULL);
            break;
        case SMC_NOOP:
            DEBUG_MSG("took an SMC(SMC_NOOP) exception\n");
            break;
        case SMC_EXIT:
            el3_shutdown();
            break;
        default:
            printf("Unrecognized AArch64 SMC opcode: iss = %d\n", iss);
        }
        break;
    case EC_DABORT_LOWER:
        printf("Data abort (%s) at lower level: address = %0lx\n",
               dai.wnr ? "write" : "read", addr);
        break;
    case EC_DABORT:
        printf("Data abort (%s) at current level (EL3): address = %0lx\n",
               dai.wnr ? "write" : "read", addr);
        el3_map_va(addr);
        break;

    default:
        printf("Unhandled EL3 exception: EC = %d  ISS = %d\n", ec, iss);
        break;
    }
}

void el3_start(uint64_t base, uint64_t size)
{
    uint64_t addr = base;
    size_t len;

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + 0xFFF) & ~0xFFF);
         len += 0x1000, addr += 0x1000) {
        el3_unmap_va(addr);
    }

    /* Clear out our secure and non-secure state buffers */
    memset(&sec_state, 0, sizeof(sec_state));
    memset(&nsec_state, 0, sizeof(nsec_state));

    /* Set-up the secure state buffer to return to the secure initialization
     * sequence. This will occur when we return from exception after monitor
     * initialization.
     */
    sec_state.elr_el3 = EL1_S_FLASH_BASE;
    sec_state.spsr_el3 = 0x5;

    /* Set-up the nonsecure state buffer to return to the non-secure
     * initialization sequence. This will occur on the first monitor context
     * switch (smc) from secure to non-secure.
     */
    nsec_state.elr_el3 = EL1_NS_FLASH_BASE;
    nsec_state.spsr_el3 = 0x5;

    /* Set-up our state to return to secure EL1 to start its init on exception
     * return.
     */
    monitor_restore_state(&sec_state);
    __exception_return();
}
