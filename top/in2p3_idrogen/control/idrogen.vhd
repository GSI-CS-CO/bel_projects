library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

-- Important:
-- You need to apply one change inside the dmtd_pll10.qsys file
-- REMOVE: <parameter name="gui_reference_clock_frequency" value="20.0" />
-- ADD:    <parameter name="gui_reference_clock_frequency" value="125.0" />

entity idrogen is
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    clk_20m_vcxo_i    : in std_logic; -- 125 MHz VCXO clock, aka DMTD, aka WR_CLK_DMTD !!!
    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference at tranceiver bank, WR_REFCLK_125

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o : out std_logic; -- aka WR_DAC_SCLK
    wr_dac_din_o  : out std_logic; -- aka WR_DAC_DIN
    wr_ndac_cs_o  : out std_logic_vector(2 downto 1); -- aka WR_DAC2_SYNCn and WR_DAC1_SYNCn

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic; -- aka WR_SFP_TxDisable
    sfp_tx_fault_i   : in    std_logic; -- aka WR_SFP_TXFAULT
    sfp_los_i        : in    std_logic; -- aka WR_SFP_LOS
    sfp_txp_o        : out   std_logic; -- aka WR_SFP_TX
    sfp_rxp_i        : in    std_logic; -- aka WR_SFP_RX
    sfp_mod0_i       : in    std_logic; -- aka WR_SFP_DET_i
    sfp_mod1_io      : inout std_logic; -- aka WR_SFP_scl_b
    sfp_mod2_io      : inout std_logic; -- aka WR_SFP_sda_b
    sfp_rate_o       : out   std_logic; -- aka WR_SFP_RATE_SELECT

    -----------------------------------------------------------------------
    -- Idrogen special
    -----------------------------------------------------------------------
    dev_clr_n      : in    std_logic; -- FPGA Reset input, active low, aka DEV_CLRn
    lmk_clkref_2   : in    std_logic; -- reference clocks from LMK, aka LMK_CLKREF_2
    lmk_clkref_12  : in    std_logic; -- reference clocks from LMK, aka LMK_CLKREF_12
    pps_in         : in    std_logic; -- aka PPS_IN
    pps_out        : out   std_logic; -- aka PPS_OUT
    trigger_in     : in    std_logic; -- aka TRIGGER_IN
    trigger_out    : out   std_logic; -- aka TRIGGER_OUT
    wr_scl_flash_b : inout std_logic; -- aka WR_SCL_FLASH_b
    wr_sda_flash_b : inout std_logic; -- aka WR_SDA_FLASH_b

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    wr_one_wire_io : inout std_logic; -- aka WR_SERNUM_b
    led_n          : out   std_logic_vector(3 downto 0); -- aka LEDn
    uart_o         : out   std_logic; -- aka WR_RX_to_UART
    uart_i         : in    std_logic); -- aka WR_TX_from_UART

end idrogen;

architecture rtl of idrogen is

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 3) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("PPS_IN     ",  IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("TRIG_IN    ",  IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("TRIG_OUT   ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("USR_LED    ",  IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL)
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
      g_en_pcie            => false,
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
      core_rstn_i             => dev_clr_n,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_onewire_io           => wr_one_wire_io,
      wr_uart_o               => uart_o,
      wr_uart_i               => uart_i,
      wbar_phy_dis_o          => sfp_tx_disable_o,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      wr_pps_o                => pps_out,
      --led_pps_o               => pps_out,
      led_link_up_o           => led_n(0),
      led_link_act_o          => led_n(1),
      led_track_o             => led_n(2),
      gpio_i(0)               => pps_in,
      gpio_i(1)               => trigger_in,
      gpio_o(0)               => trigger_out,
      gpio_o(1)               => led_n(3)
    );

  sfp_rate_o <= '0';

  wr_scl_flash_b <= 'Z';
  wr_sda_flash_b <= 'Z';

end rtl;
