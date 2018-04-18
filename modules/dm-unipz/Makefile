# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make MASP=YES PRO=NO PREFIX= all                                                   (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/dmunipz-dev deploy (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= dmunipz-fw
SW          ?= dmunipz-x86
ASL         ?= dmunipz-asl
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
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	mkdir -p $(STAGING)/firmware
	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(ASL)/dmunipz.sh $(STAGING)
	cp $(SW)/dm-unipz_start.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(SW)/dmunipz-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin # '/usr' is a hack
	cp $(FW)/dmunipz.bin $(STAGING)/firmware
#	script /common/export/nfsinit/global/timing-rte-dmunipz-dev is _here_ in sub-folder dmunipz-asl 

.PHONY: all clean

