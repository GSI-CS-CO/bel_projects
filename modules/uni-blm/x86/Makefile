# PREFIX  controls where programs and libraries get installed (default: '/usr/local')
# Note: during compile (all), PREFIX must be set to the final installation pa # Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make SYSENV=OPT PREFIX=/opt/usr all'                                             (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'                                                    (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
UNIBLMLIB  := libuniblmlib.so

# include common Makefile stuff
include targetsysbuild.db

EXTRA_FLAGS ?= $(PRODUCTIVE)
# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include 
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -luniblmlib
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) -I$(ASLINC2) $(CCFLAGS)
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(LIBS)

$(info PRO is $(PRO))
$(info PRODUCTIVE is $(PRODUCTIVE))
$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := uniblm-ctl 

all: lib $(TARGETS)

uniblm-ctl: uniblm-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o uniblm-ctl uniblm-ctl.c $(LIBS)

lib: uniblmlib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c uniblmlib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(UNIBLMLIB).1 -o $(UNIBLMLIB).1.0 uniblmlib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(UNIBLMLIB).1.0 $(UNIBLMLIB).1
	ln -sf $(UNIBLMLIB).1 $(UNIBLMLIB)

clean:
	rm -f *.o uniblm-ctl libuniblmlib.so*

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(UNIBLMLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNIBLMLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNIBLMLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNIBLMLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNIBLMLIB)

.PHONY: all clean
