library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity ftm10 is
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    clk_20m_vcxo_i        : in std_logic; -- 20MHz VCXO clock
    clk_20m_vcxo_alt_i    : in std_logic; -- 20MHz VCXO clock alternative

    clk_125m_pllref_i     : in std_logic; -- 125 MHz PLL reference
    clk_125m_local_i      : in std_logic; -- Local clk from 125Mhz oszillator
    clk_125m_sfpref_i     : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator

    clk_125m_pllref_alt_i : in std_logic; -- 125 MHz PLL reference alternative
    clk_125m_local_alt_i  : in std_logic; -- Local clk from 125Mhz oszillator alternative
    clk_125m_sfpref_alt_i : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator alternative

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
    -- OneWire
    -----------------------------------------------------------------------
    rom_data_io     : inout std_logic;
    rom_aux_data_io : inout std_logic;

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    fpga_res_i : in std_logic;
    nres_i     : in std_logic;

    -----------------------------------------------------------------------
    -- LVTTL IOs
    -----------------------------------------------------------------------
    lemo_p_i : in    std_logic_vector(19 downto 0);
    lemo_n_i : in    std_logic_vector(19 downto 0);
    lemo_p_o : out   std_logic_vector(19 downto 0);
    lemo_n_o : out   std_logic_vector(19 downto 0);

    -----------------------------------------------------------------------
    -- I2C
    -----------------------------------------------------------------------
    i2c_scl_pad_io   : inout std_logic_vector(4 downto 0);
    i2c_sda_pad_io   : inout std_logic_vector(4 downto 0);

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds_o                  : out std_logic_vector(3 downto 0) := (others => '1');
    wr_aux_leds_or_node_leds_o : out std_logic_vector(3 downto 0) := (others => '1');
    rt_leds_o                  : out std_logic_vector(3 downto 0) := (others => '1');

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    usb_slrd_o   : out   std_logic;
    usb_slwr_o   : out   std_logic;
    usb_fd_io    : inout std_logic_vector(7 downto 0);
    usb_pa_io    : inout std_logic_vector(7 downto 0) := (others => 'Z');
    usb_ctl_i    : in    std_logic_vector(2 downto 0);
    usb_uclk_i   : in    std_logic;
    usb_ures_o   : out   std_logic;
    usb_uclkin_i : in    std_logic;

    -----------------------------------------------------------------------
    -- CPLD
    -----------------------------------------------------------------------
    cpld_io : inout std_logic_vector(9 downto 0);

    -----------------------------------------------------------------------
    -- SFP (main WR Interface)
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic := '0';
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic;

    -----------------------------------------------------------------------
    -- SFP (auxiliary)
    -----------------------------------------------------------------------
    sfp_aux_tx_disable_o : out   std_logic := '0';
    sfp_aux_tx_fault_i   : in    std_logic;
    sfp_aux_los_i        : in    std_logic;
    sfp_aux_txp_o        : out   std_logic;
    sfp_aux_rxp_i        : in    std_logic;
    sfp_aux_mod0_i       : in    std_logic;
    sfp_aux_mod1_io      : inout std_logic;
    sfp_aux_mod2_io      : inout std_logic);

end ftm10;

architecture rtl of ftm10 is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_led_aux_link_up  : std_logic;
  signal s_led_aux_link_act : std_logic;
  signal s_led_aux_track    : std_logic;
  signal s_led_aux_pps      : std_logic;

  signal s_gpio_o       : std_logic_vector(13 downto 0);
  signal s_gpio_i       : std_logic_vector(9 downto 0);
  signal s_lvds_p_i     : std_logic_vector(19 downto 0);
  signal s_lvds_n_i     : std_logic_vector(19 downto 0);
  signal s_lvds_p_o     : std_logic_vector(19 downto 0);
  signal s_lvds_n_o     : std_logic_vector(19 downto 0);

  signal s_i2c_scl_pad_out  : std_logic_vector(4 downto 0);
  signal s_i2c_scl_pad_in   : std_logic_vector(4 downto 0);
  signal s_i2c_scl_padoen   : std_logic_vector(4 downto 0);
  signal s_i2c_sda_pad_out  : std_logic_vector(4 downto 0);
  signal s_i2c_sda_pad_in   : std_logic_vector(4 downto 0);
  signal s_i2c_sda_padoen   : std_logic_vector(4 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 33) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("CPLD_IO_0  ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_1  ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_2  ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_3  ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_4  ",  IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_5  ",  IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_6  ",  IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_7  ",  IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_8  ",  IO_NONE,         false,   false,  8,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_9  ",  IO_NONE,         false,   false,  9,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("LED1_BASE_R",  IO_NONE,         false,   false, 10,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_BASE_B",  IO_NONE,         false,   false, 11,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_BASE_G",  IO_NONE,         false,   false, 12,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_BASE_W",  IO_NONE,         false,   false, 13,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("USBC1_IO1  ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC1_IO2  ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC1_IO3  ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC1_IO4  ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC1_IO5  ",  IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC2_IO1  ",  IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC2_IO2  ",  IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC2_IO3  ",  IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC2_IO4  ",  IO_NONE,         false,   false,  8,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC2_IO5  ",  IO_NONE,         false,   false,  9,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC3_IO1  ",  IO_NONE,         false,   false, 10,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC3_IO2  ",  IO_NONE,         false,   false, 11,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC3_IO3  ",  IO_NONE,         false,   false, 12,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC3_IO4  ",  IO_NONE,         false,   false, 13,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC3_IO5  ",  IO_NONE,         false,   false, 14,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC4_IO1  ",  IO_NONE,         false,   false, 15,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC4_IO2  ",  IO_NONE,         false,   false, 16,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC4_IO3  ",  IO_NONE,         false,   false, 17,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC4_IO4  ",  IO_NONE,         false,   false, 18,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("USBC4_IO5  ",  IO_NONE,         false,   false, 19,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS)
  );

  constant c_family        : string := "Arria 10 GX FTM10";
  constant c_project       : string := "ftm10";
  constant c_cores         : natural:= 4;
  constant c_initf_name    : string := c_project & "_stub.mif";
  constant c_profile_name  : string := "medium_icache_debug";

begin

  main : monster
    generic map(
      g_family             => c_family,
      g_project            => c_project,
      g_flash_bits         => 25, -- !!! TODO: Check this
      g_gpio_out           => 4,
      g_gpio_inout         => 10,
      g_lvds_inout         => 20,
      g_en_i2c_wrapper     => true,
      g_num_i2c_interfaces => 5,
      g_en_pcie            => true,
      g_en_tlu             => false,
      g_en_usb             => true,
      g_io_table           => io_mapping_table,
      g_a10_use_sys_fpll   => false,
      g_a10_use_ref_fpll   => false,
      g_dual_port_wr       => true,
      g_en_eca             => false,
      g_delay_diagnostics  => true,
      g_lm32_are_ftm       => true,
      g_lm32_MSIs          => 1,
      g_lm32_cores         => c_cores,
      g_lm32_ramsizes      => c_lm32_ramsizes/4,
      g_lm32_init_files    => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles      => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i     => clk_20m_vcxo_i,
      core_clk_125m_pllref_i  => clk_125m_tcb_pllref_i,
      core_clk_125m_local_i   => clk_125m_tcb_local_i,
      core_clk_125m_sfpref_i  => clk_125m_tcb_sfpref_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_onewire_io           => rom_data_io,
      wr_aux_onewire_io       => rom_aux_data_io,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_aux_sfp_sda_io       => sfp_aux_mod2_io,
      wr_aux_sfp_scl_io       => sfp_aux_mod1_io,
      wr_aux_sfp_det_i        => sfp_aux_mod0_i,
      wr_aux_sfp_tx_o         => sfp_aux_txp_o,
      wr_aux_sfp_rx_i         => sfp_aux_rxp_i,
      sfp_tx_disable_o        => open,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      sfp_aux_tx_disable_o    => open,
      sfp_aux_tx_fault_i      => sfp_aux_tx_fault_i,
      sfp_aux_los_i           => sfp_aux_los_i,
      i2c_scl_pad_i           => s_i2c_scl_pad_in,
      i2c_scl_pad_o           => s_i2c_scl_pad_out,
      i2c_scl_padoen_o        => s_i2c_scl_padoen,
      i2c_sda_pad_i           => s_i2c_sda_pad_in,
      i2c_sda_pad_o           => s_i2c_sda_pad_out,
      i2c_sda_padoen_o        => s_i2c_sda_padoen,
      gpio_o                  => s_gpio_o,
      gpio_i                  => s_gpio_i,
      lvds_p_i                => s_lvds_p_i,
      lvds_n_i                => s_lvds_n_i,
      lvds_p_o                => s_lvds_p_o,
      lvds_n_o                => s_lvds_n_o,
      usb_rstn_o              => usb_ures_o,
      usb_ebcyc_i             => usb_pa_io(3),
      usb_speed_i             => usb_pa_io(0),
      usb_shift_i             => usb_pa_io(1),
      usb_readyn_io           => usb_pa_io(7),
      usb_fifoadr_o           => usb_pa_io(5 downto 4),
      usb_sloen_o             => usb_pa_io(2),
      usb_fulln_i             => usb_ctl_i(1),
      usb_emptyn_i            => usb_ctl_i(2),
      usb_slrdn_o             => usb_slrd_o,
      usb_slwrn_o             => usb_slwr_o,
      usb_pktendn_o           => usb_pa_io(6),
      usb_fd_io               => usb_fd_io,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      led_aux_link_up_o       => s_led_aux_link_up,
      led_aux_link_act_o      => s_led_aux_link_act,
      led_aux_track_o         => s_led_aux_track,
      led_aux_pps_o           => s_led_aux_pps,
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o);

  -- SFPs
  sfp_tx_disable_o     <= '0';
  sfp_aux_tx_disable_o <= '0';

  -- LEDs
  wr_leds_o(0)     <= not (s_led_link_act and s_led_link_up);         -- red   = traffic/no-link
  wr_leds_o(1)     <= not s_led_link_up;                              -- blue  = link
  wr_leds_o(2)     <= not s_led_track;                                -- green = timing valid
  wr_leds_o(3)     <= not s_led_pps;                                  -- white = PPS

  wr_aux_leds_or_node_leds_o(0) <= not (s_led_aux_link_act and s_led_aux_link_up); -- red   = traffic/no-link
  wr_aux_leds_or_node_leds_o(1) <= not s_led_aux_link_up;                          -- blue  = link
  wr_aux_leds_or_node_leds_o(2) <= not s_led_aux_track;                            -- green = timing valid
  wr_aux_leds_or_node_leds_o(3) <= not s_led_aux_pps;                              -- white = PPS

  rt_leds_o        <= not s_gpio_o(13 downto 10);

  -- LEMOs
  lemos : for i in 0 to 19 generate
    s_lvds_p_i(i)      <= lemo_p_i(i);
    s_lvds_n_i(i)      <= lemo_n_i(i);
    lemo_p_o(i)        <= s_lvds_p_o(i);
    lemo_n_o(i)        <= s_lvds_n_o(i);
  end generate;

  -- I2C
  interfaces : for i in 0 to 4 generate
    i2c_scl_pad_io(i)   <= s_i2c_scl_pad_out(i) when (s_i2c_scl_padoen(i) = '0') else 'Z';
    i2c_sda_pad_io(i)   <= s_i2c_sda_pad_out(i) when (s_i2c_sda_padoen(i) = '0') else 'Z';
    s_i2c_scl_pad_in(i) <= i2c_scl_pad_io(i);
    s_i2c_sda_pad_in(i) <= i2c_sda_pad_io(i);
  end generate;

  -- CPLD
  s_gpio_i(9 downto 0) <= cpld_io(9 downto 0);
  cpld_con : for i in 0 to 9 generate
    cpld_io(i) <= s_gpio_o(i) when s_gpio_o(i)='0' else 'Z';
  end generate;

end rtl;
