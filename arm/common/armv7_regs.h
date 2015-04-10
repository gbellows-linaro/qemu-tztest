#ifndef __ARMV7_REGS_H
#define __ARMV7_REGS_H

#define CPSR_M_USR  0x10
#define CPSR_M_FIQ  0x11
#define CPSR_M_IRQ  0x12
#define CPSR_M_SVC  0x13
#define CPSR_M_MON  0x16
#define CPSR_M_ABT  0x17
#define CPSR_M_HYP  0x1A
#define CPSR_M_UND  0x1B
#define CPSR_M_SYS  0x1F

#define SCR_NS      0x1

#define SPSR_EL0    CPSR_M_USR
#define SPSR_EL1    CPSR_M_SVC
#define SPSR_EL2    CPSR_M_HYP
#define SPSR_EL3    CPSR_M_MON

#endif
