VPATH			= ../common

MON_ELF			= mon.elf
MON_IMAGE 		= mon.bin
MON_LOAD		= mon.lds
MON_OBJS		= monitor_init.o \
				  monitor_exception.o \
				  monitor.o \
				  el3.o \
				  mem_util.o \
				  sm.o \
				  sm_asm.o \
				  monitor_asm.o \
				  common_mmu.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
	MON_OBJS += tztest_debug.o
endif

-include .*.d

##################################################################

$(MON_ELF): $(MON_OBJS) $(MON_LOAD)
	$(LD) -o $@ $(MON_OBJS) $(FLATLIBS) --script=$(MON_LOAD)

$(MON_IMAGE): $(MON_ELF)
	$(OBJCOPY) -O binary $(MON_ELF) $(MON_IMAGE)

$(MON_LOAD): mon.lds.S Makefile
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(MON_IMAGE)

clean:
	$(RM) $(MON_OBJS) $(MON_LOAD) $(MON_ELF) $(MON_IMAGE) .*.d
