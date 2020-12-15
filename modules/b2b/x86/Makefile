# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../fw
INCLUDE     ?=../include
TARGETS     := b2b-ctl 
B2BLIB      := libb2blib.so

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I../../common-libs/include
LIBS        ?= -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm -letherbone -lb2blib

all: $(TARGETS)

b2b-ctl: b2b-ctl.c
	$(CC) $(CFLAGS) -o b2b-ctl b2b-ctl.c $(LIBS)

b2b-ui: b2b-ui.c
	$(CC) $(CFLAGS) -o b2b-ui b2b-ui.c ../../common-libs/x86/ivtpar.c $(LIBS)

lib: b2blib.c
	$(CC) $(CFLAGS) -fPIC -c b2blib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(B2BLIB).1 -o $(B2BLIB).1.0 b2blib.o common-lib.o -L $(EB)/.libs -letherbone
	ln -sf $(B2BLIB).1.0 $(B2BLIB).1
	ln -sf $(B2BLIB).1 $(B2BLIB)

clean:
	rm -f *.o b2b-ctl b2b-ui libb2bllib.so*

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
