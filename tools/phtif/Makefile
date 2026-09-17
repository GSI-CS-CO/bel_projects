# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all'                                                    (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/phtif-dev deploy'     (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/phtif deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= /x86_64
# EB          ?= ../../ip_cores/etherbone-core/api
FW          ?= fw
SW          ?= x86
ASL         ?= asl
#TARGETS     := firmware software

#EXTRA_FLAGS ?=
#CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 
#LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all:: software

software:: 
	$(MAKE) -C $(SW) all

clean:
	$(MAKE) -C $(SW) clean

deploy: 
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/bin                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/usr/lib                         # '/usr' is a hack
	mkdir -p $(STAGING)$(ARCH)$(PREFIX)/tmp/phtifivt                    # needed for ivtpar
	cp $(ASL)/phtif.sh $(STAGING)                                       # nfs init script 
	cp $(SW)/phtif $(STAGING)$(ARCH)$(PREFIX)/usr/bin                   # '/usr' is a hack
	cp $(SW)/phtifivt/*.txt $(STAGING)$(ARCH)$(PREFIX)/tmp/phtifivt     # needed for ivtpar
	cp $(SW)/phtifivt/*.par $(STAGING)$(ARCH)$(PREFIX)/tmp/phtifivt     # needed for ivtpar

.PHONY: all clean

