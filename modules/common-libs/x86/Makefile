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
TARGETS     := example-ctl 


EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I $(INCLUDE) -I $(FW)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone

all: $(TARGETS)

example-ctl: example-ctl.c
	$(CC) $(CFLAGS) common-lib.c -o example-ctl example-ctl.c $(LIBS)

clean:
	rm -f *.o $(TARGETS)

deploy:
	 mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin

.PHONY: all clean
