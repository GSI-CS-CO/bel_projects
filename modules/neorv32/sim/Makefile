# Generic settings
# ==============================================================================
GHDL_BIN=ghdl
GHDL_FLAGS="--ieee=synopsys --warn-no-vital-generic --std=08 --Wall
GTWK_BIN=gtkwave

# Simulation settings
# ==============================================================================
STOP_TIME="1000us"
WORK_DIR=build
GHW_NAME="neorv32_tb_slim.ghw"
GTKW_NAME="neorv32_tb_slim.gtkw"
GHW_NAME_XBAR="neorv32_tb_xbar.ghw"
GTKW_NAME_XBAR="neorv32_tb_xbar.gtkw"


# Files
# ==============================================================================
ALTERA_MF_FILES:=	$(QUARTUS)/eda/sim_lib/altera_mf_components.vhd \
$(QUARTUS)/eda/sim_lib/altera_mf.vhd

HDL_NEORV32_BASE:="../../../ip_cores/neorv32/rtl/core"

HDL_OFFSET:=../../../ip_cores/neorv32/

HDL_GEN_CORES_OFFSET:=../../../ip_cores/general-cores

HDL_TB_FILES:=hdl/neorv32_tb_slim.vhd
HDL_TB_FILES_XBAR:=hdl/neorv32_tb_xbar.vhd

HDL_UART_SIM_FILES:=$(HDL_OFFSET)sim/sim_uart_rx.vhd

HDL_SHELL_FILES:=../src/hdl/neorv32_shell_pkg.vhd \
../src/hdl/neorv32_shell.vhd

HDL_RTE_CORE_FILES:= \
$(HDL_NEORV32_BASE)/neorv32_package.vhd \
$(HDL_NEORV32_BASE)/neorv32_application_image.vhd \
$(HDL_NEORV32_BASE)/neorv32_bootloader_image.vhd \
$(HDL_NEORV32_BASE)/neorv32_boot_rom.vhd \
$(HDL_NEORV32_BASE)/neorv32_prim.vhd \
$(HDL_NEORV32_BASE)/neorv32_cache.vhd \
$(HDL_NEORV32_BASE)/neorv32_cfs.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_shifter.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_muldiv.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_bitmanip.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_fpu.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_cfu.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_cond.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_cp_crypto.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_hwtrig.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_alu.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_decompressor.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_control.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_lsu.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_pmp.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_regfile.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_frontend.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_counters.vhd \
$(HDL_NEORV32_BASE)/neorv32_tracer.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu_trace.vhd \
$(HDL_NEORV32_BASE)/neorv32_cpu.vhd \
$(HDL_NEORV32_BASE)/neorv32_debug_auth.vhd \
$(HDL_NEORV32_BASE)/neorv32_debug_dm.vhd \
$(HDL_NEORV32_BASE)/neorv32_debug_dtm.vhd \
$(HDL_NEORV32_BASE)/neorv32_dma.vhd \
$(HDL_NEORV32_BASE)/neorv32_gpio.vhd \
$(HDL_NEORV32_BASE)/neorv32_gptmr.vhd \
$(HDL_NEORV32_BASE)/neorv32_neoled.vhd \
$(HDL_NEORV32_BASE)/neorv32_onewire.vhd \
$(HDL_NEORV32_BASE)/neorv32_pwm.vhd \
$(HDL_NEORV32_BASE)/neorv32_sysinfo.vhd \
$(HDL_NEORV32_BASE)/neorv32_bus.vhd \
$(HDL_NEORV32_BASE)/neorv32_xbus.vhd \
$(HDL_NEORV32_BASE)/neorv32_sdi.vhd \
$(HDL_NEORV32_BASE)/neorv32_wdt.vhd \
$(HDL_NEORV32_BASE)/neorv32_uart.vhd \
$(HDL_NEORV32_BASE)/neorv32_spi.vhd \
$(HDL_NEORV32_BASE)/neorv32_trng.vhd \
$(HDL_NEORV32_BASE)/neorv32_twi.vhd \
$(HDL_NEORV32_BASE)/neorv32_slink.vhd \
$(HDL_NEORV32_BASE)/neorv32_sys.vhd \
$(HDL_NEORV32_BASE)/neorv32_imem.vhd \
$(HDL_NEORV32_BASE)/neorv32_dmem.vhd \
$(HDL_NEORV32_BASE)/neorv32_twd.vhd \
$(HDL_NEORV32_BASE)/neorv32_clint.vhd \
$(HDL_NEORV32_BASE)/neorv32_top.vhd

HDL_RTE_GENRAM_FILES:=$(HDL_GEN_CORES_OFFSET)/modules/genrams/genram_pkg.vhd \
                      $(HDL_GEN_CORES_OFFSET)/modules/genrams/altera/generic_dpram.vhd

HDL_RTE_WISHBONE_FILES:=$(HDL_GEN_CORES_OFFSET)/modules/wishbone/wishbone_pkg.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/wishbone/wb_dpram/xwb_dpram.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/wishbone/wb_slave_adapter/wb_slave_adapter.vhd\
                        $(HDL_GEN_CORES_OFFSET)/modules/wishbone/wb_crossbar/sdb_rom.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/wishbone/wb_crossbar/xwb_crossbar.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/wishbone/wb_crossbar/xwb_sdb_crossbar.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/genrams/generic/generic_sync_fifo.vhd \
                        $(HDL_GEN_CORES_OFFSET)/modules/genrams/common/inferred_sync_fifo.vhd

# Targets
# ==============================================================================
all: compile_code prepare analyze elaborate_run

xbar: compile_code_xbar prepare analyze_xbar elaborate_run_xbar

prepare:
	@if [ ! -d "$(WORK_DIR)" ]; then \
		mkdir $(WORK_DIR); \
	fi

compile_code:
	make -C ../src/sw/sim

compile_code_xbar:
	make -C ../src/sw/sim_xbar


analyze:
	@echo "Analyzing ..."
	# Building Altera MF ...
	$(GHDL_BIN) -a --work=altera_mf --workdir=build -Wno-hide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit \
	$(ALTERA_MF_FILES)
	# Building NEORV32 ...
	$(GHDL_BIN) -a --work=neorv32 --workdir=build -Whide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit -Pbuild \
	$(HDL_RTE_CORE_FILES)
	# Building testbench
	$(GHDL_BIN) -a --work=work --workdir=build -Whide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit -Pbuild \
	$(HDL_RTE_GENRAM_FILES) $(HDL_RTE_WISHBONE_FILES) $(HDL_SHELL_FILES) $(HDL_UART_SIM_FILES) $(HDL_TB_FILES)

analyze_xbar:
	@echo "Analyzing ..."
	# Building Altera MF ...
	$(GHDL_BIN) -a --work=altera_mf --workdir=build -Wno-hide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit \
	$(ALTERA_MF_FILES)
	# Building NEORV32 ...
	$(GHDL_BIN) -a --work=neorv32 --workdir=build -Whide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit -Pbuild \
	$(HDL_RTE_CORE_FILES)
	# Building testbench
	$(GHDL_BIN) -a --work=work --workdir=build -Whide -fsynopsys --ieee=synopsys --warn-no-vital-generic -fexplicit -Pbuild \
	$(HDL_RTE_GENRAM_FILES) $(HDL_RTE_WISHBONE_FILES) $(HDL_SHELL_FILES) $(HDL_UART_SIM_FILES) $(HDL_TB_FILES_XBAR)

elaborate_run:
	@echo "Elaborating ..."
	$(GHDL_BIN) --elab-run -fexplicit -fsynopsys -Pbuild --workdir=build work.neorv32_tb_slim --stop-time=$(STOP_TIME) --wave=$(WORK_DIR)/$(GHW_NAME) --ieee-asserts=disable

elaborate_run_xbar:
	@echo "Elaborating ..."
	$(GHDL_BIN) --elab-run -fexplicit -fsynopsys -Pbuild --workdir=build work.neorv32_tb_xbar --stop-time=$(STOP_TIME) --wave=$(WORK_DIR)/$(GHW_NAME_XBAR) --ieee-asserts=disable

view: all
	$(GTWK_BIN) $(WORK_DIR)/$(GHW_NAME) $(GTKW_NAME)

view_xbar: xbar
	$(GTWK_BIN) $(WORK_DIR)/$(GHW_NAME_XBAR) $(GTKW_NAME_XBAR)

clean:
	make -C ../src/sw/sim clean
	make -C ../src/sw/sim_xbar clean
	rm -rf build || true
	rm *ghw || true
	rm *text.out || true
	rm *.o || true
	rm *1st || true
