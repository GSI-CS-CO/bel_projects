library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;
use work.ez_usb_pkg.all;
use work.mbox_pkg.all;
use work.eca_pkg.all;
use work.build_id_pkg.all;
use work.watchdog_pkg.all;
use work.monster_pkg.all;
--use work.eca_auto_pkg.all;
--use work.eca_tlu_auto_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;



-- use with socat pseudo terminals:
--   socat -d -d pty,raw,echo=0 pty,raw,echo=0  # creates /dev/pts/40 and /dev/pts/39
--   socat -u -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/42
--   socat -U -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/44
-- then start simulation and call:
--   eb-read -p dev/pts/39 0x01000000/4
entity testbench is
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';
  signal rstn_sys         : std_logic := '0';
  --constant clk_50_period  : time      := 20 ns;
  constant clk_125_local_period : time      :=  8 ns;
  --constant clk_sys_period : time      := 16 ns;
  --signal clk_50           : std_logic := '1';
  signal clk_125_local          : std_logic := '1';
  --signal clk_sys          : std_logic := '1';

  signal sfp234_ref_clk_i  : std_logic := '0';

  signal clk_sys           : std_logic;
  signal clk_ref           : std_logic;
  signal clk_dmtd          : std_logic;

  signal dac_hpll_load_p1  : std_logic;
  signal dac_hpll_data     : std_logic_vector(15 downto 0);
  signal dac_dpll_load_p1  : std_logic;
  signal dac_dpll_data     : std_logic_vector(15 downto 0);

  -- connections to EZUSB chip
  signal usb_rstn     : std_logic := '0';
  signal usb_ebcyc    : std_logic := '0';
  signal usb_readyn   : std_logic := '0';
  signal usb_fifoadr  : std_logic_vector(1 downto 0) := (others => '0');
  signal usb_fulln    : std_logic := '0';
  signal usb_sloen    : std_logic := '0';
  signal usb_emptyn   : std_logic := '0';
  signal usb_slrdn    : std_logic := '0';
  signal usb_slwrn    : std_logic := '0';
  signal usb_pktendn  : std_logic := '0';
  signal usb_fd_io    : std_logic_vector(7 downto 0) := (others => 'Z');
  signal s_usb_fd     : std_logic_vector(7 downto 0) := (others => '0');
  signal s_usb_fd_oen : std_logic := '0';

  constant c_family       : string := "Arria V";
  constant c_project      : string := "pci_control";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & "_stub.mif";
  constant c_profile_name : string := "medium_icache_debug";

  -- io
  signal gpio_o        : std_logic_vector(7 downto 0);
  signal lvds_p_i      : std_logic_vector(4 downto 0);
  signal lvds_n_i      : std_logic_vector(4 downto 0);
  signal lvds_i_led    : std_logic_vector(4 downto 0);
  signal lvds_p_o      : std_logic_vector(2 downto 0);
  signal lvds_n_o      : std_logic_vector(2 downto 0);
  signal lvds_o_led    : std_logic_vector(2 downto 0);
  signal lvds_oen      : std_logic_vector(2 downto 0);
  signal lvds_term     : std_logic_vector(2 downto 0);

  signal butis_clk_200 : std_logic;
  signal butis_t0_ts   : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 14) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1_BASE_R", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_BASE_B", IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_BASE_G", IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_BASE_W", IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED1_ADD_R ", IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_ADD_B ", IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_ADD_G ", IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_ADD_W ", IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("IO1        ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO2        ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO3        ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("MHDMR_SYIN ", IO_NONE,         false,   false,  3,     IO_INPUT,    IO_LVDS,  false,        false,       IO_LVDS),
    ("MHDMR_TRIN ", IO_NONE,         false,   false,  4,     IO_INPUT,    IO_LVDS,  false,        false,       IO_LVDS),
    ("MHDMR_CK200", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_FIXED, false,        false,       IO_LVDS),
    ("MHDMR_SYOU ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_FIXED, false,        false,       IO_LVDS)
  );


  signal sfp4_mod2     : std_logic := '0';
  signal sfp4_mod1     : std_logic := '0';
  signal sfp4_mod0     : std_logic := '0';
  signal sfp4_txp_o    : std_logic := '0';
  signal sfp4_rxp_i    : std_logic := '0';
  signal dac_sclk      : std_logic := '0';
  signal dac_din       : std_logic := '0';
  signal ndac_cs       : std_logic_vector(2 downto 1) := (others => '0');
  signal sfp4_tx_fault : std_logic := '0';
  signal sfp4_los      : std_logic := '0';
begin


  ------ generate clock and reset signal -------
  --clk_50  <= not clk_50  after clk_50_period/2;
  clk_125_local <= not clk_125_local after clk_125_local_period/2;
  --clk_sys <= not clk_sys after clk_sys_period/2;
  rst     <= '0'         after clk_125_local_period*10;
  rst_n   <= not rst;
  --rstn_sys<= not rst;
  ----------------------------------------------

  ---- instance of EZUSB-chip 
  -- this simulates the physical chip that is connected to the FPGA
  chip : entity work.ez_usb_chip
    port map (
      rstn_i    => usb_rstn,
      ebcyc_o   => usb_ebcyc,
      readyn_o  => usb_readyn,
      fifoadr_i => usb_fifoadr,
      fulln_o   => usb_fulln,
      emptyn_o  => usb_emptyn,
      sloen_i   => usb_sloen,
      slrdn_i   => usb_slrdn,
      slwrn_i   => usb_slwrn,
      pktendn_i => usb_pktendn,
      fd_io     => usb_fd_io
      );


  wrex : entity work.wr_timing
  port map(
    dac_hpll_load_p1_i => dac_hpll_load_p1,
    dac_hpll_data_i    => dac_hpll_data,
    dac_dpll_load_p1_i => dac_dpll_load_p1,
    dac_dpll_data_i    => dac_dpll_data,
    clk_ref_125_o      => clk_ref,
    clk_sys_62_5_o     => open,
    clk_dmtd_20_o      => clk_dmtd
  );



  main : monster
    generic map(
    --g_psram_bits        => false,
    --g_ram_size          => false,
    --g_tlu_fifo_size     => false,
    g_en_tlu            => false,
    g_en_vme            => false,
    g_en_scubus         => false,
    g_en_mil            => false,
    g_en_oled           => false,
    g_en_cfi            => false,
    g_en_ddr3           => false,
    g_en_ssd1325        => false,
    g_en_nau8811        => false,
    g_en_psram          => false,
    g_en_beam_dump      => false,
    g_en_pmc            => false,
    --g_a10_use_sys_fpll  => false,
    --g_a10_use_ref_fpll  => false,
    g_a10_en_phy_reconf => false,
    g_en_butis          => false,
    --g_lm32_MSIs         => false,
    --g_lm32_are_ftm      => false,
    g_delay_diagnostics => false,
    g_en_eca            => false,
    g_en_wd_tmr         => false,
    g_en_eca_tap        => false,

      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_gpio_out        => 8,
      g_lvds_in         => 2,
      g_lvds_out        => 0,
      g_lvds_inout      => 3,
      g_fixed           => 2,
      g_lvds_invert     => true,
      g_en_pcie         => false,
      g_en_usb          => true,
      g_en_lcd          => false,
      g_en_user_ow      => false,
      g_en_tempsens     => false
      --g_io_table        => io_mapping_table,
      --g_lm32_cores      => c_cores,
      --g_lm32_ramsizes   => c_lm32_ramsizes/4,
      --g_lm32_init_files => f_string_list_repeat(c_initf_name, c_cores),
      --g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i     => clk_dmtd,
      core_clk_125m_pllref_i  => clk_ref,
      core_clk_125m_sfpref_i  => sfp234_ref_clk_i,
      core_clk_125m_local_i   => clk_125_local,
      core_rstn_i             => rst_n,
      core_clk_butis_o        => open,
      core_clk_butis_t0_o     => open,
      wr_onewire_io           => open,
      wr_sfp_sda_io           => sfp4_mod2,
      wr_sfp_scl_io           => sfp4_mod1,
      wr_sfp_det_i            => sfp4_mod0,
      wr_sfp_tx_o             => sfp4_txp_o,
      wr_sfp_rx_i             => sfp4_rxp_i,
      wr_dac_sclk_o           => dac_sclk,
      wr_dac_din_o            => dac_din,
      wr_ndac_cs_o            => ndac_cs,
      sfp_tx_disable_o        => open,
      sfp_tx_fault_i          => sfp4_tx_fault,
      sfp_los_i               => sfp4_los,
      --gpio_o                  => open,
      --lvds_p_i                => lvds_p_i,
      --lvds_n_i                => lvds_n_i,
      --lvds_i_led_o            => lvds_i_led,
      --lvds_p_o                => lvds_p_o,
      --lvds_n_o                => lvds_n_o,
      --lvds_o_led_o            => lvds_o_led,
      --lvds_oen_o              => lvds_oen,
      --lvds_term_o             => lvds_term,
      led_link_up_o           => open,
      led_link_act_o          => open,
      led_track_o             => open,
      led_pps_o               => open,
      --pcie_refclk_i           => '0',
      --pcie_rstn_i             => '0',
      --pcie_rx_i               => (others => '0'),
      --pcie_tx_o               => open,
      usb_rstn_o              => usb_rstn,
      usb_ebcyc_i             => usb_ebcyc,
      usb_speed_i             => '0',
      usb_shift_i             => '0',
      usb_readyn_io           => usb_readyn,
      usb_fifoadr_o           => usb_fifoadr,
      usb_sloen_o             => usb_sloen,
      usb_fulln_i             => usb_fulln,
      usb_emptyn_i            => usb_emptyn,
      usb_slrdn_o             => usb_slrdn,
      usb_slwrn_o             => usb_slwrn,
      usb_pktendn_o           => usb_pktendn,
      usb_fd_io               => usb_fd_io,
      ow_io                   => open,
      lcd_scp_o               => open,
      lcd_lp_o                => open,
      lcd_flm_o               => open,
      lcd_in_o                => open);

  -- SFP1-3 are not mounted
  --sfp1_tx_disable_o <= '1';
  --sfp2_tx_disable_o <= '1';
  --sfp3_tx_disable_o <= '1';
  --sfp4_tx_disable_o <= '0';

  ---- Link LEDs
  --wrdis <= '0';
  --dres  <= '1';
  --di(5) <= '0' when (not led_link_up)                   = '1' else 'Z'; -- red
  --di(6) <= '0' when (    led_link_up and not led_track) = '1' else 'Z'; -- blue
  --di(4) <= '0' when (    led_link_up and     led_track) = '1' else 'Z'; -- green

  --led(1) <= not (led_link_act and led_link_up); -- red   = traffic/no-link
  --led(2) <= not led_link_up;                    -- blue  = link
  --led(3) <= not led_track;                      -- green = timing valid
  --led(4) <= not led_pps;                        -- white = PPS

  --ledsfpg(3 downto 1) <= (others => '1');
  --ledsfpr(3 downto 1) <= (others => '1');
  --ledsfpg(4) <= not led_link_up;
  --ledsfpr(4) <= not led_link_act;

  ---- GPIO LEDs
  --led(5) <= '0' when gpio_o(0)='1' else 'Z'; -- (baseboard), red
  --led(6) <= '0' when gpio_o(1)='1' else 'Z'; -- blue
  --led(7) <= '0' when gpio_o(2)='1' else 'Z'; -- green
  --led(8) <= '0' when gpio_o(3)='1' else 'Z'; -- white
  --p7     <= '0' when gpio_o(4)='1' else 'Z'; -- (add-on board), red
  --n7     <= '0' when gpio_o(5)='1' else 'Z'; -- blue
  --p8     <= '0' when gpio_o(6)='1' else 'Z'; -- green
  --n8     <= '0' when gpio_o(7)='1' else 'Z'; -- white

  ---- BuTiS/MDMHR Output
  --p19 <= butis_clk_200;
  --p24 <= not(butis_t0_ts);

  ---- BuTiS/MHDMR activity LEDs
  --p6  <= '0' when butis_clk_200='1' else 'Z'; -- LED3 (near HDMI = CK200 / LVDS3)
  --n6  <= '0' when butis_t0_ts='1'   else 'Z'; -- LED4 (near HDMI = SYOU  / LVDS4)

  ---- LVDS->LEMO output enable / termination
  --n10 <= '0' when lvds_oen(0)='1' else 'Z'; -- TTLIO1 output enable
  --n11 <= '0' when lvds_oen(1)='1' else 'Z'; -- TTLIO2 output enable
  --n14 <= '0' when lvds_oen(2)='1' else 'Z'; -- TTLIO3 output enable

  --p9  <= '1' when lvds_term(0)='1' else '0'; -- TERMEN1 (terminate when input)
  --n9  <= '1' when lvds_term(1)='1' else '0'; -- TERMEN2 (terminate when input)
  --p10 <= '1' when lvds_term(2)='1' else '0'; -- TERMEN3 (terminate when input)

  --p29 <= '0' when lvds_oen(0)='1' else 'Z'; -- FPLED1/TTLIO1 red
  --p26 <= '0' when lvds_oen(1)='1' else 'Z'; -- FPLED3/TTLIO2 red
  --p16 <= '0' when lvds_oen(2)='1' else 'Z'; -- FPLED5/TTLIO3 red

  ---- LVDS inputs
  --lvds_p_i(0) <= p21; -- TTLIO1
  --lvds_p_i(1) <= p22; -- TTLIO2
  --lvds_p_i(2) <= p23; -- TTLIO3
  --lvds_p_i(3) <= p17; -- LVDS_1 / SYIN
  --lvds_p_i(4) <= p18; -- LVDS_2 / TRIN
  --lvds_n_i(0) <= n21; -- TTLIO1
  --lvds_n_i(1) <= n22; -- TTLIO2
  --lvds_n_i(2) <= n23; -- TTLIO3
  --lvds_n_i(3) <= n17; -- LVDS_1 / SYIN
  --lvds_n_i(4) <= n18; -- LVDS_2 / TRIN

  ---- LVDS outputs
  --n25 <= lvds_n_o(0); -- TTLIO1
  --n27 <= lvds_n_o(1); -- TTLIO2
  --n28 <= lvds_n_o(2); -- TTLIO3
  ----n19 <= lvds_n_o(3); -- LVDS_3 / CK200 -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
  ----n24 <= lvds_n_o(4); -- LVDS_4 / SYOU  -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
  --p25 <= lvds_p_o(0); -- TTLIO1
  --p27 <= lvds_p_o(1); -- TTLIO2
  --p28 <= lvds_p_o(2); -- TTLIO3
  ----p19 <= lvds_p_o(3); -- LVDS_3 / CK200 -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
  ----p24 <= lvds_p_o(4); -- LVDS_4 / SYOU  -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)

  ---- LVDS activity LEDs
  --n29 <= '0' when lvds_i_led(0)='1' else 'Z'; -- FPLED2/TTLIO1 blue
  --n26 <= '0' when lvds_i_led(1)='1' else 'Z'; -- FPLED4/TTLIO2 blue
  --n16 <= '0' when lvds_i_led(2)='1' else 'Z'; -- FPLED6/TTLIO3 blue
  --p5  <= '0' when lvds_i_led(3)='1' else 'Z'; -- LED1 (near HDMI = SYIN  / LVDS1)
  --n5  <= '0' when lvds_i_led(4)='1' else 'Z'; -- LED2 (near HDMI = TRIN  / LVDS2)
  ----p6  <= '0' when lvds_o_led(3)='1' else 'Z'; -- LED3 (near HDMI = CK200 / LVDS3) -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)
  ----n6  <= '0' when lvds_o_led(4)='1' else 'Z'; -- LED4 (near HDMI = SYOU  / LVDS4) -- NEEDED FOR SERDES(FPGA) TO LVDS BUFFER(BOARD)

  ---- Wires to CPLD, currently unused
  --con <= (others => 'Z');

end architecture;



