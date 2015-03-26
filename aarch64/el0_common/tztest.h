#ifndef _TZTEST_H
#define _TZTEST_H

typedef uint32_t (*tztest_t)(uint32_t el);
extern tztest_t tztest[];

extern void tztest_init();
extern uint32_t check_smc(uint32_t el);
extern uint32_t check_register_access(uint32_t el);
extern uint32_t check_cpacr_trap(uint32_t el);
extern uint32_t check_wfx_trap(uint32_t el);

typedef enum {
    TZTEST_SMC = 0,
    TZTEST_REG_ACCESS,
    TZTEST_CPACR_TRAP,
    TZTEST_WFX_TRAP,
    TZTEST_COUNT
} tztest_func_id_t;

#endif
