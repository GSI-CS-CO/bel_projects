# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/wrunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../fw
INCLUDE     ?=../include
TARGETS     := wrunipz-ctl
WRUNIPZLIB  := libwrunipzlib.so


EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW) -I../../common-libs/include
LIBS        ?= -L $(EB)/.libs -L . -Wl,-rpath,$(PREFIX)/lib -lm -lwrunipzlib -letherbone

all: lib $(TARGETS)

wrunipz-ctl: wrunipz-ctl.c
	$(CC) $(CFLAGS) -o wrunipz-ctl wrunipz-ctl.c $(LIBS)

lib: wrunipzlib.c
	$(CC) $(CFLAGS) -fPIC -c wrunipzlib.c ../../common-libs/x86/common-lib.c $(LIBS)
	$(CC) -shared -Wl,-soname,$(WRUNIPZLIB).1 -o $(WRUNIPZLIB).1.0 wrunipzlib.o common-lib.o -L $(EB)/.libs -letherbone
	ln -sf $(WRUNIPZLIB).1.0 $(WRUNIPZLIB).1
	ln -sf $(WRUNIPZLIB).1 $(WRUNIPZLIB)

clean:
	rm -f *.o $(TARGETS) $(WRUNIPZLIB)*

deploy:
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin
	cp $(WRUNIPZLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1
	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(WRUNIPZLIB) 

.PHONY: all clean
