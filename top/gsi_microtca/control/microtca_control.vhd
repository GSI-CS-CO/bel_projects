library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.altera_lvds_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.spi_slave_pkg.all;
--use work.microtca_ctrl_auto_pkg.all;


entity microtca_control is
--  generic(
--    g_top_lvds_inout_front : natural := 5; -- front end lemos(5)
--    g_top_lvds_tclk_mtca   : natural := 4; -- TCLK
--    g_top_lvds_inout_mtca  : natural := 8; -- MicroTCA.4 backplane triggers/gates/clocks(8)
--    g_top_lvds_out_libera  : natural := 4  -- Libera backplane triggers (4) 
--  );
  port(
    clk_20m_vcxo_i      : in std_logic;  -- 20MHz VCXO clock

    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference - (clk_125m_wrpll_0  on sch)
    clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oszillator (clk_osc_1  on sch)
    clk_sfp_ref_i     : in std_logic; -- SFP clk (clk_125m_wrpll_1 on sch)
    lvt_clk_i         : in std_logic; -- LEMO front panel input

--    clk_osc_0_i         : in std_logic;  -- local clk from 100MHz or 125Mhz oscillator
    
    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_clk_i     : in  std_logic;
    pcie_rx_i      : in  std_logic_vector(3 downto 0);
    pcie_tx_o      : out std_logic_vector(3 downto 0);
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o  : out std_logic;
    wr_dac_din_o   : out std_logic;
    wr_ndac_cs_o   : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data        : inout std_logic;
    
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
    -- io
    -----------------------------------------------------------------------
    fpga_res        : in std_logic;
    nres            : in std_logic;
    pbs_f_i         : in std_logic;
    hswf_i          : in std_logic_vector(4 downto 1);
    

    hpwck           : out   std_logic;
    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
    
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
    lvt_in_clk_en_n_o   : out std_logic;

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
    hss_tx_n_o            : out std_logic_vector(4 downto 1);
    hss_tx_p_o            : out std_logic_vector(4 downto 1);
    -- enable hss buffer outputs to/from FPGA 
    hss_tx_en_o         : out std_logic_vector(4 downto 1);
    hss_rx_en_o         : out std_logic_vector(4 downto 1);
    
    -- enable Receive Equalization and Transmit Pre-Emphasis
    hss_tx_pe_en_o      : out std_logic;
    hss_rx_eq_en_o      : out std_logic;
     
    -----------------------------------------------------------------------
    -- mmc > fpga spi bus, mmc is master
    -----------------------------------------------------------------------l
    mmc_spi0_sck_i            : in  std_logic;
    mmc_spi0_miso_o           : out std_logic;
    mmc_spi0_mosi_i           : in  std_logic;
    mmc_spi0_sel_fpga_n_i   : in  std_logic;

    mmc_pcie_en_i              : in  std_logic;
    mmc_pcie_rst_n_i        : in  std_logic;

    mmc2fpga_usr_i            : in  std_logic_vector(2 downto 1);
    fpga2mmc_int_o            : out std_logic; -- interrupt to mmc

    mmc_quiesce_out_i       : in  std_logic; -- mmc alert to fpga that amc will be powered off
    mmc_quiesce_in_o        : out std_logic; -- fpga reply to mmc that is ready for power down

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd            : out   std_logic;
    slwr            : out   std_logic;
    fd              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    pa              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl             : in    std_logic_vector(2 downto 0);
    uclk            : in    std_logic;
    ures            : out   std_logic;
    ifclk           : out   std_logic;
    
    -----------------------------------------------------------------------
    -- leds (6 LEDs for WR and FTRN status)
    -----------------------------------------------------------------------
    led_status_o    : out std_logic_vector(6 downto 1) := (others => '0');
    led_user_o      : out std_logic_vector(8 downto 1) := (others => '0');
    
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
    sfp_mod2_io      : inout std_logic); -- SDA
    
end microtca_control;

architecture rtl of microtca_control is

  -- addresses for SPI bus between MMC and FPGA
  constant c_BPL_CONF_TCLK_EN       : natural := 0;
  constant c_BPL_CONF_TCLK_DIR      : natural := 1;
  constant c_BPL_CONF_MLVD_BUF_EN   : natural := 2;
  constant c_BPL_CONF_MLVD_BUF_DIR  : natural := 3;
  constant c_BPL_CONF_HSS_BUF_EN    : natural := 4;
  constant c_BPL_STAT_LIBERA_BUF_EN : natural := 5;
  constant c_BPL_STAT_MTCA4_BPL_DIS : natural := 6;


  signal clk_sys       : std_logic;
  
  signal s_led_status_monster  : std_logic_vector(6 downto 1);
  signal s_led_user_monster   : std_logic_vector(8 downto 1);

  signal s_led_status         : std_logic_vector(6 downto 1);
  signal s_led_user           : std_logic_vector(8 downto 1);
 
  signal s_gpio_out           : std_logic_vector(8 downto 0);
  signal s_gpio_in            : std_logic_vector(9 downto 0);
  
  constant c_test_pattern_a   : std_logic_vector(15 downto 0) := x"5555";
  constant c_test_pattern_b   : std_logic_vector(15 downto 0) := x"0000";  

  signal s_test_sel    : std_logic_vector(4 downto 0);  
  
  -- white rabbits leds
  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;
  
  -- front end leds
  signal s_led_frnt_red  : std_logic;
  signal s_led_frnt_blue : std_logic;
  
  -- user leds (on board)
  signal s_leds_user    : std_logic_vector(3 downto 0);
  

  signal s_lvds_p_i     : std_logic_vector(16 downto 0);
  signal s_lvds_n_i     : std_logic_vector(16 downto 0);
  signal s_lvds_i_led   : std_logic_vector(16 downto 0);
  signal s_lvds_term_en : std_logic_vector(16 downto 0);

  signal s_lvds_p_o     : std_logic_vector(20 downto 0);
  signal s_lvds_n_o     : std_logic_vector(20 downto 0);
  signal s_lvds_o_led   : std_logic_vector(20 downto 0);
  signal s_lvds_oe      : std_logic_vector(20 downto 0);

  signal s_lvds_led     : std_logic_vector(20 downto 0);


  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 39) := 
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED_USR1_R ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR2_B ", IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR3_G ", IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR4_W ", IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR5_R ", IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR6_B ", IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR7_G ", IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED_USR8_W ", IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),

    ("HWT_EN     ", IO_NONE,         false,   false,  8,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL), -- for testing front panel LEDs

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

    ("LEMO_IO1   ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO2   ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO3   ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO4   ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO5   ", IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),

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
  

       
  signal s_mtca4_trig_oe_reg      : std_logic_vector(8 downto 1);
  signal s_mtca4_trig_pdn_reg     : std_logic;

  signal s_mtca4_clk_oe_reg       : std_logic_vector(4 downto 1);
  signal s_MTCA4_CLKig_oe_reg     : std_logic;

  signal s_rstn_mmc_spi           : std_logic;
  signal s_clk_mmc_spi            : std_logic;

  -- logic analyzer
  signal s_log_oe   : std_logic_vector(16 downto 0);
  signal s_log_out  : std_logic_vector(16 downto 0);
  signal s_log_in   : std_logic_vector(16 downto 0);
  

  -- internal spi slave interface
  signal s_mmcspi_di_req    : std_logic;                      -- preload lookahead data request line
  signal s_mmcspi_di        : std_logic_vector(15 downto 0);  -- parallel load data in (clocked in on rising edge of clk_i)
  signal s_mmcspi_wren      : std_logic;                      -- user data write enable
  signal s_mmcspi_di_wrack  : std_logic;                      -- write acknowledge
  signal s_mmcspi_do_valid  : std_logic;                      -- do_o data valid strobe valid during one clk_i rising edge
  signal s_mmcspi_do        : std_logic_vector(15 downto 0);  -- parallel output (clocked out on falling clk_i)  

  alias  a_mmcspi_we        : std_logic is s_mmcspi_do(15);   -- write enable bit in mmc spi command
  alias  a_mmcspi_re        : std_logic is s_mmcspi_do(14);   -- read  enable bit in mmc spi command
  alias  a_mmcspi_addr      : std_logic_vector(5 downto 0) is s_mmcspi_do(13 downto 8);

  -- connections from microtca_ctrl registers
  signal s_monster_tclk_en  : std_logic_vector(8 downto 1);
  signal s_monster_tclk_dir     : std_logic_vector(8 downto 1);

  signal s_monster_mlvd_buf_en  : std_logic_vector(8 downto 1);
  signal s_monster_mlvd_dir     : std_logic_vector(8 downto 1);

  signal s_monster_hss_buf_en   : std_logic_vector(8 downto 1);

  -- registers written by mmc
  signal s_mmc_libera_buf_en_reg  : std_logic_vector(7 downto 0);
  signal s_mmc_mtca4_bpl_dis_reg  : std_logic_vector(7 downto 0);

  signal s_mtca4_bpl_buff_en    : std_logic;
  signal s_libera_bpl_buff_en   : std_logic;

  signal s_dis_led_green : std_logic;
  signal s_dis_led_red   : std_logic;
  signal s_dis_led_blue  : std_logic;

  signal s_lib_trig_oe_sw_mmc	: std_logic;

  signal s_counter : unsigned(23 downto 0);
  
  
begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_lvds_inout      => 17, -- 5 LEMOs on front panel, 8 MTCA4 on BPL, 4 MTCA4 clk on BPL
      g_lvds_in         => 0,
      g_lvds_out        => 4,  -- 4 libera triggers at BPL
      g_gpio_out        => 9,  -- 8 on-boards LEDs, 1 test mode enable
      g_gpio_in         => 10, -- 4 FPGA HEX switch, 4 CPLD HEX switch, 1 FPGA button, 1 CPLD buttong
      g_fixed           => 0,
      g_lvds_invert     => false,
      g_en_usb          => true,
      g_en_lcd          => true,
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
      core_rstn_i            => fpga_res,
      core_clk_butis_t0_o    => clk_sys,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp_mod2_io,
      wr_sfp_scl_io          => sfp_mod1_io,
      wr_sfp_det_i           => sfp_mod0_i,
      wr_sfp_tx_o            => sfp_txp_o,
      wr_sfp_rx_i            => sfp_rxp_i,
      wr_dac_sclk_o          => wr_dac_sclk_o,
      wr_dac_din_o           => wr_dac_din_o,
      wr_ndac_cs_o           => wr_ndac_cs_o,
      wr_ext_clk_i           => lvt_clk_i,
      
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

      pcie_refclk_i          => pcie_clk_i,
      pcie_rstn_i            => mmc_pcie_rst_n_i,
      pcie_rx_i              => pcie_rx_i,
      pcie_tx_o              => pcie_tx_o,

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

    -- g_en_lcd
      lcd_scp_o              => dis_di_o(3),
      lcd_lp_o               => dis_di_o(1),
      lcd_flm_o              => dis_di_o(2),
      lcd_in_o               => dis_di_o(0)

  );


  p_counter: process(clk_sys)
  begin
    if rising_edge(clk_sys) then
	s_counter <= s_counter + 1;
    end if;
  end process;


  -- test mode select via hex switch or sw
  -- invert FPGA button and HEX switch
  s_test_sel(4)          <= s_gpio_out(7)          when s_gpio_out(8)='1' else not pbs_f_i;
  s_test_sel(3 downto 0) <= s_gpio_out(3 downto 0) when s_gpio_out(8)='1' else not hswf_i ;

 
  sfp_tx_dis_o <= '0'; -- SFP always enabled


  -- Display
  dis_wr_o    <= '0';
  dis_rst_o   <= '1';

  -- WR status LEDs 
  s_dis_led_green <= s_gpio_out(4) when s_gpio_out(8)='1' else (    s_led_link_up and     s_led_track); -- green
  s_dis_led_red   <= s_gpio_out(5) when s_gpio_out(8)='1' else (not s_led_link_up                    ); -- red
  s_dis_led_blue  <= s_gpio_out(6) when s_gpio_out(8)='1' else (    s_led_link_up and not s_led_track); -- blue
  
  -- display backlight color - pullups
  dis_di_o(4) <= '0' when s_dis_led_green = '1' else 'Z'; -- green
  dis_di_o(5) <= '0' when s_dis_led_red   = '1' else 'Z'; -- red
  dis_di_o(6) <= '0' when s_dis_led_blue  = '1' else 'Z'; -- blue
  
  -- Link LEDs
  s_led_status_monster(1) <= s_led_link_act and s_led_link_up;   -- red   = traffic/no-link
  s_led_status_monster(2) <= s_led_link_up;                      -- blue  = link
  s_led_status_monster(3) <= s_led_track;                        -- green = timing valid
  s_led_status_monster(4) <= s_led_pps;                          -- white = PPS

  -- GPIOs
  s_led_status_monster(5) <= mmc_pcie_rst_n_i; 
  -- different blinks according to which backplane buffers are enabled
  s_led_status_monster(6) <= (std_logic(s_counter(23)) and std_logic(s_counter(23))) when (s_mtca4_bpl_buff_en  = '0' and s_lib_trig_oe_sw_mmc = '1') else
                             (std_logic(s_counter(23)) or  std_logic(s_counter(23))) when (s_mtca4_bpl_buff_en  = '1' and s_lib_trig_oe_sw_mmc = '0') else '0';


  s_gpio_in(3 downto 0) <= not  hswf_i; -- FPGA HEX switch
  s_gpio_in(7 downto 4) <= con(4 downto 1); -- CPLD HEX switch

  s_gpio_in(8) <= not pbs_f_i; -- FPGA push button
  s_gpio_in(9) <= con(5);  -- CPLD push button


  -- status LED output according to FPGA hex switch position and fpga button
  -- F position - simple led test
  with s_test_sel select
    s_led_status <= "000000"               when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                    "111111"               when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                    s_led_status_monster    when others;         -- driven by monster

  led_status_o <= not s_led_status;                  

  -- USER LED output according to fpga hex switch position and fpga button                  
  -- F position - simple led test
  -- D position - show state of CPLD hex switch and button
  with s_test_sel select
    s_led_user <= x"00"                    when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                  x"FF"                    when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                  ("000" &     con)        when ('0' & x"D"),   -- FPGA hex sw in position D, button not pressed, CPLD HEX SW and button test  
                  ("000" & not con)        when ('1' & x"D"),   -- FPGA hex sw in position D, button     pressed, CPLD HEX SW and button test  
                  s_gpio_out(7 downto 0)   when others;         -- driven by monster

  led_user_o <= not s_led_user;

 

  -- enable LEMO output buffers (active LO)
  lvtio_oe_n_o <= not s_lvds_oe(4 downto 0);

  -- LEMO activity LEDs (active HI)
  s_lvds_led(4 downto 0) <= s_lvds_i_led(4 downto 0) or s_lvds_o_led(4 downto 0);
  
  -- LVDS termination pins (active hi)
  with s_test_sel select
    lvtio_term_en_o <= (others => '0')                 when ('0' & x"E"),   -- FPGA hex sw in position E, button not pressed, termination test
                       (others => '1')                 when ('1' & x"E"),   -- FPGA hex sw in position E, button     pressed, termination test
                        s_lvds_term_en(4 downto 0) when others;             -- driven by monster (enable termination when output disabled)

  -- LVDS direction indicator RED LEDs (active hi)
  with s_test_sel select
    lvtio_led_dir_o <= (others => '0')        when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '1')        when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
                        s_lvds_oe(4 downto 0) when others;         -- driven by monster

  -- LVDS activity indicator BLUE LEDs (active hi)
  with s_test_sel select
    lvtio_led_act_o <= (others => '0')        when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '1')        when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
                       s_lvds_led(4 downto 0) when others;         -- driven by monster

                       

  -- Logic analyzer
  -- inputs
  s_log_in(15 downto 0) <= hpw(15 downto 0);
  s_log_in(16)          <= hpwck;

  -- outputs
  hpwck                 <= s_log_out(16) when s_log_oe(16) = '1' else 'Z';
  hpw_out : for i in 0 to 15 generate
    hpw(i)               <= s_log_out(i) when s_log_oe(i) = '1' else 'Z';  
  end generate;

  -----------------------------------------------------------
  -- lemo io connectors on front panel to/from monster
  -----------------------------------------------------------
  -- lvds/lvttl lemos in/out
  s_lvds_p_i(4 downto 0) <= lvtio_in_p_i(5 downto 1);
  s_lvds_n_i(4 downto 0) <= lvtio_in_n_i(5 downto 1);

  lvtio_out_p_o(5 downto 1)   <= s_lvds_p_o(4 downto 0);
  lvtio_out_n_o(5 downto 1)   <= s_lvds_n_o(4 downto 0);


  -- External white rabbit clock input
  lvt_in_clk_en_n_o <= not(s_wr_ext_in); 

  -----------------------------------------------------------
  -- microTCA.4 backplane triggers, inputs and outputs
  -----------------------------------------------------------

  -- select receiver input Type for onboard M-LVDS buffers to backplane
  -- ('0' = Type-1 , '1' = Type-2 )
  mlvdio_fsen_o <= '1'; 

  -- MTCA.4 bussed trigger lines (PORTS 17-20)
  s_lvds_n_i(12 downto 5) <= mlvdio_in_n_i(8 downto 1);
  s_lvds_p_i(12 downto 5) <= mlvdio_in_p_i(8 downto 1);

  mlvdio_out_n_o <= s_lvds_n_o(12 downto 5);
  mlvdio_out_p_o <= s_lvds_p_o(12 downto 5);

  ----------------------------------------
  -- MTCA.4 clocks (TCLKA, TCLKB, TCLKC, TCLKD)
  s_lvds_n_i(16 downto 13) <= tclk_in_n_i;
  s_lvds_p_i(16 downto 13) <= tclk_in_p_i;

  tclk_out_n_o <= s_lvds_n_o(16 downto 13);
  tclk_out_p_o <= s_lvds_p_o(16 downto 13);

  -----------------------------------------------------------
  -- Libera outputs
  -----------------------------------------------------------

  -- no intputs from Libera backplane, outputs only
  -- trigger outputs to backplane for Libera
  -- connected directly to monster
  -- Libera triggers (only outputs)
  lib_trig_n_o <= s_lvds_n_o(20 downto 17);
  lib_trig_p_o <= s_lvds_p_o(20 downto 17);



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
          dataout   => open
        );
  end generate;


  ----------------------------------------------
  fpga2mmc_int_o  <= '0'; -- irq to mmc

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
  s_monster_mlvd_dir   (8 downto 1) <= s_lvds_oe(16 downto 9);
  s_monster_hss_buf_en (4 downto 1) <= (others => '0');

  -----------------------------------------------------------------------
  -- backplane ports configuration from MMC
  ----------------------------------------------------------------------- 
  -- bpl buffer enable generation depends on the crate in wich AMC is (MTCA.0, MTCA.4, Libera)
  -- mmc2fpga_usr_i(1): 0 - we are not in Libera, 1 - we are in Libera
  s_libera_bpl_buff_en  <= '1' when mmc2fpga_usr_i(1) = '1' else '0';
  
  -----------------------------------------------------------------------
  -- lvds/lvds libera trigger buffers enable (active HI)
  -----------------------------------------------------------------------
  --  lib_trig_oe_o <=  s_libera_bpl_buff_en;
  -- before MMC is fixed this is the way to enble Libera trigger buffers
  -- USE THIS ONLY when FTRN is in Libera SLOT 8!!!
  s_lib_trig_oe_sw_mmc <= '1' when (s_gpio_out(3 downto 0) = "0111" and s_gpio_out(8)='1') 
                          else s_libera_bpl_buff_en; 
  -- cross check with MTCA.4 buffer enable
  lib_trig_oe_o        <= '1' when (s_mtca4_bpl_buff_en = '0' and s_lib_trig_oe_sw_mmc = '1') else '0';

  -----------------------------------------------------------------------
  -- lvds/m-lvds MTCA.4 buffers enable generation
  -----------------------------------------------------------------------
  -- m-lvds buffer powerdown (active low) (0 - powered down, 1 - powered up)
  -- enabled only in MTCA.4 by SW
  s_mtca4_bpl_buff_en   <= '1' when (s_gpio_out(3 downto 0) = "1001" and s_gpio_out(8)='1') else '0';
  -- crosscheck with Libera buffer enable
  mlvdio_pd_n_o    <= '1' when (s_mtca4_bpl_buff_en = '1' and s_lib_trig_oe_sw_mmc = '0')  else '0'; 

  gen_mlvd_buf_oe : for i in  1 to 8 generate
    -- enable buffer output towards BACKPLANE (m-lvds driver enable, active hi)
    mlvdio_de_o(i) <= '1' when (s_mtca4_bpl_buff_en       = '1' and 
                                s_monster_mlvd_buf_en(i)  = '1' and 
                                s_monster_mlvd_dir(i)     = '1')
                      else '0';
                               
    -- enable buffer output towards FPGA (m-lvds receiver enable, active low)
    mlvdio_re_n_o(i) <=  '0' when (s_mtca4_bpl_buff_en       = '1' and 
                                   s_monster_mlvd_buf_en(i)  = '0' and 
                                   s_monster_mlvd_dir(i)     = '0')
                          else '1';

  end generate; -- gen_mlvd_buf_oe


  -----------------------------------------------------------------------
  -- lvds/lvds MTCA.4 backplane clock buffers enable, direction generation
  -----------------------------------------------------------------------
  gen_tclk_oe_dir : for i in  1 to 4 generate
    -- enable clock switch outputs towards BACKPLANE (switch enable, active lo)
    tclk_en_n_o(i) <= '0' when (s_mtca4_bpl_buff_en   = '1' and 
                                s_monster_tclk_en(i)  = '1' and 
                                s_monster_tclk_dir(i) = '1')
                           else '1';
  
    -- enable clock buffer towards FPGA, switch clock mux to C-A (active hi)
    tclk_dir_o(i)<= '1' when (s_mtca4_bpl_buff_en   = '1' and 
                              s_monster_tclk_en(i)  = '0' and 
                              s_monster_tclk_dir(i) = '0')
                          else '0';
  end generate; -- gen_tclk_oe_dir

  -----------------------------------------------------------------------
  -- MTCA.4 PORT 12-15 buffers enable generation (active HI)
  -----------------------------------------------------------------------
  -- currently not used, keep disabled
  gen_hss_buf_oe : for i in  1 to 4 generate
    hss_tx_en_o(i) <= '0';    
    hss_rx_en_o(i) <= '0';    
  end generate; -- gen_hss_buf_oe

  -- disable  Transmit Pre-Emphasis and Receive Equalization
  hss_tx_pe_en_o <= '0';
  hss_rx_eq_en_o <= '0';

end rtl;

