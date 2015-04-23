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
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = EL0_S_BASE_VA;
    .text . : AT(0x0) {
        _EL0_S_TEXT_BASE = .;
        *(.text);
        _EL0_S_TEXT_SIZE = SIZEOF(.text);
    }

    . = ALIGN(4k);
    .data . : {
        _EL0_S_DATA_BASE = .;
        *(.*data);
        *(.*bss);
        _EL0_S_DATA_SIZE = SIZEOF(.data);
    }
}
