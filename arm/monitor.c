
#include "tztest.h"

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

