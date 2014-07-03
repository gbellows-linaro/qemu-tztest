# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.

TZBOOT		= tzboot.S
OBJS 		= tzboot.o tztest.o string.o semihosting.o

TEST		= tztest
TESTIMAGE	= tztest.img
LD_SCRIPT	= tztest.lds.S

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
CP		= $(CROSS_COMPILE)objcopy
CPPFLAGS  = -march=armv7-a -marm -g -O0 -DVEXPRESS 

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
all: $(TEST) 

clean distclean:
	rm -f $(TEST) $(TESTIMAGE) \
	tztest.lds $(OBJS)

$(TEST): $(OBJS) tztest.lds
	$(LD) -o $@ $(OBJS) --script=tztest.lds
	$(CP) -O binary $(TEST) $(TESTIMAGE) 

boot.o: $(TZBOOT)
	$(CC) $(CPPFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -O2 -I. -c -o $@ $<

tztest.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

force: ;

Makefile: ;

.PHONY: all clean distclean
