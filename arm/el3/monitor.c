#include "arm32.h"
#include "common_svc.h"
#include "common_defs.h"

void monitor_init_ns_entry(uint32_t entry_point)
{
    struct sm_nsec_ctx *nsec_ctx;


    /* Set-up the non-secure context so that the switch to nonsecure state
     * resumes at initiallizing the nonsecure svc mode.
     */
    nsec_ctx = sm_get_nsec_ctx();
    nsec_ctx->mon_lr = entry_point;
    nsec_ctx->mon_spsr = CPSR_MODE_SVC | CPSR_I;
}


void monitor_dispatch(tztest_smc_desc_t *desc)
{
    uint32_t (*func)(uint32_t) = desc->dispatch.func;
    DEBUG_MSG("Entered\n");
    desc->dispatch.ret = func(desc->dispatch.arg);
    DEBUG_MSG("Exiting\n");
}
