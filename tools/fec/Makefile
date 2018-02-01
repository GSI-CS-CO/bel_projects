#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../ip_cores/etherbone-core/api
WRPC        ?= ../../ip_cores/wrpc-sw
TARGETS     := eb-fec eb-fec-test

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(WRPC)/include -I $(WRPC)/pp_printf
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

# special for eb-fec
WBDIR    = ..
WBFLAG  += -I$(WBDIR)

all: $(TARGETS)

eb-fec: eb-fec.c
	$(CC) $(CFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-fec eb-fec.c $(LIBS)

eb-fec-test: eb-fec-test.c
	$(CC) $(CFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-fec-test eb-fec-test.c $(LIBS)

clean:
	rm -f *.o eb-fec

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
