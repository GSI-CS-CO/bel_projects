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

testbench: $(shell cat vhdl_sources) testbench.vhd
	ghdl -a -g $(GHDLFLAGS) $+
	ghdl -m $(GHDLFLAGS) testbench 




vhdl_sources: hdl_sources
	grep -r '.vhd$$' $<                        | \
	grep -v wb_pmc_host_bridge.vhd             | \
	grep -v VME_CR_CSR_Space.vhd               | \
	grep -v VME_IRQ_Controller.vhd             | \
	grep -v VME_Wb_master_eb.vhd               | \
	grep -v xVME64xCore_Top.vhd                | \
	sed '/ep_rx_buffer.vhd/cep_rx_buffer.vhd'  | \
	sed '/wr_core.vhd/cwr_core.vhd'            > $@

verilog_sources: hdl_sources
	grep -r '.v$$' $< > $@

hdl_sources: 
	hdlmake list-files > hdl_sources

# patch for ep_rx_buffer.vhd
ep_rx_buffer.vhd: ../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_buffer.vhd
	sed '/signal cur_addr  : in  std_logic_vector;/csignal cur_addr  : in  std_logic_vector(1 downto 0);' ../../ip_cores/wr-cores/modules/wr_endpoint/ep_rx_buffer.vhd > ep_rx_buffer.vhd

wr_core.vhd: /home/michael/work/bel_projects/ip_cores/wr-cores/modules/wrc_core/wr_core.vhd
	sed "/dpram_wbb_i.adr <= /cdpram_wbb_i.adr <= \(others => \'0\'\);" ../../ip_cores/wr-cores/modules/wrc_core/wr_core.vhd > wr_core.vhd

