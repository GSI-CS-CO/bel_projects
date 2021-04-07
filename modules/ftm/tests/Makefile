# DATAMASTER to test
DATAMASTER ?= $$DM
# Path for libcarpedm to test
CARPEDM_PATH ?= ../lib/

all: addDownloadCompare bpcStart dmCmd dmThreads fid7 full_test parallelBranch \
pps safe2remove singleEdgeTest schedules

addDownloadCompare:
	$(MAKE) -C addDownloadCompare/

bpcStart:
	cd bpcStart; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python bpcStart.py $(DATAMASTER)

dmCmd:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python dmpps2.py $(DATAMASTER)
	cd dmCmd; PYTHONPATH=../common/ python test_dm_cmd.py ../../ftmx86/dm-cmd $(DATAMASTER)

dmPerformance:
	$(MAKE) -C dmPerformance/ test0
	SCHEDULE=groups_4_nonDefaultPatterns_9_blocksPerPattern_150 $(MAKE) -C dmPerformance/ test0
	rm dmPerformance/debug.dot

dmThreads:
	$(MAKE) -C dmThreads/ four-threads

fid7:
	cd fid7; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common/ python fid7_test.py $(DATAMASTER)

full_test:
	LD_LIBRARY_PATH=$(CARPEDM_PATH) python full_test/dm_testman.py $(DATAMASTER)
	rm download.dot

#messageCounter:
#	LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=$(CARPEDM_WRAPPER) python messageCounter/messageCounter.py $(DATAMASTER)

parallelBranch:
	$(MAKE) -C parallelBranch/

pps:
	cd pps; LD_LIBRARY_PATH=$(CARPEDM_PATH) PYTHONPATH=../common python dmpps2.py $(DATAMASTER)

safe2remove:
	$(MAKE) -C safe2remove/

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
pps safe2remove singleEdgeTest schedules

