#include "common_svc.h"
#include "common_mmu.h"
#include "common_defs.h"

void pagetable_add_section_entries(uint32_t *ttbr, pagetable_map_entry_t *pe)
{
    uint32_t va = pe->va;
    uint32_t pa = pe->pa;
    uint32_t len = pe->len;
    uint32_t l1idx = va >> L1_INDEX_SHIFT;
    uint32_t l1attr = SECTION;
    uint32_t size = SECTION_SIZE;

    l1attr |= IS_SET(pe, NONSECURE) ?  SECTION_NS : 0;
    l1attr |= IS_SET(pe, P0_R)
              ? IS_SET(pe, P0_W) ? SECTION_P0_RW : SECTION_P0_RO
              : SECTION_P0_NONE;
    l1attr |= IS_SET(pe, P1_R)
              ? IS_SET(pe, P1_W) ? SECTION_P1_RW : SECTION_P1_RO
              : SECTION_P1_NONE;
    l1attr |= IS_SET(pe, P0_X) ? 0 : SECTION_XN;
    l1attr |= IS_SET(pe, P1_X) ? 0 : SECTION_PXN;
    l1attr |= IS_SET(pe, NOTGLOBAL) ? (SECTION_NG) : 0;
    l1attr |= IS_SET(pe, DEVICE)
              ? IS_SET(pe, SHARED)
                  ? SECTION_DEVICE_SHAREABLE : SECTION_DEVICE_UNSHAREABLE
              : IS_SET(pe, SHARED) ? (SECTION_S) : 0;
    l1attr |= IS_SET(pe, WT_CACHED)
                  ? SECTION_WT_CACHED : IS_SET(pe, WB_CACHED)
                      ? SECTION_WB_CACHED
                      : IS_SET(pe, WBA_CACHED)
                          ? SECTION_WBA_CACHED : SECTION_UNCACHED;

    /* Round up to the next segment increment */
    len = (len + (SECTION_SIZE-1)) & ~(SECTION_SIZE-1);

    for (; len > 0; va += size, pa += size, len -= size) {
        l1idx = L1_INDEX(va);
        ttbr[l1idx] = (pa & SECTION_PA_MASK) | l1attr;
        DEBUG_MSG("Mapped L1 0x%x to 0x%x (%p (0x%x) = 0x%x)\n",
                   va, pa, &ttbr[l1idx], l1idx, ttbr[l1idx]);
    }
}

void pagetable_add_page_entries(uint32_t *ttbr, pagetable_map_entry_t *pe)
{
    uint32_t va = pe->va;
    uint32_t pa = pe->pa;
    uint32_t len = pe->len;
    uint32_t l2len = 0;;
    uint32_t *l2base = ttbr + BYTE_TO_INT(L1_TABLE_SIZE);
    uint32_t *l2addr;
    uint32_t size = PAGE_SIZE(PAGE);
    uint32_t l1attr = L1_TYPE_PAGE, l2attr = L2_TYPE(pe->type);
    uint32_t l1idx, l2idx;

    l1attr |= IS_SET(pe, NONSECURE) ?  PAGE_NS : 0;
    l1attr |= IS_SET(pe, P1_X) ? 0 : PAGE_PXN;

    l2attr |= IS_SET(pe, P0_R)
            ? IS_SET(pe, P0_W) ? PAGE_P0_RW : PAGE_P0_RO
            : PAGE_P0_NONE;
    l2attr |= IS_SET(pe, P1_R)
            ? IS_SET(pe, P1_W) ? PAGE_P1_RW : PAGE_P1_RO
            : PAGE_P1_NONE;
    l2attr |= IS_SET(pe, P0_X) ? 0 : PAGE_XN(PAGE_SMALL);
    l2attr |= IS_SET(pe, NOTGLOBAL) ? (PAGE_NG) : 0;
    l2attr |= IS_SET(pe, DEVICE)
            ? IS_SET(pe, SHARED)
                ? PAGE_DEVICE_SHAREABLE
                : PAGE_DEVICE_UNSHAREABLE(PAGE_SMALL)
            : IS_SET(pe, SHARED) ? (PAGE_S) : 0;
    l2attr |= IS_SET(pe, WT_CACHED)
                ? PAGE_WT_CACHED
                : IS_SET(pe, WB_CACHED)
                    ? PAGE_WB_CACHED
                    : IS_SET(pe, WBA_CACHED)
                        ? PAGE_WBA_CACHED(PAGE_SMALL)
                        : PAGE_UNCACHED(PAGE_SMALL);

    /* Round up to the next page increment */
    len = (len + (PAGE_SIZE(PAGE)-1)) & ~(PAGE_SIZE(PAGE)-1);

    while (len > 0) {
        l1idx = L1_INDEX(va);
        l2addr = l2base + BYTE_TO_INT(L2_TABLE_SIZE * l1idx);
        ttbr[l1idx] = (uint32_t)l2addr | l1attr;

        DEBUG_MSG("Added L1 0x%x to 0x%x (ttbrn[%x] = %x)\n",
                  va, l2addr, l1idx, ttbr[l1idx]);

        /* Set the level 2 page table size to the smaller of the total
         * remaining length or the remaining length in the 1MB section.
         */
        l2len = L1_MAP_SIZE - (va & (L1_MAP_SIZE - 1));
        l2len = (l2len > len) ? len : l2len;

        for (; l2len > 0; va += size, pa += size, l2len -= size, len -= size) {
            l2idx = L2_INDEX(va);
            l2addr[l2idx] = (pa & PAGE_PA_MASK) | l2attr;
            DEBUG_MSG("Mapped L2 0x%x to 0x%x (%p (0x%x) = 0x%x)\n",
                       va, pa, &l2addr[l2idx], l2idx, l2addr[l2idx]);
        }
    }
}

void pagetable_add_entries(uint32_t *ttbr, pagetable_map_entry_t *entries,
                           uint32_t count)
{
    uint32_t i;

    for (i = 0; i < count; i++) {
        if (entries[i].type == SECTION) {
            pagetable_add_section_entries(ttbr, &entries[i]);
        } else if (entries[i].type == PAGE || entries[i].type == LARGE_PAGE) {
            pagetable_add_page_entries(ttbr, &entries[i]);
        }
    }
}

void pagetable_init(uint32_t *ttbr)
{
    int i;

    for (i = 0; i < BYTE_TO_INT(L1_TABLE_SIZE); i++) {
        ttbr[i] = 0;
    }
}
