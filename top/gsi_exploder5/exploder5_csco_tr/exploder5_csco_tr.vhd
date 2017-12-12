library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity exploder5_csco_tr is
  port(
    -- PLL clock inputs
    clk_20m_vcxo_i    : in    std_logic; -- AF21
    clk_125m_pllref_i : in    std_logic; -- G15
    clk_125m_local_i  : in    std_logic; -- C22
    
    -- Transceiver clock inputs
--    clk_osc_i         : in    std_logic; -- unused (could be useful for management PHY)
    clk_sfp_i         : in    std_logic;
    pcie_refclk_i     : in    std_logic;
    
    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_rx_i         : in    std_logic_vector(3 downto 0); -- !!! stole SFP 1+2 for testing
    pcie_tx_o         : out   std_logic_vector(3 downto 0);
    nPCI_RESET        : in    std_logic;
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk          : out   std_logic;
    dac_din           : out   std_logic;
    ndac_cs           : out   std_logic_vector(2 downto 1);
  
    ------------------------------------------------------------------------
    -- Additional WREX/WR1 pins
    ------------------------------------------------------------------------
    wr1_nc1           : inout std_logic;
    wr1_nc4           : inout std_logic;
    wr1_nc5           : inout std_logic;
    wr1_nc6           : inout std_logic;
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd              : out   std_logic;
    slwr              : out   std_logic;
    fd                : inout std_logic_vector(7 downto 0);
    pa                : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl               : in    std_logic_vector(2 downto 0);
    uclk              : in    std_logic;
    ures              : out   std_logic;
    uclkin            : in    std_logic;
    
    -----------------------------------------------------------------------
    -- SRAM (with DDR hidden inside)
    -----------------------------------------------------------------------
    sram_a            : out   std_logic_vector(23 downto 0) := (others => 'Z');
    sram_dq           : inout std_logic_vector(15 downto 0) := (others => 'Z');
    sram_clk          : out   std_logic := 'Z';
    sram_advn         : out   std_logic := 'Z';
    sram_cre          : out   std_logic := 'Z';
    sram_cen          : out   std_logic := 'Z';
    sram_oen          : out   std_logic := 'Z';
    sram_wen          : out   std_logic := 'Z';
    sram_be0          : out   std_logic := 'Z';
    sram_be1          : out   std_logic := 'Z';
    sram_wait         : in    std_logic; -- DDR magic
    
    -----------------------------------------------------------------------
    -- SD-CARD
    -----------------------------------------------------------------------
    sd_dat0           : inout std_logic := 'Z';
    sd_dat1           : inout std_logic := 'Z';
    sd_dat2           : inout std_logic := 'Z';
    sd_dat3           : inout std_logic := 'Z';
    sd_cmd            : inout std_logic := 'Z';
    sd_clk            : inout std_logic := 'Z';
    sd_switch_a       : inout std_logic := 'Z';
    sd_switch_b       : inout std_logic := 'Z';
    
    -----------------------------------------------------------------------
    -- misc base board IO: leds, cpld, 1wire
    -----------------------------------------------------------------------
    led_o             : out   std_logic_vector( 8 downto 1) := (others => '1');
    con_io            : inout std_logic_vector( 9 downto 1) := (others => 'Z'); -- unused
    nres_o            : in    std_logic; -- unused ("con_io(6)")
    fpga_res_i        : in    std_logic; -- reset from CPLD
    rom_data_io       : inout std_logic;
    
    -----------------------------------------------------------------------
    -- SFP1  
    -----------------------------------------------------------------------
    
    sfp1_tx_disable_o : out   std_logic := '1';
    sfp1_tx_fault     : in    std_logic;
    sfp1_los          : in    std_logic;
    
--  sfp1_txp_o        : out std_logic;          -- U24+U23
--  sfp1_rxp_i        : in  std_logic;          -- V26+V25
    
    sfp1_mod0         : in    std_logic;
    sfp1_mod1         : inout std_logic;
    sfp1_mod2         : inout std_logic;
    
    sfp1_red_o        : out   std_logic := 'Z';
    sfp1_green_o      : out   std_logic := 'Z';
    
    -----------------------------------------------------------------------
    -- SFP2
    -----------------------------------------------------------------------
    
    sfp2_tx_disable_o : out std_logic := '1';
    sfp2_tx_fault     : in  std_logic;
    sfp2_los          : in  std_logic;
    
--  sfp2_txp_o        : out std_logic;          -- R24+R23
--  sfp2_rxp_i        : in  std_logic;          -- T26+T25
     
    sfp2_mod0         : in    std_logic;
    sfp2_mod1         : inout std_logic;
    sfp2_mod2         : inout std_logic;
    
    sfp2_red_o        : out   std_logic := 'Z';
    sfp2_green_o      : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- SFP3 
    -----------------------------------------------------------------------
       
    sfp3_tx_disable_o : out   std_logic := '1';
    sfp3_tx_fault     : in    std_logic;
    sfp3_los          : in    std_logic;
    
--  sfp3_txp_o        : out std_logic;          -- J24+J23
--  sfp3_rxp_i        : in  std_logic;          -- K26+K25
    
    sfp3_mod0         : in    std_logic;
    sfp3_mod1         : inout std_logic;
    sfp3_mod2         : inout std_logic;
    
    sfp3_red_o        : out   std_logic := 'Z';
    sfp3_green_o      : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- SFP4 
    -----------------------------------------------------------------------
    
    sfp4_tx_disable_o : out   std_logic := '1';
    sfp4_tx_fault     : in    std_logic;
    sfp4_los          : in    std_logic;
    
    sfp4_txp_o        : out   std_logic;        -- E24+E23
    sfp4_rxp_i        : in    std_logic;        -- F26+F25
    
    sfp4_mod0         : in    std_logic;
    sfp4_mod1         : inout std_logic;
    sfp4_mod2         : inout std_logic;

    sfp4_red_o        : out   std_logic := 'Z';
    sfp4_green_o      : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- Daughter board
    -----------------------------------------------------------------------
    lemo_p_i          : in    std_logic_vector(8 downto 1);
    lemo_n_i          : in    std_logic_vector(8 downto 1);
    lemo_p_o          : out   std_logic_vector(8 downto 1);
    lemo_n_o          : out   std_logic_vector(8 downto 1);
    lemo_oen_o        : out   std_logic_vector(8 downto 1);
    lemo_term_o       : out   std_logic_vector(8 downto 1);
    lemo_oe_ledn_o    : out   std_logic_vector(8 downto 1);
    lemo_act_ledn_o   : out   std_logic_vector(8 downto 1);
    
    lvds_p_i          : in    std_logic_vector(3 downto 1);
    lvds_n_i          : in    std_logic_vector(3 downto 1);
    lvds_i_ledn_o     : out   std_logic_vector(3 downto 1);
    lvds_clk_p_i      : in    std_logic;
--    lvds_clk_n_i      : in    std_logic;
    
    lvds_p_o          : out   std_logic_vector(3 downto 1);
    lvds_n_o          : out   std_logic_vector(3 downto 1);
    lvds_o_ledn_o     : out   std_logic_vector(3 downto 1);
    lvds_clk_p_o      : out   std_logic;
--    lvds_clk_n_o      : out   std_logic;
    
    button_i          : in    std_logic_vector(4 downto 1);
    
    pe_smdat          : inout std_logic; -- unused (needed for CvP)
    pe_smclk          : out   std_logic := 'Z';
    pe_waken          : out   std_logic := 'Z';
     
    dsp_csn_o         : out   std_logic;
    dsp_resn_o        : out   std_logic;
    dsp_dcn_o         : out   std_logic;
    dsp_d1_o          : out   std_logic;
    dsp_d0_o          : out   std_logic;
    
    aud_spi_csb_o     : out   std_logic;
    aud_spi_sclk_o    : out   std_logic;
    aud_spi_sdio_o    : out   std_logic;
    aud_iis_fs_o      : out   std_logic;
    aud_iis_bclk_o    : out   std_logic;
    aud_iis_adcout_o  : out   std_logic;
    aud_iis_dacin_i   : in    std_logic;
    
    rs232_rts_i       : in    std_logic;
    rs232_cts_o       : out   std_logic := '1';
    rs232_tx_o        : out   std_logic;
    rs232_rx_i        : in    std_logic;

    db_rom_data_io    : inout std_logic);
end exploder5_csco_tr;

architecture rtl of exploder5_csco_tr is

  constant c_psram_bits : natural := 24;

  signal led_link_up  : std_logic;
  signal led_link_act : std_logic;
  signal led_track    : std_logic;
  signal led_pps      : std_logic;
  
  signal s_gpio_o       : std_logic_vector( 4 downto 1);
  signal s_gpio_i       : std_logic_vector( 4 downto 1);
  signal s_lvds_p_i     : std_logic_vector(11 downto 1);
  signal s_lvds_n_i     : std_logic_vector(11 downto 1);
  signal s_lvds_i_led   : std_logic_vector(11 downto 1);
  signal s_lvds_p_o     : std_logic_vector(11 downto 1);
  signal s_lvds_n_o     : std_logic_vector(11 downto 1);
  signal s_lvds_o_led   : std_logic_vector(11 downto 1);
  signal s_lvds_oen     : std_logic_vector( 8 downto 1);
  signal s_lvds_term    : std_logic_vector(11 downto 1);
  
  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 24) := 
  (
  -- Name[11 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1       ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2       ",  IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3       ",  IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4       ",  IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("BUT1       ",  IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("BUT2       ",  IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("BUT3       ",  IO_NONE,         false,   false,  2,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("BUT4       ",  IO_NONE,         false,   false,  3,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("IO1        ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO2        ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO3        ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO4        ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO5        ",  IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO6        ",  IO_NONE,         false,   false,  5,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO7        ",  IO_NONE,         false,   false,  6,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO8        ",  IO_NONE,         false,   false,  7,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("LVDSo1     ",  IO_NONE,         false,   false,  8,     IO_OUTPUT,   IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSo2     ",  IO_NONE,         false,   false,  9,     IO_OUTPUT,   IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSo3     ",  IO_NONE,         false,   false, 10,     IO_OUTPUT,   IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi1     ",  IO_NONE,         false,   false,  8,     IO_INPUT,    IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi2     ",  IO_NONE,         false,   false,  9,     IO_INPUT,    IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi3     ",  IO_NONE,         false,   false, 10,     IO_INPUT,    IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSoClk   ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_FIXED, false,        false,       IO_LVDS),
    ("WR200MHz   ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_FIXED, false,        false,       IO_LVDS),
    ("LVDSiClk   ",  IO_NONE,         false,   false,  0,     IO_INPUT,    IO_FIXED, false,        false,       IO_LVDS)
  );
  constant c_family       : string := "Arria V"; 
  constant c_project      : string := "exploder5_csco_tr";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & "_stub.mif";
  constant c_profile_name : string := "medium_icache_debug";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';' 

begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_psram_bits      => c_psram_bits,
      g_gpio_in         => 4,
      g_gpio_out        => 4,
      g_lvds_in         => 3,
      g_lvds_out        => 3,
      g_lvds_inout      => 8,
      g_fixed           => 3,
      g_en_pcie         => true,
      g_en_usb          => true,
      g_en_ssd1325      => true,
      g_en_nau8811      => true,
      g_en_psram        => true,
      g_en_user_ow      => true,
      g_en_tempsens     => true,
      g_io_table        => io_mapping_table,
      g_lm32_cores      => c_cores,
      g_lm32_ramsizes   => c_lm32_ramsizes/4,
      g_lm32_init_files => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => clk_sfp_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_clk_butis_o       => lvds_clk_p_o,
      core_rstn_i            => fpga_res_i,
      wr_onewire_io          => rom_data_io,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
      wr_dac_sclk_o          => dac_sclk,
      wr_dac_din_o           => dac_din,
      wr_ndac_cs_o           => ndac_cs,
      wr_ext_clk_i           => lvds_clk_p_i,
      wr_uart_o              => rs232_tx_o,
      wr_uart_i              => rs232_rx_i,      
      sfp_tx_disable_o       => open,
      sfp_tx_fault_i         => sfp4_tx_fault,
      sfp_los_i              => sfp4_los,
      gpio_o                 => s_gpio_o,
      gpio_i                 => s_gpio_i,
      lvds_p_i               => s_lvds_p_i,
      lvds_n_i               => s_lvds_n_i,
      lvds_i_led_o           => s_lvds_i_led,
      lvds_p_o               => s_lvds_p_o,
      lvds_n_o               => s_lvds_n_o,
      lvds_o_led_o           => s_lvds_o_led,
      lvds_oen_o(7 downto 0) => s_lvds_oen,
      lvds_term_o            => s_lvds_term,
      led_link_up_o          => led_link_up,
      led_link_act_o         => led_link_act,
      led_track_o            => led_track,
      led_pps_o              => led_pps,
      pcie_refclk_i          => pcie_refclk_i,
      pcie_rstn_i            => nPCI_RESET,
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
      ssd1325_rst_o          => dsp_resn_o,
      ssd1325_dc_o           => dsp_dcn_o,
      ssd1325_ss_o           => dsp_csn_o,
      ssd1325_sclk_o         => dsp_d0_o,
      ssd1325_data_o         => dsp_d1_o,
      nau8811_spi_csb_o      => aud_spi_csb_o,
      nau8811_spi_sclk_o     => aud_spi_sclk_o,
      nau8811_spi_sdio_o     => aud_spi_sdio_o,
      nau8811_iis_fs_o       => aud_iis_fs_o,
      nau8811_iis_bclk_o     => aud_iis_bclk_o,
      nau8811_iis_adcout_o   => aud_iis_adcout_o,
      nau8811_iis_dacin_i    => aud_iis_dacin_i,
      ps_clk                 => sram_clk,
      ps_addr                => sram_a,
      ps_data                => sram_dq,
      ps_seln(0)             => sram_be0,
      ps_seln(1)             => sram_be1,
      ps_cen                 => sram_cen,
      ps_oen                 => sram_oen,
      ps_wen                 => sram_wen,
      ps_cre                 => sram_cre,
      ps_advn                => sram_advn,
      ps_wait                => sram_wait,
      ow_io(0)               => db_rom_data_io,
      ow_io(1)               => 'Z');
  
  -- SFP1-3 are not mounted
  sfp1_tx_disable_o <= '1';
  sfp2_tx_disable_o <= '1';
  sfp3_tx_disable_o <= '1';
  sfp4_tx_disable_o <= '0';

  -- Base board LEDs (2.5V outputs, with 2.5V pull-up)
  led_o(1) <= '0' when (led_link_act and led_link_up)='1' else 'Z'; -- red   = traffic/no-link
  led_o(2) <= '0' when led_link_up                   ='1' else 'Z'; -- blue  = link
  led_o(3) <= '0' when led_track                     ='1' else 'Z'; -- green = timing valid
  led_o(4) <= '0' when led_pps                       ='1' else 'Z'; -- white = PPS
  
  -- Link LEDs (2.5V outputs, with 2.5V pull-up)
  sfp4_red_o   <= '0' when led_link_act='1' else 'Z';
  sfp4_green_o <= '0' when led_link_up ='1' else 'Z';
  
  -- GPIO LED outputs
  led_o(5)         <= '0' when s_gpio_o(1)='1' else 'Z'; -- (baseboard)
  led_o(6)         <= '0' when s_gpio_o(2)='1' else 'Z'; -- 2.5V output, with 2.5V pull-up
  led_o(7)         <= '0' when s_gpio_o(3)='1' else 'Z';
  led_o(8)         <= '0' when s_gpio_o(4)='1' else 'Z';
  
  -- GPIO inputs
  s_gpio_i <= not button_i;
  
  -- Bidirectional LEMOs
  lemos : for i in 1 to 8 generate
    s_lvds_p_i(i) <= lemo_p_i(i);
    s_lvds_n_i(i) <= lemo_n_i(i);
    lemo_p_o(i) <= s_lvds_p_o(i);
    lemo_n_o(i) <= s_lvds_n_o(i);
    
    lemo_oen_o(i)      <= '0' when s_lvds_oen(i)  ='1' else '1'; -- has pull-up to 3.3V, output is 3.3V
    lemo_term_o(i)     <= '1' when s_lvds_term(i) ='1' else '0'; -- has pull-down,       output is 3.3V
    lemo_oe_ledn_o(i)  <= '0' when s_lvds_oen(i)  ='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
    lemo_act_ledn_o(i) <= '0' when s_lvds_i_led(i)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
  end generate;
  
  lvds : for i in 1 to 3 generate
    s_lvds_p_i(i+8) <= lvds_p_i(i);
    s_lvds_n_i(i+8) <= lvds_n_i(i);
    lvds_p_o(i) <= s_lvds_p_o(i+8);
    lvds_n_o(i) <= s_lvds_n_o(i+8);
    
    lvds_i_ledn_o(i) <= '0' when s_lvds_i_led(i+8)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
    lvds_o_ledn_o(i) <= '0' when s_lvds_o_led(i+8)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
  end generate;
  
  -- Wires to CPLD, currently unused
  con_io <= (others => 'Z');
  
end rtl;
