#ifndef _TZTEST_H
#define _TZTEST_H

typedef uint32_t (*tztest_t)();
extern tztest_t tztest[];

extern void tztest_init();
extern uint32_t P0_check_smc();
extern uint32_t P0_check_register_access();
extern uint32_t P0_check_trap_to_EL3();

typedef enum {
    TZTEST_P0_SMC = 0,
    TZTEST_REG_ACCESS,
    TZTEST_TRAP_TO_EL3,
    TZTEST_COUNT
} tztest_func_id_t;

#endif
