VPATH			= ../common

NSEC_ELF		= nsec.elf
NSEC_IMAGE 		= nsec.bin
NSEC_LOAD		= tztest_nonsecure.lds
NSEC_OBJS		= nonsecure_init.o \
				  nonsecure_svc.o \
				  tztest.o \
				  common_mmu.o \
				  common_svc.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
	NSEC_OBJS += tztest_debug.o
endif

-include .*.d

##################################################################

$(NSEC_ELF): $(NSEC_OBJS) $(NSEC_LOAD)
	$(LD) -o $@ $(NSEC_OBJS) $(FLATLIBS) --script=$(NSEC_LOAD)

$(NSEC_IMAGE): $(NSEC_ELF)
	$(OBJCOPY) -O binary $(NSEC_ELF) $(NSEC_IMAGE)

$(NSEC_LOAD): tztest_nonsecure.lds.S Makefile
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(NSEC_IMAGE)

clean:
	$(RM) $(NSEC_OBJS)  $(NSEC_LOAD) $(NSEC_ELF) $(NSEC_IMAGE) .*.d
