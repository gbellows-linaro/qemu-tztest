#include "platform.h"
#include "arm_builtins.h"

extern void *el1_load_el0(char *base, char *start_va);

void el1_init_el0()
{
    int (*main)(void);

    main = el1_load_el0((char *)EL0_NS_FLASH_BASE, (char *)EL0_NS_BASE_VA);

    __set_exception_return(main);
    __exception_return();
}