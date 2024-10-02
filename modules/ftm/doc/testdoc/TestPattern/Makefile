PDFs = static_basic.pdf static_coupling1.pdf static_coupling2.pdf static_prio_and_type.pdf \
	static_prio_and_type_meta.pdf dynamic_async.pdf dynamic_basic_run_cpu0_single.pdf \
	dynamic_basic_run_all_single.pdf dynamic_basic_start_stop_abort.pdf dynamic_branch_single.pdf \
	dynamic_loop.pdf \
	cpu0-1-flow-block-target.pdf cpu0-1-switch-block-target.pdf cpu0-1-flush-block-flushovr.pdf cpu0-1-origin-block-origindst.pdf \
	booster_all_threads-s.pdf booster_startthread-0-s.pdf booster_startthread-1-s.pdf booster_startthread-2-s.pdf \
	booster_startthread-3-s.pdf booster_thread_0_loop-s.pdf booster_thread_0-s.pdf booster_8_loops.pdf \
	pps-all-threads-cpu0.pdf twothreads.pdf \
	threadsStartStop.pdf nodeInTwoThreads.pdf blink-download.pdf unilac100.pdf \
	branch1.pdf pps-subgraph.pdf \
	pps-test0-0.pdf pps-test0-1-download.pdf \
	pps-test1-0.pdf pps-test1-1-download.pdf \
	pps-test2-0.pdf pps-test2-1.pdf pps-test2-1-download.pdf pps-test2-2-download.pdf \
	pps-test3-0.pdf pps-test3-1.pdf pps-test3-2.pdf pps-test3-3.pdf pps-test3-4.pdf \
	pps-test3-1-download.pdf pps-test3-2-download.pdf pps-test3-3-download.pdf pps-test3-4-download.pdf \
	pps-test4-0.pdf pps-test4-1.pdf pps-test4-2.pdf \
	pps-test4-1-download.pdf pps-test4-2-download.pdf \
	pps-test5-0.pdf pps-test5-1.pdf pps-test5-1-download.pdf pps-test5-2-download.pdf \
	pps-test6-0.pdf pps-test6-1.pdf pps-test6-2.pdf \
	pps-test6-0-download.pdf pps-test6-1-download.pdf pps-test6-2-download.pdf \
	pps-test8-0.pdf pps-test8-1.pdf pps-test8-1-download.pdf \
	overwrite1-0.pdf overwrite1-1.pdf overwrite1-1-download.pdf \
	overwrite2-0.pdf overwrite2-1-download.pdf overwrite2-2-download.pdf \
	overwrite3-0.pdf overwrite3-1.pdf overwrite3-0-download.pdf overwrite3-1-download.pdf overwrite3-2-download.pdf \
	pps-no-queue.pdf pps-qlo.pdf pps-qlo-status.pdf pps-qhi.pdf pps-qhi-status.pdf \
	patternA-v1-v2.pdf priorityQueue-5.pdf reference-loop3.pdf simultaneousThreads4.pdf \
	remove1-0.pdf remove1-1.pdf remove1-1-download.pdf \
	remove2-0.pdf remove2-1.pdf remove2-1-download.pdf \
	remove3-0.pdf remove3-1.pdf remove3-1-download.pdf \
	remove4-0.pdf remove4-1.pdf remove4-1-download.pdf remove4-2.pdf remove4-2-download.pdf

all: $(PDFs)

booster_all_threads-s.pdf: booster_all_threads-s.dot
	./create-pdf.sh $< neato

booster_startthread-0-s.pdf: booster_startthread-0-s.dot
	./create-pdf.sh $< neato

booster_startthread-1-s.pdf: booster_startthread-1-s.dot
	./create-pdf.sh $< neato

booster_startthread-2-s.pdf: booster_startthread-2-s.dot
	./create-pdf.sh $< neato

booster_thread_0_loop-s.pdf: booster_thread_0_loop-s.dot
	./create-pdf.sh $< neato

booster_8_loops.pdf: booster_8_loops.dot
	./create-pdf.sh $< neato

unilac100.pdf: unilac100.dot
	./create-pdf.sh $< neato

%.pdf: %.dot
	./create-pdf.sh $<

clean:
	rm -f $(PDFs)
