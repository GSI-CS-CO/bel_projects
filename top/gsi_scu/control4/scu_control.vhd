library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity scu_control is
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
    rom_data_io : inout std_logic;

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
    -- Misc.
    -----------------------------------------------------------------------
    fpga_res_i : in std_logic;
    nres_i     : in std_logic;

    -----------------------------------------------------------------------
    -- LVTTL IOs
    -----------------------------------------------------------------------
    lemo_p_i : in    std_logic_vector(1 downto 0);
    lemo_n_i : in    std_logic_vector(1 downto 0);
    lemo_p_o : out   std_logic_vector(1 downto 0);
    lemo_n_o : out   std_logic_vector(1 downto 0);

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds_o : out std_logic_vector(3 downto 0) := (others => '1');
    rt_leds_o : out std_logic_vector(3 downto 0) := (others => '1');
	 
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
    psram_be0          : out   std_logic := 'Z';
    psram_be1          : out   std_logic := 'Z';
    psram_wait         : in    std_logic; -- DDR magic
    
    -----------------------------------------------------------------------
     -- Fast-SRAM (2x 16Mbit)
     -----------------------------------------------------------------------
     sram_a            : out   std_logic_vector(19 downto 0) := (others => 'Z');
     sram_dq           : inout std_logic_vector(15 downto 0) := (others => 'Z');
     sram_csn          : out   std_logic_vector(1 downto 0) := (others => '1');
     sram_oen          : out   std_logic := 'Z';
     sram_wen          : out   std_logic := 'Z';
     sram_lbn          : out   std_logic := 'Z';
     sram_ubn          : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_led_fpg_o    : out   std_logic;
    sfp_led_fpr_o    : out   std_logic;
    sfp_tx_disable_o : out   std_logic := '0';
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic);

end scu_control;

architecture rtl of scu_control is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_gpio_o       : std_logic_vector(3 downto 0);
  signal s_lvds_p_i     : std_logic_vector(1 downto 0);
  signal s_lvds_n_i     : std_logic_vector(1 downto 0);
  signal s_lvds_p_o     : std_logic_vector(1 downto 0);
  signal s_lvds_n_o     : std_logic_vector(1 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

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

  constant c_family        : string := "Arria 10 GX SCU4";
  constant c_project       : string := "scu_control";
  constant c_cores         : natural:= 1;
  constant c_initf_name    : string := c_project & "_stub.mif";
  constant c_profile_name  : string := "medium_icache_debug";

begin

  main : monster
    generic map(
      g_family           => c_family,
      g_project          => c_project,
      g_flash_bits       => 25, -- !!! TODO: Check this
      g_gpio_out         => 4,
      g_lvds_inout       => 2,
      g_en_scubus        => true,
      g_en_pcie          => true,
      g_en_tlu           => false,
      g_en_usb           => false,
      g_io_table         => io_mapping_table,
      g_en_tempsens      => false,
      g_a10_use_sys_fpll => false,
      g_a10_use_ref_fpll => false,
      g_lm32_cores       => c_cores,
      g_lm32_ramsizes    => c_lm32_ramsizes/4,
      g_lm32_init_files  => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles    => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i     => clk_20m_vcxo_i,
      core_clk_125m_pllref_i  => clk_125m_tcb_pllref_i,
      core_clk_125m_local_i   => clk_125m_tcb_local_i,
      core_clk_125m_sfpref_i  => clk_125m_tcb_sfpref_i,
      wr_onewire_io           => rom_data_io,
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
      lvds_p_i                => s_lvds_p_i,
      lvds_n_i                => s_lvds_n_i,
      lvds_p_o                => s_lvds_p_o,
      lvds_n_o                => s_lvds_n_o,
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
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o);

  -- SFP
  sfp_tx_disable_o <= '0';

  -- LEDs
  wr_leds_o(0)  <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  wr_leds_o(1)  <= not s_led_link_up;                      -- blue  = link
  wr_leds_o(2)  <= not s_led_track;                        -- green = timing valid
  wr_leds_o(3)  <= not s_led_pps;                          -- white = PPS
  sfp_led_fpg_o <= not s_led_link_up;
  sfp_led_fpr_o <= not s_led_link_act;
  rt_leds_o     <= not s_gpio_o(3 downto 0);

  -- LEMOs
  lemos : for i in 0 to 1 generate
    s_lvds_p_i(i)      <= lemo_p_i(i);
    s_lvds_n_i(i)      <= lemo_n_i(i);
    lemo_p_o(i)        <= s_lvds_p_o(i);
    lemo_n_o(i)        <= s_lvds_n_o(i);
  end generate;

end rtl;
