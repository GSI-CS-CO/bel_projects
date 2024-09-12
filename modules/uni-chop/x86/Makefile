# PREFIX  controls where programs and libraries get installed (default: '/usr/local')
# Note: during compile (all), PREFIX must be set to the final installation pa # Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make SYSENV=OPT PREFIX=/opt/usr all'                                             (hack: leave PREFIX empty for SCU path)
# Example install:
#   'make PREFIX=/opt/usr install'                                                    (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?=../fw
UNICHOPLIB  := libunichoplib.so

# include common Makefile stuff
include targetsysbuild.db

# special
# set MASP to YES, if unichop-serv-mon should function as status emitter to MASP (when in monitoring mode)
MASP        ?= NO              
# set PRO to YES, if unichop-servi-mon should function as status emitter to MASP PRO(otherwise: MASP DEV)
PRO         ?= NO

# stuff below required for MASP support
ifeq ($(MASP), YES)
GENERAL_LIBPATH = -L$(PREFIX)/$(ARCH) 
GENERAL_LIBS    = -lrt -lpthread

GSI_3RDPARTY_LOCATION ?= /opt/gsi/3rdparty
BOOST_VERSION  = 1.54.0
BOOST_HOME    ?= $(GSI_3RDPARTY_LOCATION)/boost/$(BOOST_VERSION)
BOOST_INCL     = -isystem$(BOOST_HOME)/include
BOOST_LIBPATH  = -L$(BOOST_HOME)/lib$(ARCH)
BOOST_LIBS     = -lboost_thread -lboost_system -lboost_atomic -lboost_chrono -lboost_filesystem -lboost_program_options


MASP_VERSION   = 1.0.5
MASP_DIR       = /common/usr/cscofe/opt/MASP/$(MASP_VERSION)
MASP_INCL      = -I $(MASP_DIR)/include
MASP_LIBPATH   = -L$(MASP_DIR)/lib$(ARCH) 
MASP_LIBS      = -lmasp_emitter -lmasp_status -lmasp_core

USEMASP        = -D USEMASP

ifeq ($(PRO), YES)
PRODUCTIVE     = -D PRODUCTIVE
endif

endif
# stuff above required for MASP support

EXTRA_FLAGS ?= $(USEMASP) $(PRODUCTIVE)
# the following is for c code
CCFLAGS     ?= $(EXTRA_FLAGS) -I$(EB) -I$(USRPATH)/include -I$(FW) -I../include -I../../common-libs/include 
LIBS        ?= -L. -L$(EB)/.libs -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -letherbone -lm -lunichoplib
# the following is for c++ code
CXFLAGS     ?= `pkg-config  saftlib --cflags` -I$(ASLINC) $(CCFLAGS) $(BOOST_INCL) $(MASP_INCL) 
XLIBS       ?= `pkg-config  saftlib --libs` -L$(ASLLIB) $(BOOST_LIBPATH) $(MASP_LIBPATH) $(LIBS) $(MASP_LIBS) $(GENERAL_LIBS) $(BOOST_LIBS) 

$(info PRO is $(PRO))
$(info PRODUCTIVE is $(PRODUCTIVE))
$(info MASP is $(MASP))
$(info ours/theirs CFLAGS is $(CFLAGS))
$(info compiling x86 binaries for $(SYSENV))
$(info ours CCFLAGS for c code is $(CCFLAGS))
$(info ours CXFLAGS for c++ code is $(CXFLAGS))

TARGETS     := unichop-ctl lib #unichop-serv-mon unichop-client-mon 

all: lib $(TARGETS)

unichop-ctl: unichop-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o unichop-ctl unichop-ctl.c $(LIBS)

unichop-client-mon: unichop-client-mon.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o unichop-client-mon unichop-client-mon.c $(LIBS) -ldim -lpthread 

unichop-serv-mon: unichop-serv-mon.cpp
	$(CXX) $(CFLAGS) $(CXFLAGS) ../../common-libs/x86/common-lib.c -o unichop-serv-mon unichop-serv-mon.cpp $(XLIBS) -ldim -lpthread

lib: unichoplib.c
	$(CC) $(CFLAGS) $(CCFLAGS) -fPIC -c unichoplib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(UNICHOPLIB).1 -o $(UNICHOPLIB).1.0 unichoplib.o common-lib.o -L $(EB)/.libs -letherbone -lm
	ln -sf $(UNICHOPLIB).1.0 $(UNICHOPLIB).1
	ln -sf $(UNICHOPLIB).1 $(UNICHOPLIB)

clean:
	rm -f *.o unichop-ctl wrf50-ctl unichop-serv-mon unichop-client-mon wrf50-serv-mon libunichoplib.so*

install:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin	
	cp $(UNICHOPLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNICHOPLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNICHOPLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNICHOPLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(UNICHOPLIB)

.PHONY: all clean
