#include "el1_common.h"

uintptr_t EL1_NS_INIT_BASE = (uintptr_t)&_EL1_NS_INIT_BASE;
uintptr_t EL1_NS_INIT_SIZE = (uintptr_t)&_EL1_NS_INIT_SIZE;
uintptr_t EL1_NS_FLASH_TEXT = (uintptr_t)&_EL1_NS_FLASH_TEXT;
uintptr_t EL1_NS_TEXT_BASE = (uintptr_t)&_EL1_NS_TEXT_BASE;
uintptr_t EL1_NS_DATA_BASE = (uintptr_t)&_EL1_NS_DATA_BASE;
uintptr_t EL1_NS_TEXT_SIZE = (uint32_t)&_EL1_NS_TEXT_SIZE;
uintptr_t EL1_NS_DATA_SIZE = (uintptr_t)&_EL1_NS_DATA_SIZE;

const char *sec_state_str = "non-secure";

void el1_init_el0()
{
    int (*main)(void);

    main = el1_load_el0((char *)EL0_NS_FLASH_BASE, (char *)EL0_NS_BASE_VA);

    __set_exception_return((uint32_t)main);
    __exception_return();
}
