# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
#   'make MASP=YES PRO=YES PREFIX= all'                                                 (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/dmunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
#   'make MASP=YES PRO=YES PREFIX= STAGING=/common/export/timing-rte/dmunipz deploy'    (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= b2btest-fw
SW          ?= b2btest-x86
ASL         ?= b2btest-asl
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
	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(ASL)/b2btest.sh $(STAGING)                               # nfs init script
	cp $(SW)/b2btest_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin  # FEC init script
	cp tr-cbu.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin               # init stuff for cbu
	cp tr-pm.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                # init stuff for pm
	cp $(SW)/b2btest-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin       # '/usr' is a hack
	cp $(FW)/b2bcbu.bin $(STAGING)/firmware                   
	cp $(FW)/b2bpm.bin $(STAGING)/firmware
#	script /common/export/nfsinit/global/timing-rte-dmunipz-dev is _here_ in sub-folder dmunipz-asl 

.PHONY: all clean

