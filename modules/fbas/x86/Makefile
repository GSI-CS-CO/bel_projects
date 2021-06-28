# If expandtab is set in vi, then press CTRL+v and tab to insert tab.
# expandtab is used to convert tab to spaces.

.PHONY: mytest \
	saftd setup_nodes setup_fbastx setup_fbasrx \
	start_test3 \
	disable_mps enable_mps \
	stop_mps_tx stop_mps_rx

SHELL=/bin/bash

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

start_test3:
	source setup.sh && start_test3

stop_mps_tx:
	source setup.sh && stop_mps ${FBASTX}

stop_mps_rx:
	source setup.sh && stop_mps ${FBASRX}

disable_mps:
	source setup.sh && disable_mps

enable_mps:
	source setup.sh && enable_mps
