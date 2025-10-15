# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EB          ?= ../../ip_cores/etherbone-core/api
WRPC        ?= ../../ip_cores/wrpc-sw
WRPC_IMPORT ?= ../../modules/lm32-include/wrpc-import
TARGETS     := eb-mon eb-massmon
USE_RPATH ?= yes

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I$(WRPC_IMPORT)/dev -I$(WRPC_IMPORT)  # -I $(WRPC)/include -I $(WRPC)/pp_printf
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
	$(CC) $(CFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC_IMPORT)/dev/w1.c $(WRPC_IMPORT)/dev/w1-temp.c $(WRPC_IMPORT)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-mon eb-mon.c $(LIBS)

eb-massmon: eb-massmon.c
	$(CC) $(CFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c $(WRPC_IMPORT)/dev/w1.c $(WRPC_IMPORT)/dev/w1-temp.c $(WRPC_IMPORT)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-massmon eb-massmon.c $(LIBS)

clean:
	rm -f *.o eb-mon eb-massmon

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
