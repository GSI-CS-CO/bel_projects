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
# library, if it exists (later maybe)

# include common Makefile stuff
include targetsysbuild.db

# special
# set MASP to YES, if wrmil-serv-mon should function as status emitter to MASP (when in monitoring mode)
MASP        ?= NO              
# set PRO to YES, if wrmil-servi-mon should function as status emitter to MASP PRO(otherwise: MASP DEV)
PRO         ?= NO

# stuff below required for MASP support
ifeq ($(MASP), YES)
GENERAL_LIBPATH = -L$(PREFIX)/$(ARCH) 
GENERAL_LIBS    = -lrt -lpthread

ifeq ($(PRO), YES)
PRODUCTIVE     = -D PRODUCTIVE
endif

endif
# stuff above required for MASP support

EXTRA_FLAGS ?= $(PRODUCTIVE)
# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include 
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm 
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) $(CCFLAGS)
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(LIBS) $(GENERAL_LIBS) 

$(info PRO is $(PRO))
$(info PRODUCTIVE is $(PRODUCTIVE))
$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := sync-serv-mon sync-mon

all: $(TARGETS)

sync-serv-mon: sync-serv-mon.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c syncmonlib.c -o sync-serv-mon sync-serv-mon.cpp $(XLIBS) -ldim -lpthread

sync-mon: sync-mon.c
	$(CC) $(CFLAGS) $(CCFLAGS) ../../common-libs/x86/common-lib.c syncmonlib.c -o sync-mon sync-mon.c $(LIBS) -ldim -lpthread


#lib: wrmillib.c
#	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c wrmillib.c ../../common-libs/x86/common-lib.c $(LIBS)
#	$(CC) -shared -Wl,-soname,$(WRMILLIB).1 -o $(WRMILLIB).1.0 wrmillib.o common-lib.o -L $(EB)/.libs -letherbone -lm
#	ln -sf $(WRMILLIB).1.0 $(WRMILLIB).1
#	ln -sf $(WRMILLIB).1 $(WRMILLIB)

clean:
	rm -f *.o sync-serv-mon sync-mon

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	
#	cp $(WRMILLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
#	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1
#	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB)

.PHONY: all clean
