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
# 2.1 Measure message transmission (set of timing messages with the same deadline, 1 msg in 1 frame)
#  make saftd                     # load saftd
#  make restart_fbasdm_multi_msg  # start the test
#
# 2.2 Measure message transmission (set of timing messages with the same deadline, many msgs in 1 frame)
#  make saftd                     # load saftd
#  make restart_fbasdm_multi_max_msg  # start the test
#
# During the test with multiple messages, which is done by 'restart_fbasdm_multi_max_msg' Makefile target, FBASRX looses a few timing messages.
# The problem was caused by old gateware (dm-enigma-merge-v3-2631) of datamaster and solved in newer version (dm-fallout-merge-v2-3297).
# For more details refer to [dm_bad_evtid_20210714] in the FBAS project folder.

# 3. Send a single timing message by datamaster
#  make saftd
#  make restart_fbasdm_single_msg   # restart the test
#  make continue_fbasdm_single_msg  # continue the test

.PHONY: mytest \
	saftd setup_nodes setup_fbastx setup_fbasrx \
	do_test3 \
	disable_mps_tx disable_mps_rx disable_mps_all enable_mps_all \
	restart_fbasdm_finite restart_fbasdm_loop \
	restart_fbasdm_multi_msg restart_fbasdm_multi_max_msg \
	restart_fbasdm_single_msg continue_fbasdm_single_msg \
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

# Single timing message in single Ethernet frame
set_single_msg_per_frame:
	source dm.sh && set_dm_maxmsg 1

# Up to 40 timing messages in single Ethernet frame
set_multi_msgs_per_frame:
	source dm.sh && set_dm_maxmsg 40

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Arbitrary number of patterns are sent.
restart_fbasdm_multi_loop:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_loop.dot
	source dm.sh && load_dm_patt my_mps_loop1.dot
	source dm.sh && load_dm_patt my_mps_loop2.dot
	source dm.sh && load_dm_patt my_mps_loop3.dot
	source dm.sh && print_dm_patt
	source dm.sh && start_dm_synchron $$cmd_file_start_loop
	echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop
	source dm.sh && stop_dm_patt PatLoop1
	source dm.sh && stop_dm_patt PatLoop2
	source dm.sh && stop_dm_patt PatLoop3
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Finite number of patterns are sent.
restart_fbasdm_multi_finite:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_pata.dot
	source dm.sh && load_dm_patt my_mps_patb.dot
	source dm.sh && load_dm_patt my_mps_patc.dot
	source dm.sh && load_dm_patt my_mps_patd.dot
	source dm.sh && print_dm_patt
	source dm.sh && start_dm_synchron $$cmd_file_start_finite
	echo "sleep 1" && source setup.sh && wait_print_seconds 1
	source dm.sh && stop_dm_patt PatA
	source dm.sh && stop_dm_patt PatB
	source dm.sh && stop_dm_patt PatC
	source dm.sh && stop_dm_patt PatD
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1
	@echo "expected received tim. msgs: 0x1f40"

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# each pattern is packed in single Ethernet frame.
restart_fbasdm_multi_msg: set_single_msg_per_frame restart_fbasdm_multi_finite

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# multiple patterns can be packed in single Ethernet frame.
restart_fbasdm_multi_max_msg: set_multi_msgs_per_frame restart_fbasdm_multi_finite

# Datamaster sends a single timing message
restart_fbasdm_single_msg:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_single_msg.dot && start_dm_patt PatSingle
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

continue_fbasdm_single_msg:
	source dm.sh && start_dm_patt PatSingle
	source dm.sh && print_dm_diag
	source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1

print_fbasdm_status:
	source dm.sh && print_dm_diag
