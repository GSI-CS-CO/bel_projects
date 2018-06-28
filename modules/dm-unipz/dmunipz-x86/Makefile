# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX= all                                                     (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   make PREFIX= STAGING=/common/export/timing-rte/dmunipz-dev deploy    (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
EB          ?= ../../../ip_cores/etherbone-core/api
FW          ?=../dmunipz-fw
TARGETS     := dmunipz-ctl 

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(FW)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all: $(TARGETS)

dmunipz-ctl: dmunipz-ctl.c
	$(CC) $(CFLAGS) -o dmunipz-ctl dmunipz-ctl.c $(LIBS)

clean:
	rm -f *.o $(TARGETS)

deploy:
	 mkdir -p $(STAGING)$(ARCH)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(ARCH)$(PREFIX)/bin

.PHONY: all clean
