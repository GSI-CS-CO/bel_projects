PREFIX      ?= /usr/local
STAGING     ?= 
EB          ?= ../ip_cores/etherbone-core/api
WRPC_INC    ?= ../../../ip_cores/wrpc-sw/include
WRPC_DEV    ?= ../../../ip_cores/wrpc-sw/dev
WRPC_TOOLS  ?= ../../../ip_cores/wrpc-sw/tools

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(ECA) -I $(TLU) -I $(WRPC_INC)
LIBS        ?= -L $(EB) -L $(ECA) -L $(TLU) -Wl,-rpath,$(PREFIX)/lib -letherbone -leca -ltlu

all: onewire-scanner

onewire-scanner: onewire-scanner.c $(WRPC_TOOLS)/eb-w1.c  $(WRPC_DEV)/w1.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ -L $(EB) -letherbone

clean:
	rm -f $(all)

