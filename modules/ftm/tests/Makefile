# DATAMASTER to test
DATAMASTER ?= $$DM
# Path for libcarpedm to test and path to python wrapper for libcarpedm
CARPEDM_PATH ?= ~/bel_projects/test/modules/ftm/lib/
CARPEDM_WRAPPER ?= ~/bel_projects/test/modules/ftm/ftmx86/
CARPEDM_PATH_1 ?= ../lib/
#CARPEDM_WRAPPER ?= ../ftmx86/

all: bpcStart dmPerformance dmThreads fid7 full_test messageCounter pps schedules singleEdgeTest unittest

bpcStart:
	cd bpcStart; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER):../common/ python bpcStart.py $(DATAMASTER)

dmPerformance:
	$(MAKE) -C dmPerformance/ test0
	SCHEDULE=groups_4_nonDefaultPatterns_9_blocksPerPattern_150 $(MAKE) -C dmPerformance/ test0

dmThreads:
	$(MAKE) -C dmThreads/ four-threads

fid7:
	cd fid7; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER):../common/ python fid7_test.py $(DATAMASTER)

full_test:
	LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python full_test/dm_testman.py $(DATAMASTER)
	rm download.dot

messageCounter:
	LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python messageCounter/messageCounter.py $(DATAMASTER)

pps:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python dmpps2.py $(DATAMASTER)

schedules:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear
	dm-sched $(DATAMASTER) add schedules/schedule.dot
	./schedules/startAllPattern.sh

singleEdgeTest:
	test -d dot/ || mkdir dot/
	$(MAKE) -C singleEdgeTest/
	LD_LIBRARY_PATH=$(CARPEDM_PATH_1) ./singleEdgeTest/SingleEdgeTest > singleEdgeTest/result.txt
	diff --color singleEdgeTest/result.txt singleEdgeTest/expected-result.txt
	rm -f singleEdgeTest/result.txt

unittest:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python dmpps2.py $(DATAMASTER)
	python unittests/test_dm_cmd.py ../ftmx86/dm-cmd $(DATAMASTER)

prepare:
	$(MAKE) -C ../ftmx86/ clean
	$(MAKE) -C ../ftmx86/
#	$(MAKE) -C ../ftmx86/ dmpy

.PHONY: bpcStart dmPerformance dmThreads fid7 full_test messageCounter pps schedules singleEdgeTest unittest

