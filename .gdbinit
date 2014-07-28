set arch arm
target remote :1234
file arm/tztest_secure.elf
add-symbol-file arm/tztest_nonsecure.elf &_ram_nsec_base
