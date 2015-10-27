library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;

entity pci_pmc is
  port(
    -----------------------------------------
    -- Clocks
    -----------------------------------------
    clk_20m_vcxo_i    : in std_logic; -- 20MHz VCXO clock
    clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference
    clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oszillator
    sfp234_ref_clk_i  : in std_logic; -- SFP clk
   
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
    lvttio_in_p_1     : in  std_logic;
    lvttio_in_p_2     : in  std_logic;
    lvttio_in_p_3     : in  std_logic;
    lvttio_in_p_4     : in  std_logic;
    lvttio_in_p_5     : in  std_logic;
    lvttio_in_n_1     : in  std_logic;
    lvttio_in_n_2     : in  std_logic;
    lvttio_in_n_3     : in  std_logic;
    lvttio_in_n_4     : in  std_logic;
    lvttio_in_n_5     : in  std_logic;
                            
    lvttio_out_p_1    : out std_logic;
    lvttio_out_p_2    : out std_logic;
    lvttio_out_p_3    : out std_logic;
    lvttio_out_p_4    : out std_logic;
    lvttio_out_p_5    : out std_logic;
    lvttio_out_n_1    : out std_logic;
    lvttio_out_n_2    : out std_logic;
    lvttio_out_n_3    : out std_logic;
    lvttio_out_n_4    : out std_logic;
    lvttio_out_n_5    : out std_logic;
                            
    lvttio_oe_1       : out std_logic;
    lvttio_oe_2       : out std_logic;
    lvttio_oe_3       : out std_logic;
    lvttio_oe_4       : out std_logic;
    lvttio_oe_5       : out std_logic;
                            
    lvttio_term_en_1  : out std_logic;
    lvttio_term_en_2  : out std_logic;
    lvttio_term_en_3  : out std_logic;
    lvttio_term_en_4  : out std_logic;
    lvttio_term_en_5  : out std_logic;
                            
    lvttio_dir_led_1  : out std_logic;
    lvttio_dir_led_2  : out std_logic;
    lvttio_dir_led_3  : out std_logic;
    lvttio_dir_led_4  : out std_logic;
    lvttio_dir_led_5  : out std_logic;
                            
    lvttio_act_led_1  : out std_logic;
    lvttio_act_led_2  : out std_logic;
    lvttio_act_led_3  : out std_logic;
    lvttio_act_led_4  : out std_logic;
    lvttio_act_led_5  : out std_logic;
    
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
  
  signal s_gpio         : std_logic_vector(9 downto 0);
  
  signal s_lvds_p_i     : std_logic_vector(4 downto 0);
  signal s_lvds_n_i     : std_logic_vector(4 downto 0);
  signal s_lvds_i_led   : std_logic_vector(4 downto 0);
  signal s_lvds_p_o     : std_logic_vector(4 downto 0);
  signal s_lvds_n_o     : std_logic_vector(4 downto 0);
  signal s_lvds_o_led   : std_logic_vector(4 downto 0);
  signal s_lvds_oen     : std_logic_vector(4 downto 0);
  
  signal s_log_oe       : std_logic_vector(16 downto 0);
  signal s_log_out      : std_logic_vector(16 downto 0);
  signal s_log_in       : std_logic_vector(16 downto 0);
  
  signal s_wr_ext_in    : std_logic;
  
  signal s_butis        : std_logic;
  signal s_butis_t0     : std_logic;
  
  constant c_family     : string := "Arria V"; 
  constant c_project    : string := "pci_pmc";
  constant c_initf      : string := c_project & ".mif";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';' 

begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_lvds_inout      => 5,  -- 5 LEMOs at front panel
      g_gpio_out        => 10, -- 2 LEDs at front panel + 8 on-boards LEDs
      g_en_usb          => true,
      g_en_lcd          => true,
      g_en_pmc          => true,
      g_en_pmc_ctrl     => true,
      g_lm32_init_files => c_initf)
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
      gpio_o                 => s_gpio,
      lvds_p_i               => s_lvds_p_i,
      lvds_n_i               => s_lvds_n_i,
      lvds_i_led_o           => s_lvds_i_led,
      lvds_p_o               => s_lvds_p_o,
      lvds_n_o               => s_lvds_n_o,
      lvds_o_led_o           => s_lvds_o_led,
      lvds_oen_o             => s_lvds_oen,
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

      pmc_ctrl_hs_i          => hswf,
      pmc_pb_i               => pbs_f,
      pmc_ctrl_hs_cpld_i     => con(4 downto 1),
      pmc_pb_cpld_i          => con(5),
      pmc_clk_oe_o           => s_wr_ext_in,
      pmc_log_oe_o           => s_log_oe,
      pmc_log_out_o          => s_log_out,
      pmc_log_in_i           => s_log_in,
      lcd_scp_o              => dis_di(3),
      lcd_lp_o               => dis_di(1),
      lcd_flm_o              => dis_di(2),
      lcd_in_o               => dis_di(0));


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
  status_led_o(1) <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  status_led_o(2) <= not s_led_link_up;                      -- blue  = link
  status_led_o(3) <= not s_led_track;                        -- green = timing valid
  status_led_o(4) <= not s_led_pps;                          -- white = PPS

  -- GPIOs
  status_led_o(6 downto 5) <= not s_gpio (1 downto 0);
--  user_led_o(8 downto 1)   <= not s_gpio (9 downto 2);

-- for testing CPLD to FPGA connection
p_led_val_sel : process( pbs_f, con, s_gpio ) 
begin
  if pbs_f = '0' then -- when pressed pass CPLD hex switch and button to FPGA user LEDs
    user_led_o(8 downto 6)   <= (others => '0'); 
    user_led_o(5 downto 1)   <= not con; -- driven by con from CPLD (HEX switch and button)
  else 
    user_led_o(8 downto 1)   <= not s_gpio (9 downto 2); -- driven by monster
  end if;
end process;
  
  -- LVDS inputs
  s_lvds_p_i(0) <= lvttio_in_p_1;
  s_lvds_p_i(1) <= lvttio_in_p_2;
  s_lvds_p_i(2) <= lvttio_in_p_3;
  s_lvds_p_i(3) <= lvttio_in_p_4;
  s_lvds_p_i(4) <= lvttio_in_p_5;
  s_lvds_n_i(0) <= lvttio_in_n_1;
  s_lvds_n_i(1) <= lvttio_in_n_2;
  s_lvds_n_i(2) <= lvttio_in_n_3;
  s_lvds_n_i(3) <= lvttio_in_n_4;
  s_lvds_n_i(4) <= lvttio_in_n_5;
  
  -- LVDS outputs
  lvttio_out_p_1 <= s_lvds_p_o(0);
  lvttio_out_p_2 <= s_lvds_p_o(1);
  lvttio_out_p_3 <= s_lvds_p_o(2);
  lvttio_out_p_4 <= s_lvds_p_o(3);
  lvttio_out_p_5 <= s_lvds_p_o(4);
  lvttio_out_n_1 <= s_lvds_n_o(0);
  lvttio_out_n_2 <= s_lvds_n_o(1);
  lvttio_out_n_3 <= s_lvds_n_o(2);
  lvttio_out_n_4 <= s_lvds_n_o(3);
  lvttio_out_n_5 <= s_lvds_n_o(4);
  
  -- LVDS output enable pins
  lvttio_oe_1 <= '0' when s_lvds_oen(0) = '0' else '1';
  lvttio_oe_2 <= '0' when s_lvds_oen(1) = '0' else '1';
  lvttio_oe_3 <= '0' when s_lvds_oen(2) = '0' else '1';
  lvttio_oe_4 <= '0' when s_lvds_oen(3) = '0' else '1';
  lvttio_oe_5 <= '0' when s_lvds_oen(4) = '0' else '1';
  
  -- LVDS termination pins
  lvttio_term_en_1 <= '0' when s_lvds_oen(0) = '0' else '1';
  lvttio_term_en_2 <= '0' when s_lvds_oen(1) = '0' else '1';
  lvttio_term_en_3 <= '0' when s_lvds_oen(2) = '0' else '1';
  lvttio_term_en_4 <= '0' when s_lvds_oen(3) = '0' else '1';
  lvttio_term_en_5 <= '0' when s_lvds_oen(4) = '0' else '1';
  
  -- LVDS direction indicator LEDs
  lvttio_dir_led_1 <= '0' when s_lvds_oen(0) = '1' else 'Z';
  lvttio_dir_led_2 <= '0' when s_lvds_oen(1) = '1' else 'Z';
  lvttio_dir_led_3 <= '0' when s_lvds_oen(2) = '1' else 'Z';
  lvttio_dir_led_4 <= '0' when s_lvds_oen(3) = '1' else 'Z';
  lvttio_dir_led_5 <= '0' when s_lvds_oen(4) = '1' else 'Z';
  
  -- LVDS activity indicator LEDs
  lvttio_act_led_1 <= '1' when s_lvds_i_led(0) = '1' else '0';
  lvttio_act_led_2 <= '1' when s_lvds_i_led(1) = '1' else '0';
  lvttio_act_led_3 <= '1' when s_lvds_i_led(2) = '1' else '0';
  lvttio_act_led_4 <= '1' when s_lvds_i_led(3) = '1' else '0';
  lvttio_act_led_5 <= '1' when s_lvds_i_led(4) = '1' else '0';

  -- Logic analyzer
  s_log_in(15 downto 0) <= hpw(15 downto 0);
  s_log_in(16)          <= hpwck;
  hpwck                 <= s_log_out(16) when s_log_oe(16) = '1' else 'Z';
  hpw(15)               <= s_log_out(15) when s_log_oe(15) = '1' else 'Z';
  hpw(14)               <= s_log_out(14) when s_log_oe(14) = '1' else 'Z';
  hpw(13)               <= s_log_out(13) when s_log_oe(13) = '1' else 'Z';
  hpw(12)               <= s_log_out(12) when s_log_oe(12) = '1' else 'Z';
  hpw(11)               <= s_log_out(11) when s_log_oe(11) = '1' else 'Z';
  hpw(10)               <= s_log_out(10) when s_log_oe(10) = '1' else 'Z';
  hpw(9)                <= s_log_out(9)  when s_log_oe(9)  = '1' else 'Z';
  hpw(8)                <= s_log_out(8)  when s_log_oe(8)  = '1' else 'Z';
  hpw(7)                <= s_log_out(7)  when s_log_oe(7)  = '1' else 'Z';
  hpw(6)                <= s_log_out(6)  when s_log_oe(6)  = '1' else 'Z';
  hpw(5)                <= s_log_out(5)  when s_log_oe(5)  = '1' else 'Z';
  hpw(4)                <= s_log_out(4)  when s_log_oe(4)  = '1' else 'Z';
  hpw(3)                <= s_log_out(3)  when s_log_oe(3)  = '1' else 'Z';
  hpw(2)                <= s_log_out(2)  when s_log_oe(2)  = '1' else 'Z';
  hpw(1)                <= s_log_out(1)  when s_log_oe(1)  = '1' else 'Z';
  hpw(0)                <= s_log_out(0)  when s_log_oe(0)  = '1' else 'Z';
  
  -- External white rabbit clock input
  lvttl_in_clk_en_o <= not(s_wr_ext_in); 
  
end rtl;
