
#ifndef _TZTEST_MMU_H
#define _TZTEST_H

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
#define SECTION_DEVICE_UNSHAREABLE  (SECTION_TEX(2))
#define SECTION_WT_CACHED           (SECTION_C)
#define SECTION_WB_CACHED           (SECTION_C | SECTION_B)
#define SECTION_UNCACHED            (SECTION_TEX(1))
#define SECTION_WBA_CACHED          (SECTION_TEX(1) | SECTION_C | SECTION_B)

typedef struct {
    uint32_t    va;
    uint32_t    pa;
    uint32_t    size;
    uint32_t    attr;
} pagetable_map_entry_t;

extern void pagetable_init(uint32_t  *);
extern void pagetable_add_sections(uint32_t *, pagetable_map_entry_t *);

#endif
