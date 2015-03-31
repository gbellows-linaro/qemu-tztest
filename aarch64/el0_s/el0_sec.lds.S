OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = 0x0;
    .text . : {
        *(.text);
        _EL0_S_TEXT_BASE = EL0_S_BASE_VA;
        _EL0_S_TEXT_SIZE = SIZEOF(.text);
    }

    . = ALIGN(4k);
    .data . : {
        *(.*data);
        *(.*bss);
        _EL0_S_DATA_SIZE = SIZEOF(.data);
    }
}
