#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "el3_loader.h"
#include "string.h"
#include "libcflat.h"
#include "smc.h"
#include "svc.h"
#include "armv8_exception.h"
#include "armv8_vmsa.h"
#include "el3_monitor.h"
#include "arm_builtins.h"
#include "syscntl.h"

#define SEC_STATE_STR "EL3"
#include "debug.h"

state_buf sec_state;
state_buf nsec_state;

sys_control_t *syscntl;
smc_op_desc_t *smc_interop_buf;

uint64_t el3_next_pa = 0;
uint64_t el3_heap_pool = 0xF800000000;

void el3_shutdown() {
    uintptr_t *sysreg_cfgctrl = (uintptr_t *)(SYSREG_BASE + SYSREG_CFGCTRL);

    printf("\nTest complete\n");

    *sysreg_cfgctrl = SYS_SHUTDOWN;

    while(1);   /* Shutdown does not work on all machines */
}

uint64_t el3_allocate_pa()
{
    uint64_t next = el3_next_pa;
    el3_next_pa += 0x1000;
    return next;
}

void el3_map_pa(uintptr_t vaddr, uint64_t paddr)
{
    uint64_t pa = EL3_PGTBL_BASE, off;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 3; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = el3_allocate_pa();
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
    *pte |= (PTE_PAGE | PTE_ACCESS);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x (PTE:0x%lx)",
              vaddr, paddr, pte);
}

void el3_map_va(uintptr_t addr)
{
    uint64_t pa = EL3_PGTBL_BASE, off;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = el3_allocate_pa();
            *pte = pa;
            *pte |= PTE_PAGE;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    *pte |= PTE_ACCESS;
}

int el3_unmap_va(uint64_t addr)
{
    uint64_t pa = EL3_PGTBL_BASE;
    uint32_t i;
    uint64_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uint64_t off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uint64_t *)(pa | off);
        if (!(*pte & 0x1)) {
            /* This is not a valid page, return an error */
            return -1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* Clear the page descriptor */
    *pte = 0;

    return 0;
}

void *el3_heap_allocate(size_t len)
{
    void *addr = (void *)el3_heap_pool;
    size_t off;

    for (off = 0; off < len; off += 0x1000) {
        el3_map_va(el3_heap_pool + off);
    }

    el3_heap_pool += off;

    return addr;
}

void el3_alloc_mem(op_alloc_mem_t *alloc)
{
    alloc->addr = el3_heap_allocate(alloc->len);
}

void *el3_lookup_pa(void *va)
{
    uint64_t pa = EL3_PGTBL_BASE;
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

uint32_t el3_map_mem(op_map_mem_t *map)
{
    if ((map->type & OP_MAP_EL3) == OP_MAP_EL3) {
        el3_map_pa((uintptr_t)map->va, (uintptr_t)map->pa);
        DEBUG_MSG("Mapped VA:0x%lx to PA:0x%lx\n", map->va, map->pa);
    }

    if ((map->type & OP_MAP_SEC_EL1) == OP_MAP_SEC_EL1) {
        map->type &= ~OP_MAP_EL3;
        DEBUG_MSG("Initiating SVC_OP_MAP\n");
        return SVC_OP_MAP;
    }

    return 0;
}

int el3_handle_smc(uint64_t op, smc_op_desc_t *desc)
{
    op_test_t *test = (op_test_t*)desc;

    DEBUG_MSG("Took an smc(%s) - desc = %p\n", smc_op_name[op], desc);
    switch (op) {
    case SMC_OP_YIELD:
        return SVC_OP_YIELD;
        break;
    case SMC_OP_MAP:
        return el3_map_mem((op_map_mem_t *)desc);
        break;
    case SMC_OP_NOOP:
        break;
    case SMC_OP_EXIT:
        el3_shutdown();
        break;
    case SMC_OP_DISPATCH:
        return SVC_OP_DISPATCH;
        break;
    case SMC_OP_TEST:
        if (test->val != test->orig >> test->count) {
            test->fail++;
        }
        test->val >>= 1;
        test->count++;
        return SVC_OP_TEST;
    case SMC_OP_GET_REG:
        if (desc->get.el == 3) {
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
            case SCR_EL3:
                desc->get.data = read_scr_el3();
                break;
            }
        }
        break;
    case SMC_OP_SET_REG:
        if (desc->set.el == 3) {
            switch (desc->set.key) {
            case CURRENTEL:
                write_currentel(desc->set.data);
                break;
            case CPTR_EL3:
                write_cptr_el3(desc->set.data);
                break;
            case CPACR_EL1:
                write_cpacr_el1(desc->set.data);
                break;
            case SCR_EL3:
                write_scr_el3(desc->set.data);
                break;
            }
        }
        break;
    default:
        printf("Unrecognized AArch64 SMC opcode: op = %d\n", op);
        el3_shutdown();
        break;
    }

    return 0;
}

int el3_handle_exception(uint64_t ec, uint64_t iss)
{
    armv8_data_abort_iss_t dai = {.raw = iss};
    uint64_t elr, far;

    __get_exception_address(far);
    __get_exception_return(elr);

    if (syscntl->excp_log || syscntl->el3_excp.log) {
        syscntl->el3_excp.taken = true;
        syscntl->el3_excp.ec = ec;
        syscntl->el3_excp.iss = iss;
        syscntl->el3_excp.far = far;
    }

    switch (ec) {
    case EC_SMC64:
    case EC_SMC32:
        printf("Took an SMC exception from EL3\n");
        break;
    case EC_IABORT_LOWER:
        printf("Instruction abort at lower level: far = %0lx\n", far);
        el3_shutdown();
        break;
    case EC_IABORT:
        printf("Instruction abort at EL3: far = %0lx\n", far);
        el3_shutdown();
        break;
    case EC_DABORT_LOWER:
        printf("Data abort (%s) at lower level: far = %0lx elr = %0lx\n",
               dai.wnr ? "write" : "read", far, elr);
        el3_shutdown();
        break;
    case EC_DABORT:
        printf("Data abort (%s) at EL3: far = %0lx elr = %0lx\n",
               dai.wnr ? "write" : "read", far, elr);
        el3_shutdown();
        break;
    case EC_SYSINSN:
        DEBUG_MSG("System instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->el3_excp.action == EXCP_ACTION_SKIP ||
            syscntl->excp_action == EXCP_ACTION_SKIP) {
            elr +=4;
            __set_exception_return(elr);
        }
        break;
    case EC_WFI_WFE:
        DEBUG_MSG("WFI/WFE instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->el3_excp.action == EXCP_ACTION_SKIP ||
            syscntl->excp_action == EXCP_ACTION_SKIP) {
            elr +=4;
            __set_exception_return(elr);
        }
        break;
    case EC_SIMD:
        DEBUG_MSG("Adv SIMD or FP access exception - far = 0x%lx elr = 0x%lx\n",
                  far, elr);
        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->el3_excp.action == EXCP_ACTION_SKIP ||
            syscntl->excp_action == EXCP_ACTION_SKIP) {
            elr +=4;
            __set_exception_return(elr);
        }
        break;
    default:
        printf("Unhandled EL3 exception: EC = 0x%lx  ISS = 0x%lx\n", ec, iss);
        el3_shutdown();
        break;
    }

    return 0;
}

void el3_monitor_init()
{
    /* Clear out our secure and non-secure state buffers */
    memset(&sec_state, 0, sizeof(sec_state));
    memset(&nsec_state, 0, sizeof(nsec_state));

    /* Set-up the secure state buffer to return to the secure initialization
     * sequence. This will occur when we return from exception after monitor
     * initialization.
     */
    sec_state.elr_el3 = EL1_S_FLASH_BASE;
    sec_state.spsr_el3 = 0x5;
    sec_state.spsel = 0x1;
    sec_state.x[0] = (uint64_t)el3_lookup_pa(syscntl);

    /* Set-up the nonsecure state buffer to return to the non-secure
     * initialization sequence. This will occur on the first monitor context
     * switch (smc) from secure to non-secure.
     */
    nsec_state.elr_el3 = EL1_NS_FLASH_BASE;
    nsec_state.spsr_el3 = 0x5;
    nsec_state.spsel = 0x1;
    nsec_state.x[0] = (uint64_t)el3_lookup_pa(syscntl);
}

void el3_start(uint64_t base, uint64_t size)
{
    uint64_t addr = base;
    size_t len;

    printf("EL3 started...\n");

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + 0xFFF) & ~0xFFF);
         len += 0x1000, addr += 0x1000) {
        el3_unmap_va(addr);
    }

    syscntl = el3_heap_allocate(0x1000);

    smc_interop_buf = el3_heap_allocate(0x1000);
    syscntl->smc_interop.buf_va = smc_interop_buf;
    syscntl->smc_interop.buf_pa = el3_lookup_pa(smc_interop_buf);

    el3_monitor_init();

    /* Set-up our state to return to secure EL1 to start its init on exception
     * return.
     */
    monitor_restore_state(&sec_state);
    __exception_return();
}
