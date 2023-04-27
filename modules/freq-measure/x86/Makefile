# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# Example usage:
#   'make clean'                                                 
#   'make YOCTO=YES PREFIX= all'                                                       (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# support Yocto SDK
ifeq ($(YOCTO), YES)
EB          ?= .
ARCH        := /x86_64
CFLAGS      ?=
DIMPATH     := /common/usr/timing/b2b/yocto/dim_v20r33
ASLLIB      := .
ASLINC      := .
else
EB          ?= ../../../ip_cores/etherbone-core/api
ARCH        ?= /x86_64
CFLAGS      ?= -Wall -O2 -g
#DIMPATH     := /opt/dim/dim_v20r33
# the following four lines are a hack required for building software for sl7 (deprecated)
DIMPATH     := /common/usr/timing/b2b/dim_v20r29
ASLLIB      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib
ASLINC      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/include/saftlib
CXX         := g++ --std=c++0x
endif

EXTRA_FLAGS ?=
LIBS        ?= -L. -Wl,-rpath,$(PREFIX)/lib -L$(DIMPATH)/linux
CCFLAGS     ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -I$(EB) -I$(FW) -I$(ASLINC) -I../include -I../../common-libs/include  -I$(DIMPATH)/dim
SAFTLIBS    ?= `pkg-config  saftlib --libs` -L. -L$(ASLLIB) -Wl,-rpath,$(PREFIX)/lib -lm -L$(DIMPATH)/linux

$(info    CCFLAGS is $(CCFLAGS))
$(info     CFLAGS is $(CFLAGS))

TARGETS     := freq-measure freq-mon-simple

all: $(TARGETS)

freq-measure: freq-measure.cpp
	$(CXX) $(CFLAGS) $(CCFLAGS) -o freq-measure freq-measure.cpp $(SAFTLIBS) -ldim -lpthread

freq-mon-simple: freq-mon-simple.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o freq-mon-simple freq-mon-simple.c $(LIBS) -ldim -lpthread

clean: 
	rm -f freq-measure freq-mon-simple
