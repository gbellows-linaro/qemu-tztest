#include "libcflat.h"
#include "armv7_vmsa.h"
#include <stdint.h>
#include <stddef.h>
#include "mem_util.h"

#define SEC_STATE_STR "EL3"
#include "debug.h"

extern uintptr_t mem_pgtbl_base;
extern uintptr_t mem_next_pa;
extern uintptr_t mem_next_l1_page;
extern uintptr_t mem_heap_pool;

uintptr_t mem_allocate_pa()
{
    uintptr_t next = mem_next_pa;
    mem_next_pa += PAGE_SIZE;
    return next;
}

uintptr_t mem_allocate_l1_page()
{
    uintptr_t next = mem_next_l1_page;
    mem_next_pa += PAGE_SIZE;
    return next;
}

void mem_map_pa(uintptr_t vaddr, uintptr_t paddr,
                uintptr_t tblattr, uintptr_t pgattr)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t l1idx, l2idx;
    uintptr_t *pte;

    l1idx = (vaddr >> 20) & 0xFFF;   /* Mask based on TRBCR.N-0 */
    l2idx = (vaddr >> 12) & 0xFF;

    /* Get the L1 PTE address */
    pte = (uintptr_t *)((pa & 0xFFFFC000) | (l1idx << 2));

    /* Check that the PTE is valid and return an error if not */
    if ((*pte & 0x3) == 0x0) {
        pa = mem_allocate_l1_page();
        *pte = pa;
        *pte |= (PTE_TABLE | tblattr);
    }

    /* Ge tthe L2 PTE address */
    pte = (uintptr_t *)((*pte & 0xFFFFFC00) | (l2idx << 2));
    *pte = paddr & ~0xFFF;
    *pte |= (PTE_PAGE | PTE_ACCESS | pgattr);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x - PTE:0x%lx (0x%lx)",
              vaddr, paddr, pte, *pte);
}

void mem_map_va(uintptr_t addr)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t l1idx, l2idx;
    uintptr_t *pte;

    l1idx = (addr >> 20) & 0xFFF;   /* Mask based on TRBCR.N-0 */
    l2idx = (addr >> 12) & 0xFF;

    /* Get the L1 PTE address */
    pte = (uintptr_t *)((pa & 0xFFFFC000) | (l1idx << 2));

    /* Check if the L1 PTE is valid. If not, allocate it now. */
    if ((*pte & 0x3) == 0x0) {
        pa = mem_allocate_l1_page();
        *pte = pa;
        *pte |= PTE_TABLE;
    }

    /* Get the L2 PTE address */
    pte = (uintptr_t *)((*pte & 0xFFFFFC00) | (l2idx << 2));

    pa = mem_allocate_pa();
    *pte = pa;
    *pte |= (PTE_PAGE | PTE_ACCESS | PTE_USER_RW);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x - PTE:0x%lx (0x%lx)",
                addr, pa, pte, *pte);
}

int mem_unmap_va(uintptr_t addr)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t l1idx, l2idx;
    uintptr_t *pte;

    l1idx = (addr >> 20) & 0xFFF;   /* Mask based on TRBCR.N-0 */
    l2idx = (addr >> 12) & 0xFF;

    /* Get the L1 PTE address */
    pte = (uintptr_t *)((pa & 0xFFFFC000) | (l1idx << 2));

    /* Check that the PTE is valid and return an error if not */
    if ((*pte & 0x3) == 0x0) {
        return -1;
    }

    /* Ge tthe L2 PTE address */
    pte = (uintptr_t *)((*pte & 0xFFFFFC00) | (l2idx << 2));

    /* Clear the page descriptor */
    *pte = 0;
    DEBUG_MSG("unmapped PTE 0x%lx (VA:0x%lx, PA:0x%x)",
              pte, addr, pa);

    return 0;
}

void *mem_heap_allocate(size_t len)
{
    void *addr = (void *)mem_heap_pool;
    size_t off;

    for (off = 0; off < len; off += 0x1000) {
        mem_map_va(mem_heap_pool + off);
    }

    mem_heap_pool += off;

    return addr;
}

void *mem_lookup_pa(void *va)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t l1idx, l2idx;
    uintptr_t *pte, addr = (uintptr_t)va;

    l1idx = (addr >> 20) & 0xFFF;   /* Mask based on TRBCR.N-0 */
    l2idx = (addr >> 12) & 0xFF;

    /* Get the L1 PTE address */
    pte = (uintptr_t *)((pa & 0xFFFFC000) | (l1idx << 2));

    /* Check that the PTE is valid and return an error if not */
    if ((*pte & 0x3) == 0x0) {
        DEBUG_MSG("Failed Lookup: invalid table page");
        /* This is not a valid page, return an error */
        return (void *)-1;
    }

    /* Ge tthe L2 PTE address */
    pte = (uintptr_t *)((*pte & 0xFFFFFC00) | (l2idx << 2));

    pa = (*pte & 0xFFFFF000) | (addr & 0xFFF);

    return (void *)pa;
}
