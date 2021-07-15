# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# export PKG_CONFIG_PATH=/common/export/timing-rte/tg-fallout-v6.0.1/x86_64/lib/pkgconfig/
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all' ;                          don't send kick trigger evts on MIL  (hack: leave PREFIX empty for SCU path)
#   'make EXTRA_FLAGS='-D USEMIL' PREFIX= all' ;  send kick trigger evts on MIL        (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b deploy'                        (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-dev deploy'                    (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= fw
SW          ?= x86
ASL         ?= asl
INC         ?= include
DIM         ?= /common/usr/timing/b2b/dim_v20r29
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
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include                     # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt                      # needed for ivtpar
	mkdir -p $(STAGING)/firmware
#	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin         # '/usr' is a hack
# NFS init scripts
	cp $(ASL)/b2b-sis18-rf.sh $(STAGING)                                # nfs init script SIS18 RF room
	cp $(ASL)/b2b-sis18-rf-div.sh $(STAGING)                            # nfs init script SIS18 RF room, other sutff
	cp $(ASL)/b2b-sis18-kick.sh $(STAGING)                              # nfs init script SIS18 kicker room
	cp $(ASL)/b2b-sis18-kick-div.sh $(STAGING)                          # nfs init script SIS18 kicker room, other stuff
	cp $(ASL)/b2b-sis18-daq.sh $(STAGING)                               # nfs init script SIS18 kicker room, other stuff
	cp $(ASL)/b2b-esr-rf.sh $(STAGING)                                  # nfs init script ESR RF room
	cp $(ASL)/b2b-esr-rf-div.sh $(STAGING)                              # nfs init script ESR RF room, other stuff
	cp $(ASL)/b2b-esr-kick.sh $(STAGING)                                # nfs init script ESR kicker room
	cp $(ASL)/b2b-esr-kick-div.sh $(STAGING)                            # nfs init script ESR kicker room, other stufff
	cp $(ASL)/b2b-esr-daq.sh $(STAGING)                                 # nfs init script SIS18 kicker room, other stuff
	cp $(ASL)/b2b-sis18-bg2.sh $(STAGING)                               # nfs init script SIS18 BG2 electronic room (for INT)
	cp $(ASL)/b2b-tools.sh $(STAGING)                                   # nfs init script tools (UI ...)
# FEC configuration scripts
	cp $(SW)/b2b-sis18-rf_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin   # FEC init script SIS18 RF room
	cp $(SW)/b2b-sis18-kick_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin # FEC init script SIS18 RF room
	cp $(SW)/b2b-esr-rf_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin     # FEC init script ESR RF room
	cp $(SW)/b2b-esr-kick_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin   # FEC init script ESR RF foom
	cp $(SW)/b2b-sis18-bg2_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin  # FEC init script SIS18 BG2 electronic room
# FEC x86 binaries 
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(SW)/saft-b2b-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp -a $(SW)/*.so.* $(STAGING)$(ARCH)$(PREFIX)/usr/lib               # '/usr' is a hack
	cp -a $(SW)/*.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib                 # '/usr' is a hack
	cp $(SW)/b2b-ui $(STAGING)$(ARCH)$(PREFIX)/usr/bin                  # '/usr' is a hack
	cp $(SW)/b2bivt/*.txt $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt         # needed for ivtpar
# FEC lm32 firmware
	cp $(FW)/b2bcbu.bin $(STAGING)/firmware                   
	cp $(FW)/b2bpm.bin $(STAGING)/firmware
	cp $(FW)/b2bkd.bin $(STAGING)/firmware
# DAQ
	cp $(DIM)/linux/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib        # '/usr' is a hack
	cp $(SW)/b2b-serv-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-client-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack 
	cp $(SW)/b2b-serv-raw $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-analyzer $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-viewer $(STAGING)$(ARCH)$(PREFIX)/usr/bin              # '/usr' is a hack 
	cp $(SW)/b2b-archiver $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack 
# FESA @ ASL
	cp $(INC)/b2blib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include           # '/usr' is a hack	

.PHONY: all clean

