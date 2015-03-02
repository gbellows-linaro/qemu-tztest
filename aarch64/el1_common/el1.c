#include "libcflat.h"
#include "platform.h"
#include "smc.h"
#include "svc.h"
#include "el1_loader.h"
#include "string.h"
#include "el1.h"
#include "armv8_exception.h"
#include "armv8_vmsa.h"
#include "arm_builtins.h"

uint64_t el1_next_pa = 0;
uint64_t el1_allocate_pa() {
    uint64_t next = el1_next_pa;
    el1_next_pa += 0x1000;
    return next;
}

void el1_map_va(uintptr_t addr)
{
    uint64_t pa = EL1_S_PGTBL_BASE;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uint64_t off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = el1_allocate_pa();
            *pte = pa;
            *pte = PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    *pte |= PTE_ACCESS;
}

void el1_handle_exception(uint64_t ec, uint64_t iss, uint64_t addr)
{
    armv8_data_abort_iss_t dai = {.raw = iss};
//    armv8_inst_abort_iss_t iai = {.raw = iss};
    switch (ec) {
    case EC_IABORT_LOWER:
        printf("Instruction abort at lower level: address = %0lx\n",
               addr);
        break;
    case EC_IABORT:
        printf("Instruction abort at current level (EL3): address = %0lx\n",
               addr);
        el1_map_va(addr);
        break;
    case EC_DABORT_LOWER:
        printf("Data abort (%s) at lower level: address = %0lx\n",
               dai.wnr ? "write" : "read", addr);
        break;
    case EC_DABORT:
        printf("Data abort (%s) at current level (EL3): address = %0lx\n",
               dai.wnr ? "write" : "read", addr);
        el1_map_va(addr);
        break;

    default:
        printf("Unhandled EL3 exception: EC = %d  ISS = %d\n", ec, iss);
        break;
    }
}

void el1_start()
{
    int i = 0;

    while (1) {
        printf("%d: Entered %s el1_start\n", i, SECURE_STATE);

        __smc(SMC_YIELD);
        i++;
    }

    return;
}
