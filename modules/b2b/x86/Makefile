# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make MASP=NO SYSENV=OPT PREFIX= all'                                              (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'               (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
B2BLIB      := libb2blib.so

# target system environment, OPT is default
SYSENV      ?= OPT

ifeq ($(SYSENV), OPT)
USRPATH     ?= /opt/usr
ARCH        ?= /x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
CFLAGS      ?= -Wall -O2 -g
CXX         := g++ --std=c++0x
ASLLIB      := .
ASLINC      := .
endif

ifeq ($(SYSENV), YOCTO)
USRPATH     := /common/usr/timing/b2b/yocto/usr
ARCH        := /x86_64
EB          ?= .
CFLAGS      ?=
ASLLIB      := .
ASLINC      := .
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/b2b/centos7/usr
ARCH        := /x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
CFLAGS      ?= -Wall -O2 -g
CXX         := g++ --std=c++0x
ASLLIB      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib
ASLINC      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/include/saftlib
endif

EXTRA_FLAGS ?=
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lb2blib 
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(FW) -I$(ASLINC) -I$(USRPATH)/include -I../include -I../../common-libs/include
CXFLAGS     ?= `pkg-config  saftlib --cflags` $(CCFLAGS)
#CCFLAGS     ?=  $(EXTRA_FLAGS) -I$(EB) -I$(FW) -I$(ASLINC) -I$(USRPATH)/include  -I../include -I../../common-libs/include
SAFTLIBS    ?= `pkg-config  saftlib --libs` -L. -L$(ASLLIB) -L$(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -L$(USRPATH)/lib -letherbone -lm -lb2blib

$(info theirs CFLAGS is $(CFLAGS))
$(info   compiling x86 binaries for $(SYSENV))
$(info   ours CCFLAGS for c code is   $(CCFLAGS))
$(info   ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := lib b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys b2b-viewer b2b-analyzer b2b-archiver b2b-mon b2b-pname b2b-serv-raw

all: $(TARGETS)

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
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o b2b-serv-raw b2b-serv-raw.cpp $(LIBS) $(SAFTLIBS) -ldim -lpthread

b2b-pname: b2b-pname-info.cpp
	$(CXX) $(CFLAGS) $(CCFLAGS) -o b2b-pname-info b2b-pname-info.cpp $(SAFTLIBS) -ldim -lpthread

lib: b2blib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(B2BLIB).1 -o $(B2BLIB).1.0 b2blib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(B2BLIB).1.0 $(B2BLIB).1
	ln -sf $(B2BLIB).1 $(B2BLIB)

clean:
	rm -f *.o b2b-mon b2b-pname-info b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys b2b-analyzer b2b-viewer b2b-serv-raw b2b-archiver libb2blib.so*

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
