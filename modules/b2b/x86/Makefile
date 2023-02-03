# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
#EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/fw
INCLUDE     ?=../include
TARGETS     := b2b-ctl b2b-ui lib b2b-serv-sys b2b-client-sys b2b-serv-raw b2b-viewer b2b-analyzer b2b-archiver b2b-mon b2b-pname
B2BLIB      := libb2blib.so

#quick hack fix for buildingn on ASL

#DIMPATH     := /opt/dim/dim_v20r29
#DIMPATH     := /common/usr/timing/b2b/dim_v20r29
DIMPATH      := /home/bel/dbeck/lnx/dim_v20r33_yocto
COMPATH      := /home/bel/dbeck/lnx/GITSCC/b2byocto/modules/common-libs
#ASLLIB      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/lib
#ASLINC      := /common/export/timing-rte/tg-fallout-v6.0.1/x86_64/include/saftlib

EXTRA_FLAGS ?=
#CCFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/fw -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/common-libs/include -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/include -I$(DIMPATH)/dim 
LIBS        ?= -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib -L$(DIMPATH)/linux
#CCFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I $(ASLINC) -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/common-libs/include -I $(DIMPATH)/dim
CCFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/fw -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/common-libs/include -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/include -I$(DIMPATH)/dim
#CCFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/fw -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/common-libs/include -I/home/bel/dbeck/lnx/GITSCC/b2byocto/modules/b2b/include -I$(DIMPATH)/dim
#LIBS        ?= `pkg-config  saftlib --libs` -L . -L $(ASLLIB) -L $(EB)/.libs -L $(DIMPATH)/linux -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib
#LIBS        ?= `pkg-config  saftlib --libs` -L . -L $(ASLLIB) -L $(DIMPATH)/linux -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib
SAFTLIBS     ?= `pkg-config  saftlib --libs` -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib -L$(DIMPATH)/linux

$(info    CFFLAGS       is $(CCFLAGS))

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
#	$(CPP) --std=c++0x $(CCFLAGS) b2b-serv-raw.cpp -o b2b-serv-raw.o#
#	$(CPP) -L$(DIMPATH)/linux -lm -letherbone -lb2blib b2b-serv-raw.o -o b2b-serv-raw

b2b-pname: b2b-pname-info.cpp
	$(CXX) $(CCFLAGS) -o b2b-pname-info b2b-pname-info.cpp $(SAFTLIBS) -ldim -lpthread

saft-b2b: saft-b2b-mon.cpp
	$(CXX) $(CCFLAGS) -o saft-b2b-mon saft-b2b-mon.cpp $(LIBS)

lib: b2blib.c
	$(CC) $(CCFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(B2BLIB).1 -o $(B2BLIB).1.0 b2blib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(B2BLIB).1.0 $(B2BLIB).1
	ln -sf $(B2BLIB).1 $(B2BLIB)

clean:
	rm -f *.o b2b-ctl b2b-ui b2b-serv-sys b2b-client-sys saft-b2b-mon b2b-analyzer b2b-viewer b2b-serv-raw b2b-archiver libb2bllib.so*

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
