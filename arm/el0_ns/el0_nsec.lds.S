OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = 0x0;
    .text . : {
        *(.text);
        _EL0_NS_TEXT_BASE = EL0_NS_BASE_VA;
        _EL0_NS_TEXT_SIZE = SIZEOF(.text);
    }

    . = ALIGN(4k);
    .data . : {
        *(.*data);
        *(.*bss);
        _EL0_NS_DATA_SIZE = SIZEOF(.data);
    }
}
