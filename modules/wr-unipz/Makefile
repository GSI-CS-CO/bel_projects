# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make clean'                                                                       (!!! this is important !!!)
#   'make MASP=NO PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
#   'make MASP=NO PRO=YES PREFIX= all'                                                 (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=NO PRO=NO PREFIX= STAGING=/common/export/timing-rte/wrunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
#   'make MASP=NO PRO=YES PREFIX= STAGING=/common/export/timing-rte/wrunipz deploy'    (hack: leave PREFIX empty for SCU path)
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
	$(MAKE) -C $(FW)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin     # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib     # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include # '/usr' is a hack
	mkdir -p $(STAGING)/firmware
	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(ASL)/wrunipz.sh $(STAGING)
	cp $(SW)/wr-unipz_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(SW)/wrunipz-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(FW)/wrunipz.bin $(STAGING)/firmware
	cp -a $(SW)/libwrunipzlib.so* $(STAGING)$(ARCH)$(PREFIX)/usr/lib # '/usr' is a hack
	cp $(SW)/../include/wrunipzlib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include

#	script /common/export/nfsinit/global/timing-rte-wrunipz-dev is _here_ in sub-folder wrunipz-asl 

.PHONY: all clean

