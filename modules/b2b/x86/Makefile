# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: for building against yocto, see https://www-acc.gsi.de/wiki/Frontend/Yocto#Application_Development
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        := x86_64
FW          ?=../fw
INCLUDE     ?=../include
COMPATH     := ../../common-libs
TARGETS     := b2b-ctl b2b-ui lib b2b-serv-sys b2b-client-sys b2b-serv-raw b2b-viewer b2b-analyzer b2b-archiver b2b-mon b2b-pname
B2BLIB      := libb2blib.so

# hack for building on local linux box
#DIMPATH     := /opt/dim/dim_v20r29
#EB          ?= ../../../ip_cores/etherbone-core/api
# hack for building on ASL with yocto SDK
DIMPATH      := /common/usr/timing/b2b/yocto/dim_v20r33
EB           ?= .

EXTRA_FLAGS ?=
LIBS        ?= -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib -L$(DIMPATH)/linux
CCFLAGS     ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I$(EB) -I$(FW) -I$(COMPATH)/include -I$(INCLUDE) -I$(DIMPATH)/dim
SAFTLIBS    ?= `pkg-config  saftlib --libs` -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib -L$(DIMPATH)/linux

$(info    FFLAGS       is $(CCFLAGS))

all: $(TARGETS)

b2b-ctl: b2b-ctl.c
	$(CC) $(CCFLAGS) -o b2b-ctl b2b-ctl.c $(LIBS)

b2b-ui: b2b-ui.c
	$(CC) $(CCFLAGS) -o b2b-ui b2b-ui.c ../../common-libs/x86/ivtpar.c $(LIBS)

b2b-serv-sys: b2b-serv-sys.c
	$(CC) $(CCFLAGS) -o b2b-serv-sys b2b-serv-sys.c $(LIBS) -ldim -lpthread

b2b-client-sys: b2b-client-sys.c
	$(CC) $(CCFLAGS) -o b2b-client-sys b2b-client-sys.c $(LIBS) -ldim -lpthread 

b2b-viewer: b2b-viewer.c
	$(CC) $(CCFLAGS) -o b2b-viewer b2b-viewer.c $(LIBS) -ldim -lpthread 

b2b-mon: b2b-mon.c
	$(CC) $(CCFLAGS) -o b2b-mon b2b-mon.c $(LIBS) -ldim -lpthread 

b2b-analyzer: b2b-analyzer.c
	$(CC) $(CCFLAGS) -o b2b-analyzer b2b-analyzer.c $(LIBS) -ldim -lpthread 

b2b-archiver: b2b-archiver.c
	$(CC) $(CCFLAGS) -o b2b-archiver b2b-archiver.c $(LIBS) -ldim -lpthread 

b2b-serv-raw: b2b-serv-raw.cpp
	$(CXX) $(CCFLAGS) -o b2b-serv-raw b2b-serv-raw.cpp $(SAFTLIBS) -ldim -lpthread

b2b-pname: b2b-pname-info.cpp
	$(CXX) $(CCFLAGS) -o b2b-pname-info b2b-pname-info.cpp $(SAFTLIBS) -ldim -lpthread

#saft-b2b: saft-b2b-mon.cpp, deprecated
#	$(CXX) $(CCFLAGS) -o saft-b2b-mon saft-b2b-mon.cpp $(LIBS)

lib: b2blib.c
	$(CC) $(CCFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
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
