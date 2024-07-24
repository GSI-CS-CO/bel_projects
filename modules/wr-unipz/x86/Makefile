# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make SYSENV=OPT PREFIX=/opt/usr all'                                               (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'                                                      (hack: leave PREFIX empty for SCU path)

# install 
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
WRUNIPZLIB  := libwrunipzlib.so

# include common Makefile stuff
include targetsysbuild.db


#TARGETS     := wrunipz-ctl
#

# the following is for c code
EXTRA_FLAGS ?=
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lwrunipzlib

$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
#$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := wrunipz-ctl

all: lib $(TARGETS)

wrunipz-ctl: wrunipz-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o wrunipz-ctl wrunipz-ctl.c $(LIBS)

lib: wrunipzlib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c wrunipzlib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(WRUNIPZLIB).1 -o $(WRUNIPZLIB).1.0 wrunipzlib.o common-lib.o -L $(EB)/.libs -letherbone
	ln -sf $(WRUNIPZLIB).1.0 $(WRUNIPZLIB).1
	ln -sf $(WRUNIPZLIB).1 $(WRUNIPZLIB)

clean:
	rm -f *.o $(TARGETS) $(WRUNIPZLIB)*

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin
	cp $(WRUNIPZLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB) 

.PHONY: all clean
