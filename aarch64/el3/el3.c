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

state_buf sec_state;
state_buf nsec_state;

void el3_dispatch(tztest_smc_desc_t *desc)
{
    uint32_t (*func)(uint32_t) = desc->dispatch.func;
    DEBUG_MSG("Entered\n");
    desc->dispatch.ret = func(desc->dispatch.arg);
    DEBUG_MSG("Exiting\n");
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
