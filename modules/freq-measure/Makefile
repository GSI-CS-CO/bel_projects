# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# 'export PKG_CONFIG_PATH=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib/pkgconfig/'
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all' ;                                                               (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/fm-dev deploy'                     (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/fm deploy'                         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
# FW          ?= fw
SW          ?= x86
ASL         ?= asl
INC         ?= include
DIM         ?= /common/usr/timing/b2b/dim_v20r29
#TARGETS     := firmware software

#EXTRA_FLAGS ?=
#CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(FW)
#LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

#all:: firmware software
all:: software

software:: 
	$(MAKE) -C $(SW) all

#firmware:
#	$(MAKE) TARGET=X -C $(FW)

clean:
	$(MAKE) -C $(SW) clean
#	$(MAKE) -C $(FW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
#	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/include                     # '/usr' is a hack
#	mkdir -p $(STAGING)/firmware
#	cp ../../tools/eb-fwload $(STAGING)$(ARCH)$(PREFIX)/usr/bin         # '/usr' is a hack

# NFS init scripts, the format is 'fm-<environment>-<machine>-<room>.sh'
	cp $(ASL)/fm-*.sh $(STAGING)                                        # nfs init scripts

# FEC configuration scripts; the format is 'fm-<environment>-<machine>-<room>_start.sh'
#	cp $(SW)/fm-*.sh $(STAGING)$(ARCH)$(PREFIX)/usr/bin        

# FEC x86 binaries 
	cp $(SW)/freq-measure $(STAGING)$(ARCH)$(PREFIX)/usr/bin            # '/usr' is a hack

# FEC lm32 firmware
#	cp $(FW)/*.bin $(STAGING)/firmware                   

# DAQ
	cp $(DIM)/linux/libdim.so $(STAGING)$(ARCH)$(PREFIX)/usr/lib        # '/usr' is a hack

# FESA @ ASL
#	cp $(INC)/Y.h $(STAGING)$(ARCH)$(PREFIX)/usr/include                # '/usr' is a hack	

.PHONY: all clean

