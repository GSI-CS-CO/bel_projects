# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= ..
TARGETS     := wr-mil-gw-ctl

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(FW) -std=c99
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all: $(TARGETS)

wr-mil-gw-ctl: wr-mil-gw-ctl.c
	$(CC) $(CFLAGS) -o wr-mil-gw-ctl wr-mil-gw-ctl.c $(LIBS)

clean:
	rm -f *.o wr-mil-gw-ctl

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean

