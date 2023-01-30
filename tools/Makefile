PREFIX  ?= /usr/local
STAGING ?=
EB      ?= ../ip_cores/etherbone-core/api
ECA	?= ../ip_cores/wr-cores/modules/wr_eca
TLU	?= ../ip_cores/wr-cores/modules/wr_tlu
WRPC    ?= ../ip_cores/wrpc-sw
WBDIR   ?= .
TARGETS := eb-flash eb-info eb-console eb-config-nv eb-time eb-sflash eb-iflash eb-reset eb-daq-dump eb-fwload eb-i2c-master eb-fg-statistic
EXTRA_I := eb-flash-secure
USE_RPATH ?= yes

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -std=gnu99 -Wall -O2 -I $(EB)
ifeq ($(USE_RPATH),yes)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm
else
LIBS        ?= -letherbone -lm
endif

all:	$(TARGETS) monitoring

clean:  monitoring-clean
	rm -f $(TARGETS)

install: monitoring-install
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin
	cp $(EXTRA_I) $(STAGING)$(PREFIX)/bin

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

eb-reset: eb-reset.c
	$(CC) $(CFLAGS) -I$(WBDIR) -I$(WRPC)/include -I$(WRPC)/pp_printf $(WBDIR)/wb_api.c  $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-reset eb-reset.c $(LIBS)
