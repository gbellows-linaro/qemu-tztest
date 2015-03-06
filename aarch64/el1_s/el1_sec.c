#include "platform.h"
#include "arm_builtins.h"
#include <stdint.h>
#include "smc.h"

void el1_init_el0()
{
    __smc(SMC_YIELD, NULL);
}
