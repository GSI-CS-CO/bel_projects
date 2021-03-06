# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/dmunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../fw
TARGETS     := dmunipz-ctl 

# set MASP to YES, if dmunipz-ctl should function as status emitter to MASP (when in monitoring mode)
MASP        ?= NO              
# set PRO to YES, if dmunipz-ctl should function as status emitter to MASP PRO(otherwise: MASP DEV)
PRO         ?= NO

# stuff below required for MASP support
ifeq ($(MASP), YES)
GENERAL_LIBPATH = -L$(PREFIX)/$(ARCH) 
GENERAL_LIBS    = -lrt -lpthread

GSI_3RDPARTY_LOCATION ?= /opt/gsi/3rdparty
BOOST_VERSION  = 1.54.0
BOOST_HOME    ?= $(GSI_3RDPARTY_LOCATION)/boost/$(BOOST_VERSION)
BOOST_INCL    = -isystem$(BOOST_HOME)/include
BOOST_LIBPATH = -L $(BOOST_HOME)/lib/$(ARCH)
BOOST_LIBS    = -lboost_thread -lboost_system -lboost_atomic -lboost_chrono -lboost_filesystem -lboost_program_options


MASP_VERSION = 1.0.5
MASP_DIR      = /common/usr/cscofe/opt/MASP/$(MASP_VERSION)
MASP_INCL     = -I $(MASP_DIR)/include
MASP_LIBPATH  = -L $(MASP_DIR)/lib/$(ARCH) 
MASP_LIBS     = -lmasp_emitter -lmasp_status -lmasp_core

# CC          = g++

USEMASP       = -D USEMASP

ifeq ($(PRO), YES)
PRODUCTIVE    = -D PRODUCTIVE
endif

endif
# stuff above required for MASP support

EXTRA_FLAGS  ?= $(USEMASP) $(PRODUCTIVE)
CFLAGS       ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(FW) -I $(FW)/../include -I $(FW) -I../../common-libs/include $(BOOST_INCL) $(MASP_INCL) 
LIBS         ?= -L $(EB)/.libs $(BOOST_LIBPATH) $(MASP_LIBPATH) -Wl,-rpath,$(PREFIX)/lib -letherbone -lm $(MASP_LIBS) $(GENERAL_LIBS) $(BOOST_LIBS) 

# compile everything as C++ (always)
CC            = g++

all: $(TARGETS)

dmunipz-ctl: dmunipz-ctl.cpp
	$(CC) $(CFLAGS)  ../../common-libs/x86/common-lib.c -o dmunipz-ctl dmunipz-ctl.cpp $(LIBS)

clean:
	rm -f *.o $(TARGETS)

deploy:
	 mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin

.PHONY: all clean
