library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.pcie_wb_pkg.all;
use work.lpc_uart_pkg.all;
use work.wr_altera_pkg.all;
use work.trans_pkg.all;
use work.scu_bus_pkg.all;
use work.gencores_pkg.all;
use work.cfi_flash_pkg.all;
use work.wb_mil_scu_pkg.all;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;

entity scu_bus is
  port(
		 -----------------------------------------
		 -- Clocking pins
		 -----------------------------------------
		 clk125_i : in std_logic;

		 -----------------------------------------
		 -- PCI express pins
		 -----------------------------------------
		 pcie_refclk_i : in  std_logic;
		 pcie_rx_i     : in  std_logic_vector(3 downto 0);
		 pcie_tx_o     : out std_logic_vector(3 downto 0);
			
		 -----------------------------------------------------------------------
		 -- LEDs
		 -----------------------------------------------------------------------
		 nuser_led:					out 	std_logic_vector(4 downto 1);
		 a_lemo_1_led:			out		std_logic;
		 a_lemo_1_en_in:		out		std_logic;
		 a_lemo_2_led:			out		std_logic;
		 a_lemo_2_en_in:		out		std_logic;
		 naux_sfp_grn:			out		std_logic;
		 naux_sfp_red:			out		std_logic;
		 ntiming_sfp_grn:		out		std_logic;
		 ntiming_sfp_red:		out		std_logic;
		 
		 -----------------------------------------------------------------------
		 -- Lemo IOs
		 -----------------------------------------------------------------------
		 a_lemo_1_io:				inout	std_logic;
		 a_lemo_2_io:				inout	std_logic;
		 
		 -----------------------------------------------------------------------
		 -- OneWire
		 -----------------------------------------------------------------------
		 onewire_cb:				inout std_logic;
	 
		-----------------------------------------------------------------------
		-- Parallel Flash
		-----------------------------------------------------------------------
		ad:									out		std_logic_vector(25 downto 1);
		df:									inout std_logic_vector(15 downto 0);
		adv_fsh:						out		std_logic;
		nce_fsh:						out		std_logic;
		clk_fsh:						out		std_logic;
		nwe_fsh:						out		std_logic;
		noe_fsh:						out		std_logic;
		nrst_fsh:						out		std_logic;
		wait_fsh:						in		std_logic;
		
		-----------------------------------------------------------------------
		-- LPC interface from ComExpress
		-----------------------------------------------------------------------
		lpc_ad:							inout std_logic_vector(3 downto 0);
		lpc_fpga_clk:				in		std_logic;
		lpc_serirq:					inout std_logic;
		nlpc_drq0:					in		std_logic;
		nlpc_frame:					in		std_logic;
		npci_reset:					in		std_logic;
		
		-----------------------------------------
    -- UART on front panel
    -----------------------------------------
    uart_rxd_i:					in  std_logic_vector(1 downto 0);
    uart_txd_o:					out std_logic_vector(1 downto 0);

    serial_to_cb_o:			out std_logic;
      
    -----------------------------------------------------------------------
    -- SCU Bus
    -----------------------------------------------------------------------
    a_d:								inout std_logic_vector(15 downto 0);
    a_a:								out   std_logic_vector(15 downto 0);
    a_ntiming_cycle:		out   std_logic;
    a_nds:							out   std_logic;
    a_nreset:						out   std_logic;
    nsel_ext_data_drv:	out   std_logic;
    a_rnw:							out   std_logic;
    a_spare:						out   std_logic_vector(1 downto 0);
    a_nsel:							out   std_logic_vector(12 downto 1);
    a_ndtack:						in    std_logic;
    a_nsrq:							in    std_logic_vector(12 downto 1);
    a_sysclock:					out   std_logic;
    adr_to_scub:				out   std_logic;
    nadr_en:						out   std_logic;
    a_onewire:					inout std_logic;
 

    -----------------------------------------------------------------------
    -- Ext_Conn2
    -----------------------------------------------------------------------
		-- io_2_5v(0)		-> EXT_CONN2 pin b4
		-- io_2_5v(1)		-> EXT_CONN2 pin b5
		-- io_2_5v(2)		-> EXT_CONN2 pin b6
		-- io_2_5v(3)		-> EXT_CONN2 pin b7
		-- io_2_5v(4)		-> EXT_CONN2 pin b8
		-- io_2_5v(5)		-> EXT_CONN2 pin b9
		-- io_2_5v(6)		-> EXT_CONN2 pin b10
		-- io_2_5v(7)		-> EXT_CONN2 pin b11
		-- io_2_5v(8)		-> EXT_CONN2 pin b14
		-- io_2_5v(9)		-> EXT_CONN2 pin b15
		-- io_2_5v(10)	-> EXT_CONN2 pin b16
		-- io_2_5v(11)	-> EXT_CONN2 pin b17
		-- io_2_5v(12)	-> EXT_CONN2 pin b18
		-- io_2_5v(13)	-> EXT_CONN2 pin b19
		-- io_2_5v(14)	-> EXT_CONN2 pin b20
		-- io_2_5v(15)	-> EXT_CONN2 pin b21
    io_2_5v:        		inout   std_logic_vector(15 downto 0);
    
		-- eio(0)				-> EXT_CONN2 pin a4
		-- eio(1)				-> EXT_CONN2 pin a5
		-- eio(2)				-> EXT_CONN2 pin a6
		-- eio(3)				-> EXT_CONN2 pin a7
		-- eio(4)				-> EXT_CONN2 pin a8
		-- eio(5)				-> EXT_CONN2 pin a9
		-- eio(6)				-> EXT_CONN2 pin a10
		-- eio(7)				-> EXT_CONN2 pin a11
		-- eio(8)				-> EXT_CONN2 pin a14
		-- eio(9)				-> EXT_CONN2 pin a15
		-- eio(10)			-> EXT_CONN2 pin a16
		-- eio(11)			-> EXT_CONN2 pin a17
		-- eio(12)			-> EXT_CONN2 pin a18
		-- eio(13)			-> EXT_CONN2 pin a19
		-- eio(14)			-> EXT_CONN2 pin a20
		-- eio(15)			-> EXT_CONN2 pin a21
		-- eio(16)			-> EXT_CONN2 pin a23
		-- eio(17)			-> EXT_CONN2 pin a24
    eio:            		inout   std_logic_vector(17 downto 0);
	
		onewire_ext:				inout		std_logic;

    -----------------------------------------------------------------------
    -- AUX SFP 
    -----------------------------------------------------------------------
    sfp1_tx_disable_o:	out 	std_logic := '0';
		sfp1_tx_fault:			in		std_logic;
		sfp1_los:						in		std_logic;
		
    sfp1_txp_o:					out 	std_logic;
    sfp1_rxp_i:					in  	std_logic;
    
    sfp1_mod0:					in    std_logic; -- grounded by module
    sfp1_mod1:					inout std_logic; -- SCL
    sfp1_mod2:					inout std_logic; -- SDA

    -----------------------------------------------------------------------
    -- Timing SFP 
    -----------------------------------------------------------------------
		sfp2_tx_fault:			in		std_logic;
		sfp2_los:						in		std_logic;
    
    sfp2_tx_disable_o:	out 	std_logic := '0';
    sfp2_txp_o:					out 	std_logic;
    sfp2_rxp_i:					in  	std_logic;
    
    sfp2_mod0:					in    std_logic; -- grounded by module
    sfp2_mod1:					inout std_logic; -- SCL
    sfp2_mod2:					inout std_logic; -- SDA

 
    -----------------------------------------------------------------------
    -- Ext_Conn3
    -----------------------------------------------------------------------
    A_EXT_LVDS_RX_p0: in   std_logic;    -- 2.5V IO to pin b4 of EXT_CONN3, name is hint for possible use as differtial input
   -- A_EXT_LVDS_RX_n0: in   std_logic;    -- 2.5V IO to pin b5 of EXT_CONN3, name is hint for possible use as differtial input
    A_EXT_LVDS_RX_p1: in   std_logic;    -- 2.5V IO to pin b8 of EXT_CONN3, name is hint for possible use as differtial input
   -- A_EXT_LVDS_RX_n1: in   std_logic;    -- 2.5V IO to pin b9 of EXT_CONN3, name is hint for possible use as differtial input
    A_EXT_LVDS_RX_p2: in   std_logic;    -- 2.5V IO to pin b12 of EXT_CONN3, name is hint for possible use as differtial input
   -- A_EXT_LVDS_RX_n2: in   std_logic;    -- 2.5V IO to pin b13 of EXT_CONN3, name is hint for possible use as differtial input
    A_EXT_LVDS_RX_p3: in   std_logic;    -- 2.5V IO to pin b16 of EXT_CONN3, name is hint for possible use as differtial input
   -- A_EXT_LVDS_RX_n3: in   std_logic;    -- 2.5V IO to pin b17 of EXT_CONN3, name is hint for possible use as differtial input
		A_EXT_LVDS_CLKIN_p:	in		std_logic;    -- 2.5V in to pin b20 of EXT_CONN3, name is hint for possible use as differtial input	
	--	A_EXT_LVDS_CLKIN_n:	in		std_logic;    -- 2.5V in to pin b21 of EXT_CONN3, name is hint for possible use as differtial input
		
    A_EXT_LVDS_TX_p0: out   std_logic;    -- 2.5V IO to pin a2 of EXT_CONN3, name is hint for possible use as differtial output
   -- A_EXT_LVDS_TX_n0: out   std_logic;    -- 2.5V IO to pin a3 of EXT_CONN3, name is hint for possible use as differtial output
    A_EXT_LVDS_TX_p1: out   std_logic;    -- 2.5V IO to pin a6 of EXT_CONN3, name is hint for possible use as differtial output
   -- A_EXT_LVDS_TX_n1: out   std_logic;    -- 2.5V IO to pin a7 of EXT_CONN3, name is hint for possible use as differtial output
    A_EXT_LVDS_TX_p2: out   std_logic;    -- 2.5V IO to pin a10 of EXT_CONN3, name is hint for possible use as differtial output
   -- A_EXT_LVDS_TX_n2: out   std_logic;    -- 2.5V IO to pin a11 of EXT_CONN3, name is hint for possible use as differtial output
    A_EXT_LVDS_TX_p3: out   std_logic;    -- 2.5V IO to pin a14 of EXT_CONN3, name is hint for possible use as differtial output
   -- A_EXT_LVDS_TX_n3: out   std_logic;    -- 2.5V IO to pin a15 of EXT_CONN3, name is hint for possible use as differtial output
		A_EXT_LVDS_CLKOUT_p:	out   std_logic;    -- 2.5V IO to pin a18 of EXT_CONN3, name is hint for possible use as differtial output	
	--	A_EXT_LVDS_CLKOUT_n:	out   std_logic;    -- 2.5V IO to pin a19 of EXT_CONN3, name is hint for possible use as differtial output

    
		-----------------------------------------------------------------------
    -- LA port
    -----------------------------------------------------------------------
    hpla_ch:						out std_logic_vector(15 downto 0);
    hpla_clk:						out std_logic; 
    
    -----------------------------------------------------------------------
    -- Board configuration
    -----------------------------------------------------------------------
    a_nconfig:					out   std_logic;          -- triggers reconfig
    npwrbtn:						out   std_logic;          -- Powerbutton for ComExpress
    nfpga_res_out:			out   std_logic := '1';   -- drives sys_reset
		
    -----------------------------------------------------------------------
    -- DDR3
    -----------------------------------------------------------------------
    ddr3_dq:						inout std_logic_vector(15 downto 0);
    DDR3_DM           : out   std_logic_vector( 1 downto 0);
    DDR3_BA           : out   std_logic_vector( 2 downto 0);
    ddr3_addr:					out   std_logic_vector(12 downto 0);
    DDR3_CS_n         : out   std_logic_vector( 0 downto 0);
--    DDR3_DQSn         : inout std_logic_vector(1 downto 0);
    DDR3_RES_n        : out   std_logic;
    DDR3_CKE          : out   std_logic_vector( 0 downto 0);
    DDR3_ODT          : out   std_logic_vector( 0 downto 0);
    DDR3_CAS_n        : out   std_logic;
    DDR3_RAS_n        : out   std_logic;
--    DDR3_CLK          : inout std_logic_vector(0 downto 0);
--    DDR3_CLK_n        : inout std_logic_vector(0 downto 0);
    DDR3_WE_n         : out   std_logic

	);
	 
end scu_bus;

architecture rtl of scu_bus is


  constant c_xwb_gpio32_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f", -- three 4 byte registers
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"35aa6b95",
    version       => x"00000001",
    date          => x"20120305",
    name          => "GSI_GPIO_32        ")));
	 
	 
	 constant c_xwb_owm : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"000000000000CE42", -- CERN
    device_id     => x"779c5443",
    version       => x"00000001",
    date          => x"20120603",
    name          => "WR-Periph-1Wire    ")));
	 
	 constant c_xwb_uart : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"000000000000CE42", -- CERN
    device_id     => x"e2d13d04",
    version       => x"00000001",
    date          => x"20120603",
    name          => "WR-Periph-UART     ")));
    
  constant c_scu_bus_master : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"02",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"2", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000003fffff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"9602eb6f",
    version       => x"00000001",
    date          => x"20120720",
    name          => "SCU-BUS-Master     ")));
    
   constant c_cfi_flash : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"2", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000effffff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"3245f450",
    version       => x"00000001",
    date          => x"20130116",
    name          => "CFI-flash          ")));
    
  -- Top crossbar layout
  constant c_slaves : natural := 8;
  constant c_masters : natural := 5;
  constant c_dpram_size : natural := 16384; -- in 32-bit words (64KB)
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size),  x"00000000"),
    1 => f_sdb_embed_device(c_xwb_gpio32_sdb,           x"00100400"),
    2 => f_sdb_embed_device(c_xwb_dma_sdb,              x"00100500"),
    3 => f_sdb_embed_device(c_xwb_owm,				 		      x"00100600"),
    4 => f_sdb_embed_device(c_xwb_uart,                 x"00100700"),
    5 => f_sdb_embed_device(c_scu_bus_master,           x"00400000"),
    6 => f_sdb_embed_device(c_cfi_flash,                x"10000000"),
    7 => f_sdb_embed_device(c_xwb_gsi_mil_scu,          x"20000000"));
  constant c_sdb_address : t_wishbone_address :=        x"00100000";

  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);

  signal clk_sys, clk_cal, rstn, locked : std_logic;
  signal clk_sys_rstn : std_logic;
  signal reset_clks : std_logic_vector(0 downto 0);
  signal reset_rstn : std_logic_vector(0 downto 0);
  signal lm32_interrupt : std_logic_vector(31 downto 0);
  signal lm32_rstn : std_logic;
  
  signal gpio_slave_o : t_wishbone_slave_out;
  signal gpio_slave_i : t_wishbone_slave_in;
  
  signal cfi_slave_o : t_wishbone_slave_out;
  signal cfi_slave_i : t_wishbone_slave_in;
  
  signal r_leds : std_logic_vector(7 downto 0);
  signal r_reset    : std_logic := '0';
  signal r_loopback : std_logic_vector(11 downto 0) := x"000";
  
  signal owr_pwren_o: std_logic_vector(1 downto 0);
  signal owr_en_o: std_logic_vector(1 downto 0);
  signal owr_i:	std_logic_vector(1 downto 0);
  
  signal rcfg_toloop:     std_logic_vector(3 downto 0);
  signal rx_drst:         std_logic_vector(1 downto 0);
  signal tx_drst:         std_logic_vector(1 downto 0);
  signal rcfg_fromloop:   std_logic_vector(50 downto 0);
  signal rx_bistdone:     std_logic_vector(1 downto 0);
  signal rx_bisterr:      std_logic_vector(1 downto 0);
  signal rx_signaldetect: std_logic_vector(1 downto 0);
  signal tx_dataout:      std_logic_vector(3 downto 0);
  
begin
--	Inst_flash_loader_v01 : flash_loader
--    port map (
--      noe_in   => '0'
--    );
	 
	-- open drain buffer for one wire
	owr_i(0) <= OneWire_CB;
	OneWire_CB <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';
	
  -- Obtain core clocking
  sys_pll_inst : sys_pll -- Altera megafunction
    port map (
      inclk0 => clk125_i,    -- 125Mhz oscillator from board
      c0     => clk_sys,     -- 125MHz system clk (cannot use external pin as clock for RAM blocks)
      c1     => clk_cal,     -- 50Mhz calibration clock for Altera reconfig cores
      locked => locked);     -- '1' when the PLL has locked
      
  reset : gc_reset
    port map(
      free_clk_i => clk125_i,
      locked_i   => locked,
      clks_i     => reset_clks,
      rstn_o     => reset_rstn);
  reset_clks(0) <= clk_sys;
  clk_sys_rstn <= reset_rstn(0);
  
  
  -- The top-most Wishbone B.4 crossbar
  interconnect : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves  => c_slaves,
     g_registered  => true,
     g_wraparound  => false, -- Should be true for nested buses
     g_layout      => c_layout,
     g_sdb_addr    => c_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => clk_sys_rstn,
     -- Master connections (INTERCON is a slave)
     slave_i       => cbar_slave_i,
     slave_o       => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => cbar_master_i,
     master_o      => cbar_master_o);
  
  -- Master 0 is the PCIe bridge
  PCIe : pcie_wb
    generic map(
      sdb_addr => c_sdb_address)
    port map(
      clk125_i      => clk125_i,      -- Free running clock
      cal_clk50_i   => clk_cal,       -- Transceiver global calibration clock
      pcie_refclk_i => pcie_refclk_i, -- External PCIe 100MHz bus clock
      pcie_rstn_i   => nPCI_RESET,    -- External PCIe system reset pin
      pcie_rx_i     => pcie_rx_i,
      pcie_tx_o     => pcie_tx_o,
      master_clk_i  => clk_sys,       -- Desired clock for the WB bus
      master_rstn_i => clk_sys_rstn,
      master_o      => cbar_slave_i(0),
      master_i      => cbar_slave_o(0));
  
   -- The LM32 is master 1+2
  lm32_rstn <= clk_sys_rstn and r_reset;
  LM32 : xwb_lm32
    generic map(
      g_profile => "medium_icache_debug") -- Including JTAG and I-cache (no divide)
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => lm32_rstn,
      irq_i     => lm32_interrupt,
      dwb_o     => cbar_slave_i(1), -- Data bus
      dwb_i     => cbar_slave_o(1),
      iwb_o     => cbar_slave_i(2), -- Instruction bus
      iwb_i     => cbar_slave_o(2));
  
  -- The other 31 interrupt pins are unconnected
  lm32_interrupt(31 downto 1) <= (others => '0');

  -- A DMA controller is master 3+4, slave 2, and interrupt 0
  dma : xwb_dma
    port map(
      clk_i       => clk_sys,
      rst_n_i     => clk_sys_rstn,
      slave_i     => cbar_master_o(2),
      slave_o     => cbar_master_i(2),
      r_master_i  => cbar_slave_o(3),
      r_master_o  => cbar_slave_i(3),
      w_master_i  => cbar_slave_o(4),
      w_master_o  => cbar_slave_i(4),
      interrupt_o => lm32_interrupt(0));
  
  -- Slave 0 is the RAM
  ram : xwb_dpram
    generic map(
      g_size                  => c_dpram_size,
      g_slave1_interface_mode => PIPELINED, -- Why isn't this the default?!
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => WORD)
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => clk_sys_rstn,
      -- First port connected to the crossbar
      slave1_i  => cbar_master_o(0),
      slave1_o  => cbar_master_i(0),
      -- Second port disconnected
      slave2_i  => cc_dummy_slave_in, -- CYC always low
      slave2_o  => open);
  
  
  -- Slave 1 is the example LED driver
  gpio_slave_i <= cbar_master_o(1);
  cbar_master_i(1) <= gpio_slave_o;
  nUSER_LED <= not r_leds(3 downto 0);
  
  -- There is a tool called 'wbgen2' which can autogenerate a Wishbone
  -- interface and C header file, but this is a simple example.
  gpio : process(clk_sys)
  begin
    if rising_edge(clk_sys) then
      -- It is vitally important that for each occurance of
      --   (cyc and stb and not stall) there is (ack or rty or err)
      --   sometime later on the bus.
      --
      -- This is an easy solution for a device that never stalls:
      gpio_slave_o.ack <= gpio_slave_i.cyc and gpio_slave_i.stb;
      
      -- Detect a write to the register byte
      if gpio_slave_i.cyc = '1' and gpio_slave_i.stb = '1' and
         gpio_slave_i.we = '1' and gpio_slave_i.sel(0) = '1' then
      -- Register 0x0 = LEDs, 0x4 = CPU reset
      if gpio_slave_i.adr(2) = '0' then
          r_leds <= gpio_slave_i.dat(7 downto 0);
      else
          r_reset <= gpio_slave_i.dat(0);
      end if;
      end if;
      
      if gpio_slave_i.adr(2) = '0' then
        gpio_slave_o.dat(31 downto 8) <= (others => '0');
        gpio_slave_o.dat(7 downto 0) <= r_leds;
      else
        gpio_slave_o.dat(31 downto 2) <= (others => '0');
        gpio_slave_o.dat(0) <= r_reset;
      end if;
    end if;
  end process;
  gpio_slave_o.int <= '0'; -- In my opinion, this should not be in the structure,
                           -- but it is in there. Bother Thomasz to remove it.
  gpio_slave_o.err <= '0';
  gpio_slave_o.rty <= '0';
  gpio_slave_o.stall <= '0'; -- This simple example is always ready

	
	--------------------------------------
  -- UART
  --------------------------------------
  UART : xwb_simple_uart
    generic map(
      g_with_virtual_uart   => false,
      g_with_physical_uart  => true,
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => clk_sys_rstn,

      -- Wishbone
      slave_i => cbar_master_o(4),
      slave_o => cbar_master_i(4),
      desc_o  => open,

      uart_rxd_i => uart_rxd_i(0),
      uart_txd_o => uart_txd_o(0)
      );

  --------------------------------------
  -- 1-WIRE
  --------------------------------------
  ONEWIRE : xwb_onewire_master
    generic map(
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE,
      g_num_ports           => 2,
      g_ow_btp_normal       => "5.0",
      g_ow_btp_overdrive    => "1.0"
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => clk_sys_rstn,

      -- Wishbone
      slave_i => cbar_master_o(3),
      slave_o => cbar_master_i(3),
      desc_o  => open,

		owr_pwren_o => owr_pwren_o,
      owr_en_o => owr_en_o,
      owr_i    => owr_i
      );
  
	
--	-- needed for starting up the ComExpress module
--	lpc_slave: lpc_uart
--		port map (
--      lpc_clk => LPC_FPGA_CLK,
--			lpc_serirq => LPC_SERIRQ,
--			lpc_ad => LPC_AD,
--			lpc_frame_n => nLPC_FRAME,
--			lpc_reset_n => nPCI_RESET,
--			serial_rxd => uart_rxd_i(1),
--			serial_txd => uart_txd_o(1),
--			serial_dtr => open,
--			serial_dcd => '0',
--			serial_dsr => '0',
--			serial_ri => '0',
--			serial_cts => '0',
--			serial_rts => open,
--			seven_seg_L => open,
--			seven_seg_H => open
--			);
  
--  trans_loop_ql0 : trans_loop 
--  GENERIC MAP (
--    starting_channel_number => 0
--  )
--  PORT MAP (
--    cal_blk_clk	 => clk_cal,
--		pll_inclk	 => clk_sys,
--		reconfig_clk	 => clk_cal,
--		reconfig_togxb	 => rcfg_toloop,
--		rx_datain	 => QL0_GXB_RX,
--		rx_digitalreset	 => (others => r_reset),
--    rx_seriallpbken  => r_loopback(3 downto 0),
--		tx_digitalreset	 => (others => r_reset),
--		reconfig_fromgxb	 => rcfg_fromloop(16 downto 0),
--		rx_bistdone	 => r_pio(3 downto 0),
--		rx_bisterr	 => r_pio(7 downto 4),
--		rx_clkout	 => open,
--		rx_signaldetect	 => open,
--		tx_clkout	 => open,
--		tx_dataout	 => QL0_GXB_TX
--	);
  
--  trans_loop_ql1 : trans_loop 
--  GENERIC MAP (
--    starting_channel_number => 4
--  )
--  PORT MAP (
--    cal_blk_clk	 => clk_cal,
--		pll_inclk	 => clk_sys,
--		reconfig_clk	 => clk_cal,
--		reconfig_togxb	 => rcfg_toloop,
--		rx_datain	 => QL1_GXB_RX,
--		rx_digitalreset	 => (others => r_reset),
--		rx_seriallpbken  => r_loopback(7 downto 4),
--    tx_digitalreset	 => (others => r_reset),
--		reconfig_fromgxb	 => rcfg_fromloop(33 downto 17),
--		rx_bistdone	 => r_pio(11 downto 8),
--		rx_bisterr	 => r_pio(15 downto 12),
--		rx_clkout	 => open,
--		rx_signaldetect	 => open,
--		tx_clkout	 => open,
--		tx_dataout	 => QL1_GXB_TX
--	);
--  
--  trans_loop_ql2 : trans_loop 
--  GENERIC MAP (
--    starting_channel_number => 8
--  )
--  PORT MAP (
--    cal_blk_clk	 => clk_cal,
--		pll_inclk	 => clk_sys,
--		reconfig_clk	 => clk_cal,
--		reconfig_togxb	 => rcfg_toloop,
--		rx_datain	 => QL2_GXB_RX,
--		rx_digitalreset	 => (others => r_reset),
--    rx_seriallpbken  => r_loopback(11 downto 8),
--		tx_digitalreset	 => (others => r_reset),
--		reconfig_fromgxb	 => rcfg_fromloop(50 downto 34),
--		rx_bistdone	 => r_pio(19 downto 16),
--		rx_bisterr	 => r_pio(23 downto 20),
--		rx_clkout	 => open,
--		rx_signaldetect	 => open,
--		tx_clkout	 => open,
--		tx_dataout	 => QL2_GXB_TX
--	);
  
--  trans_rcfg_inst : trans_rcfg PORT MAP (
--		reconfig_clk	 => clk_cal,
--		reconfig_fromgxb	 => rcfg_fromloop,
--		busy	 => open,
--		reconfig_togxb	 => rcfg_toloop
--	);
  
--  scub_master : scu_bus_master generic map (
--    g_interface_mode => PIPELINED,
--    g_address_granularity => BYTE,
--    CLK_in_Hz => 125_000_000,
--    Test => 0
--  )
--  port map (
--    slave_i => cbar_master_o(5),
--    slave_o => cbar_master_i(5),
--    
--    clk =>  clk_sys,
--    nrst => clk_sys_rstn,
--    
--    SCUB_Data => A_D,
--    nSCUB_DS => A_nDS,
--    nSCUB_Dtack => A_nDtack,
--    SCUB_Addr => A_A,
--    SCUB_RDnWR => A_RnW,
--    nSCUB_SRQ_Slaves => A_nSRQ,
--    nSCUB_Slave_Sel => A_nSEL,
--    nSCUB_Timing_Cycle => A_nTiming_Cycle,
--    nSel_Ext_Data_Drv => nSel_Ext_Data_DRV
--  );

  cfi_slave_i <= cbar_master_o(6);
  cbar_master_i(6) <= cfi_slave_o;

--  cfi_flash: cfi_ctrl
--  port map (
--    wb_dat_i => cfi_slave_i.dat,
--    wb_adr_i => cfi_slave_i.adr,
--    wb_stb_i => cfi_slave_i.stb,
--    wb_cyc_i => cfi_slave_i.cyc,
--    wb_we_i => cfi_slave_i.we,
--    wb_sel_i => cfi_slave_i.sel,
--    wb_dat_o => cfi_slave_o.dat,
--    wb_ack_o => cfi_slave_o.ack,
--    wb_err_o => cfi_slave_o.err,
--    wb_rty_o => cfi_slave_o.rty,
--    wb_stall_o => cfi_slave_o.stall,
--
--    wb_clk_i =>  clk_sys,
--    wb_rst_i => not clk_sys_rstn,
--    
--    flash_dq_io       => DF,
--    flash_adr_o       => AD,
--    flash_adv_n_o     => ADV_FSH,
--    flash_ce_n_o      => nCE_FSH,
--    flash_clk_o       => CLK_FSH,
--    flash_oe_n_o      => nOE_FSH,
--    flash_rst_n_o     => nRST_FSH,
--    flash_wait_i      => WAIT_FSH,
--    flash_we_n_o      => nWE_FSH,
--    flash_wp_n_o      => open
--  );

mil: wb_mil_scu
generic map (
		Clk_in_Hz	=> 125_000_000		-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
																-- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
		)
port map (
		clk_i  			=> clk_sys,
		nRst_i 			=> clk_sys_rstn,
		slave_i			=> cbar_master_o(7),
		slave_o			=> cbar_master_i(7),
		
		-- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
		nME_BOO				=> io_2_5v(11),		-- in: HD6408-output:	transmit bipolar positive.
		nME_BZO				=> io_2_5v(12),		-- in: HD6408-output:	transmit bipolar negative.
				
		ME_SD					=> io_2_5v(10),		-- in: HD6408-output:	'1' => send data is active.
		ME_ESC				=> io_2_5v(9),		-- in: HD6408-output:	encoder shift clock for shifting data into the encoder. The
																		--										encoder samples ME_SDI on low-to-high transition of ME_ESC.
		ME_SDI				=> eio(4),				-- out: HD6408-input:	serial data in accepts a serial data stream at a data rate
																		--										equal to encoder shift clock.
		ME_EE					=> eio(5),				-- out: HD6408-input:	a high on encoder enable initiates the encode cycle.
																		--										(Subject to the preceding cycle being completed).
		ME_SS					=> eio(6),				-- out: HD6408-input:	sync select actuates a Command sync for an input high
																		--										and data sync for an input low.

		-- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
		ME_BOI				=> eio(1),				-- out: HD6408-input:	A high input should be applied to bipolar one in when the bus is in its
																		--										positive state, this pin must be held low when the Unipolar input is used.
		ME_BZI				=> eio(2),				-- out: HD6408-input:	A high input should be applied to bipolar zero in when the bus is in its
																		--										negative state. This pin must be held high when the Unipolar input is used.
		ME_UDI				=> eio(3),				-- out: HD6408-input:	With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
																		--										transition finder circuit. If not used this input must be held low.
		ME_CDS				=> io_2_5v(7),		-- in: HD6408-output:	high occurs during output of decoded data which was preced
																		--										by a command synchronizing character. Low indicares a data sync.
		ME_SDO				=> io_2_5v(5),		-- in: HD6408-output:	serial data out delivers received data in correct NRZ format.
		ME_DSC				=> io_2_5v(4),		-- in: HD6408-output:	decoder shift clock delivers a frequency (decoder clock : 12),
																		--										synchronized by the recovered serial data stream.
		ME_VW					=> io_2_5v(6),		-- in: HD6408-output:	high indicates receipt of a VALID WORD.
		ME_TD					=> io_2_5v(8),		-- in: HD6408-output:	take data is high during receipt of data after identification
																		--										of a sync pulse and two valid Manchester data bits

		-- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
		ME_12MHz			=> eio(0),					-- out: HD6408-input:	is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
				
		
		Mil_BOI				=> io_2_5v(13),		-- out: HD6408-input:	connect positive bipolar receiver, in FPGA directed to the external
																		--										manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
																		--										vhdl manchester macro.
		Mil_BZI				=> io_2_5v(14),		-- out: HD6408-input:	connect negative bipolar receiver, in FPGA directed to the external
																		--										manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
																		--										vhdl manchester macro.
		Sel_Mil_Drv		=> eio(9),				-- output:	active high, enable the external open collector driver to the transformer
		nSel_Mil_Rcv	=> eio(7),				-- output:	active low, enable the external differtial receive circuit.
		Mil_nBOO			=> io_2_5v(11),		-- out:	connect bipolar positive output to external open collector driver of
																		--			the transformer. Source is the external manchester en/decoder HD6408 via
																		--			nME_BOO or the internal FPGA vhdl manchester macro.
		Mil_nBZO			=> io_2_5v(12),		-- out: connect bipolar negative output to external open collector driver of
																		--			the transformer. Source is the external manchester en/decoder HD6408 via
																		--			nME_BZO or the internal FPGA vhdl manchester macro.
		nLed_Mil_Rcv	=> open,--A_EXT_LVDS_TX_p3,	
		nLed_Mil_Trm	=> open,--A_EXT_LVDS_TX_n3,
		nLed_Mil_Err	=> open,--A_EXT_LVDS_TX_n2,
		error_limit_reached => open,
		Mil_Decoder_Diag_p => open,
		Mil_Decoder_Diag_n => open,
		timing				=> '0',
		nLed_Timing	=> eio(17),
		Interlock_Intr	=> '1',
		Data_Rdy_Intr		=> '1',
		Data_Req_Intr		=> '1'
		);
  
						
	serial_to_cb_o   <= '0'; 				-- connects the serial ports to the carrier board
  A_nCONFIG <= '1';
  nPWRBTN <= '1';
  ADR_TO_SCUB <= '1';
  nADR_EN <= '0';
  A_nReset <= clk_sys_rstn;
  
end rtl;
