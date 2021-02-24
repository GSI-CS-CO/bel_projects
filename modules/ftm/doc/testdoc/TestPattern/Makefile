all: static_basic.pdf static_coupling1.pdf static_coupling2.pdf static_prio_and_type.pdf \
 static_prio_and_type_meta.pdf dynamic_async.pdf dynamic_basic_run_cpu0_single.pdf \
 dynamic_basic_run_all_single.pdf dynamic_basic_start_stop_abort.pdf dynamic_branch_single.pdf \
 dynamic_loop.pdf

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

%.pdf: %.dot
	./create-pdf.sh $<

clean:
	rm -f static_basic.pdf static_coupling1.pdf static_coupling2.pdf static_prio_and_type.pdf \
	static_prio_and_type_meta.pdf dynamic_async.pdf dynamic_basic_run_cpu0_single.pdf \
	dynamic_basic_run_all_single.pdf dynamic_basic_start_stop_abort.pdf dynamic_branch_single.pdf \
	dynamic_loop.pdf
