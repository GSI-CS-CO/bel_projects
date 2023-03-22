# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# Example usage:
#   'make clean'                                                 
#   'make ENV=int YOCTO=YES PREFIX= all'                                               (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/fm-dev-yocto deploy'               (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/fm deploy'                         (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
SW          ?= x86
SYSTEMD     ?= systemd
NFSINIT     ?= nfs-init
GENNFSINIT  ?= ../../../../ci_cd/scripts/yocto_helper/nfsinit/fec-init
INC         ?= include

# support Yocto SDK
ifeq ($(YOCTO), YES)
EB          ?= .
ARCH        := /x86_64
DIMPATH      := /common/usr/timing/b2b/yocto/dim_v20r33
else
EB          ?= ../../ip_cores/etherbone-core/api
ARCH        ?= /x86_64
DIMPATH      := /common/usr/timing/b2b/dim_v20r29
endif

# set enviorinment, default is int
ENV         ?= int
ifeq ($(ENV), pro)
PRO         ?= YES
else
PRO         ?= NO
endif

# hack for building on local linux box
#DIMPATH     := /opt/dim/dim_v20r29

TARGETS := software

all: $(TARGETS)

software:: 
	$(MAKE) -C $(SW) all


clean:
	$(MAKE) -C $(SW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack

# NFS init scripts, the format is 'fm-<environment>-<machine>-<room>.sh'
	cp $(ASL)/fm-*.sh $(STAGING)                                        # nfs init scripts

# FEC x86 binaries 
	cp $(SW)/freq-measure $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack

.PHONY: all clean

