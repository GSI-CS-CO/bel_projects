# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example build:
#   'make clean'                                                 
#   'make MASP=NO ENV=int SYSENV=ACC7 PREFIX= all'                                    (hack: leave PREFIX empty for SCU path)
# Example deploy (for ASL only):
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/wrmil-dev deploy'      (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/wrmil deploy'          (hack: leave PREFIX empty for SCU path)

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
USRPATH     := /common/usr/timing/wrmil/yocto/usr
ARCH        := /x86_64
endif

ifeq ($(SYSENV), ACC7)
USRPATH     := /common/usr/timing/wrmil/centos7/usr/
ARCH        := /x86_64
endif

# set environment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

TARGETS := firmware software nfsinit

all: $(TARGETS)

software:: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) TARGET=wrmil -C $(FW)
	$(MAKE) TARGET=wrf50 -C $(FW)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean
	rm -f nfs-init/wrmil-int*.sh
	rm -f nfs-init/wrmil-pro*.sh
	rm -f nfs-init/wrmil-tools.sh

deploy:
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include                     # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)/firmware

# nfsinit scripts, the format is 'wrmil-<environment>-<machine>-<room>-<function>.sh'
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
	cp $(SW)/wrmil-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin               # '/usr' is a hack
	cp $(SW)/wrf50-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin               # '/usr' is a hack
	cp -a $(SW)/*.so.* $(STAGING)$(ARCH)$(PREFIX)/usr/lib               # '/usr' is a hack
	cp -a $(SW)/*.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib                 # '/usr' is a hack
	cp $(USRPATH)/lib/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib      # '/usr' is a hack
	cp $(SW)/wrmil-serv-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack
	cp $(SW)/wrmil-client-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin        # '/usr' is a hack
	cp $(SW)/wrf50-serv-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack
	cp $(FW)/*.bin $(STAGING)/firmware                   

# configuration; the format is 'wrmil-<environment>-<machine>-<room>-<function>_start.sh'
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

# header files, required for building FESA
	cp $(INC)/wrmillib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include         # '/usr' is a hack	

.PHONY: all clean

