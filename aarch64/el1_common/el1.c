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
#include "syscntl.h"

extern void el1_init_el0();

smc_op_desc_t *smc_interop_buf;
sys_control_t *syscntl;
uint64_t el1_next_pa = 0;
uint64_t el1_heap_pool = 0x40000000;

uint64_t el1_allocate_pa() {
    uint64_t next = el1_next_pa;
    el1_next_pa += 0x1000;
    return next;
}

void el1_map_pa(uintptr_t vaddr, uintptr_t paddr)
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
            *pte |= PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* The last level is the physical page to map */
    off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
    pte = (uint64_t *)(pa | off);
    *pte = paddr & ~0xFFF;
    *pte |= (PTE_PAGE | PTE_ACCESS | PTE_USER_RW);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x (PTE:0x%lx)",
              vaddr, paddr, pte);
}

void el1_map_va(uintptr_t addr)
{
    uint64_t pa = EL1_PGTBL_BASE, off;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
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

void el1_alloc_mem(op_alloc_mem_t *alloc)
{
    alloc->addr = el1_heap_allocate(alloc->len);
}

void *el1_lookup_pa(void *va)
{
    uint64_t pa = EL1_PGTBL_BASE;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uint64_t off = ((((uint64_t)va) >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            DEBUG_MSG("Failed Lookup: invalid table page");
            /* This is not a valid page, return an error */
            return (void *)-1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    return (void *)pa;
}

void el1_map_secure(op_map_mem_t *map)
{
    if ((map->type & OP_MAP_EL3) == OP_MAP_EL3) {
        smc_op_desc_t *desc = (smc_op_desc_t *)smc_interop_buf;
        memcpy(desc, map, sizeof(op_map_mem_t));
        if (desc->map.pa) {
            desc->map.pa = el1_lookup_pa(desc->map.va);
        }
        __smc(SMC_OP_MAP, desc);
    } else {
        el1_map_pa((uint64_t)map->va, (uint64_t)map->pa);
    }
}

void el1_interop_test(op_test_t *desc)
{
    op_test_t *test = (op_test_t*)smc_interop_buf;

    memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
    if (test->val != test->orig >> test->count) {
        test->fail++;
    }
    test->val >>= 1;
    test->count++;

    __smc(SMC_OP_TEST, smc_interop_buf);
    if (test->val != test->orig >> test->count) {
        test->fail++;
    }
    test->val >>= 1;
    test->count++;
    memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
}

int el1_handle_svc(uint32_t op, svc_op_desc_t *desc)
{
    uint32_t ret = 0;

    DEBUG_MSG("Took an svc(%s) - desc = %p\n", svc_op_name[op], desc);
    switch (op) {
    case SVC_OP_EXIT:
        SMC_EXIT();
        break;
    case SVC_OP_YIELD:
        ret = SMC_YIELD();
        memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        break;
    case SVC_OP_ALLOC:
        el1_alloc_mem((op_alloc_mem_t *)desc);
        break;
    case SVC_OP_MAP:
        el1_map_secure((op_map_mem_t *)desc);
        break;
    case SVC_OP_GET_SYSCNTL:
        desc->get.data = (uint64_t)syscntl;
        break;
    case SVC_OP_GET_REG:
        if (desc->get.el == 1) {
            switch (desc->get.key) {
            case CURRENTEL:
                desc->get.data = read_currentel();
                break;
            case CPTR_EL3:
                desc->get.data = read_cptr_el3();
                break;
            case CPACR_EL1:
                desc->get.data = read_cpacr_el1();
                break;
            }
        } else if (desc->get.el == 3) {
            memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
            __smc(SMC_OP_GET_REG, smc_interop_buf);
            memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        }
        break;
    case SVC_OP_SET_REG:
        if (desc->set.el == 1) {
            switch (desc->set.key) {
            case CURRENTEL:
                read_currentel(desc->set.data);
                break;
            case CPTR_EL3:
                read_cptr_el3(desc->set.data);
                break;
            case CPACR_EL1:
                read_cpacr_el1(desc->set.data);
                break;
            }
        } else if (desc->set.el == 3) {
            memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
            __smc(SMC_OP_SET_REG, smc_interop_buf);
        }
        break;
    case SVC_OP_TEST:
        el1_interop_test((op_test_t *)desc);
        break;
    case SVC_OP_DISPATCH:
        memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
        __smc(SMC_OP_DISPATCH, smc_interop_buf);
        memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        break;
    default:
        DEBUG_MSG("Unrecognized AArch64 SVC opcode: op = %d\n", op);
        break;
    }

    return ret;
}

void el1_handle_exception(uint64_t ec, uint64_t iss)
{
#ifdef DEBUG
    armv8_data_abort_iss_t dai = {.raw = iss};
//    armv8_inst_abort_iss_t iai = {.raw = iss};
#endif
    uint64_t elr, far;

    __get_exception_address(far);
    __get_exception_return(elr);

    if (syscntl->excp_log || syscntl->el1_excp[SEC_STATE].log) {
        syscntl->el1_excp[SEC_STATE].taken = true;
        syscntl->el1_excp[SEC_STATE].ec = ec;
        syscntl->el1_excp[SEC_STATE].iss = iss;
        syscntl->el1_excp[SEC_STATE].far = far;
    }

    switch (ec) {
    case EC_UNKNOWN:
        DEBUG_MSG("Unknown exception far = 0x%lx  elr = 0x%lx\n", far, elr);

        if (syscntl->el1_excp[SEC_STATE].action == EXCP_ACTION_SKIP ||
            syscntl->excp_action == EXCP_ACTION_SKIP) {
            elr +=4;
            __set_exception_return(elr);
        }
        break;

    case EC_IABORT_LOWER:
        DEBUG_MSG("Instruction abort at lower level: far = %0lx\n", far);
        SMC_EXIT();
        break;
    case EC_IABORT:
        DEBUG_MSG("Instruction abort at EL1: far = %0lx\n", far);
        SMC_EXIT();
        break;
    case EC_DABORT_LOWER:
        DEBUG_MSG("Data abort (%s) at lower level: far = %0lx elr = %0lx\n",
                  dai.wnr ? "write" : "read", far, elr);
        SMC_EXIT();
        break;
    case EC_DABORT:
        DEBUG_MSG("Data abort (%s) at EL1: far = %0lx elr = %0lx\n",
                  dai.wnr ? "write" : "read", far, elr);
        SMC_EXIT();
        break;
    default:
        DEBUG_MSG("Unhandled EL1 exception: EC = %d  ISS = %d\n", ec, iss);
        SMC_EXIT();
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
        SMC_EXIT();
    } else if (ehdr->e_type != ET_DYN &&
               (ehdr->e_machine != EM_ARM || ehdr->e_machine != EM_AARCH64)) {
        printf("Incorrect ELF type (type = %d, machine = %d), exiting...\n",
               ehdr->e_type, ehdr->e_machine);
        SMC_EXIT();
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
            DEBUG_MSG("\tloading %s section: 0x%x bytes @ 0x%lx\n",
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

    printf("EL1 (%s) started...\n", SEC_STATE_STR);

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + 0xFFF) & ~0xFFF);
         len += 0x1000, addr += 0x1000) {
        el1_unmap_va(addr);
    }

    void *pa = syscntl;
    syscntl = (sys_control_t *)el1_heap_allocate(0x1000);
    el1_map_pa((uintptr_t)syscntl, (uintptr_t)pa);
    el1_map_pa((uintptr_t)syscntl->smc_interop.buf_va,
               (uintptr_t)syscntl->smc_interop.buf_pa);
    smc_interop_buf = syscntl->smc_interop.buf_va;

    el1_init_el0();

    return;
}
