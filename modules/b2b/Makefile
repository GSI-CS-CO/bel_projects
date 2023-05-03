# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Set SYSENV to the target system environment (YOCTO, ACC7, ACC9, OPT)
# Example usage:
#   'make clean'                                                 
#   'make ENV=int SYSENV=OPT PREFIX= all'                                              (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= YOCTO=YES STAGING=/common/export/timing-rte/b2b-dev-yocto deploy'    (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-dev deploy'                    (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b deploy'                        (hack: leave PREFIX empty for SCU path)


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
ARCH        ?= /x86_64
endif

ifeq ($(SYSENV), YOCTO)
USRPATH     := /common/usr/timing/b2b/yocto/usr
ARCH        := /x86_64
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

software:: 
	$(MAKE) -C $(SW) all

firmware:
	$(MAKE) TARGET=b2bcbu -C $(FW)
	$(MAKE) TARGET=b2bpm -C $(FW)
	$(MAKE) TARGET=b2bkd -C $(FW)
	$(MAKE) TARGET=b2bpmstub -C $(FW)

nfsinit:
	echo $(shell cd $(NFSINIT); $(GENNFSINIT)/generate-main.sh $(ENV); cd ..)

clean:
	$(MAKE) -C $(SW) clean
	$(MAKE) -C $(FW) clean

deploy:
# create folders
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include                     # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt                      # needed for ivtpar
	mkdir -p $(STAGING)/$(SYSTEMD)
	mkdir -p $(STAGING)/firmware

# nfsinit scripts, the format is 'b2b-<environment>-<machine>-<room>-<function>.sh'
	cp $(NFSINIT)/*.sh $(STAGING)

# tools
	cp $(SW)/b2b-mon $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp $(SW)/b2b-ctl $(STAGING)$(ARCH)$(PREFIX)/usr/bin                 # '/usr' is a hack
	cp -a $(SW)/*.so.* $(STAGING)$(ARCH)$(PREFIX)/usr/lib               # '/usr' is a hack
	cp -a $(SW)/*.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib                 # '/usr' is a hack
	cp $(SW)/b2b-ui $(STAGING)$(ARCH)$(PREFIX)/usr/bin                  # '/usr' is a hack
	cp $(SW)/b2bivt/*.txt $(STAGING)$(ARCH)$(PREFIX)/tmp/b2bivt         # needed for ivtpar
	cp $(USRPATH)/lib/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib      # '/usr' is a hack
	cp $(SW)/b2b-serv-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-client-sys $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack 
	cp $(SW)/b2b-serv-raw $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-analyzer $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack 
	cp $(SW)/b2b-viewer $(STAGING)$(ARCH)$(PREFIX)/usr/bin              # '/usr' is a hack 
	cp $(SW)/b2b-archiver $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack
	cp $(FW)/*.bin $(STAGING)/firmware                   
#	cp $(SW)/b2b-pname-info $(STAGING)$(ARCH)$(PREFIX)/usr/bin          # '/usr' is a hack; this is special and works only on ASL

# configuration; the format is 'b2b-<environment>-<machine>-<room>-<function>_start.sh'
	cp $(SW)/*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin                    # '/usr' is a hack

# systemd
	cp $(SYSTEMD)/*.service $(STAGING)/$(SYSTEMD)                       # systemd units

# header files, required for building FESA
	cp $(INC)/b2blib.h $(STAGING)$(ARCH)$(PREFIX)/usr/include           # '/usr' is a hack	

.PHONY: all clean

