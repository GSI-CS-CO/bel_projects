# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../ip_cores/etherbone-core/api
WRPC        ?= ../../ip_cores/wrpc-sw
TARGETS     := eb-mon eb-massmon
USE_RPATH ?= yes

EXTRA_FLAGS ?=
CCFLAGS     ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(WRPC)/include -I $(WRPC)/pp_printf
ifeq ($(USE_RPATH),yes)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm
else
LIBS        ?= -letherbone -lm
endif

# special for eb-mon
WBDIR    = ..
WBFLAG   = -DNOWB_SIMULATE
WBFLAG  += -I$(WBDIR)

all: $(TARGETS)

eb-mon: eb-mon.c
	$(CC) $(CCFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-mon eb-mon.c $(LIBS)

eb-massmon: eb-massmon.c
	$(CC) $(CCFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-massmon eb-massmon.c $(LIBS)

clean:
	rm -f *.o eb-mon eb-massmon

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
