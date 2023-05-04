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
    clk_20m_vcxo_i    : in std_logic; -- 20MHz VCXO clock, aka DMTD
    clk_62m5_local_i  : in std_logic; -- Local clk from 62.5Mhz oszillator
    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference at tranceiver bank

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
    sfp_mod2_io      : inout std_logic;

    -----------------------------------------------------------------------
    -- Idrogen special
    -----------------------------------------------------------------------
    DEV_CLRn      : in std_logic; -- FPGA Reset input, active low.
    LMK_CLKREF_2  : in std_logic ; -- reference clocks from LMK
    LMK_CLKREF_12 : in std_logic ; -- reference clocks from LMK

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    gpio_o       : out std_logic_vector(2 downto 1);
    uart_o       : out std_logic;
    uart_i       : in  std_logic);

end idrogen;

architecture rtl of idrogen is

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 5) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("PPS_IN     ",  IO_NONE,         false,   false,  0,     IO_INPUT,     IO_GPIO,  false,        false,       IO_TTL),
    ("TRIG_IN    ",  IO_NONE,         false,   false,  1,     IO_INPUT,     IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR_1  ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR_2  ",  IO_NONE,         false,   false,  1,     IO_OUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR_3  ",  IO_NONE,         false,   false,  2,     IO_OUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR_4  ",  IO_NONE,         false,   false,  3,     IO_OUTPUT,    IO_GPIO,  false,        false,       IO_TTL)
  );

  constant c_family       : string := "Arria 10 GX SCU4"; -- string := "Arria 10 GX IDROGEN";
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
      g_flash_bits         => 25,
      g_psram_bits         => c_psram_bits,
      g_gpio_out           => 2,
      g_gpio_in            => 2,
      g_en_tlu             => false,
      g_en_pcie            => true,
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
      core_clk_125m_pllref_i  => clk_125m_pllref_i,
      core_clk_125m_local_i   => clk_125m_pllref_i, -- TBD: Feed 62m5 in to a PLL and generate 125 MHz
      core_clk_125m_sfpref_i  => clk_125m_pllref_i,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_uart_o               => uart_o,
      wr_uart_i               => uart_i,
      wbar_phy_dis_o          => sfp_tx_disable_o,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      gpio_o                  => gpio_o);

end rtl;
