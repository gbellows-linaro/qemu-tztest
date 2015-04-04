#ifndef _EL3_MONITOR_H
#define _EL3_MONITOR_H

typedef struct state_buf {
    uintptr_t sp_sys;
    uintptr_t lr_sys;
    uintptr_t spsr_irq;
    uintptr_t sp_irq;
    uintptr_t lr_irq;
    uintptr_t spsr_svc;
    uintptr_t sp_svc;
    uintptr_t lr_svc;
    uintptr_t spsr_abt;
    uintptr_t sp_abt;
    uintptr_t lr_abt;
    uintptr_t spsr_und;
    uintptr_t sp_und;
    uintptr_t lr_und;
    uintptr_t spsr_mon;
    uintptr_t lr_mon;
    uintptr_t r[9];    /* r4-r12 */
} state_buf;

extern void monitor_switch(int op, smc_op_desc_t *desc);
extern void monitor_restore_state(state_buf *);
extern void monitor_save_state(state_buf *);

#endif
