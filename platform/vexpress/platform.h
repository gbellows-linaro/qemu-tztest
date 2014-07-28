#ifndef _ARCH_H
#define _ARCH_H

/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */
#define FLASH_SEC_BASE 0x00000000
#define FLASH_NSEC_BASE 0x10000

#define RAM_SEC_BASE 0x80000000
#define RAM_NSEC_BASE 0x88000000

#define UART0_BASE 0x1c090000

#define STACK_SIZE 0x4000

#endif
