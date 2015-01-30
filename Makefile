ifeq ($(wildcard config.mak),)
$(error run ./configure first. See ./configure -h)
endif

include config.mak

.PHONY: arch_clean clean distclean

libgcc := $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)

export FLATLIBS = ../libcflat/libcflat.a $(libgcc) ../libcflat/$(ARCH)/libeabi.a

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

autodepend-flags = -MMD -MF $(dir $*).$(notdir $*).d

CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99 -DASM
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Wextra -Werror -Wall
CFLAGS += -g -O0
CFLAGS += -I. -I/include -I$(ARCH)/include
CFLAGS += -I../libcflat/include
CFLAGS += -I../platform/$(PLAT)
CFLAGS += $(call cc-option, -fomit-frame-pointer, "")
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")

export CFLAGS

all:
	$(MAKE) -C libcflat all
	$(MAKE) -C $(ARCH) all

clean:
	$(MAKE) -C libcflat clean
	$(MAKE) -C $(ARCH) clean

distclean: clean
	$(RM) config.mak cscope.*
