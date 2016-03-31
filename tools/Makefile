PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../ip_cores/etherbone-core/api
ECA	?= ../ip_cores/wr-cores/modules/wr_eca
TLU	?= ../ip_cores/wr-cores/modules/wr_tlu
TARGETS := eb-flash eb-sflash eb-info eb-console eb-config-nv eca-snoop button-game io-test eb-time

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(ECA) -I $(TLU)
LIBS        ?= -L $(EB)/.libs -L $(ECA) -L $(TLU) -Wl,-rpath,$(PREFIX)/lib -letherbone -leca -ltlu -lm

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS)
