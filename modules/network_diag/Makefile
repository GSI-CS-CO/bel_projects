# PREFIX controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# If using the Yocto SDK, you must additionally use YOCTO=YES  
# Example usage:
#   'make clean'                                                                          (!!! this is important !!!)
#   'make MASP=YES ENV=all YOCTO=YES PREFIX= nfsinit'                                     (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= YOCTO=YES STAGING=/common/export/timing-rte/network-diag-yocto deploy'  (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
SYSTEMD     ?= systemd
NFSINIT     ?= nfs-init
GENNFSINIT  ?= ../../../../ci_cd/scripts/yocto_helper/nfsinit/fec-init

TARGETS := nfsinit

all: $(TARGETS)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	rm -f nfs-init/network-diag*.sh

deploy: 
# create folders
	mkdir -p $(STAGING)/$(SYSTEMD)

# nfsinit scripts
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
# eb-mon is hack until next release
	cp ../../tools/monitoring/eb-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin               # '/usr' is a hack

# configuration


# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

.PHONY: all clean

