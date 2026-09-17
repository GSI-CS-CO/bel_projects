# PREFIX controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make clean'                                                                    (!!! this is important !!!)
#   'make MASP=YES ENV=int SYSENV=ACC7 PREFIX= all'                                 (hack: leave PREFIX empty for SCU path)
#   'make MASP=YES ENV=pro SYSENV=ACC7 PREFIX= all'                                 (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/dmunipz-dev deploy' (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/dmunipz deploy'     (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?= fw
SW          ?= x86
SYSTEMD     ?= systemd
NFSINIT     ?= nfs-init
GENNFSINIT  ?= ../../../../ci_cd/scripts/yocto_helper/nfsinit/fec-init
INC         ?= include

# target system environment, OPT is default
SYSENV      ?= OPT

ifeq ($(SYSENV), OPT)
USRPATH     ?= /opt/usr
ARCH        ?=
endif

ifeq ($(SYSENV), YOCTO)
USRPATH     := /common/usr/timing/user-dietrich/yocto/usr
ARCH        := /x86_64
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/user-dietrich/centos7/usr/
ARCH        := /x86_64
endif

# set environment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

export PRO

TARGETS := firmware software nfsinit

all: $(TARGETS)

software: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) TARGET=dmunipz -C $(FW)
	$(MAKE) TARGET=dmunidm -C $(FW)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean
	rm -f $(NFSINIT)/dmunipz-*.sh
	rm -f $(NFSINIT)/dmunidm-*.sh

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
	cp $(FW)/dmunidm.bin $(STAGING)/firmware

# configuration
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

.PHONY: all clean

