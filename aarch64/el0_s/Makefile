VPATH	= ../el0_common:../common

EL0_S_ELF		= el0_sec.elf
EL0_S_IMAGE 	= el0_sec.bin
EL0_S_LOAD		= el0_sec.lds
EL0_S_OBJS		= tztest_sec.o \
				  tztest.o \
				  el0.o \
                  builtins.o

-include .*.d

CFLAGS += -I../el0_common

##################################################################

$(EL0_S_ELF): $(EL0_S_OBJS) $(EL0_S_LOAD)
	$(LD) -o $@ $(EL0_S_OBJS) $(FLATLIBS) --script=$(EL0_S_LOAD)

$(EL0_S_IMAGE): $(EL0_S_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL0_S_LOAD): el0_sec.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $(notdir $@) $<

all: $(EL0_S_IMAGE)

clean:
	$(RM) $(EL0_S_OBJS) $(EL0_S_LOAD) $(EL0_S_ELF) $(EL0_S_IMAGE) .*.d
