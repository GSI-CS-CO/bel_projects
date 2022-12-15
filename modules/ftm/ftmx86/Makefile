############################
# Version Numbers to build #
############################

VERSION_FW     = 8.0.4
VERSION_TOOL   = 0.36.3

############################
# ASL build paths          #
############################

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  EBPATH	=/common/export/timing-rte/tg-fallout-v6.2.x-rocky8/x86_64
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  EBPATH	=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
endif
endif
############################
# Default build settings   #
# ##########################

TOP            ?= ../../..
BOOSTPATH      ?=/usr
EBPATH         ?=/usr/local
EBPATH1 ?=/usr/local/include
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
# check that make runs with  sudo rights
RIGHTS := $(shell id -u)

ifeq ($(RIGHTS),0)
  PREFIX ?= /usr/local
else
  PREFIX ?= $$HOME/.local
endif
INSTALLPATH     = $(PREFIX)/bin

BUILD_DATE     = $(shell date +'%Y%m%d')
BUILD_ID_ROM_ADR = 0x100
BUILD_ID_ROM_SIZE = 0x400

T_CXX           = g++
T_CXXFLAGS      = -std=c++11 -fPIC -I$(FTMPATH)/include -I$(EBPATH)/include -I$(EBPATH1) -I$(DIAGPATH) -I$(LZMAPATH) -I$(BOOSTPATH)/include -Wall -O
T_CXXFLAGS_CDM  = -DBUILD_DATE=\"$(BUILD_DATE)\" -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION_FW)\" -DETHERBONE_THROWS=1 -DBUILDID_OFFS=$(BUILD_ID_ROM_ADR) -DBUILDID_SIZE=$(BUILD_ID_ROM_SIZE)
T_CC            = gcc
T_CFLAGS        = -Wall -I$(LZMAPATH) -fPIC -Wfatal-errors -D_7ZIP_ST -O

LZMA_SRC_FILES  = LzmaEnc.c LzmaDec.c LzFind.c
CDM_SRC_FILES   = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDMimpl.cpp carpeDM.cpp carpeDMcommand.cpp carpeDMuploadschedule.cpp carpeDMdownloadschedule.cpp carpeDMdiagnostics.cpp graph.cpp alloctable.cpp mempool.cpp dotstr.cpp idformat.cpp grouptable.cpp validation.cpp visitorvalidation.cpp common.cpp carpeDMsafe2remove.cpp lzmaCompression.cpp delayDiagnostics.cpp lockmanager.cpp blocklock.cpp ebwrapper.cpp
TEST_SRC_FILES   = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDMimpl.cpp carpeDMcommand.cpp carpeDMuploadschedule.cpp carpeDMdownloadschedule.cpp carpeDMdiagnostics.cpp graph.cpp alloctable.cpp mempool.cpp dotstr.cpp idformat.cpp grouptable.cpp validation.cpp visitorvalidation.cpp common.cpp carpeDMsafe2remove.cpp lzmaCompression.cpp delayDiagnostics.cpp lockmanager.cpp blocklock.cpp ebwrapper.cpp

T_SOURCES       = $(addprefix $(SRCPATH)/, $(CDM_SRC_FILES)) $(addprefix $(LZMAPATH)/, $(LZMA_SRC_FILES))
T_OBJECTS       = $(subst .c,.o,$(LZMA_SRC_FILES)) $(subst .cpp,.o,$(CDM_SRC_FILES))

TEST_SOURCES    = $(addprefix $(SRCPATH)/, $(TEST_SRC_FILES)) $(addprefix $(LZMAPATH)/, $(LZMA_SRC_FILES))
TEST_OBJECTS    = $(subst .c,.o,$(LZMA_SRC_FILES)) $(subst .cpp,.o,$(TEST_SRC_FILES))

T_LIBS          = -Wl,-rpath,$(EBPATH)/lib,-rpath,$(EBPATH1)/.libs,-rpath,$(BOOSTPATH)/lib -L$(BOOSTPATH)/lib -L$(EBPATH)/lib -L$(EBPATH1)/.libs -letherbone -lboost_serialization -lboost_graph -lboost_regex
EXECS           = dm-sched dm-cmd
TOOLS           = $(addprefix $(TOOLPATH)/, $(EXECS))
BINS            = $(addprefix $(BINPATH)/, $(EXECS))



all: lib tools

lib: libcarpedm.so libdmtest.so

tools: $(TOOLS)


carpeDM.o: $(SRCPATH)/carpeDM.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@

%.o: 	$(SRCPATH)/%.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@

%.o: 	$(LZMAPATH)/%.c
	$(T_CC) $(T_CFLAGS) -fPIC -c $^ -o $@


libcarpedm.so: $(T_OBJECTS)
	$(T_CXX) $(T_CXXFLAGS) -shared -o $@ $^ $(T_LIBS)
	mkdir -p $(LIBPATH)
	cp $@ $(LIBPATH)

libdmtest.so: $(TEST_OBJECTS)
	$(T_CXX) $(T_CXXFLAGS) -shared -o $@ $^ $(T_LIBS)
	mkdir -p $(LIBPATH)
	cp $@ $(LIBPATH)

install: $(BINS)
	cp $(BINS) $(INSTALLPATH)
	cp $(LIBPATH)/libcarpedm.so $(LIBPATH)/libdmtest.so $(PREFIX)/lib

clean::
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.a $(TOOLPATH)/*.so $(TOOLPATH)/*.elf $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(LIBPATH)/libcarpedm.so $(LIBPATH)/libdmtest.so

.SECONDEXPANSION:
$(TOOLS): $$@.cpp libcarpedm.so
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -o $@ $< -L$(LIBPATH) -lcarpedm $(T_LIBS)
	mkdir -p $(BINPATH)
	cp $@ $(BINPATH)


