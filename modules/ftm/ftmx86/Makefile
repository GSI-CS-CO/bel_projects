TOP          ?= ../../..

VERSION      ?= 0.6.2
VERSION_TOOL ?= 0.6.3


EBMPATH      ?= $(TOP)/ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH     ?= $(TOP)/modules/prioq2
FTMPATH      ?= $(TOP)/modules/ftm
SRCPATH      = $(FTMPATH)/src
BINPATH      = $(FTMPATH)/bin
LIBPATH      = $(FTMPATH)/lib
FWPATH       ?= $(FTMPATH)/ftmfw
TOOLPATH     ?= $(FTMPATH)/ftmx86
PREFIX       ?= /usr/local
INSTALLPATH  = $(PREFIX)/bin

T_CXX         = g++
T_CXXFLAGS    = -Wall -g -std=c++11 -I$(FTMPATH)/include -I$(FWPATH) -Wfatal-errors -DPLATFORM=$(BUILD) -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION)\" 

SRC_FILES     = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDM.cpp carpeDMcommand.cpp carpeDMschedule.cpp graph.cpp alloctable.cpp mempool.cpp
T_SOURCES     = $(addprefix $(SRCPATH)/, $(SRC_FILES))
T_LIBS        = -Wl,-rpath,/usr/local/lib -letherbone -lboost_graph -l:libboost_serialization.so.1.62.0

all: lib tools

lib: $(LIBPATH)/libcarpedm.a   

tools: $(BINPATH)/dm-sched $(BINPATH)/dm-cmd
	

$(BINPATH)/dm-sched: $(TOOLPATH)/dm-sched
	mkdir -p $(BINPATH) 
	cp $^ $(BINPATH)

$(BINPATH)/dm-cmd: $(TOOLPATH)/dm-cmd
	mkdir -p $(BINPATH)
	cp $^ $(BINPATH)


$(FWPATH)/ftm_shared_mmap.h:
	$(MAKE) -C $(FWPATH) ftm_shared_mmap.h


$(LIBPATH)/libcarpedm.a: $(T_SOURCES) $(FWPATH)/ftm_shared_mmap.h 
	mkdir -p $(LIBPATH)
	$(T_CXX) $(T_CXXFLAGS) -c $(T_SOURCES) $(T_LIBS)
	ar rcsv $@ *.o

$(TOOLPATH)/dm-sched: $(LIBPATH)/libcarpedm.a
	$(T_CXX) $(T_CXXFLAGS) -o $@ $(TOOLPATH)/dm-schedule.cpp $^ $(T_LIBS)

$(TOOLPATH)/dm-cmd: $(LIBPATH)/libcarpedm.a
	$(T_CXX) $(T_CXXFLAGS) -o $@ $(TOOLPATH)/dm-cmd.cpp $^ $(T_LIBS)

install: $(BINPATH)/dm-sched $(BINPATH)/dm-cmd
	cp $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(INSTALLPATH) 

clean::
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.a $(TOOLPATH)/*.elf $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(LIBPATH)/libcarpedm.a
