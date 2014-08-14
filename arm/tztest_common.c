#include "tztest.h"
#include "tztest_builtins.h"
#include "libcflat.h"

int tztest_get_saved_cpsr()
{
    int cpsr = 0;

    /* Save the callers CPSR and LR to the stack so we cat fetch it for
     * return.
     */
    __srsdb_svc(CPSR_MODE_SVC);

    /* SRS saves both the LR and CPSR so we have to pop two values off the
     * stack.  We are only interested in the latter so we discard (overwrite)
     * the first read value.
     */
    __pop(cpsr);
    __pop(cpsr);

    return cpsr;
}

int tztest_read_register(tztest_reg_t reg)
{
    int val = 0;

    switch (reg) {
        case TZTEST_REG_CPSR:
            /* Assuming we are coming from some other mode, call to get the
             * saved CPSR to return.
             */
            val = tztest_get_saved_cpsr();
            break;
    }

    return val;
}

#ifdef DEBUG
void validate_state(uint32_t mode, uint32_t state)
{
    tztest_svc_desc_t desc;

    assert((_read_cpsr() & CPSR_MODE_MASK) == mode);

    CLEAR_SVC_DESC(desc);
    __svc(SVC_CHECK_SECURE_STATE, &desc);
    assert(desc.secure_state.state == state);
}
#else
void validate_state(uint32_t mode, uint32_t state) {}
#endif

