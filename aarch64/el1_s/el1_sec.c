#include "el1_common.h"

uintptr_t EL1_S_INIT_BASE = (uintptr_t)&_EL1_S_INIT_BASE;
uintptr_t EL1_S_INIT_SIZE = (uintptr_t)&_EL1_S_INIT_SIZE;
uintptr_t EL1_S_FLASH_TEXT = (uintptr_t)&_EL1_S_FLASH_TEXT;
uintptr_t EL1_S_TEXT_BASE = (uintptr_t)&_EL1_S_TEXT_BASE;
uintptr_t EL1_S_DATA_BASE = (uintptr_t)&_EL1_S_DATA_BASE;
uintptr_t EL1_S_TEXT_SIZE = (uint64_t)&_EL1_S_TEXT_SIZE;
uintptr_t EL1_S_DATA_SIZE = (uint64_t)&_EL1_S_DATA_SIZE;

void el1_init_el0()
{
    int (*main)(void);

    main = el1_load_el0((char *)EL0_S_FLASH_BASE, (char *)EL0_NS_BASE_VA);

    __set_exception_return((uint64_t)main);
    __exception_return();
}
