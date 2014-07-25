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
RAM_BASE = 0x80000000;
RAM_NSEC_BASE = 0x88000000;

/* Start our stack at the top of sram so that it moves toward the data section.
 */
STACK_SIZE = 0x40000;

SECTIONS
{
 . = 0;
 .boot : { arm/tzboot.o }
 _eboot = .;

 _flash_sectext_start = .;
 . = RAM_BASE;
 sec_ram_base = .;
 _ram_sectext_start = .;
 .text : AT ( _flash_sectext_start ) { *(.text); }
 _ram_sectext_end = .;
 _sectext_size = SIZEOF(.text);

 _flash_secdata_start = _flash_sectext_start + _sectext_size;
 _ram_secdata_start = .;
 .rodata : { *(.rodata); }
 .data : { *(.data); }
 .bss :  { *(.bss); }
 _ram_secdata_end =.;
 _secdata_size = _ram_secdata_end - _ram_secdata_start;

. = ALIGN(0x2000);
 sec_l1_page_table = .;

. = ALIGN(0x2000) + STACK_SIZE;
 sec_usr_stacktop = . + STACK_SIZE;
 sec_und_stacktop = sec_usr_stacktop + STACK_SIZE;
 sec_abt_stacktop = sec_und_stacktop + STACK_SIZE;
 sec_svc_stacktop = sec_und_stacktop + STACK_SIZE;
 boot_stacktop = sec_svc_stacktop;
 mon_stacktop = sec_svc_stacktop + STACK_SIZE;

 _ram_nsec_base = RAM_NSEC_BASE;
}
