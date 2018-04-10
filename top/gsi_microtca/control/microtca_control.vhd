library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.altera_lvds_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.altera_networks_pkg.all;

entity microtca_control is
  generic( 
    g_LOAD_SHIFT_REG_EN : boolean := false
  );
  port(
    -----------------------------------------
    -- Clocks
    -----------------------------------------
    clk_20m_vcxo_i    : in std_logic; -- 20MHz VCXO clock (BANK 3A - AF21)
    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference - (clk_125m_wrpll_0  on sch)
    clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oszillator (clk_osc_1  on sch)
    clk_sfp_ref_i     : in std_logic; -- SFP clk (clk_125m_wrpll_1 on sch)
    clk_lvtio_p_i     : in std_logic; -- LEMO front panel clock input
    clk_lvtio_n_i     : in std_logic; -- LEMO front panel clock input

--    clk_osc_0_i         : in std_logic;  -- local clk from 100MHz or 125Mhz oscillator

    -----------------------------------------------------------------------
    -- reset
    -----------------------------------------------------------------------
    fpga_res_i        : in std_logic;
    nres_i            : in std_logic;
    
    -----------------------------------------------------------------------
    -- SFP 
    -----------------------------------------------------------------------
   
    sfp_tx_dis_o     : out std_logic := '0';
    sfp_tx_fault_i   : in std_logic;
    sfp_los_i        : in std_logic;
    
    sfp_txp_o        : out std_logic;
    sfp_rxp_i        : in  std_logic;
    
    sfp_mod0_i       : in    std_logic;  -- grounded by module
    sfp_mod1_io      : inout std_logic;  -- SCL
    sfp_mod2_io      : inout std_logic; -- SDA

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o  : out std_logic;
    wr_dac_din_o   : out std_logic;
    wr_ndac_cs_o   : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data_io     : inout std_logic;
    
    -----------------------------------------------------------------------
    -- lcd display
    -----------------------------------------------------------------------
    dis_di_o        : out std_logic_vector(6 downto 0);
    dis_ai_i        : in  std_logic_vector(1 downto 0);
    dis_do_i        : in  std_logic;
    dis_wr_o        : out std_logic := '0';
    dis_rst_o       : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con             : in std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- logic analyzer
    -----------------------------------------------------------------------
    hpwck           : out   std_logic := 'Z';
    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer

    -----------------------------------------------------------------------
    -- hex switch
    -----------------------------------------------------------------------
    hswf_i          : in std_logic_vector(4 downto 1);
    
    -----------------------------------------------------------------------
    -- push buttons
    -----------------------------------------------------------------------
    pbs_f_i         : in std_logic;

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd              : out   std_logic;
    slwr              : out   std_logic;
    fd                : inout std_logic_vector(7 downto 0) := (others => 'Z');
    pa                : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl               : in    std_logic_vector(2 downto 0);
    uclk              : in    std_logic;
    ures              : out   std_logic;
    ifclk             : inout std_logic := 'Z';
    wakeup            : inout std_logic := 'Z';
    
    -----------------------------------------------------------------------
    -- leds on board - user
    -----------------------------------------------------------------------
    led_user_o      : out std_logic_vector(8 downto 1) := (others => '0');
    
    -----------------------------------------------------------------------
    -- leds on front panel - status
    -----------------------------------------------------------------------
    led_status_o      : out std_logic_vector(6 downto 1);

    -----------------------------------------------------------------------
    -- lvds/lvttl lemos on front panel
    -----------------------------------------------------------------------
    lvtio_in_n_i     : in  std_logic_vector(5 downto 1);
    lvtio_in_p_i     : in  std_logic_vector(5 downto 1);
    lvtio_out_n_o    : out std_logic_vector(5 downto 1);
    lvtio_out_p_o    : out std_logic_vector(5 downto 1);
    lvtio_oe_n_o     : out std_logic_vector(5 downto 1);
    lvtio_term_en_o  : out std_logic_vector(5 downto 1);
    lvtio_led_act_o  : out std_logic_vector(5 downto 1);
    lvtio_led_dir_o  : out std_logic_vector(5 downto 1);

    -- enable clock input from front panel LEMO
    lvtio_in_clk_en_n_o   : out std_logic;



    -----------------------------------------------------------------------
    -----------------------------------------------------------------------
    -- form factor specific pins/interfaces
    -----------------------------------------------------------------------
    -----------------------------------------------------------------------

    -----------------------------------------
    -- host bus interface PCI express pins
    -----------------------------------------
    pcie_clk_i     : in  std_logic;
    pcie_rx_i      : in  std_logic_vector(3 downto 0);
    pcie_tx_o      : out std_logic_vector(3 downto 0);
    


    -----------------------------------------------------------------------
    -- lvds/lvds libera triggers on backplane
    -----------------------------------------------------------------------
    lib_trig_n_o        : out std_logic_vector(3 downto 0);
    lib_trig_p_o        : out std_logic_vector(3 downto 0);
    lib_trig_oe_o       : out std_logic;

    -----------------------------------------------------------------------
    -- lvds/m-lvds MTCA.4 triggers, gates, clocks on backplane
    -----------------------------------------------------------------------
    mlvdio_in_n_i       : in  std_logic_vector(8 downto 1);
    mlvdio_in_p_i       : in  std_logic_vector(8 downto 1);
    mlvdio_out_n_o      : out std_logic_vector(8 downto 1);
    mlvdio_out_p_o      : out std_logic_vector(8 downto 1);

    -- enable buffer output towards BACKPLANE (driver enable, active hi)
    mlvdio_de_o     : out std_logic_vector(8 downto 1);
    -- enable buffer output towards FPGA      (receiver enable, active lo)
    mlvdio_re_n_o   : out std_logic_vector(8 downto 1); 
      -- m-lvds receiver type select ( 0 - type 1, 1 - type 2)
    mlvdio_fsen_o   : out std_logic;
      -- m-lvds buffer powerdown, active low
    mlvdio_pd_n_o   : out std_logic; 

    -----------------------------------------------------------------------
    -- lvds/lvds MTCA.4 backplane clocks
    -----------------------------------------------------------------------
    tclk_in_n_i         : in  std_logic_vector(4 downto 1);
    tclk_in_p_i         : in  std_logic_vector(4 downto 1);
    tclk_out_n_o        : out std_logic_vector(4 downto 1);
    tclk_out_p_o        : out std_logic_vector(4 downto 1);
    -- enable clock buffer outputs towards BACKPLANE
    tclk_en_n_o         : out std_logic_vector(4 downto 1);
    -- enable clock buffer outputs towards FPGA
    tclk_dir_o          : out std_logic_vector(4 downto 1);

    -----------------------------------------------------------------------
    -- MTCA.4 high-speed serial connections to neighbouring slots
    -- foreseen for high speed serial links
    -----------------------------------------------------------------------
    hss_rx_n_i          : in  std_logic_vector(4 downto 1);
    hss_rx_p_i          : in  std_logic_vector(4 downto 1);
    hss_tx_n_o          : out std_logic_vector(4 downto 1);
    hss_tx_p_o          : out std_logic_vector(4 downto 1);
    -- enable hss buffer outputs to/from FPGA 
    hss_tx_en_o         : out std_logic_vector(4 downto 1);
    hss_rx_en_o         : out std_logic_vector(4 downto 1);
    
    -- enable Receive Equalization and Transmit Pre-Emphasis
    hss_tx_pe_en_o      : out std_logic;
    hss_rx_eq_en_o      : out std_logic;
     
    -----------------------------------------------------------------------
    -- mmc > fpga spi bus, mmc is master
    -----------------------------------------------------------------------l
    mmc_spi0_sck_i          : in  std_logic;
    mmc_spi0_miso_o         : out std_logic;
    mmc_spi0_mosi_i         : in  std_logic;
    mmc_spi0_sel_fpga_n_i   : in  std_logic;

    mmc_pcie_en_i           : in  std_logic;
    mmc_pcie_rst_n_i        : in  std_logic;

    mmc2fpga_usr_io         : inout std_logic_vector(2 downto 1);
    fpga2mmc_int_io         : inout std_logic; -- used for passing info to MMC if MTCA.4 buffers are enabled

    mmc_quiesce_out_i       : in  std_logic; -- mmc alert to fpga that amc will be powered off
    mmc_quiesce_in_o        : out std_logic  -- fpga reply to mmc that is ready for power down

);    
end microtca_control;

architecture rtl of microtca_control is

  constant c_HWT_EN_BIT       : natural := 8;
  constant c_IO_CLKIN_EN_BIT  : natural := 9;

  signal clk_sys              : std_logic;
  signal clk_200m             : std_logic;
  
  signal s_led_status_monster : std_logic_vector(6 downto 1);
  signal s_led_user_monster   : std_logic_vector(8 downto 1);

  signal s_led_status         : std_logic_vector(6 downto 1);
  signal s_led_user           : std_logic_vector(8 downto 1);
 
  signal s_gpio_out           : std_logic_vector(9 downto 0);
  signal s_gpio_in            : std_logic_vector(9 downto 0);
  
  signal s_test_sel           : std_logic_vector( 4 downto 0);  
  signal core_debug_out       : std_logic_vector(15 downto 0);
  
  -- white rabbit status leds
  signal s_led_link_up    : std_logic;
  signal s_led_link_act   : std_logic;
  signal s_led_track      : std_logic;
  signal s_led_pps        : std_logic;
  
  -- front panel io leds
  signal s_led_frnt_red   : std_logic;
  signal s_led_frnt_blue  : std_logic;
  
  -- user leds (on board)
  signal s_leds_user      : std_logic_vector(3 downto 0);
  
  -- monster lvds ios
  signal s_lvds_p_i     : std_logic_vector(16 downto 0);
  signal s_lvds_n_i     : std_logic_vector(16 downto 0);
  signal s_lvds_i_led   : std_logic_vector(16 downto 0);
  signal s_lvds_term_en : std_logic_vector(16 downto 0);

  signal s_lvds_p_o     : std_logic_vector(20 downto 0);
  signal s_lvds_n_o     : std_logic_vector(20 downto 0);
  signal s_lvds_o_led   : std_logic_vector(20 downto 0);
  signal s_lvds_oe      : std_logic_vector(20 downto 0);

  signal s_lvds_led     : std_logic_vector(20 downto 0);

  signal s_lvds_act_led_lvtio       : std_logic_vector(5 downto 1);
  signal s_lvds_act_led_mtca4_io    : std_logic_vector(8 downto 1);
  signal s_lvds_act_led_mtca4_clk   : std_logic_vector(4 downto 1);
  signal s_lvds_act_led_libera      : std_logic_vector(4 downto 1);


  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 40) := 
  (
 -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED_USR1   ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR2   ", IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR3   ", IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR4   ", IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR5   ", IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR6   ", IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR7   ", IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR8   ", IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("HWT_EN     ", IO_NONE,         false,   false,  8,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL), -- for testing front panel LEDs
    ("IO_CLKIN_EN", IO_NONE,         true ,   false,  9,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),

    ("HSWF1      ", IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWF2      ", IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWF3      ", IO_NONE,         false,   false,  2,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWF4      ", IO_NONE,         false,   false,  3,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),

    ("HSWP1      ", IO_NONE,         false,   false,  4,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWP2      ", IO_NONE,         false,   false,  5,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWP3      ", IO_NONE,         false,   false,  6,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("HSWP4      ", IO_NONE,         false,   false,  7,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),

    ("PBF        ", IO_NONE,         false,   false,  8,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("PBP        ", IO_NONE,         false,   false,  9,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),

    ("IO1        ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO2        ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO3        ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO4        ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO5        ", IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),

    ("MTCA4_CLK1 ", IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_CLK2 ", IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_CLK3 ", IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_CLK4 ", IO_NONE,         false,   false,  8,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),

    ("MTCA4_IO1  ", IO_NONE,         false,   false,  9,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO2  ", IO_NONE,         false,   false, 10,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO3  ", IO_NONE,         false,   false, 11,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO4  ", IO_NONE,         false,   false, 12,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO5  ", IO_NONE,         false,   false, 13,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO6  ", IO_NONE,         false,   false, 14,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO7  ", IO_NONE,         false,   false, 15,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),
    ("MTCA4_IO8  ", IO_NONE,         false,   false, 16,     IO_INOUTPUT, IO_LVDS,  true,         false,       IO_LVDS),

    ("LIBERA_TR1 ", IO_NONE,         false,   false, 17,     IO_OUTPUT  , IO_LVDS,  false,        false,       IO_LVDS),
    ("LIBERA_TR2 ", IO_NONE,         false,   false, 18,     IO_OUTPUT  , IO_LVDS,  false,        false,       IO_LVDS),
    ("LIBERA_TR3 ", IO_NONE,         false,   false, 19,     IO_OUTPUT  , IO_LVDS,  false,        false,       IO_LVDS),
    ("LIBERA_TR4 ", IO_NONE,         false,   false, 20,     IO_OUTPUT  , IO_LVDS,  false,        false,       IO_LVDS)

  );

  
  constant c_family  : string := "Arria V"; 
  constant c_project : string := "microtca_control";
  constant c_cores      : natural:= 1;
  constant c_initf_name : string := c_project & "_stub.mif";
  constant c_profile_name : string := "medium_icache_debug";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';' 
  signal s_wr_ext_in    : std_logic;
  
  -- logic analyzer
  signal s_log_oe                 : std_logic_vector(16 downto 0);
  signal s_log_out                : std_logic_vector(16 downto 0);
  signal s_log_in                 : std_logic_vector(16 downto 0);

  
  -- connections from monster
  signal s_monster_tclk_en        : std_logic_vector(8 downto 1);
  signal s_monster_tclk_dir       : std_logic_vector(8 downto 1);

  signal s_monster_mlvd_buf_en    : std_logic_vector(8 downto 1);
  signal mlvdio_de                : std_logic_vector(8 downto 1);

  signal s_monster_hss_buf_en     : std_logic_vector(8 downto 1);

  signal s_dis_led_green          : std_logic;
  signal s_dis_led_red            : std_logic;
  signal s_dis_led_blue           : std_logic;

  signal s_mmc_in_libera          : std_logic;
  signal s_mmc_in_libera_slot8    : std_logic;

  signal s_mtca4_bpl_buf_en_sw    : std_logic;
  signal s_mtca4_bpl_buf_en       : std_logic;

  signal s_libera_trig_buf_en_sw  : std_logic;
  signal s_libera_trig_buf_en     : std_logic;

  signal s_shift_reg_to_leds      : std_logic_vector(15 downto 0):=(others =>'0');

  signal clk_lvtio                : std_logic;
  signal clk_lvtio_global         : std_logic;

  signal lvtio_clk_divider        : unsigned(23 downto 0);

  signal hss_rx                   : std_logic_vector(4 downto 1);
 
  
begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_lvds_inout      => 17, -- 5 LEMOs on front panel, 8 MTCA4 on BPL, 4 MTCA4 clk on BPL
      g_lvds_in         => 0,
      g_lvds_out        => 4,  -- 4 libera triggers at BPL
      g_gpio_out        => 10,  -- 8 on-boards LEDs, 1 test mode enable, 1 IO5 input clock buffer enable
      g_gpio_in         => 10, -- 4 FPGA HEX switch, 4 CPLD HEX switch, 1 FPGA button, 1 CPLD button
      g_fixed           => 0,
      g_lvds_invert     => false,
      g_en_usb          => true,
      g_en_lcd          => true,
      g_en_user_ow      => false,
      g_en_tempsens     => true,
      g_en_pcie         => true,
      g_io_table        => io_mapping_table,
      g_lm32_cores      => c_cores,
      g_lm32_ramsizes   => c_lm32_ramsizes/4,
      g_lm32_init_files => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
    )  
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => clk_sfp_ref_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_rstn_i            => fpga_res_i,
      core_clk_sys_o         => clk_sys,
      core_clk_200m_o        => clk_200m,
      core_debug_o           => core_debug_out,

      wr_onewire_io          => rom_data_io,

      wr_sfp_sda_io          => sfp_mod2_io,
      wr_sfp_scl_io          => sfp_mod1_io,
      wr_sfp_det_i           => sfp_mod0_i,
      wr_sfp_tx_o            => sfp_txp_o,
      wr_sfp_rx_i            => sfp_rxp_i,

      wr_dac_sclk_o          => wr_dac_sclk_o,
      wr_dac_din_o           => wr_dac_din_o,
      wr_ndac_cs_o           => wr_ndac_cs_o,
      wr_ext_clk_i           => '0',

      sfp_tx_disable_o       => sfp_tx_dis_o,
      sfp_tx_fault_i         => sfp_tx_fault_i,
      sfp_los_i              => sfp_los_i,

      gpio_o                 => s_gpio_out,
      gpio_i                 => s_gpio_in,

      lvds_p_i               => s_lvds_p_i,
      lvds_n_i               => s_lvds_n_i,
      lvds_i_led_o           => s_lvds_i_led,

      lvds_p_o               => s_lvds_p_o,
      lvds_n_o               => s_lvds_n_o,
      lvds_o_led_o           => s_lvds_o_led,
      lvds_oen_o             => s_lvds_oe,
      lvds_term_o            => s_lvds_term_en,

      led_link_up_o          => s_led_link_up,
      led_link_act_o         => s_led_link_act,
      led_track_o            => s_led_track,
      led_pps_o              => s_led_pps,

      usb_rstn_o             => ures,
      usb_ebcyc_i            => pa(3),
      usb_speed_i            => pa(0),
      usb_shift_i            => pa(1),
      usb_readyn_io          => pa(7),
      usb_fifoadr_o          => pa(5 downto 4),
      usb_sloen_o            => pa(2),
      usb_fulln_i            => ctl(1),
      usb_emptyn_i           => ctl(2),
      usb_slrdn_o            => slrd,
      usb_slwrn_o            => slwr,
      usb_pktendn_o          => pa(6),
      usb_fd_io              => fd,

      lcd_scp_o              => dis_di_o(3),
      lcd_lp_o               => dis_di_o(1),
      lcd_flm_o              => dis_di_o(2),
      lcd_in_o               => dis_di_o(0),

      pcie_refclk_i          => pcie_clk_i,
      pcie_rstn_i            => mmc_pcie_rst_n_i,
      pcie_rx_i              => pcie_rx_i,
      pcie_tx_o              => pcie_tx_o
  );


  -- hex switches as gpio inputs
  s_gpio_in(3 downto 0) <= not hswf_i; -- FPGA HEX switch
  s_gpio_in(7 downto 4) <= hss_rx when s_gpio_out(c_HWT_EN_BIT)='1' else con(4 downto 1); -- CPLD HEX switch (already inverted in CPLD)

  -- button as gpio inputs
  s_gpio_in(8) <= not pbs_f_i; -- FPGA push button
  s_gpio_in(9) <= con(5);      -- CPLD push button (already inverted in CPLD)


  -- test mode select via hex switch or sw
  -- invert FPGA button and HEX switch
  s_test_sel(4)          <= s_gpio_out(7)          when s_gpio_out(c_HWT_EN_BIT)='1' else not pbs_f_i;
  s_test_sel(3 downto 0) <= s_gpio_out(3 downto 0) when s_gpio_out(c_HWT_EN_BIT)='1' else not hswf_i ;

  -- Display
  dis_wr_o    <= '0';
  dis_rst_o   <= '1';

  -----------------------------------------------------------
  -- LEDs
  -----------------------------------------------------------

  -- WR status LEDs 
  s_dis_led_green <= s_gpio_out(4) when s_gpio_out(8)='1' else (    s_led_link_up and     s_led_track); -- green
  s_dis_led_red   <= s_gpio_out(5) when s_gpio_out(8)='1' else (not s_led_link_up                    ); -- red
  s_dis_led_blue  <= s_gpio_out(6) when s_gpio_out(8)='1' else (    s_led_link_up and not s_led_track); -- blue
  
  -- display backlight color - pullups
  dis_di_o(4) <= '0' when s_dis_led_green = '1' else 'Z'; -- green
  dis_di_o(5) <= '0' when s_dis_led_red   = '1' else 'Z'; -- red
  dis_di_o(6) <= '0' when s_dis_led_blue  = '1' else 'Z'; -- blue
  
  -- Link LEDs
  s_led_status_monster(6) <= s_led_link_act and s_led_link_up;   -- red   = traffic/no-link
  s_led_status_monster(5) <= s_led_link_up;                      -- blue  = link
  s_led_status_monster(4) <= s_led_track;                        -- green = timing valid
  s_led_status_monster(3) <= s_led_pps;                          -- white = PPS

  -- Front panels user LEDs
  -- PCI reset indicator
  s_led_status_monster(2) <= not mmc_pcie_rst_n_i; --(~100ms blink when PCIe reset asserted from MMC)
  s_led_status_monster(1) <= s_gpio_out(0);

  -- status LED output according to FPGA hex switch position and fpga button
  with s_test_sel select
    s_led_status <= "000000"                        	when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                    "111111"                          when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                    s_shift_reg_to_leds(15 downto 10) when ('0' & x"A"),   -- FPGA hex sw in position A, button not pressed, led test
                    (  s_libera_trig_buf_en_sw 
                     & s_libera_trig_buf_en & '0'
                     & s_mtca4_bpl_buf_en_sw
                     & s_mtca4_bpl_buf_en & '0')      when ('0' & x"9"),   -- FPGA hex sw in position 9, button not pressed, backplane buffer enables
                    s_led_status_monster              when others;         -- driven by monster

  led_status_o <= not s_led_status;                  

  -- USER LED output according to fpga hex switch position and fpga button                  
  with s_test_sel select
    s_led_user <= x"00"                       when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                  x"FF"                       when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                  ("000" &     con)           when ('0' & x"D"),   -- FPGA hex sw in position D, button not pressed, CPLD HEX SW and button test  
                  ("000" & not con)           when ('1' & x"D"),   -- FPGA hex sw in position D, button     pressed, CPLD HEX SW and button test  
                  s_gpio_out(7 downto 0)      when others;         -- driven by monster

  led_user_o <= not s_led_user;
 

  -- enable LEMO output buffers (active LO)
  lvtio_oe_n_o <= not s_lvds_oe(4 downto 0);

  -- LEMO activity LEDs (active HI)
  s_lvds_act_led_lvtio  <= s_lvds_i_led( 4 downto  0) or s_lvds_o_led( 4 downto  0);

  -- MTCA.4 CLK activity LEDs (active HI)
  s_lvds_act_led_mtca4_clk  <= s_lvds_i_led(8 downto 5) or s_lvds_o_led(8 downto 5);

  -- MTCA.4 IO activity LEDs (active HI)
  s_lvds_act_led_mtca4_io  <= s_lvds_i_led(16 downto  9) or s_lvds_o_led(16 downto  9);

  -- Libera out activity LEDs (active HI)
  s_lvds_act_led_libera <= s_lvds_o_led(20 downto 17);
  
  -- LVDS termination pins (active hi)
  with s_test_sel select
    lvtio_term_en_o <= (others => '0')                 when ('0' & x"E"),   -- FPGA hex sw in position E, button not pressed, termination test
                       (others => '1')                 when ('1' & x"E"),   -- FPGA hex sw in position E, button     pressed, termination test
                        s_lvds_term_en(4 downto 0) when others;             -- driven by monster (enable termination when output disabled)

  -- LVDS direction indicator RED LEDs (active hi)
  with s_test_sel select
    lvtio_led_dir_o <= (others => '0')                          when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '1')                          when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
                        s_shift_reg_to_leds(9 downto 5)         when ('0' & x"A"),   -- FPGA hex sw in position A, button not pressed, shift reg to leds
                        '0' & hss_rx                            when ('0' & x"8"),   -- FPGA hex sw in position 8, button not pressed, hss rx inputs showing libera trigger lines (AC-coupled)
                        '0' & mlvdio_de(8 downto 5)             when ('0' & x"7"),   -- FPGA hex sw in position 7, button not pressed, mtca4io output enable 8-5
                        '0' & mlvdio_de(4 downto 1)             when ('0' & x"6"),   -- FPGA hex sw in position 6, button not pressed, mtca4io output enable 4-1
                        '0' & s_monster_tclk_dir(4 downto 1)    when ('0' & x"5"),   -- FPGA hex sw in position 5, button not pressed, mtca4 clk direction
                        std_logic_vector(lvtio_clk_divider(4 downto 0))           when ('0' & x"4"),   -- FPGA hex sw in position 4, button not pressed, lvtio clk divider
                        s_lvds_oe(4 downto 0)                   when others;         -- LEMO IO direction

  -- LVDS activity indicator BLUE LEDs (active hi)
  with s_test_sel select
    lvtio_led_act_o <= (others => '0')                              when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '1')                              when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
                        s_shift_reg_to_leds(4 downto 0)             when ('0' & x"A"),   -- FPGA hex sw in position A, button not pressed, shift reg to leds
                        '0' & s_lvds_act_led_libera                 when ('0' & x"8"),   -- FPGA hex sw in position 8, button not pressed, libera out activity to front io leds
                        '0' & s_lvds_act_led_mtca4_io(8 downto 5)   when ('0' & x"7"),   -- FPGA hex sw in position 7, button not pressed, mtca4io 5-8 activity to front io leds
                        '0' & s_lvds_act_led_mtca4_io(4 downto 1)   when ('0' & x"6"),   -- FPGA hex sw in position 6, button not pressed, mtca4io 1-4 activity to front io leds
                        '0' & s_lvds_act_led_mtca4_clk              when ('0' & x"5"),   -- FPGA hex sw in position 5, button not pressed, mtca4 clk activity
                        std_logic_vector(lvtio_clk_divider(23 downto 19))             when ('0' & x"4"),   -- FPGA hex sw in position 4, button not pressed, lvtio clk divider
                        s_lvds_act_led_lvtio                        when others;         -- LEMO IO activity

  -----------------------------------------------------------
  -- lemo io connectors on front panel to/from monster
  -----------------------------------------------------------
  -- lvds/lvttl lemos in/out
  s_lvds_p_i(4 downto 0) <= lvtio_in_p_i(5 downto 1);
  s_lvds_n_i(4 downto 0) <= lvtio_in_n_i(5 downto 1);

  lvtio_out_p_o(5 downto 1)   <= s_lvds_p_o(4 downto 0);
  lvtio_out_n_o(5 downto 1)   <= s_lvds_n_o(4 downto 0);


  -- External white rabbit clock input enable (active low)
  lvtio_in_clk_en_n_o <= not s_gpio_out(c_IO_CLKIN_EN_BIT); 


  ------------------------------------------------------------
  -- IO5 clk input 
  ------------------------------------------------------------
  lvtio_clk_inbuf : altera_lvds_ibuf
  generic map(
    g_family  => c_family)
  port map(
    datain_b  => clk_lvtio_n_i,
    datain    => clk_lvtio_p_i,
    dataout   => clk_lvtio
  );

  lvtio_clk_buf : global_region
  port map(
    inclk  => clk_lvtio,
    outclk => clk_lvtio_global
  );

  p_lvtio_clk_div: process(clk_lvtio_global)
  begin 
    if rising_edge(clk_lvtio_global) then
      lvtio_clk_divider <= lvtio_clk_divider + 1;
    else
      lvtio_clk_divider <= lvtio_clk_divider ;
    end if;
  end process;
  

  -----------------------------------------------------------------------
  -----------------------------------------------------------------------
  -- form factor specific inputs/outputs
  -----------------------------------------------------------------------
  -----------------------------------------------------------------------

  -----------------------------------------------------------
  -- microTCA.4 backplane triggers, inputs and outputs
  -----------------------------------------------------------

  -- select receiver input Type for onboard M-LVDS buffers to backplane
  -- ('0' = Type-1 , '1' = Type-2 )
  mlvdio_fsen_o <= s_gpio_out(7); 

  -- MTCA.4 bussed trigger lines (PORTS 17-20)
  s_lvds_n_i(16 downto 9) <= mlvdio_in_n_i(8 downto 1);
  s_lvds_p_i(16 downto 9) <= mlvdio_in_p_i(8 downto 1);

  mlvdio_out_n_o <= s_lvds_n_o(16 downto 9);
  mlvdio_out_p_o <= s_lvds_p_o(16 downto 9);

  ----------------------------------------
  -- MTCA.4 clocks (TCLKA, TCLKB, TCLKC, TCLKD)
  s_lvds_n_i(8 downto 5) <= tclk_in_n_i;
  s_lvds_p_i(8 downto 5) <= tclk_in_p_i;

  tclk_out_n_o <= s_lvds_n_o(8 downto 5);
  tclk_out_p_o <= s_lvds_p_o(8 downto 5);

  -----------------------------------------------------------
  -- Libera outputs
  -----------------------------------------------------------

  -- no inputs from Libera backplane, outputs only
  lib_trig_n_o <= s_lvds_n_o(20 downto 17);
  lib_trig_p_o <= s_lvds_p_o(20 downto 17);


  -----------------------------------------------------------
  -- SRIO (High-Speed-Serial links) backplane ports 12-15
  -----------------------------------------------------------

  -- usage of backplane ports 12-15 currently not defined
  -- therefore only dummy buffers to keep Quartus happy
  unused_hss_ios: for i in 1 to 4 generate
    hss_obuf : altera_lvds_obuf
      generic map(
        g_family  => c_family)
      port map(
        datain    => '0',
        dataout   => hss_tx_p_o(i),
        dataout_b => hss_tx_n_o(i)
      );

    hss_inbuf : altera_lvds_ibuf
        generic map(
          g_family  => c_family)
        port map(
          datain_b  => hss_rx_n_i(i),
          datain    => hss_rx_p_i(i),
          dataout   => hss_rx(i)
        );
  end generate;


  ----------------------------------------------
  -- mmc alerts fpga that amc will be powered off and
  -- fpga replies to mmc that is ready for power down.
  -- If needed, response to mmc can be delayed 
  -- (for example to finish writing to flash or to complete data transfer, etc)
  mmc_quiesce_in_o  <= mmc_quiesce_out_i;        

  -----------------------------------------------------------------------
  -- backplane ports configuration from monster
  ----------------------------------------------------------------------- 
  s_monster_tclk_en (4 downto 1)    <= s_lvds_oe(8 downto 5);
  s_monster_tclk_dir(4 downto 1)    <= s_lvds_oe(8 downto 5);
  s_monster_mlvd_buf_en(8 downto 1) <= s_lvds_oe(16 downto 9);
  s_monster_hss_buf_en (4 downto 1) <= (others => '0');

  -----------------------------------------------------------------------
  -- backplane ports configuration from MMC
  ----------------------------------------------------------------------- 
  -- bpl buffer enable generation depends on the crate in wich AMC is (MTCA.0, MTCA.4, Libera)


  -----------------------------------------------------------------------
  -- lvds/lvds libera trigger buffers enable (active HI)
  -- libera trigger bufferes are disabled by default,
  -- MMC enables Libera trigger buffer only in Libera Slot 8
  -- mmc2fpga_usr_io(1): 0 - we are not in Libera, 1 - we are in Libera
  -- mmc2fpga_usr_io(2): 0 - we are not in Libera Slot8, 1 - we are in Libera Slot8
  s_mmc_in_libera       <= mmc2fpga_usr_io(1);
  s_mmc_in_libera_slot8 <= mmc2fpga_usr_io(2);

  -- Libera trigger buffer enable from SW
  -- USE THIS ONLY when FTRN is not in the crate or is on the AMC extender with cut PORT 6-7 lines!!!
  s_libera_trig_buf_en_sw <= '1' when (s_gpio_out(6 downto 4) = "101" and s_gpio_out(c_HWT_EN_BIT)='1') else '0'; 

  -- enable libera trigger buffers when 
  -- in Libera Slot8 or when
  -- not in Libera (outside or MTCA/MTCA.4 - !!! this option is foreseen only for testing libera trigger outputs
  --                                             on the modified extender board with cut PORT 6-7 lines
  s_libera_trig_buf_en  <= '1' when (  ( s_mmc_in_libera       = '1' and 
                                         s_mmc_in_libera_slot8 = '1' and 
                                         mmc_quiesce_out_i     = '0'
                                       ) or
                                       (  s_mmc_in_libera         = '0' and 
                                          s_libera_trig_buf_en_sw = '1'
                                       )
                                    )
                       else '0';

  lib_trig_oe_o <= s_libera_trig_buf_en;

  -----------------------------------------------------------------------
  -- MTCA.4 buffers enable generation by software
  -----------------------------------------------------------------------
  -- enabled only in MTCA.4 by SW
  s_mtca4_bpl_buf_en_sw <= '1' when (s_gpio_out(6 downto 4) = "110" and s_gpio_out(c_HWT_EN_BIT)='1') else '0';

  -- crosscheck with Libera and quiesce.
  -- because there is no way to know if we are in MTCA or MTCA.4 crate
  -- it is up to user to ensure that MTCA.4 buffers are not turned on in MTCA crate!
  s_mtca4_bpl_buf_en <= '1' when (s_mmc_in_libera       = '0' and
                                  s_mtca4_bpl_buf_en_sw = '1' and 
                                  mmc_quiesce_out_i     = '0'
                                 )
                         else '0'; 

  -- pass state of the MTCA.4 buffer enable to the MMC (in MMC implemented as discrete sensor)
  fpga2mmc_int_io <= s_mtca4_bpl_buf_en;

  -----------------------------------------------------------------------
  -- lvds/m-lvds MTCA.4 buffers enable generation
  -----------------------------------------------------------------------
  -- m-lvds buffer powerdown (active low) (0 - powered down, 1 - powered up)
  mlvdio_pd_n_o <= s_mtca4_bpl_buf_en; -- buffer power enable

  -- m-lvds direction
  gen_mlvd_buf_oe : for i in  1 to 8 generate
    -- enable buffer output towards BACKPLANE (m-lvds output driver enable, active hi)
    mlvdio_de(i) <= '1' when (s_mtca4_bpl_buf_en        = '1' and 
                              s_monster_mlvd_buf_en(i)  = '1')
                      else '0';

    mlvdio_de_o(i) <= mlvdio_de(i);
                               
    -- enable buffer output towards FPGA (m-lvds receiver enable, active low)
    mlvdio_re_n_o(i) <=  '0'; -- always on (listen while talk)

  end generate; -- gen_mlvd_buf_oe


  -----------------------------------------------------------------------
  -- lvds/lvds MTCA.4 backplane clock buffers enable, direction generation
  -----------------------------------------------------------------------
  gen_tclk_oe_dir : for i in  1 to 4 generate
    -- enable clock switches, connect FPGA and BACKPLANE (0 - normal operation, 1 - HI-Z shutdown)
    tclk_en_n_o(i) <= '0' when s_mtca4_bpl_buf_en   = '1' else '1';

    -- clock direction
    -- A - backplane, B - FPGA clk output, C - FPGA clk input
    -- dir: 0 - B <> A, 1 - C <> A
    -- enable clock buffer towards FPGA, switch clock mux to B-A (active )
    tclk_dir_o(i)<= '0' when (s_mtca4_bpl_buf_en    = '1' and 
                              s_monster_tclk_en(i)  = '1' and 
                              s_monster_tclk_dir(i) = '1')
                     else '1';
  end generate; -- gen_tclk_oe_dir

  -----------------------------------------------------------------------
  -- MTCA.4 PORT 12-15 buffers enable generation (active HI)
  -----------------------------------------------------------------------
  -- currently not used, keep disabled
  gen_hss_buf_oe : for i in  1 to 4 generate
    hss_tx_en_o(i) <= '0';    
    hss_rx_en_o(i) <= s_gpio_out(c_HWT_EN_BIT);    
  end generate; -- gen_hss_buf_oe

  -- disable  Transmit Pre-Emphasis and Receive Equalization
  hss_tx_pe_en_o <= '0';
  hss_rx_eq_en_o <= '0';


  gen_load_shift_reg_true: if g_LOAD_SHIFT_REG_EN = true generate

    -- for testing
    constant c_LOAD_SHIFT_REG_WIDTH : positive := 16;
    constant c_LOAD_SHIFT_REG_DEPTH : positive := 6*(2**9);

    type t_bit_array is array (natural range<>) of std_logic_vector(c_LOAD_SHIFT_REG_DEPTH-1 downto 0);

    signal s_reg_blink_counter  : unsigned(27 downto 0);


    signal s_load_shift_en      : std_logic;
    signal s_shift_reg_out      : std_logic_vector(c_LOAD_SHIFT_REG_WIDTH-1 downto 0);
    signal s_load_shift_reg_arr : t_bit_array(c_LOAD_SHIFT_REG_WIDTH-1 downto 0);
    signal s_pseudo_rand_reg 	  : std_logic_vector(31 downto 0) := (others => '0');
    signal s_led_reg_en         : std_logic;

  begin
    -----------------------------------------------------------------------
    -- TEST: Shift register to consume resources not used by monster.
    --       Enabled by SW.
    -----------------------------------------------------------------------
    s_load_shift_en <= '1' when s_test_sel = ('0' & x"A") else '0';
    
    -- mega shift regitster to use up the rest of resources, input is pseudo random generated vectors
    p_load_shift_reg: process(clk_200m)
      -- maximal length 32-bit xnor LFSR based on xilinx app note XAPP210
	    function lfsr32(x : std_logic_vector(31 downto 0)) return std_logic_vector is
	    begin
		   return x(30 downto 0) & (x(0) xnor x(1) xnor x(21) xnor x(31));
	    end function;
    begin
      if rising_edge(clk_200m) then 

        s_pseudo_rand_reg <= lfsr32(s_pseudo_rand_reg);
          
        for i in 0 to (c_LOAD_SHIFT_REG_WIDTH- 1) loop
          if s_load_shift_en = '1' then 
            s_load_shift_reg_arr(i) <= s_load_shift_reg_arr(i)(c_LOAD_SHIFT_REG_DEPTH-2 downto 0) & s_pseudo_rand_reg(i);
          else
            s_load_shift_reg_arr(i) <= s_load_shift_reg_arr(i);
          end if;

          -- assign shift register output to logic analyzer port
          s_shift_reg_out(i)     <= s_load_shift_reg_arr(i)(c_LOAD_SHIFT_REG_DEPTH-1);
			   
				  if s_led_reg_en = '1' then 
            s_shift_reg_to_leds(i) <= s_load_shift_reg_arr(i)(c_LOAD_SHIFT_REG_DEPTH-1) ;
				  else
            s_shift_reg_to_leds(i) <= s_shift_reg_to_leds(i);
				  end if;
        end loop;
      end if;
    end process;

    p_reg_blink_counter: process(clk_200m)
    begin
      if rising_edge(clk_200m) then
        if s_led_reg_en = '1' then
          s_reg_blink_counter <= (others => '0');
        else
          s_reg_blink_counter <= s_reg_blink_counter + 1;
        end if;
      end if;
    end process;    

    -- show shift reg output on front panel leds, set refresh rate with CPLD hex switch)
    s_led_reg_en <= '1' when ((to_integer(s_reg_blink_counter) >=  25_000_000 and con(4 downto 1) = x"0") or
                              (to_integer(s_reg_blink_counter) >=  50_000_000 and con(4 downto 1) = x"1") or
                              (to_integer(s_reg_blink_counter) >= 100_000_000 and con(4 downto 1) = x"2")
                             ) 
                     else '0';
    
    -- assign shift register output also to logic analyzer port
    hpw <= s_shift_reg_out(hpw'range);
    
    s_log_in(15 downto 0) <= (others => '0');
    s_log_in(16)          <= '0';

  end generate; -- gen_load_shift_reg_true

  gen_load_shift_reg_false: if g_LOAD_SHIFT_REG_EN = false generate

    -- Logic analyzer
    -- inputs
    s_log_in(15 downto 0) <= hpw(15 downto 0);
    s_log_in(16)          <= hpwck;

    -- outputs
    hpwck     <= s_log_out(16) when s_log_oe(16) = '1' else 'Z';
    hpw_out : for i in 0 to 15 generate
      hpw(i)  <= s_log_out(i) when s_log_oe(i) = '1' else 'Z';
    end generate;  
  end generate; -- gen_load_shift_reg_false
  
end rtl;

