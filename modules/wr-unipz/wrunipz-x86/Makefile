# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/wrunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../wrunipz-fw
TARGETS     := wrunipz-ctl 


EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(FW)/../include -I $(FW)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all: $(TARGETS)

wrunipz-ctl: wrunipz-ctl.cpp
	$(CC) $(CFLAGS) -o wrunipz-ctl wrunipz-ctl.cpp $(LIBS)

clean:
	rm -f *.o $(TARGETS)

deploy:
	 mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin

.PHONY: all clean
