OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
TARGET(binary)

#include "platform.h"

SECTIONS
{
    . = SEC_FLASH_BASE;
    .init . : {
        secure_init.o(.init);
    }

    . = ALIGN(16);
    _SEC_FLASH_TEXT = .;
    . = SEC_RAM_BASE;
    .text . : AT(_SEC_FLASH_TEXT) {
        _SEC_RAM_TEXT = .;
        *(.text);
        *(.vectors);
    }
    _SEC_TEXT_SIZE = SIZEOF(.text);

    .data ALIGN(0x1000) : {
        _SEC_RAM_DATA = .;
        _SEC_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _SEC_DATA_SIZE = SIZEOF(.data);
}
