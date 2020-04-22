GHDL_ALTERA=/home/michael/build/ghdl_vendor_scripts/altera

GHDLFLAGS = --ieee=synopsys --std=93c \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_ALTERA)/altera/v93      \
	-P$(GHDL_ALTERA)/altera_lnsim/v93\
	-P$(GHDL_ALTERA)/altera_mf/v93   \
	-P$(GHDL_ALTERA)/arriav/v93      \
	-P$(GHDL_ALTERA)/arriaii/v93     \
	-P$(GHDL_ALTERA)/lpm/v93         \
	-P$(GHDL_ALTERA)/sgate/v93       

# main target is the wave output file
all: simulation.ghw

# view target generates the wave file and starts the viewer
view: simulation.ghw 
	gtkwave simulation.ghw --save=simulation.gtkw &

notrace: compile_testbench 
	ghdl -r testbench --ieee-asserts=disable 

# start simulation (which regenerates wave file), then update viewer
simulation.ghw: compile_testbench 
	ghdl -r testbench --stop-time=1ms --wave=simulation.ghw --ieee-asserts=disable
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
	gsettings set com.geda.gtkwave reload 0

compile_testbench: file_access_c.o sockit_owm_c.o testbench

testbench: 	\
			../../ip_cores/general-cores/modules/genrams/genram_pkg.vhd                  \
			../../ip_cores/general-cores/modules/common/gc_sync_register.vhd             \
			../../ip_cores/general-cores/modules/common/gc_sync_ffs.vhd                  \
			../../ip_cores/general-cores/modules/common/gencores_pkg.vhd                 \
			../../ip_cores/general-cores/modules/common/matrix_pkg.vhd                   \
			../../ip_cores/general-cores/modules/genrams/generic/generic_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/generic/generic_async_fifo_dual_rst.vhd \
			../../ip_cores/general-cores/modules/genrams/common/inferred_async_fifo.vhd  \
			../../ip_cores/general-cores/modules/genrams/common/inferred_async_fifo_dual_rst.vhd  \
			../common/inferred_sync_fifo.vhd   \
			../common/generic_spram.vhd        \
			../common/generic_simple_dpram.vhd \
			../common/generic_dpram.vhd        \
			../../ip_cores/general-cores/modules/genrams/generic/generic_sync_fifo.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wishbone_pkg.vhd               \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/sdb_rom.vhd        \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_crossbar.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_crossbar/xwb_sdb_crossbar.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_clock_crossing/xwb_clock_crossing.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_register/wb_skidpad.vhd    \
			../../ip_cores/general-cores/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_irq/wb_irq_pkg.vhd         \
			../../ip_cores/general-cores/modules/wishbone/wbgenplus/wbgenplus_pkg.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_onewire_master/wb_onewire_master.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_onewire_master/xwb_onewire_master.vhd \
			../common/wb_irq_slave.vhd       \
			../../ip_cores/general-cores/modules/wishbone/wb_dpram/xwb_dpram.vhd        \
			../../ip_cores/wr-cores/modules/fabric/wr_fabric_pkg.vhd                    \
			../../ip_cores/wr-cores/modules/wr_eca/eca_internals_pkg.vhd                \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_hdr_pkg.vhd              \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_internals_pkg.vhd        \
			../common/eb_fifo.vhd                 \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_top.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_slave_fsm.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tx_mux.vhd               \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_tag_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_pass_fifo.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/etherbone_pkg.vhd           \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_cfg_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_wbm_fifo.vhd             \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_raw_slave.vhd            \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_widen.vhd         \
			../../ip_cores/etherbone-core/hdl/eb_slave_core/eb_stream_narrow.vhd        \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_rx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_tx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_dma/xwb_streamer.vhd       \
			../../ip_cores/general-cores/modules/wishbone/wb_dma/xwb_dma.vhd            \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_baud_gen.vhd     \
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb_pkg.vhd                \
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb_fifos.vhd              \
			../../ip_cores/etherbone-core/hdl/eb_usb_core/ez_usb.vhd                    \
			../../modules/mbox/mbox_pkg.vhd	                                            \
			../../modules/mbox/mbox.vhd    	                                            \
			../../modules/dm_diag/dm_diag_auto_pkg.vhd                                  \
			../common/dm_diag.vhd  \
			../../modules/dm_diag/dm_diag_auto.vhd                                      \
			../../modules/ftm/ftm_pkg.vhd                                               \
			../../modules/prioq2/prio_pkg.vhd                                           \
			../../modules/prioq2/prio_auto_pkg.vhd                                      \
			../../modules/prioq2/prio_auto.vhd                                          \
			../../modules/prioq2/prio.vhd                                               \
			../../modules/prioq2/arbiter.vhd                                            \
			../../modules/prioq2/queue_unit.vhd                                         \
			../../modules/prioq2/min9_64.vhd                                            \
			../../modules/prioq2/min3.vhd                                               \
			../../modules/ftm/ftm_lm32.vhd                                              \
			../../modules/ftm/ftm_lm32_cluster.vhd                                      \
			../../modules/aux_functions/aux_functions_pkg.vhd                           \
			../../modules/aux_functions/div_n.vhd                                       \
			../../modules/wb_arria_reset/wb_arria_reset_pkg.vhd                         \
			../../modules/wb_arria_reset/arria_reset.vhd                                \
			../../modules/wb_arria_reset/arria5_reset.vhd                               \
			../common/wb_arria_reset.vhd \
			../common/interface_lm32.vhd                                                \
			../common/xwb_lm32.vhd                                                      \
			../common/file_access.vhd    \
			../common/ez_usb_chip.vhd    \
			../common/sockit_owm.vhd  \
			testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $?
	ghdl -m -Wl,sockit_owm_c.o \
	        -Wl,obj_dir/verilated.o \
	        -Wl,obj_dir/verilated_vcd_c.o \
	        -Wl,obj_dir/Vsockit_owm__ALL.a \
	        -Wl,-lm -Wl,-lstdc++ \
	        -Wl,file_access_c.o \
	        $(GHDLFLAGS) testbench 

# this is the C++-implementation (with C-linkage) of the VHDL foreign function interfaces
sockit_owm_c.o: ../common/main_sockit_owm.cpp ../common/sockit_owm_c.cpp main_sockit_owm
	g++ -DVM_TRACE -Iobj_dir -I/usr/share/verilator/include -c ../common/sockit_owm_c.cpp

# file_access_c is for the usb chip simulation 
file_access_c.o: ../common/file_access_c.c
	gcc -c ../common/file_access_c.c

# The 'main' target is a stand-alone application to run the "verilated" lm32 code.
# It is needed to run the auto generated Vlm32_top.mk makefile which generates the 
# lm32 library that is linked together with the GHDL object files into the final
# testbench executable.
SOCKIT_OWM_SRC=../../ip_cores/general-cores/modules/wishbone/wb_onewire_master
SOCKIT_OWM_TOP=$(SOCKIT_OWM_SRC)/sockit_owm.v
main_sockit_owm: ../common/main_sockit_owm.cpp sockit_owm.v
	verilator -Wno-lint --trace --cc -I. -I$(SOCKIT_OWM_SRC) sockit_owm.v --exe ../common/main_sockit_owm.cpp
	make -C obj_dir -f Vsockit_owm.mk

sockit_owm.v: $(SOCKIT_OWM_TOP)
	sed    '/initial                    owr_sel <= /i \/\* verilator lint_off INITIALDLY \*\/' $(SOCKIT_OWM_TOP) > sockit_owm.v
	sed -i '/initial                    owr_sel <= /a \/\* verilator lint_on  INITIALDLY \*\/'                     sockit_owm.v
	sed -i 's/parameter BTP_O = "0.5"/parameter BTP_O = "1.0"/g'                                                   sockit_owm.v





clean:
	rm -f *.o testbench work-obj*.cf simulation.ghw -r obj_dir sockit_owm.v

