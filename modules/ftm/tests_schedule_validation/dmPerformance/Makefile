SCHEDULE ?= groups_1_nonDefaultPatterns_1_blocksPerPattern_1
DATAMASTER ?=tcp/dal029.acc

PHONY: test0 test1 test2 test2x testx test3 coverage

test0:
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) clear
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) add $(SCHEDULE)_globalSchedule.dot
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) chkrem G1_P1
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) remove $(SCHEDULE)_removeSchedule.dot
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) add g1_p1_update_schedule.dot
	date --rfc-3339=ns

test1:
	date --rfc-3339=ns
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot
	date --rfc-3339=ns

test2:
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot

test2x:
	touch upload.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) status -vo step-before.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	mv upload.dot upload-add.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) status -vo step-add.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	mv debug.dot debug-check.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) status -vo step-check.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) status -vo step-remove.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot
	if test -f "upload.dot" ; then mv upload.dot upload-update.dot; fi
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) status -vo step-update.dot

testx:
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	LD_LIBRARY_PATH=$(PATH_DM1)/../lib $(PATH_DM1)/ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot

test3:
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	valgrind --tool=callgrind LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	valgrind --tool=callgrind LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	valgrind --tool=callgrind LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	valgrind --tool=callgrind LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot

coverage:
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v halt
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v clear
	lcov --zerocounters --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add $(SCHEDULE)_globalSchedule.dot
	lcov --capture --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86 --output-file add_globalSchedule.info
	genhtml add_globalSchedule.info --output-directory coverage_add/
	lcov --zerocounters --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-cmd $(DATAMASTER) -v chkrem G1_P1
	lcov --capture --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86 --output-file checkremove_Schedule.info
	genhtml checkremove_Schedule.info --output-directory coverage_checkremove/
	lcov --zerocounters --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v remove $(SCHEDULE)_removeSchedule.dot
	lcov --capture --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86 --output-file remove_Schedule.info
	genhtml remove_Schedule.info --output-directory coverage_remove/
	lcov --zerocounters --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86
	LD_LIBRARY_PATH=../../lib ../../ftmx86/dm-sched $(DATAMASTER) -v add g1_p1_update_schedule.dot
	lcov --capture --directory . --directory ~/dev/bel_projects/dm-enigma-merge-v3/modules/ftm/ftmx86 --output-file add_updateSchedule.info
	genhtml add_updateSchedule.info --output-directory coverage_update/
