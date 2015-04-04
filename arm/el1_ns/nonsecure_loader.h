#ifndef _NONSECURE_LOADER_H
#define _NONSECURE_LOADER_H
#include <stdint.h>

extern uint32_t _nsec_l1_page_table;
extern uint32_t _ram_nsectext_start;
extern uint32_t _ram_nsecdata_start;
extern uint32_t _nsecstack_start;
extern uint32_t _nsectext_size;
extern uint32_t _nsecdata_size;
extern uint32_t _nsecstack_size;
extern uint32_t _shared_memory_heap_base;
extern uint32_t _common_memory_heap_base;
extern volatile int _tztest_exception;
extern volatile int _tztest_exception_status;
extern volatile int _tztest_exception_addr;

#endif
