# PREFIX  controls where programs and libraries get installed (default: '/usr/local')
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make SYSENV=OPT PREFIX=/opt/usr all'                                             (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'                                                    (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
B2BLIB      := libb2blib.so

# include common Makefile stuff
include targetsysbuild.db

EXTRA_FLAGS ?=
# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lb2blib 
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) $(CCFLAGS)
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(LIBS)

$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys b2b-viewer b2b-analyzer b2b-archiver b2b-mon b2b-pname-info b2b-serv-raw b2b-jitter-check b2b-serv-kickdiag b2b-sim ecpu-mon

all: lib $(TARGETS)

b2b-ctl: b2b-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-ctl b2b-ctl.c $(LIBS)

b2b-ui: b2b-ui.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-ui b2b-ui.c ../../common-libs/x86/ivtpar.c $(LIBS)

b2b-serv-sys: b2b-serv-sys.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-serv-sys b2b-serv-sys.c $(LIBS) -ldim -lpthread

b2b-client-sys: b2b-client-sys.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-client-sys b2b-client-sys.c $(LIBS) -ldim -lpthread 

b2b-viewer: b2b-viewer.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-viewer b2b-viewer.c $(LIBS) -ldim -lpthread 

b2b-mon: b2b-mon.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-mon b2b-mon.c $(LIBS) -ldim -lpthread 

b2b-analyzer: b2b-analyzer.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-analyzer b2b-analyzer.c $(LIBS) -ldim -lpthread 

b2b-archiver: b2b-archiver.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-archiver b2b-archiver.c $(LIBS) -ldim -lpthread 

b2b-serv-raw: b2b-serv-raw.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o b2b-serv-raw b2b-serv-raw.cpp $(XLIBS) -ldim -lpthread

b2b-serv-kickdiag: b2b-serv-kickdiag.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o b2b-serv-kickdiag b2b-serv-kickdiag.cpp $(XLIBS) -ldim -lpthread

b2b-sim: b2b-sim.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o b2b-sim b2b-sim.c $(LIBS)	

b2b-pname-info: b2b-pname-info.cpp
	$(CXX) $(CFLAGS) $(CCFLAGS) -o b2b-pname-info b2b-pname-info.cpp $(XLIBS) -ldim -lpthread

b2b-jitter-check: b2b-jitter-check.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o b2b-jitter-check b2b-jitter-check.cpp $(XLIBS) -ldim -lpthread

ecpu-mon: ecpu-mon.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ecpu-mon ecpu-mon.c ../../common-libs/x86/common-lib.c  $(LIBS) -ldim -lpthread 


lib: b2blib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(B2BLIB).1 -o $(B2BLIB).1.0 b2blib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(B2BLIB).1.0 $(B2BLIB).1
	ln -sf $(B2BLIB).1 $(B2BLIB)

clean:
	rm -f *.o b2b-mon b2b-pname-info b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys b2b-analyzer b2b-viewer b2b-serv-raw b2b-archiver b2b-jitter-check b2b-serv-kickdiag b2b-sim libb2blib.so*

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
