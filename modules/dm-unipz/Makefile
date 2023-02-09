# PREFIX controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# If using the Yocto SDK, you must additionally use YOCTO=YES  
# Example usage:
#   'make clean'                                                                        (!!! this is important !!!)
#   'make MASP=YES PRO=NO YOCTO=YES PREFIX= all'                                        (hack: leave PREFIX empty for SCU path)
#   'make MASP=YES PRO=YES YOCTO=YES PREFIX= all'                                       (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/dmunipz-dev-yocto deploy'           (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/dmunipz deploy'                     (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths
FW          ?= fw
SW          ?= x86
ASL         ?= asl
SYSTEMD     ?= systemd

# support Yocto SDK
ifeq ($(YOCTO), YES)
EB          ?= .
ARCH        := /x86_64
else
EB          ?= ../../ip_cores/etherbone-core/api
ARCH        ?= /x86_64
endif

TARGETS := firmware software

all:: $(TARGETS)

software:: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) -C $(FW)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy: 
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)/firmware

# nfsinit scripts
	cp $(ASL)/*.sh $(STAGING)

# tools
	cp $(SW)/dmunipz-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin             # '/usr' is a hack
	cp $(FW)/dmunipz.bin $(STAGING)/firmware

# configuration
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

.PHONY: all clean

