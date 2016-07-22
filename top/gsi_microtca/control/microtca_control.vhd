library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.altera_lvds_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.spi_slave_pkg.all;
use work.microtca_ctrl_auto_pkg.all;


entity microtca_control is
--  generic(
--    g_top_lvds_inout_front : natural := 5; -- front end lemos(5)
--    g_top_lvds_tclk_mtca   : natural := 4; -- TCLK
--    g_top_lvds_inout_mtca  : natural := 8; -- MicroTCA.4 backplane triggers/gates/clocks(8)
--    g_top_lvds_out_libera  : natural := 4  -- Libera backplane triggers (4) 
--  );
  port(
    clk_20m_vcxo_i      : in std_logic;  -- 20MHz VCXO clock

    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference - (clk_125m_wrpll_0  on schl)
    clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oszillator (clk_osc_1  on sch)
    sfp234_ref_clk_i  : in std_logic; -- SFP clk (clk_125m_wrpll_1 on sch)
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
    hss_tx_n_o  	      : out std_logic_vector(4 downto 1);
    hss_tx_p_o  	      : out std_logic_vector(4 downto 1);
    -- enable hss buffer outputs to/from FPGA 
    hss_tx_en_o         : out std_logic_vector(4 downto 1);
    hss_rx_en_o         : out std_logic_vector(4 downto 1);
    
    -- enable Receive Equalization and Transmit Pre-Emphasis
    hss_tx_pe_en_o      : out std_logic;
    hss_rx_eq_en_o      : out std_logic;
	 
    -----------------------------------------------------------------------
    -- mmc > fpga spi bus, mmc is master
    -----------------------------------------------------------------------l
		mmc_spi0_sck_i	        : in  std_logic;
		mmc_spi0_miso_o 	      : out std_logic;
		mmc_spi0_mosi_i 	      : in  std_logic;
		mmc_spi0_sel_fpga_n_i   : in  std_logic;

 		mmc_pcie_en_i	          : in  std_logic;
    mmc_pcie_rst_n_i        : in  std_logic;

		mmc2fpga_usr_i	        : in  std_logic_vector(2 downto 1);
		fpga2mmc_int_o	        : out std_logic; -- interrupt to mmc

    mmc_quiesce_out_i       : in  std_logic; -- mmc alert to fpga that amc will be powered off
    mmc_quiesce_in_o        : out std_logic; -- fpga reply to mmc that is ready for power down

    mmc_i2c_sda_io          : inout std_logic; -- mmc's I2C bus
    mmc_i2c_clk_i           : in    std_logic; -- mmc's I2C bus

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

  signal clk_sys       : std_logic;
  
  signal s_led_status_moster  : std_logic_vector(6 downto 1);
  signal s_led_user_monster   : std_logic_vector(8 downto 1);

  signal s_led_status         : std_logic_vector(6 downto 1);
  signal s_led_user           : std_logic_vector(8 downto 1);
 
  signal s_gpio               : std_logic_vector(7 downto 0);
  
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

  signal s_lvds_p_o     : std_logic_vector(20 downto 0);
  signal s_lvds_n_o     : std_logic_vector(20 downto 0);
  signal s_lvds_o_led   : std_logic_vector(20 downto 0);
  signal s_lvds_oe      : std_logic_vector(20 downto 0);
  signal s_lvds_oen_monster   : std_logic_vector(20 downto 0);

  signal s_lvds_led     : std_logic_vector(20 downto 0);


  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 28) := 
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1_USR_R ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_USR_B ", IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_USR_G ", IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_USR_W ", IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED5_USR_R ", IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED6_USR_B ", IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED7_USR_G ", IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED8_USR_W ", IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),

    ("LEMO_IO1   ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO2   ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO3   ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO4   ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LEMO_IO5   ", IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),

    ("MTCA4_CLK1 ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_CLK2 ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_CLK3 ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_CLK4 ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),

    ("MTCA4_IO1  ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO2  ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO3  ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO4  ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO5  ", IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO6  ", IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO7  ", IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),
    ("MTCA4_IO8  ", IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVDS),

    ("LIBERA_TR1 ", IO_NONE,         false,   false,  0,     IO_OUTPUT  , IO_LVDS,  true,         true,        IO_LVDS),
    ("LIBERA_TR2 ", IO_NONE,         false,   false,  1,     IO_OUTPUT  , IO_LVDS,  true,         true,        IO_LVDS),
    ("LIBERA_TR3 ", IO_NONE,         false,   false,  2,     IO_OUTPUT  , IO_LVDS,  true,         true,        IO_LVDS),
    ("LIBERA_TR4 ", IO_NONE,         false,   false,  3,     IO_OUTPUT  , IO_LVDS,  true,         true,        IO_LVDS)

  );

  
  constant c_family  : string := "Arria V"; 
  constant c_project : string := "microtca_control";
  constant c_cores      : natural:= 1;
  constant c_initf_name : string := c_project & ".mif";
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

  -- backplane configuration settings from microtca_ctrl module
  signal s_mtca_bpl_conf0  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf1  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf2  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf3  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf4  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf5  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf6  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_conf7  : std_logic_vector(31 downto 0);

  -- backplane configuration status
  signal s_mtca_bpl_stat0  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat1  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat2  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat3  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat4  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat5  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat6  : std_logic_vector(31 downto 0);
  signal s_mtca_bpl_stat7  : std_logic_vector(31 downto 0);


  -- connections from microtca_ctrl registers
  signal s_monster_tclk_en  : std_logic_vector(8 downto 1);
  signal s_monster_tclk_dir     : std_logic_vector(8 downto 1);

  signal s_monster_mlvd_buf_en  : std_logic_vector(8 downto 1);
  signal s_monster_mlvd_dir     : std_logic_vector(8 downto 1);

  signal s_monster_hss_buf_en   : std_logic_vector(8 downto 1);

  -- registers written by mmc
  signal s_mmc_libera_buf_en_reg  : std_logic_vector(7 downto 0);
  signal s_mmc_mtca4_bpl_dis_reg  : std_logic_vector(7 downto 0);


  
  
begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_lvds_inout      => 17,  -- 5 LEMOs at front panel, 8 MTCA4 at BPL, 4 hss at BPL, 4 MTCA4 clk at BPL
      g_lvds_in         => 0,
      g_lvds_out        => 4,   -- 4 libera triggers at BPL
      g_gpio_out        => 8,  -- 8 on-boards LEDs
      g_en_usb          => true,
      g_en_lcd          => true,
      g_io_table        => io_mapping_table,
      g_en_mtca_ctrl     => true,
      g_lm32_cores      => c_cores,
      g_lm32_ramsizes   => c_lm32_ramsizes/4,
      g_lm32_init_files => f_string_list_repeat(c_initf_name & ".mif", c_cores),
      g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
)
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_rstn_i            => fpga_res,
      clk_sys_o              => clk_sys,
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
      
      gpio_o                 => s_gpio,

      lvds_p_i               => s_lvds_p_i,
      lvds_n_i               => s_lvds_n_i,
      lvds_i_led_o           => s_lvds_i_led,

      lvds_p_o               => s_lvds_p_o,
      lvds_n_o               => s_lvds_n_o,
      lvds_o_led_o           => s_lvds_o_led,
      lvds_oen_o             => s_lvds_oen_monster,
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
      lcd_in_o               => dis_di_o(0),

      mtca_ctrl_hs_i         => hswf_i,
      mtca_pb_i              => pbs_f_i,
      mtca_ctrl_hs_cpld_i    => con(4 downto 1),
      mtca_pb_cpld_i         => con(5),
      mtca_clk_oe_o          => s_wr_ext_in,
      mtca_log_oe_o          => s_log_oe,
      mtca_log_out_o         => s_log_out,
      mtca_log_in_i          => s_log_in,

      mtca_backplane_conf0_o  => s_mtca_bpl_conf0,
      mtca_backplane_conf1_o  => s_mtca_bpl_conf1,
      mtca_backplane_conf2_o  => s_mtca_bpl_conf2,
      mtca_backplane_conf3_o  => s_mtca_bpl_conf3,
      mtca_backplane_conf4_o  => s_mtca_bpl_conf4,
      mtca_backplane_conf5_o  => s_mtca_bpl_conf5,
      mtca_backplane_conf6_o  => s_mtca_bpl_conf6,
      mtca_backplane_conf7_o  => s_mtca_bpl_conf7,


      mtca_backplane_stat0_i  => s_mtca_bpl_stat0,
      mtca_backplane_stat1_i  => s_mtca_bpl_stat1,
      mtca_backplane_stat2_i  => open,
      mtca_backplane_stat3_i  => open,
      mtca_backplane_stat4_i  => open,
      mtca_backplane_stat5_i  => open,
      mtca_backplane_stat6_i  => open,
      mtca_backplane_stat7_i  => open
  );
 
  sfp_tx_dis_o <= '0'; -- SFP always enabled


  -- Display
  dis_wr_o    <= '0';
  dis_rst_o   <= '1';
  
  -- WR LEDs
  dis_di_o(5) <= '0' when (not s_led_link_up)                     = '1' else 'Z'; -- red
  dis_di_o(6) <= '0' when (    s_led_link_up and not s_led_track) = '1' else 'Z'; -- blue
  dis_di_o(4) <= '0' when (    s_led_link_up and     s_led_track) = '1' else 'Z'; -- green
  
  -- Link LEDs
  s_led_status_moster(1) <= s_led_link_act and s_led_link_up;   -- red   = traffic/no-link
  s_led_status_moster(2) <= s_led_link_up;                      -- blue  = link
  s_led_status_moster(3) <= s_led_track;                        -- green = timing valid
  s_led_status_moster(4) <= s_led_pps;                          -- white = PPS

  -- GPIOs
  s_led_status_moster(6 downto 5) <= s_gpio (1 downto 0);

  -- invert FPGA button and HEX switch
  s_test_sel(4)          <= not pbs_f_i;
  s_test_sel(3 downto 0) <= not hswf_i;

  -- status LED output according to FPGA hex switch position and fpga button
  -- F position - simple led test
  with s_test_sel select
    s_led_status <= "101010"               when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                    "010101"               when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                    s_led_status_moster    when others;         -- driven by monster

  led_status_o <= not s_led_status;                  

  -- USER LED output according to fpga hex switch position and fpga button                  
  -- F position - simple led test
  -- D position - show state of CPLD hex switch and button
  with s_test_sel select
    s_led_user <= x"AA"                    when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, led test
                  x"55"                    when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, led test
                  ("000" &     con)        when ('0' & x"D"),   -- FPGA hex sw in position D, button not pressed, CPLD HEX SW and button test  
                  ("000" & not con)        when ('1' & x"D"),   -- FPGA hex sw in position D, button     pressed, CPLD HEX SW and button test  
                  s_gpio                   when others;         -- driven by monster

  led_user_o <= not s_led_user;

  

  

  -- LVDS output enable pins (active low)
  s_lvds_oe <= not s_lvds_oen_monster;
  
  lvtio_oe_n_o <= s_lvds_oen_monster(4 downto 0);

  s_lvds_led(s_lvds_i_led'range) <= s_lvds_i_led or s_lvds_o_led(s_lvds_i_led'range);
  
  -- LVDS termination pins (active hi)
  with s_test_sel select
    lvtio_term_en_o <= (others => '0')                 when ('0' & x"E"),   -- FPGA hex sw in position E, button not pressed, termination test
                       (others => '1')                 when ('1' & x"E"),   -- FPGA hex sw in position E, button     pressed, termination test
                        s_lvds_oen_monster(4 downto 0) when others;         -- driven by monster (enable termination when output disabled)

  -- LVDS direction indicator RED LEDs (active hi)
  with s_test_sel select
    lvtio_led_dir_o <= (others => '0')        when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '1')        when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
                        s_lvds_oe(4 downto 0) when others;         -- driven by monster

  -- LVDS activity indicator BLUE LEDs (active hi)
  with s_test_sel select
    lvtio_led_act_o <= (others => '1')        when ('0' & x"F"),   -- FPGA hex sw in position F, button not pressed, LED test
                       (others => '0')        when ('1' & x"F"),   -- FPGA hex sw in position F, button     pressed, LED test
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
  -- lemo io connectors on front panel
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

  -- select reciver input Type for onboard M-LVDS buffers to backplane
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

  -- Libera triggers (only outputs
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



  -----------------------------------------------------------
  -- Libera outputs
  -----------------------------------------------------------

  -- no intputs from Libera backplane, outputs only
  -- trigger outputs to backplane for Libera
  -- connected directly to monster

  ----------------------------------------------
  fpga2mmc_int_o  <= '0'; -- irq to mmc

  -- mmc alerts fpga that amc will be powered off and
  -- fpga replies to mmc that is ready for power down.
  -- If needed, response to mmc can be delayed 
  -- (for example to finish writing to flash or to complete data transfer, etc)
  mmc_quiesce_in_o  <= mmc_quiesce_out_i;        
  


  -----------------------------------------------------------------------
  -- backplane ports configuration from monster microtca_ctrl registers
  ----------------------------------------------------------------------- 
  s_monster_tclk_en       <= s_mtca_bpl_conf0(s_monster_tclk_en'range);
  s_monster_tclk_dir      <= s_mtca_bpl_conf1(s_monster_tclk_dir'range);
  s_monster_mlvd_buf_en   <= s_mtca_bpl_conf2(s_monster_mlvd_buf_en'range);
  s_monster_mlvd_dir      <= s_mtca_bpl_conf3(s_monster_mlvd_dir'range);
  s_monster_hss_buf_en    <= s_mtca_bpl_conf4(s_monster_hss_buf_en'range);

  -----------------------------------------------------------------------
  -- backplane ports configuration status to monster microtca_ctrl registers
  ----------------------------------------------------------------------- 
  s_mtca_bpl_stat0(s_mmc_libera_buf_en_reg'range) <= s_mmc_libera_buf_en_reg;
  s_mtca_bpl_stat1(s_mmc_mtca4_bpl_dis_reg'range) <= s_mmc_mtca4_bpl_dis_reg;

  
  -----------------------------------------------------------------
  -- SPI slave module connected to MMC via SPI
  ----------------------------------------------------------------------- 
  -- enables reading of the microtca_ctrl module registers state
  -- and mmc Libera B trigger enable register

  mmc_spi : spi_slave
      generic map (   
        N 		    => 16,        -- 16bit serial word length
        CPOL 	    => '0',       -- SPI mode selection (mode 0 default)
        CPHA 	    => '0',       -- CPOL = clock polarity, CPHA = clock phase.
        PREFETCH  => 3)         -- prefetch lookahead cycles
      port map(  
        clk_i 			  => clk_sys,               -- internal interface clock (clocks di/do registers)
        -- spi pins
        spi_ssel_i 		=> mmc_spi0_sel_fpga_n_i, -- spi bus slave select line
        spi_sck_i 		=> mmc_spi0_sck_i,        -- spi bus sck clock (clocks the shift register core)
        spi_mosi_i 		=> mmc_spi0_mosi_i,       -- spi bus mosi input
        spi_miso_o 		=> mmc_spi0_miso_o,       -- spi bus spi_miso_o output
        
        -- internal interface
        di_req_o 		  => s_mmcspi_di_req,     -- preload lookahead data request line
        di_i 			    => s_mmcspi_di,         -- parallel load data in (clocked in on rising edge of clk_i)
        wren_i 			  => s_mmcspi_wren,       -- user data write enable
        wr_ack_o 		  => s_mmcspi_di_wrack,   -- write acknowledge
        do_valid_o 		=> s_mmcspi_do_valid,   -- do_o data valid strobe, valid during one clk_i rising edge.
        do_o 			    => s_mmcspi_do,         -- parallel output (clocked out on falling clk_i)
        
        -- debug ports: can be removed for the application circuit
        do_transfer_o 	=> open,    -- debug: internal transfer driver
        wren_o 			    => open,    -- debug: internal state of the wren_i pulse stretcher
        rx_bit_next_o 	=> open,    -- debug: internal rx bit
        state_dbg_o 	  => open,    -- debug: internal state register
        sh_reg_dbg_o 	  => open     -- debug: internal shift register
      );                      

  s_mmcspi_wren <= s_mmcspi_di_req;      
      
  -----------------------------------------------------------------------
  -- mmc read data select
  -- addresses are defined in microtca_ctrl module
  ----------------------------------------------------------------------- 
  with a_mmcspi_addr select    
    s_mmcspi_di(7 downto 0) <= 
        s_monster_tclk_en       when std_logic_vector(to_unsigned(c_slave_BACKPLANE_CONF0_RW, a_mmcspi_addr'length)),
        s_monster_tclk_dir      when std_logic_vector(to_unsigned(c_slave_BACKPLANE_CONF1_RW, a_mmcspi_addr'length)),
        s_monster_mlvd_buf_en   when std_logic_vector(to_unsigned(c_slave_BACKPLANE_CONF2_RW, a_mmcspi_addr'length)),
        s_monster_mlvd_dir      when std_logic_vector(to_unsigned(c_slave_BACKPLANE_CONF3_RW, a_mmcspi_addr'length)),
        s_monster_hss_buf_en    when std_logic_vector(to_unsigned(c_slave_BACKPLANE_CONF4_RW, a_mmcspi_addr'length)),
        
        s_mmc_libera_buf_en_reg when std_logic_vector(to_unsigned(c_slave_BACKPLANE_STAT0_GET, a_mmcspi_addr'length)),
        s_mmc_mtca4_bpl_dis_reg when std_logic_vector(to_unsigned(c_slave_BACKPLANE_STAT1_GET, a_mmcspi_addr'length)),
       (others => '0')          when others;

  s_mmcspi_di(15 downto 8) <= (others => '0');      

  
  s_rstn_mmc_spi <= '1';
  s_clk_mmc_spi  <= clk_sys;
  
  -----------------------------------------------------------------------
  -- mmc data write to local registers
  ----------------------------------------------------------------------- 
  -- controlled from MMC via SPI - MCH/MMC enable of Libera triggers and 
  -- disable for MTCA.4 triggers, clocks and HSS links
  p_mmc_write_reg : process(s_clk_mmc_spi)
  begin
    if rising_edge(s_clk_mmc_spi) then
      if s_rstn_mmc_spi = '0' then
        s_mmc_mtca4_bpl_dis_reg    <= (others => '0');
        s_mmc_libera_buf_en_reg    <= (others => '1');
      else
        -- store settings given by mmc
        -- for now simply invert libera trigger enable setting
        -- this prevents connecting MTCA.4 ports to Libera B backplane
        -- and Libera triggers to MTCA.4 backplane
        if s_mmcspi_do_valid = '1' and a_mmcspi_we = '1' then 
          s_mmc_mtca4_bpl_dis_reg  <= not s_mmcspi_do(s_mmc_mtca4_bpl_dis_reg'range);
        else -- hold
          s_mmc_mtca4_bpl_dis_reg  <= s_mmc_mtca4_bpl_dis_reg;
        end if;
        -- store settings given by mmc
        if s_mmcspi_do_valid = '1' and a_mmcspi_we = '1' then 
          s_mmc_libera_buf_en_reg  <= s_mmcspi_do(s_mmc_libera_buf_en_reg'range);
        else -- hold
          s_mmc_libera_buf_en_reg  <= s_mmc_libera_buf_en_reg;
        end if;
        
      end if; -- reset
    end if; -- clk
  end process p_mmc_write_reg;  
  

  -----------------------------------------------------------------------
  -- lvds/lvds libera trigger buffers enable generation
  -----------------------------------------------------------------------
  lib_trig_oe_o <= s_mmc_libera_buf_en_reg(0);

  -----------------------------------------------------------------------
  -- lvds/m-lvds MTCA.4 buffers enable generation
  -----------------------------------------------------------------------
  gen_mlvd_buf_oe : for i in  1 to 8 generate
    -- enable buffer output towards BACKPLANE (m-lvds driver enable, active hi)
    mlvdio_de_o(i) <= '1' when (s_mmc_mtca4_bpl_dis_reg(0)  = '0' and 
                                s_monster_mlvd_buf_en(i)    = '1' and 
                                s_monster_mlvd_dir(i)       = '1')
                           else '0';
                               
    -- enable buffer output towards FPGA (m-lvds receiver enable, active low)
    mlvdio_re_n_o(i) <= '0' when (s_mmc_mtca4_bpl_dis_reg(0)  = '0' and 
                                  s_monster_mlvd_buf_en(i)    = '1' and 
                                  s_monster_mlvd_dir(i)       = '0')
                             else '1';
  end generate; -- gen_mlvd_buf_oe

  -- m-lvds buffer powerdown, active low
  -- when in Libera or when not enabled from monster
  mlvdio_pd_n_o    <= '0' when (s_mmc_mtca4_bpl_dis_reg(0)  = '1' or 
                                s_monster_mlvd_buf_en       = x"00")
                           else '1'; 

  -----------------------------------------------------------------------
  -- lvds/lvds MTCA.4 backplane clock buffers enable, direction generation
  -----------------------------------------------------------------------
  gen_tclk_oe_dir : for i in  1 to 4 generate
    -- enable clock switch outputs towards BACKPLANE (switch enable, active lo)
    tclk_en_n_o(i) <= '0' when (s_mmc_mtca4_bpl_dis_reg(0)  = '0' and 
                                s_monster_tclk_en(i)        = '1' and 
                                s_monster_tclk_dir(i)       = '1')
                           else '1';
  
    -- enable clock buffer towards FPGA, switch clock mux to C-A (active hi)
    tclk_dir_o(i)<= '1' when (s_mmc_mtca4_bpl_dis_reg(0) = '0' and 
                              s_monster_tclk_en(i)       = '1' and 
                              s_monster_tclk_dir(i)      = '0')
                          else '0';
  end generate; -- gen_tclk_oe_dir

  -----------------------------------------------------------------------
  -- MTCA.4 PORT 12-15 buffers enable generation
  -----------------------------------------------------------------------
  gen_hss_buf_oe : for i in  1 to 4 generate

    hss_tx_en_o(i) <= '1' when (s_mmc_mtca4_bpl_dis_reg(0) = '0' and 
                                 s_monster_hss_buf_en(i)   = '1')
                               else '0';    
    hss_rx_en_o(i) <= '1' when (s_mmc_mtca4_bpl_dis_reg(0) = '0' and 
                                 s_monster_hss_buf_en(i+4) = '1')
                               else '0';    
  end generate; -- gen_hss_buf_oe

  -- disable  Transmit Pre-Emphasis and Receive Equalization
  hss_tx_pe_en_o <= '0';
  hss_rx_eq_en_o <= '0';



end rtl;

