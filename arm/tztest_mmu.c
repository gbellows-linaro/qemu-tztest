#include "arm32.h"
#include "libcflat.h"
#include "tztest.h"

#define SECTION_SECTION             (1 << 1)
#define SECTION_B                   (1 << 2)
#define SECTION_C                   (1 << 3)
#define SECTION_NO_EXEC             (1 << 4)
#define SECTION_TEX(v)              (1 << 12)
#define SECTION_SHARED              (1 << 16)
#define SECTION_NOTGLOBAL           (1 << 17)
#define SECTION_NONSECURE           (1 << 19)
/* Fix SCTLR.AFE bit */

#define SECTION_P1_RW               ((0 << 15) | (1 << 10))
#define SECTION_P1_RO               ((1 << 15) | (1 << 10))
#define SECTION_P0_RW               ((0 << 15) | (3 << 10))
#define SECTION_P0_RO               ((0 << 15) | (2 << 10))
#define SECTION_P0_NONE             ((0 << 15) | (1 << 10))

/* These remap description describe the memory region if SCTLR.TRE = 0 
 * (no region atrribute remap).
 */
#define SECTION_DEVICE_SHAREABLE    (SECTION_B)
#define SECTION_DEVICE_SHAREABLE    (SECTION_TEX(2))
#define SECTION_WT_CACHED    (SECTION_C)
#define SECTION_WB_CACHED    (SECTION_C | SECTION_B)
#define SECTION_UNCACHED     (SECTION_TEX(1))
#define SECTION_WBA_CACHED   (SECTION_TEX(1) | SECTION_C | SECTION_B)

void pagetable_add_sections(uint32_t *ttbrn, uint32_t base, uint32_t size, 
                            uint32_t attr)
{
    uint32_t Mcnt;  /* Number of 1MB sections */
    uint32_t Midx;  /* 1MB Index into page table */

    attr |= SECTION_SECTION;

    for (Midx = (base >> 20), Mcnt = size >> 20; Mcnt > 0; Midx++, Mcnt--) {
        ttbrn[Midx] = (Midx << 20) | attr;
        DEBUG_MSG("Added ttbrn[%x] (%p) = %x\n", 
                  Midx, &ttbrn[Midx], ttbrn[Midx]); 
    }
}

uint32_t *pagetable_init(uint32_t *ttbrn, uint32_t base)
{
    int attr;

    attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_WBA_CACHED | 
           SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE;
    pagetable_add_sections(ttbrn, base, 0x200000, attr);

    attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_UNCACHED | 
           SECTION_P1_RW | SECTION_P0_RW | SECTION_NONSECURE;
    pagetable_add_sections(ttbrn, 0x1c000000, 0x20000000-0x1c000000, attr);
    
    return ttbrn;
}
