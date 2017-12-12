# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
# Example deploy:
#   make PREFIX=/usr STAGING=/common/export/timing-rte/dmunipz-dev/x86_64 deploy
PREFIX      ?= /usr/local
STAGING     ?=
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

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
