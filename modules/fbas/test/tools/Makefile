# If expandtab is set in vi, then press CTRL+v and tab to insert tab.
# expandtab is used to convert tab to spaces.
#
# 0. Prerequisites
#
#  IP address assignment
#  - echo -e "\rip\r" | eb-console $rx_node_dev
#  - systemctl status isc-dhcp-server
#
# 1. Basic test with the TX and RX nodes:
#  make saftd            # load saft daemon
#  make do_test3         # transmit MPS flags and events, count/print timing messages, IO connection with LEMO: RX:IO1 -> TX:IO2
#
# 1.1 Repeat basic test (launch it only after 'do_test3' invocation):
#  make repeat_test3
#
# 2. Measure message transmission with the DM node:
#  make saftd  (or 'make saftbusd' for saftlib v3.x)  # load saft daemon
#  make restart_fbasdm_finite                         # or 'make restart_fbasdm_loop'
#
# 2.1 Measure message transmission (set of timing messages with the same deadline, 1 msg in 1 frame)
#  make saftd                     # load saftd
#  make restart_fbasdm_multi_msg  # start the test
#
# 2.2 Measure message transmission (set of timing messages with the same deadline, many msgs in 1 frame)
#  make saftd                     # load saftd
#  make restart_fbasdm_multi_max_msg  # start the test
#
# During the test with multiple messages, which is done by 'restart_fbasdm_multi_max_msg' Makefile target, rx_node_dev looses a few timing messages.
# The problem was caused by old gateware (dm-enigma-merge-v3-2631) of datamaster and solved in newer version (dm-fallout-merge-v2-3297).
# For more details refer to [dm_bad_evtid_20210714] in the FBAS project folder.

# 3. Send a single timing message by datamaster
#  make saftd
#  make restart_fbasdm_single_msg   # restart the test
#  make continue_fbasdm_single_msg  # continue the test

# 4. Test with 2 senders and 1 receivers
#  senders: datamaster and tx_node_dev node
#  note: LEMO cable connection between tx_node_dev and rx_node_dev nodes is not required
#
#  make saftd (or 'make saftbusd' for saftlib v3.x)
#  make restart_two_senders_multi_loop or
#  make restart_two_senders_multi_finite

# 5. Measuring the maximum data rate for receiver
#  senders: datamaster (dev/wbm1)
#  receivers: rx_node_dev node (dev/wbm2)
#
#  make saftd
#  make restart_test_rx_rate

# 6. TTF traffic test - uses the remote datamaster and receiver
#
# senders: datamaster (remote, tsl014)
# receivers: SCU3 (remote, scuxl0497 - specified in test_ttf_rx_rate.sh)
#
# This test setup can be used for testing new WRS software.

.PHONY: mytest \
	saftd saftbusd setup_nodes \
	do_test3 repeat_test3 \
	disable_mps_tx disable_mps_rx disable_mps_all enable_mps_all \
	set_single_msg_per_frame set_multi_msgs_per_frame \
	print_fbasdm_status print_fbasdm_pattern \
	clear_fbasdm load_fbasdm_loop_patterns load_fbasdm_finite_patterns stop_fbasdm_loop_patterns \
	show_rx_stats show_tx_stats show_fbasdm_result \
	restart_fbasdm_finite restart_fbasdm_loop \
	restart_fbasdm_multi_finite restart_fbasdm_multi_loop \
	restart_fbasdm_multi_msg restart_fbasdm_multi_max_msg restart_fbasdm_multi_max_loop_msg \
	restart_fbasdm_single_msg continue_fbasdm_single_msg \
	restart_two_senders_loop restart_two_senders_multi_finite restart_two_senders_multi_loop \
	show_two_senders_result show_rx_tx_stats \
	restart_test_rx_rate restart_test_rx_rate_bad restart_fbasdm_run_pattern \
	test_ttf_traffic

.SILENT: test_ttf_traffic

# tell make not to print the message about entering and leaving the working directory
MAKEFLAGS += --no-print-directory

SHELL=/bin/bash
sleep_period=20                        # seconds
fw_rxpci="fbas128.pcicontrol.bin"      # default LM32 FW for RX node
sched_filename="my_mps_rx_rate_1.dot"  # default DM schedule

mytest:
	$(info source setup.sh)

setup_nodes:
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da # dev/wbm0 (TX)

disable_mps_tx:
	@source setup.sh && disable_mps $$tx_node_dev
	@source setup.sh && eb-read $$tx_node_dev $$addr_cnt/4

disable_mps_rx:
	@source setup.sh && disable_mps $$rx_node_dev
	@source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4

disable_mps_all:
	@source setup.sh && disable_mps_all
	@source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4
	@source setup.sh && eb-read $$tx_node_dev $$addr_cnt/4

enable_mps_all:
	@source setup.sh && enable_mps_all; sleep 2

# Single timing message in single Ethernet frame
set_single_msg_per_frame:
	@source dm.sh && set_dm_maxmsg 1

# Up to 40 timing messages in single Ethernet frame
set_multi_msgs_per_frame:
	@source dm.sh && set_dm_maxmsg 40

print_fbasdm_status:
	@source dm.sh && print_dm_diag

print_fbasdm_pattern:
	@source dm.sh && print_dm_patt

# Targets used to test with 2 senders (datamaster and tx_node_dev)
clear_fbasdm:
	@source dm.sh && clear_dm_patt
	@source dm.sh && clear_dm_diag

load_fbasdm_loop_patterns:
	@source dm.sh && load_dm_patt my_mps_loop.dot
	@source dm.sh && load_dm_patt my_mps_loop1.dot
	@source dm.sh && load_dm_patt my_mps_loop2.dot
	@source dm.sh && load_dm_patt my_mps_loop3.dot

stop_fbasdm_loop_patterns:
	@source dm.sh && stop_dm_patt PatLoop
	@source dm.sh && stop_dm_patt PatLoop1
	@source dm.sh && stop_dm_patt PatLoop2
	@source dm.sh && stop_dm_patt PatLoop3

load_fbasdm_finite_patterns:
	@source dm.sh && load_dm_patt my_mps_pata.dot
	@source dm.sh && load_dm_patt my_mps_patb.dot
	@source dm.sh && load_dm_patt my_mps_patc.dot
	@source dm.sh && load_dm_patt my_mps_patd.dot

show_rx_stats:
	@echo "RX stats"
	@echo -n "ECA valid cnt:    "; source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4
	@echo -n "ECA overflow cnt: "; source setup.sh && eb-read $$rx_node_dev $$addr_eca_ovf/4
	@echo

show_tx_stats:
	@echo "TX stats"
	@echo -n "msg sent by TX: "; source setup.sh && eb-read $$tx_node_dev $$addr_cnt/4
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	@echo

show_two_senders_result:
	@$(eval rx_msg=$(shell source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4))
	@$(eval tx_msg=$(shell source setup.sh && eb-read $$tx_node_dev $$addr_cnt/4))
	@$(eval dm_msg=$(shell source dm.sh && cnt_dm_msg))
	@source setup.sh && report_two_senders_result ${rx_msg} ${tx_msg} ${dm_msg}

show_fbasdm_result:
	@echo -n "msg got by RX:  "; source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4
	@echo -n "msg sent by DM: "; source dm.sh && cnt_dm_msg
	@echo
	@$(eval rx_msg=$(shell source setup.sh && eb-read $$rx_node_dev $$addr_eca_vld/4))
	@$(eval dm_msg=$(shell source dm.sh && cnt_dm_msg))
	@source setup.sh && report_two_senders_result ${rx_msg} 0 ${dm_msg}

saftd:
	@source setup.sh && start_saftd      # saftlib v2.x

saftbusd:
	@source setup.sh && start_saftbusd   # saftlib v3.x

do_test3:
	@$(MAKE) setup_nodes
	@source setup.sh && precheck_test3
	@source setup.sh && start_test3

repeat_test3:
	@source setup.sh && reset_node rx_node_dev SENDER_TX 00267b0004da # dev/wbm0 (TX)
	@source setup.sh && reset_node tx_node_dev           # node device label!
	@source setup.sh && start_test3

restart_fbasdm_finite:
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@source setup.sh && enable_mps $$rx_node_dev
	@$(MAKE) clear_fbasdm
	@source dm.sh && load_dm_patt my_mps_finite.dot && start_dm_patt PatA
	@echo "started DM broadcast, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) print_fbasdm_status
	@source setup.sh && disable_mps $$rx_node_dev
	@$(MAKE) show_fbasdm_result

restart_fbasdm_loop:
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@source setup.sh && enable_mps $$rx_node_dev
	@$(MAKE) clear_fbasdm
	@source dm.sh && load_dm_patt my_mps_loop.dot && start_dm_patt PatLoop
	@echo "started DM broadcast, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@source dm.sh && stop_dm_patt PatLoop
	@$(MAKE) print_fbasdm_status
	@source setup.sh && disable_mps $$rx_node_dev
	@$(MAKE) show_fbasdm_result

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Arbitrary number of patterns are sent.
restart_fbasdm_multi_loop:
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@source setup.sh && enable_mps $$rx_node_dev
	@$(MAKE) clear_fbasdm
	@$(MAKE) load_fbasdm_loop_patterns
	@$(MAKE) print_fbasdm_pattern
	@source dm.sh && start_dm_synchron $$cmd_file_start_loop
	@echo "started DM broadcast, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_loop_patterns
	@source setup.sh && disable_mps $$rx_node_dev
	@$(MAKE) show_fbasdm_result

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Finite number of patterns are sent.
restart_fbasdm_multi_finite:
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@source setup.sh && enable_mps $$rx_node_dev
	@$(MAKE) clear_fbasdm
	@$(MAKE) load_fbasdm_finite_patterns
	@$(MAKE) print_fbasdm_pattern
	@source dm.sh && start_dm_synchron $$cmd_file_start_finite
	@echo "started DM broadcast, wait for 1" && source setup.sh && wait_print_seconds 1
	@source setup.sh && disable_mps $$rx_node_dev
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_fbasdm_result

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# each pattern is packed in single Ethernet frame.
restart_fbasdm_multi_msg:
	@$(MAKE) set_single_msg_per_frame
	@$(MAKE) restart_fbasdm_multi_finite

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# multiple patterns can be packed in single Ethernet frame.
restart_fbasdm_multi_max_msg:
	@$(MAKE) set_multi_msgs_per_frame
	@$(MAKE) restart_fbasdm_multi_finite

# Datamaster sends a set of patterns (timing messages) with the same deadline,
# multiple patterns can be packed in single Ethernet frame.
restart_fbasdm_multi_max_loop_msg:
	@$(MAKE) set_multi_msgs_per_frame
	@$(MAKE) restart_fbasdm_multi_loop

# Datamaster sends a single timing message
restart_fbasdm_single_msg:
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@$(MAKE) clear_fbasdm
	@source dm.sh && load_dm_patt my_single_msg.dot
	@$(MAKE) continue_fbasdm_single_msg

continue_fbasdm_single_msg:
	@source setup.sh && enable_mps $$rx_node_dev
	@source dm.sh && start_dm_patt PatSingle
	@$(MAKE) print_fbasdm_status
	@source setup.sh && disable_mps $$rx_node_dev
	@$(MAKE) show_rx_stats

# Simple test: 2 senders, each EB frame with single timing message
#
# To evaluate the test result compare the sum of messages sent by the datamaster (decimal
# numbers given in a table) and tx_node_dev (hexadecimal number) against
# the number of messages received by rx_node_dev (last hexadecimal output).
# If they match, then test is passed, otherwise test is failed.
restart_two_senders_loop: clear_fbasdm set_single_msg_per_frame
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da 00267b0004dc # dev/wbm0 (TX), dev/wbm1 (DM)
	@source setup.sh && enable_mps_all
	@source dm.sh && load_dm_patt my_mps_loop3.dot
	@source dm.sh && start_dm_patt PatLoop3
	@echo "started DM/TX transmission, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@source dm.sh && stop_dm_patt PatLoop3
	@source setup.sh && disable_mps_all
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_rx_stats
	@$(MAKE) show_tx_stats
	@$(MAKE) show_two_senders_result

# datamaster sends EB frame with multiple timing messages
# rx_node_dev is getting started to loose timing messages at transmission period of 180us
restart_two_senders_multi_loop: clear_fbasdm set_multi_msgs_per_frame
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da 00267b0004dc # dev/wbm0 (TX), dev/wbm1 (DM)
	@source setup.sh && enable_mps_all
	@$(MAKE) load_fbasdm_loop_patterns
	@source dm.sh && start_dm_synchron $$cmd_file_start_loop
	@echo "started DM/TX transmission, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_loop_patterns
	@source setup.sh && disable_mps_all
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_rx_stats
	@$(MAKE) show_tx_stats
	@$(MAKE) show_two_senders_result

restart_two_senders_multi_finite: clear_fbasdm set_multi_msgs_per_frame
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da 00267b0004dc # dev/wbm0 (TX), dev/wbm1 (DM)
	@source setup.sh && enable_mps_all
	@$(MAKE) load_fbasdm_finite_patterns
	@source dm.sh && start_dm_synchron $$cmd_file_start_finite
	@echo "started DM/TX transmission, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@source setup.sh && disable_mps_all
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_rx_stats
	@$(MAKE) show_tx_stats
	@$(MAKE) show_two_senders_result

restart_test_rx_rate:
	@$(MAKE) clear_fbasdm
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@echo "start DM broadcast from '${sched_filename}'"
	@source dm.sh && run_pattern ${sched_filename}
	@source setup.sh && read_counters $$rx_node_dev verbose
	@source setup.sh && result_msg_delay $$rx_node_dev

restart_fbasdm_run_pattern:
	@$(MAKE) clear_fbasdm
	@source dm.sh && clear_dm_patt
	@echo "start DM broadcast from '${sched_filename}'"
	@source dm.sh && run_pattern ${sched_filename}

test_ttf_traffic:
	./test_ttf_rx_rate.sh -s my_mps_rx_rate_1.dot -f fbas16.scucontrol.bin -m
