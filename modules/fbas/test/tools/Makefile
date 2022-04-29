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
# 1.1 Repeat basic test (launch it only after 'do_test3' invokation):
#  make repeat_test3
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

# 4. Test with 2 senders and 1 receivers
#  senders: datamaster and FBASTX node
#  note: LEMO cable connection between FBASTX and FBASRX nodes is not required
#
#  make saftd
#  make restart_two_senders_multi_loop or
#  make restart_two_senders_multi_finite

# 5. Measuring the maximum data rate for receiver
#  senders: datamaster (dev/wbm1)
#  receivers: FBASRX node (dev/wbm2)
#
#  make saftd
#  make restart_test_rx_rate

.PHONY: mytest \
	saftd setup_nodes setup_fbastx setup_fbasrx \
	do_test3 repeat_test3 \
	disable_mps_tx disable_mps_rx disable_mps_all enable_mps_all \
	restart_fbasdm_finite restart_fbasdm_loop \
	restart_fbasdm_multi_msg restart_fbasdm_multi_max_msg \
	restart_fbasdm_single_msg continue_fbasdm_single_msg \
	print_fbasdm_status \
	clear_fbasdm load_fbasdm_loop_patterns load_fbasdm_finite_patterns \
	restart_two_senders_multi_finite restart_two_senders_multi_loop \
	show_two_senders_result \
	restart_test_rx_rate restart_test_rx_rate_bad restart_fbasdm_run_pattern

SHELL=/bin/bash
sleep_period=20                        # seconds
fw_rxpci="fbas.pcicontrol.bin"         # default LM32 FW for RX node
sched_filename="my_mps_rx_rate_1.dot"  # default DM schedule

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
	source setup.sh && precheck_test3
	source setup.sh && start_test3

repeat_test3:
	source setup.sh && reset_node $$FBASRX
	source setup.sh && reset_node $$FBASTX
	source setup.sh && start_test3

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
	@echo -n "msg got by RX:  "; source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	$(MAKE) show_fbasdm_result

restart_fbasdm_loop:
	source setup.sh && setup_fbasrx
	source dm.sh && clear_dm_diag
	source dm.sh && clear_dm_patt
	source dm.sh && load_dm_patt my_mps_loop.dot && start_dm_patt PatLoop
	echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop
	source dm.sh && print_dm_diag
	@echo -n "msg got by RX:  "; source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	$(MAKE) show_fbasdm_result

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
	source dm.sh && cnt_dm_msg
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

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# multiple patterns can be packed in single Ethernet frame.
restart_fbasdm_multi_max_loop_msg: set_multi_msgs_per_frame restart_fbasdm_multi_loop

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

# Targets used to test with 2 senders (datamaster and FBASTX)
clear_fbasdm:
	source dm.sh && clear_dm_patt
	source dm.sh && clear_dm_diag

load_fbasdm_loop_patterns:
	source dm.sh && load_dm_patt my_mps_loop.dot
	source dm.sh && load_dm_patt my_mps_loop1.dot
	source dm.sh && load_dm_patt my_mps_loop2.dot
	source dm.sh && load_dm_patt my_mps_loop3.dot

load_fbasdm_finite_patterns:
	source dm.sh && load_dm_patt my_mps_pata.dot
	source dm.sh && load_dm_patt my_mps_patb.dot
	source dm.sh && load_dm_patt my_mps_patc.dot
	source dm.sh && load_dm_patt my_mps_patd.dot

# Simple test: 2 senders, each EB frame with single timing message
#
# To evaluate result add the number of messages sent by the datamaster (decimal
# numbers given in a table) and FBASTX (hexadecimal number) and compare the sum
# with the number of messages received by FBASRX (last hexadecimal output).
# If they match, then test is passed, otherwise test is failed.
restart_two_senders_loop: clear_fbasdm set_single_msg_per_frame
	source dm.sh && load_dm_patt my_mps_loop3.dot
	source setup.sh && setup_fbastx && setup_fbasrx
	source setup.sh && enable_mps_all
	source dm.sh && start_dm_patt PatLoop3
	@echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop3
	source setup.sh && disable_mps_all
	source dm.sh && print_dm_diag
	@echo -n "msg got by RX:  "; source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1
	@echo
	@echo -n "msg sent by TX: "; source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	$(MAKE) show_two_senders_result

# datamaster sends EB frame with multiple timing messages
# FBASRX is getting started to loose timing messages at transmission period of 180us
restart_two_senders_multi_loop: clear_fbasdm set_multi_msgs_per_frame load_fbasdm_loop_patterns
	source setup.sh && setup_fbastx && setup_fbasrx
	source setup.sh && enable_mps_all
	source dm.sh && start_dm_synchron $$cmd_file_start_loop
	@echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source dm.sh && stop_dm_patt PatLoop
	source dm.sh && stop_dm_patt PatLoop1
	source dm.sh && stop_dm_patt PatLoop2
	source dm.sh && stop_dm_patt PatLoop3
	source setup.sh && disable_mps_all
	source dm.sh && print_dm_diag
	@echo -n "msg got by RX:  "; source setup.sh && read_shared_mem $$FBASRX $$addr_eca_vld
	@echo
	@echo -n "msg sent by TX: "; source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	@echo
	$(MAKE) show_two_senders_result

restart_two_senders_multi_finite: clear_fbasdm set_multi_msgs_per_frame load_fbasdm_finite_patterns
	source setup.sh && setup_fbastx && setup_fbasrx
	source setup.sh && enable_mps_all
	source dm.sh && start_dm_synchron $$cmd_file_start_finite
	@echo "sleep ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	source setup.sh && disable_mps_all
	source dm.sh && print_dm_diag
	@echo -n "msg got by RX:  "; source setup.sh && read_shared_mem $$FBASRX $$addr_eca_vld
	@echo
	@echo -n "msg sent by TX: "; source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	@echo
	$(MAKE) show_two_senders_result

show_two_senders_result:
	@$(eval rx_msg=$(shell source setup.sh && read_shared_mem $$FBASRX $$addr_eca_vld))
	@$(eval tx_msg=$(shell source setup.sh && read_shared_mem $$FBASTX $$addr_cnt1))
	@$(eval dm_msg=$(shell source dm.sh && cnt_dm_msg))
	@source setup.sh && report_two_senders_result ${rx_msg} ${tx_msg} ${dm_msg}

show_fbasdm_result:
	@$(eval rx_msg=$(shell source setup.sh && read_shared_mem $$FBASRX $$addr_cnt1))
	@$(eval dm_msg=$(shell source dm.sh && cnt_dm_msg))
	@source setup.sh && report_two_senders_result ${rx_msg} 0 ${dm_msg}

restart_test_rx_rate:
	@source setup.sh && setup_fbasrx ${fw_rxpci}
	@source dm.sh && clear_dm_diag
	@source dm.sh && clear_dm_patt
	@echo "start schedule from '${sched_filename}'"
	@source dm.sh && run_pattern ${sched_filename}
	@source setup.sh && read_counters $$FBASRX
	@source setup.sh && result_ow_delay $$FBASRX

restart_fbasdm_run_pattern:
	@source dm.sh && clear_dm_diag
	@source dm.sh && clear_dm_patt
	@echo "start schedule from '${sched_filename}'"
	@source dm.sh && run_pattern ${sched_filename}
