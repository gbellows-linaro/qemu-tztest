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
    . = EL1_S_FLASH_BASE;
    .init . : {
        _EL1_S_INIT_BASE = .;
        el1_init.o(.init);
        *(.init);
    }
    _EL1_S_INIT_SIZE = SIZEOF(.init);

    . = ALIGN(4K);
    _EL1_S_FLASH_TEXT = .;
    . = EL1_S_BASE_VA;
    .text . : AT(_EL1_S_FLASH_TEXT) {
        _EL1_S_TEXT_BASE = .;
        *(.text);
        *(.vectors);
    }
    _EL1_S_TEXT_SIZE = SIZEOF(.text);

    . = ALIGN(4k);
    .data . : {
        _EL1_S_DATA_BASE = .;
        _EL1_S_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _EL1_S_DATA_SIZE = SIZEOF(.data);
}
