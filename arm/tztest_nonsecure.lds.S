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

/* SRAM is mapped from 0x14000000 to 0x16000000.  Our Data section must be
 * located here so it can be written.
 */
RAM_NSEC_BASE = 0x88000000;
FLASH_NSEC_BASE = 0x10000;

/* Start our stack at the top of sram so that it moves toward the data section.
 */
STACK_SIZE = 0x40000;

SECTIONS
{
 . = FLASH_NSEC_BASE;
 _flash_nsectext_start = .;
 . = RAM_NSEC_BASE;
 _ram_nsec_base = .;
 _ram_nsectext_start = .;
 .text : AT ( _flash_nsectext_start ) { *(.text); }
 _ram_nsectext_end = .;
 _nsectext_size = SIZEOF(.text);

 _flash_nsecdata_start = _flash_nsectext_start + _nsectext_size;
 _ram_nsecdata_start = .;
 .rodata : { *(.rodata); }
 .data : { *(.data); }
 .bss :  { *(.bss); }
 _ram_nsecdata_end =.;
 _nsecdata_size = _ram_nsecdata_end - _ram_nsecdata_start;

. = ALIGN(0x2000);
 nsec_l1_page_table = .;

. = ALIGN(0x2000) + STACK_SIZE;
 nsec_usr_stacktop = .;
 nsec_und_stacktop = nsec_usr_stacktop + STACK_SIZE;
 nsec_abt_stacktop = nsec_und_stacktop + STACK_SIZE;
 nsec_svc_stacktop = nsec_und_stacktop + STACK_SIZE;
}
