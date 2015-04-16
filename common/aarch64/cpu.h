#ifndef _CPU_H
#define _CPU_H

#define PSTATE_EL_EL0    0x00
#define PSTATE_EL_EL1H   0x04
#define PSTATE_EL_EL1T   0x05
#define PSTATE_EL_EL2H   0x08
#define PSTATE_EL_EL2T   0x09
#define PSTATE_EL_EL3H   0x0C
#define PSTATE_EL_EL3T   0x0D

#define SPSR_EL0    PSTATE_EL_EL0
#define SPSR_EL1    PSTATE_EL_EL1T
#define SPSR_EL2    PSTATE_EL_EL2T
#define SPSR_EL3    PSTATE_EL_EL3T

#endif
