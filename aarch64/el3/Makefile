VPATH	= ../common

EL3_OBJS		= el3_init.o \
                  el3_exception.o \
                  el3.o \
                  el3_monitor_asm.o \
                  builtins.o
EL3_ELF			= el3.elf
EL3_IMAGE		= el3.bin
EL3_LOAD		= el3.lds

-include .*.d

##################################################################

$(EL3_ELF): $(EL3_OBJS) $(EL3_LOAD)
	$(LD) -o $@ $(EL3_OBJS) $(FLATLIBS) --script=$(EL3_LOAD)

$(EL3_IMAGE): $(EL3_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL1_S_ELF): $(EL1_S_OBJS) $(EL1_S_LOAD)
	$(LD) -o $@ $(EL1_S_OBJS) $(FLATLIBS) --script=$(EL1_S_LOAD)

$(EL3_LOAD): el3.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -mcmodel=large -c -nostdlib -o $@ $<

all: $(EL3_IMAGE)

clean:
	$(RM) $(EL3_OBJS) $(EL3_ELF) $(EL3_IMAGE) $(EL3_LOAD) .*.d
