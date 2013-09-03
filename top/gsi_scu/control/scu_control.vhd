library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.wrcore_pkg.all;
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.eca_pkg.all;
use work.wb_cores_pkg_gsi.all;
use work.pcie_wb_pkg.all;
use work.wr_altera_pkg.all;
use work.etherbone_pkg.all;
use work.scu_bus_pkg.all;
use work.altera_flash_pkg.all;
use work.oled_display_pkg.all;
use work.lpc_uart_pkg.all;
use work.wb_irq_pkg.all;

entity scu_control is
  port(
    clk_20m_vcxo_i    : in std_logic;  -- 20MHz VCXO clock
    clk_125m_pllref_i : in std_logic;  -- 125 MHz PLL reference
    clk_125m_local_i  : in std_logic;  -- local clk from 125Mhz oszillator
    nres              : in std_logic; -- powerup reset
    
    -----------------------------------------
    -- UART on front panel
    -----------------------------------------
    uart_rxd_i     : in  std_logic_vector(1 downto 0);
    uart_txd_o     : out std_logic_vector(1 downto 0);
    serial_to_cb_o : out std_logic;
    
    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_refclk_i  : in  std_logic;
    pcie_rx_i      : in  std_logic_vector(3 downto 0);
    pcie_tx_o      : out std_logic_vector(3 downto 0);
    nPCI_RESET     : in std_logic;
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk       : out std_logic;
    dac_din        : out std_logic;
    ndac_cs        : out std_logic_vector(2 downto 1);

    -----------------------------------------
    -- LEMO on front panel (LED        = B1/B2 act)
    --                     (lemo_en_in = B1/B2 out)
    -----------------------------------------
    lemo_io        : inout std_logic_vector(2 downto 1);
    lemo_en_in     : out   std_logic_vector(2 downto 1);
    lemo_led       : out   std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- LPC interface from ComExpress
    -----------------------------------------------------------------------
    LPC_AD         : inout std_logic_vector(3 downto 0);
    LPC_FPGA_CLK   : in    std_logic;
    LPC_SERIRQ     : inout std_logic;
    nLPC_DRQ0      : in    std_logic;
    nLPC_FRAME     : in    std_logic;

    -----------------------------------------------------------------------
    -- User LEDs (U1-U4)
    -----------------------------------------------------------------------
    leds_o         : out std_logic_vector(4 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB     : inout std_logic;
    
    -----------------------------------------------------------------------
    -- QL1 serdes
    -----------------------------------------------------------------------
--    QL1_GXB_RX        : in std_logic_vector(3 downto 0);
--    QL1_GXB_TX        : out std_logic_vector(3 downto 0);
    
    -----------------------------------------------------------------------
    -- AUX SFP 
    -----------------------------------------------------------------------
    sfp1_tx_disable_o : out std_logic := '0';
    --sfp1_txp_o        : out std_logic;
    --sfp1_rxp_i        : in  std_logic;
    
    sfp1_mod0         : in    std_logic; -- grounded by module
    sfp1_mod1         : inout std_logic; -- SCL
    sfp1_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- Timing SFP 
    -----------------------------------------------------------------------
    sfp2_ref_clk_i    : in  std_logic;
    
    sfp2_tx_disable_o : out std_logic := '0';
    sfp2_txp_o        : out std_logic;
    sfp2_rxp_i        : in  std_logic;
    
    sfp2_mod0         : in    std_logic; -- grounded by module
    sfp2_mod1         : inout std_logic; -- SCL
    sfp2_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- LA port
    -----------------------------------------------------------------------
    hpla_ch           : out std_logic_vector(15 downto 0);
    hpla_clk          : out std_logic;
    
    -----------------------------------------------------------------------
    -- EXT CONN
    -----------------------------------------------------------------------
    IO_2_5            : out std_logic_vector(13 downto 0);
    A_EXT_LVDS_RX     : in  std_logic_vector( 3 downto 0);
    A_EXT_LVDS_TX     : out std_logic_vector( 3 downto 0);
    A_EXT_LVDS_CLKOUT : out std_logic;
    A_EXT_LVDS_CLKIN  : in  std_logic;
    EIO               : out std_logic_vector(16 downto 0);
    
    -----------------------------------------------------------------------
    -- serial channel SCU bus
    -----------------------------------------------------------------------
    
    --A_MASTER_CON_RX   : in std_logic_vector(3 downto 0);
    --A_MASTER_CON_TX   : out std_logic_vector(3 downto 0);
    
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
    -- ComExpress signals
    -----------------------------------------------------------------------
    nTHRMTRIP         : in  std_logic;
    nEXCD0_PERST      : in  std_logic;
    WDT               : in  std_logic;
    A20GATE           : out std_logic;
    nPWRBTN           : out std_logic;
    nFPGA_Res_Out     : out std_logic;
    A_nCONFIG         : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- Parallel Flash
    -----------------------------------------------------------------------
    AD                : out   std_logic_vector(25 downto 1);
    DF                : inout std_logic_vector(15 downto 0);
    ADV_FSH           : out   std_logic;
    nCE_FSH           : out   std_logic;
    CLK_FSH           : out   std_logic;
    nWE_FSH           : out   std_logic;
    nOE_FSH           : out   std_logic;
    nRST_FSH          : out   std_logic;
    WAIT_FSH          : in    std_logic;
    
    -----------------------------------------------------------------------
    -- DDR3
    -----------------------------------------------------------------------
    DDR3_DQ           : inout std_logic_vector(15 downto 0);
    DDR3_DM           : out   std_logic_vector( 1 downto 0);
    DDR3_BA           : out   std_logic_vector( 2 downto 0);
    DDR3_ADDR         : out   std_logic_vector(12 downto 0);
    DDR3_CS_n         : out   std_logic_vector( 0 downto 0);
--    DDR3_DQS          : inout std_logic_vector(1 downto 0);
--    DDR3_DQSn         : inout std_logic_vector(1 downto 0);
    DDR3_RES_n        : out   std_logic;
    DDR3_CKE          : out   std_logic_vector( 0 downto 0);
    DDR3_ODT          : out   std_logic_vector( 0 downto 0);
    DDR3_CAS_n        : out   std_logic;
    DDR3_RAS_n        : out   std_logic;
--    DDR3_CLK          : inout std_logic_vector(0 downto 0);
--    DDR3_CLK_n        : inout std_logic_vector(0 downto 0);
    DDR3_WE_n         : out   std_logic);
    
end scu_control;

architecture rtl of scu_control is
  
  -- WR core layout
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  constant c_xwb_gpio32_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000001f", -- five 4 byte registers
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"35aa6b95",
    version       => x"00000001",
    date          => x"20120305",
    name          => "GSI_GPIO_32        ")));

	constant c_dpram_size : natural := 131072/4;


  ----------------------------------------------------------------------------------
  -- MSI IRQ Crossbar --------------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_irq_slaves   : natural := 4;
  constant c_irq_masters  : natural := 1;
  constant c_irq_layout   : t_sdb_record_array(c_irq_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(c_irq_ep_sdb, 	      x"00000000"),
    1 => f_sdb_embed_device(c_irq_ep_sdb,             x"00000100"),
    2 => f_sdb_embed_device(c_irq_ep_sdb,             x"00000200"),
    3 => f_sdb_embed_device(c_irq_hostbridge_ep_sdb,  x"00001000"));
  constant c_irq_sdb_address : t_wishbone_address := x"00002000";

  signal irq_cbar_slave_i  : t_wishbone_slave_in_array (c_irq_masters-1 downto 0);
  signal irq_cbar_slave_o  : t_wishbone_slave_out_array(c_irq_masters-1 downto 0);
  signal irq_cbar_master_i : t_wishbone_master_in_array(c_irq_slaves-1 downto 0);
  signal irq_cbar_master_o : t_wishbone_master_out_array(c_irq_slaves-1 downto 0);

  -- END OF MSI IRQ Crossbar
  ----------------------------------------------------------------------------------	 
  
  ----------------------------------------------------------------------------------
  -- GSI Periphery Crossbar --------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_per_slaves   : natural := 9;
  constant c_per_masters  : natural := 1;
  constant c_per_layout   : t_sdb_record_array(c_per_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(c_xwr_wb_timestamp_latch_sdb, x"00000000"),
    1 => f_sdb_embed_device(c_eca_sdb,                    x"00000800"),
    2 => f_sdb_embed_device(c_eca_evt_sdb,                x"00000C00"),
	 3 => f_sdb_embed_device(c_irq_ctrl_sdb,               x"00000D00"),
	 4 => f_sdb_embed_device(c_scu_bus_master,             x"00400000"),
    5 => f_sdb_embed_device(c_xwb_gpio32_sdb,             x"00800000"),
    6 => f_sdb_embed_device(c_wrc_periph1_sdb,            x"00800100"),
    7 => f_sdb_embed_device(c_oled_display,               x"00900000"),
    8 => f_sdb_embed_device(f_wb_spi_flash_sdb(24),       x"01000000"));
  constant c_per_sdb_address : t_wishbone_address := x"00001000";
  constant c_per_bridge_sdb  : t_sdb_bridge       :=
    f_xwb_bridge_layout_sdb(true, c_per_layout, c_per_sdb_address);
	
  signal per_cbar_slave_i  : t_wishbone_slave_in_array (c_per_masters-1 downto 0);
  signal per_cbar_slave_o  : t_wishbone_slave_out_array(c_per_masters-1 downto 0);
  signal per_cbar_master_i : t_wishbone_master_in_array(c_per_slaves-1 downto 0);
  signal per_cbar_master_o : t_wishbone_master_out_array(c_per_slaves-1 downto 0);	
  
  -- END OF GSI Periphery Crossbar
  ----------------------------------------------------------------------------------		 
  
  ----------------------------------------------------------------------------------
  -- Top crossbar ------------------------------------------------------------------
  ----------------------------------------------------------------------------------  
  constant c_top_slaves : natural := 4;
  constant c_top_masters : natural := 4;
  constant c_top_layout : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size),    x"00000000"),
    1 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,          x"00080000"),
	 2 => f_sdb_embed_device(c_ebm_sdb,          			 x"01000000"),
    3 => f_sdb_embed_bridge(c_per_bridge_sdb,             x"02000000")
   );
  constant c_top_sdb_address : t_wishbone_address := x"000F0000";	 
  
  signal top_cbar_slave_i  : t_wishbone_slave_in_array (c_top_masters-1 downto 0);
  signal top_cbar_slave_o  : t_wishbone_slave_out_array(c_top_masters-1 downto 0);
  signal top_cbar_master_i : t_wishbone_master_in_array(c_top_slaves-1 downto 0);
  signal top_cbar_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);
  
  -- END OF Top crossbar
  ----------------------------------------------------------------------------------		

  signal eca_2_wb_i : t_wishbone_master_in;
  signal eca_2_wb_o : t_wishbone_master_out;
  
  signal pcie_slave_i : t_wishbone_slave_in;
  signal pcie_slave_o : t_wishbone_slave_out;
  
  signal gpio_slave_i : t_wishbone_slave_in;
  signal gpio_slave_o : t_wishbone_slave_out;

  -- Sys PLL from clk_125m_local_i
  signal sys_locked       : std_logic;
  signal clk_62_5         : std_logic;
  signal clk_50           : std_logic;
  signal clk_20           : std_logic;
  signal rstn_sys         : std_logic;
  
  -- Logical clock names
  signal clk_pcie         : std_logic;
  signal clk_sys          : std_logic;
  signal clk_reconf       : std_logic;
  signal clk_flash        : std_logic;
  signal clk_scubus       : std_logic;
  
  -- RX PLL
  signal gxb_locked       : std_logic;
  signal rstn_wr          : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal ref_locked       : std_logic;
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_25m          : std_logic;
  signal rstn_ref         : std_logic;
  
  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(3 downto 0);
  
  -- DMTD PLL from clk_20m_vcxo_i
  -- signal dmtd_locked      : std_logic;
  signal clk_dmtd         : std_logic;
  
  signal dac_hpll_load_p1 : std_logic;
  signal dac_dpll_load_p1 : std_logic;
  signal dac_hpll_data    : std_logic_vector(15 downto 0);
  signal dac_dpll_data    : std_logic_vector(15 downto 0);
  
  signal link_up  : std_logic;
  signal link_act : std_logic;
  signal ext_pps  : std_logic;
  signal pps      : std_logic;

  signal phy_tx_data      : std_logic_vector(7 downto 0);
  signal phy_tx_k         : std_logic;
  signal phy_tx_disparity : std_logic;
  signal phy_tx_enc_err   : std_logic;
  signal phy_rx_data      : std_logic_vector(7 downto 0);
  signal phy_rx_rbclk     : std_logic;
  signal phy_rx_k         : std_logic;
  signal phy_rx_enc_err   : std_logic;
  signal phy_rx_bitslide  : std_logic_vector(3 downto 0);
  signal phy_rst          : std_logic;
  signal phy_loopen       : std_logic;

  signal wrc_master_i  : t_wishbone_master_in;
  signal wrc_master_o  : t_wishbone_master_out;

  signal mb_src_out    : t_wrf_source_out;
  signal mb_src_in     : t_wrf_source_in;
  signal mb_snk_out    : t_wrf_sink_out;
  signal mb_snk_in     : t_wrf_sink_in;
  
  signal tm_valid  : std_logic;
  signal tm_tai    : std_logic_vector(39 downto 0);
  signal tm_cycles : std_logic_vector(27 downto 0);

  signal channels : t_channel_array(1 downto 0);
  
  signal owr_pwren : std_logic_vector(1 downto 0);
  signal owr_en    : std_logic_vector(1 downto 0);
  signal owr       : std_logic_vector(1 downto 0);
  
  signal sfp2_scl_o : std_logic;
  signal sfp2_scl_i : std_logic;
  signal sfp2_sda_o : std_logic;
  signal sfp2_sda_i : std_logic;
  signal sfp2_det_i : std_logic;
  
  signal eca_gpio : std_logic_vector(15 downto 0);
  
  signal r_lemo_dir : std_logic_vector(1 downto 0);
  signal r_gpio_mux : std_logic_vector(7 downto 0);
  signal r_gpio_val : std_logic_vector(3 downto 0);
  signal r_resets   : std_logic_vector(3 downto 0) := (others => '0');
    
  signal s_lemo_dat : std_logic_vector(2 downto 1);
  signal s_uled_dat : std_logic_vector(2 downto 1);
  signal s_lemo_led : std_logic_vector(2 downto 1);
  signal s_lemo_in  : std_logic_vector(3 downto 0);
  
  signal kbc_out_port : std_logic_vector(7 downto 0);
  signal kbc_in_port  : std_logic_vector(7 downto 0);
  
  signal rst_usr_lm32_n : std_logic;
begin

  ----------------------------------------------------------------------------------
  -- Reset & PLLs and Tranceiver ---------------------------------------------------
  ----------------------------------------------------------------------------------
  rstn_wr <= rstn_sys and gxb_locked;

  dmtd_inst : dmtd_pll port map(
    inclk0 => clk_20m_vcxo_i,    --  20  Mhz 
    c0     => clk_dmtd,          --  62.5MHz
    locked => open);
  
  ref_inst : ref_pll port map(
    inclk0 => clk_125m_pllref_i, -- 125 MHz
    c0     => clk_ref,           -- 125 MHz
    c1     => clk_butis,         -- 200 MHz
    c2     => clk_25m,           --  25 MHz
    locked => ref_locked,
    scanclk            => clk_reconf,
    phasedone          => phase_done,
    phasecounterselect => phase_sel,
    phasestep          => phase_step,
    phaseupdown        => '0');

  sys_inst : sys_pll port map(
    inclk0 => clk_125m_local_i, -- 125  Mhz 
    c0     => clk_62_5,         --  62.5MHz
    c1     => clk_50,           --  50  Mhz
    c2     => clk_20,           --  20  MHz
    locked => sys_locked);
  
  clk_pcie  <= clk_ref;
  clk_sys   <= clk_62_5;
  clk_reconf <= clk_50;
  clk_flash  <= clk_50;
  clk_scubus <= clk_20;
  
  sys_reset : gc_reset
    generic map(
      g_clocks => 1)
    port map(
      free_clk_i => clk_sys,
      locked_i   => sys_locked,
      clks_i(0)  => clk_sys,
      rstn_o(0)  => rstn_sys);

  ref_reset : gc_reset
    generic map(
      g_clocks => 1)
    port map(
      free_clk_i => clk_ref,
      locked_i   => ref_locked,
      clks_i(0)  => clk_ref,
      rstn_o(0)  => rstn_ref);

  butis : altera_butis
    port map(
      clk_ref_i   => clk_ref,
      clk_25m_i   => clk_25m,
      clk_scan_i  => clk_reconf,
      locked_i    => ref_locked,
      pps_i       => pps,
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);
  
 
    wr_gxb_arria2 : wr_arria2_phy
    port map (
      clk_reconf_i   => clk_reconf,
      clk_pll_i      => clk_ref,
      clk_cru_i      => sfp2_ref_clk_i,
      clk_sys_i      => clk_sys,
      rstn_sys_i     => rstn_sys,
      locked_o       => gxb_locked,
      loopen_i       => phy_loopen,
      drop_link_i    => phy_rst,
      tx_data_i      => phy_tx_data,
      tx_k_i         => phy_tx_k,
      tx_disparity_o => phy_tx_disparity,
      tx_enc_err_o   => phy_tx_enc_err,
      rx_rbclk_o     => phy_rx_rbclk,
      rx_data_o      => phy_rx_data,
      rx_k_o         => phy_rx_k,
      rx_enc_err_o   => phy_rx_enc_err,
      rx_bitslide_o  => phy_rx_bitslide,
      pad_txp_o      => sfp2_txp_o,
      pad_rxp_i      => sfp2_rxp_i);    
  
  -- Reset & PLLs and Tranceiver
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- WB Bus Interconnects ----------------------------------------------------------
  ----------------------------------------------------------------------------------
   IRQ_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_irq_masters,
     g_num_slaves  => c_irq_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_irq_layout,
     g_sdb_addr    => c_irq_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => irq_cbar_slave_i,
     slave_o       => irq_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => irq_cbar_master_i,
     master_o      => irq_cbar_master_o);
  
  TOP_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_top_masters,
     g_num_slaves  => c_top_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_top_layout,
     g_sdb_addr    => c_top_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => top_cbar_slave_i,
     slave_o       => top_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => top_cbar_master_i,
     master_o      => top_cbar_master_o);
  
  ------------------------------------------------
  -- Connect periphery crossbar to top crossbar
  per_cbar_slave_i(0)  <= top_cbar_master_o(3);
  top_cbar_master_i(3) <= per_cbar_slave_o(0);
  ------------------------------------------------
  
  PER_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_per_masters,
     g_num_slaves  => c_per_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_per_layout,
     g_sdb_addr    => c_per_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => per_cbar_slave_i,
     slave_o       => per_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => per_cbar_master_i,
     master_o      => per_cbar_master_o);

  -- END OF WB Bus Interconnects
  ----------------------------------------------------------------------------------
  
  
  ----------------------------------------------------------------------------------
  -- Top LM32 CPU & RAM ------------------------------------------------------------
  ----------------------------------------------------------------------------------
    DPRAM : xwb_dpram
    generic map(
      g_size                  => c_dpram_size,
      g_init_file             => "",
      g_must_have_init_file   => false,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => WORD)  
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => rstn_sys,

      slave1_i => top_cbar_master_o(0),
      slave1_o => top_cbar_master_i(0),
      slave2_i => cc_dummy_slave_in,
      slave2_o => open
      );

    

   LM32_CORE : wb_irq_lm32
    generic map(g_profile => "medium_icache_debug")
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => rst_usr_lm32_n,

      dwb_o => top_cbar_slave_i(2),
      dwb_i => top_cbar_slave_o(2),
      iwb_o => top_cbar_slave_i(3),
      iwb_i => top_cbar_slave_o(3),
      
      irq_slave_o   => irq_cbar_master_i(c_irq_slaves-1-1 downto 0), 
      irq_slave_i   => irq_cbar_master_o(c_irq_slaves-1-1 downto 0),
           
      ctrl_slave_o  => per_cbar_master_i(3),
      ctrl_slave_i  => per_cbar_master_o(3)
      );	

  -- END OF Top LM32 CPU & RAM 
  ----------------------------------------------------------------------------------
  

  ----------------------------------------------------------------------------------
  -- GSI WB Periphery --------------------------------------------------------------
  ----------------------------------------------------------------------------------

  flash : flash_top
    generic map(
      g_family                 => "Arria II GX",
      g_port_width             => 1,   -- single-lane SPI bus
      g_addr_width             => 24,  -- 3 byte addressed chip
      g_dummy_time             => 8,   -- 8 cycles between address and data
      g_input_latch_edge       => '1', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
      g_output_latch_edge      => '0', -- falling edge to meet SPI setup times
      g_input_to_output_cycles => 2)   -- delayed to work-around unconstrained design
    port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => per_cbar_master_o(8),
      slave_o   => per_cbar_master_i(8),
      clk_out_i => clk_flash,
      clk_in_i  => clk_flash); -- no need to phase shift at 50MHz
      
U_DAC_ARB : spec_serial_dac_arb
    generic map (
      g_invert_sclk    => false,
      g_num_extra_bits => 8) -- AD DACs with 24bit interface
    port map (
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,

      val1_i  => dac_dpll_data,
      load1_i => dac_dpll_load_p1,

      val2_i  => dac_hpll_data,
      load2_i => dac_hpll_load_p1,

      dac_cs_n_o(0) => ndac_cs(1),
      dac_cs_n_o(1) => ndac_cs(2),
      dac_clr_n_o   => open,
      dac_sclk_o    => dac_sclk,
      dac_din_o     => dac_din);


  eb : eb_master_slave_wrapper
  generic map(
    g_with_master         	=> true,
    g_ebs_sdb_address		=> (x"00000000" & c_top_sdb_address),
	 g_ebm_adr_bits_hi 		=> 10)               
  port map(
	 clk_i       => clk_sys,
	 nRst_i      => rstn_sys,
	 snk_i       => mb_snk_in,
	 snk_o       => mb_snk_out,
	 src_o       => mb_src_out,
	 src_i       => mb_src_in,
  
    --ebs
    ebs_cfg_slave_o => wrc_master_i,
    ebs_cfg_slave_i => wrc_master_o,
    ebs_wb_master_o => top_cbar_slave_i(0),
    ebs_wb_master_i => top_cbar_slave_o(0),
    
    --ebm (optional)
    ebm_wb_slave_i  => top_cbar_master_o(2),
    ebm_wb_slave_o  => top_cbar_Master_i(2));
		 

  
  PCIe : pcie_wb
    generic map(
       sdb_addr => c_top_sdb_address)
    port map(
       clk125_i      => clk_pcie,
       cal_clk50_i   => clk_reconf,
       
       pcie_refclk_i => pcie_refclk_i,
       pcie_rstn_i   => nPCI_RESET,
       pcie_rx_i     => pcie_rx_i,
       pcie_tx_o     => pcie_tx_o,
       
       master_clk_i  => clk_sys,
       master_rstn_i => rstn_sys,
       master_o      => top_cbar_slave_i(1),
       master_i      => top_cbar_slave_o(1),
       
       slave_clk_i   => clk_ref,
       slave_rstn_i  => rstn_ref,
       slave_i       => irq_cbar_master_o(3),
       slave_o       => irq_cbar_master_i(3));
  
  TLU : wb_timestamp_latch
    generic map (
      g_num_triggers => 5,
      g_fifo_depth   => 10)
    port map (
      ref_clk_i       => clk_ref,
      ref_rstn_i      => rstn_ref,
      sys_clk_i       => clk_sys,
      sys_rstn_i      => rstn_sys,
      triggers_i(0)   => lemo_io(1),
      triggers_i(1)   => lemo_io(2),
		triggers_i(2)	 => eca_gpio(0),
		triggers_i(3)	 => eca_gpio(1),
		triggers_i(4)	 => eca_gpio(2),
      tm_time_valid_i => tm_valid,
      tm_tai_i        => tm_tai,
      tm_cycles_i     => tm_cycles,
      wb_slave_i      => per_cbar_master_o(0),
      wb_slave_o      => per_cbar_master_i(0));

  ECA0 : wr_eca
    generic map(
      g_eca_name       => f_name("SCU top"),
      g_channel_names  => (f_name("GPIO: LEMOs(0=B1,1=B2) LEDs(2=U1,3=U2)"), 
                           f_name("PCIe: Interrupt generator")),
      g_log_table_size => 7,
      g_log_queue_len  => 8,
      g_num_channels   => 2,
      g_num_streams    => 1)
    port map(
      e_clk_i  (0)=> clk_sys,
      e_rst_n_i(0)=> rstn_sys,
      e_slave_i(0)=> per_cbar_master_o(2),
      e_slave_o(0)=> per_cbar_master_i(2),
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => per_cbar_master_o(1),
      c_slave_o   => per_cbar_master_i(1),
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_tai_i     => tm_tai,
      a_cycles_i  => tm_cycles,
      a_channel_o => channels);
  
  C0 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(0),
      gpio_o    => eca_gpio);
  
  C1 : eca_wb_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(1),
      master_o  => eca_2_wb_o,
      master_i  => eca_2_wb_i);
		
	 eca_2_irq : xwb_clock_crossing 
	 generic map(
				g_size => 256)
	 port map(
    slave_clk_i    => clk_ref,
    slave_rst_n_i  => rstn_ref,
    slave_i        => eca_2_wb_o,
    slave_o        => eca_2_wb_i,
    master_clk_i   => clk_sys, 
    master_rst_n_i => rstn_sys,
    master_i       => irq_cbar_slave_o(0),
    master_o       => irq_cbar_slave_i(0));	
		
		
  
  scub_master : wb_scu_bus 
    generic map(
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE,
      CLK_in_Hz             => 62_500_000,
      Test                  => 0,
      Time_Out_in_ns        => 350)
   port map(
     clk     =>  clk_sys,
     nrst    => rstn_sys,
     slave_i => per_cbar_master_o(4),
     slave_o => per_cbar_master_i(4),
     
     SCUB_Data          => A_D,
     nSCUB_DS           => A_nDS,
     nSCUB_Dtack        => A_nDtack,
     SCUB_Addr          => A_A,
     SCUB_RDnWR         => A_RnW,
     nSCUB_SRQ_Slaves   => A_nSRQ,
     nSCUB_Slave_Sel    => A_nSEL,
     nSCUB_Timing_Cycle => A_nTiming_Cycle,
     nSel_Ext_Data_Drv  => nSel_Ext_Data_DRV);
  
  ADR_TO_SCUB <= '1';
  nADR_EN     <= '0';
  A_SysClock  <= clk_scubus;
  A_nReset    <= rstn_sys;
  A_Spare     <= (others => 'Z');
  A_OneWire   <= 'Z';
  A20GATE     <= kbc_out_port(1);
     
  gpio_slave_i <= per_cbar_master_o(5);
  per_cbar_master_i(5) <= gpio_slave_o;
  
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
      gpio_slave_o.dat <= (others => '0');
      
      if rstn_sys = '0' then
        r_lemo_dir <= (others => '0');
        r_gpio_mux <= (others => '0');
        r_gpio_val <= (others => '0');
        r_resets   <= (others => '0');
      else
        -- Detect a write to the register byte
        if gpio_slave_i.cyc = '1' and gpio_slave_i.stb = '1' and
           gpio_slave_i.we = '1' and gpio_slave_i.sel(0) = '1' then
          case to_integer(unsigned(gpio_slave_i.adr(3 downto 2))) is
            when 0 => r_gpio_val <= gpio_slave_i.dat(r_gpio_val'range);
            when 1 => r_lemo_dir <= gpio_slave_i.dat(r_lemo_dir'range);
            when 2 => r_gpio_mux <= gpio_slave_i.dat(r_gpio_mux'range);
            when 3 => r_resets   <= gpio_slave_i.dat(r_resets'range);
            when others => null;
          end case;
        end if;
        
        case to_integer(unsigned(gpio_slave_i.adr(4 downto 2))) is
          when 0 => gpio_slave_o.dat(r_gpio_val'range) <= r_gpio_val;
          when 1 => gpio_slave_o.dat(r_lemo_dir'range) <= r_lemo_dir;
          when 2 => gpio_slave_o.dat(r_gpio_mux'range) <= r_gpio_mux;
          when 3 => gpio_slave_o.dat(r_resets'range)   <= r_resets;
          when 4 => gpio_slave_o.dat(s_lemo_in'range)  <= s_lemo_in;
          when others => null;
        end case;
      end if;
    end if;
  end process;
  
  gpio_slave_o.int <= '0'; -- In my opinion, this should not be in the structure.
  gpio_slave_o.err <= '0';
  gpio_slave_o.rty <= '0';
  gpio_slave_o.stall <= '0'; -- This simple example is always ready
  

  -- UART
  UART : xwb_simple_uart
    generic map(
      g_with_virtual_uart   => false,
      g_with_physical_uart  => true,
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE)
    port map(
      clk_sys_i  => clk_sys,
      rst_n_i    => rstn_sys,
      slave_i    => per_cbar_master_o(6),
      slave_o    => per_cbar_master_i(6),
      desc_o     => open,
      uart_rxd_i => '1',
      uart_txd_o => open);
      
  -- OLED display
  dcon :  display_console
    port map(	
      clk_i      => clk_sys,
      nRst_i     => rstn_sys,
      slave_i    => per_cbar_master_o(7),
      slave_o    => per_cbar_master_i(7),
      RST_DISP_o => hpla_ch(8),
      DC_SPI_o   => hpla_ch(6),
      SS_SPI_o   => hpla_ch(4),
      SCK_SPI_o  => hpla_ch(2),	
      SD_SPI_o   => hpla_ch(10),
      SH_VR_o    => hpla_ch(0));
    

        
  -- END OF GSI WB Periphery
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- WR Core -----------------------------------------------------------------------
  ----------------------------------------------------------------------------------
  U_WR_CORE : xwr_core
    generic map (
      g_simulation                => 0,
      g_phys_uart                 => true,
      g_virtual_uart              => false,
      g_with_external_clock_input => true,
      g_aux_clks                  => 1,
      g_ep_rxbuf_size             => 1024,
      g_dpram_initf               => "../../../ip_cores/wrpc-sw/wrc.mif",
      g_dpram_size                => 131072/4,
      g_interface_mode            => PIPELINED,
      g_address_granularity       => BYTE,
      g_aux_sdb                   => c_etherbone_sdb)
    port map (
      clk_sys_i  => clk_sys,
      clk_dmtd_i => clk_dmtd,
      clk_ref_i  => clk_ref,
      clk_aux_i  => (others => '0'),
      clk_ext_i  => '0', -- g_with_external_clock_input controls usage
      pps_ext_i  => '0',
      rst_n_i    => rstn_wr,

      dac_hpll_load_p1_o => dac_hpll_load_p1,
      dac_hpll_data_o    => dac_hpll_data,
      dac_dpll_load_p1_o => dac_dpll_load_p1,
      dac_dpll_data_o    => dac_dpll_data,
		
      phy_ref_clk_i      => clk_ref,
      phy_tx_data_o      => phy_tx_data,
      phy_tx_k_o         => phy_tx_k,
      phy_tx_disparity_i => phy_tx_disparity,
      phy_tx_enc_err_i   => phy_tx_enc_err,
      phy_rx_data_i      => phy_rx_data,
      phy_rx_rbclk_i     => phy_rx_rbclk,
      phy_rx_k_i         => phy_rx_k,
      phy_rx_enc_err_i   => phy_rx_enc_err,
      phy_rx_bitslide_i  => phy_rx_bitslide,
      phy_rst_o          => phy_rst,
      phy_loopen_o       => phy_loopen,
      
      led_act_o   => link_act,
      led_link_o  => link_up,
      
      scl_o       => open, -- No second I2C bus on SCU
      scl_i       => '0',
      sda_i       => '0',
      sda_o       => open,
      sfp_scl_i   => sfp2_scl_i,
      sfp_sda_i   => sfp2_sda_i,
      sfp_scl_o   => sfp2_scl_o,
      sfp_sda_o   => sfp2_sda_o,
      sfp_det_i   => sfp2_det_i,
      btn1_i      => '0',
      btn2_i      => '0',

      uart_rxd_i => uart_rxd_i(0),
      uart_txd_o => uart_txd_o(0),
      
      owr_pwren_o => owr_pwren,
      owr_en_o    => owr_en,
      owr_i       => owr,
      slave_i => top_cbar_master_o(1),
      slave_o => top_cbar_master_i(1),

      wrf_src_o => mb_snk_in,
      wrf_src_i => mb_snk_out,
      wrf_snk_o => mb_src_in,
      wrf_snk_i => mb_src_out,

      aux_master_o => wrc_master_o,
      aux_master_i => wrc_master_i,
 
      tm_link_up_o         => open,
      tm_dac_value_o       => open,
      tm_dac_wr_o          => open,
      tm_clk_aux_lock_en_i => (others => '0'),
      tm_clk_aux_locked_o  => open,
      tm_time_valid_o      => tm_valid,
      tm_tai_o             => tm_tai,
      tm_cycles_o          => tm_cycles,
      pps_p_o              => pps,
      
      dio_o                => open,
      rst_aux_n_o          => open,
      link_ok_o            => open);

 

  -- END OF WR Core
  ----------------------------------------------------------------------------------


  ----------------------------------------------------------------------------------
  -- System IOs --------------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
    -- LPC UART
  lpc_slave: lpc_uart
    port map(
      lpc_clk         => LPC_FPGA_CLK,
      lpc_serirq      => LPC_SERIRQ,
      lpc_ad          => LPC_AD,
      lpc_frame_n     => nLPC_FRAME,
      lpc_reset_n     => nPCI_RESET and rstn_wr, -- !!! add signal
      
      kbc_out_port    => kbc_out_port,
      kbc_in_port     => x"00",
      
      serial_rxd      => uart_rxd_i(1),
      serial_txd      => uart_txd_o(1),
      serial_dtr      => open,
      serial_dcd      => '0',
      serial_dsr      => '0',
      serial_ri       => '0',
      serial_cts      => '0',
      serial_rts      => open,
      seven_seg_L     => open,
      seven_seg_H     => open);
  
    U_Extend_PPS : gc_extend_pulse
    generic map (
      g_width => 10000000)
    port map (
      clk_i      => clk_sys,
      rst_n_i    => rstn_sys,
      pulse_i    => pps,
      extended_o => ext_pps);
  
  -- open drain buffer for one wire
  owr(0) <= OneWire_CB;
  OneWire_CB <= owr_pwren(0) when (owr_pwren(0) = '1' or owr_en(0) = '1') else 'Z';
  
  -- no second onewire is connected
  owr(1) <= 'Z';
  
  -- connects the serial ports to the carrier board
  serial_to_cb_o <= '0';
  
  -- Disable SFP1
  sfp1_tx_disable_o <= '1';
  sfp1_mod1 <= 'Z';
  sfp1_mod2 <= 'Z';
  
  -- Enable SFP2 as timing
  sfp2_tx_disable_o <= '0';
  sfp2_scl_i <= sfp2_mod1;
  sfp2_sda_i <= sfp2_mod2;
  sfp2_det_i <= sfp2_mod0;
  sfp2_mod1  <= '0' when sfp2_scl_o = '0' else 'Z';
  sfp2_mod2  <= '0' when sfp2_sda_o = '0' else 'Z';
  
  -- lemo input register
  lemo_in: process (clk_sys)
  begin
    if rising_edge(clk_sys) then
      s_lemo_in(0) <= lemo_io(1);
      s_lemo_in(1) <= lemo_io(2);
      s_lemo_in(2) <= '0';
      s_lemo_in(3) <= '0';
    end if;
  
  end process;
  
  -- Output MUXes
  with r_gpio_mux(1 downto 0) select
    s_lemo_dat(1) <= 
      '0'           when "00",
      eca_gpio(0)   when "01",
      r_gpio_val(0) when "10",
      '-'           when others;
  
  with r_gpio_mux(3 downto 2) select
    s_lemo_dat(2) <= 
      ext_pps       when "00",
      eca_gpio(1)   when "01",
      r_gpio_val(1) when "10",
      '-'           when others;
  
  with r_gpio_mux(5 downto 4) select
    s_uled_dat(1) <= 
      ext_pps       when "00",
      eca_gpio(2)   when "01",
      r_gpio_val(2) when "10",
      '-'           when others;
  
  with r_gpio_mux(7 downto 6) select
    s_uled_dat(2) <= 
      link_act      when "00",
      eca_gpio(3)   when "01",
      r_gpio_val(3) when "10",
      '-'           when others;
  
  -- LEMO control
  lemo_en_in <= r_lemo_dir;
  lemo_io(1) <= s_lemo_dat(1) when r_lemo_dir(0) = '0' else 'Z';
  lemo_io(2) <= s_lemo_dat(2) when r_lemo_dir(1) = '0' else 'Z';
  
  -- LED control
  leds_o(1) <= not s_uled_dat(1);
  leds_o(2) <= not s_uled_dat(2);
  leds_o(3) <= not link_up;
  leds_o(4) <= not tm_valid;
  
  -- Extend LEMO input/outputs to LEDs at 20Hz
  lemo_leds : for i in 1 to 2 generate
    lemo_led(i) <= not s_lemo_led(i);
    lemo_ledx : gc_extend_pulse
      generic map(
        g_width => 125_000_000/20) -- 20 Hz
      port map(
        clk_i      => clk_ref,
        rst_n_i    => rstn_ref,
        pulse_i    => lemo_io(i),
        extended_o => s_lemo_led(i));
  end generate;
  
  -- Logic analyzer port (0,2,4,6,8,10 = OLED)
  -- Don't put debug clocks too close (makes display flicker)
  hpla_clk <= 'Z';
  hpla_ch <= (others => 'Z');
  
  -- hpla_ch(13) <= clk_ref;      -- pin 6
  -- hpla_ch(14) <= dbg_tx_clk;   -- pin 5
  -- hpla_ch(15) <= phy_rx_rbclk; -- pin 4
  -- 20 is ground
  
  -- EXT CONN not connected
  IO_2_5            <= (others => 'Z');
  a_EXT_LVDS_CLKOUT <= '0';
  EIO               <= (others => 'Z');
  
  A_EXT_LVDS_TX(0) <= clk_butis;
  A_EXT_LVDS_TX(1) <= clk_ref;
  A_EXT_LVDS_TX(2) <= '0';
  A_EXT_LVDS_TX(3) <= '0';
  
  -- Parallel Flash not connected
  nRST_FSH <= '0';
  AD <= (others => 'Z');
  DF <= (others => 'Z');
  ADV_FSH  <= 'Z';
  nCE_FSH  <= 'Z';
  CLK_FSH  <= 'Z';
  nWE_FSH  <= 'Z';
  nOE_FSH  <= 'Z';
  
  -- DDR3 not connected
  DDR3_RES_n <= '0';
  DDR3_DQ    <= (others => 'Z');
  DDR3_DM    <= (others => 'Z');
  DDR3_BA    <= (others => 'Z');
  DDR3_ADDR  <= (others => 'Z');
  DDR3_CS_n  <= (others => 'Z');
  DDR3_CKE   <= (others => 'Z');
  DDR3_ODT   <= (others => 'Z');
  DDR3_CAS_n <= 'Z';
  DDR3_RAS_n <= 'Z';
  DDR3_WE_n  <= 'Z';
  
  -- External reset values
  nFPGA_Res_Out <= not r_resets(0) and rstn_wr;
  nPWRBTN       <= not r_resets(1);
  A_nCONFIG     <= not r_resets(2);

  --Internal Resets
  rst_usr_lm32_n <= not r_resets(3) and rstn_sys;
  
  -- END OF System IOs
  ----------------------------------------------------------------------------------
  
end rtl;
