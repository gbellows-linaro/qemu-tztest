#ifndef _TZTEST_MMU_H
#define _TZTEST_MMU_H

#include <stdint.h>

#define LARGE_PAGE  (0 << 0)
#define PAGE        (1 << 0)
#define SECTION     (1 << 1)
#define SECURE      (0 << 0)
#define NONSECURE   (1 << 0)
#define P0_R        (1 << 1)
#define P0_W        (2 << 1)
#define P0_X        (4 << 1)
#define P0_NONE     (0 << 1)
#define P1_R        (1 << 4)
#define P1_W        (2 << 4)
#define P1_X        (4 << 4)
#define SHARED      (1 << 7)
#define NOTGLOBAL   (1 << 8)
#define DEVICE      (1 << 9)
#define UNCACHED    (0 << 10)
#define WT_CACHED   (1 << 10)
#define WB_CACHED   (2 << 10)
#define WBA_CACHED  (3 << 10)
#define DOMAIN(_d)  ((_d) << 16)

#define L1_TYPE_PAGE                (1 << 0)
#define L1_TYPE_SECTION             ((0 << 18) | (1 << 1))

#define L1_INDEX_BITS               12
#define L1_TABLE_SIZE               ((1 << L1_INDEX_BITS) << 2)
#define L1_INDEX_SHIFT              20
#define L1_MAP_SIZE                 (1 << L1_INDEX_SHIFT)
#define L1_INDEX_MASK               ((1 << L1_INDEX_BITS) - 1)

#define L2_TYPE(_sz)                (1 << (_sz))

#define L2_INDEX_BITS               8
#define L2_TABLE_SIZE               (1 << 10)
#define L2_INDEX_SHIFT              12
#define L2_MAP_SIZE                 (1 << L2_INDEX_SHIFT)
#define L2_INDEX_MASK               ((1 << L2_INDEX_BITS) - 1)

#define BYTE_TO_INT(_val)           ((_val) >> 2)
#define INT_TO_BYTE(_val)           ((_val) << 2)
#define L1_INDEX(_va)               (((_va)>>L1_INDEX_SHIFT) & L1_INDEX_MASK)
#define L2_INDEX(_va)               (((_va)>>L2_INDEX_SHIFT) & L2_INDEX_MASK)

#define SECTION_PXN                 (1 << 0)
#define SECTION_B                   (1 << 2)
#define SECTION_C                   (1 << 3)
#define SECTION_XN                  (1 << 4)
#define SECTION_DOMAIN(_d)          ((_d) << 5)
#define SECTION_NS                  (1 << 19)
#define SECTION_TEX(v)              (1 << 12)
#define SECTION_S                   (1 << 16)
#define SECTION_NG                  (1 << 17)
#define SECTION_SIZE                L1_MAP_SIZE
#define SECTION_PA_SHIFT            L1_INDEX_SHIFT
#define SECTION_PA_MASK             ~((1 << SECTION_PA_SHIFT) - 1)

/* Fix SCTLR.AFE bit */

#define SECTION_P1_RW               ((0 << 15) | (1 << 10))
#define SECTION_P1_RO               ((1 << 15) | (1 << 10))
#define SECTION_P1_NONE             0
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

#define SMALL_PAGE_SIZE             0x1000
#define LARGE_PAGE_SIZE             0x10000
#define PAGE_SIZE(_type)            ((_type == PAGE) \
                                        ? SMALL_PAGE_SIZE : LARGE_PAGE_SIZE)

#define PAGE_PASHIFT                L2_INDEX_SHIFT
#define PAGE_PA_MASK                ~((1 << PAGE_PASHIFT) - 1)
#define PAGE_LARGE                  (0 << 1)
#define PAGE_SMALL                  (1 << 1)
#define PAGE_PXN                    (1 << 2)
#define PAGE_NS                     (1 << 3)
#define PAGE_DOMAIN(_d)             ((_d) << 5)
#define PAGE_B                      (1 << 2)
#define PAGE_C                      (1 << 3)
#define PAGE_LARGE_XN               (1 << 15)
#define PAGE_SMALL_XN               (1 << 0)
#define PAGE_XN(_sz)                ((PAGE_LARGE == (_sz)) \
                                        ? PAGE_LARGE_XN : PAGE_SMALL_XN)
#define PAGE_S                      (1 << 10)
#define PAGE_NG                     (1 << 11)
#define PAGE_LARGE_TEX(_v)          ((_v) << 12)
#define PAGE_SMALL_TEX(_v)          ((_v) << 6)
#define PAGE_TEX(_sz, _v)           (PAGE_LARGE == (_sz)) \
                                        ? PAGE_LARGE_TEX(_v) \
                                        : PAGE_SMALL_TEX(_v)

#define PAGE_P1_RW                  ((0 << 9) | (1 << 4))
#define PAGE_P1_RO                  ((1 << 9) | (1 << 4))
#define PAGE_P1_NONE                0
#define PAGE_P0_RW                  ((0 << 9) | (3 << 4))
#define PAGE_P0_RO                  ((0 << 9) | (2 << 4))
#define PAGE_P0_NONE                ((0 << 9) | (1 << 4))

/* These remap description describe the memory region if SCTLR.TRE = 0
 * (no region atrribute remap).
 */
#define PAGE_DEVICE_SHAREABLE           (PAGE_B)
#define PAGE_DEVICE_UNSHAREABLE(_sz)    (PAGE_TEX((_sz), 2))
#define PAGE_WT_CACHED                  (PAGE_C)
#define PAGE_WB_CACHED                  (PAGE_C | PAGE_B)
#define PAGE_UNCACHED(_sz)              (PAGE_TEX((_sz), 1))
#define PAGE_WBA_CACHED(_sz)            (PAGE_TEX((_sz), 1) | PAGE_C | PAGE_B)

typedef struct {
    uint32_t    type;
    uint32_t    va;
    uint32_t    pa;
    uint32_t    len;
    uint32_t    attr;
} pagetable_map_entry_t;

#define ENTRIES_COUNT(_ent) (sizeof(_ent)/sizeof((_ent)[0]))

#define PT_ADD_ENTRIES(_tbl, _ent) \
            pagetable_add_entries((_tbl), (_ent), ENTRIES_COUNT(_ent))
#define IS_SET(_pe, _attr) ((_attr) == ((_pe)->attr & (_attr)))


extern void pagetable_init(uint32_t  *);
extern void pagetable_init_common(uint32_t *table);
extern void pagetable_add_entries(uint32_t *, pagetable_map_entry_t *,
                                   uint32_t);

#endif
