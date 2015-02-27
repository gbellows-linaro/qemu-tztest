#ifndef _EL1_S_LOADER_H
#define _EL1_S_LOADER_H

extern uintptr_t _EL1_S_TEXT_BASE;
uintptr_t EL1_S_TEXT_BASE = (uintptr_t)&_EL1_S_TEXT_BASE;
extern uintptr_t _EL1_S_DATA_BASE;
uintptr_t EL1_S_DATA_BASE = (uintptr_t)&_EL1_S_DATA_BASE;
extern uintptr_t _EL1_S_TEXT_SIZE;
uint64_t EL1_S_TEXT_SIZE = (uint64_t)&_EL1_S_TEXT_SIZE;
extern uintptr_t _EL1_S_DATA_SIZE;
uint64_t EL1_S_DATA_SIZE = (uint64_t)&_EL1_S_DATA_SIZE;

#endif
