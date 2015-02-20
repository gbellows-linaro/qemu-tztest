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

#include "platform.h"

SECTIONS
{
    . = EL3_FLASH_BASE;
    .init . : {
        tzboot.o(.init);
    }

    . = ALIGN(16);
    _EL3_FLASH_TEXT = .;
    . = EL3_RAM_BASE;
    .text . : AT(_EL3_FLASH_TEXT) {
        _EL3_RAM_TEXT = .;
        *(.text);
        *(.vectors);
    }
    _EL3_TEXT_SIZE = SIZEOF(.text);

    . = ALIGN(0x1000);
    .data . : {
        _EL3_RAM_DATA = .;
        _EL3_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _EL3_DATA_SIZE = SIZEOF(.data);
}
