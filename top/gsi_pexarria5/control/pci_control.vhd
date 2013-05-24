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
use work.oled_display_pkg.all;

entity pci_control is
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
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB     : inout std_logic;
    
    -----------------------------------------------------------------------
    -- Timing SFP 
    -----------------------------------------------------------------------
    sfp2_ref_clk_i    : in  std_logic;
    
    sfp2_tx_disable_o : out std_logic := '0';
    sfp2_txp_o        : out std_logic;
    sfp2_rxp_i        : in  std_logic;
    
    sfp2_mod0         : in    std_logic; -- grounded by module
    sfp2_mod1         : inout std_logic; -- SCL
    sfp2_mod2         : inout std_logic); -- SDA

end pci_control;

architecture rtl of pci_control is
  
  -- WR core layout
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  -- Top crossbar layout
  constant c_slaves  : natural := 5;
  constant c_masters : natural := 2;
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,          x"00000000"),
    1 => f_sdb_embed_device(c_xwr_wb_timestamp_latch_sdb, x"00100000"),
    2 => f_sdb_embed_device(c_eca_sdb,                    x"00100800"),
    3 => f_sdb_embed_device(c_eca_evt_sdb,                x"00100C00"),
    4 => f_sdb_embed_device(c_wb_spi_flash_sdb,           x"01000000"));
  constant c_sdb_address : t_wishbone_address := x"00300000";

  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
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
  signal rstn_ref         : std_logic;
  
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
  signal r_resets   : std_logic_vector(2 downto 0) := (others => '0');
  
  signal s_lemo_dat : std_logic_vector(2 downto 1);
  signal s_uled_dat : std_logic_vector(2 downto 1);
  signal s_lemo_led : std_logic_vector(2 downto 1);
begin

  dmtd_inst : dmtd_pll port map(
    inclk0 => clk_20m_vcxo_i,    --  20  Mhz 
    c0     => clk_dmtd,          --  62.5MHz
    locked => open);
  
  ref_inst : ref_pll port map(
    inclk0 => clk_125m_pllref_i, -- 125 MHz
    c0     => clk_ref,           -- 125 MHz
    locked => ref_locked);

  sys_inst : sys_pll port map(
    inclk0 => clk_125m_local_i, -- 125  Mhz 
    c0     => clk_62_5,         --  62.5MHz
    c1     => clk_50,           --  50  Mhz
    c2     => clk_20,           --  20  MHz
    locked => sys_locked);
  
  clk_sys    <= clk_62_5;
  clk_reconf <= pcie_refclk_i; -- required when Hard IP active
  clk_flash  <= clk_62_5; -- 100 MHz is max
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

  flash : flash_top
    generic map(
      g_family                 => "Arria V",
      g_port_width             => 1,   -- single-lane SPI bus
      g_addr_width             => 24,  -- 3 byte addressed chip
      g_input_latch_edge       => '1', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
      g_output_latch_edge      => '0', -- falling edge to meet SPI setup times
      g_input_to_output_cycles => 2)   -- delayed to work-around unconstrained design
    port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => cbar_master_o(4),
      slave_o   => cbar_master_i(4),
      clk_out_i => clk_flash,
      clk_in_i  => clk_flash); -- no need to phase shift at 50MHz
  
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
  
  rstn_wr <= rstn_sys and gxb_locked;
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

      uart_rxd_i => '0',
      uart_txd_o => open,
      
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
 
      tm_link_up_o         => open,
      tm_dac_value_o       => open,
      tm_dac_wr_o          => open,
      tm_clk_aux_lock_en_i => '0',
      tm_clk_aux_locked_o  => open,
      tm_time_valid_o      => tm_valid,
      tm_tai_o             => tm_tai,
      tm_cycles_o          => tm_cycles,
      pps_p_o              => pps,
      
      dio_o                => open,
      rst_aux_n_o          => open,
      link_ok_o            => open);

  wr_arria5_phy_inst : wr_arria5_phy
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
      clk_i      => clk_sys,
      rst_n_i    => rstn_sys,
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
      triggers_i(0)   => lemo_io(1),
      triggers_i(1)   => lemo_io(2),
      tm_time_valid_i => tm_valid,
      tm_tai_i        => tm_tai,
      tm_cycles_i     => tm_cycles,
      wb_slave_i      => cbar_master_o(1),
      wb_slave_o      => cbar_master_i(1));

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
  
  -- open drain buffer for one wire
  owr(0) <= OneWire_CB;
  OneWire_CB <= owr_pwren(0) when (owr_pwren(0) = '1' or owr_en(0) = '1') else 'Z';
  
  -- no second onewire is connected
  owr(1) <= 'Z';
  
  -- Enable SFP2 as timing
  sfp2_tx_disable_o <= '0';
  sfp2_scl_i <= sfp2_mod1;
  sfp2_sda_i <= sfp2_mod2;
  sfp2_det_i <= sfp2_mod0;
  sfp2_mod1  <= '0' when sfp2_scl_o = '0' else 'Z';
  sfp2_mod2  <= '0' when sfp2_sda_o = '0' else 'Z';
  
end rtl;
