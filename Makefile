
ifeq ($(wildcard config.mak),)
$(error run ./configure first. See ./configure -h)
endif

include config.mak

.PHONY: arch_clean clean distclean cscope

# libcflat paths
LIBCFLAT_objdir = lib/libcflat
LIBCFLAT_srcdir = lib/libcflat
LIBCFLAT_archdir = lib/libcflat/$(ARCH)
LIBCFLAT_archive = $(LIBCFLAT_objdir)/libcflat.a
LIBCFLAT_OBJS = $(LIBCFLAT_objdir)/argv.o \
	            $(LIBCFLAT_objdir)/printf.o \
            	$(LIBCFLAT_objdir)/string.o \
                $(LIBCFLAT_objdir)/devicetree.o \
            	$(LIBCFLAT_objdir)/virtio.o \
            	$(LIBCFLAT_objdir)/virtio-testdev.o \
            	$(LIBCFLAT_objdir)/report.o \
                $(LIBCFLAT_archdir)/io.o \
            	$(LIBCFLAT_archdir)/misc.o \
            	$(LIBCFLAT_archdir)/setup.o \
            	$(LIBCFLAT_archdir)/spinlock.o \
            	$(LIBCFLAT_archdir)/processor.o

# libfdt paths
LIBFDT_objdir = lib/libfdt
LIBFDT_srcdir = lib/libfdt
LIBFDT_archive = $(LIBFDT_objdir)/libfdt.a
LIBFDT_include = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_INCLUDES))
LIBFDT_version = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_VERSION))

libeabi = $(LIBCFLAT_archdir)/libeabi.a
eabiobjs = $(LIBCFLAT_archdir)/eabi_compat.o
libgcc := $(shell $(CC) -m$(ARCH) --print-libgcc-file-name)
CFLAGS += -I$(ARCH) -DASM

FLATLIBS = $(LIBCFLAT_archive) $(LIBFDT_archive) $(libgcc) $(libeabi)

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS += -marm
CFLAGS += -mcpu=$(PROCESSOR)
CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99
CFLAGS += -ffreestanding
CFLAGS += -Wextra -Werror -Wall
CFLAGS += -g -O0
CFLAGS += -Ilib -I$(LIBFDT_srcdir) -I $(LIBCFLAT_srcdir) -I$(LIBCFLAT_archdir)
#CFLAGS += -Ilib -I/usr/arm-linux-gnueabi/include/
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

include $(LIBFDT_srcdir)/Makefile.libfdt
$(LIBFDT_archive): CFLAGS += -Wno-sign-compare
$(LIBFDT_archive): $(addprefix $(LIBFDT_objdir)/,$(LIBFDT_OBJS))
	$(AR) rcs $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

-include */.*.d */*/.*.d

$(libeabi): $(eabiobjs)
	$(AR) rcs $@ $^

libeabi_clean:
	$(RM) $(libeabi) $(eabiobjs)

clean: arch_clean libeabi_clean
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS) $(LIBCFLAT_objdir)/.*.d

libfdt_clean:
	$(RM) $(LIBFDT_archive) $(addprefix $(LIBFDT_objdir)/,$(LIBFDT_OBJS)) \
		  $(LIBFDT_objdir)/.*.d

libcflat_clean:
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS)

distclean: clean libfdt_clean libcflat_clean
	$(RM) config.mak ..d
