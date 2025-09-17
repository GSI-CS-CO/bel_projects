# PREFIX controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make clean'                                                                      (!!! this is important !!!)
#   'make ENV=all PREFIX= SYSENV=ACC7 nfsinit'                                        (hack: leave PREFIX empty for SCU path)
# Example deploy (requires binary of eb-mon in ../../tools/monitoring)
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/network-diag deploy'  (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
SYSTEMD     ?= systemd
NFSINIT     ?= nfs-init
GENNFSINIT  ?= ../../../../ci_cd/scripts/yocto_helper/nfsinit/fec-init

# target system environment, OPT is default                                                                                                              
SYSENV      ?= OPT

ifeq ($(SYSENV), OPT)
USRPATH     ?= /opt/usr
ARCH        ?=
endif

ifeq ($(SYSENV), YOCTO)
USRPATH     := /common/usr/timing/b2b/yocto/usr
ARCH        := /x86_64
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/b2b/centos7/usr/
ARCH        := /x86_64
endif

# set environment, default is int                                                                                                                        
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

TARGETS := nfsinit

all: $(TARGETS)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	rm -f nfs-init/network-diag*.sh

deploy: 
# create folders
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                             # '/usr' is a hack

# nfsinit scripts
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
# eb-mon is hack until next release
	cp ../../tools/monitoring/eb-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin     # '/usr' is a hack

# configuration


# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                           # systemd units

.PHONY: all clean

