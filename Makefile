
ifeq ($(wildcard config.mak),)
$(error run ./configure first. See ./configure -h)
endif

include config.mak

.PHONY: arch_clean clean distclean

# libcflat paths
LIBCFLAT_objdir = libcflat
LIBCFLAT_srcdir = libcflat
LIBCFLAT_archdir = libcflat/$(ARCH)
LIBCFLAT_archive = $(LIBCFLAT_objdir)/libcflat.a
LIBCFLAT_OBJS = $(LIBCFLAT_objdir)/printf.o \
            	$(LIBCFLAT_objdir)/string.o \
                $(LIBCFLAT_archdir)/io.o \
            	$(LIBCFLAT_archdir)/misc.o
LIBEABI_OBJS = $(LIBCFLAT_archdir)/eabi_compat.o

LIBEABI = $(LIBCFLAT_archdir)/libeabi.a
libgcc := $(shell $(CC) -m$(ARCH) --print-libgcc-file-name)
CFLAGS += -I$(ARCH) -DASM

FLATLIBS = $(LIBCFLAT_archive) $(libgcc) $(LIBEABI)

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS += -m${ARCH} -mcpu=$(PROCESSOR)
CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99
CFLAGS += -ffreestanding
CFLAGS += -Wextra -Werror -Wall
CFLAGS += -g -O0
CFLAGS += -Ilib -I $(LIBCFLAT_srcdir)/include -I$(LIBCFLAT_archdir)/include
CFLAGS += -Iplatform/$(PLAT)
CFLAGS += $(call cc-option, -fomit-frame-pointer, "")
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS)

autodepend-flags = -MMD -MF $(dir $*).$(notdir $*).d

#include architecure specific make rules
include $(ARCH)/Makefile.tztest

$(LIBCFLAT_archive): $(LIBCFLAT_OBJS)
	$(AR) rcs $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

-include */.*.d */*/.*.d

$(LIBEABI): $(LIBEABI_OBJS)
	$(AR) rcs $@ $^

libeabi_clean:
	$(RM) $(LIBEABI) $(LIBEABI_OBJS)

clean: arch_clean libeabi_clean
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS) $(LIBCFLAT_objdir)/.*.d

libcflat_clean:
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS)

distclean: clean libcflat_clean
	$(RM) config.mak ..d cscope.*
