# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= ..
TARGETS     := saft-wrmilgw-ctl

# set MASP to YES, if saft-wrmilgw-ctl has additional command line parameters to be configured as status emitter to MASP (when in monitoring mode)
MASP        ?= NO              

# stuff below required for MASP support
ifeq ($(MASP), YES)

SYSTEM_HOME ?= /usr

CPU ?= x86_64
ifeq ($(CPU),x86_64)
	SYSTEM_LIB=lib64
endif
ifeq ($(CPU),i686)
	SYSTEM_LIB=lib
endif

GENERAL_LIBS += -L$(SYSTEM_HOME)/$(SYSTEM_LIB)
GENERAL_LIBS += -lm -lrt -lpthread
GENERAL_INCL = -I.

GSI_3RDPARTY_LOCATION ?= /opt/gsi/3rdparty
BOOST_VERSION = 1.54.0
BOOST_HOME ?= $(GSI_3RDPARTY_LOCATION)/boost/$(BOOST_VERSION)
BOOST_INCL += -isystem$(BOOST_HOME)/include
BOOST_LIBS += -L$(BOOST_HOME)/lib/$(CPU)
BOOST_LIBS += -lboost_thread -lboost_system -lboost_atomic -lboost_chrono -lboost_filesystem -lboost_program_options

MASP_VERSION = 1.0.5
MASP_DIR = /common/usr/cscofe/opt/MASP/$(MASP_VERSION)
MASP_INCL += -I$(MASP_DIR)/include
MASP_LIBS += -L$(MASP_DIR)/lib/$(CPU) -lmasp_emitter -lmasp_status -lmasp_core

INCLUDES += $(BOOST_INCL) $(GENERAL_INCL) $(MASP_INCL)
MASP_LIBS +=  $(BOOST_LIBS) $(GENERAL_LIBS)

USEMASP        = -D USEMASP

endif
# stuff above required for MASP support


EXTRA_FLAGS ?= $(USEMASP) $(PRODUCTIVE)
CFLAGS      ?= `pkg-config --define-variable=prefix=$(STAGING) saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(FW) $(BOOST_INCL) $(MASP_INCL) 
LIBS        ?= `pkg-config --define-variable=prefix=$(STAGING) saftlib --libs` -L $(EB)/.libs $(BOOST_LIBPATH) $(MASP_LIBPATH) -Wl,-rpath,$(PREFIX)/lib -letherbone -lm $(MASP_LIBS) $(GENERAL_LIBS) $(BOOST_LIBS) 
CC          = g++ --std=c++0x

#EXTRA_FLAGS ?=
#CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(FW) -I. 
#LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm


all: $(TARGETS)

saft-wrmilgw-ctl: saft-wrmilgw-ctl.cpp
	$(CC) $(CFLAGS) -o saft-wrmilgw-ctl saft-wrmilgw-ctl.cpp  $(LIBS)

clean:
	rm -f *.o saft-wrmilgw-ctl

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean

