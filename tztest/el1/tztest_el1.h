#ifndef _TZTEST_EL1_H
#define _TZTEST_EL1_H

uint32_t el1_check_cpacr_trap(uint32_t __attribute__((unused))arg);
uint32_t el1_check_wfx_trap(uint32_t __attribute__((unused))arg);
uint32_t el1_check_register_access(uint32_t __attribute__((unused))arg);

#endif
