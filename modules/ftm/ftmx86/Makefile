TOP          ?= ../../..
EBMPATH      = $(TOP)/ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH     = $(TOP)/modules/prioq2
BASEPATH     = $(TOP)/modules/ftm
SRCPATH      = $(BASEPATH)/src
BINPATH      = $(BASEPATH)/bin
LIBPATH      = $(BASEPATH)/lib
TOOLPATH     = $(BASEPATH)/ftmx86
INSTALLPATH  = /usr/bin

VERSION      ?= 0.6.1
VERSION_TOOL ?= 0.6.1

T_CXX         = g++
T_CXXFLAGS    = -Wall -g -std=c++11 -I$(BASEPATH)/include -I$(BASEPATH)/ftmfw -Wfatal-errors -DPLATFORM=$(BUILD) -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION)\"

SRC_FILES     = event.cpp meta.cpp block.cpp memunit.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDM.cpp graph.cpp alloctable.cpp
T_SOURCES     = $(addprefix $(SRCPATH)/, $(SRC_FILES))
T_LIBS        = -Wl,-rpath,/usr/local/lib -letherbone -lboost_graph

all: $(LIBPATH)/libcarpedm.a tools

tools: $(BINPATH)/dm-sched $(BINPATH)/dm-cmd
	

$(BINPATH)/dm-sched: $(TOOLPATH)/dm-sched 
	cp $^ $(BINPATH)

$(BINPATH)/dm-cmd: $(TOOLPATH)/dm-cmd
	cp $^ $(BINPATH)

$(LIBPATH)/libcarpedm.a: $(T_SOURCES) $(BASEPATH)/ftmfw/ftm_shared_mmap.h
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
	        




