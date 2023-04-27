library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity idrogen is
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    clk_20m_vcxo_i        : in std_logic; -- 20MHz VCXO clock
    clk_20m_vcxo_alt_i    : in std_logic; -- 20MHz VCXO clock alternative

    clk_125m_local_i      : in std_logic; -- Local clk from 125Mhz oszillator
    clk_125m_local_alt_i  : in std_logic; -- Local clk from 125Mhz oszillator alternative

    clk_125m_tcb_pllref_i : in std_logic; -- 125 MHz PLL reference at tranceiver bank
    clk_125m_tcb_local_i  : in std_logic; -- Local clk from 125Mhz oszillator at tranceiver bank
    clk_125m_tcb_sfpref_i : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator at tranceiver bank

    ------------------------------------------------------------------------
    -- PCI express pins
    ------------------------------------------------------------------------
    pcie_refclk_i : in    std_logic;
    pcie_rx_i     : in    std_logic_vector(3 downto 0);
    pcie_tx_o     : out   std_logic_vector(3 downto 0);
    nPCI_RESET_i  : in    std_logic;

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o : out std_logic;
    wr_dac_din_o  : out std_logic;
    wr_ndac_cs_o  : out std_logic_vector(2 downto 1);

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic;
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic);

end idrogen;

architecture rtl of idrogen is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_gpio_o    : std_logic_vector(6 downto 0);
  signal s_lvds_p_i  : std_logic_vector(2 downto 0);
  signal s_lvds_n_i  : std_logic_vector(2 downto 0);
  signal s_lvds_p_o  : std_logic_vector(2 downto 0);
  signal s_lvds_term : std_logic_vector(2 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 1) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED_USR_1  ",  IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR_2  ",  IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL)
  );

  constant c_family       : string := "Arria 10 GX IDROGEN";
  constant c_project      : string := "idrogen";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & "_stub.mif";
  constant c_profile_name : string := "medium_icache_debug";
  constant c_psram_bits   : natural := 24;

begin

  main : monster
    generic map(
      g_family             => c_family,
      g_project            => c_project,
      g_flash_bits         => 25, -- !!! TODO: Check this
      g_psram_bits         => c_psram_bits,
      g_gpio_out           => 2,
      g_en_pcie            => true,
      g_en_tlu             => false,
      g_io_table           => io_mapping_table,
      g_en_tempsens        => false,
      g_a10_use_sys_fpll   => false,
      g_a10_use_ref_fpll   => false,
      g_lm32_cores         => c_cores,
      g_lm32_ramsizes      => c_lm32_ramsizes/4,
      g_lm32_init_files    => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles      => f_string_list_repeat(c_profile_name, c_cores),
      g_en_asmi            => true
    )
    port map(
      core_clk_20m_vcxo_i     => clk_20m_vcxo_i,
      core_clk_125m_pllref_i  => clk_125m_tcb_pllref_i,
      core_clk_125m_local_i   => clk_125m_tcb_local_i,
      core_clk_125m_sfpref_i  => clk_125m_tcb_sfpref_i,
      wr_onewire_io           => OneWire_CB,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_uart_o               => ser0_rxd,
      wr_uart_i               => ser0_txd,
      wbar_phy_dis_o          => sfp_tx_disable_o,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      gpio_i                  => lemo_in,
      gpio_o                  => s_gpio_o,
      lvds_p_i                => s_lvds_p_i,
      lvds_n_i                => s_lvds_n_i,
      lvds_p_o                => s_lvds_p_o,
      lvds_term_o             => s_lvds_term,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      scubus_a_a              => A_A,
      scubus_a_d              => A_D,
      scubus_nsel_data_drv    => nSel_Ext_Data_DRV,
      scubus_a_nds            => A_nDS,
      scubus_a_rnw            => A_RnW,
      scubus_a_ndtack         => A_nDtack,
      scubus_a_nsrq           => A_nSRQ,
      scubus_a_nsel           => A_nSEL,
      scubus_a_ntiming_cycle  => A_nTiming_Cycle,
      scubus_a_sysclock       => A_SysClock,
      ow_io(0)                => onewire_ext,
      ow_io(1)                => A_OneWire,
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o,
      -- I2C
      i2c_scl_pad_i            => s_i2c_scl_pad_in,
      i2c_scl_pad_o            => s_i2c_scl_pad_out,
      i2c_scl_padoen_o         => s_i2c_scl_padoen,
      i2c_sda_pad_i            => s_i2c_sda_pad_in,
      i2c_sda_pad_o            => s_i2c_sda_pad_out,
      i2c_sda_padoen_o         => s_i2c_sda_padoen,
      -- FX2 USB
      usb_rstn_o              => ures,
      usb_ebcyc_i             => pa(3),
      usb_speed_i             => pa(0),
      usb_shift_i             => pa(1),
      usb_readyn_io           => pa(7),
      usb_fifoadr_o           => pa(5 downto 4),
      usb_sloen_o             => pa(2),
      usb_fulln_i             => ctl(1),
      usb_emptyn_i            => ctl(2),
      usb_slrdn_o             => slrd,
      usb_slwrn_o             => slwr,
      usb_pktendn_o           => pa(6),
      usb_fd_io               => fd,
      -- PSRAM TODO: Multi Chip
      ps_clk                 => psram_clk,
      ps_addr                => psram_a,
      ps_data                => psram_dq,
      ps_seln(0)             => psram_ubn,
      ps_seln(1)             => psram_lbn,
      ps_cen                 => psram_cen (0),
      ps_oen                 => psram_oen,
      ps_wen                 => psram_wen,
      ps_cre                 => psram_cre,
      ps_advn                => psram_advn,
      ps_wait                => psram_wait,
      hw_version             => x"0000000" & not scu_cb_version);

  -- LEDs
  wr_led_pps    <= not s_led_pps;                            -- white = PPS
  wr_rgb_led(0) <= not s_led_link_act;                       -- WR-RGB Red
  wr_rgb_led(1) <= not s_led_track;                          -- WR-RGB Green
  wr_rgb_led(2) <= not (not s_led_track and  s_led_link_up); -- WR-RGB Blue

end rtl;
