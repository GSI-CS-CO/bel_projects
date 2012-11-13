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
		 -- User LEDs
		 -----------------------------------------------------------------------
		 leds_o			: out std_logic_vector(5 downto 0);
		 
		 -----------------------------------------------------------------------
		 -- OneWire
		 -----------------------------------------------------------------------
		 OneWire_CB		: inout std_logic;
	 
	 
		-----------------------------------------------------------------------
		-- Parallel Flash
		-----------------------------------------------------------------------
			
		AD						: out std_logic_vector(25 downto 1);
		DF						: inout std_logic_vector(15 downto 0);
		ADV_FSH				: out std_logic;
		nCE_FSH				: out std_logic;
		CLK_FSH				: out std_logic;
		nWE_FSH				: out std_logic;
		nOE_FSH				: out std_logic;
		nRST_FSH			: out std_logic;
		WAIT_FSH			: in std_logic;
		
		-----------------------------------------------------------------------
		-- LPC interface from ComExpress
		-----------------------------------------------------------------------
		LPC_AD			  : inout std_logic_vector(3 downto 0);
		LPC_FPGA_CLK	: in std_logic;
		LPC_SERIRQ		: inout std_logic;
		nLPC_DRQ0		  : in std_logic;
		nLPC_FRAME		: in std_logic;
		nPCI_RESET		: in std_logic;
		
		-----------------------------------------
    -- UART on front panel
    -----------------------------------------
    uart_rxd_i : in  std_logic_vector(1 downto 0);
    uart_txd_o : out std_logic_vector(1 downto 0);

    serial_to_cb_o : out std_logic;
      
    -----------------------------------------------------------------------
    -- SCU Bus
    -----------------------------------------------------------------------
    A_D               : inout std_logic_vector(15 downto 0);
    A_A               : out   std_logic_vector(15 downto 0);
    A_nTiming_Cycle   : out   std_logic;
    A_nDS             : out   std_logic;
    A_nReset          : out   std_logic;
    nSel_Ext_Data_DRV : out   std_logic;
    A_RnW             : out   std_logic;
    A_Spare           : out   std_logic_vector(1 downto 0);
    A_nSEL            : out   std_logic_vector(12 downto 1);
    A_nDtack          : in    std_logic;
    A_nSRQ            : in    std_logic_vector(12 downto 1);
    A_SysClock        : out   std_logic;
    ADR_TO_SCUB       : out   std_logic;
    nADR_EN           : out   std_logic;
    A_OneWire         : inout std_logic;
    
    -----------------------------------------------------------------------
    -- QL0 serdes
    -----------------------------------------------------------------------
 
    --QL0_GXB_RX        : in std_logic_vector(3 downto 0);
    --QL0_GXB_TX        : out std_logic_vector(3 downto 0);
    
    -----------------------------------------------------------------------
    -- QL1 serdes
    -----------------------------------------------------------------------

    --SFP1_TX_DIS       : out   std_logic := '0';
    
    --QL1_GXB_RX        : in std_logic_vector(3 downto 0);
    --QL1_GXB_TX        : out std_logic_vector(3 downto 0);
 
    -----------------------------------------------------------------------
    -- QL2 serdes
    -----------------------------------------------------------------------
    

    --SFP2_TX_DIS       : out   std_logic := '0';
    
    --QL2_GXB_RX        : in std_logic_vector(3 downto 0);
    --QL2_GXB_TX        : out std_logic_vector(3 downto 0);
    
    
    
    -----------------------------------------------------------------------
    -- Board configuration
    -----------------------------------------------------------------------
    A_nCONFIG         : out   std_logic;          -- triggers reconfig
    nPWRBTN           : out   std_logic;          -- Powerbutton for ComExpress
    nFPGA_Res_Out     : out   std_logic := '1'    -- drives sys_reset

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
    
  -- Top crossbar layout
  constant c_slaves : natural := 6;
  constant c_masters : natural := 5;
  constant c_dpram_size : natural := 16384; -- in 32-bit words (64KB)
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size),  x"00000000"),
    1 => f_sdb_embed_device(c_xwb_gpio32_sdb,           x"00100400"),
    2 => f_sdb_embed_device(c_xwb_dma_sdb,              x"00100500"),
    3 => f_sdb_embed_device(c_xwb_owm,				 		      x"00100600"),
    4 => f_sdb_embed_device(c_xwb_uart,                 x"00100700"),
    5 => f_sdb_embed_device(c_scu_bus_master,           x"00400000"));
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
	Inst_flash_loader_v01 : flash_loader
    port map (
      noe_in   => '0'
    );
	 
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
      wb_clk        => clk_sys,       -- Desired clock for the WB bus
      wb_rstn_i     => clk_sys_rstn,
      master_o      => cbar_slave_i(0),
      master_i      => cbar_slave_o(0));
  
   -- The LM32 is master 1+2
  lm32_rstn <= clk_sys_rstn and not r_reset;
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
  leds_o <= not r_leds(5 downto 0);
  
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
  
  scub_master : scu_bus_master generic map (
    g_interface_mode => PIPELINED,
    g_address_granularity => BYTE,
    CLK_in_Hz => 125_000_000,
    Test => 0
  )
  port map (
    slave_i => cbar_master_o(5),
    slave_o => cbar_master_i(5),
    
    clk =>  clk_sys,
    nrst => clk_sys_rstn,
    
    SCUB_Data => A_D,
    nSCUB_DS => A_nDS,
    nSCUB_Dtack => A_nDtack,
    SCUB_Addr => A_A,
    SCUB_RDnWR => A_RnW,
    nSCUB_SRQ_Slaves => A_nSRQ,
    nSCUB_Slave_Sel => A_nSEL,
    nSCUB_Timing_Cycle => A_nTiming_Cycle,
    nSel_Ext_Data_Drv => nSel_Ext_Data_DRV
  );
  
  
						
	serial_to_cb_o   <= '0'; 				-- connects the serial ports to the carrier board
  A_nCONFIG <= '1';
  nPWRBTN <= '1';
  ADR_TO_SCUB <= '1';
  nADR_EN <= '0';
  A_nReset <= clk_sys_rstn;
  
end rtl;
