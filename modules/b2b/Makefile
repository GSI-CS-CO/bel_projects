# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# 'export PKG_CONFIG_PATH=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib/pkgconfig/'
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all' ;                                                               (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-dev deploy'                    (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b deploy'                        (hack: leave PREFIX empty for SCU path)
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
	$(MAKE) TARGET=b2bpmstub -C $(FW)

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

# NFS init scripts, the format is 'b2b-<environment>-<machine>-<room>-<function>.sh'
	cp $(ASL)/b2b-*.sh $(STAGING)                                           # nfs init script SIS18 RF room
# FEC configuration scripts; the format is 'b2b-<environment>-<machine>-<room>-<function>.sh'
	cp $(SW)/b2b-*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin        
# FEC x86 binaries 
	cp $(SW)/b2b-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp -a $(SW)/*.so.* $(STAGING)$(ARCH)$(PREFIX)/usr/lib               # '/usr' is a hack
	cp -a $(SW)/*.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib                 # '/usr' is a hack
	cp $(SW)/b2b-ui $(STAGING)$(ARCH)$(PREFIX)/usr/bin                  # '/usr' is a hack
	cp $(SW)/b2bivt/*.txt $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt         # needed for ivtpar
# FEC lm32 firmware
	cp $(FW)/*.bin $(STAGING)/firmware                   
# DAQ
	cp $(DIM)/linux/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib        # '/usr' is a hack
	cp $(SW)/b2b-serv-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-client-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack 
	cp $(SW)/b2b-serv-raw $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-analyzer $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-viewer $(STAGING)$(ARCH)$(PREFIX)/usr/bin              # '/usr' is a hack 
	cp $(SW)/b2b-archiver $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(SW)/b2b-pname-info $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack; this is special and works only on ASL
# FESA @ ASL
	cp $(INC)/b2blib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include           # '/usr' is a hack	

.PHONY: all clean

