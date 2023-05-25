# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make clean'                                                                       (!!! this is important !!!)
#   'make ENV=int SYSENV=ACC7 PREFIX= all'                                             (hack: leave PREFIX empty for SCU path)
#   'make ENV=pro SYSENV=ACC7 PREFIX= all'                                             (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make SYSENV=ACC7 PREFIX= STAGING=/common/export/timing-rte/wrunipz-dev deploy'    (hack: leave PREFIX empty for SCU path)
#   'make SYSENV=ACC7 PREFIX= STAGING=/common/export/timing-rte/wrunipz deploy'        (hack: leave PREFIX empty for SCU path)
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
USRPATH     := /common/usr/timing/b2b/yocto/usr
ARCH        := /x86_64
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/b2b/centos7/usr/
ARCH        := /x86_64
endif

# set environment, default is int
ENV         ?= int

all:: firmware software nfsinit

software:: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) -C $(FW)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean
	rm -f $(NFSINIT)/wrunipz-*.sh

deploy: 
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include                     # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)/firmware

# nfsinit scripts
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
	cp $(SW)/wrunipz-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin             # '/usr' is a hack
	cp $(FW)/wrunipz.bin $(STAGING)/firmware
	cp -a $(SW)/libwrunipzlib.so* $(STAGING)$(ARCH)$(PREFIX)/usr/lib    # '/usr' is a hack

# configuration
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

# header files, required for building Device Access
	cp $(INC)/wrunipzlib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include       # '/usr' is a hack

.PHONY: all clean

