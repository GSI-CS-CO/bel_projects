
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
SCU_DAQT = ./scu-daq

.PHOMY: all
all:
	$(MAKE) -C $(SCU_DAQT) all

.PHOMY: clean
clean:
	$(MAKE) -C $(SCU_DAQT) clean

.PHONY: lload
lload:
	$(MAKE) -C $(SCU_DAQT) load

.PHOMY: run
run:
	$(MAKE) -C $(SCU_DAQT) run

.PHONY: dbg
dbg:
	$(MAKE) -C $(SCU_DAQT) dbg