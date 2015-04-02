BIOS_IMAGE 		= tztest.img
MON_IMAGE		= monitor/mon.bin
SEC_IMAGE		= secure/sec.bin
NSEC_IMAGE		= nonsecure/nsec.bin

CFLAGS += -I../../platform/$(PLAT) -I../../libcflat/include
CFLAGS += -I../common -I../monitor
CFLAGS += -DAARCH64

-include .*.d

##################################################################

$(BIOS_IMAGE): $(MON_IMAGE) $(SEC_IMAGE) $(NSEC_IMAGE)
	dd if=$(MON_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=65536 if=$(SEC_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=131072 if=$(NSEC_IMAGE) of=$@ 2> /dev/null
	chmod +x $(BIOS_IMAGE)

$(MON_IMAGE):
	$(MAKE) -C monitor all

$(SEC_IMAGE):
	$(MAKE) -C secure all

$(NSEC_IMAGE):
	$(MAKE) -C nonsecure all

all: $(BIOS_IMAGE)

clean:
	$(MAKE) -C monitor clean
	$(MAKE) -C secure clean
	$(MAKE) -C nonsecure clean
	$(RM) $(BIOS_IMAGE) .*.d
