#ifndef _EL3_LOADER_H
#define _EL3_LOADER_H

extern uintptr_t _EL3_RAM_TEXT;
uintptr_t EL3_RAM_TEXT = (uintptr_t)&_EL3_RAM_TEXT;
extern uintptr_t _EL3_RAM_DATA;
uintptr_t EL3_RAM_DATA = (uintptr_t)&_EL3_RAM_DATA;
extern uintptr_t _EL3_TEXT_SIZE;
uint64_t EL3_TEXT_SIZE = (uint64_t)&_EL3_TEXT_SIZE;
extern uintptr_t _EL3_DATA_SIZE;
uint64_t EL3_DATA_SIZE = (uint64_t)&_EL3_DATA_SIZE;

#endif
