BIOS_IMAGE 		= tztest.img
EL3_IMAGE		= el3/el3.bin
EL1_S_IMAGE		= el1_s/el1_sec.bin
EL1_NS_IMAGE	= el1_ns/el1_nsec.bin
EL0_NS_IMAGE 	= el0_ns/el0_nsec.elf
EL0_S_IMAGE 	= el0_s/el0_sec.elf

CFLAGS += -I../../platform/$(PLAT) -I../../libcflat/include
CFLAGS += -I../common -I../monitor
CFLAGS += -DAARCH32

-include .*.d

##################################################################

$(BIOS_IMAGE): $(EL3_IMAGE) $(EL1_S_IMAGE) $(EL1_NS_IMAGE) \
			   $(EL0_S_IMAGE) $(EL0_NS_IMAGE)
	dd if=$(EL3_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=65536 if=$(EL1_S_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=131072 if=$(EL1_NS_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=196608 if=$(EL0_NS_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=327680 if=$(EL0_S_IMAGE) of=$@ 2> /dev/null
	chmod +x $(BIOS_IMAGE)

$(EL3_IMAGE):
	$(MAKE) -C el3 all

$(EL1_S_IMAGE):
	$(MAKE) -C el1_s all

$(EL1_NS_IMAGE):
	$(MAKE) -C el1_ns all

$(EL0_NS_IMAGE):
	$(MAKE) -C el0_ns all

$(EL0_S_IMAGE):
	$(MAKE) -C el0_s all

all: $(BIOS_IMAGE)

clean:
	$(MAKE) -C el3 clean
	$(MAKE) -C el1_s clean
	$(MAKE) -C el1_ns clean
	$(MAKE) -C el0_s clean
	$(MAKE) -C el0_ns clean
	$(RM) $(BIOS_IMAGE) .*.d
