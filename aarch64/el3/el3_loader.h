#ifndef _EL3_LOADER_H
#define _EL3_LOADER_H

extern uintptr_t _EL3_TEXT_BASE;
uintptr_t EL3_TEXT_BASE = (uintptr_t)&_EL3_TEXT_BASE;
extern uintptr_t _EL3_DATA_BASE;
uintptr_t EL3_DATA_BASE = (uintptr_t)&_EL3_DATA_BASE;
extern uintptr_t _EL3_TEXT_SIZE;
uint64_t EL3_TEXT_SIZE = (uint64_t)&_EL3_TEXT_SIZE;
extern uintptr_t _EL3_DATA_SIZE;
uint64_t EL3_DATA_SIZE = (uint64_t)&_EL3_DATA_SIZE;

#endif
