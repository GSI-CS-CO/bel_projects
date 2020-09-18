library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.stub_pll_pkg.all;

------------------------------------------------------------------------
-- Clocking
------------------------------------------------------------------------
-- JTAG: 10AX115S2F45I1SG
-- Tool: arria10GX_10ax115sf45_fpga_v15.1.2/examples/board_test_system$ java -Xmx256m -jar bts.jar -mode clk
-- clk_125m_pllref_i is connected to U14 (Si5338A) -> Clock 3 (schematic name: REFCLK_SMA) => Set this to 125MHz
-- clk_125m_local_i  is connected to U26 (Si5338A) -> Clock 2 (schematic name: REFCLK_QSFP) => Set this to 125MHz
-- clk_125m_sfpref_i is connected to U26 (Si5338A) -> Clock 1 (schematic name: REFCLK_QSFP) => Set this to 125MHz
-- clk_20m_vcxo_i    is connected to U26 (Si5338A) -> Clock 0 (schematic name: REFCLK_DP) (here: looped)
-- clk_pll_i         is connected to U26 (Si5338A) -> Clock 3 (schematic name: CLK_EMI) (here: signal tap clock)

------------------------------------------------------------------------
-- LEDs @ 1.8V
------------------------------------------------------------------------
-- D10 => USER_LED_G0 => L28 => wr_leds_o(0)
-- D9  => USER_LED_G1 => K26 => wr_leds_o(1)
-- D8  => USER_LED_G2 => K25 => wr_leds_o(2)
-- D7  => USER_LED_G3 => L25 => wr_leds_o(3)
-- D6  => USER_LED_G4 => J24 => phy_rx_ready_o
-- D5  => USER_LED_G5 => A19 => phy_tx_ready_o
-- D4  => USER_LED_G6 => C18 => phy_debug_o
-- D3  => USER_LED_G7 => D18 => gpio

------------------------------------------------------------------------
-- DIP SWITCH @ 1.8V => phy_debug_i
------------------------------------------------------------------------
-- A24 => USER_DIPSW0
-- B23 => USER_DIPSW1
-- A23 => USER_DIPSW2
-- B22 => USER_DIPSW3
-- A22 => USER_DIPSW4
-- B21 => USER_DIPSW5
-- C21 => USER_DIPSW6
-- A20 => USER_DIPSW7

entity pci_control is
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    --clk_20m_vcxo_i        : in std_logic; -- 20MHz VCXO clock
    --clk_20m_vcxo_alt_i    : in std_logic; -- 20MHz VCXO clock alternative

    clk_125m_pllref_i     : in std_logic; -- 125 MHz PLL reference
    clk_125m_local_i      : in std_logic; -- Local clk from 125Mhz oszillator
    clk_125m_sfpref_i     : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator

    --clk_125m_pllref_alt_i : in std_logic; -- 125 MHz PLL reference alternative
    --clk_125m_local_alt_i  : in std_logic; -- Local clk from 125Mhz oszillator alternative
    --clk_125m_sfpref_alt_i : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator alternative

    --clk_125m_tcb_pllref_i : in std_logic; -- 125 MHz PLL reference at tranceiver bank
    --clk_125m_tcb_local_i  : in std_logic; -- Local clk from 125Mhz oszillator at tranceiver bank
    --clk_125m_tcb_sfpref_i : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator at tranceiver bank

    clk_pll_i             : in std_logic; -- Evaluation board

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
    rom_data_io : inout std_logic;

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    --fpga_res_i : in std_logic;
    --nres_i     : in std_logic;

    -----------------------------------------------------------------------
    -- LVTTL IOs
    -----------------------------------------------------------------------
    --lemo_p_i : in    std_logic_vector(1 downto 0);
    --lemo_n_i : in    std_logic_vector(1 downto 0);
    --lemo_p_o : out   std_logic_vector(1 downto 0);
    --lemo_n_o : out   std_logic_vector(1 downto 0);

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds_o          : out std_logic_vector(3 downto 0) := (others => '1');
    rt_leds_o          : out std_logic_vector(3 downto 0) := (others => '1');
    phy_debug_i        : in  std_logic_vector(7 downto 0);
    green_leds_n_o     : out std_logic_vector(7 downto 0) := (others => '1');
    red_leds_o         : out std_logic_vector(7 downto 0) := (others => '1');

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_rs0_o        : out   std_logic; -- RS0=0 -> RX datarates <= 4.25GB/s, RS0=1 -> RX datarates > 4.25GB/s
    sfp_rs1_o        : out   std_logic; -- RS1=0 -> TX datarates <= 4.25GB/s, RS1=1 -> TX datarates > 4.25GB/s
    --sfp_led_fpg_o    : out   std_logic;
    --sfp_led_fpr_o    : out   std_logic;
    sfp_tx_disable_o : out   std_logic := '0';
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic);

end pci_control;

architecture rtl of pci_control is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_phy_rx_ready : std_logic;
  signal s_phy_tx_ready : std_logic;
  signal s_phy_debug    : std_logic;
  signal s_sfp_mod2_io    : std_logic;
  signal s_sfp_mod1_io    : std_logic;
  signal s_sfp_mod0_i     : std_logic;
  signal s_sfp_los_i      : std_logic;
  signal s_sfp_tx_fault_i : std_logic;

  signal s_gpio_o       : std_logic_vector(3 downto 0);
  signal s_lvds_p_i     : std_logic_vector(1 downto 0);
  signal s_lvds_n_i     : std_logic_vector(1 downto 0);
  signal s_lvds_p_o     : std_logic_vector(1 downto 0);
  signal s_lvds_n_o     : std_logic_vector(1 downto 0);

  signal s_clk_20m_vcxo           : std_logic;
  signal s_clk_125m_pllref        : std_logic;
  signal s_clk_125m_local         : std_logic;
  signal s_clk_125m_sfpref        : std_logic;
  signal s_stub_pll_reset         : std_logic;
  signal s_stub_pll_locked        : std_logic;
  signal s_stub_pll_locked_prev   : std_logic;
  signal s_clk_20m_vcxo_i_stub    : std_logic;
  signal s_clk_125m_pllref_i_stub : std_logic;
  signal s_clk_125m_local_i_stub  : std_logic;
  signal s_clk_125m_sfpref_i_stub : std_logic;
  signal s_clk_20m_loop           : std_logic;

  constant c_use_stub_pll : boolean := false;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 5) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1_BASE_R",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_BASE_B",  IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_BASE_G",  IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_BASE_W",  IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LVDS_DUMMY1",  IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDS_DUMMY2",  IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  false,        false,       IO_LVDS)
  );

  constant c_family        : string := "Arria 10 GX E3P1";
  constant c_project       : string := "pci_control";
  constant c_cores         : natural:= 1;
  constant c_initf_name    : string := c_project & "_stub.mif";
  constant c_profile_name  : string := "medium_icache_debug";

begin

  main : monster
    generic map(
      g_family            => c_family,
      g_project           => c_project,
      g_flash_bits        => 25, -- !!! TODO: Check this
      g_gpio_out          => 4,
      g_lvds_inout        => 2,
      g_en_pcie           => true,
      g_en_tlu            => false,
      g_en_usb            => false,
      g_io_table          => io_mapping_table,
      g_en_tempsens       => false,
      g_a10_use_sys_fpll  => false,
      g_a10_use_ref_fpll  => false,
      g_a10_en_phy_reconf => true,
      g_lm32_cores        => c_cores,
      g_lm32_ramsizes     => c_lm32_ramsizes/4,
      g_lm32_init_files   => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles     => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i     => s_clk_20m_vcxo,
      core_clk_125m_pllref_i  => s_clk_125m_pllref,
      core_clk_125m_local_i   => s_clk_125m_local,
      core_clk_125m_sfpref_i  => s_clk_125m_sfpref,
      core_clk_20m_o          => s_clk_20m_loop,
      core_clk_debug_i        => clk_pll_i,
      wr_onewire_io           => rom_data_io,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      sfp_tx_disable_o        => sfp_tx_disable_o,
      sfp_tx_fault_i          => s_sfp_tx_fault_i,
      sfp_los_i               => s_sfp_los_i,
      phy_rx_ready_o          => s_phy_rx_ready,
      phy_tx_ready_o          => s_phy_tx_ready,
      phy_debug_o             => s_phy_debug,
      phy_debug_i             => phy_debug_i,
      gpio_o                  => s_gpio_o,
      --lvds_p_i                => s_lvds_p_i,
      --lvds_n_i                => s_lvds_n_i,
      --lvds_p_o                => s_lvds_p_o,
      --lvds_n_o                => s_lvds_n_o,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o);

  -- SFP
  --sfp_tx_disable_o <= '0';
  sfp_rs0_o        <= '1';
  sfp_rs1_o        <= '1';
  s_sfp_los_i      <= sfp_los_i;
  s_sfp_tx_fault_i <= sfp_tx_fault_i;

  red_leds_o(0) <= '1';
  red_leds_o(1) <= '1';
  red_leds_o(2) <= '1';
  red_leds_o(3) <= '1';
  red_leds_o(4) <= '1';
  red_leds_o(5) <= '1';
  red_leds_o(6) <= '1';
  red_leds_o(7) <= '1';

  -- LEDs
  wr_leds_o(0)      <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  wr_leds_o(1)      <= not s_led_link_up;                      -- blue  = link
  wr_leds_o(2)      <= not s_led_track;                        -- green = timing valid
  wr_leds_o(3)      <= not s_led_pps;                          -- white = PPS
  green_leds_n_o(0) <= not (s_led_link_act and s_led_link_up);
  green_leds_n_o(1) <= not s_led_link_up;
  green_leds_n_o(2) <= not s_led_track;
  green_leds_n_o(3) <= not s_led_pps;
  rt_leds_o         <= not s_gpio_o(3 downto 0);
  green_leds_n_o(4) <= not s_phy_rx_ready;
  green_leds_n_o(5) <= not s_phy_tx_ready;
  green_leds_n_o(6) <= not s_phy_debug;
  green_leds_n_o(7) <= s_sfp_los_i;

  -- LEMOs
  --lemos : for i in 0 to 1 generate
  --  s_lvds_p_i(i)      <= lemo_p_i(i);
  --  s_lvds_n_i(i)      <= lemo_n_i(i);
  --  lemo_p_o(i)        <= s_lvds_p_o(i);
  --  lemo_n_o(i)        <= s_lvds_n_o(i);
  --end generate;

  real_pll_a10 : if not(c_use_stub_pll) generate
    s_clk_20m_vcxo    <= s_clk_20m_loop;
    s_clk_125m_pllref <= clk_125m_pllref_i;
    s_clk_125m_local  <= clk_125m_local_i;
    s_clk_125m_sfpref <= clk_125m_sfpref_i;
  end generate;

  stub_pll_a10 : if c_use_stub_pll generate
    -- Connect to monster
    s_clk_20m_vcxo    <= s_clk_20m_vcxo_i_stub;
    s_clk_125m_pllref <= s_clk_125m_pllref_i_stub;
    s_clk_125m_local  <= s_clk_125m_local_i_stub;
    s_clk_125m_sfpref <= s_clk_125m_sfpref_i_stub;

    -- Stub PLL
    stub_inst : stub_pll_altera_iopll_160_z2kwsvq port map(
      rst        => s_stub_pll_reset,
      refclk     => clk_pll_i,
      locked     => s_stub_pll_locked,
      outclk_0   => s_clk_20m_vcxo_i_stub,
      outclk_1   => s_clk_125m_pllref_i_stub,
      outclk_2   => s_clk_125m_local_i_stub,
      outclk_3   => s_clk_125m_sfpref_i_stub);

    -- PLL reset, don't do this at home
    p_stub_pll_reset : process(clk_pll_i)
    begin
      if (rising_edge(clk_pll_i)) then
        s_stub_pll_locked_prev <= s_stub_pll_locked;
        if (s_stub_pll_reset = '1') then
          s_stub_pll_reset <= '0';
        elsif (s_stub_pll_locked = '0' and s_stub_pll_locked_prev = '1') then
          s_stub_pll_reset <= '1';
        else
          s_stub_pll_reset <= '0';
        end if;
      end if;
    end process;
  end generate;

end rtl;
