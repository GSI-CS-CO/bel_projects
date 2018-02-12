EBMPATH   ?= ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  ?= ../../../modules/prioq2
PATHTOOL  ?= .
XMLPATH   = /usr/include/libxml2
VERSION         ?= 0.2.4
VERSION_TOOL    ?= 0.1.4

T_FLAGS   = -fPIC -Wall -O2 -g -I$(PRIOPATH) -I$(EBMPATH) -I$(XMLPATH) -I$(PATHTOOL)
T_CFLAGS  = $(T_FLAGS) -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD) -DTOOL_VER=\"$(VERSION_TOOL)\"  -DEXP_VER=\"$(VERSION)\" -std=gnu99
T_SOURCES	= $(PATHTOOL)/ftmx86.c $(PATHTOOL)/xmlaux.c $(PATHTOOL)/access.c
T_LIBS    = -Wl,-rpath,/usr/local/lib -letherbone -lxml2
T_LIBRARY = libetherbone.so libxml2.so
T_EXTRA   = libetherbone.so.*
T_CXXFLAGS= $(T_FLAGS)
T_CC      = gcc
T_CXX     = g++


tool: $(PATHTOOL)/ftm-ctl

$(PATHTOOL)/libftm.a: $(T_SOURCES)
	$(T_CC) $(T_CFLAGS) -c $(T_SOURCES) $(T_LIBS)
	ar rcsv $@ *.o 

$(PATHTOOL)/ftm-ctl:	$(PATHTOOL)/libftm.a 
	$(T_CC) $(T_CFLAGS) -o $@ $(PATHTOOL)/main.c $^ $(T_LIBS)

install: $(PATHTOOL)/ftm-ctl
	cp $(PATHTOOL)/ftm-ctl $(PREFIX)/bin/

clean::
	rm -f $(PATHTOOL)/*.o $(PATHTOOL)/*.a $(PATHTOOL)/*.elf $(PATHTOOL)/*.bin $(PATHTOOL)/ftm-ctl $(PATHTOOL)/mockup-test
	        
mockup: $(PATHTOOL)/ftmx86.c $(PATHTOOL)/xmlaux.c $(PATHTOOL)/mockup.c
	$(T_CC) $(T_CFLAGS) -DMOCKUP=1 -c $^ -Wl,-rpath, -lxml2
	ar rcsv libftmmockup.a *.o  

mockup-test:	mockup
	$(T_CC) $(T_CFLAGS) -DMOCKUP=1 -o $@ $(PATHTOOL)/main.c libftmmockup.a -Wl,-rpath, -lxml2



