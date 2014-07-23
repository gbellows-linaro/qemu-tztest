#include "arm32.h"
#include "libcflat.h"

#define SECTION_SECURE               (0 << 19)
#define SECTION_NOTSECURE            (1 << 19)
#define SECTION_SHARED               (1 << 16)
#define SECTION_NOTGLOBAL            (1 << 17)
/* Fix SCTLR.AFE bit */

#define SECTION_P1_RW               ((0 << 15) | (1 << 10))
#define SECTION_P1_RO               ((1 << 15) | (1 << 10))
#define SECTION_P0_RW               ((0 << 15) | (3 << 10))
#define SECTION_P0_RO               ((0 << 15) | (2 << 10))
#define SECTION_P0_NONE             ((0 << 15) | (1 << 10))

#define SECTION_TEXCB(tex, c, b)     ((tex << 12) | (c << 3) | (b << 2))
#define SECTION_DEVICE               SECTION_TEXCB(0, 0, 1)
#define SECTION_NORMAL               SECTION_TEXCB(1, 0, 0)
#define SECTION_NORMAL_CACHED        SECTION_TEXCB(1, 1, 1)
#define SECTION_NO_EXEC              (1 << 4)
#define SECTION_SECTION              (2 << 0)

uint32_t *pagetable_init(uint32_t *ttbr0)
{
    uint32_t size = 0x2000000;
#if 0
    uint32_t num_entries = size >> 20;  /* Number of 1MB sections */
    uint32_t index = 0x14000000 >> 20;

    for (; 0 < num_entries; index++, num_entries--) {
        ttbr0[index] = ((index << 20) | 
                        (1 << 19) |                 /* Nonsecure */
                        (1 << 17) |                 /* Not global */
                        (1 << 16) |                 /* Shared */
                        (0 << 15) | (1 << 10) |     /* RW */
                        (1 <<  2));                 /* Section */
    }
#endif

    int attr = SECTION_SHARED | SECTION_NOTGLOBAL | SECTION_SECTION;
    int m = (0x80000000 >> 20) *4;
    int n = size >> 20;

    attr |= SECTION_NORMAL_CACHED;
    attr |= SECTION_P1_RW;
    attr |= SECTION_P0_RW;
    attr |= SECTION_NOTSECURE;

    while (n--) {
        *((uint32_t *)((uint32_t)ttbr0 + m)) = (m << 18) | attr;
        m += 4;
    }

    m = (0x1c000000 >> 20) *4;
    n = (0x20000000-0x1c000000) >> 20;

    attr |= SECTION_NORMAL;
    attr |= SECTION_P1_RW;
    attr |= SECTION_P0_RW;
    attr |= SECTION_NOTSECURE;

    while (n--) {
        *((uint32_t *)((uint32_t)ttbr0 + m)) = (m << 18) | attr;
        m += 4;
    }

    return ttbr0;
}
