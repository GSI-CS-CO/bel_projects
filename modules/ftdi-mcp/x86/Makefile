# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make clean'
#   'make SYSENV=OPT DIM=YES PREFIX=/opt/usr all'                             (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'


# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths

# include common Makefile stuff
include ../../ftdi-mcp/x86/targetsysbuild.db

ifeq ($(DIM), YES)
# DIM headers should be installed in USRPATH/include
# DIM library should be installed in USRPATH/lib
USEDIM      = -D USEDIM
DIMLIB      := -ldim
endif

EXTRA_FLAGS ?= $(USEDIM)
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I../include -I$(USRPATH)/include -I../../common-libs/include
LIBS        ?= -L. -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -lm -lftd2xx -lmpsse $(DIMLIB) -lpthread -lb2blib

$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))

TARGETS     := ftdimcp-ctl ftdimcp-ui

all: $(TARGETS)

ftdimcp-ctl: ftdimcp-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ftdimcp-ctl ftdimcp-ctl.c ftdimcp-lib.c $(LIBS)

ftdimcp-ui: ftdimcp-ui.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ftdimcp-ui ftdimcp-ui.c ftdimcp-lib.c $(LIBS)

clean:
	rm -f *.o $(TARGETS)

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	

.PHONY: all clean
