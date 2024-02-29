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
WRMILLIB    := libwrmillib.so

# include common Makefile stuff
include targetsysbuild.db

EXTRA_FLAGS ?=
# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lwrmillib 
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) $(CCFLAGS)
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(LIBS)

$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := wrmil-ctl wrmil-serv-mon wrmil-client-mon

all: lib $(TARGETS)

wrmil-ctl: wrmil-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o wrmil-ctl wrmil-ctl.c $(LIBS)

wrmil-client-mon: wrmil-client-mon.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o wrmil-client-mon wrmil-client-mon.c $(LIBS) -ldim -lpthread 

wrmil-serv-mon: wrmil-serv-mon.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o wrmil-serv-mon wrmil-serv-mon.cpp $(XLIBS) -ldim -lpthread

lib: wrmillib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c wrmillib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(WRMILLIB).1 -o $(WRMILLIB).1.0 wrmillib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(WRMILLIB).1.0 $(WRMILLIB).1
	ln -sf $(WRMILLIB).1 $(WRMILLIB)

clean:
	rm -f *.o wrmil-ctl libwrmillib.so*

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(WRMILLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRMILLIB)

.PHONY: all clean
