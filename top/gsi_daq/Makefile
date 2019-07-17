
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
LINUX_TEST_DIR = $(REPOSITORY_DIR)/modules/lm32-test/daq_app_test

.PHOMY: all
all:
	$(MAKE) -C ./lm32  all
	$(MAKE) -C $(LINUX_TEST_DIR) all
	$(MAKE) -C ./tools/scu-daq           all

.PHOMY: clean
clean:
	$(MAKE) -C $(LINUX_TEST_DIR) clean
	$(MAKE) -C ./tools/scu-daq clean
	$(MAKE) -C ./lm32  clean

.PHOMY: load
load:
	$(MAKE) -C ./lm32  load

.PHONY: lload
lload:
	$(MAKE) -C $(LINUX_TEST_DIR) load

.PHOMY: run
run:
	$(MAKE) -C $(LINUX_TEST_DIR) run

.PHONY: dbg
dbg:
	$(MAKE) -C $(LINUX_TEST_DIR) dbg