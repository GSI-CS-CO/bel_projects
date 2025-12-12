# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example build:
#   'make clean'                                                 
#   'make ENV=int SYSENV=YOCTO DIM=YES PREFIX= all'                                             (hack: leave PREFIX empty for SCU path)
# Example deploy (for ASL only):
#   'make PREFIX= SYSENV=YOCTO STAGING=/common/export/timing-rte/ftdimcp-dev-yocto deploy'      (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= SYSENV=YOCTO STAGING=/common/export/timing-rte/ftdimcp-yocto deploy'          (hack: leave PREFIX empty for SCU path)

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
USRPATH     := /common/usr/timing/user-dietrich/centos7/usr
ARCH        := /x86_64
endif

# set environment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

TARGETS := software nfsinit

all: $(TARGETS)

software:: 
ifeq ($(SYSENV), YOCTO)
# I really hate this hack, but I found no simpler solution
	bash x86/make-yocto-software-all
else
	$(MAKE) -C $(SW) all
endif 

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	rm -f nfs-init/ftdimcp-int*.sh
	rm -f nfs-init/ftdimcp-pro*.sh
	rm -f nfs-init/ftdimcp-tools.sh

deploy:
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)

# nfsinit scripts, the format is 'ftdimcp-<environment>-<machine>-<room>-<function>.sh'
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
	cp $(SW)/ftdimcp-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack
	cp $(SW)/ftdimcp-ui $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack
	cp $(USRPATH)/lib/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib             # '/usr' is a hack
	cp $(USRPATH)/lib/libftd2xx.so.1.4.27 $(STAGING)$(ARCH)$(PREFIX)/usr/lib   # '/usr' is a hack
	cp -a $(USRPATH)/lib/libftd2xx.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib       # '/usr' is a hack
	cp $(USRPATH)/lib/libmpsse.so.1.0.3 $(STAGING)$(ARCH)$(PREFIX)/usr/lib     # '/usr' is a hack
	cp -a $(USRPATH)/lib/libmpsse.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib        # '/usr' is a hack

# configuration; the format is 'ftdimcp-<environment>-<machine>-<room>-<function>_start.sh'
#	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

.PHONY: all clean

