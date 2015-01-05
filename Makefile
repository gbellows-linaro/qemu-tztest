
ifeq ($(wildcard config.mak),)
$(error run ./configure first. See ./configure -h)
endif

include config.mak

.PHONY: arch_clean clean distclean

libgcc := $(shell $(CC) -m$(ARCH) --print-libgcc-file-name)

export FLATLIBS = libcflat/libcflat.a $(libgcc) libcflat/libeabi.a

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS += -I$(ARCH) -DASM
CFLAGS += -m${ARCH} -mcpu=$(PROCESSOR)
CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99
CFLAGS += -ffreestanding
CFLAGS += -Wextra -Werror -Wall
CFLAGS += -g -O0
CFLAGS += -Ilib -Ilibcflat/include -Ilibcflat/$(ARCH)/include
CFLAGS += -Iplatform/$(PLAT)
CFLAGS += $(call cc-option, -fomit-frame-pointer, "")
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")

export CFLAGS

#autodepend-flags = -MMD -MF $(dir $*).$(notdir $*).d
#-include */.*.d */*/.*.d

all:
	$(MAKE) -C libcflat all

clean:
	$(MAKE) -C libcflat clean

distclean: clean
	$(RM) config.mak ..d cscope.*
