/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */
/* Taken from Linux vexpress support */
_ram_nsec_base = 0x48000000;
SECTIONS
{
 . = 0x00000000;
 .boot : { tzboot.o }
 _eboot = .;
 _flash_secvecs_start = .;
 . = 0x40000000;
 _ram_sec_base = .;
 /* We separate out the secure vector table and level 1 handlers, so we
     * can map it to hivecs (0xffff0000).
	 */
 _ram_secvecs_start = .;
 .vectors : AT ( _flash_secvecs_start ) { *(.vectors); }
 _ram_secvecs_end = .;
 _secvecs_size = _ram_secvecs_end - _ram_secvecs_start;
 /* Page align the text section as we map it differently than the above
     * vector text.
	 */
 . = ALIGN(0x1000);
 _flash_sectext_start = _flash_secvecs_start + _secvecs_size;
 _ram_sectext_start = .;
 .text : AT ( _flash_sectext_start ) { *(.text); }
 _ram_sectext_end = .;
 _sectext_size = _ram_sectext_end - _ram_sectext_start;
 _flash_secdata_start = _flash_sectext_start + _sectext_size;
 . = ALIGN(1M);
 _ram_secdata_start = .;
 .rodata : AT ( _flash_secdata_start ) { *(.rodata); }
 .data : { *(.data); }
 .bss : { *(.bss); }
 _ram_secdata_end = .;
 _secdata_size = _ram_secdata_end - _ram_secdata_start;
 . = ALIGN(0x4000);
 _secstack_start = .;
 sec_usr_stacktop = . + 0x4000;
 sec_und_stacktop = sec_usr_stacktop + 0x4000;
 sec_abt_stacktop = sec_und_stacktop + 0x4000;
 sec_svc_stacktop = sec_und_stacktop + 0x4000;
 boot_stacktop = sec_svc_stacktop;
 mon_stacktop = sec_svc_stacktop + 0x4000;
 _secstack_size = mon_stacktop - _secstack_start;
 . = ALIGN(1M);
 secure_memory_heap = .;
 . = ALIGN(16M);
 _sec_l1_page_table = .;
 . = 0x49000000;
 _common_memory_heap_base = .;
 . = 0x4A000000;
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
