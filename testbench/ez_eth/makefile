GHDL_ALTERA=/home/mkreider/projects/ghdl_sims

GHDLFLAGS = --ieee=synopsys --std=93c \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v93       \
	-P$(GHDL_ALTERA)/altera_lnsim/v93 \
	-P$(GHDL_ALTERA)/altera_mf/v93    \
	-P$(GHDL_ALTERA)/arriav/v93       

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

# start simulation (which regenerates wave file), then update viewer
simulation.ghw: compile_testbench 
	ghdl -r testbench --stop-time=200000ns --wave=simulation.ghw --ieee-asserts=disable
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
#	gsettings set com.geda.gtkwave reload 0
	gconftool-2 --type string --set /com.geda.gtkwave/0/reload 0

compile_testbench: file_access.o testbench

testbench: 	\
			../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd                  \
			../../ip_cores/general-cores/modules/common/gc_sync_register.vhd             \
			../../ip_cores/general-cores/modules/common/gc_sync_ffs.vhd                  \
			../../ip_cores/general-cores/modules/common/gencores_pkg.vhd                 \
			../../ip_cores/general-cores/modules/common/matrix_pkg.vhd                   \
			../../ip_cores/general-cores/modules/genrams/generic/generic_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/common/inferred_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/altera/generic_spram.vhd        \
			../../ip_cores/general-cores/modules/genrams/altera/generic_simple_dpram.vhd \
			../../ip_cores/general-cores/modules/genrams/generic/generic_sync_fifo.vhd   \
			../common/inferred_sync_fifo.vhd   \
			../common/generic_dpram.vhd        \
			../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd               \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/sdb_rom.vhd        \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_crossbar.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_sdb_crossbar.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_clock_crossing/xwb_clock_crossing.vhd \
			../../ip_cores/wr-cores/modules/fabric/wr_fabric_pkg.vhd                    \
			../../ip_cores/wr-cores/modules/fabric/xwrf_mux.vhd                         \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_hdr_pkg.vhd              \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_internals_pkg.vhd        \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_fifo.vhd                 \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_top.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_fsm.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tx_mux.vhd               \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tag_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_pass_fifo.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/etherbone_pkg.vhd           \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_cfg_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_wbm_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd			\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd			\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_widen.vhd   		\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_narrow.vhd  		\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_ethernet_slave.vhd       \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_checksum.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_eth_tx.vhd               \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_eth_rx.vhd               \
			../common/eb_commit_fifo.vhd          \
			../../ip_cores/etherbone-core/hdl/eb_master_core/ebm_auto_pkg.vhd           \
			../common/eb_commit_len_fifo.vhd     \
			../../ip_cores/etherbone-core/hdl/eb_master_core/eb_framer.vhd              \
			../../ip_cores/etherbone-core/hdl/eb_master_core/eb_master_eth_tx.vhd       \
			../../ip_cores/etherbone-core/hdl/eb_master_core/eb_master_slave_wrapper.vhd\
			../../ip_cores/etherbone-core/hdl/eb_master_core/eb_master_top.vhd          \
			../../ip_cores/etherbone-core/hdl/eb_master_core/eb_record_gen.vhd          \
			../../ip_cores/etherbone-core/hdl/eb_master_core/ebm_auto.vhd               \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_rx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_tx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_dma/xwb_streamer.vhd       \
			../../ip_cores/general-cores/modules/wishbone/wb_dma/xwb_dma.vhd        	\
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_baud_gen.vhd 	\
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb_pkg.vhd				\
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb_fifos.vhd				\
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb.vhd					\
			../../modules/mbox/mbox_pkg.vhd	                                            \
			../../modules/mbox/mbox.vhd    	                                            \
			ez_eth_pkg.vhd    \
			file_access.vhd    \
			ez_eth_chip.vhd    \
			testbench.vhd
	ghdl -a $(GHDLFLAGS) $?
	ghdl -m -Wl,file_access.o  $(GHDLFLAGS) testbench 

file_access.o: file_access.cpp
	gcc -c file_access.cpp

clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw 
