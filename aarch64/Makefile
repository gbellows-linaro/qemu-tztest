SECTEST			= tztest_secure.elf
SECTESTIMAGE 	= tztest_secure.flat
TZTEST_IMAGE 	= tztest.img
TZBOOT			= tzboot.o \
                  el3_init.o \
                  monitor_asm.o \
                  monitor.o
TZBOOTELF		= tzboot.elf
TZBOOTIMAGE		= tzboot.flat
TZSECLOAD		= tztest_secure.lds
TZBOOTLOAD		= tzboot.lds
TZSECOBJS		= secure_init.o #\
			  	  secure_svc.o \
			  	  secure_asm.o \
                  sm_asm.o \
			  	  sm.o
TZOBJS			= common_mmu.o #\
                  common_svc.o

-include .*.d

##################################################################

$(TZBOOTELF): $(TZBOOT) $(TZOBJS) $(TZBOOTLOAD)
	$(LD) -o $@ $(TZBOOT) $(TZOBJS) $(FLATLIBS) --script=$(TZBOOTLOAD)

$(TZBOOTIMAGE): $(TZBOOTELF)
	$(OBJCOPY) -O binary $< $@

$(SECTEST): $(TZSECOBJS) $(TZOBJS) $(TZSECLOAD)
	$(LD) -o $@ $(TZSECOBJS) $(TZOBJS) $(FLATLIBS) --script=$(TZSECLOAD)

$(SECTESTIMAGE): $(SECTEST)
	$(OBJCOPY) -O binary $< $@

$(TZTEST_IMAGE): $(TZBOOTIMAGE) $(SECTESTIMAGE) $(NSECTESTIMAGE)
	dd if=$(TZBOOTIMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=65536 if=$(SECTESTIMAGE) of=$@ 2> /dev/null
	chmod +x $(TZTEST_IMAGE)

$(TZBOOTLOAD): tzboot.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

$(TZSECLOAD): tztest_secure.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(TZTEST_IMAGE)

clean:
	$(RM) $(TZOBJS) $(TZSECOBJS) $(TZNSECOBJS) \
		  $(TZSECLOAD) $(TZNSECLOAD) \
          $(TZBOOT) $(TZBOOTELF) $(TZBOOTIMAGE) $(TZBOOTLOAD) \
		  $(SECTEST) $(SECTESTIMAGE) $(NSECTEST) $(NSECTESTIMAGE) \
		  $(TZTEST_IMAGE) .*.d
