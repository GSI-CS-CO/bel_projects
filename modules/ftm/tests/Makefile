# DATAMASTER to test
DATAMASTER ?= $$DM
# Command for saft-ctl snoop. May be different for remote execution.
SNOOP ?= saft-ctl tr0 -xv snoop 0 0 0
SNOOP_FEL0069 ?= ssh -t root@fel0069.acc.gsi.de 'saft-ctl tr1 -xv snoop 0 0 0'
# Path for libcarpedm to test
CARPEDM_PATH ?= ../lib/
# folders to ignore (no tests inside)
IGNORE = --ignore=addDownloadCompare/ --ignore=blockAlign/ --ignore=dmPerformance/ --ignore=dmThreads/ --ignore=full_test/ --ignore=schedules/
# pass the command line arguments to pytest
OPTIONS ?=

test:
	TEST_BINARY_DM_CMD=dm-cmd SNOOP_COMMAND="$(SNOOP)" DATAMASTER="$(DATAMASTER)" TEST_SCHEDULES=schedules/ pytest $(IGNORE) $(OPTIONS)

remote:
	TEST_BINARY_DM_CMD=dm-cmd SNOOP_COMMAND="$(SNOOP_FEL0069)" DATAMASTER="tcp/fel0069.acc.gsi.de" TEST_SCHEDULES=schedules/ pytest $(IGNORE) $(OPTIONS)

test-default:
	DATAMASTER="$(DATAMASTER)" pytest $(IGNORE) $(OPTIONS)

all: dmPerformance dmThreads full_test pps singleEdgeTest

dmPerformance:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmPerformance/ test0
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" SCHEDULE=groups_4_nonDefaultPatterns_9_blocksPerPattern_150 $(MAKE) -C dmPerformance/ test0
	rm dmPerformance/debug.dot

dmThreads:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmThreads/ four-threads

full_test:
	LD_LIBRARY_PATH=$(CARPEDM_PATH) python full_test/dm_testman.py "$(DATAMASTER)"
	rm download.dot

#messageCounter:
#	LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python messageCounter/messageCounter.py $(DATAMASTER)

pps:
	DATAMASTER="$(DATAMASTER)" pytest -k pps $(IGNORE) $(OPTIONS)

singleEdgeTest:
	test -d dot/ || mkdir dot/
	$(MAKE) -C singleEdgeTest/
	LD_LIBRARY_PATH=$(CARPEDM_PATH) ./singleEdgeTest/SingleEdgeTest > singleEdgeTest/result.txt
	diff --color singleEdgeTest/result.txt singleEdgeTest/expected-result.txt
	rm -f singleEdgeTest/result.txt
	rm -r dot/

prepare:
	$(MAKE) -C ../ftmx86/ clean
	$(MAKE) -C ../ftmx86/
#	$(MAKE) -C ../ftmx86/ dmpy

.PHONY: dmPerformance dmThreads full_test messageCounter pps singleEdgeTest schedules test test-default
