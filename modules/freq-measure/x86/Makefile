# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example build:
#   'make clean'                                                 
#   'make SYSENV=OPT PREFIX=/opt/usr all'                                             (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'                                                    (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths

# target system environment, OPT is default
SYSENV      ?= OPT
ASLINC      ?= .
ASLLIB      ?= .

ifeq ($(SYSENV), OPT)
USRPATH     ?= /opt/usr
ARCH        ?=
EB          ?= ../../../ip_cores/etherbone-core/api
CFLAGS      ?= -Wall -O2 -g
CXX         := g++ --std=c++0x
endif

ifeq ($(SYSENV), YOCTO)
USRPATH     := /common/usr/timing/b2b/yocto/usr
ARCH        := /x86_64
EB          ?= .
CFLAGS      ?=
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/b2b/centos7/usr
ARCH        := /x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
CFLAGS      ?= -Wall -O2 -g
CXX         := g++ --std=c++0x
ASLINC      := $(PKG_CONFIG_PATH)../../include/saftlib
ASLLIB      := $(PKG_CONFIG_PATH)../../lib
endif

EXTRA_FLAGS ?=

# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I../include -I../../common-libs/include
LIBS        ?= -L. -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) $(CCFLAGS)
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(LIBS)

$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := freq-measure freq-mon-simple

all: $(TARGETS)

freq-measure: freq-measure.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) -o freq-measure freq-measure.cpp $(XLIBS) -ldim -lpthread

freq-mon-simple: freq-mon-simple.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o freq-mon-simple freq-mon-simple.c $(LIBS) -ldim -lpthread

clean: 
	rm -f freq-measure freq-mon-simple

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	

.PHONY: all clean
