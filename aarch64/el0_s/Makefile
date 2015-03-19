VPATH	= ../common

EL0_S_ELF		= tztest.elf
EL0_S_IMAGE 	= tztest.bin
EL0_S_LOAD		= tztest.lds
EL0_S_OBJS		= tztest.o \
                  builtins.o

-include .*.d

##################################################################

$(EL0_S_ELF): $(EL0_S_OBJS) $(EL0_S_LOAD)
	$(LD) -fpic -pie -o $@ $(EL0_S_OBJS) $(FLATLIBS) --script=$(EL0_S_LOAD)

$(EL0_S_IMAGE): $(EL0_S_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL0_S_LOAD): tztest.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -fpic -pie -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -mcmodel=large -c -nostdlib -o $(notdir $@) $<

all: $(EL0_S_IMAGE)

clean:
	$(RM) $(EL0_S_OBJS) $(EL0_S_LOAD) $(EL0_S_ELF) $(EL0_S_IMAGE) .*.d
