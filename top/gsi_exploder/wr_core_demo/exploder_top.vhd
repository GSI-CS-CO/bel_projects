library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.gencores_pkg.all; -- pulse extend LEDs

entity exploder_top is
  port(
    clk_20m_vcxo_i    : in std_logic; -- N4p  N3n  --  20 MHz WR VCXO clock
    clk_125m_pllref_i : in std_logic; -- F12p F11n -- 125 MHz WR PLL reference
    clk_125m_local_i  : in std_logic; -- D11p C11n -- 125 MHz local oscillator (CKPLL_125P)
    nres_i            : in std_logic; -- E3        -- powerup reset 2.5V
        
    -----------------------------------------------------------------------
    -- OneWire 3.3V
    -----------------------------------------------------------------------
    rom_data_io : inout std_logic; -- AB2
    
    -----------------------------------------
    -- Timing SFPs 2.5V
    -----------------------------------------
    sfp_ref_clk_i   : in    std_logic; -- G23p G24n
    
    sfp1_td_o       : out   std_logic; -- B21p B22n
    sfp1_rd_i       : in    std_logic; -- C23p C24n
    sfp1_tx_fault_i : in    std_logic; -- P1
    sfp1_los_i      : in    std_logic; -- N1
    sfp1_tx_dis_o   : out   std_logic; -- M7

    sfp1_mod0_i     : in    std_logic; -- L7
    sfp1_mod1_io    : inout std_logic; -- L1
    sfp1_mod2_io    : inout std_logic; -- K1
    
    --sfp2_td_o       : out   std_logic; -- D21p D22n
    --sfp2_rd_i       : in    std_logic; -- E23p E24n
    sfp2_tx_fault_i : in    std_logic; -- K4
    sfp2_los_i      : in    std_logic; -- K5
    sfp2_tx_dis_o   : out   std_logic; -- J1

    sfp2_mod0_i     : in    std_logic; -- H1
    sfp2_mod1_io    : inout std_logic; -- J5
    sfp2_mod2_io    : inout std_logic; -- J6
    
    --sfp3_td_o       : out   std_logic; -- H21p H22n
    --sfp3_rd_i       : in    std_logic; -- J23p J24n
    sfp3_tx_fault_i : in    std_logic; -- F1
    sfp3_los_i      : in    std_logic; -- E1
    sfp3_tx_dis_o   : out   std_logic; -- H6

    sfp3_mod0_i     : in    std_logic; -- H7
    sfp3_mod1_io    : inout std_logic; -- D1
    sfp3_mod2_io    : inout std_logic; -- D2
    
    --sfp4_td_o       : out   std_logic; -- K21p K22n
    --sfp4_rd_i       : in    std_logic; -- L23p L24n
    sfp4_tx_fault_i : in    std_logic; -- G5
    sfp4_los_i      : in    std_logic; -- G6
    sfp4_tx_dis_o   : out   std_logic; -- C2

    sfp4_mod0_i     : in    std_logic; -- C3
    sfp4_mod1_io    : inout std_logic; -- G7
    sfp4_mod2_io    : inout std_logic; -- F7
    
    ------------------------------------------------------------------------
    -- WR DAC signals 3.3V
    ------------------------------------------------------------------------
    dac_sclk_o      : out std_logic; -- R3
    dac_din_o       : out std_logic; -- R4
    ndac_cs_o       : out std_logic_vector(2 downto 1);
    -- ndac_cs_o(1) -- T2
    -- ndac_cs_o(2) -- T3
    
    -----------------------------------------
    -- Logic analyzer HPLA1 2.5V
    -----------------------------------------
    hpw_io : inout std_logic_vector(15 downto 0);
    -- hpw_io( 0) -- M3 -- close to VXCO clock; do not drive quickly!
    -- hpw_io( 1) -- M4 -- close to VXCO clock; do not drive quickly!
    -- hpw_io( 2) -- M1
    -- hpw_io( 3) -- N2
    -- hpw_io( 4) -- L3
    -- hpw_io( 5) -- L4
    -- hpw_io( 6) -- K2
    -- hpw_io( 7) -- K3
    -- hpw_io( 8) -- J3
    -- hpw_io( 9) -- J4
    -- hpw_io(10) -- G1
    -- hpw_io(11) -- G2
    -- hpw_io(12) -- H3
    -- hpw_io(13) -- H4
    -- hpw_io(14) -- G3
    -- hpw_io(15) -- F3
    
    -----------------------------------------
    -- LEDs on baseboard 2.5V
    -----------------------------------------
    hpv_o : out std_logic_vector(7 downto 0);
    -- hpv_o(0) -- AC12  red
    -- hpv_o(1) -- AB12  blue
    -- hpv_o(2) -- AD12  green
    -- hpv_o(3) -- AD11  white
    -- hpv_o(4) -- AA12  red
    -- hpv_o(5) -- Y12   blue
    -- hpv_o(6) -- AD10  green
    -- hpv_o(7) -- AD9   white
    
    -----------------------------------------
    -- USB micro controller 3.3V
    -----------------------------------------
    pres_o    : out   std_logic; -- AC1   res must be '0'
    sres_o    : out   std_logic; -- AD5   active low reset#
    slrdn_o   : out   std_logic; -- AD2   read strobe
    slwrn_o   : out   std_logic; -- T4    write strobe
    speed_i   : in    std_logic; -- PA0 = AA1
    shift_i   : in    std_logic; -- PA1 = AB1
    sloen_o   : out   std_logic; -- PA2 = U1
    ebcyc_i   : in    std_logic; -- PA3 = V1
    fifoadr_o : out   std_logic_vector(1 downto 0); -- 0=PA4=R6, 1=PA5=R7
    pktendn_o : out   std_logic; -- PA6=R5
    readyn_io : inout std_logic; -- PA7=P7
    fulln_i   : in    std_logic; -- CTL1 = W2
    emptyn_i  : in    std_logic; -- CTL2 = T5
    fd_io     : inout std_logic_vector(7 downto 0); -- FIFO data bus
    -- fd_io(0) -- AD4
    -- fd_io(1) -- U6
    -- fd_io(2) -- AD3
    -- fd_io(3) -- AB4
    -- fd_io(4) -- AA4
    -- fd_io(5) -- V7
    -- fd_io(6) -- T6
    -- fd_io(7) -- V6
    -- CTL0 = V3, unused
    
    -----------------------------------------
    -- LVDSCON1 (exploder2b_db2) 2.5V
    -----------------------------------------
    -- Select mode on CH_OU*/ttnim_o (TTL or NIM)
    -- select_o=1 (selectn_o=0) is TTL
    -- select_o=0 (selectn_o=1) is NIM
    select_o  : out   std_logic; -- G13  P1  5
    selectn_o : out   std_logic; -- F13  N1  7
    
    -- CH_OU* output ports (standard from 'select')
    -- Max frequency < 200MHz
    ttnim_o : out  std_logic_vector(8 downto 1);
    -- ttnim_o(1) -- G14  P2 11
    -- ttnim_o(2) -- F14  N2 13
    -- ttnim_o(3) -- E13  P3 17 -- close to WR ref clock; do not drive quickly!
    -- ttnim_o(4) -- D13  N3 19 -- close to WR ref clock; do not drive quickly!
    -- ttnim_o(5) -- A20  P4 23
    -- ttnim_o(6) -- A19  N4 25
    -- ttnim_o(7) -- A18  P5 29
    -- ttnim_o(8) -- A17  N5 31
    
    -- Output J_LVDS1
    -- Max frequency < 500MHz
    lvds_o : out std_logic_vector(8 downto 1);
    -- lvds_o(1) -- F15  P11 18
    -- lvds_o(2) -- E15  N11 20
    -- lvds_o(3) -- D15  P12 24
    -- lvds_o(4) -- C15  N12 26
    -- lvds_o(5) -- C19  P13 30
    -- lvds_o(6) -- B18  N13 32
    -- lvds_o(7) -- C16  P14 36
    -- lvds_o(8) -- B16  N14 38
    
    -- Located above the CH_IN* input ports
    -- Max frequency < 20Hz (due to human eye and proximity to WR clock pins)
    led_o : out std_logic_vector(8 downto 1);
    -- led_o(1) -- B10  P17 55
    -- led_o(2) -- A10  N17 57
    -- led_o(3) -- B9   P18 61
    -- led_o(4) -- A9   N18 63
    -- led_o(5) -- B7   P19 67
    -- led_o(6) -- B6   N19 69
    -- led_o(7) -- D8   P20 73
    -- led_o(8) -- C7   N20 75
    
    -- Input J_ANY1: ECL, LVDS, LVDS, PECL
    any_i : in std_logic_vector(8 downto 1);
    -- any_i(1) -- C4   P21 79
    -- any_i(2) -- B4   N21 81
    -- any_i(3) -- D9   P22 85
    -- any_i(4) -- C9   N22 87
    -- any_i(5) -- F10  P23 91
    -- any_i(6) -- E10  N23 93
    -- any_i(7) -- G11  P24 97
    -- any_i(8) -- G10  N24 99
    
    -- Enable type2 ('1') or type1 ('0') on inputs (type1 is normal)
    fsen1_o : out std_logic;  -- B15  P15 42
    fsen2_o : out std_logic;  -- A14  N15 44
    
    -- Power-up the TRIGGER ports (drive with '1')
    mde_o   : out std_logic;  -- B13  P16 48
    
    -- Trigger inputs from TRIGER1 (1-8) and TRIGGER2 (9-16)
    rc_i : in std_logic_vector(16 downto 1);
    -- rc_i( 1) -- D10  P25 56
    -- rc_i( 2) -- C10  N25 58
    -- rc_i( 3) -- A8   P26 62
    -- rc_i( 4) -- A7   N26 64
    -- rc_i( 5) -- A6   P27 68
    -- rc_i( 6) -- A5   N27 70
    -- rc_i( 7) -- B3   P28 74
    -- rc_i( 8) -- A4   N28 76
    -- rc_i( 9) -- A3   P29 80
    -- rc_i(10) -- A2   N29 82
    -- rc_i(11) -- D6   P30 86
    -- rc_i(12) -- C6   N30 88
    -- rc_i(13) -- F9   P31 92
    -- rc_i(14) -- E9   N31 94
    -- rc_i(15) -- H9   P32 98
    -- rc_i(16) -- G9   N32 100
    
    -----------------------------------------
    -- LVDSCON2 (exploder2b_db2) 2.5V
    -----------------------------------------
    
    -- Driver input for TRIGGER1+2 (should always be '1')
    di_o : out std_logic_vector(8 downto 1);
    -- di_o(1) -- Y15  P33  5
    -- di_o(2) -- AA15 N33  7
    -- di_o(3) -- AA19 P34 11
    -- di_o(4) -- AB19 N34 13
    -- di_o(5) -- AD20 P35 17
    -- di_o(6) -- AD21 N35 19
    -- di_o(7) -- AC19 P36 23
    -- di_o(8) -- AD19 N36 25
    
    -- Driver enable for TRIGGER1+2 simultaneously
    -- Max frequency < 100 MHz
    de_o : out std_logic_vector(8 downto 1);
    -- de_o(1) -- AA18 P37 29
    -- de_o(2) -- AB18 N37 31
    -- de_o(3) -- AD17 P38 35
    -- de_o(4) -- AD18 N38 37
    -- de_o(5) -- AB15 P39 41
    -- de_o(6) -- AC15 N39 43
    -- de_o(7) -- AD15 P40 47
    -- de_o(8) -- AD16 N40 49
    
    -- Reserve; directly connected to FPGA
    res_io : inout std_logic_vector(8 downto 1);
    -- res_io(1) -- U16  P41  6
    -- res_io(2) -- V16  N41  8
    -- res_io(3) -- V15  P42 12
    -- res_io(4) -- W15  N42 14
    -- res_io(5) -- V14  P43 18
    -- res_io(6) -- W14  N43 20
    -- res_io(7) -- AB21 P44 24
    -- res_io(8) -- AC21 N44 26
    
    -- Display
    red_o   : out std_logic; -- AB16 N46 38 -- drive to '0' to go red
    green_o : out std_logic; -- AB17 P45 30
    blue_o  : out std_logic; -- AB9  P49 73
    bll_o   : out std_logic; -- AB14 N47 44 -- drive to '1'
    
    discp_o : out std_logic; -- AC18 N45 32 -- clock (run at 2MHz)
    dilp_o  : out std_logic; -- AA14 P47 42 -- latch pulse (end-of-40-bit-row)
    diflm_o : out std_logic; -- V13  P48 48 -- first-line marker
    diin_o  : out std_logic; -- AA16 P46 36 -- shift register in
    diout_i : in  std_logic; -- W13  N48 50 -- shift register out
    
    -- J_ECL1 outputs
    -- Max frequency < 80 MHz
    ecl_o : out std_logic_vector(8 downto 1);
    -- ecl_o(1) -- AB13 P54 56
    -- ecl_o(2) -- AC13 N54 58
    -- ecl_o(3) -- V12  P55 62
    -- ecl_o(4) -- W12  N55 64
    -- ecl_o(5) -- AA11 P56 68
    -- ecl_o(6) -- AB11 N56 70
    -- ecl_o(7) -- AA10 P57 74
    -- ecl_o(8) -- AB10 N57 76
    
    -- CH_IN* as TTL input (nor with nimin)
    ttlin_i : in std_logic_vector(8 downto 1);
    -- ttlin_i(1) -- AD7  P50 79
    -- ttlin_i(2) -- AD8  N50 81
    -- ttlin_i(3) -- AA8  P51 85
    -- ttlin_i(4) -- AB8  N51 87
    -- ttlin_i(5) -- W9   P58 80
    -- ttlin_i(6) -- W10  N58 82
    -- ttlin_i(7) -- V11  P59 86
    -- ttlin_i(8) -- W11  N59 88
    
    -- CH_IN* as NIM input (nor with ttlin)
    nimin_i : in std_logic_vector(8 downto 1));
    -- nimin_i(1) -- AC6  P52 91
    -- nimin_i(2) -- AD6  N52 93
    -- nimin_i(3) -- AA6  P53 97
    -- nimin_i(4) -- AB6  N53 99
    -- nimin_i(5) -- AA7  P60 92
    -- nimin_i(6) -- AB7  N60 94
    -- nimin_i(7) -- U9   P61 98
    -- nimin_i(8) -- V9   N61 100
end exploder_top;

architecture rtl of exploder_top is
  
  signal led_link_up  : std_logic;
  signal led_link_act : std_logic;
  signal led_track    : std_logic;
  signal led_pps      : std_logic;
  
  signal clk_ref   : std_logic;
  signal clk_butis : std_logic;
  
  signal lemo_ttl : std_logic;
  signal lemo_i   : std_logic_vector(8 downto 1);
  signal lemo_o   : std_logic_vector(4 downto 1);
  
  constant c_family  : string := "Arria II"; 
  constant c_project : string := "exploder_top";
  constant c_initf   : string := c_project & ".mif";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';'
  
begin

  main : monster
    generic map(
      g_family     => c_family,
      g_project    => c_project,
      g_gpio_in    => 16,
      g_gpio_out   => 16,
      g_flash_bits => 24,
      g_en_usb     => true,
      g_en_lcd     => true,
      g_lm32_init_files => c_initf
    )
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_clk_wr_ref_o      => clk_ref,
      core_clk_butis_o       => clk_butis,
      gpio_o(15 downto 12)   => de_o  (4 downto 1),
      gpio_o(11 downto  8)   => ecl_o (4 downto 1),
      gpio_o( 7 downto  4)   => lvds_o(4 downto 1),
      gpio_o( 3 downto  0)   => lemo_o(4 downto 1),
      gpio_i(15 downto 12)   => rc_i  (16 downto 13),
      gpio_i(11 downto  8)   => rc_i  (8 downto 5),
      gpio_i( 7 downto  4)   => any_i (4 downto 1),
      gpio_i( 3 downto  0)   => lemo_i(4 downto 1),
      wr_onewire_io          => rom_data_io,
      wr_sfp_sda_io          => sfp1_mod2_io,
      wr_sfp_scl_io          => sfp1_mod1_io,
      wr_sfp_det_i           => sfp1_mod0_i,
      wr_sfp_tx_o            => sfp1_td_o,
      wr_sfp_rx_i            => sfp1_rd_i,
      wr_dac_sclk_o          => dac_sclk_o,
      wr_dac_din_o           => dac_din_o,
      wr_ndac_cs_o           => ndac_cs_o,
      wr_ext_clk_i           => lemo_i(8),
      wr_ext_pps_i           => lemo_i(7),
      led_link_up_o          => led_link_up,
      led_link_act_o         => led_link_act,
      led_track_o            => led_track,
      led_pps_o              => led_pps,
      usb_rstn_o             => sres_o,
      usb_ebcyc_i            => ebcyc_i,
      usb_speed_i            => speed_i,
      usb_shift_i            => shift_i,
      usb_readyn_io          => readyn_io,
      usb_fifoadr_o          => fifoadr_o,
      usb_sloen_o            => sloen_o,
      usb_fulln_i            => fulln_i,
      usb_emptyn_i           => emptyn_i,
      usb_slrdn_o            => slrdn_o,
      usb_slwrn_o            => slwrn_o,
      usb_pktendn_o          => pktendn_o,
      usb_fd_io              => fd_io,
      lcd_scp_o              => discp_o,
      lcd_lp_o               => dilp_o,
      lcd_flm_o              => diflm_o,
      lcd_in_o               => diin_o);
  
  sfp1_tx_dis_o <= '0'; -- enable SFP1
  sfp2_tx_dis_o <= '1'; -- disable SFP2
  sfp3_tx_dis_o <= '1'; -- disable SFP3
  sfp4_tx_dis_o <= '1'; -- disable SFP4
  
  -- Unused SFP I2C
  sfp2_mod1_io <= 'Z';
  sfp2_mod2_io <= 'Z';
  sfp3_mod1_io <= 'Z';
  sfp3_mod2_io <= 'Z';
  sfp4_mod1_io <= 'Z';
  sfp4_mod2_io <= 'Z';

  -- Display back light
  -- red=nolink, blue=link+notrack, green=track
  bll_o   <= 'Z';
  red_o   <= '0' when (not led_link_up)                   = '1' else 'Z';
  blue_o  <= '0' when (    led_link_up and not led_track) = '1' else 'Z';
  green_o <= '0' when (    led_link_up and     led_track) = '1' else 'Z';
  
  -- Baseboard LEDs
  hpv_o(0) <= not led_link_act and led_link_up; -- red   = traffic/no-link
  hpv_o(1) <= not led_link_up;                  -- blue  = link
  hpv_o(2) <= not led_track;                    -- green = timing valid
  hpv_o(3) <= not led_pps;                      -- white = PPS
  
  -- LEMO inputs can come from TTL or NIM standard
  lemo_i <= not (ttlin_i or nimin_i);
  
  -- Extend input LEMO pulse to 20Hz on LED
  lemo_leds : for i in 1 to 8 generate
    lemo_led : gc_extend_pulse
      generic map(
        g_width => 125000000/20)
      port map(
        clk_i      => clk_ref,
        rst_n_i    => '1',
        pulse_i    => lemo_i(i),
        extended_o => led_o(i));
  end generate;

  -- Use output LEMOs in TTL mode
  lemo_ttl  <= '1';
  select_o  <= lemo_ttl;
  selectn_o <= not lemo_ttl;
  
  -- LEMO outputs (1-4 = GPIO0-3)
  ttnim_o(8) <= clk_butis;
  ttnim_o(7) <= '0';
  ttnim_o(6) <= led_pps;
  ttnim_o(5) <= clk_ref;
  ttnim_o(4 downto 1) <= lemo_o;
  hpv_o  (7 downto 4) <= lemo_o; -- LEDs show status
  
  -- ECA outputs (1-4 = GPIO4-7)
  lvds_o(8) <= clk_butis;
  lvds_o(7) <= '0';
  lvds_o(6) <= led_pps;
  lvds_o(5) <= clk_ref;
  
  -- ECL outputs (1-4 = GPIO8-12)
  ecl_o(8) <= clk_butis;
  ecl_o(7) <= '0';
  ecl_o(6) <= led_pps;
  ecl_o(5) <= clk_ref;
  
  -- Trigger 1+2 outputs
  de_o(8 downto 5) <= (others => '0');
  
  -- Use TRIGGER ports in type 1 mode.
  fsen1_o <= '0';
  fsen2_o <= '0';
  -- Enable control of the trigger bus
  mde_o   <= '1'; 
  -- Drive a '1' out the trigger bus on enable
  di_o <= (others => '1');
  
  -- reserved USB pin connected to FPGA by mistake. must be ground.
  pres_o <= '0'; 
  
  -- Baseboard logic analyzer (HPLA1)
  hpw_io(15 downto 0) <= (others => 'Z');
  
  -- RES is unused for now
  res_io(8 downto 1) <= (others => 'Z');
  
end rtl;
