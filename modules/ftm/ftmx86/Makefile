############################
# Version Numbers to build #
############################

VERSION_FW     = 9.0.0
VERSION_TOOL   = 0.39.2
VERSION_CARPEDM= 1.1.2

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  # ASL build paths for asl75x
  EBPATH	=/common/export/timing-rte/tg-fallout-v6.3.1-rocky9
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  # ASL build paths for asl74x
  EBPATH	=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
endif
endif
############################
# Default build settings   #
# ##########################

TOP            ?= ../../..
EBPATH         ?= $(TOP)/ip_cores/etherbone-core/api/
EBPATH1        ?= .
BOOSTPATH      ?= .
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

# assume that Boost is installed on the maschine. Use BOOSTPATH if the global installed version should not be used.
# set BOOST_FLAGS depending on the Boost version
ifeq (,$(wildcard $(BOOSTPATH)/include/boost/version.hpp))
  BOOST_VERSION:=$(shell grep 'BOOST_LIB_VERSION "' /usr/include/boost/version.hpp | cut -b28-31)
else
  BOOST_VERSION:=$(shell grep 'BOOST_LIB_VERSION "' $(BOOSTPATH)/include/boost/version.hpp | cut -b28-31)
endif
ifeq ($(BOOST_VERSION),1_75)
  BOOST_FLAGS=-DBOOST_ALLOW_DEPRECATED_HEADERS -Wno-nonnull
endif

BUILD_DATE     = $(shell date +'%Y%m%d')
BUILD_ID_ROM_ADR = 0x100
BUILD_ID_ROM_SIZE = 0x400

CXX ?= g++
T_CXX           = $(CXX)
T_CXXFLAGS      = $(CXXFLAGS) -std=c++11 -fPIC -I$(FTMPATH)/include -I$(EBPATH)/include -I$(EBPATH1) -I$(DIAGPATH) -I$(LZMAPATH) -I$(BOOSTPATH)/include -Wfatal-errors -O
T_CXXFLAGS_CDM  = -DBUILD_DATE=\"$(BUILD_DATE)\" -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION_FW)\" -DETHERBONE_THROWS=1 -DBUILDID_OFFS=$(BUILD_ID_ROM_ADR) -DBUILDID_SIZE=$(BUILD_ID_ROM_SIZE) $(BOOST_FLAGS)

CC ?= gcc
T_CC            = $(CC)
T_CFLAGS        = $(CFLAGS) -Wall -I$(LZMAPATH) -fPIC -Wfatal-errors -D_7ZIP_ST -O

LZMA_SRC_FILES   = LzmaEnc.c LzmaDec.c LzFind.c
COMMON_SRC_FILES = event.cpp meta.cpp block.cpp visitoruploadcrawler.cpp visitordownloadcrawler.cpp visitorvertexwriter.cpp hashmap.cpp carpeDMimpl.cpp \
                   carpeDMcommand.cpp carpeDMuploadschedule.cpp carpeDMdownloadschedule.cpp carpeDMdiagnostics.cpp graph.cpp alloctable.cpp mempool.cpp \
                   dotstr.cpp idformat.cpp grouptable.cpp validation.cpp visitorvalidation.cpp common.cpp carpeDMsafe2remove.cpp lzmaCompression.cpp \
                   delayDiagnostics.cpp lockmanager.cpp blocklock.cpp ebwrapper.cpp log.cpp reflocation.cpp globalreftable.cpp global.cpp

CDM_SRC_FILES    = $(COMMON_SRC_FILES) carpeDM.cpp
TEST_SRC_FILES   = $(COMMON_SRC_FILES)

T_SOURCES       = $(addprefix $(SRCPATH)/, $(CDM_SRC_FILES)) $(addprefix $(LZMAPATH)/, $(LZMA_SRC_FILES))
T_OBJECTS       = $(subst .c,.o,$(LZMA_SRC_FILES)) $(subst .cpp,.o,$(CDM_SRC_FILES))

TEST_SOURCES    = $(addprefix $(SRCPATH)/, $(TEST_SRC_FILES)) $(addprefix $(LZMAPATH)/, $(LZMA_SRC_FILES))
TEST_OBJECTS    = $(subst .c,.o,$(LZMA_SRC_FILES)) $(subst .cpp,.o,$(TEST_SRC_FILES))

T_LIBS          = -L$(BOOSTPATH)/lib -L$(EBPATH)/lib -L$(EBPATH1)/.libs -letherbone -lboost_serialization -lboost_graph -lboost_regex
EXECS           = dm-sched dm-cmd
TOOLS           = $(addprefix $(TOOLPATH)/, $(EXECS))
BINS            = $(addprefix $(BINPATH)/, $(EXECS))



all: lib tools

lib: libcarpedm.so.$(VERSION_CARPEDM) libdmtest.so.$(VERSION_CARPEDM)

tools: $(TOOLS)


carpeDM.o: $(SRCPATH)/carpeDM.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@

%.o: 	$(SRCPATH)/%.cpp
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -c $< -o $@

%.o: 	$(LZMAPATH)/%.c
	$(T_CC) $(T_CFLAGS) -fPIC -c $^ -o $@


libcarpedm.so.$(VERSION_CARPEDM): $(T_OBJECTS)
	$(T_CXX) $(T_CXXFLAGS) -shared -o $@ $^ $(T_LIBS)
	mkdir -p $(LIBPATH)
	cp $@ $(LIBPATH)
	ln -sf $@ $(LIBPATH)/libcarpedm.so.$$(echo $(VERSION_CARPEDM) | cut -b1)
	ln -sf $@ $(LIBPATH)/libcarpedm.so

libdmtest.so.$(VERSION_CARPEDM): $(TEST_OBJECTS)
	$(T_CXX) $(T_CXXFLAGS) -shared -o $@ $^ $(T_LIBS)
	mkdir -p $(LIBPATH)
	cp $@ $(LIBPATH)
	ln -sf $@ $(LIBPATH)/libdmtest.so.$$(echo $(VERSION_CARPEDM) | cut -b1)
	ln -sf $@ $(LIBPATH)/libdmtest.so

install: $(BINS)
	cp $(BINS) $(INSTALLPATH)
	cp $(LIBPATH)/libcarpedm.so $(PREFIX)/lib/libcarpedm.so.$(VERSION_CARPEDM)
	ln -sf libcarpedm.so.$(VERSION_CARPEDM) $(PREFIX)/lib/libcarpedm.so.$$(echo $(VERSION_CARPEDM) | cut -b1)
	ln -sf libcarpedm.so.$(VERSION_CARPEDM) $(PREFIX)/lib/libcarpedm.so
	cp $(LIBPATH)/libdmtest.so $(PREFIX)/lib/libdmtest.so.$(VERSION_CARPEDM)
	ln -sf libdmtest.so.$(VERSION_CARPEDM) $(PREFIX)/lib/libdmtest.so.$$(echo $(VERSION_CARPEDM) | cut -b1)
	ln -sf libdmtest.so.$(VERSION_CARPEDM) $(PREFIX)/lib/libdmtest.so

clean::
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.a $(TOOLPATH)/libcarpedm.so* $(TOOLPATH)/libdmtest.so* $(TOOLPATH)/*.elf $(BINPATH)/dm-sched $(BINPATH)/dm-cmd $(LIBPATH)/libcarpedm.so* $(LIBPATH)/libdmtest.so*

.SECONDEXPANSION:
$(TOOLS): $$@.cpp libcarpedm.so.$(VERSION_CARPEDM)
	$(T_CXX) $(T_CXXFLAGS) $(T_CXXFLAGS_CDM) -o $@ $< -L. -l:libcarpedm.so.$(VERSION_CARPEDM) $(T_LIBS)
	mkdir -p $(BINPATH)
	cp $@ $(BINPATH)


