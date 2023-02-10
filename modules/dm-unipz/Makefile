# PREFIX controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# If using the Yocto SDK, you must additionally use YOCTO=YES  
# Example usage:
#   'make clean'                                                                        (!!! this is important !!!)
#   'make MASP=YES ENV=int YOCTO=YES PREFIX= all'                                       (hack: leave PREFIX empty for SCU path)
#   'make MASP=YES ENV=pro YOCTO=YES PREFIX= all'                                       (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= YOCTO=YES STAGING=/common/export/timing-rte/dmunipz-dev-yocto deploy' (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= YOCTO=YES STAGING=/common/export/timing-rte/dmunipz-yocto deploy'     (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?= fw
SW          ?= x86
SYSTEMD     ?= systemd
NFSINIT     ?= nfs-init
GENNFSINIT  ?= ../../../../ci_cd/scripts/yocto_helper/nfsinit/fec-init

# support Yocto SDK
ifeq ($(YOCTO), YES)
EB          ?= .
ARCH        := /x86_64
else
EB          ?= ../../ip_cores/etherbone-core/api
ARCH        ?= /x86_64
endif

# set enviorinment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

TARGETS := firmware software nfsinit

all: $(TARGETS)

software: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) -C $(FW)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy: 
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)/firmware

# nfsinit scripts
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
	cp $(SW)/dmunipz-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin             # '/usr' is a hack
	cp $(FW)/dmunipz.bin $(STAGING)/firmware

# configuration
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

.PHONY: all clean

