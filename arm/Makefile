SECTEST			= tztest_secure.elf
NSECTEST		= tztest_nonsecure.elf
SECTESTIMAGE 	= tztest_secure.flat
NSECTESTIMAGE 	= tztest_nonsecure.flat
TZTEST_IMAGE 	= tztest.img
TZBOOT			= tzboot.o
TZSECLOAD		= tztest_secure.lds
TZNSECLOAD		= tztest_nonsecure.lds
TZSECOBJS		= secure_init.o \
			  	  secure_svc.o \
			  	  secure_asm.o \
                  monitor.o \
                  monitor_asm.o \
                  sm_asm.o \
			  	  sm.o
TZNSECOBJS		= nonsecure_init.o \
			  	  nonsecure_svc.o \
			  	  tztest.o
TZOBJS			= common_mmu.o \
                  common_svc.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
    TZNSECOBJS += tztest_debug.o
endif

-include .*.d

##################################################################

$(SECTEST): $(TZSECOBJS) $(TZOBJS) $(TZSECLOAD) $(TZNSECLOAD) $(TZBOOT)
	$(LD) -o $@ $(TZSECOBJS) $(FLATLIBS) $(TZOBJS) --script=$(TZSECLOAD)

$(SECTESTIMAGE): $(SECTEST)
	$(OBJCOPY) -O binary $(SECTEST) $(SECTESTIMAGE)

$(NSECTEST): $(TZNSECOBJS) $(TZOBJS) $(TZNSECLOAD) $(TZSECLOAD)
	$(LD) -o $@ $(TZNSECOBJS) $(FLATLIBS) $(TZOBJS) --script=$(TZNSECLOAD)

$(NSECTESTIMAGE): $(NSECTEST)
	$(OBJCOPY) -O binary $(NSECTEST) $(NSECTESTIMAGE)

$(TZTEST_IMAGE): $(SECTESTIMAGE) $(NSECTESTIMAGE)
	dd if=$(SECTESTIMAGE) of=$@ 2> /dev/null
	dd if=$(NSECTESTIMAGE) of=$@ seek=65536 oflag=seek_bytes 2> /dev/null
	chmod +x $(TZTEST_IMAGE)

$(TZBOOT): tzboot.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(TZSECLOAD): tztest_secure.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

$(TZNSECLOAD): tztest_nonsecure.lds.S Makefile ../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(TZTEST_IMAGE)

clean:
	$(RM) $(TZOBJS) $(TZSECOBJS) $(TZNSECOBJS) \
		  $(TZSECLOAD) $(TZNSECLOAD) $(TZBOOT) \
		  $(SECTEST) $(SECTESTIMAGE) $(NSECTEST) $(NSECTESTIMAGE) \
		  $(TZTEST_IMAGE) .*.d
