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
#  make restart_basic    # transmit MPS flags and events, count/print timing messages, IO connection with LEMO: RX:IO1 -> TX:IO2
#
# 1.1 Repeat basic test (launch it only after 'restart_basic' invocation):
#  make continue_basic
#
# 2. Test message transmission with the DM node:
#  make saftd  (or 'make saftbusd' for saftlib v3.x)  # load saft daemon
#  make restart_fbasdm_finite                         # or 'make restart_fbasdm_loop'
#
# 2.1 Test message transmission (set of timing messages with the same deadline, 1 msg in 1 frame)
#  make saftd                     # load saftd
#  make restart_fbasdm_multi_msg  # start the test
#
# 2.2 Test message transmission (set of timing messages with the same deadline, multiple msgs in 1 frame)
#  make saftd                         # load saftd
#  make restart_fbasdm_multi_max_msg  # start the test
#
# During the test with multiple messages, which was done by 'restart_fbasdm_multi_max_msg' Makefile target, rx_node_dev lost a few timing messages.
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
# receivers: SCU3 (remote, scuxl0497 - specified in measure_ttf_rx_rate.sh)
#
# This test setup can be used for testing new WRS software.

.PHONY: mytest \
	saftd saftbusd setup_tx_rx_nodes \
	restart_basic continue_basic \
	disable_tx_node disable_rx_node enable_rx_node enable_rx_tx_nodes \
	set_single_msg_per_frame set_multi_msgs_per_frame \
	print_fbasdm_status print_fbasdm_pattern \
	clear_fbasdm load_fbasdm_multi_finite_patterns load_fbasdm_multi_loop_patterns stop_fbasdm_multi_loop_patterns \
	show_rx_stats show_tx_stats show_fbasdm_result \
	prepare_fbasdm_broadcast stop_fbasdm_test \
	restart_fbasdm_finite restart_fbasdm_loop \
	restart_fbasdm_multi_finite restart_fbasdm_multi_loop \
	restart_fbasdm_multi_msg restart_fbasdm_multi_max_msg restart_fbasdm_multi_max_loop_msg \
	restart_fbasdm_single_msg continue_fbasdm_single_msg \
	restart_two_senders_loop restart_two_senders_multi_finite restart_two_senders_multi_loop \
	show_two_senders_result show_rx_tx_stats \
	restart_test_rx_rate restart_fbasdm_run_pattern \
	test_ttf_traffic

.SILENT: test_ttf_traffic

# tell make not to print the message about entering and leaving the working directory
MAKEFLAGS += --no-print-directory

SHELL=/bin/bash
sleep_period=20                        # seconds
fw_rxpci="fbas128.pcicontrol.bin"      # default LM32 FW for RX node (pexaria)
fw_rxscu="fbas128.scucontrol.bin"      # default LM32 FW for RX node (scu)
sched_def="my_mps_rx_rate_1.dot"       # default DM schedule
sched_loop="my_mps_loop.dot"           # DM schedule with loop
pattern_loop="PatLoop"                 # pattern name of the loop schedule
sched_finite="my_mps_finite.dot"       # DM schedule with finite commands
pattern_finite="PatA"                  # pattern name of the finite schedule
sched_single="my_single_msg.dot"
pattern_single="PatSingle"

mytest:
	$(info source setup.sh)

setup_tx_rx_nodes:
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da # dev/wbm0 (TX)

disable_tx_node:
	@source setup.sh && disable_mps $$tx_node_dev

disable_rx_node:
	@source setup.sh && disable_mps $$rx_node_dev

enable_rx_node:
	@source setup.sh && enable_mps $$rx_node_dev

enable_rx_tx_nodes:
	@source setup.sh && enable_mps_all

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

load_fbasdm_multi_loop_patterns:
	@source dm.sh && load_dm_patt my_mps_loop.dot
	@source dm.sh && load_dm_patt my_mps_loop1.dot
	@source dm.sh && load_dm_patt my_mps_loop2.dot
	@source dm.sh && load_dm_patt my_mps_loop3.dot

stop_fbasdm_multi_loop_patterns:
	@source dm.sh && stop_dm_patt PatLoop
	@source dm.sh && stop_dm_patt PatLoop1
	@source dm.sh && stop_dm_patt PatLoop2
	@source dm.sh && stop_dm_patt PatLoop3

load_fbasdm_multi_finite_patterns:
	@source dm.sh && load_dm_patt my_mps_pata.dot
	@source dm.sh && load_dm_patt my_mps_patb.dot
	@source dm.sh && load_dm_patt my_mps_patc.dot
	@source dm.sh && load_dm_patt my_mps_patd.dot

start_fbasdm_1_loop_pattern:
	@source dm.sh && load_dm_patt ${sched_loop} && start_dm_patt ${pattern_loop}

stop_fbasdm_1_loop_pattern:
	@source dm.sh && stop_dm_patt ${pattern_loop}

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

# used in restart_fbasdm_loop/finite tests
prepare_fbasdm_broadcast:
	@$(MAKE) clear_fbasdm
	@source setup.sh && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004dc # dev/wbm1 (DM)
	@$(MAKE) enable_rx_node

# used in restart_fbasdm_loop/finite tests
stop_fbasdm_test:
	@$(MAKE) disable_rx_node
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_fbasdm_result

# used in restart_two_senders_*
prepare_two_senders:
	@source setup.sh && setup_fbastx && setup_fbasrx ${fw_rxpci} SENDER_TX 00267b0004da 00267b0004dc # dev/wbm0 (TX), dev/wbm1 (DM)

# used in restart_two_senders_*
stop_two_senders:
	@source setup.sh && disable_mps_all
	@$(MAKE) print_fbasdm_status
	@$(MAKE) show_rx_stats
	@$(MAKE) show_tx_stats
	@$(MAKE) show_two_senders_result

saftd:
	@source setup.sh && start_saftd      # saftlib v2.x

saftbusd:
	@source setup.sh && start_saftbusd   # saftlib v3.x

restart_basic:
	@$(MAKE) setup_tx_rx_nodes
	@source setup.sh && precheck_basic
	@source setup.sh && start_basic

continue_basic:
	@source setup.sh && reset_node rx_node_dev SENDER_TX 00267b0004da # dev/wbm0 (TX)
	@source setup.sh && reset_node tx_node_dev           # node device label!
	@source setup.sh && start_basic

restart_fbasdm_finite:
	@$(MAKE) prepare_fbasdm_broadcast
	@source dm.sh && load_dm_patt ${sched_finite} && start_dm_patt ${pattern_finite}
	@$(MAKE) stop_fbasdm_test

restart_fbasdm_loop:
	@$(MAKE) prepare_fbasdm_broadcast
	@$(MAKE) start_fbasdm_1_loop_pattern
	@echo "started DM broadcast, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_1_loop_pattern
	@$(MAKE) stop_fbasdm_test

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Arbitrary number of patterns are sent.
restart_fbasdm_multi_loop:
	@$(MAKE) prepare_fbasdm_broadcast
	@$(MAKE) load_fbasdm_multi_loop_patterns
	@$(MAKE) print_fbasdm_pattern
	@source dm.sh && start_dm_synchron $$cmd_file_start_loop
	@echo "started DM broadcast, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_multi_loop_patterns
	@$(MAKE) disable_rx_node
	@$(MAKE) show_fbasdm_result

# Datamaster sends a set of patterns (timing messages) with the same deadline.
# Finite number of patterns are sent.
restart_fbasdm_multi_finite:
	@$(MAKE) prepare_fbasdm_broadcast
	@$(MAKE) load_fbasdm_multi_finite_patterns
	@$(MAKE) print_fbasdm_pattern
	@source dm.sh && start_dm_synchron $$cmd_file_start_finite
	@echo "started DM broadcast, wait for 1" && source setup.sh && wait_print_seconds 1
	@$(MAKE) disable_rx_node
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
	@$(MAKE) prepare_fbasdm_broadcast
	@source dm.sh && load_dm_patt ${sched_single}
	@$(MAKE) continue_fbasdm_single_msg

continue_fbasdm_single_msg:
	@$(MAKE) enable_rx_node
	@source dm.sh && start_dm_patt ${pattern_single}
	@$(MAKE) print_fbasdm_status
	@$(MAKE) disable_rx_node
	@$(MAKE) show_rx_stats

# Simple test: 2 senders, each EB frame with single timing message
#
# To evaluate the test result compare the sum of messages sent by the datamaster (decimal
# numbers given in a table) and tx_node_dev (hexadecimal number) against
# the number of messages received by rx_node_dev (last hexadecimal output).
# If they match, then test is passed, otherwise test is failed.
restart_two_senders_loop: clear_fbasdm set_single_msg_per_frame
	@$(MAKE) prepare_two_senders
	@$(MAKE) enable_rx_tx_nodes
	@$(MAKE) start_fbasdm_1_loop_pattern
	@echo "started DM/TX transmission, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_1_loop_pattern
	@$(MAKE) stop_two_senders

# datamaster sends EB frame with multiple timing messages
# rx_node_dev is getting started to loose timing messages at transmission period of 180us
restart_two_senders_multi_loop: clear_fbasdm set_multi_msgs_per_frame
	@$(MAKE) prepare_two_senders
	@$(MAKE) enable_rx_tx_nodes
	@$(MAKE) load_fbasdm_multi_loop_patterns
	@source dm.sh && start_dm_synchron $$cmd_file_start_loop
	@echo "started DM/TX transmission, wait for ${sleep_period}" && source setup.sh && wait_print_seconds ${sleep_period}
	@$(MAKE) stop_fbasdm_multi_loop_patterns
	@$(MAKE) stop_two_senders

restart_two_senders_multi_finite: clear_fbasdm set_multi_msgs_per_frame
	@$(MAKE) prepare_two_senders
	@$(MAKE) enable_rx_tx_nodes
	@$(MAKE) load_fbasdm_multi_finite_patterns
	@source dm.sh && start_dm_synchron $$cmd_file_start_finite
	@$(MAKE) stop_two_senders

restart_test_rx_rate:
	@$(MAKE) prepare_fbasdm_broadcast
	@source dm.sh && run_pattern ${sched_def} && echo
	@$(MAKE) show_fbasdm_result
	@echo -n "Msg delay: " && source setup.sh && result_msg_delay $$rx_node_dev

restart_fbasdm_run_pattern:
	@$(MAKE) clear_fbasdm
	@source dm.sh && run_pattern ${sched_def} && echo

test_ttf_traffic:
	./measure_ttf_rx_rate.sh -s my_mps_rx_rate_1.dot -f ${fw_rxscu} -m
