# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make PREFIX= all'                                                    (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-sis18 deploy' (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-esr deploy'   (hack: leave PREFIX empty for SCU path)
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

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	mkdir -p $(STAGING)/firmware
	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin         # '/usr' is a hack
	cp $(ASL)/b2b-sis18.sh $(STAGING)                                   # nfs init script SIS18
	cp $(ASL)/b2b-esr.sh $(STAGING)                                     # nfs init script ESR
	cp $(SW)/b2b-sis18_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin      # FEC init script SIS18
	cp $(SW)/b2b-esr_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin        # FEC init script ESR
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(FW)/b2bcbu.bin $(STAGING)/firmware                   
	cp $(FW)/b2bpm.bin $(STAGING)/firmware

.PHONY: all clean

