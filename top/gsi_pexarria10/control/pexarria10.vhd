library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity pexarria10 is
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
    -- SPI Flash User Mode
    -----------------------------------------------------------------------
    UM_AS_D           : inout std_logic_vector(3 downto 0) := (others => 'Z');
    UM_nCSO           : out   std_logic := 'Z';
    UM_DCLK           : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB : inout std_logic;
    OneWire_CB_splz : out std_logic;  --Strong Pull-Up for Onewire

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    nuser_pb_i   : in std_logic;  --User Button
    -- Reset MAX10
    nres_out_o   : out std_logic;

    -----------------------------------------------------------------------
    -- USBC LVTTL IOs, no USB functionality these are physical interface only
    -----------------------------------------------------------------------
    
    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds_o                  : out std_logic_vector(1 downto 0) := (others => '1');
    --wr_aux_leds_or_node_leds_o : out std_logic_vector(3 downto 0) := (others => '1');
    --rt_leds_o                  : out std_logic_vector(3 downto 0) := (others => '1');

   -----------------------------------------------------------------------
    -- Pseudo-SRAM (4x 256Mbit)
    -----------------------------------------------------------------------
    psram_a            : out   std_logic_vector(23 downto 0) := (others => 'Z');
    psram_dq           : inout std_logic_vector(15 downto 0) := (others => 'Z');
    psram_clk          : out   std_logic := 'Z';
    psram_advn         : out   std_logic := 'Z';
    psram_cre          : out   std_logic := 'Z';
    psram_cen          : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_oen          : out   std_logic := 'Z';
    psram_wen          : out   std_logic := 'Z';
    psram_ubn          : out   std_logic := 'Z';
    psram_lbn          : out   std_logic := 'Z';
    psram_wait         : in    std_logic; -- DDR magic

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
    --usb_uclkin_i : in    std_logic;

    -----------------------------------------------------------------------
    -- CPLD (F2F)
    -----------------------------------------------------------------------
    cpld_io : inout std_logic_vector(7 downto 0);

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic := '0';
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic;
    sfp_led_fpg_o    : out   std_logic:='0';
    sfp_led_fpr_o    : out   std_logic:='0';

    -----------------------------------------------------------------------
    -- SFP (auxiliary - not used here)
    -----------------------------------------------------------------------
    -- sfp_aux_tx_disable_o_nc : out   std_logic := '0';
    -- sfp_aux_tx_fault_i_nc   : in    std_logic;
    -- sfp_aux_los_i_nc        : in    std_logic;
    -- sfp_aux_txp_o_nc        : out   std_logic;
    -- sfp_aux_rxp_i_nc        : in    std_logic;
    -- sfp_aux_mod0_i_nc       : in    std_logic;
    -- sfp_aux_mod1_io_nc      : inout std_logic;
    -- sfp_aux_mod2_io_nc      : inout std_logic
    -----------------------------------------------------------------------
    -- USBC no USB functionality only LVDS signals
    -----------------------------------------------------------------------
    usbc_i2c_scl_io : inout std_logic_vector(5 downto 1);
    usbc_i2c_sda_io : inout std_logic_vector(5 downto 1);
    usbc_tx1_en     : out std_logic_vector(5 downto 1);
    usbc_tx2_en     : out std_logic_vector(5 downto 1);
    usbc_tx3_en     : out std_logic_vector(5 downto 1);
    usbc_tx4_en     : out std_logic_vector(5 downto 1);
    usbc_tx5_en     : out std_logic_vector(5 downto 1);
    --usbc_tx1_n     : out std_logic_vector(5 downto 1);
    usbc_tx1_p     : out std_logic_vector(5 downto 1);
    --usbc_tx2_n     : out std_logic_vector(5 downto 1);
    usbc_tx2_p     : out std_logic_vector(5 downto 1);
    --usbc_tx3_n     : out std_logic_vector(5 downto 1);
    usbc_tx3_p     : out std_logic_vector(5 downto 1);
    --usbc_tx4_n     : out std_logic_vector(5 downto 1);
    usbc_tx4_p     : out std_logic_vector(5 downto 1);
    --usbc_tx5_n     : out std_logic_vector(5 downto 1);
    usbc_tx5_p     : out std_logic_vector(5 downto 1);   
    usbc_rx1_n     : in std_logic_vector(5 downto 1);
    usbc_rx1_p     : in std_logic_vector(5 downto 1);
    usbc_rx2_n     : in std_logic_vector(5 downto 1);
    usbc_rx2_p     : in std_logic_vector(5 downto 1);
    usbc_rx3_n     : in std_logic_vector(5 downto 1);
    usbc_rx3_p     : in std_logic_vector(5 downto 1);
    usbc_rx4_n     : in std_logic_vector(5 downto 1);
    usbc_rx4_p     : in std_logic_vector(5 downto 1);
    usbc_rx5_n     : in std_logic_vector(5 downto 1); 
    usbc_rx5_p     : in std_logic_vector(5 downto 1)
    );

end pexarria10;

architecture rtl of pexarria10 is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_gpio_o       : std_logic_vector(42 downto 0);
  signal s_gpio_i       : std_logic_vector(17 downto 0);
  signal s_lvds_p_i     : std_logic_vector(24 downto 0);
  signal s_lvds_n_i     : std_logic_vector(24 downto 0);
  signal s_lvds_p_o     : std_logic_vector(24 downto 0);
  signal s_lvds_n_o     : std_logic_vector(24 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 92) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("CPLD_IO_0  ",   IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_1  ",   IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_2  ",   IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_3  ",   IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_4  ",   IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_5  ",   IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_6  ",   IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("CPLD_IO_7  ",   IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_GPIO,  false,        false,       IO_TTL),
    ("USBC_SCL1  ",  IO_NONE,         false,   false,  8,     IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SCL2  ",  IO_NONE,         false,   false,  9,     IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SCL3  ",  IO_NONE,         false,   false,  10,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SCL4  ",  IO_NONE,         false,   false,  11,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SCL5  ",  IO_NONE,         false,   false,  12,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SDA1  ",  IO_NONE,         false,   false,  13,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SDA2  ",  IO_NONE,         false,   false,  14,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SDA3  ",  IO_NONE,         false,   false,  15,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SDA4  ",  IO_NONE,         false,   false,  16,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC_SDA5  ",  IO_NONE,         false,   false,  17,    IO_INOUTPUT,   IO_GPIO,  false,        false,     IO_TTL),
    ("USBC1_TXEN1",   IO_NONE,         false,   false,  18,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC1_TXEN2",   IO_NONE,         false,   false,  19,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC1_TXEN3",   IO_NONE,         false,   false,  20,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC1_TXEN4",   IO_NONE,         false,   false,  21,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC1_TXEN5",   IO_NONE,         false,   false,  22,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC2_TXEN1",   IO_NONE,         false,   false,  23,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC2_TXEN2",   IO_NONE,         false,   false,  24,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC2_TXEN3",   IO_NONE,         false,   false,  25,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC2_TXEN4",   IO_NONE,         false,   false,  26,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC2_TXEN5",   IO_NONE,         false,   false,  27,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC3_TXEN1",   IO_NONE,         false,   false,  28,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC3_TXEN2",   IO_NONE,         false,   false,  29,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC3_TXEN3",   IO_NONE,         false,   false,  30,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC3_TXEN4",   IO_NONE,         false,   false,  31,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC3_TXEN5",   IO_NONE,         false,   false,  32,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC4_TXEN1",   IO_NONE,         false,   false,  33,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC4_TXEN2",   IO_NONE,         false,   false,  34,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC4_TXEN3",   IO_NONE,         false,   false,  35,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC4_TXEN4",   IO_NONE,         false,   false,  36,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC4_TXEN5",   IO_NONE,         false,   false,  37,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC5_TXEN1",   IO_NONE,         false,   false,  38,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC5_TXEN2",   IO_NONE,         false,   false,  39,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC5_TXEN3",   IO_NONE,         false,   false,  40,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC5_TXEN4",   IO_NONE,         false,   false,  41,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC5_TXEN5",   IO_NONE,         false,   false,  42,    IO_OUTPUT,   IO_GPIO,  false,        false,      IO_TTL),
    ("USBC1_O1   ",     IO_NONE,         false,   false,  0,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_O2   ",     IO_NONE,         false,   false,  1,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_O3   ",     IO_NONE,         false,   false,  2,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_O4   ",     IO_NONE,         false,   false,  3,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_O5   ",     IO_NONE,         false,   false,  4,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_O1   ",     IO_NONE,         false,   false,  5,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_O2   ",     IO_NONE,         false,   false,  6,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_O3   ",     IO_NONE,         false,   false,  7,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_O4   ",     IO_NONE,         false,   false,  8,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_O5   ",     IO_NONE,         false,   false,  9,     IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_O1   ",     IO_NONE,         false,   false,  10,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_O2   ",     IO_NONE,         false,   false,  11,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_O3   ",     IO_NONE,         false,   false,  12,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_O4   ",     IO_NONE,         false,   false,  13,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_O5   ",     IO_NONE,         false,   false,  14,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_O1   ",     IO_NONE,         false,   false,  15,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_O2   ",     IO_NONE,         false,   false,  16,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_O3   ",     IO_NONE,         false,   false,  17,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_O4   ",     IO_NONE,         false,   false,  18,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_O5   ",     IO_NONE,         false,   false,  19,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_O1   ",     IO_NONE,         false,   false,  20,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_O2   ",     IO_NONE,         false,   false,  21,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_O3   ",     IO_NONE,         false,   false,  22,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_O4   ",     IO_NONE,         false,   false,  23,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_O5   ",     IO_NONE,         false,   false,  24,    IO_OUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_O1   ",     IO_NONE,         false,   false,  0,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_I2   ",     IO_NONE,         false,   false,  1,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_I3   ",     IO_NONE,         false,   false,  2,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_I4   ",     IO_NONE,         false,   false,  3,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC1_I5   ",     IO_NONE,         false,   false,  4,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_I1   ",     IO_NONE,         false,   false,  5,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_I2   ",     IO_NONE,         false,   false,  6,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_I3   ",     IO_NONE,         false,   false,  7,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_I4   ",     IO_NONE,         false,   false,  8,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC2_I5   ",     IO_NONE,         false,   false,  9,     IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_I1   ",     IO_NONE,         false,   false,  10,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_I2   ",     IO_NONE,         false,   false,  11,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_I3   ",     IO_NONE,         false,   false,  12,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_I4   ",     IO_NONE,         false,   false,  13,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC3_I5   ",     IO_NONE,         false,   false,  14,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_I1   ",     IO_NONE,         false,   false,  15,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_I2   ",     IO_NONE,         false,   false,  16,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_I3   ",     IO_NONE,         false,   false,  17,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_I4   ",     IO_NONE,         false,   false,  18,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC4_I5   ",     IO_NONE,         false,   false,  19,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_I1   ",     IO_NONE,         false,   false,  20,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_I2   ",     IO_NONE,         false,   false,  21,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_I3   ",     IO_NONE,         false,   false,  22,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_I4   ",     IO_NONE,         false,   false,  23,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("USBC5_I5   ",     IO_NONE,         false,   false,  24,    IO_INPUT, IO_LVDS,  false,        false,       IO_LVDS)
  );
 constant c_family        : string := "Arria 10 GX PEX10";
  constant c_project       : string := "pexarria10";
  constant c_cores         : natural:= 1;
  constant c_initf_name    : string := c_project & "_stub.mif";
  constant c_profile_name  : string := "medium_icache_debug";
  constant c_psram_bits    : natural := 24;

begin

  main : monster
    generic map(
      g_family            => c_family,
      g_project           => c_project,
      g_flash_bits        => 25, -- !!! TODO: Check this
      g_psram_bits        => c_psram_bits,
      g_gpio_out          => 25,
      g_gpio_inout        => 18,
      g_lvds_inout        => 25,
      g_en_pcie           => true,
      g_en_tlu            => false,
      g_en_usb            => true,
      g_en_psram         => true,
      g_io_table          => io_mapping_table,
      g_a10_use_sys_fpll  => false,
      g_a10_use_ref_fpll  => false,
      g_lm32_cores        => c_cores,
      g_lm32_ramsizes     => c_lm32_ramsizes/4,
      g_lm32_init_files   => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles     => f_string_list_repeat(c_profile_name, c_cores)
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
      sfp_tx_disable_o        => open,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
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
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o,
      --PSRAM TODO: Multi Chip
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
      ps_wait                => psram_wait);

  -- SFP
  sfp_tx_disable_o        <= '0';
 
  -- LEDs
  wr_leds_o(0)  <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  wr_leds_o(1)  <= not s_led_link_up;                      -- blue  = link
  sfp_led_fpg_o <= not s_led_track;                        -- green = timing valid
  sfp_led_fpr_o <= not s_led_pps;                          -- white = PPS
 -- wr_leds_o(2)  <= not s_led_track;                        -- green = timing valid
 -- wr_leds_o(3)  <= not s_led_pps;                          -- white = PPS
  
----------------------------------------------------------
-- GPIO mapping
----------------------------------------------------------
  -- CPLD
  s_gpio_i(7 downto 0) <= cpld_io(7 downto 0);
  cpld_con : for i in 0 to 7 generate
    cpld_io(i) <= s_gpio_o(i) when s_gpio_o(i)='0' else 'Z';
  end generate;

  -- USBC I2C
  s_gpio_i(12 downto 8) <= usbc_i2c_scl_io(5 downto 1);
  i2c1 : for i in 1 to 5 generate
    usbc_i2c_scl_io(i) <= s_gpio_o(7+i) when s_gpio_o(7+i)='0' else 'Z';
  end generate; 

  s_gpio_i(17 downto 13) <= usbc_i2c_sda_io(5 downto 1);
  i2c2 : for i in 1 to 5 generate
    usbc_i2c_sda_io(i) <= s_gpio_o(12+i) when s_gpio_o(12+i)='0' else 'Z';
  end generate; 

  -- USBC TX Enable
  usbc_en : for i in 1 to 5 generate  
    usbc_tx1_en(i) <= s_gpio_o(17+i);
    usbc_tx2_en(i) <= s_gpio_o(22+i);
    usbc_tx3_en(i) <= s_gpio_o(27+i);
    usbc_tx4_en(i) <= s_gpio_o(32+i);
    usbc_tx5_en(i) <= s_gpio_o(37+i);
  end generate;
-------------------------------------------------
-- LVDS USBC mapping
-------------------------------------------------
  -- USBC TX LVDS output
  usbc_tx : for i in 0 to 4 generate  
    --usbc_tx1_n(i+1) <= s_lvds_n_o(i);    
    usbc_tx1_p(i+1) <= s_lvds_p_o(i);
    --usbc_tx2_n(i+1) <= s_lvds_n_o(i+5);    
    usbc_tx2_p(i+1) <= s_lvds_p_o(i+5);
    --usbc_tx3_n(i+1) <= s_lvds_n_o(i+10);    
    usbc_tx3_p(i+1) <= s_lvds_p_o(i+10);
    --usbc_tx4_n(i+1) <= s_lvds_n_o(i+15);    
    usbc_tx4_p(i+1) <= s_lvds_p_o(i+15);
    --usbc_tx5_n(i+1) <= s_lvds_n_o(i+20);    
    usbc_tx5_p(i+1) <= s_lvds_p_o(i+20);
  end generate; 

  -- USBC RX LVDS input
  usbc_rx : for i in 0 to 4 generate  
    s_lvds_n_i(i) <= usbc_rx1_n(i+1);
    s_lvds_p_i(i) <= usbc_rx1_p(i+1);
    s_lvds_n_i(i+5) <= usbc_rx2_n(i+1);
    s_lvds_p_i(i+5) <= usbc_rx2_p(i+1);
    s_lvds_n_i(i+10) <= usbc_rx3_n(i+1);
    s_lvds_p_i(i+10) <= usbc_rx3_p(i+1);
    s_lvds_n_i(i+15) <= usbc_rx4_n(i+1);
    s_lvds_p_i(i+15) <= usbc_rx4_p(i+1);
    s_lvds_n_i(i+20) <= usbc_rx5_n(i+1);
    s_lvds_p_i(i+20) <= usbc_rx5_p(i+1);
  end generate;
 
  ------------------
  OneWire_CB_splz   <= '1';  --Strong Pull-Up disabled

end rtl;
