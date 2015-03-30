#ifndef _SECURE_LOADER_H
#define _SECURE_LOADER_H
#include "sm.h"

extern int _ram_nsec_base;
extern uint32_t _sec_l1_page_table;
extern uint32_t _ram_secvecs_start;
extern uint32_t _ram_sectext_start;
extern uint32_t _ram_secdata_start;
extern uint32_t _secstack_start;
extern uint32_t _secvecs_size;
extern uint32_t _sectext_size;
extern uint32_t _secdata_size;
extern uint32_t _secstack_size;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile uint32_t _tztest_exception;
extern volatile uint32_t _tztest_exception_addr;
extern volatile uint32_t _tztest_exception_status;

#endif
