#include "libcflat.h"
#include "platform.h"
#include "smc.h"
#include "svc.h"
#include "string.h"
#include "el1.h"
#include "el1_loader.h"
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
    uint64_t pa = EL1_PGTBL_BASE;
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

int el1_unmap_va(uint64_t addr)
{
    uint64_t pa = EL1_PGTBL_BASE;
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

void el1_handle_exception(uint64_t ec, uint64_t iss, uint64_t addr)
{
    armv8_data_abort_iss_t dai = {.raw = iss};
//    armv8_inst_abort_iss_t iai = {.raw = iss};
    switch (ec) {
    case EC_SVC32:
    case EC_SVC64:
        printf("Took and SVC exception\n");
        break;
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

void el1_start(uint64_t base, uint64_t size)
{
    int i = 0;
    uint64_t addr = base;
    size_t len;

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + 0xFFF) & ~0xFFF);
         len += 0x1000, addr += 0x1000) {
        if (el1_unmap_va(addr)) {
            printf("Failed to unmap va 0x%x\n", addr);
        } else {
            printf("Unmapped va 0x%x\n", addr);
        }
    }

    while (1) {
        printf("%d: Entered %s el1_start\n", i, SECURE_STATE);

        __smc(SMC_YIELD);
        i++;
    }

    return;
}
