DATAMASTER ?= $$DM

all: blockalign1

blockalign1:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear
	dm-cmd $(DATAMASTER) cleardiag
	dm-sched $(DATAMASTER) add blockalign1.dot
	dm-cmd $(DATAMASTER) startpattern PPS0_TEST
	dm-cmd $(DATAMASTER) startpattern PPS1_TEST
	dm-cmd $(DATAMASTER) chkrem PPS1_TEST
	sed -i /Created/d debug.dot
	diff debug.dot blockalign1-debug-forbidden.dot
	rm debug.dot
	dm-cmd $(DATAMASTER) abortpattern PPS1_TEST
	dm-cmd $(DATAMASTER) chkrem PPS1_TEST
	sed -i /Created/d debug.dot
	diff debug.dot blockalign1-debug-safe.dot
	rm debug.dot
	dm-sched $(DATAMASTER) remove blockalign1-remove.dot
	dm-sched $(DATAMASTER) status -o status.dot
	diff status.dot blockalign1-status.dot
	rm status.dot


