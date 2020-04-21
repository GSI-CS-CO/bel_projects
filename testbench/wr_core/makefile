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
	ghdl -r testbench --stop-time=3ms --wave=simulation.ghw --ieee-asserts=disable
	#vcd2fst simulation.vcd simulation.fst && rm simulation.vcd
	gsettings set com.geda.gtkwave reload 0

compile_testbench: file_access_c.o interface_lm32_c.o testbench

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
			../../ip_cores/general-cores/modules/genrams/common/generic_shiftreg_fifo.vhd \
			../../ip_cores/general-cores/modules/genrams/altera/gc_shiftreg.vhd          \
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
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_baud_gen.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_tx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/uart_async_rx.vhd     \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/simple_uart_pkg.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/simple_uart_wb.vhd    \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/wb_simple_uart.vhd    \
			../../ip_cores/general-cores/modules/wishbone/wb_uart/xwb_simple_uart.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wb_onewire_master/wb_onewire_master.vhd \
			../../ip_cores/general-cores/modules/wishbone/wb_onewire_master/xwb_onewire_master.vhd \
			../../ip_cores/general-cores/modules/common/gc_frequency_meter.vhd          \
			../../ip_cores/general-cores/modules/common/gc_pulse_synchronizer.vhd       \
			../../ip_cores/general-cores/modules/common/gc_pulse_synchronizer2.vhd      \
			../../ip_cores/general-cores/modules/wishbone/wbgen2/wbgen2_pkg.vhd         \
			../../ip_cores/general-cores/modules/wishbone/wbgen2/wbgen2_fifo_sync.vhd   \
			../../ip_cores/general-cores/modules/wishbone/wbgen2/wbgen2_eic.vhd         \
			../../ip_cores/general-cores/modules/common/gc_sync_word_wr.vhd             \
			../../ip_cores/general-cores/modules/common/gc_extend_pulse.vhd             \
			../../ip_cores/general-cores/modules/common/gc_crc_gen.vhd                  \
			../common/wb_irq_slave.vhd       \
			../../ip_cores/general-cores/modules/wishbone/wb_dpram/xwb_dpram.vhd        \
			../../ip_cores/wr-cores/modules/fabric/wr_fabric_pkg.vhd                    \
			../../ip_cores/wr-cores/modules/wr_eca/eca_internals_pkg.vhd                \
			../../ip_cores/wr-cores/modules/wr_endpoint/endpoint_pkg.vhd                \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/softpll_pkg.vhd               \
			../../ip_cores/wr-cores/modules/wrc_core/wrc_syscon_pkg.vhd                 \
			../../ip_cores/wr-cores/modules/wrc_core/wrc_diags_pkg.vhd                  \
			../../ip_cores/wr-cores/modules/wrc_core/wrcore_pkg.vhd                     \
			../../ip_cores/wr-cores/modules/wrc_core/wrc_diags_wb.vhd                   \
			../../ip_cores/wr-cores/modules/wrc_core/xwrc_diags_wb.vhd                  \
			../../ip_cores/wr-cores/modules/wrc_core/wrc_syscon_wb.vhd                  \
			../../ip_cores/wr-cores/modules/wr_pps_gen/xwr_pps_gen.vhd                  \
			../../ip_cores/wr-cores/modules/wr_pps_gen/pps_gen_wb.vhd                   \
			../../ip_cores/wr-cores/modules/wr_pps_gen/wr_pps_gen.vhd                   \
			../../ip_cores/wr-cores/modules/timing/dmtd_with_deglitcher.vhd             \
			../../ip_cores/wr-cores/modules/timing/dmtd_phase_meas.vhd                  \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/spll_aligner.vhd              \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/spll_wbgen2_pkg.vhd           \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/spll_wb_slave.vhd             \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/wr_softpll_ng.vhd             \
			../../ip_cores/wr-cores/modules/wr_softpll_ng/xwr_softpll_ng.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_registers_pkg.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/endpoint_private_pkg.vhd        \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_crc32_pkg.vhd                \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_leds_controller.vhd          \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_1000basex_pcs.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_path.vhd                  \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_path.vhd                  \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_timestamping_unit.vhd        \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_wishbone_controller.vhd      \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_registers_pkg.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/wr_endpoint.vhd                 \
			../../ip_cores/wr-cores/modules/wr_endpoint/xwr_endpoint.vhd                \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_pcs_16bit.vhd             \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_pcs_16bit.vhd             \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_pcs_8bit.vhd              \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_pcs_8bit.vhd              \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_header_processor.vhd      \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_inject_ctrl.vhd           \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_vlan_unit.vhd             \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_packet_injection.vhd      \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_tx_crc_inserter.vhd          \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_early_address_match.vhd   \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_packet_filter.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_clock_alignment_fifo.vhd     \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_oob_insert.vhd            \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_crc_size_check.vhd        \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_autonegotiation.vhd          \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_pcs_tbi_mdio_wb.vhd          \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_vlan_unit.vhd             \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rtu_header_extract.vhd       \
			../common/ep_rx_buffer.vhd                \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_status_reg_insert.vhd     \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_wb_master.vhd             \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_ts_counter.vhd               \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_sync_detect_16bit.vhd        \
			../../ip_cores/wr-cores/modules/wr_endpoint/ep_sync_detect.vhd              \
			../../ip_cores/wr-cores/modules/wr_mini_nic/minic_wbgen2_pkg.vhd            \
			../../ip_cores/wr-cores/modules/wr_mini_nic/wr_mini_nic.vhd                 \
			../../ip_cores/wr-cores/modules/wr_mini_nic/xwr_mini_nic.vhd                \
			../../ip_cores/wr-cores/modules/wr_mini_nic/minic_wb_slave.vhd              \
			../../ip_cores/wr-cores/modules/wr_mini_nic/minic_wb_slave.vhd              \
			../../ip_cores/wr-cores/modules/wrc_core/wrc_periph.vhd                     \
			../../ip_cores/wr-cores/modules/fabric/xwrf_mux.vhd                         \
			../common/wr_core.vhd                        \
			../../ip_cores/wr-cores/modules/wrc_core/xwr_core.vhd                       \
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
			../common/wr_timing.vhd      \
			testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $?
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
# It is needed to run the auto generated Vlm32_top.mk makefile which generates the 
# lm32 library that is linked together with the GHDL object files into the final
# testbench executable.
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

