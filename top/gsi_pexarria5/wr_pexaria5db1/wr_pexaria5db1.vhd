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
use work.altera_flash_pkg.all;
use work.altera_networks_pkg.all;
use work.build_id_pkg.all;
use work.oled_display_pkg.all;
use work.ez_usb_pkg.all;

entity wr_pexaria5db1 is
  port(
    clk_20m_vcxo_i    : in std_logic;  -- 20MHz VCXO clock
    clk_125m_pllref_i : in std_logic;  -- 125 MHz PLL reference
    clk_125m_local_i  : in std_logic;  -- local clk from 125Mhz oszillator
    
    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_refclk_i  : in  std_logic;
    pcie_rx_i      : in  std_logic_vector(3 downto 0);
    pcie_tx_o      : out std_logic_vector(3 downto 0);
    nPCI_RESET     : in std_logic;
    
    pe_smdat        : inout std_logic;
    pe_snclk        : out std_logic;
    pe_waken        : out std_logic;
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk       : out std_logic;
    dac_din        : out std_logic;
    ndac_cs        : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data        : inout std_logic;
    
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    di              : out std_logic_vector(6 downto 0);
    ai              : in  std_logic_vector(1 downto 0);
    dout_LCD        : in  std_logic;
    wrdis           : out std_logic := '0';
    dres            : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    fpga_res        : in std_logic;
    nres            : in std_Logic;
    pbs2            : in std_logic;
    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
    ant             : inout std_logic_vector(26 downto 1) := (others => 'Z'); -- trigger bus
    
    p1              : inout std_logic := 'Z';
    p2              : inout std_logic := 'Z';
    p3              : inout std_logic := 'Z';
    p4              : inout std_logic := 'Z';
    p5              : inout std_logic := 'Z';
    p6              : inout std_logic := 'Z';
    p7              : inout std_logic := 'Z';
    p8              : inout std_logic := 'Z';
    p9              : inout std_logic := 'Z';
    p10             : inout std_logic := 'Z';
    p11             : inout std_logic := 'Z';
    p12             : out   std_logic := 'Z';
    p13             : out   std_logic := 'Z';
    p14             : inout std_logic := 'Z';
    p15             : inout std_logic := 'Z';
    p16             : inout std_logic := 'Z';
	 p17             : in    std_logic;
    p18             : in    std_logic;
    p19             : out   std_logic;
    p21             : in    std_logic;
    p22             : in    std_logic;
    p23             : in    std_logic;
    p24             : out   std_logic;
    p25             : out   std_logic;
    p26             : in    std_logic;
    p27             : out   std_logic;
    p28             : out   std_logic;
    p29             : out   std_logic;
    p30             : out   std_logic;
	 n1              : inout std_logic := 'Z';
    n2              : inout std_logic := 'Z';
    n3              : inout std_logic := 'Z';
    n4              : inout std_logic := 'Z';
    n5              : inout std_logic := 'Z';
    n6              : inout std_logic := 'Z';
    n7              : inout std_logic := 'Z';
    n8              : inout std_logic := 'Z';
    n9              : inout std_logic := 'Z';
    n10             : inout std_logic := 'Z';
    n11             : inout std_logic := 'Z';
    n12             : out   std_logic := 'Z';
    n13             : out   std_logic := 'Z';
    n14             : inout std_logic := 'Z';
    n15             : inout std_logic := 'Z';
    n16             : inout std_logic := 'Z';
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con             : out std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd            : out   std_logic;
    slwr            : out   std_logic;
    fd              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    pa              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl             : in    std_logic_vector(2 downto 0);
    uclk            : in    std_logic;
    ures            : out   std_logic;
    
    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    led             : out std_logic_vector(8 downto 1) := (others => '1');
    
    -----------------------------------------------------------------------
    -- leds SFPs
    -----------------------------------------------------------------------
    ledsfpr          : out std_logic_vector(4 downto 1);
    ledsfpg          : out std_logic_vector(4 downto 1);

    sfp234_ref_clk_i    : in  std_logic;

    -----------------------------------------------------------------------
    -- SFP1  
    -----------------------------------------------------------------------
    
    sfp1_tx_disable_o : out std_logic := '0';
    sfp1_tx_fault     : in std_logic;
    sfp1_los          : in std_logic;
    
    --sfp1_txp_o        : out std_logic;
    --sfp1_rxp_i        : in  std_logic;
    
    sfp1_mod0         : in    std_logic; -- grounded by module
    sfp1_mod1         : inout std_logic; -- SCL
    sfp1_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP2
    -----------------------------------------------------------------------
    
    sfp2_tx_disable_o : out std_logic := '0';
    sfp2_tx_fault     : in  std_logic;
    sfp2_los          : in  std_logic;
    
    --sfp2_txp_o        : out std_logic;
    --sfp2_rxp_i        : in  std_logic;
    
    sfp2_mod0         : in    std_logic; -- grounded by module
    sfp2_mod1         : inout std_logic; -- SCL
    sfp2_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP3 
    -----------------------------------------------------------------------
       
    sfp3_tx_disable_o : out std_logic := '0';
    sfp3_tx_fault     : in std_logic;
    sfp3_los          : in std_logic;
    
    --sfp3_txp_o        : out std_logic;
    --sfp3_rxp_i        : in  std_logic;
    
    sfp3_mod0         : in    std_logic; -- grounded by module
    sfp3_mod1         : inout std_logic; -- SCL
    sfp3_mod2         : inout std_logic; -- SDA
    
    -----------------------------------------------------------------------
    -- SFP4 
    -----------------------------------------------------------------------
    
    sfp4_tx_disable_o : out std_logic := '0';
    sfp4_tx_fault     : in std_logic;
    sfp4_los          : in std_logic;
    
    sfp4_txp_o        : out std_logic;
    sfp4_rxp_i        : in  std_logic;
    
    sfp4_mod0         : in    std_logic; -- grounded by module
    sfp4_mod1         : inout std_logic; -- SCL
    sfp4_mod2         : inout std_logic); -- SDA
end wr_pexaria5db1;

architecture rtl of wr_pexaria5db1 is
component lvds_rx
	PORT
	(
		rx_enable		: IN STD_LOGIC ;
		rx_in		: IN STD_LOGIC_VECTOR (0 DOWNTO 0);
		rx_inclock		: IN STD_LOGIC ;
		rx_out		: OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
	);
end component;
 component lvds_tx
	PORT
	(
		tx_enable		: IN STD_LOGIC ;
		tx_in		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		tx_inclock		: IN STD_LOGIC ;
		tx_out		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
	);
end component;
component arriav_pll_lvds_output
    generic (
      pll_loaden_enable_disable  : STRING;
      pll_lvdsclk_enable_disable : STRING
    );
    
    port (ccout           : in  std_logic_vector (1 downto 0);
          loaden          : out std_logic ;
          lvdsclk         : out std_logic 
    );
end component;
 
  -- WR core layout
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  -- Top crossbar layout
  constant c_slaves  : natural := 7;
  constant c_masters : natural := 3;
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,          x"00000000"),
    1 => f_sdb_embed_device(c_xwr_wb_timestamp_latch_sdb, x"00100000"),
    2 => f_sdb_embed_device(c_eca_sdb,                    x"00100800"),
    3 => f_sdb_embed_device(c_eca_evt_sdb,                x"00100C00"),
    4 => f_sdb_embed_device(c_wb_serial_lcd_sdb,          x"00100D00"),
    5 => f_sdb_embed_device(c_build_id_sdb,               x"00200000"),
    6 => f_sdb_embed_device(f_wb_spi_flash_sdb(25),       x"04000000"));
  constant c_sdb_address : t_wishbone_address := x"00300000";

  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  signal pcie_slave_i : t_wishbone_slave_in;
  signal pcie_slave_o : t_wishbone_slave_out;
  
  signal gpio_slave_i : t_wishbone_slave_in;
  signal gpio_slave_o : t_wishbone_slave_out;
  
  --------------------------------------------------------------
  -- Clocking
  --------------------------------------------------------------

  -- Non-PLL reset stuff
  signal clk_free         : std_logic;
  signal rstn_free        : std_logic;
  signal gxb_locked       : std_logic;
  signal pll_rst          : std_logic;
  
  -- Sys PLL from clk_125m_local_i
  signal sys_locked       : std_logic;
  signal clk_sys0         : std_logic;
  signal clk_sys1         : std_logic;
  signal clk_sys2         : std_logic;
  signal clk_sys3         : std_logic;
  signal rstn_sys         : std_logic;
  
  signal clk_sys          : std_logic;
  signal clk_reconf       : std_logic;
  signal clk_flash        : std_logic;
  signal clk_display      : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal clk_cascade      : std_logic;
  
  signal ref_locked       : std_logic;
  signal clk_ref0         : std_logic;
  signal clk_ref1         : std_logic;
  signal clk_ref2         : std_logic;
  signal clk_ref3         : std_logic;
  signal clk_ref4         : std_logic;
  
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_phase        : std_logic;
  signal clk_lvds         : std_logic;
  signal clk_enable       : std_logic;
  signal rstn_ref         : std_logic;
  signal rstn_butis       : std_logic;
  signal rstn_phase       : std_logic;
  
  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(4 downto 0);
  
  signal phase_butis      : phase_offset;
  
  -- DMTD PLL from clk_20m_vcxo_i
  signal dmtd_locked      : std_logic;
  signal clk_dmtd0        : std_logic;
  signal clk_dmtd         : std_logic;
  
  --------------------------------------------------------------
  -- White Rabbit
  --------------------------------------------------------------

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
  
  signal tm_up     : std_logic;
  signal tm_valid  : std_logic;
  signal tm_tai    : std_logic_vector(39 downto 0);
  signal tm_cycles : std_logic_vector(27 downto 0);

  signal channels : t_channel_array(1 downto 0);
  
  signal owr_pwren : std_logic_vector(1 downto 0);
  signal owr_en    : std_logic_vector(1 downto 0);
  signal owr       : std_logic_vector(1 downto 0);
  
  signal sfp_scl_o : std_logic;
  signal sfp_scl_i : std_logic;
  signal sfp_sda_o : std_logic;
  signal sfp_sda_i : std_logic;
  signal sfp_det_i : std_logic;
  
  signal eca_gpio : std_logic_vector(15 downto 0);
  
  signal s_uart_rxd_i   : std_logic;
  signal s_uart_txd_o   : std_logic;
  
  signal fd_oen : std_logic;
  signal fd_o   : std_logic_vector(7 downto 0);
  
  signal di_scp : std_logic;
  signal di_lp  : std_logic;
  signal di_flm : std_logic;
  signal di_dat : std_logic;

  signal s_chi1 : std_logic_vector(7 downto 0);
  signal s_chi2 : std_logic_vector(7 downto 0);
  signal s_chi5 : std_logic_vector(7 downto 0);
  signal s_chi6 : std_logic_vector(7 downto 0);
  signal s_chi7 : std_logic_vector(7 downto 0);
  signal r_chi1 : std_logic_vector(7 downto 0);
  signal r_chi2 : std_logic_vector(7 downto 0);
  signal r_chi5 : std_logic_vector(7 downto 0);
  signal r_chi6 : std_logic_vector(7 downto 0);
  signal r_chi7 : std_logic_vector(7 downto 0);
  signal r_cho3 : std_logic_vector(7 downto 0);
  signal r_cho4 : std_logic_vector(7 downto 0);
  signal r_cho5 : std_logic_vector(7 downto 0);
  signal r_cho6 : std_logic_vector(7 downto 0);
  signal r_cho7 : std_logic_vector(7 downto 0);
  
begin

  -- We need at least one off-chip free running clock to setup PLLs
  clk_free <= clk_125m_local_i;

  reset : altera_reset
    port map(
      clk_free_i    => clk_free,
      rstn_i        => pbs2,
      pll_lock_i(0) => dmtd_locked,
      pll_lock_i(1) => ref_locked,
      pll_lock_i(2) => sys_locked,
      pll_lock_i(3) => gxb_locked,
      pll_arst_o    => pll_rst,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => clk_free,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_free);
      
  dmtd_inst : dmtd_pll5 port map(   --  FRACTIONALPLL_X0_Y18_N0 (down-to-up)
    rst      => pll_rst,
    refclk   => clk_20m_vcxo_i,     --  20  Mhz 
    outclk_0 => clk_dmtd0,          --  62.5MHz, counter 12 = PLLOUTPUTCOUNTER_X0_Y20_N1
    locked   => dmtd_locked);
  
  dmtd_clk : single_region port map(
    inclk  => clk_dmtd0,
    outclk => clk_dmtd);
  
  sys_inst : sys_pll5 port map(     -- FRACTIONALPLL_X0_Y60_N0 (up-to-down)
    rst      => pll_rst,
    refclk   => clk_125m_local_i,   -- 125  Mhz 
    outclk_0 => clk_sys0,           --  62.5MHz, counter 0 = PLLOUTPUTCOUNTER_X0_Y67_N1
    outclk_1 => clk_sys1,           --  20  MHz, counter 1 = PLLOUTPUTCOUNTER_X0_Y66_N1
    outclk_2 => clk_sys2,           -- 100  MHz, counter 2 = PLLOUTPUTCOUNTER_X0_Y65_N1
    outclk_3 => clk_sys3,           -- 100  MHz, counter 3 = PLLOUTPUTCOUNTER_X0_Y64_N1
    locked   => sys_locked);

  sys_clk : global_region port map(
    inclk  => clk_sys0,
    outclk => clk_sys);             -- GCLK0
  
  display_clk : single_region port map(
    inclk  => clk_sys1,
    outclk => clk_display);
  
  reconf_clk : dual_region port map(
    inclk  => clk_sys2,
    outclk => clk_reconf);
  
  flash_clk : single_region port map(
    inclk  => clk_sys3,
    outclk => clk_flash);
  
  cascade : global_region port map(
    inclk => clk_125m_pllref_i,
	 outclk => clk_cascade);
  
  ref_inst : ref_pll5 port map(     -- FRACTIONALPLL_X0_Y51_N0 (up-to-down)
    rst        => pll_rst,
    refclk     => clk_cascade,-- 125 MHz
    outclk_0   => clk_ref0,         -- 125 MHz, counter 12 = PLLOUTPUTCOUNTER_X0_Y55_N1
    outclk_1   => clk_ref1,         -- 200 MHz, counter 13 = PLLOUTPUTCOUNTER_X0_Y54_N1
    outclk_2   => clk_ref2,         --  25 MHz, counter 14 = PLLOUTPUTCOUNTER_X0_Y53_N1
	 outclk_3   => clk_ref3,
	 outclk_4   => clk_ref4,
    locked     => ref_locked,
    scanclk    => clk_free,
    cntsel     => phase_sel,
    phase_en   => phase_step,
    updn       => '1',              -- positive phase shift (widen period)
    phase_done => phase_done);
  
  ref_clk : global_region port map(
    inclk  => clk_ref0,
    outclk => clk_ref);             -- GCLK12
  
  butis_clk : global_region port map(
    inclk  => clk_ref1,
    outclk => clk_butis);           -- GCLK13
    
  phase_clk : single_region port map(
    inclk  => clk_ref2,
    outclk => clk_phase);
 
  lvds_clk : arriav_pll_lvds_output
    generic map(
      pll_loaden_enable_disable  => "true",
      pll_lvdsclk_enable_disable => "true")
    port map (
	   ccout   => clk_ref4 & clk_ref3,
		loaden  => clk_enable,
		lvdsclk => clk_lvds);
  
  phase : altera_phase
    generic map(
      g_select_bits   => 5,
      g_outputs       => 3,
      g_base          => 6500/(1000/8), -- 6500ps shift
      g_vco_freq      => 1000, -- 1GHz
      g_output_freq   => (0 => 125, 1 => 200, 2 => 25),
      g_output_select => (0 =>  12, 1 =>  13, 2 => 14))
    port map(
      clk_i       => clk_free,
      rstn_i      => rstn_free,
      clks_i(0)   => clk_ref,
      clks_i(1)   => clk_butis,
      clks_i(2)   => clk_phase,
      rstn_o(0)   => rstn_ref,
      rstn_o(1)   => rstn_butis,
      rstn_o(2)   => rstn_phase,
      offset_i(0) => (others => '0'),
      offset_i(1) => phase_butis,
      offset_i(2) => (others => '0'),
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);
  
  butis : altera_butis
    port map(
      clk_ref_i => clk_ref,
      clk_25m_i => clk_phase,
      pps_i     => pps,
      phase_o   => phase_butis);
  
  id : build_id
    port map(
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,
      slave_i => cbar_master_o(5),
      slave_o => cbar_master_i(5));
  
  flash : flash_top
    generic map(
      g_family                 => "Arria V",
      g_port_width             => 4,  -- quad-lane SPI bus
      g_addr_width             => 25, -- 256Mb = 32MB = 25 bits
      g_dummy_time             => 10,
      g_config                 => true,
      g_input_latch_edge       => '1',
      g_output_latch_edge      => '1',
      g_input_to_output_cycles => 3)
    port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => cbar_master_o(6),
      slave_o   => cbar_master_i(6),
      clk_ext_i => clk_flash,
      clk_out_i => clk_flash,
      clk_in_i  => clk_flash);
  
  GSI_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves  => c_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_layout,
     g_sdb_addr    => c_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => cbar_slave_i,
     slave_o       => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => cbar_master_i,
     master_o      => cbar_master_o);
  
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
      rst_n_i    => rstn_sys,

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
      sfp_scl_i   => sfp_scl_i,
      sfp_sda_i   => sfp_sda_i,
      sfp_scl_o   => sfp_scl_o,
      sfp_sda_o   => sfp_sda_o,
      sfp_det_i   => sfp_det_i,
      btn1_i      => '0',
      btn2_i      => '0',

      uart_rxd_i => s_uart_rxd_i,
      uart_txd_o => s_uart_txd_o,
      
      owr_pwren_o => owr_pwren,
      owr_en_o    => owr_en,
      owr_i       => owr,
      slave_i => cbar_master_o(0),
      slave_o => cbar_master_i(0),

      wrf_src_o => mb_snk_in,
      wrf_src_i => mb_snk_out,
      wrf_snk_o => mb_src_in,
      wrf_snk_i => mb_src_out,

      aux_master_o => wrc_master_o,
      aux_master_i => wrc_master_i,
 
      tm_link_up_o         => tm_up,
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

  -- Enable SFP2 as timing
  sfp4_tx_disable_o <= '0';
  sfp_scl_i <= sfp4_mod1;
  sfp_sda_i <= sfp4_mod2;
  sfp_det_i <= sfp4_mod0;
  sfp4_mod1  <= '0' when sfp_scl_o = '0' else 'Z';
  sfp4_mod2  <= '0' when sfp_sda_o = '0' else 'Z';
  
  wr_arria5_phy_inst : wr_arria5_phy
    port map (
      clk_reconf_i   => clk_reconf,
      clk_phy_i      => sfp234_ref_clk_i,
      locked_o       => gxb_locked,
      loopen_i       => phy_loopen,
      drop_link_i    => phy_rst,
      tx_clk_i       => clk_ref,
      tx_data_i      => phy_tx_data,
      tx_k_i         => phy_tx_k,
      tx_disparity_o => phy_tx_disparity,
      tx_enc_err_o   => phy_tx_enc_err,
      rx_rbclk_o     => phy_rx_rbclk,
      rx_data_o      => phy_rx_data,
      rx_k_o         => phy_rx_k,
      rx_enc_err_o   => phy_rx_enc_err,
      rx_bitslide_o  => phy_rx_bitslide,
      pad_txp_o      => sfp4_txp_o,
      pad_rxp_i      => sfp4_rxp_i);

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

  U_Extend_PPS : gc_extend_pulse
    generic map (
      g_width => 10000000)
    port map (
      clk_i      => clk_ref,
      rst_n_i    => rstn_ref,
      pulse_i    => pps,
      extended_o => ext_pps);
  
   U_ebone : eb_ethernet_slave
     generic map(
       g_sdb_address => x"00000000" & c_sdb_address)
     port map(
       clk_i       => clk_sys,
       nRst_i      => rstn_sys,
       snk_i       => mb_snk_in,
       snk_o       => mb_snk_out,
       src_o       => mb_src_out,
       src_i       => mb_src_in,
       cfg_slave_o => wrc_master_i,
       cfg_slave_i => wrc_master_o,
       master_o    => cbar_slave_i(0),
       master_i    => cbar_slave_o(0));
  
  PCIe : pcie_wb
    generic map(
       g_family => "Arria V",
       sdb_addr => c_sdb_address)
    port map(
       clk125_i      => '0',        -- unused on arria5
       cal_clk50_i   => clk_reconf, -- actually 100MHz on arria5
       
       pcie_refclk_i => pcie_refclk_i,
       pcie_rstn_i   => nPCI_RESET,
       pcie_rx_i     => pcie_rx_i,
       pcie_tx_o     => pcie_tx_o,
       
       master_clk_i  => clk_sys,
       master_rstn_i => rstn_sys,
       master_o      => cbar_slave_i(1),
       master_i      => cbar_slave_o(1),
       
       slave_clk_i   => clk_ref,
       slave_rstn_i  => rstn_ref,
       slave_i       => pcie_slave_i,
       slave_o       => pcie_slave_o);
  
  TLU : wb_timestamp_latch
    generic map (
      g_num_triggers => 2,
      g_fifo_depth   => 10)
    port map (
      ref_clk_i       => clk_ref,
      ref_rstn_i      => rstn_ref,
      sys_clk_i       => clk_sys,
      sys_rstn_i      => rstn_sys,
      triggers_i(0)   => '0',
      triggers_i(1)   => '0',
      tm_time_valid_i => tm_valid,
      tm_tai_i        => tm_tai,
      tm_cycles_i     => tm_cycles,
      wb_slave_i      => cbar_master_o(1),
      wb_slave_o      => cbar_master_i(1));

  ECA0 : wr_eca
    generic map(
      g_eca_name       => f_name("Pexaria5 top"),
      g_channel_names  => (f_name("GPIO: LEDs(12-15)"), 
                           f_name("PCIe: Interrupt generator")),
      g_log_table_size => 7,
      g_log_queue_len  => 8,
      g_num_channels   => 2,
      g_num_streams    => 1)
    port map(
      e_clk_i  (0)=> clk_sys,
      e_rst_n_i(0)=> rstn_sys,
      e_slave_i(0)=> cbar_master_o(3),
      e_slave_o(0)=> cbar_master_i(3),
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => cbar_master_o(2),
      c_slave_o   => cbar_master_i(2),
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
      master_o  => pcie_slave_i,
      master_i  => pcie_slave_o);
      
  -- USB micro controller
  -----------------
  fd <= fd_o when fd_oen='1' else (others => 'Z');
  
  EZUSB : ez_usb
    generic map(
      g_sdb_address => c_sdb_address)
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => cbar_slave_o(2),
      master_o  => cbar_slave_i(2),
      uart_o    => s_uart_rxd_i,
      uart_i    => s_uart_txd_o,
      
      rstn_o    => ures,
      ebcyc_i   => pa(3),
      speed_i   => pa(0),
      shift_i   => pa(1),
      fifoadr_o => pa(5 downto 4),
      readyn_i  => pa(7),
      fulln_i   => ctl(1),
      emptyn_i  => ctl(2),
      sloen_o   => pa(2),
      slrdn_o   => slrd,
      slwrn_o   => slwr,
      pktendn_o => pa(6),
      fd_i      => fd,
      fd_o      => fd_o,
      fd_oen_o  => fd_oen);
      
  -- Display
  display : wb_serial_lcd
   generic map(
      g_wait => 1,
      g_hold => 15)
   port map(
    slave_clk_i  => clk_sys,
    slave_rstn_i => rstn_sys,
    slave_i      => cbar_master_o(4),
    slave_o      => cbar_master_i(4),
    di_clk_i     => clk_display,
    di_scp_o     => di_scp,
    di_lp_o      => di_lp,
    di_flm_o     => di_flm,
    di_dat_o     => di_dat);

  di(3) <= '0' when (di_scp = '0') else 'Z'; -- clock (run at 2MHz)
  di(1) <= '0' when (di_lp  = '0') else 'Z'; -- latch pulse (end-of-40-bit-row)
  di(2) <= '0' when (di_flm = '0') else 'Z'; -- first-line marker
  di(0) <= '0' when (di_dat = '0') else 'Z'; -- shift register in
  di(5) <= '0' when (not tm_up)                  = '1' else 'Z'; -- red
  di(6) <= '0' when (    tm_up and not tm_valid) = '1' else 'Z'; -- blue
  di(4) <= '0' when (    tm_up and     tm_valid) = '1' else 'Z'; -- green

  -- LEDs
  led(1) <= not (link_act and link_up); -- red   = traffic/no-link
  led(2) <= not link_up;                -- blue  = link
  led(3) <= not tm_valid;               -- green = timing valid
  led(4) <= not ext_pps;                -- white = PPS
  led(8 downto 5) <= not eca_gpio(15 downto 12);
  
  ledsfpg(3 downto 1) <= (others => '1');
  ledsfpr(3 downto 1) <= (others => '1');
  ledsfpg(4) <= not link_up;
  ledsfpr(4) <= not link_act;
      
  -- open drain buffer for one wire
  owr(0) <= rom_data;
  rom_data <= owr_pwren(0) when (owr_pwren(0) = '1' or owr_en(0) = '1') else 'Z';
  
  -- no second onewire is connected
  owr(1) <= 'Z';
  
  -- see if we can control these pins as gpio
  n12 <= eca_gpio(0);
  p12 <= eca_gpio(1);
  n13 <= eca_gpio(2);
  p13 <= eca_gpio(3);

  rx1 : lvds_rx port map(
	 rx_inclock => clk_lvds,
    rx_enable  => clk_enable,
	 rx_in(0)   => p17,
	 rx_out     => s_chi1);

  rx2 : lvds_rx port map(
	 rx_inclock => clk_lvds,
    rx_enable  => clk_enable,
	 rx_in(0)   => p18,
	 rx_out     => s_chi2);

  tx3 : lvds_tx port map(
	 tx_inclock => clk_lvds,
    tx_enable  => clk_enable,
	 tx_out(0)   => p19,
	 tx_in       => r_cho3);

  tx4 : lvds_tx port map(
	 tx_inclock => clk_lvds,
    tx_enable  => clk_enable,
	 tx_out(0)  => p24,
	 tx_in      => r_cho4);

  rx5 : lvds_rx port map(
	 rx_inclock => clk_lvds,
    rx_enable  => clk_enable,
	 rx_in(0)   => p21,
	 rx_out     => s_chi5);
  
  tx5 : lvds_tx port map(
	 tx_inclock => clk_lvds,
    tx_enable  => clk_enable,
	 tx_out(0)  => p25,
	 tx_in      => r_cho5);

  rx6 : lvds_rx port map(
	 rx_inclock => clk_lvds,
    rx_enable  => clk_enable,
	 rx_in(0)   => p22,
	 rx_out     => s_chi6);
  
  tx6 : lvds_tx port map(
	 tx_inclock => clk_lvds,
    tx_enable  => clk_enable,
	 tx_out(0)  => p27,
	 tx_in      => r_cho6);

  rx7 : lvds_rx port map(
	 rx_inclock => clk_lvds,
    rx_enable  => clk_enable,
	 rx_in(0)   => p23,
	 rx_out     => s_chi7);
  
  tx7 : lvds_tx port map(
	 tx_inclock => clk_lvds,
    tx_enable  => clk_enable,
	 tx_out(0)   => p28,
	 tx_in       => r_cho7);

  lvds : process(clk_ref) is
  begin
    if rising_edge(clk_ref) then
	   r_chi1 <= s_chi1;
	   r_chi2 <= s_chi2;
	   r_chi5 <= s_chi5;
	   r_chi6 <= s_chi6;
	   r_chi7 <= s_chi7;
		
		r_cho3 <= r_chi1;
		r_cho4 <= r_chi2;
		r_cho5 <= r_chi5;
		r_cho6 <= r_chi6;
		r_cho7 <= r_cho7;
	 end if;
  end process;
  
end rtl;
