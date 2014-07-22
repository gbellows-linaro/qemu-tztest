/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)

/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */
TEXT_OFFSET  = 0x00010000;
/* SRAM is mapped from 0x14000000 to 0x16000000.  Our Data section must be
 * located here so it can be written.
 */
SRAM_OFFSET = 0x14000000;

/* Start our stack at the top of sram so that it moves toward the data section.
 */
SEC_STACK_BASE = 0x16000000;
MON_STACKTOP = SEC_STACK_BASE;
SEC_SVC_STACKTOP = MON_STACKTOP - 0x40000;
SEC_UND_STACKTOP = SEC_SVC_STACKTOP - 0x40000;
SEC_USR_STACKTOP = SEC_UND_STACKTOP - 0x40000;

NSEC_STACK_BASE = 0x15800000;
NSEC_SVC_STACKTOP = NSEC_STACK_BASE;
NSEC_UND_STACKTOP = NSEC_SVC_STACKTOP - 0x40000;
NSEC_USR_STACKTOP = NSEC_UND_STACKTOP - 0x40000;

SECTIONS
{
 . = 0;
 .boot : { arm/tzboot.o }

 . = TEXT_OFFSET;
 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : { *(.text) }
 .rodata : { *(.rodata) }
 .data : { *(.data) }

 . = SRAM_OFFSET;

 .bss : AT ( ADDR (.data) + SIZEOF (.data) ) { *(.bss) }

 . = NSEC_USR_STACKTOP;
 nsec_usr_stacktop = .;

 . = NSEC_UND_STACKTOP;
 nsec_und_stacktop = .;

 . = NSEC_SVC_STACKTOP;
 nsec_svc_stacktop = .;

 . = SEC_USR_STACKTOP;
 sec_usr_stacktop = .;

 . = SEC_UND_STACKTOP;
 sec_und_stacktop = .;

 . = SEC_SVC_STACKTOP;
 sec_svc_stacktop = .;
 boot_stacktop = sec_svc_stacktop;

 . = MON_STACKTOP;
 mon_stacktop = .;

}
