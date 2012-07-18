library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_altera_pkg.all;
use work.trans_pkg.all;
use work.pcie_wb_pkg.all;
use work.wishbone_pkg.all;

entity sio_top is
  port (
          clk_fpga              : in std_logic;
          clk_20                : in std_logic;
          nCB_RESET             : in std_logic;
          A_nLED                : out std_logic_vector(15 downto 0) := (others => '1');
                    
          nExtension_Res_Out    : out std_logic := '1';
          
          ---------------------------------------------------------------------
          -- SCU Bus
          ---------------------------------------------------------------------
          A_D                   : out std_logic_vector(15 downto 0);
          A_nDtack              : out std_logic := '1';
          A_nSRQ                : out std_logic := '1';
          A_nSel_Ext_Data_Drv   : out std_logic := '1';
          A_Ext_Data_RD         : out std_logic;
          A_nADR_EN             : out std_logic := '0';
          A_nADR_SCUB           : out std_logic := '0';
          A_nExt_Signal_In      : out std_logic := '0';
          A_nSEL_Ext_Signal_RDV : out std_logic := '0';
                  
                  
          -----------------------------------------
          -- PCI express pins
          -----------------------------------------
          pcie_refclk_i : in  std_logic;
          --pcie_rstn_i   : in  std_logic;
          pcie_rx_i     : in  std_logic_vector(3 downto 0);
          pcie_tx_o     : out std_logic_vector(3 downto 0);
          -----------------------------------------------------------------------
          -- QL0 serdes
          -----------------------------------------------------------------------
          --QL0_GXB_RX        : in std_logic_vector(3 downto 0);
          --QL0_GXB_TX        : out std_logic_vector(3 downto 0);
          
          -----------------------------------------------------------------------
          -- QL1 serdes
          -----------------------------------------------------------------------
          QL1_GXB_RX        : in std_logic_vector(3 downto 0);
          QL1_GXB_TX        : out std_logic_vector(3 downto 0)
               
  );
end sio_top;

architecture sio_top_arch of sio_top is
	 
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
    
  -- Top crossbar layout
  constant c_slaves : natural := 2;
  constant c_masters : natural := 1;
  constant c_dpram_size : natural := 16384; -- in 32-bit words (64KB)
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size),  x"00000000"),
    1 => f_sdb_embed_device(c_xwb_uart,                 x"00100100"));
  constant c_sdb_address : t_wishbone_address :=        x"00100000";
 
  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  component sio_rcfg
	PORT
	(
		reconfig_clk		    : IN STD_LOGIC ;
		reconfig_fromgxb		: IN STD_LOGIC_VECTOR (33 DOWNTO 0);
		busy		            : OUT STD_LOGIC ;
		reconfig_togxb		  : OUT STD_LOGIC_VECTOR (3 DOWNTO 0)
	);
  end component;
  
  component led_blink is
	generic (
				count:				integer := 8;
				Clk_in_Hz:			integer := 100_000_000;
				blink_duration:		integer := 1_000_000_000		-- 1s
			);
	port (
			clk:	in std_logic;
			nrst:	in std_logic;
			led:	out std_logic_vector(count-1 downto 0)
		);
  end component;

  signal clk_cal:         std_logic;
  signal clk_sys:         std_logic;
  signal locked:          std_logic;
  signal rstn:            std_logic;
  signal clk_150:				  std_logic;

  signal rcfg_toloop:     std_logic_vector(3 downto 0);
  signal rx_drst:         std_logic_vector(1 downto 0);
  signal tx_drst:         std_logic_vector(1 downto 0);
  signal rcfg_fromloop:   std_logic_vector(33 downto 0);
  signal rx_bistdone:     std_logic_vector(1 downto 0);
  signal rx_bisterr:      std_logic_vector(1 downto 0);
  signal rx_signaldetect: std_logic_vector(1 downto 0);


begin


  
	-- Obtain core clocking
	sys_pll_inst : sys_pll      -- Altera megafunction
    port map (
      inclk0 => clk_fpga,     -- 125Mhz oscillator from board
      c0     => clk_sys,      -- 125MHz system clk (cannot use external pin as clock for RAM blocks)
      c1     => clk_cal,      -- 50Mhz calibration clock for Altera reconfig cores
      locked => locked);      -- '1' when the PLL has locked
	
	t_pll: trans_pll
		port map (
			inclk0 	=> clk_fpga,
			c0			=> clk_150,
			locked	=> open);
      
  -- Hold the entire WB bus reset until the PLL has locked
  rstn <= locked;
  
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
     rst_n_i       => rstn,
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
      clk125_i      => clk_sys,       -- Free running clock
      cal_clk50_i   => clk_cal,       -- Transceiver global calibration clock
      rstn_i        => locked,        -- Reset for the PCIe decoder logic
      pcie_refclk_i => pcie_refclk_i, -- External PCIe 100MHz bus clock
      pcie_rstn_i   => locked,   		  -- External PCIe system reset pin
      pcie_rx_i     => pcie_rx_i,
      pcie_tx_o     => pcie_tx_o,
      wb_clk        => clk_sys,       -- Desired clock for the WB bus
      master_o      => cbar_slave_i(0),
      master_i      => cbar_slave_o(0));
      
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
      rst_n_i   => rstn,

      -- Wishbone
      slave_i => cbar_master_o(1),
      slave_o => cbar_master_i(1),
      desc_o  => open,

      uart_rxd_i => '0',
      uart_txd_o => open
      );

  
--	trans_loop_ql0 : reverse_lpb
--	GENERIC MAP (
--		starting_channel_number => 0
--	)
--	PORT MAP (
--		cal_blk_clk	 => clk_cal,
--		pll_inclk	 => clk_150,
--		reconfig_clk	 => clk_cal,
--		reconfig_togxb	 => rcfg_toloop,
--		rx_datain	 => QL0_GXB_RX,
--		rx_digitalreset	 => (others => locked),
--		tx_digitalreset	 => (others => locked),
--		reconfig_fromgxb	 => rcfg_fromloop(16 downto 0),
--    rx_enapatternalign => (others => '0'),
--		tx_datain		=> (others => '0'),
--		rx_clkout	 => open,
--		tx_clkout	 => open,
--    rx_dataout   => open,
--		tx_dataout	 => QL0_GXB_TX
--	);
   
  trans_loop_ql1 : reverse_lpb 
  GENERIC MAP (
    starting_channel_number => 0
  )
  PORT MAP (
    cal_blk_clk	 => clk_cal,
		pll_inclk	 => clk_150,
		reconfig_clk	 => clk_cal,
		reconfig_togxb	 => rcfg_toloop,
		rx_datain	 => QL1_GXB_RX,
		rx_digitalreset	 => (others => locked),
		tx_digitalreset	 => (others => locked),
		reconfig_fromgxb	 => rcfg_fromloop(16 downto 0),
    rx_enapatternalign => (others => '0'),
		tx_datain		=> (others => '0'),
		rx_clkout	 => open,
		tx_clkout	 => open,
    rx_dataout   => open,
		tx_dataout	 => QL1_GXB_TX
	);
  
   sio_rcfg_inst : sio_rcfg PORT MAP (
		reconfig_clk	      => clk_cal,
		reconfig_fromgxb	  => rcfg_fromloop,
		busy	              => open,
		reconfig_togxb	    => rcfg_toloop
	);
  
  blinker: led_blink generic map (
    count => 1,
    Clk_in_Hz => 125_000_000 )
    port map (
    clk => clk_sys,
    nrst => locked,
    led => A_nLED(15 downto 15)
    );

end architecture;
