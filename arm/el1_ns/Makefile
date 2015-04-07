VPATH			= ../el1_common:../common

NSEC_ELF		= el1_nsec.elf
NSEC_IMAGE 		= el1_nsec.bin
NSEC_LOAD		= el1_nsec.lds
NSEC_OBJS		= el1_init.o \
				  el1_exception.o \
				  el1.o \
				  el1_nsec.o \
				  el1_loader.o \
				  svc.o \
				  mem_util.o \
				  builtins.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
	NSEC_OBJS += tztest_debug.o
endif

-include .*.d

CFLAGS += -I../el0_ns/ -I../el1_common

##################################################################

$(NSEC_ELF): $(NSEC_OBJS) $(NSEC_LOAD)
	$(LD) -o $@ $(NSEC_OBJS) $(FLATLIBS) --script=$(NSEC_LOAD)

$(NSEC_IMAGE): $(NSEC_ELF)
	$(OBJCOPY) -O binary $(NSEC_ELF) $(NSEC_IMAGE)

$(NSEC_LOAD): el1_nsec.lds.S Makefile
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(NSEC_IMAGE)

clean:
	$(RM) $(NSEC_OBJS)  $(NSEC_LOAD) $(NSEC_ELF) $(NSEC_IMAGE) .*.d
