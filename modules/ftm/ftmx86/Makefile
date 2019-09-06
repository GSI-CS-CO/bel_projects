
#hack for fuckin lib chaos (both on asl cluster and my computer... )

#ifeq ($(findstring asl,$(shell hostname)),asl)
  EBPATH        =/common/export/timing-rte/tg-enigma-v5.0.0-pre-alpha/x86_64
  BOOSTPATH     =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
#  SER_LIB       =/lib/libboost_serialization.so
#  GRAPH_LIB     =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation/lib/libboost_graph.so
#else
# FIXME WTF ? why doesn't this work with all rpath's and -llib notation ? it does for graph!
  EB_LIB        =$(EBPATH)/lib/libetherbone.so
  GRAPH_LIB     =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation/lib/libboost_graph.so
  SER_LIB       =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation/lib/libboost_serialization.so
  RE_LIB       =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation/lib/libboost_regex.so
#  SER_LIB       =-lboost_serialization   
#  BOOSTPATH     =..
#endif

TOP            ?= ../../..

VERSION_FW     = 6.2.0
VERSION_TOOL   = 0.33.4

BUILD_DATE     = $(shell date +'%Y%m%d')
BUILD_ID_ROM_ADR = 0x100
BUILD_ID_ROM_SIZE = 0x400

EBMPATH        ?= $(TOP)/ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH       ?= $(TOP)/modules/prioq2
DIAGPATH       ?= $(TOP)/modules/dm_diag
FTMPATH        ?= $(TOP)/modules/ftm
SRCPATH         = $(FTMPATH)/src
LZMAPATH        = $(SRCPATH)/lzma/C
BINPATH         = $(FTMPATH)/bin
LIBPATH         = $(FTMPATH)/lib
FWPATH         ?= $(FTMPATH)/ftmfw
TOOLPATH       ?= $(FTMPATH)/ftmx86
PREFIX         ?= /usr/local
INSTALLPATH     = $(PREFIX)/bin

T_CXX           = g++
T_CXXFLAGS      = -g -std=c++11 -fPIC -I$(FTMPATH)/include -I$(EBPATH)/include -I$(DIAGPATH) -I$(LZMAPATH) -I$(BOOSTPATH)/include -Wall
T_CXXFLAGS_CDM  = -DBUILD_DATE=\"$(BUILD_DATE)\" -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION_FW)\" -DETHERBONE_THROWS=1 -DBUILDID_OFFS=$(BUILD_ID_ROM_ADR) -DBUILDID_SIZE=$(BUILD_ID_ROM_SIZE)
T_CC            = gcc
T_CFLAGS        = -Wall -g -I$(LZMAPATH) -fPIC -Wfatal-errors -D_7ZIP_ST 

LZMA_SRC_FILES  = LzmaEnc.c LzmaDec.c LzFind.c
CDM_SRC_FILES   = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDMimpl.cpp carpeDM.cpp carpeDMcommand.cpp carpeDMuploadschedule.cpp carpeDMdownloadschedule.cpp carpeDMdiagnostics.cpp graph.cpp alloctable.cpp mempool.cpp dotstr.cpp idformat.cpp grouptable.cpp validation.cpp visitorvalidation.cpp common.cpp carpeDMsafe2remove.cpp lzmaCompression.cpp delayDiagnostics.cpp lockmanager.cpp blocklock.cpp ebwrapper.cpp

T_SOURCES       = $(addprefix $(SRCPATH)/, $(CDM_SRC_FILES)) $(addprefix $(LZMAPATH)/, $(LZMA_SRC_FILES))
T_OBJECTS       = $(subst .c,.o,$(LZMA_SRC_FILES)) $(subst .cpp,.o,$(CDM_SRC_FILES))
T_LIBS          = -Wl,-rpath,$(EBPATH)/lib,-rpath,$(BOOSTPATH)/lib,-rpath,/usr/local/lib $(EB_LIB) $(GRAPH_LIB) $(SER_LIB) $(RE_LIB)
EXECS           = dm-sched dm-cmd 
TOOLS           = $(addprefix $(TOOLPATH)/, $(EXECS))
BINS            = $(addprefix $(BINPATH)/, $(EXECS))

all: lib tools

lib: libcarpedm.so

tools: $(TOOLS)


carpeDM.o: $(SRCPATH)/carpeDM.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@

%.o: 	$(SRCPATH)/%.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@ $(T_LIBS)

%.o: 	$(LZMAPATH)/%.c
	$(T_CC) $(T_CFLAGS) -fPIC -c $^ -o $@


libcarpedm.so: $(T_OBJECTS)
	$(T_CXX) $(T_CXXFLAGS) -shared -o $@ $^ $(T_LIBS)
	mkdir -p $(LIBPATH)
	cp $@ $(LIBPATH)

install: $(BINS)
	cp $(BINS) $(INSTALLPATH)
	cp $(LIBPATH)/libcarpedm.so $(PREFIX)/lib

clean::
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.a $(TOOLPATH)/*.so $(TOOLPATH)/*.elf $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(LIBPATH)/libcarpedm.so

.SECONDEXPANSION:
$(TOOLS): $$@.cpp libcarpedm.so
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -o $@ $< -L$(LIBPATH) -lcarpedm
	mkdir -p $(BINPATH)
	cp $@ $(BINPATH)


