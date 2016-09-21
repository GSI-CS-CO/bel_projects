library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;

entity pci_pmc is
  generic(
		g_HW_TEST : boolean := false
  );
  port(
    -----------------------------------------
    -- Clocks
    -----------------------------------------
    clk_20m_vcxo_i    : in std_logic; -- 20MHz VCXO clock (BANK 3A - AF21)
    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference (BANK 8D - G12, H15)
    clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oscillator (BANK 8A - C23, C22)
    sfp234_ref_clk_i  : in std_logic; -- SFP clk (BANK GXB_L1 - N18, N19)

    -----------------------------------------
    -- PMC/PCI2.2 pins
    -----------------------------------------
    pmc_clk_i         : in    std_logic;
    pmc_rst_i         : in    std_logic;
    pmc_buf_oe_o      : out   std_logic;
    pmc_busmode_io    : inout std_logic_vector(4 downto 1);
    pmc_ad_io         : inout std_logic_vector(31 downto 0);
    pmc_c_be_io       : inout std_logic_vector(3 downto 0);
    pmc_par_io        : inout std_logic;
    pmc_frame_io      : inout std_logic;
    pmc_trdy_io       : inout std_logic;
    pmc_irdy_io       : inout std_logic;
    pmc_stop_io       : inout std_logic;
    pmc_devsel_io     : inout std_logic;
    pmc_idsel_i       : in    std_logic;
    pmc_perr_io       : inout std_logic;
    pmc_serr_io       : inout std_logic;
    pmc_inta_o        : out   std_logic;
    pmc_req_o         : out   std_logic;
    pmc_gnt_i         : in    std_logic;
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk       : out std_logic;
    wr_dac_din        : out std_logic;
    wr_ndac_cs        : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data          : inout std_logic;
    
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    dis_di            : out std_logic_vector(6 downto 0);
    dis_ai            : in  std_logic_vector(1 downto 0);
    dis_do            : in  std_logic;
    dis_wr            : out std_logic := '0';
    dis_res           : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- reset
    -----------------------------------------------------------------------
    fpga_res          : in std_logic;
    nres              : in std_logic;
    
    -----------------------------------------------------------------------
     -- logic analyzer
    -----------------------------------------------------------------------
    hpwck             : inout std_logic := 'Z';
    hpw               : inout std_logic_vector(15 downto 0) := (others => 'Z');
   
    -----------------------------------------------------------------------
    -- lvttio/lvds
    -----------------------------------------------------------------------
    lvttio_in_p       : in  std_logic_vector(5 downto 1);
    lvttio_in_n       : in  std_logic_vector(5 downto 1);
    
    lvttio_out_p     : out  std_logic_vector(5 downto 1);
    lvttio_out_n     : out  std_logic_vector(5 downto 1);

    lvttio_oe         : out std_logic_vector(5 downto 1);
    lvttio_term_en    : out std_logic_vector(5 downto 1);
    lvttio_dir_led    : out std_logic_vector(5 downto 1);
    lvttio_act_led    : out std_logic_vector(5 downto 1);
                           
    lvttl_clk_i       : in  std_logic;
    lvttl_in_clk_en_o : out std_logic;
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con               : in std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- hex switch
    -----------------------------------------------------------------------
    hswf              : in std_logic_vector(4 downto 1);
    
    -----------------------------------------------------------------------
    -- push buttons
    -----------------------------------------------------------------------
    pbs_f             : in std_logic;
    
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
    -- leds on board
    -----------------------------------------------------------------------
    user_led_o        : out std_logic_vector(8 downto 1);
    
    -----------------------------------------------------------------------
    -- leds front panel
    -----------------------------------------------------------------------
    status_led_o      : out std_logic_vector(6 downto 1);
    
    -----------------------------------------------------------------------
    -- SFP 
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic := '0';
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_txp_o        : out   std_logic;
    sfp_rxp_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic; -- grounded by module
    sfp_mod1         : inout std_logic; -- SCL
    sfp_mod2         : inout std_logic  -- SDA
    );
end pci_pmc;

architecture rtl of pci_pmc is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_status_led_moster  : std_logic_vector(6 downto 1);
  signal s_user_led_monster   : std_logic_vector(8 downto 1);

  signal s_status_led         : std_logic_vector(6 downto 1);
  signal s_user_led           : std_logic_vector(8 downto 1);
  
  signal s_gpio_out           : std_logic_vector(8 downto 0);
  signal s_gpio_in            : std_logic_vector(9 downto 0);
  
  signal s_lvds_p_i     : std_logic_vector(5 downto 1);
  signal s_lvds_n_i     : std_logic_vector(5 downto 1);
  signal s_lvds_i_led   : std_logic_vector(5 downto 1);
  signal s_lvds_p_o     : std_logic_vector(5 downto 1);
  signal s_lvds_n_o     : std_logic_vector(5 downto 1);
  signal s_lvds_o_led   : std_logic_vector(5 downto 1);
  signal s_lvds_led     : std_logic_vector(5 downto 1);  
  signal s_lvds_oe      : std_logic_vector(5 downto 1);
  signal s_lvds_term_en : std_logic_vector(5 downto 1);

  
  signal s_log_oe       : std_logic_vector(16 downto 0);
  signal s_log_out      : std_logic_vector(16 downto 0);
  signal s_log_in       : std_logic_vector(16 downto 0);

  signal s_wr_ext_in    : std_logic;
  
  signal s_butis        : std_logic;
  signal s_butis_t0     : std_logic;


  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 23) := 
  (
  -- Name[11 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1       ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL), -- user LEDs
    ("LED2       ", IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3       ", IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4       ", IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED5       ", IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED6       ", IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED7       ", IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED8       ", IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("IO1        ", IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL), -- front panel IOs
    ("IO2        ", IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO3        ", IO_NONE,         false,   false,  2,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO4        ", IO_NONE,         false,   false,  3,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO5        ", IO_NONE,         false,   false,  4,     IO_INOUTPUT, IO_LVDS,  true,         true,        IO_LVTTL),
    ("PBF        ", IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL), -- FPGA push button
    ("PBC        ", IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL), -- CPLD push button
    ("SWF0       ", IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL), -- FPGA HEX switch
    ("SWF1       ", IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("SWF2       ", IO_NONE,         false,   false,  2,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("SWF3       ", IO_NONE,         false,   false,  3,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("SWC0       ", IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL), -- CPLD HEX switch
    ("SWC1       ", IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("SWC2       ", IO_NONE,         false,   false,  2,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("SWC3       ", IO_NONE,         false,   false,  3,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVTTL),
    ("IOCLKEN    ", IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_LVTTL)  -- enable for IO clk buffer
  );

  
  constant c_family     : string := "Arria V"; 
  constant c_project    : string := "pci_pmc";
  constant c_cores      : natural:= 1;
  constant c_initf_name : string := c_project & ".mif";
  constant c_profile_name : string := "medium_icache_debug";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';' 

begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_lvds_inout      => 5,  -- 5 LEMOs at front panel
      g_lvds_in         => 0,
      g_lvds_out        => 0,
      g_gpio_out        => 9,  -- 8 on-boards LEDs
      g_gpio_in         => 10, -- FPGA button and HEX switch (1+4), CPLD button and HEX switch (1+4)
      g_en_usb          => true,
      g_en_lcd          => true,
      g_io_table        => io_mapping_table,
      g_en_pmc          => true,
--     g_en_pmc_ctrl     => true,
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
      core_clk_butis_o       => s_butis,
      core_clk_butis_t0_o    => s_butis_t0,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp_mod2,
      wr_sfp_scl_io          => sfp_mod1,
      wr_sfp_det_i           => sfp_mod0_i,
      wr_sfp_tx_o            => sfp_txp_o,
      wr_sfp_rx_i            => sfp_rxp_i,
      wr_ext_clk_i           => lvttl_clk_i,
      wr_dac_sclk_o          => wr_dac_sclk,
      wr_dac_din_o           => wr_dac_din,
      wr_ndac_cs_o           => wr_ndac_cs,
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
 
      pmc_pci_clk_i          => pmc_clk_i,
      pmc_pci_rst_i          => pmc_rst_i,
      pmc_buf_oe_o           => open, -- pmc_buf_oe_o,
      pmc_busmode_io         => pmc_busmode_io,
      pmc_ad_io              => pmc_ad_io,
      pmc_c_be_io            => pmc_c_be_io,
      pmc_par_io             => pmc_par_io,
      pmc_frame_io           => pmc_frame_io,
      pmc_trdy_io            => pmc_trdy_io,
      pmc_irdy_io            => pmc_irdy_io,
      pmc_stop_io            => pmc_stop_io,
      pmc_devsel_io          => pmc_devsel_io,
      pmc_idsel_i            => pmc_idsel_i,
      pmc_perr_io            => pmc_perr_io,
      pmc_serr_io            => pmc_serr_io,
      pmc_inta_o             => pmc_inta_o,
      pmc_req_o              => pmc_req_o,
      pmc_gnt_i              => pmc_gnt_i,

      lcd_scp_o              => dis_di(3),
      lcd_lp_o               => dis_di(1),
      lcd_flm_o              => dis_di(2),
      lcd_in_o               => dis_di(0)
);
 


  -- button and hex switches as gpio inputs
  s_gpio_in(0)  <= not pbs_f;  -- fpga button
  s_gpio_in(1)  <= not con(5); -- cpld button
  s_gpio_in(5 downto 2) <= not hswf; --  fpga hex switch
  s_gpio_in(9 downto 6) <= not con(4 downto 1); -- cpld hex switch


  pmc_buf_oe_o <= '1'; -- enable PCI bus translators

  -- SFP always enabled
  sfp_tx_disable_o <= '0';

  -- Display
  dis_wr    <= '0';
  dis_res   <= '1';
  
  -- WR LEDs
  dis_di(5) <= '0' when (not s_led_link_up)                     = '1' else 'Z'; -- red
  dis_di(6) <= '0' when (    s_led_link_up and not s_led_track) = '1' else 'Z'; -- blue
  dis_di(4) <= '0' when (    s_led_link_up and     s_led_track) = '1' else 'Z'; -- green
  
  -- Link LEDs
  s_status_led_moster(1) <= s_led_link_act and s_led_link_up;   -- red   = traffic/no-link
  s_status_led_moster(2) <= s_led_link_up;                      -- blue  = link
  s_status_led_moster(3) <= s_led_track;                        -- green = timing valid
  s_status_led_moster(4) <= s_led_pps;                          -- white = PPS

  -- GPIOs
  s_status_led_moster(6 downto 5) <= s_gpio_out (1 downto 0);


  -- status LED output (active low)
  status_led_o <= not s_status_led_moster;         -- driven by monster

  -- USER LED output (active_low)
  user_led_o <= not s_gpio_out(7 downto 0);         -- driven by monster

  
  -- LVDS inputs
  s_lvds_p_i <= lvttio_in_p;
  s_lvds_n_i <= lvttio_in_n;
  
  -- LVDS outputs
  lvttio_out_p <= s_lvds_p_o;
  lvttio_out_n <= s_lvds_n_o;

  io_ctrl : for i in 1 to 5 generate
	  -- LVDS output enable pins (active low)
	  lvttio_oe(i) <= '0' when s_lvds_oe(i) = '1' else 'Z'; -- driven by monster

	  -- LVDS termination pins (active hi)
	  lvttio_term_en(i) <= '1' when s_lvds_term_en(i) = '1' else '0'; -- driven by monster

	  -- LVDS direction indicator RED LEDs (active hi)
	  lvttio_dir_led(i) <= '1' when s_lvds_oe(i) = '1' else '0';    -- driven by monster

	  -- LVDS activity indicator BLUE LEDs (active hi)
	  lvttio_act_led(i) <= s_lvds_i_led(i) or s_lvds_o_led(i);   -- driven by monster
	end generate;
	
  -- Logic analyzer
  -- inputs
  --s_log_in(15 downto 0) <= hpw(15 downto 0);
  --s_log_in(16)          <= hpwck;

  -- outputs
  hpwck           <= 'Z';
  hpw_out : for i in 0 to 15 generate
    hpw(i)        <= 'Z';  
  end generate;
  
  -- Enable clock input from IO
  lvttl_in_clk_en_o <= '0' when s_gpio_out(8)= '1' else 'Z'; 

  
end rtl;
