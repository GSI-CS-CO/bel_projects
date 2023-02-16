# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# If using the Yocto SDK, you must additionally use YOCTO=YES
# Example usage:
#   'make MASP=NO YOCTO=YES PREFIX= all'                                               (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'               (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
B2BLIB      := libb2blib.so

# support Yocto SDK
ifeq ($(YOCTO), YES)
EB          ?= .
ARCH        := /x86_64
CFLAGS      ?=
DIMPATH     := /common/usr/timing/b2b/yocto/dim_v20r33
else
EB          ?= ../../../ip_cores/etherbone-core/api
ARCH        ?= /x86_64
CFLAGS      ?= -Wall -O2 -g
DIMPATH     := /opt/dim/dim_v20r29
endif

EXTRA_FLAGS ?=
LIBS        ?= -L. -L$(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lb2blib -L$(DIMPATH)/linux
CCFLAGS     ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -I$(EB) -I$(FW) -I../include -I../../common-libs/include  -I$(DIMPATH)/dim
SAFTLIBS    ?= `pkg-config  saftlib --libs` -L. -L$(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lb2blib -L$(DIMPATH)/linux

$(info    CCFLAGS is $(CCFLAGS))
$(info     CFLAGS is $(CFLAGS))

TARGETS     := b2b-ctl b2b-ui lib b2b-serv-sys b2b-client-sys b2b-serv-raw b2b-viewer b2b-analyzer b2b-archiver b2b-mon b2b-pname

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
	$(CXX) $(CFLAGS) $(CCFLAGS) ../../common-libs/x86/common-lib.c -o b2b-serv-raw b2b-serv-raw.cpp $(LIBS) $(SAFTLIBS) -ldim -lpthread

b2b-pname: b2b-pname-info.cpp
	$(CXX) $(CFLAGS) $(CCFLAGS) -o b2b-pname-info b2b-pname-info.cpp $(SAFTLIBS) -ldim -lpthread

lib: b2blib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(B2BLIB).1 -o $(B2BLIB).1.0 b2blib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(B2BLIB).1.0 $(B2BLIB).1
	ln -sf $(B2BLIB).1 $(B2BLIB)

clean:
	rm -f *.o b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys b2b-analyzer b2b-viewer b2b-serv-raw b2b-archiver libb2bllib.so*

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
