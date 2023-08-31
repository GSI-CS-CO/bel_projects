DATAMASTER ?=$$DM
SNOOP ?=saft-ctl tr0 -xv snoop 0 0 0
DOT_FILE ?= pps-all-threads.dot
PROTOCOL_FILE ?= pps-threads.log
DURATION ?= 5
SNOOP_FILE ?= pps-snoop.txt
# since timeout terminates saft-ctl, we need echo to get a status code of 0. Otherwise, make would stop.
SNOOP_AND_LOG ?= $(SNOOP) $(DURATION) >> $(SNOOP_FILE)
DUMP = eb-get -v $(DATAMASTER) 0x4120000/0x20000
# start test1 from jenkins environment on tsl025.acc:
#TEST_BINARY_DM_CMD="../bin/dm-cmd" TEST_BINARY_DM_SCHED="../bin/dm-sched" LD_LIBRARY_PATH="../lib/" SNOOP="ssh -t root@fel0069.acc.gsi.de 'saft-ctl tr1 -xv snoop 0 0 0'" DATAMASTER="tcp/fel0069.acc.gsi.de" make -C dmThreads/ test1

test1:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear
	dm-sched $(DATAMASTER) add $(DOT_FILE)
	dm-cmd $(DATAMASTER) -v > $(PROTOCOL_FILE)
	$(DUMP) cpu0-dump-0.txt
	dm-cmd $(DATAMASTER) startpattern PPS0a -t 0
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-a.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0b -t 1
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-b.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0c -t 2
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-c.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0d -t 3
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-d.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0e -t 4
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-e.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0f -t 5
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-f.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0g -t 6
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-g.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0h -t 7
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-h.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)


clean:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear -f
	eb-reset $(DATAMASTER) cpureset 0
	eb-reset $(DATAMASTER) cpureset 1
	eb-reset $(DATAMASTER) cpureset 2
	eb-reset $(DATAMASTER) cpureset 3
	dm-cmd $(DATAMASTER) cleardiag
	dm-cmd $(DATAMASTER) -v
	sleep 2
	dm-cmd $(DATAMASTER) -v

analyze-dump:
	# binary files converted to hexdumps (all lines, option -v)
	for x in cpu0-dump-*.txt; do hexdump -v "$$x" > "$$x-hex"; done
	# merge all 9 hexdumps
	sort -m cpu0-dump-?.txt-hex > cpu0-dump.txt-hex
	# strip of duplicate lines, count incidence (option -c)
	uniq -c cpu0-dump.txt-hex > cpu0-dump.txt-hex-uniq
	# remove (grep -v) all lines which are equal in all 9 dumps.
	grep -v ' 9 ' cpu0-dump.txt-hex-uniq > cpu0-dump.txt-hex-uniq-9

four-threads:
	dm-cmd $(DATAMASTER) halt
	dm-sched $(DATAMASTER) clear
	dm-sched $(DATAMASTER) add $(DOT_FILE)
	dm-cmd $(DATAMASTER) -v > $(PROTOCOL_FILE)
	$(DUMP) cpu0-dump-0.txt
	dm-cmd $(DATAMASTER) startpattern PPS0a -t 0
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-a.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0b -t 1
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-b.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0c -t 2
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-c.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)
	dm-cmd $(DATAMASTER) startpattern PPS0d -t 3
	$(SNOOP_AND_LOG)
	$(DUMP) cpu0-dump-d.txt
	dm-cmd $(DATAMASTER) -v >> $(PROTOCOL_FILE)

.PHONY: test1 clean analyze-dump four-threads
