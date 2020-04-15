GHDL_ALTERA=/home/michael/build/ghdl_vendor_scripts/altera

GHDLFLAGS = --ieee=synopsys --std=08 \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v08      \
	-P$(GHDL_ALTERA)/altera_lnsim/v08\
	-P$(GHDL_ALTERA)/altera_mf/v08   \
	-P$(GHDL_ALTERA)/arriav/v08      

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

# start simulation (which regenerates wave file), then update viewer
simulation.ghw: compile_testbench 
	ghdl -r testbench --stop-time=700000ns --wave=simulation.ghw --ieee-asserts=disable
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
	gsettings set com.geda.gtkwave reload 0

compile_testbench: file_access_c.o  interface_lm32_c.o testbench

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
			../../ip_cores/etherbone-core/hdl/eb_slave_core/etherbone_pkg.vhd           \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_cfg_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_wbm_fifo.vhd             \
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
			../../ip_cores/general-cores/modules/wishbone/wb_irq/wb_irq_pkg.vhd         \
			../../ip_cores/general-cores/modules/wishbone/wbgenplus/wbgenplus_pkg.vhd   \
			../../modules/dm_diag/dm_diag_auto_pkg.vhd                                  \
			../../modules/ftm/ftm_pkg.vhd                                               \
			../../ip_cores/general-cores/modules/common/matrix_pkg.vhd                  \
			../../modules/prioq2/prio_pkg.vhd                                           \
			../../modules/prioq2/prio_auto_pkg.vhd                                      \
			../../modules/prioq2/prio.vhd                                               \
			../../modules/ftm/ftm_lm32_cluster.vhd                                      \
			../../ip_cores/wr-cores/modules/wr_eca/eca_internals_pkg.vhd                \
			../common/file_access.vhd    \
			../common/ez_usb_chip.vhd    \
			wb_minislave.vhd   \
			testbench.vhd
	ghdl -a $(GHDLFLAGS) $?
	ghdl -m -Wl,interface_lm32_c.o \
	        -Wl,obj_dir/verilated.o \
	        -Wl,obj_dir/verilated_vcd_c.o \
	        -Wl,obj_dir/Vlm32_top__ALL.a  \
	        -Wl,-lm -Wl,-lstdc++ \
	        -Wl,file_access_c.o \
	        $(GHDLFLAGS) testbench 

# this is the C++-implementation (with C-linkage) of the VHDL foreign function interfaces
interface_lm32_c.o: main ../common/interface_lm32_c.cpp ../common/interface_lm32.vhd
	g++ -DVM_TRACE -Iobj_dir -I/usr/share/verilator/include -c ../common/interface_lm32_c.cpp

# file_access_c is for the usb chip simulation 
file_access_c.o: ../common/file_access_c.c
	gcc -c ../common/file_access_c.c

# The 'main' target is a stand-alone application to run the "verilated" lm32 code.
# It is needed to run the auto generated Vlm32_top.mk makefile to generate the 
# lm32 library that is eventually linked to the GHDL object files
LM32_SRC=../../ip_cores/general-cores/modules/wishbone/wb_lm32/src
LM32_MUL=../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
LM32_TOP=$(LM32_SRC)/lm32_top.v
main: main.cpp lm32_top.v
	verilator -Wno-lint --trace --cc -I. -I$(LM32_SRC) -I$(LM32_MUL) lm32_top.v --exe main.cpp
	make -C obj_dir -f Vlm32_top.mk

lm32_top.v: $(LM32_TOP)
	sed    '/input...LM32_INTERRUPT_RNG..interrupt.*/i \/\* verilator lint_off SYMRSVDWORD \*\/' $(LM32_TOP) > lm32_top.v
	sed -i '/input...LM32_INTERRUPT_RNG..interrupt.*/a \/\* verilator lint_on  SYMRSVDWORD \*\/' lm32_top.v

clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw -r obj_dir lm32_top.v

