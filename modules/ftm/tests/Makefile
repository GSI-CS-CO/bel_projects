# DATAMASTER to test
DATAMASTER ?= $$DM
# Command for saft-ctl snoop. May be different for remote execution.
SNOOP ?= saft-ctl tr0 -xv snoop 0 0 0
# Path for libcarpedm to test
CARPEDM_PATH ?= ../lib/

test:
	TEST_BINARY_DM_CMD=dm-cmd SNOOP_COMMAND="$(SNOOP)" DATAMASTER="$(DATAMASTER)" TEST_SCHEDULES=schedules/ PYTHONPATH=common/ pytest

test1:
	DATAMASTER="$(DATAMASTER)" PYTHONPATH=common/ pytest

all: addDownloadCompare bpcStart dmCmd dmThreads fid7 full_test parallelBranch \
pps safe2remove singleEdgeTest schedules

addDownloadCompare:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C addDownloadCompare/

bpcStart:
	cd bpcStart; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python bpcStart.py "$(DATAMASTER)" "$(SNOOP)"

dmCmd:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python dmpps2.py "$(DATAMASTER)"
	cd dmCmd; PYTHONPATH=../common/ python test_dm_cmd.py ../../ftmx86/dm-cmd "$(DATAMASTER)"

dmPerformance:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmPerformance/ test0
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" SCHEDULE=groups_4_nonDefaultPatterns_9_blocksPerPattern_150 $(MAKE) -C dmPerformance/ test0
	rm dmPerformance/debug.dot

dmThreads:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C dmThreads/ four-threads

fid7:
	cd fid7; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python fid7_test.py "$(DATAMASTER)" "$(SNOOP)"

full_test:
	LD_LIBRARY_PATH=$(CARPEDM_PATH) python full_test/dm_testman.py "$(DATAMASTER)"
	rm download.dot

#messageCounter:
#	LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python messageCounter/messageCounter.py $(DATAMASTER)

parallelBranch:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C parallelBranch/

pps:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common python dmpps2.py "$(DATAMASTER)"

safe2remove:
	SNOOP="$(SNOOP)" DATAMASTER="$(DATAMASTER)" $(MAKE) -C safe2remove/

singleEdgeTest:
	test -d dot/ || mkdir dot/
	$(MAKE) -C singleEdgeTest/
	LD_LIBRARY_PATH=$(CARPEDM_PATH) ./singleEdgeTest/SingleEdgeTest > singleEdgeTest/result.txt
	diff --color singleEdgeTest/result.txt singleEdgeTest/expected-result.txt
	rm -f singleEdgeTest/result.txt
	rm -r dot/

schedules:
	$(MAKE) -C schedules/

schedules1:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear
	dm-sched $(DATAMASTER) add schedules/schedule.dot
	./schedules/startAllPattern.sh

prepare:
	$(MAKE) -C ../ftmx86/ clean
	$(MAKE) -C ../ftmx86/
#	$(MAKE) -C ../ftmx86/ dmpy

.PHONY: addDownloadCompare bpcStart dmCmd dmPerformance dmThreads fid7 full_test messageCounter parallelBranch \
pps safe2remove singleEdgeTest schedules test test1
