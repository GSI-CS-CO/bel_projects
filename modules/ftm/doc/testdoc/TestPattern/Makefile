PDFs = static_basic.pdf static_coupling1.pdf static_coupling2.pdf static_prio_and_type.pdf \
	static_prio_and_type_meta.pdf dynamic_async.pdf dynamic_basic_run_cpu0_single.pdf \
	dynamic_basic_run_all_single.pdf dynamic_basic_start_stop_abort.pdf dynamic_branch_single.pdf \
	dynamic_loop.pdf \
	booster_all_threads-s.pdf booster_startthread-0-s.pdf booster_startthread-1-s.pdf booster_startthread-2-s.pdf \
	booster_startthread-3-s.pdf booster_thread_0_loop-s.pdf booster_thread_0-s.pdf booster_8_loops.pdf \
	threadsStartStop.pdf nodeInTwoThreads.pdf blink-download.pdf


all: $(PDFs)

static_basic.pdf: static_basic.dot
static_coupling1.pdf: static_coupling1.dot
static_coupling2.pdf: static_coupling2.dot
static_prio_and_type.pdf: static_prio_and_type.dot
static_prio_and_type_meta.pdf: static_prio_and_type_meta.dot
dynamic_async.pdf: dynamic_async.dot
dynamic_basic_run_cpu0_single.pdf: dynamic_basic_run_cpu0_single.dot
dynamic_basic_run_all_single.pdf: dynamic_basic_run_all_single.dot
dynamic_basic_start_stop_abort.pdf: dynamic_basic_start_stop_abort.dot
dynamic_branch_single.pdf: dynamic_branch_single.dot
dynamic_loop.pdf: dynamic_loop.dot
blink-download.pdf: blink-download.dot
booster_all_threads-s.pdf: booster_all_threads-s.dot
	./create-pdf.sh $< neato

booster_startthread-0-s.pdf: booster_startthread-0-s.dot
	./create-pdf.sh $< neato

booster_startthread-1-s.pdf: booster_startthread-1-s.dot
	./create-pdf.sh $< neato

booster_startthread-2-s.pdf: booster_startthread-2-s.dot
	./create-pdf.sh $< neato

booster_startthread-3-s.pdf: booster_startthread-3-s.dot
booster_thread_0_loop-s.pdf: booster_thread_0_loop-s.dot
	./create-pdf.sh $< neato

booster_thread_0-s.pdf: booster_thread_0-s.dot

booster_8_loops.pdf: booster_8_loops.dot
	./create-pdf.sh $< neato

%.pdf: %.dot
	./create-pdf.sh $<

clean:
	rm -f $(PDFs)
