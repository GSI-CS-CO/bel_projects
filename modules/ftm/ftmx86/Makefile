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


#hack for fuckin libetherbone chaos on asl cluster ...

ifeq ($(findstring asl,$(shell hostname)),asl) 
	EB_INC=-I/common/export/timing-rte/tg-cherry-v2/x86_64/include
	EB_LIB=/common/export/timing-rte/tg-cherry-v2/x86_64/lib/libetherbone.so
else
	EB_LIB=-letherbone
endif


T_CXX         = g++
T_CXXFLAGS    = -Wall -g -std=c++11 -I$(FTMPATH)/include $(EB_INC) -I$(FWPATH) -Wfatal-errors -DPLATFORM=$(BUILD) -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION)\" 

SRC_FILES     = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDM.cpp carpeDMcommand.cpp carpeDMuploadschedule.cpp carpeDMdownloadschedule.cpp graph.cpp alloctable.cpp mempool.cpp dotstr.cpp idformat.cpp
T_SOURCES     = $(addprefix $(SRCPATH)/, $(SRC_FILES))
T_OBJECTS     = $(subst .cpp,.o,$(SRC_FILES))
T_LIBS        = -Wl,-rpath,/usr/local/lib $(EB_LIB) -lboost_graph -lboost_serialization



all: lib tools

lib: $(LIBPATH)/libcarpedm.so  

tools: $(BINPATH)/dm-sched $(BINPATH)/dm-cmd
	

$(BINPATH)/dm-sched: $(TOOLPATH)/dm-sched
	mkdir -p $(BINPATH) 
	cp $^ $(BINPATH)

$(BINPATH)/dm-cmd: $(TOOLPATH)/dm-cmd
	mkdir -p $(BINPATH)
	cp $^ $(BINPATH)


$(FWPATH)/ftm_shared_mmap.h:
	$(MAKE) -C $(FWPATH) ftm_shared_mmap.h


$(LIBPATH)/libcarpedm.so: $(T_SOURCES) $(FWPATH)/ftm_shared_mmap.h 
	mkdir -p $(LIBPATH)
	$(T_CXX) $(T_CXXFLAGS) -fPIC -c $(T_SOURCES) $(T_LIBS)
	$(T_CXX) -shared -o $@ $(T_OBJECTS)


$(TOOLPATH)/dm-sched: $(LIBPATH)/libcarpedm.so
	$(T_CXX) $(T_CXXFLAGS) -o $@ $(TOOLPATH)/dm-schedule.cpp $^ $(T_LIBS) -L$(LIBPATH) -lcarpedm


$(TOOLPATH)/dm-cmd: $(LIBPATH)/libcarpedm.so
	$(T_CXX) $(T_CXXFLAGS) -o $@ $(TOOLPATH)/dm-cmd.cpp $^ $(T_LIBS) -L$(LIBPATH) -lcarpedm

install: $(BINPATH)/dm-sched $(BINPATH)/dm-cmd
	cp $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(INSTALLPATH) 

clean::
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.a $(TOOLPATH)/*.elf $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(LIBPATH)/libcarpedm.so
