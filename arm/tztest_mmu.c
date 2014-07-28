#include "arm32.h"
#include "libcflat.h"
#include "tztest.h"
#include "tztest_mmu.h"

void pagetable_add_sections(uint32_t *ttbrn, pagetable_map_entry_t *entries)
{
    uint32_t Mcnt;  /* Number of 1MB sections */
    uint32_t Midx;  /* 1MB Index into page table */

    uint32_t base = entries[0].va;
    uint32_t size = (entries[0].size + (0x100000-1)) & ~(0x100000-1);
    uint32_t attr = entries[0].attr | SECTION_SECTION;

    DEBUG_MSG("Mapping addresses 0x%x - 0x%x in page table  at %p\n",
              base, base+size, ttbrn);
    for (Midx = (base >> 20), Mcnt = size >> 20; Mcnt > 0; Midx++, Mcnt--) {
        ttbrn[Midx] = (Midx << 20) | attr;
        DEBUG_MSG("Added ttbrn[%x] (%p) = %x\n", 
                  Midx, &ttbrn[Midx], ttbrn[Midx]); 
    }
}

void pagetable_init(uint32_t *ttbrn)
{
    int i;

    for (i = 0; i < 4096; i++) {
        ttbrn[0] = 0;
    }
}
