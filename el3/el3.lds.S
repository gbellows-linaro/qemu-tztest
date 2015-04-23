/*
 * Copyright (c) 2015 Linaro Limited
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

OUTPUT_FORMAT(FORMAT)
OUTPUT_ARCH(ARCH)
TARGET(binary)

#include "memory.h"

SECTIONS
{
    . = EL3_FLASH_BASE;
    .init . : {
        _EL3_INIT_BASE = .;
        el3_init.o(.init);
        *(.init);
    }
    _EL3_INIT_SIZE = SIZEOF(.init);

    . = ALIGN(4K);
    _EL3_FLASH_TEXT = .;
    . = EL3_BASE_VA;
    .text . : AT(_EL3_FLASH_TEXT) {
        _EL3_TEXT_BASE = .;
        *(.text);
        *(.vectors);
    }
    _EL3_TEXT_SIZE = SIZEOF(.text);

    . = ALIGN(4k);
    .data . : {
        _EL3_DATA_BASE = .;
        _EL3_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _EL3_DATA_SIZE = SIZEOF(.data);
}
