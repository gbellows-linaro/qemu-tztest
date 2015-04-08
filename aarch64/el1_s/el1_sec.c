#include "el1_common.h"

uintptr_t EL1_S_INIT_BASE = (uintptr_t)&_EL1_S_INIT_BASE;
uintptr_t EL1_S_INIT_SIZE = (uintptr_t)&_EL1_S_INIT_SIZE;
uintptr_t EL1_S_FLASH_TEXT = (uintptr_t)&_EL1_S_FLASH_TEXT;
uintptr_t EL1_S_TEXT_BASE = (uintptr_t)&_EL1_S_TEXT_BASE;
uintptr_t EL1_S_DATA_BASE = (uintptr_t)&_EL1_S_DATA_BASE;
uintptr_t EL1_S_TEXT_SIZE = (uintptr_t)&_EL1_S_TEXT_SIZE;
uintptr_t EL1_S_DATA_SIZE = (uintptr_t)&_EL1_S_DATA_SIZE;

const char *sec_state_str = "secure";

void el1_init_el0()
{
    uintptr_t main;
    bool is_32 = false;

    is_32 = el1_load_el0(EL0_S_FLASH_BASE, &main);

    if (!is_32) {
        __exception_return(main, EL0T);
    }
}
