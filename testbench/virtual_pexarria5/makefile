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


all: vhdl_sources verilog_sources testbench

testbench: \
            $(shell cat vhdl_sources) \
           ref_pll.vhd   ref_pll_5_10.vhd   dmtd_pll.vhd   dmtd_pll_5_10.vhd   sys_pll.vhd    sys_pll_5_10.vhd   single_region.vhd   global_region.vhd    \
           ez_usb_dev_c.o ez_usb_dev.vhd ez_usb_chip.vhd wr_timing.vhd \
           .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd  \
           .gvi/lm32_top_full/lm32_top_full_wrapper.vhd  \
           .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd  \
           .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd  \
           .gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd  \
           .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd  \
           testbench.vhd 
	ghdl -a -g $(GHDLFLAGS) $(shell cat vhdl_sources) $(filter-out ez_usb_dev_c.o, $+) 
	ghdl -m $(GHDLFLAGS) \
         $(shell cat .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_full/lm32_top_full_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_medium/lm32_top_medium_wrapper.flags)  \
         $(shell cat .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.flags)  \
	     $(shell cat .gvi/common.flags) \
	     -Wl,ez_usb_dev_c.o \
	     testbench 

##
ez_usb_dev_c.o: ez_usb_dev_c.c
	gcc -c $+

## auto generate lm32 vhdl wrapper 
.gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_full_debug -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_full_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_full_debug/lm32_top_full_debug_wrapper.vhd

.gvi/lm32_top_full/lm32_top_full_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_full -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_full is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_full/lm32_top_full_wrapper.vhd

.gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_medium_icache_debug -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_icache_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_icache_debug/lm32_top_medium_icache_debug_wrapper.vhd

.gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_medium_debug -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_debug is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_debug/lm32_top_medium_debug_wrapper.vhd

.gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_medium_icache -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium_icache is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium_icache/lm32_top_medium_icache_wrapper.vhd

.gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_medium -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_medium is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_medium/lm32_top_medium_wrapper.vhd

.gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd: lm32_allprofiles.v gvi
	./gvi -v lm32_allprofiles.v -t lm32_top_minimal -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/src -I ../../ip_cores/general-cores/modules/wishbone/wb_lm32/platform/generic
	sed -i '/entity lm32_top_minimal is/ageneric(eba_reset: std_logic_vector(31 downto 0);sdb_address: std_logic_vector(31 downto 0));' .gvi/lm32_top_minimal/lm32_top_minimal_wrapper.vhd



#lm32 
lm32_allprofiles.v: ../../ip_cores/general-cores/modules/wishbone/wb_lm32/generated/lm32_allprofiles.v
	sed    '/input \[ (32-1):0\] interrupt;/i \/\* verilator lint_off SYMRSVDWORD \*\/' $< > $@
	sed -i '/input \[ (32-1):0\] interrupt;/a \/\* verilator lint_on  SYMRSVDWORD \*\/' $@

# modify the list of VHDL source files ( exclude certain files or replace them with patched files)
vhdl_sources: hdl_sources ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd
	grep -r '.vhd$$' $<                         | \
	grep -v wb_pmc_host_bridge.vhd              | \
	grep -v VME_CR_CSR_Space.vhd                | \
	grep -v VME_IRQ_Controller.vhd              | \
	grep -v VME_Wb_master_eb.vhd                | \
	grep -v xVME64xCore_Top.vhd                 | \
	sed '/\/eca_sdp.vhd/ceca_sdp.vhd'           | \
	sed '/\/eca_tdp.vhd/ceca_tdp.vhd'           | \
	sed '/\/ep_rx_buffer.vhd/cep_rx_buffer.vhd' | \
	sed '/\/wr_core.vhd/cwr_core.vhd'           > $@

verilog_sources: hdl_sources
	grep -r '.v$$' $< > $@

hdl_sources: 
	hdlmake list-files > hdl_sources

# patches for certain source files
ep_rx_buffer.vhd: ../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_buffer.vhd
	sed '/signal cur_addr  : in  std_logic_vector;/csignal cur_addr  : in  std_logic_vector(1 downto 0);' $< > $@

wr_core.vhd: ../../ip_cores/wr-cores/modules/wrc_core/wr_core.vhd
	sed "/dpram_wbb_i.adr <= /cdpram_wbb_i.adr <= \(others => \'0\'\);" $< > $@


# remove the assertions that trigger if an address has 'X'es in it
eca_tdp.vhd: ../../ip_cores/wr-cores/modules/wr_eca/eca_tdp.vhd
	sed "s/assert/--assert/g" $< > $@
	sed -i "s/report/--report/g" $@
	sed -i "s/severity/--severity/g" $@

eca_sdp.vhd: ../../ip_cores/wr-cores/modules/wr_eca/eca_sdp.vhd
	sed "s/assert/--assert/g" $< > $@
	sed -i "s/report/--report/g" $@
	sed -i "s/severity/--severity/g" $@
	sed -i "s/bug :/--bug :/g" $@



clean:
	rm -f ep_rx_buffer.vhd wr_core.vhd eca_tdp.vhd eca_sdp.vhd *.o hdl_sources verilog_sources vhdl_sources work-obj93.cf
	rm -rf .gvi




