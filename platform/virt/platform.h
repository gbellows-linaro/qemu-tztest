#ifndef _PLATFORM_H
#define _PLATFORM_H

/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */
#define FLASH_BASE 0x00000000               // From QEMU virt.c
#define FLASH_SIZE 0x08000000               // From QEMU virt.c
#define EL3_FLASH_BASE FLASH_BASE
#define SEC_FLASH_BASE (FLASH_BASE+0x10000)
#define NSEC_FLASH_BASE (FLASH_BASE+0x20000)

#define RAM_BASE 0x40000000                 // From QEMU virt.c
#define RAM_SIZE (2*1024*1024*1024)         // Only 2GB needed
#define EL3_RAM_BASE RAM_BASE
#define EL3_RAM_SIZE (256*1024)
#define SEC_RAM_BASE (RAM_BASE+0x1000000)
#define SEC_RAM_SIZE ((RAM_SIZE>>1)-EL3_RAM_SIZE)
#define NSEC_RAM_BASE (RAM_BASE+0x8000000)
#define NSEC_RAM_SIZE (RAM_SIZE/2)

#define EL3_STACK_SIZE 0x40000
#define EL3_PGTBL_BASE 0x50000000
#define EL3_STACK_BASE (EL3_PGTBL_BASE-0x1000)
#define SEC_STACK_SIZE 0x40000
#define SEC_PGTBL_BASE 0x80000000
#define SEC_STACK_BASE (EL3_PGTBL_BASE-0x1000)
#define NSEC_STACK_SIZE 0x40000
#define NSEC_STACK_BASE (NSEC_RAM_BASE+NSEC_RAM_SIZE-NSEC_STACK_SIZE)

#define UART0_BASE 0x09000000

/* Taken from Linux vexpress support */
#define SYSREG_BASE 0x1c010000
#define SYSREG_CFGCTRL 0xa4
#define SYS_CFGCTRL_FUNC(n) (((n) & 0x3f) << 20)
#define SYS_CFGFUNC_RESET 5
#define SYS_CFGFUNC_SHUTDOWN 8
#define SYS_CFGCTRL_START       (1 << 31)
#define SYS_CFGCTRL_WRITE       (1 << 30)
#define SYS_SHUTDOWN    (SYS_CFGCTRL_FUNC(SYS_CFGFUNC_SHUTDOWN) | \
                         SYS_CFGCTRL_START | SYS_CFGCTRL_WRITE)
#define SYS_RESET       (SYS_CFGCTRL_FUNC(SYS_CFGFUNC_SHUTDOWN) | \
                         SYS_CFGCTRL_START | SYS_CFGCTRL_WRITE)

#define STACK_SIZE 0x4000

#define TZTEST_COMMON_HEAP_BASE 0x49000000
#define TZTEST_SHARED_HEAP_BASE 0x4A000000
#endif
