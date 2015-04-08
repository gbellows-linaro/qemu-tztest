OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = EL0_S_BASE_VA;
    .text . : {
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
