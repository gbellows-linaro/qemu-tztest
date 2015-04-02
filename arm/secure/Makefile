VPATH			= ../common

SEC_ELF			= sec.elf
SEC_IMAGE 		= sec.bin
SEC_LOAD		= tztest_secure.lds
SEC_OBJS		= secure_init.o \
				  secure_exception.o \
				  secure_svc.o \
				  secure_asm.o \
				  common_mmu.o \
				  common_svc.o

ifneq (,$(findstring DEBUG, $(CFLAGS)))
	SEC_OBJS += tztest_debug.o
endif

-include .*.d

##################################################################

$(SEC_ELF): $(SEC_OBJS) $(SEC_LOAD)
	$(LD) -o $@ $(SEC_OBJS) $(FLATLIBS) --script=$(SEC_LOAD)

$(SEC_IMAGE): $(SEC_ELF)
	$(OBJCOPY) -O binary $(SEC_ELF) $(SEC_IMAGE)

$(SEC_LOAD): tztest_secure.lds.S Makefile
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(SEC_IMAGE)

clean:
	$(RM) $(SEC_OBJS) $(SEC_LOAD) $(SEC_ELF) $(SEC_IMAGE) .*.d
