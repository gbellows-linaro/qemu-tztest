set arch arm
target remote :1234
set print pretty

define load_el3
file arm/monitor/mon.elf
add-symbol-file arm/monitor/mon.elf &_EL3_TEXT_BASE
end

define load_el1s
file arm/secure/sec.elf
add-symbol-file arm/secure/sec.elf &_EL1_S_TEXT_BASE
end
