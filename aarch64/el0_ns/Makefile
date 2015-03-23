VPATH	= ../el0_common:../common

EL0_NS_ELF		= tztest.elf
EL0_NS_IMAGE 	= tztest.bin
EL0_NS_LOAD		= tztest.lds
EL0_NS_OBJS		= tztest_nsec.o \
				  tztest.o \
                  builtins.o

-include .*.d

CFLAGS += -I../el0_common

##################################################################

$(EL0_NS_ELF): $(EL0_NS_OBJS) $(EL0_NS_LOAD)
	$(LD) -fpic -pie -o $@ $(EL0_NS_OBJS) $(FLATLIBS) --script=$(EL0_NS_LOAD)

$(EL0_NS_IMAGE): $(EL0_NS_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL0_NS_LOAD): tztest.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -fpic -pie -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -mcmodel=large -c -nostdlib -o $(notdir $@) $<

all: $(EL0_NS_IMAGE)

clean:
	$(RM) $(EL0_NS_OBJS) $(EL0_NS_LOAD) $(EL0_NS_ELF) $(EL0_NS_IMAGE) .*.d
