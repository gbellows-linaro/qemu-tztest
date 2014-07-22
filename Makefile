
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
LIBCFLAT_OBJS = \
	$(LIBCFLAT_objdir)/argv.o \
	$(LIBCFLAT_objdir)/printf.o \
	$(LIBCFLAT_objdir)/string.o \
	$(LIBCFLAT_objdir)/report.o

# libfdt paths
LIBFDT_objdir = lib/libfdt
LIBFDT_srcdir = lib/libfdt
LIBFDT_archive = $(LIBFDT_objdir)/libfdt.a
LIBFDT_include = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_INCLUDES))
LIBFDT_version = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_VERSION))

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS += -marm
CFLAGS += -mcpu=$(PROCESSOR)
CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99
CFLAGS += -ffreestanding
CFLAGS += -Wextra
CFLAGS += -g -O0
CFLAGS += -Ilib -I$(LIBFDT_srcdir) -I $(LIBCFLAT_srcdir) -I$(LIBCFLAT_archdir)
CFLAGS += -Ilib -I/usr/arm-linux-gnueabi/include/
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

clean: arch_clean
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS) $(LIBCFLAT_objdir)/.*.d

libfdt_clean:
	$(RM) $(LIBFDT_archive) $(addprefix $(LIBFDT_objdir)/,$(LIBFDT_OBJS)) \
		  $(LIBFDT_objdir)/.*.d

libcflat_clean:
	$(RM) $(LIBCFLAT_archive) $(LIBCFLAT_OBJS)

distclean: clean libfdt_clean libcflat_clean
	$(RM) config.mak ..d
