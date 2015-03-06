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
#include "elf.h"
#include "debug.h"

extern void el1_init_el0();

uint64_t el1_next_pa = 0;
uint64_t el1_heap_pool = 0x40000000;
uint64_t el1_allocate_pa() {
    uint64_t next = el1_next_pa;
    el1_next_pa += 0x1000;
    return next;
}

void el1_map_pa(uintptr_t vaddr, uint64_t paddr)
{
    uint64_t pa = EL1_PGTBL_BASE, off;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 3; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = el1_allocate_pa();
            *pte = pa;
            *pte = PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* The last level is the physical page to map */
    off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
    pte = (uint64_t *)(pa | off);
    *pte = paddr & ~0xFFF;
    *pte |= (PTE_PAGE | PTE_ACCESS);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x (PTE:0x%lx)",
              vaddr, paddr, pte);
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
            *pte |= PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    *pte |= (PTE_ACCESS | PTE_USER_RW);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x (PTE:0x%lx)",
                addr, pa, pte);
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
            DEBUG_MSG("Failed unmap: invalid table page");
            /* This is not a valid page, return an error */
            return -1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* Clear the page descriptor */
    *pte = 0;
    DEBUG_MSG("unmapped PTE 0x%lx (VA:0x%lx, PA:0x%x)",
              pte, addr, pa);

    return 0;
}

void *el1_heap_allocate(size_t len)
{
    void *addr = (void *)el1_heap_pool;
    size_t off;

    for (off = 0; off < len; off += 0x1000) {
        el1_map_va(el1_heap_pool + off);
    }

    el1_heap_pool += off;

    return addr;
}

void el1_alloc_mem(alloc_mem_t *alloc)
{
    alloc->addr = el1_heap_allocate(alloc->len);
}

void el1_handle_exception(uint64_t ec, uint64_t iss, svc_op_desc_t *op)
{
    armv8_data_abort_iss_t dai = {.raw = iss};
//    armv8_inst_abort_iss_t iai = {.raw = iss};
    uint64_t elr, far;

    __get_exception_address(far);
    __get_exception_return(elr);

    switch (ec) {
    case EC_SVC32:
    case EC_SVC64:
        printf("SVC exception: iss = %d\n", iss);
        switch (iss) {
        case SVC_EXIT:
            __smc(SMC_EXIT);
            break;
        case SVC_ALLOC:
            el1_alloc_mem((alloc_mem_t *)op);
            break;
        default:
            printf("Unrecognized AArch64 SVC opcode: iss = %d\n", iss);
            break;
        }
    case EC_IABORT_LOWER:
        printf("Instruction abort at lower level: far = %0lx\n", far);
        break;
    case EC_IABORT:
        printf("Instruction abort at EL3: far = %0lx\n", far);
        break;
    case EC_DABORT_LOWER:
        printf("Data abort (%s) at lower level: far = %0lx elr = %0lx\n",
               dai.wnr ? "write" : "read", far, elr);
        el1_map_va(far);
        break;
    case EC_DABORT:
        printf("Data abort (%s) at EL1: far = %0lx elr = %0lx\n",
               dai.wnr ? "write" : "read", far, elr);
        el1_map_va(far);
        break;
    default:
        printf("Unhandled EL1 exception: EC = %d  ISS = %d\n", ec, iss);
        break;
    }
}

/* Simple ELF loader for loading EL0 image */
void *el1_load_el0(char *elfbase, char *start_va)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elfbase;
    size_t off;
    int i;

    /* Map the ELF header in so we can determine how much more to map */
    el1_map_pa((uint64_t)elfbase, (uint64_t)elfbase);

    /* Make sure this is an appropriate ELF image */
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        printf("Invalid ELF header, exiting...\n");
        __smc(SMC_EXIT);
    } else if (ehdr->e_type != ET_DYN &&
               (ehdr->e_machine != EM_ARM || ehdr->e_machine != EM_AARCH64)) {
        printf("Incorrect ELF type (type = %d, machine = %d), exiting...\n",
               ehdr->e_type, ehdr->e_machine);
        __smc(SMC_EXIT);
    } else {
        printf("Loading %s EL0 test image...\n",
               (ehdr->e_machine == EM_ARM) ?  "aarch32" : "aarch64");
    }

    /* Size of the ELF to map */
    size_t elf_len = ehdr->e_shoff + (ehdr->e_shentsize * ehdr->e_shnum);

    /* Finish mapping the remainder of the ELF pages in if any */
    for (off = 0x1000; off < elf_len; off += 0x1000) {
        el1_map_pa((uint64_t)elfbase + off, (uint64_t)elfbase + off);
    }

    Elf64_Shdr *shdr = (Elf64_Shdr *)((char *)elfbase + ehdr->e_shoff);

    Elf64_Shdr *strshdr = &shdr[ehdr->e_shstrndx];
    char *strsec = (char *)ehdr + strshdr->sh_offset;
    for (i = 0; i < ehdr->e_shnum; i++) {
        char *secname = strsec + shdr[i].sh_name;
        if (!strcmp(secname, ".text") || !strcmp(secname, ".data")) {
            uint64_t sect = (uint64_t)((char *)elfbase + shdr[i].sh_offset);
            char *base_va = start_va + shdr[i].sh_addr;
            printf("\tloading %s section: 0x%x bytes @ 0x%lx\n",
                  secname, shdr[i].sh_size, base_va);
            for (off = 0; off < shdr[i].sh_size; off += 0x1000) {
                el1_map_va((uintptr_t)(base_va + off));
                memcpy((void *)(base_va + off), (void *)(sect + off), 0x1000);
            }
        }
    }

    /* Unmap the FLASH ELF image */
    for (off = 0; off < elf_len; off += 0x1000) {
        el1_map_va((uint64_t)elfbase + off);
    }

    return (void *)(start_va + ehdr->e_entry);
}

void el1_start(uint64_t base, uint64_t size)
{
    uint64_t addr = base;
    size_t len;

    printf("EL1 (%s) started...\n", SECURE_STATE);

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + 0xFFF) & ~0xFFF);
         len += 0x1000, addr += 0x1000) {
        el1_unmap_va(addr);
    }

    el1_init_el0();

    return;
}
