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
USR_STACKTOP = 0x16000000;
SYS_STACKTOP = 0x15ff0000;
SVC_STACKTOP = 0x15fe0000;
MON_STACKTOP = 0x15fd0000;

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

 /*.data : AT ( ADDR (.rodata) + SIZEOF (.rodata) ) { *(.data) }*/
 .bss : AT ( ADDR (.data) + SIZEOF (.data) ) { *(.bss) }

 . = MON_STACKTOP;
 mon_stacktop = .;

 . = SVC_STACKTOP;
 svc_stacktop = .;

 . = SYS_STACKTOP;
 sys_stacktop = .;

 . = USR_STACKTOP;
 usr_stacktop = .;
}
