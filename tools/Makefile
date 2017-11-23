PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../ip_cores/etherbone-core/api
ECA	?= ../ip_cores/wr-cores/modules/wr_eca
TLU	?= ../ip_cores/wr-cores/modules/wr_tlu
TARGETS := eb-flash eb-info eb-console eb-config-nv eb-time eb-sflash eb-iflash

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) 
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all:	$(TARGETS) monitoring

clean:  monitoring-clean
	rm -f $(TARGETS)

install: monitoring-install
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS)

monitoring: monitoringg

monitoring-install:
	$(MAKE) -C monitoring install

monitoring-clean:
	$(MAKE) -C monitoring clean

monitoringg:
	$(MAKE) -C monitoring EXTRA_FLAGS="$(EXTRA_FLAGS)" all	

ecatools: button-game io-test

ecatools-clean:
	rm -f button-game io-test

ecatools-install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp button-game io-test $(STAGING)$(PREFIX)/bin

button-game: button-game.cpp
	g++ $(CFLAGS) -I $(ECA) -I $(TLU) -o button-game button-game.cpp -L $(ECA) -L $(TLU) $(LIBS) -leca -ltlu

io-test: io-test.cpp
	g++ $(CFLAGS) -I $(ECA) -I $(TLU) -o io-test io-test.cpp -L $(ECA) -L $(TLU) $(LIBS) -leca -ltlu





