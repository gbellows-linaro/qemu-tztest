#include "arm32.h"
#include "libcflat.h"
#include "tztest.h"
#include "tztest_mmu.h"

void pagetable_add_sections(uint32_t *ttbrn, pagetable_map_entry_t *entries,
                            uint32_t count)
{
    uint32_t num;  /* Number of 1MB sections */
    uint32_t Vidx;  /* 1MB Index into page table */
    uint32_t Pidx;  /* 1MB Index into page table */
    uint32_t i; 
    uint32_t shift = SECTION_SHIFT, size = SECTION_SIZE;
    
    for (i = 0; i < count; i++) {
        Vidx = entries[i].va >> shift;
        Pidx = entries[i].pa >> shift;
        num = ((entries[i].size + (size-1)) & ~(size-1)) >> shift;
        for (; num > 0; Vidx++, Pidx++, num--) {
            ttbrn[Vidx] = (Pidx << shift) | entries[i].attr;
            DEBUG_MSG("Mapped 0x%x to 0x%x in entry ttbrn[%x] = %x\n", 
                      Vidx << shift, Pidx << shift, Vidx, ttbrn[Vidx]); 
        }
    }
}

void pagetable_init(uint32_t *ttbrn)
{
    int i;

    for (i = 0; i < 4096; i++) {
        ttbrn[0] = 0;
    }
}
