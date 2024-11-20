# DATAMASTER to test
DATAMASTER ?= $$DM
# Command for saft-ctl snoop. May be different for remote execution.
SNOOP ?= saft-ctl tr0 -xv snoop 0 0 0
SNOOP_FEL0069 ?= ssh root@fel0069.acc.gsi.de 'saft-ctl tr1 -xv snoop 0 0 0'
# Path for libcarpedm to test
CARPEDM_PATH ?= ../lib/
# DM Tools to test
TEST_DM_CMD ?= ../bin/dm-cmd
TEST_DM_SCHED ?= ../bin/dm-sched
# folders to ignore (no tests inside)
IGNORE = --ignore=dmPerformance/ --ignore=messageCounter/ --ignore=other/ --ignore=schedules/ --ignore=singleEdgeTest/ --ignore=tools/
# pass the command line arguments to pytest
# Example: OPTIONS=--runslow   this runs also the slow tests.
# Example: OPTIONS="-rP"       show stdout for all tests.
# Example: OPTIONS="-k <test name pattern>" run only tests which match the pattern in the test name.
# Example: OPTIONS=" --junitxml=dm-test-result.xml" generate a report on failed or passed tests.
OPTIONS ?=

test:
	TEST_BINARY_DM_CMD="$(TEST_DM_CMD)" TEST_BINARY_DM_SCHED="$(TEST_DM_SCHED)" LD_LIBRARY_PATH="$(CARPEDM_PATH):$(LD_LIBRARY_PATH)" \
		SNOOP_COMMAND="$(SNOOP)" DATAMASTER="$(DATAMASTER)" TEST_SCHEDULES=schedules/ \
		python3 -m pytest $(IGNORE) $(OPTIONS)

remote:
	TEST_BINARY_DM_CMD="$(TEST_DM_CMD)" TEST_BINARY_DM_SCHED="$(TEST_DM_SCHED)" LD_LIBRARY_PATH="$(CARPEDM_PATH):$(LD_LIBRARY_PATH)" \
		SNOOP_COMMAND="$(SNOOP_FEL0069)" DATAMASTER="tcp/fel0069.acc.gsi.de" TEST_SCHEDULES=schedules/ \
		python3 -m pytest --junitxml=dm-test-result.xml $(IGNORE) $(OPTIONS)

test-default:
	DATAMASTER="$(DATAMASTER)" python3 -m pytest $(IGNORE) $(OPTIONS)

test-all:
	TEST_BINARY_DM_CMD="$(TEST_DM_CMD)" TEST_BINARY_DM_SCHED="$(TEST_DM_SCHED)" LD_LIBRARY_PATH="$(CARPEDM_PATH):$(LD_LIBRARY_PATH)" \
		SNOOP_COMMAND="$(SNOOP)" DATAMASTER="$(DATAMASTER)" TEST_SCHEDULES=schedules/ \
		python3 -m pytest $(IGNORE) --runslow $(OPTIONS)

test-32-threads:
	TEST_BINARY_DM_CMD="$(TEST_DM_CMD)" TEST_BINARY_DM_SCHED="$(TEST_DM_SCHED)" LD_LIBRARY_PATH="$(CARPEDM_PATH):$(LD_LIBRARY_PATH)" \
		SNOOP_COMMAND="$(SNOOP)" DATAMASTER="$(DATAMASTER)" TEST_SCHEDULES=schedules/ \
		python3 -m pytest $(IGNORE) --runslow --runthread32 $(OPTIONS)

all: test-all dmPerformance dmThreads pps singleEdgeTest

dmPerformance:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmPerformance/ test0
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" SCHEDULE=groups_4_nonDefaultPatterns_9_blocksPerPattern_150 $(MAKE) -C dmPerformance/ test0
	rm dmPerformance/debug.dot

dmThreads:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmThreads/ four-threads

pps:
	DATAMASTER="$(DATAMASTER)" python3 -m pytest -k pps $(IGNORE) $(OPTIONS)

singleEdgeTest:
	test -d dot/ || mkdir dot/
	$(MAKE) -C singleEdgeTest/
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(CARPEDM_PATH) ./singleEdgeTest/singleEdgeTest > singleEdgeTest/result.txt
	diff --color singleEdgeTest/result.txt singleEdgeTest/expected-result.txt
	rm -f singleEdgeTest/result.txt
	rm -r dot/

prepare:
	$(MAKE) -C ../ftmx86/ clean
	$(MAKE) -C ../ftmx86/
	$(MAKE) -C ../analysis/scheduleCompare/main/ clean
	$(MAKE) -C ../analysis/scheduleCompare/main/
	$(MAKE) -C singleEdgeTest/ clean
	$(MAKE) -C singleEdgeTest/

prepare-all:
	$(MAKE) -C ../ftmx86/ clean
	$(MAKE) -C ../ftmx86/
	$(MAKE) -C ../ftmx86/ install
	$(MAKE) -C ../analysis/scheduleCompare/main/ clean
	$(MAKE) -C ../analysis/scheduleCompare/main/
	$(MAKE) -C ../analysis/scheduleCompare/main/ install
	$(MAKE) -C singleEdgeTest/ clean
	$(MAKE) -C singleEdgeTest/
	$(MAKE) -C singleEdgeTest/ install

checkEnvironment:
	./tools/getBoostVersion.sh
	gcc --version
	python --version
	python3 -m pytest -VV

.PHONY: dmPerformance dmThreads pps remote singleEdgeTest test test-default remote test-all prepare prepare-all checkEnvironment
