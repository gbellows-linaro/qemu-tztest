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

#include "memory.h"

/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */

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

 . = ALIGN(1M);
 _flash_nsecdata_start = _flash_nsectext_start + _nsectext_size;
 _ram_nsecdata_start = .;
 .rodata : AT ( _flash_nsecdata_start ) { *(.rodata); }
 .data : { *(.data); }
 .bss :  { *(.bss); }
 _ram_nsecdata_end =.;
 _nsecdata_size = _ram_nsecdata_end - _ram_nsecdata_start;

 . = ALIGN(STACK_SIZE);
 _nsecstack_start = .;
 nsec_usr_stacktop = . + STACK_SIZE;
 nsec_und_stacktop = nsec_usr_stacktop + STACK_SIZE;
 nsec_abt_stacktop = nsec_und_stacktop + STACK_SIZE;
 nsec_svc_stacktop = nsec_und_stacktop + STACK_SIZE;
 _nsecstack_size = nsec_svc_stacktop - _nsecstack_start;
 . += 1M;

 . = ALIGN(1M);
 _nsec_l1_page_table = .;
 . += 1M;
 . = ALIGN(1M);
 _nsec_l2_page_table = .;
 . += 16M;

 . = TZTEST_COMMON_HEAP_BASE;
 _common_memory_heap_base = .;

 . = TZTEST_SHARED_HEAP_BASE;
 _shared_memory_heap_base = .;
 . += 4;
 _tztest_test_count = .;
 . += 4;
 _tztest_fail_count = .;
 . += 4;
 _tztest_exception = .;
 . += 4;
 _tztest_exception_status = .;
 . += 4;
 _tztest_exception_addr = .;
 . += 4;
}
