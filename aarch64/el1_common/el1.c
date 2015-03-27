#include "el1_common.h"

smc_op_desc_t *smc_interop_buf;
sys_control_t *syscntl;
uint64_t el1_next_pa = 0;
uint64_t el1_heap_pool = 0x40000000;

uint64_t el1_allocate_pa()
{
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
                desc->get.data = READ_CURRENTEL();
                break;
            case CPTR_EL3:
                desc->get.data = READ_CPTR_EL3();
                break;
            case CPACR:
                desc->get.data = READ_CPACR();
                break;
            case SCR:
                desc->get.data = READ_SCR();
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
                WRITE_CURRENTEL(desc->set.data);
                break;
            case CPTR_EL3:
                WRITE_CPTR_EL3(desc->set.data);
                break;
            case CPACR:
                WRITE_CPACR(desc->set.data);
                break;
            case SCR:
                WRITE_SCR(desc->set.data);
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
        DEBUG_MSG("Data abort at lower level: far = %0lx elr = %0lx\n",
                  far, elr);
        SMC_EXIT();
        break;
    case EC_DABORT:
        DEBUG_MSG("Data abort at EL1: far = %0lx elr = %0lx\n", far, elr);
        SMC_EXIT();
        break;
    case EC_WFI_WFE:
        DEBUG_MSG("WFI/WFE instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

        if (syscntl->el1_excp[SEC_STATE].action == EXCP_ACTION_SKIP ||
            syscntl->excp_action == EXCP_ACTION_SKIP) {
            elr +=4;
            __set_exception_return(elr);
        }
        break;
    default:
        DEBUG_MSG("Unhandled EL1 exception: ec = %d iss = %d\n", ec, iss);
        SMC_EXIT();
        break;
    }
}

void el1_start(uint64_t base, uint64_t size)
{
    uint64_t addr = base;
    size_t len;

    printf("EL1 (%s) started...\n", SEC_STATE_STR);

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
         len += PAGE_SIZE, addr += PAGE_SIZE) {
        el1_unmap_va(addr);
    }

    void *pa = syscntl;
    syscntl = (sys_control_t *)el1_heap_allocate(PAGE_SIZE);
    el1_map_pa((uintptr_t)syscntl, (uintptr_t)pa);
    el1_map_pa((uintptr_t)syscntl->smc_interop.buf_va,
               (uintptr_t)syscntl->smc_interop.buf_pa);
    smc_interop_buf = syscntl->smc_interop.buf_va;

    el1_init_el0();

    return;
}
