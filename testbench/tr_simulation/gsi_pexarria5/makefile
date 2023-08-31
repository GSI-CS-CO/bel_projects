GHDL_INTEL=`pwd`/intel
GHDL_INTEL_OBJ=$(GHDL_INTEL)/altera_mf/v93/altera_mf.o
BEL_PROJECTS_DIR=../../..

GHDLFLAGS = --ieee=synopsys --std=93c \
	-fexplicit -frelaxed-rules --no-vital-checks --warn-binding --mb-comments \
	-P$(GHDL_INTEL)/altera/v93      \
	-P$(GHDL_INTEL)/altera_lnsim/v93\
	-P$(GHDL_INTEL)/altera_mf/v93   \
	-P$(GHDL_INTEL)/arriav/v93      \
	-P$(GHDL_INTEL)/arriaii/v93     \
	-P$(GHDL_INTEL)/lpm/v93         \
	-P$(GHDL_INTEL)/sgate/v93       \
	-P$(GHDL_INTEL)

all: testbench

intel-vendor-lib: ${QUARTUS_PATH}/quartus/eda/sim_lib/altera_mf.vhd
	/usr/lib/ghdl/vendors/compile-altera.sh --skip-largefiles --all --vhdl93 --source ${QUARTUS_PATH}/quartus/eda/sim_lib --output $(GHDL_INTEL)
	touch intel-vendor-lib

run: testbench 
	ghdl -r testbench --stop-time=1000us --wave=simulation.ghw --ieee-asserts=disable -gg_en_simbridge=false

notrace: testbench 
	ghdl -r testbench --ieee-asserts=disable

testbench: intel-vendor-lib \
           vhdl_sources \
           $(shell cat vhdl_sources) \
           ref_pll.vhd   ref_pll_5_10.vhd   dmtd_pll.vhd   dmtd_pll_5_10.vhd   sys_pll.vhd    sys_pll_5_10.vhd   single_region.vhd   global_region.vhd    \
           simbridge_pkg_c.o ez_usb_dev_c.o ez_usb_dev.vhd ez_usb_chip.vhd wr_timing.vhd \
           .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd  \
           .gvi/lm32_top_full/lm32_top_full_wrapper.vhd  \
           .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd  \
           .gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd  \
           .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd  \
           testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $(shell cat vhdl_sources) $(filter-out vhdl_sources simbridge_pkg_c.o ez_usb_dev_c.o intel-vendor-lib, $+) # filter out all object file dependencies... ghdl doesn't like them as input in -a stage
	ghdl -m $(GHDLFLAGS) \
         $(shell cat .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_full/lm32_top_full_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium/lm32_top_medium_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.flags)  \
	     $(shell cat .gvi/common.flags) \
	     -Wl,simbridge_pkg_c.o \
	     -Wl,ez_usb_dev_c.o \
	     testbench 

##
simbridge_pkg_c.o: ./eb_sim_core/simbridge_pkg_c.cpp
	g++ -fPIC -c $+

ez_usb_dev_c.o: ez_usb_dev_c.c
	gcc -fPIC -c $+

## auto generate lm32 vhdl wrapper 
.gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_full_debug -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_full_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd

.gvi/lm32_top_full/lm32_top_full_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_full -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_full is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_full/lm32_top_full_wrapper.vhd

.gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_medium_icache_debug -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_icache_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd

.gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_medium_debug -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd

.gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_medium_icache -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_icache is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd

.gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_medium -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd

.gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -G sdb_address=1024 -G eba_reset=0x10000000 -v lm32_allprofiles.v -t lm32_top_minimal -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/src -I $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_minimal is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd

gvi: gvi.cpp

#lm32 "interrupt" is a C++ keyword => the file must be patched so that Verilator can handle it
lm32_allprofiles.v: $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_lm32/generated/lm32_allprofiles.v
	sed    '/input \[ (32-1):0\] interrupt;/i \/\* verilator lint_off SYMRSVDWORD \*\/' $< > $@
	sed -i '/input \[ (32-1):0\] interrupt;/a \/\* verilator lint_on  SYMRSVDWORD \*\/' $@

# modify the list of VHDL source files ( exclude certain files or replace them with patched files)
vhdl_sources: hdl_sources wb_irq_slave.vhd ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd monster_pkg.vhd io_control_pkg.vhd io_control.vhd wb_mil_scu_pkg.vhd wb_mil_scu.vhd wb_scu_bus.vhd wb_irq_scu_bus.vhd
	grep -r '.vhd$$' $<                         | \
	grep -v wb_pmc_host_bridge.vhd              | \
	grep -v VME_CR_CSR_Space.vhd                | \
	grep -v VME_IRQ_Controller.vhd              | \
	grep -v VME_Wb_master_eb.vhd                | \
	grep -v xVME64xCore_Top.vhd                 | \
	grep -v asmi10.vhd                          | \
	grep -v wb_asmi.vhd                         | \
	grep -v arria10_reset.vhd                   | \
	grep -v mil_en_decoder.vhd                  | \
	grep -v mil_hw_or_soft_ip.vhd               | \
	grep -v event_processing.vhd                | \
	grep -v wb_temp_sense.vhd                   | \
	grep -v hw6408_vhdl.vhd                     | \
	sed '/\/wb_mil_scu_pkg.vhd/cwb_mil_scu_pkg.vhd' | \
	sed '/\/wb_mil_scu.vhd/cwb_mil_scu.vhd'     | \
	sed '/\/wb_irq_scu_bus.vhd/cwb_irq_scu_bus.vhd' | \
	sed '/\/wb_scu_bus.vhd/cwb_scu_bus.vhd'     | \
	sed '/\/io_control_pkg.vhd/cio_control_pkg.vhd' | \
	sed '/\/io_control.vhd/cio_control.vhd'     | \
	sed '/\/wb_irq_slave.vhd/cwb_irq_slave.vhd' | \
	sed '/\/wb_arria_reset.vhd/cwb_arria_reset.vhd' | \
	sed '/\/monster_pkg.vhd/cmonster_pkg.vhd'   | \
	sed '/\/monster.vhd/cmonster.vhd'           | \
	sed '/\/eca_sdp.vhd/ceca_sdp.vhd'           | \
	sed '/\/eca_tdp.vhd/ceca_tdp.vhd'           | \
	sed '/\/ep_rx_buffer.vhd/cep_rx_buffer.vhd' | \
	sed '/\/wr_core.vhd/cwr_core.vhd'           > $@

verilog_sources: hdl_sources
	grep -r '.v$$' $< > $@

hdl_sources: 
	hdlmake list-files > hdl_sources

# patches for certain source files
ep_rx_buffer.vhd: $(BEL_PROJECTS_DIR)/ip_cores/wr-cores/modules/wr_endpoint/ep_rx_buffer.vhd
	sed '/signal cur_addr  : in  std_logic_vector;/csignal cur_addr  : in  std_logic_vector(1 downto 0);' $< > $@

wr_core.vhd: $(BEL_PROJECTS_DIR)/ip_cores/wr-cores/modules/wrc_core/wr_core.vhd
	sed "/dpram_wbb_i.adr <= /cdpram_wbb_i.adr <= \(others => \'0\'\);" $< > $@

# remove the assertions that trigger if an address has 'X'es in it
eca_tdp.vhd: $(BEL_PROJECTS_DIR)/ip_cores/wr-cores/modules/wr_eca/eca_tdp.vhd
	sed "s/assert/--assert/g" $< > $@
	sed -i "s/report/--report/g" $@
	sed -i "s/severity/--severity/g" $@

eca_sdp.vhd: $(BEL_PROJECTS_DIR)/ip_cores/wr-cores/modules/wr_eca/eca_sdp.vhd
	sed "s/assert/--assert/g" $< > $@
	sed -i "s/report/--report/g" $@
	sed -i "s/severity/--severity/g" $@
	sed -i "s/bug :/--bug :/g" $@

monster_pkg.vhd: $(BEL_PROJECTS_DIR)/modules/monster/monster_pkg.vhd
	sed "s/t_io_mapping_table_arg_array(natural range <>)/t_io_mapping_table_arg_array/g" $< > $@

io_control_pkg.vhd: $(BEL_PROJECTS_DIR)/modules/io_control/src/hdl/io_control_pkg.vhd
	sed "s/t_io_mapping_table_arg_array(natural range <>)/t_io_mapping_table_arg_array/g" $< > $@

io_control.vhd: $(BEL_PROJECTS_DIR)/modules/io_control/src/hdl/io_control.vhd
	sed "s/t_io_mapping_table_arg_array(natural range <>)/t_io_mapping_table_arg_array/g" $< > $@

wb_mil_scu_pkg.vhd: $(BEL_PROJECTS_DIR)/modules/wb_mil_scu/wb_mil_scu_pkg.vhd
	sed "s/t_sdb_component.addr_last'length/16*4/g" $< > $@

wb_mil_scu.vhd: $(BEL_PROJECTS_DIR)/modules/wb_mil_scu/wb_mil_scu.vhd
	sed "s/Reset   => NOT nRst_i/Reset   => Rst_i/g" $< > $@
	sed -i "s/signal   task_runs:                 std_logic;/signal   task_runs:                 std_logic; signal Rst_i:               STD_LOGIC;/g" $@
	sed -i "s/reset_6408                <= '0';/reset_6408                <= '0'; Rst_i <= not nRst_i;/g" $@
	sed -i "s/signal   n_modulreset:              std_logic;/signal   n_modulreset:              std_logic; signal modulreset: std_logic; signal tx_req_not_tx_fifo_empty : std_logic_vector(255 downto 0);/g" $@
	sed -i "s/=>  not n_modulreset/=>  modulreset/g" $@
	sed -i "s/input => tx_req & not tx_fifo_empty/input => tx_req_not_tx_fifo_empty/g" $@
	sed -i "s/prio_enc:/tx_req_not_tx_fifo_empty <= tx_req \& not tx_fifo_empty;\n modulreset<=not n_modulreset; \nprio_enc:/g" $@

wb_scu_bus.vhd: $(BEL_PROJECTS_DIR)/modules/scu_bus/wb_scu_bus.vhd
	sed "s/unsigned(s_adr(c_adr_width-1 downto 0))/s_adr_c_adr/g" $< > $@
	sed -i "s/signal    s_sw_tag            : std_logic;/signal    s_sw_tag            : std_logic;\n    signal s_adr_c_adr : unsigned(c_adr_width-1 downto 0);/g" $@
	sed -i "s/end process p_wb_ctrl;/end process p_wb_ctrl;\n s_adr_c_adr <= unsigned(s_adr(c_adr_width-1 downto 0));/g" $@
	sed -i "s/unsigned(c_adr_width-1 DOWNTO 0) := to_unsigned(16#/unsigned(15 downto 0) := x\"/g" $@
	sed -i "s/#, c_adr_width);	-- real address is multiplied by two/\";/g" $@

wb_irq_scu_bus.vhd: $(BEL_PROJECTS_DIR)/modules/scu_bus/wb_irq_scu_bus.vhd
	sed "s/slave_o            => scu_slave_o/slave_o            => open/g" $< > $@

# patch for wb_irq_slave to prevent out of bounds error
wb_irq_slave.vhd: $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_irq/wb_irq_slave.vhd
	sed "s/irq_q(queue_offs)/irq_q(queue_offs mod irq_q'length)/g" $< > $@

clean:
	rm -f ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd *.o hdl_sources verilog_sources vhdl_sources work-obj93.cf
	rm -rf .gvi $(GHDL_INTEL) intel-vendor-lib




