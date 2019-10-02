GHDL_ALTERA=/home/michael/build/ghdl_vendor_scripts/altera

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
	@socat -d -d pty,raw,echo=0 pty,raw,echo=0 2> pts.dat &
	@printf "device for eb-tools is /dev/pts/%d\n"       `grep PTY pts.dat | tail -n 1 | cut -d"N" -f 2 | cut -d"/" -f4`
	@printf "simulation connects to /dev/pts/%d\n" `grep PTY pts.dat | head -n 1 | cut -d"N" -f 2 | cut -d"/" -f4`
	ghdl -r testbench --stop-time=700000ns --wave=simulation.ghw --ieee-asserts=disable -gPTS_NUMBER=`grep PTY pts.dat | head -n 1 | cut -d"N" -f 2 | cut -d"/" -f4`
	@killall socat; rm pts.dat
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
	gsettings set com.geda.gtkwave reload 0

compile_testbench: file_access_c.o testbench

testbench: 	\
			../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd                  \
			../../ip_cores/general-cores/modules/common/gc_sync_register.vhd             \
			../../ip_cores/general-cores/modules/common/gc_sync_ffs.vhd                  \
			../../ip_cores/general-cores/modules/common/gencores_pkg.vhd                 \
			../../ip_cores/general-cores/modules/genrams/generic/generic_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/common/inferred_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/altera/generic_spram.vhd        \
			../../ip_cores/general-cores/modules/genrams/altera/generic_simple_dpram.vhd \
			../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd               \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/sdb_rom.vhd        \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_crossbar.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_sdb_crossbar.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_clock_crossing/xwb_clock_crossing.vhd \
			../../ip_cores/wr-cores/modules/fabric/wr_fabric_pkg.vhd                    \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_hdr_pkg.vhd              \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_internals_pkg.vhd        \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_fifo.vhd                 \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_top.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_fsm.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tx_mux.vhd               \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tag_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_pass_fifo.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_cfg_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_wbm_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_msi_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/etherbone_pkg.vhd           \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd			\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd			\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_widen.vhd   		\
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_narrow.vhd  		\
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
			file_access.vhd    \
			ez_usb_chip.vhd    \
			wb_minislave.vhd   \
			testbench.vhd
	ghdl -a $(GHDLFLAGS) $?
	ghdl -m -Wl,file_access_c.o  $(GHDLFLAGS) testbench 

file_access_c.o: file_access_c.c
	gcc -c file_access_c.c

clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw 
