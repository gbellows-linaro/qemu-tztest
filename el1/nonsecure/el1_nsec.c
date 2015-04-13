#include "el1_common.h"
#include "register.h"

uintptr_t EL1_NS_INIT_BASE = (uintptr_t)&_EL1_NS_INIT_BASE;
uintptr_t EL1_NS_INIT_SIZE = (uintptr_t)&_EL1_NS_INIT_SIZE;
uintptr_t EL1_NS_FLASH_TEXT = (uintptr_t)&_EL1_NS_FLASH_TEXT;
uintptr_t EL1_NS_TEXT_BASE = (uintptr_t)&_EL1_NS_TEXT_BASE;
uintptr_t EL1_NS_DATA_BASE = (uintptr_t)&_EL1_NS_DATA_BASE;
uintptr_t EL1_NS_TEXT_SIZE = (uintptr_t)&_EL1_NS_TEXT_SIZE;
uintptr_t EL1_NS_DATA_SIZE = (uintptr_t)&_EL1_NS_DATA_SIZE;

const char *sec_state_str = "non-secure";

void el1_init_el0()
{
    uintptr_t main;

    el1_load_el0(EL0_NS_FLASH_BASE, &main);

    __exception_return(main, SPSR_EL0);

}
