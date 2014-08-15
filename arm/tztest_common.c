#include "tztest.h"

#ifdef DEBUG
void validate_state(uint32_t mode, uint32_t state)
{
    tztest_svc_desc_t desc;

    assert((_read_cpsr() & CPSR_MODE_MASK) == mode);

    CLEAR_SVC_DESC(desc);
    __svc(SVC_GET_SECURE_STATE, &desc);
    assert(desc.secure_state.state == state);
}
#else
void validate_state(__attribute__((unused)) uint32_t mode,
                    __attribute__((unused)) uint32_t state) {}
#endif

