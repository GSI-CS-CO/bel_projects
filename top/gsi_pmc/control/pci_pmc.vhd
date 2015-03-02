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
    clk_20m_vcxo_i    : in    std_logic;  -- 20MHz VCXO clock                 -- AF21
    clk_125m_pllref_i : in    std_logic;  -- 125 MHz PLL reference            -- N => G15 -- P => H15
    clk_125m_local_i  : in    std_logic;  -- local clk from 125Mhz oszillator -- N => C22 -- P => C223
    sfp234_ref_clk_i  : in    std_logic;                                      -- AF21
   
    -----------------------------------------
    -- PMC/PCI2.2 pins
    -----------------------------------------
    pmc_clk_i         : in    std_logic;                     -- P => B15 -- N => C15
    pmc_rst_i         : in    std_logic;                     -- H19
    pmc_buf_oe_o      : out   std_logic;                     -- A23
    pmc_busmode_io    : inout std_logic_vector(4 downto 1);  -- 04 => B19  -- 03 => C19  -- 02 => D19  -- 01 => A20
    pmc_ad_io         : inout std_logic_vector(31 downto 0); -- 31 => AD18 -- 23 => AF15 -- 15 => AA18 -- 07 => AA15
                                                             -- 30 => AE18 -- 22 => AF16 -- 14 => AB18 -- 06 => AB15
                                                             -- 29 => W18  -- 21 => V15  -- 13 => Y17  -- 05 => AD15
                                                             -- 28 => Y18  -- 20 => W15  -- 12 => AA17 -- 04 => AE15
                                                             -- 27 => AB16 -- 19 => AC14 -- 11 => AC17 -- 03 => Y14
                                                             -- 26 => AC16 -- 18 => AD14 -- 10 => AD17 -- 02 => AA14
                                                             -- 25 => V16  -- 17 => AF14 -- 09 => AD16 -- 01 => AA16
                                                             -- 24 => W16  -- 16 => AF13 -- 08 => AE16 -- 00 => AD19
    pmc_c_be_io       : inout std_logic_vector(3 downto 0);  -- 03 => C20  -- 02 => D20  -- 01 => B21  -- 00 => C21
    pmc_par_io        : inout std_logic;                     -- B22
    pmc_frame_io      : inout std_logic;                     -- P19
    pmc_trdy_io       : inout std_logic;                     -- A22
    pmc_irdy_io       : inout std_logic;                     -- E19
    pmc_stop_io       : inout std_logic;                     -- A21
    pmc_devsel_io     : inout std_logic;                     -- G21
    pmc_idsel_i       : in    std_logic;                     -- H21
    pmc_perr_io       : inout std_logic;                     -- J19
    pmc_serr_io       : inout std_logic;                     -- E21
    pmc_inta_o        : out   std_logic;                     -- D21
    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk       : out std_logic;                    -- AC8
    wr_dac_din        : out std_logic;                    -- AD8
    wr_ndac_cs        : out std_logic_vector(2 downto 1); -- 2 => AF7 -- 1 => AF8
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data          : inout std_logic; -- AA2
    
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    dis_di            : out   std_logic_vector(6 downto 0); -- 6 => AB1 -- 5 => AA1 -- 4 => W4 -- 3 => W3
                                                            -- 2 => Y4  -- 1 => Y5  -- 0 => AA4
    dis_ai            : in    std_logic_vector(1 downto 0); -- 1 => W6 -- 0 => W7
    dis_do            : in    std_logic;                    -- AA5
    dis_wr            : out   std_logic := '0';             -- AA6
    dis_res           : out   std_logic := '1';             -- AA3
    
    -----------------------------------------------------------------------
    -- reset
    -----------------------------------------------------------------------
    fpga_res          : in    std_logic; -- Y1
    nres              : in    std_logic; -- AC7
    
    -----------------------------------------------------------------------
     -- logic analyzer
    -----------------------------------------------------------------------
    hpwck             : inout std_logic := 'Z';                                 -- K3
    hpw               : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- 15 => D1 -- 14 => F1 -- 13 => B1 -- 12 => D2
                                                                                -- 11 => J2 -- 10 => F2 --  9 => J3 --  8 => E3
                                                                                --  7 => G2 --  6 => K6 --  5 => G3 --  4 => K4
                                                                                --  3 => F3 --  2 => J2 --  1 => H3 --  0 => L3
    
    -----------------------------------------------------------------------
    -- lvttio/lvds
    -----------------------------------------------------------------------
    lvttio_in_p_1     : in    std_logic; -- D6
    lvttio_in_p_2     : in    std_logic; -- E7
    lvttio_in_p_3     : in    std_logic; -- A5
    lvttio_in_p_4     : in    std_logic; -- B6
    lvttio_in_p_5     : in    std_logic; -- G9
    lvttio_in_n_1     : in    std_logic; -- E6
    lvttio_in_n_2     : in    std_logic; -- F7
    lvttio_in_n_3     : in    std_logic; -- A6
    lvttio_in_n_4     : in    std_logic; -- C6
    lvttio_in_n_5     : in    std_logic; -- H9
                              
    lvttio_out_p_1    : out   std_logic; -- J9
    lvttio_out_p_2    : out   std_logic; -- C5
    lvttio_out_p_3    : out   std_logic; -- F8
    lvttio_out_p_4    : out   std_logic; -- B7
    lvttio_out_p_5    : out   std_logic; -- A8
    lvttio_out_n_1    : out   std_logic; -- J8
    lvttio_out_n_2    : out   std_logic; -- D5
    lvttio_out_n_3    : out   std_logic; -- G8
    lvttio_out_n_4    : out   std_logic; -- C7
    lvttio_out_n_5    : out   std_logic; -- A9
                              
    lvttio_oe_1       : out   std_logic; -- N5
    lvttio_oe_2       : out   std_logic; -- R3
    lvttio_oe_3       : out   std_logic; -- T3
    lvttio_oe_4       : out   std_logic; -- U5
    lvttio_oe_5       : out   std_logic; -- W1
                              
    lvttio_term_en_1  : out   std_logic; -- P1
    lvttio_term_en_2  : out   std_logic; -- R6
    lvttio_term_en_3  : out   std_logic; -- T6
    lvttio_term_en_4  : out   std_logic; -- V1
    lvttio_term_en_5  : out   std_logic; -- W2
                              
    lvttio_dir_led_1  : out   std_logic; -- R2
    lvttio_dir_led_2  : out   std_logic; -- T2
    lvttio_dir_led_3  : out   std_logic; -- U1
    lvttio_dir_led_4  : out   std_logic; -- V5
    lvttio_dir_led_5  : out   std_logic; -- P4
                              
    lvttio_act_led_1  : out   std_logic; -- P3
    lvttio_act_led_2  : out   std_logic; -- T1
    lvttio_act_led_3  : out   std_logic; -- T7
    lvttio_act_led_4  : out   std_logic; -- V4
    lvttio_act_led_5  : out   std_logic; -- N6
    
    lvttl_clk_i       : in    std_logic; -- P => M1 -- N => N1
    lvttl_in_clk_en_o : out   std_logic; -- P6
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con               : out   std_logic_vector(5 downto 1); -- 5 => L4 -- 4 => K7 -- 3 => J5 -- 2 => E1
                                                            -- 1 => C1
    
    -----------------------------------------------------------------------
    -- hex switch
    -----------------------------------------------------------------------
    hswf              : in    std_logic_vector(4 downto 1); -- 4 => M2 -- 3 => L1 -- 2 => H1 -- 1 => J1
    
    -----------------------------------------------------------------------
    -- push buttons
    -----------------------------------------------------------------------
    pbs_f             : in    std_logic; -- AF18
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd              : out   std_logic;                                       -- AF5
    slwr              : out   std_logic;                                       -- AE4
    fd                : inout std_logic_vector(7 downto 0) := (others => 'Z'); -- 7 => AC1 -- 6 => AF3 -- 5 => AD4 -- 4 => AE3
                                                                               -- 3 => AB2 -- 2 => AD5 -- 1 => AB4 -- 0 => AB3
    pa                : inout std_logic_vector(7 downto 0) := (others => 'Z'); -- 7 => AC5 -- 6 => AD6 -- 5 => AF6 -- 4 => AE6
                                                                               -- 3 => AA9 -- 2 => Y9  -- 1 => W9  -- 0 => V9
    ctl               : in    std_logic_vector(2 downto 0);                    -- 2 => AD3 -- 1 => AC3 -- 0 => AA7
    uclk              : in    std_logic;                                       -- AB6
    ures              : out   std_logic;                                       -- AC6
    ifclk             : inout std_logic := 'Z';                                -- AD7
    wakeup            : inout std_logic := 'Z';                                -- AA8
    
    -----------------------------------------------------------------------
    -- leds on board
    -----------------------------------------------------------------------
    user_led_o        : out   std_logic_vector(8 downto 1); -- 8 => AC19 -- 7 => AC20 -- 6 => AE23 -- 5 => AD23
                                                            -- 4 => AA21 -- 3 => Y21  -- 2 => AB19 -- 1 => AA20
    
    -----------------------------------------------------------------------
    -- leds front panel
    -----------------------------------------------------------------------
    status_led_o      : out   std_logic_vector(6 downto 1); -- 6 => AE19 -- 5 => AF19 -- 4 => AF23 -- 3 => AC22
                                                            -- 2 => AC21 -- 1 => AB21

    -----------------------------------------------------------------------
    -- SFP4 
    -----------------------------------------------------------------------
    
    sfp4_tx_disable_o : out std_logic := '0';                  -- H12
    sfp4_tx_fault     : in std_logic;                          -- C12
    sfp4_los          : in std_logic;                          -- B12
    
    sfp4_txp_o        : out std_logic;                         -- N => J23 -- P => J24
    sfp4_rxp_i        : in  std_logic;                         -- N => K25 -- P => K26
    
    sfp4_mod0         : in    std_logic; -- grounded by module -- A12
    sfp4_mod1         : inout std_logic; -- SCL                -- A11
    sfp4_mod2         : inout std_logic  -- SDA                -- J12
    
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
  
  signal s_butis        : std_logic;
  signal s_butis_t0     : std_logic;

begin

  main : monster
    generic map(
      g_family      => "Arria V",
      g_project     => "pci_pmc",
      g_flash_bits  => 25,
      g_lvds_inout  => 5,  -- 5 LEMOs at front panel
      g_gpio_out    => 10, -- 2 LEDs at front panel + 8 on-boards LEDs
      g_en_usb      => true,
      g_en_lcd      => true,
      g_en_pmc      => true,
      g_en_pmc_ctrl => true)
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_rstn_i            => pbs_f,
      core_clk_butis_o       => s_butis,
      core_clk_butis_t0_o    => s_butis_t0,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
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
      pmc_buf_oe_o           => pmc_buf_oe_o,
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
      pmc_ctrl_hs_i          => hswf,
      pmc_clk_en_o           => lvttl_in_clk_en_o,
      lcd_scp_o              => dis_di(3),
      lcd_lp_o               => dis_di(1),
      lcd_flm_o              => dis_di(2),
      lcd_in_o               => dis_di(0));

  -- SFP1-3 are not mounted
  sfp4_tx_disable_o <= '0';

  -- Link LEDs
  dis_wr    <= '0';
  dis_res   <= '1';
  dis_di(5) <= '0' when (not s_led_link_up)                     = '1' else 'Z'; -- red
  dis_di(6) <= '0' when (    s_led_link_up and not s_led_track) = '1' else 'Z'; -- blue
  dis_di(4) <= '0' when (    s_led_link_up and     s_led_track) = '1' else 'Z'; -- green
  
  status_led_o(1) <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  status_led_o(2) <= not s_led_link_up;                      -- blue  = link
  status_led_o(3) <= not s_led_track;                        -- green = timing valid
  status_led_o(4) <= not s_led_pps;                          -- white = PPS

  -- GPIOs
  status_led_o(6 downto 5) <= s_gpio (1 downto 0);
  user_led_o(8 downto 1)   <= s_gpio (9 downto 2);
  
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
  lvttio_act_led_1 <= '0' when s_lvds_i_led(0) = '1' else 'Z';
  lvttio_act_led_2 <= '0' when s_lvds_i_led(1) = '1' else 'Z';
  lvttio_act_led_3 <= '0' when s_lvds_i_led(2) = '1' else 'Z';
  lvttio_act_led_4 <= '0' when s_lvds_i_led(3) = '1' else 'Z';
  lvttio_act_led_5 <= '0' when s_lvds_i_led(4) = '1' else 'Z';
  
  -- Wires to CPLD, currently unused
  con <= (others => 'Z');
  
end rtl;
