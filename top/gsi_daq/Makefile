
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
LINUX_TEST_DIR = $(REPOSITORY_DIR)/modules/lm32-test/daq_app_test

.PHOMY: all
all:
	$(MAKE) -C ./buildLm32  all
	$(MAKE) -C $(LINUX_TEST_DIR) all
	$(MAKE) -C ./tools           all

.PHOMY: clean
clean:
	$(MAKE) -C ./buildLm32  clean
	$(MAKE) -C $(LINUX_TEST_DIR) clean
	$(MAKE) -C ./tools clean

.PHOMY: load
load:
	$(MAKE) -C ./buildLm32  load

.PHONY: lload
lload:
	$(MAKE) -C $(LINUX_TEST_DIR) load

.PHOMY: run
run:
	$(MAKE) -C $(LINUX_TEST_DIR) run

.PHONY: dbg
dbg:
	$(MAKE) -C $(LINUX_TEST_DIR) dbg