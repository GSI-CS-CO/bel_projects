# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example build:
#   'make clean'                                                 
#   'make ENV=int SYSENV=ACC7 PREFIX= all'                                             (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/fm-dev deploy'         (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= SYSENV=ACC7 STAGING=/common/export/timing-rte/fm deploy'             (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
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


# set envirornment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

# hack for building on local linux box
#DIMPATH     := /opt/dim/dim_v20r29

TARGETS := software nfsinit

all: $(TARGETS)

software:: 
	$(MAKE) -C $(SW) all

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)


clean:
	$(MAKE) -C $(SW) clean

deploy:
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)

# NFS init scripts, the format is 'fm-<environment>-<machine>-<room>.sh'
	cp $(NFSINIT)/*.sh $(STAGING)                                       # nfs init scripts

# tools
	cp $(SW)/freq-measure $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp $(SW)/freq-mon-simple $(STAGING)$(ARCH)$(PREFIX)/usr/bin         # '/usr' is a hack

# configuration; the format is 'fm-<environment>-<machine>-<room>-<function>_start.sh'
#	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units


.PHONY: all clean

