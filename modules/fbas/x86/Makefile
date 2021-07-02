# If expandtab is set in vi, then press CTRL+v and tab to insert tab.
# expandtab is used to convert tab to spaces.
#
# 0. Prerequisites
#
#  IP address assignment
#  - eb-console $$FBASRX
#  - systemctl status isc-dhcp-server
#
# 1. Basic test:
#  make saftd            # load saft daemon
#  make setup_nodes      # load fw, set ECA rules
#  make do_test3         # transmit MPS flags and events, count/print timing messages
#
# 2. Measure message transmission:
#  make saftd            # load saft daemon
#  make setup_fbasrx     # load fw, set ECA rules only for RX node
#  make restart_fbasdm_finite OR make restart_fbasdm_loop
#
# 2.1 Measure message transmission (multi pattern)
#  make saftd                     # load saftd
#  make setup_fbasrx              # load fw binary, set ECA rules
#  make restart_fbasdm_multi_loop # start multiple patterns

.PHONY: mytest \
	saftd setup_nodes setup_fbastx setup_fbasrx \
	do_test3 \
	disable_mps_tx disable_mps_rx disable_mps_all enable_mps_all \
	restart_fbasdm_finite restart_fbasdm_loop \
	print_fbasdm_status

SHELL=/bin/bash
sleep_period=20    # seconds

mytest:
	$(info source setup.sh)

saftd:
	source setup.sh && start_saftd

setup_nodes:
	source setup.sh && setup_fbastx && setup_fbasrx

setup_fbastx:
	source setup.sh && setup_fbastx

setup_fbasrx:
	source setup.sh && setup_fbasrx

do_test3:
	source setup.sh && do_test3

disable_mps_tx:
	source setup.sh && disable_mps $$FBASTX
	source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1

disable_mps_rx:
	source setup.sh && disable_mps $$FBASRX
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

disable_mps_all:
	source setup.sh && disable_mps_all
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1
	source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1

enable_mps_all:
	source setup.sh && enable_mps_all

restart_fbasdm_finite:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_finite.dot && start_dm_patt PatA
	echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

restart_fbasdm_loop:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_loop.dot && start_dm_patt PatLoop
	echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

restart_fbasdm_multi_loop:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_loop.dot
	source dm.sh && load_dm_patt my_mps_loop1.dot
	source dm.sh && load_dm_patt my_mps_loop2.dot
	source dm.sh && load_dm_patt my_mps_loop3.dot
	source dm.sh && print_dm_patt
	source dm.sh && set_dm_maxmsg # to restore default: eb-write $fbasdm 0x41000a4/4 0x28
	source dm.sh && start_dm_synchron
	echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop
	source dm.sh && stop_dm_patt PatLoop1
	source dm.sh && stop_dm_patt PatLoop2
	source dm.sh && stop_dm_patt PatLoop3
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

print_fbasdm_status:
	source dm.sh && print_dm_diag
