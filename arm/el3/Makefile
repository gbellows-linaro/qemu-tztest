VPATH			= ../common

EL3_ELF			= el3.elf
EL3_IMAGE 		= el3.bin
EL3_LOAD		= el3.lds
EL3_OBJS		= monitor_init.o \
				  monitor_exception.o \
				  el3.o \
				  mem_util.o \
				  el3_monitor_asm.o \
				  builtins.o \
				  common_mmu.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
	EL3_OBJS += tztest_debug.o
endif

-include .*.d

##################################################################

$(EL3_ELF): $(EL3_OBJS) $(EL3_LOAD)
	$(LD) -o $@ $(EL3_OBJS) $(FLATLIBS) --script=$(EL3_LOAD)

$(EL3_IMAGE): $(EL3_ELF)
	$(OBJCOPY) -O binary $(EL3_ELF) $(EL3_IMAGE)

$(EL3_LOAD): el3.lds.S Makefile
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(EL3_IMAGE)

clean:
	$(RM) $(EL3_OBJS) $(EL3_LOAD) $(EL3_ELF) $(EL3_IMAGE) .*.d
