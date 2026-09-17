# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../../ip_cores/etherbone-core/api
WRPC        ?= ../../../ip_cores/wrpc-sw
TARGETS     := fbas-mon
USE_RPATH   ?= yes
WRPCSRC     = $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c \
                $(WRPC)/tools/eb-w1.c

EXTRA_FLAGS ?=
CFLAGS      += $(EXTRA_FLAGS) -Wall -O2 \
                -I ../include -I ../../common-libs/include \
                -I $(EB) -I $(WRPC)/include -I $(WRPC)/pp_printf
ifeq ($(USE_RPATH),yes)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm
else
LIBS        ?= -letherbone -lm
endif

# WB APIs
WBDIR    = ../../../tools
WBFLAG   = -DNOWB_SIMULATE
WBFLAG  += -I$(WBDIR)
WBSRC    = $(WBDIR)/wb_api.c

all: $(TARGETS)

fbas-mon: fbas-mon.c
	$(CC) $(CFLAGS) $(WBFLAG) $(WBSRC) $(WRPCSRC) -o $@ $? $(LIBS)

clean:
	rm -f *.o fbas-mon

#install:
#	mkdir -p $(STAGING)$(PREFIX)/bin
#	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
