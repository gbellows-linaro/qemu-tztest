BIOS_IMAGE 		= tztest.img
EL1_S_ELF		= el1_sec.elf
EL1_S_IMAGE 	= el1_sec.bin
EL3_OBJS		= tzboot.o \
                  el3_init.o \
                  el3_exception.o \
                  el3.o \
                  el3_monitor_asm.o
EL3_ELF			= el3.elf
EL3_IMAGE		= el3.bin
EL3_LOAD		= el3.lds
EL1_S_LOAD		= el1_sec.lds
EL1_S_OBJS		= el1_sec_init.o #\
			  	  secure_svc.o \
			  	  secure_asm.o \
                  sm_asm.o \
			  	  sm.o
TZOBJS			= common_mmu.o #\
                  common_svc.o

-include .*.d

##################################################################

$(EL3_ELF): $(EL3_OBJS) $(EL3_LOAD)
	$(LD) -o $@ $(EL3_OBJS) $(FLATLIBS) --script=$(EL3_LOAD)

$(EL3_IMAGE): $(EL3_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL1_S_ELF): $(EL1_S_OBJS) $(TZOBJS) $(EL1_S_LOAD)
	$(LD) -o $@ $(EL1_S_OBJS) $(TZOBJS) $(FLATLIBS) --script=$(EL1_S_LOAD)

$(EL1_S_IMAGE): $(EL1_S_ELF)
	$(OBJCOPY) -O binary $< $@

$(BIOS_IMAGE): $(EL3_IMAGE) $(EL1_S_IMAGE) $(EL1_NS_IMAGE)
	dd if=$(EL3_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=65536 if=$(EL1_S_IMAGE) of=$@ 2> /dev/null
	chmod +x $(BIOS_IMAGE)

$(EL3_LOAD): el3.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

$(EL1_S_LOAD): el1_sec.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -mcmodel=large -c -nostdlib -o $@ $<

all: $(BIOS_IMAGE)

clean:
	$(RM) $(TZOBJS) $(EL1_S_OBJS) $(TZNSECOBJS) \
		  $(EL1_S_LOAD) $(TZNSECLOAD) \
          $(EL3_OBJS) $(EL3_ELF) $(EL3_IMAGE) $(EL3_LOAD) \
		  $(EL1_S_ELF) $(EL1_S_IMAGE) $(NSECTEST) $(NSECTESTIMAGE) \
		  $(BIOS_IMAGE) .*.d
