# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all'                                                (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b deploy'       (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= fw
SW          ?= x86
ASL         ?= asl
#TARGETS     := firmware software

#EXTRA_FLAGS ?=
#CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(FW)
#LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all:: firmware software

software:: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) TARGET=b2bcbu -C $(FW)
	$(MAKE) TARGET=b2bpm -C $(FW)	
	$(MAKE) TARGET=b2bkd -C $(FW)	

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt                      # needed for ivtpar
	mkdir -p $(STAGING)/firmware
#	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin         # '/usr' is a hack
	cp $(ASL)/b2b-sis18-rf.sh $(STAGING)                                # nfs init script SIS18 RF room
	cp $(ASL)/b2b-sis18-kick.sh $(STAGING)                              # nfs init script SIS18 kicker room
	cp $(ASL)/b2b-esr-rf.sh $(STAGING)                                  # nfs init script ESR
	cp $(ASL)/b2b-esr-kick.sh $(STAGING)                                # nfs init script ESR kicker room
	cp $(ASL)/b2b-tools.sh $(STAGING)                                   # nfs init script tools (UI ...)
	cp $(SW)/b2b-sis18-rf_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin   # FEC init script SIS18
	cp $(SW)/b2b-sis18-kick_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin # FEC init script SIS18
	cp $(SW)/b2b-esr-rf_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin     # FEC init script ESR
	cp $(SW)/b2b-esr-kick_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin   # FEC init script ESR
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(SW)/saft-b2b-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp -a $(SW)/*.so.* $(STAGING)$(ARCH)$(PREFIX)/usr/lib               # '/usr' is a hack
	cp -a $(SW)/*.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib                 # '/usr' is a hack
	cp $(SW)/b2b-ui $(STAGING)$(ARCH)$(PREFIX)/usr/bin                  # '/usr' is a hack
	cp $(SW)/b2bivt/*.txt $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt         # needed for ivtpar
	cp $(FW)/b2bcbu.bin $(STAGING)/firmware                   
	cp $(FW)/b2bpm.bin $(STAGING)/firmware
	cp $(FW)/b2bkd.bin $(STAGING)/firmware

.PHONY: all clean

