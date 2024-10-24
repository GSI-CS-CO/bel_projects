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
	ghdl -r testbench --stop-time=1000us --wave=simulation.ghw --ieee-asserts=disable

notrace: testbench 
	ghdl -r testbench --ieee-asserts=disable

firmware/firmware.mif: firmware/*.c firmware/*.h
	PATH=$(shell pwd)/../../../lm32-toolchain/bin:${PATH} make -C firmware

testbench: intel-vendor-lib \
           firmware/firmware.mif \
           vhdl_sources \
           $(shell cat vhdl_sources) \
           ref_pll.vhd   ref_pll_5_10.vhd   dmtd_pll.vhd   dmtd_pll_5_10.vhd   sys_pll.vhd    sys_pll_5_10.vhd   single_region.vhd   global_region.vhd    \
           simbridge_pkg_c.o \
           .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd  \
           .gvi/lm32_top_full/lm32_top_full_wrapper.vhd  \
           .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd  \
           .gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd  \
           .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd  \
           testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $(shell cat vhdl_sources) $(filter-out vhdl_sources simbridge_pkg_c.o firmware/firmware.mif intel-vendor-lib, $+) # filter out all object file dependencies... ghdl doesn't like them as input in -a stage
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
	     testbench 

##
simbridge_pkg_c.o: ./eb_sim_core/simbridge_pkg_c.cpp
	g++ -fPIC -c $+

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
vhdl_sources: hdl_sources wb_irq_slave.vhd ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd 
	grep -r '.vhd$$' $<                         | \
	grep -v wb_pmc_host_bridge.vhd              | \
	grep -v VME_CR_CSR_Space.vhd                | \
	grep -v VME_IRQ_Controller.vhd              | \
	grep -v VME_Wb_master_eb.vhd                | \
	grep -v xVME64xCore_Top.vhd                 | \
	grep -v asmi10.vhd                          | \
	grep -v asmi10_pkg.vhd                      | \
	grep -v arria10_reset.vhd                   | \
	grep -v mil_en_decoder.vhd                  | \
	grep -v wb_mil_scu_pkg.vhd                  | \
	grep -v wb_mil_scu.vhd                      | \
	grep -v monster.vhd                         | \
	grep -v monster_pkg.vhd                     | \
	grep -v beam_dump.vhd                       | \
	grep -v beam_dump_pkg.vhd                   | \
	sed '/\/wb_arria_reset.vhd/cwb_arria_reset.vhd' | \
	sed '/\/wb_irq_slave.vhd/cwb_irq_slave.vhd' | \
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

# patch for wb_irq_slave to prevent out of bounds error
wb_irq_slave.vhd: $(BEL_PROJECTS_DIR)/ip_cores/general-cores/modules/wishbone/wb_irq/wb_irq_slave.vhd
	sed "s/irq_q(queue_offs)/irq_q(queue_offs mod irq_q'length)/g" $< > $@


interact: interact.cpp
	g++ -o $@ $< `pkg-config saftlib --libs --cflags` 

clean:
	rm -f wb_irq_slave.vhd eca_sdp.vhd eca_tdp.vhd lm32_allprofiles.v
	rm -f Vlm32_top_medium_icache_debug_vlt_dump_00.vcd simulation.ghw uart_output.txt gvi testbench ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd *.o hdl_sources verilog_sources vhdl_sources work-obj93.cf
	rm -rf .gvi $(GHDL_INTEL) intel-vendor-lib
	rm -f interact
	make -C firmware clean




