library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.wrcore_pkg.all;
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.wb_cores_pkg_gsi.all;
use work.eca_pkg.all;
use work.wr_altera_pkg.all;
use work.altera_flash_pkg.all;
use work.altera_networks_pkg.all;
use work.build_id_pkg.all;
use work.etherbone_pkg.all;
use work.ez_usb_pkg.all;

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
  
  -- WR core layout
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  -- Top crossbar layout
  constant c_slaves  : natural := 7;
  constant c_masters : natural := 3;
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,          x"00000000"),
    1 => f_sdb_embed_device(c_xwr_wb_timestamp_latch_sdb, x"00100000"),
    2 => f_sdb_embed_device(c_eca_sdb,                    x"00100800"),
    3 => f_sdb_embed_device(c_eca_evt_sdb,                x"00100C00"),
    4 => f_sdb_embed_device(c_wb_serial_lcd_sdb,          x"00100D00"),
    5 => f_sdb_embed_device(c_build_id_sdb,               x"00200000"),
    6 => f_sdb_embed_device(f_wb_spi_flash_sdb(24),       x"04000000"));
  constant c_sdb_address : t_wishbone_address := x"00300000";

  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  signal eca_master_i : t_wishbone_master_in;
  signal eca_master_o : t_wishbone_master_out;
  
  --------------------------------------------------------------
  -- Clocking
  --------------------------------------------------------------
  
  -- Non-PLL reset stuff
  signal clk_free         : std_logic;
  signal rstn_free        : std_logic;
  signal gxb_locked       : std_logic;
  signal pll_arst         : std_logic;
  
  -- Sys PLL from clk_125m_local_i
  signal sys_locked       : std_logic;
  signal clk_sys0         : std_logic;
  signal clk_sys1         : std_logic;
  signal clk_sys2         : std_logic;
  
  signal clk_sys          : std_logic;
  signal clk_reconf       : std_logic;
  signal clk_flash        : std_logic;
  signal clk_lcd          : std_logic;
  signal rstn_sys         : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal ref_locked       : std_logic;
  signal clk_ref0         : std_logic;
  signal clk_ref1         : std_logic;
  signal clk_ref2         : std_logic;
  
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_phase        : std_logic;
  signal rstn_ref         : std_logic;
  signal rstn_butis       : std_logic;
  
  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(3 downto 0);
  
  signal phase_butis      : phase_offset;
  
  -- DMTD PLL from clk_20m_vcxo_i
  signal dmtd_locked      : std_logic;
  signal clk_dmtd0        : std_logic;
  signal clk_dmtd         : std_logic;
  
  --------------------------------------------------------------
  -- White Rabbit
  --------------------------------------------------------------
  
  signal dac_hpll_load_p1 : std_logic;
  signal dac_dpll_load_p1 : std_logic;
  signal dac_hpll_data    : std_logic_vector(15 downto 0);
  signal dac_dpll_data    : std_logic_vector(15 downto 0);

  signal link_act : std_logic;
  signal link_up  : std_logic;
  signal ext_pps  : std_logic;
  signal pps      : std_logic;

  signal phy_tx_data      : std_logic_vector(7 downto 0);
  signal phy_tx_k         : std_logic;
  signal phy_tx_disparity : std_logic;
  signal phy_tx_enc_err   : std_logic;
  signal phy_rx_data      : std_logic_vector(7 downto 0);
  signal phy_rx_rbclk     : std_logic;
  signal phy_rx_k         : std_logic;
  signal phy_rx_enc_err   : std_logic;
  signal phy_rx_bitslide  : std_logic_vector(3 downto 0);
  signal phy_rst          : std_logic;
  signal phy_loopen       : std_logic;

  signal wrc_master_i  : t_wishbone_master_in;
  signal wrc_master_o  : t_wishbone_master_out;

  signal mb_src_out    : t_wrf_source_out;
  signal mb_src_in     : t_wrf_source_in;
  signal mb_snk_out    : t_wrf_sink_out;
  signal mb_snk_in     : t_wrf_sink_in;
  
  signal tm_up     : std_logic;
  signal tm_valid  : std_logic;
  signal tm_tai    : std_logic_vector(39 downto 0);
  signal tm_cycles : std_logic_vector(27 downto 0);

  constant c_channels : natural := 4;
  signal channels : t_channel_array(c_channels-1 downto 0);
  
  signal owr_pwren : std_logic_vector(1 downto 0);
  signal owr_en    : std_logic_vector(1 downto 0);
  signal owr       : std_logic_vector(1 downto 0);
  
  signal uart_rx, uart_tx : std_logic;
  
  signal sfp1_scl_o : std_logic;
  signal sfp1_scl_i : std_logic;
  signal sfp1_sda_o : std_logic;
  signal sfp1_sda_i : std_logic;
  signal sfp1_det_i : std_logic;
  
  signal eca_lemo_led : std_logic_vector(15 downto 0);
  signal eca_lvds_ecl : std_logic_vector(15 downto 0);
  signal eca_trigger  : std_logic_vector(15 downto 0);
  
  signal lemo_ttl   : std_logic;
  signal lemo_i     : std_logic_vector(8 downto 1);
  signal ref_toggle : std_logic;
  
  signal di_scp : std_logic;
  signal di_lp  : std_logic;
  signal di_flm : std_logic;
  signal di_dat : std_logic;
  signal di_bll : std_logic;
  
  signal fd_oen : std_logic;
  signal fd_o   : std_logic_vector(7 downto 0);
begin

  -- We need at least one off-chip free running clock to setup PLLs
  clk_free <= clk_20m_vcxo_i;
  
  reset : altera_reset
    generic map(
      g_clocks      => 3)
    port map(
      clk_free_i    => clk_free,
      rstn_i        => '1',
      pll_lock_i(0) => dmtd_locked,
      pll_lock_i(1) => ref_locked,
      pll_lock_i(2) => sys_locked,
      pll_lock_i(3) => gxb_locked,
      pll_arst_o    => pll_arst,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => clk_free,
      clocks_i(2)   => clk_ref,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_free,
      rstn_o(2)     => rstn_ref);

  dmtd_inst : dmtd_pll port map(
    areset => pll_arst,
    inclk0 => clk_20m_vcxo_i,    --  20  Mhz 
    c0     => clk_dmtd0,         --  62.5MHz
    locked => dmtd_locked);
  
  dmtd_clk : single_region port map(
    inclk  => clk_dmtd0,
    outclk => clk_dmtd);
  
  sys_inst : sys_pll port map(
    areset => pll_arst,
    inclk0 => clk_125m_local_i, -- 125  Mhz 
    c0     => clk_sys0,         --  62.5 MHz
    c1     => clk_sys1,         --  50  Mhz
    c2     => clk_sys2,         --  20  MHz
    locked => sys_locked);
  
  sys_clk : global_region port map(
    inclk  => clk_sys0,
    outclk => clk_sys);
  
  reconf_clk : global_region port map(
    inclk  => clk_sys1,
    outclk => clk_reconf);
  
  clk_flash <= clk_reconf;
  
  lcd_clk : single_region port map(
    inclk  => clk_sys2,
    outclk => clk_lcd);
  
  ref_inst : ref_pll port map( -- see "Phase Counter Select Mapping" table for arria2gx
    areset => pll_arst,
    inclk0 => clk_125m_pllref_i, -- 125 MHz
    c0     => clk_ref0,          -- 125 MHz, counter: 0010 - #2
    c1     => clk_ref1,          -- 200 MHz, counter: 0011 = #3
    c2     => clk_ref2,          --  25 MHz, counter: 0100 = #4
    locked => ref_locked,
    scanclk            => clk_free,
    phasedone          => phase_done,
    phasecounterselect => phase_sel,
    phasestep          => phase_step,
    phaseupdown        => '1');
  
  ref_clk : global_region port map(
    inclk  => clk_ref0,
    outclk => clk_ref);
  
  butis_clk : global_region port map(
    inclk  => clk_ref1,
    outclk => clk_butis);
  
  phase_clk : single_region port map(
    inclk  => clk_ref2,
    outclk => clk_phase);

  phase : altera_phase
    generic map(
      g_select_bits   => 4,
      g_outputs       => 1,
      g_base          => 0,
      g_vco_freq      => 1000, -- 1GHz
      g_output_freq   => (0 => 200),
      g_output_select => (0 =>   3))
    port map(
      clk_i       => clk_free,
      rstn_i      => rstn_free,
      clks_i(0)   => clk_butis,
      rstn_o(0)   => rstn_butis,
      offset_i(0) => phase_butis,
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);
  
  butis : altera_butis
    port map(
      clk_ref_i => clk_ref,
      clk_25m_i => clk_phase,
      pps_i     => pps,
      phase_o   => phase_butis);
  
  id : build_id
    port map(
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,
      slave_i => cbar_master_o(5),
      slave_o => cbar_master_i(5));
  
  flash : flash_top
    generic map(
      g_family                 => "Arria II GX",
      g_port_width             => 1,   -- single-lane SPI bus
      g_addr_width             => 24,  -- 3 byte addressed chip
      g_dummy_time             => 8,   -- 8 cycles between addr and data
      g_input_latch_edge       => '0', -- experimentally determined...
      g_output_latch_edge      => '1', -- ... and held fixed using logic lock
      g_input_to_output_cycles => 2)
    port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => cbar_master_o(6),
      slave_o   => cbar_master_i(6),
      clk_ext_i => clk_flash,
      clk_out_i => clk_flash,
      clk_in_i  => clk_flash); -- no need to phase shift at 50MHz
  
  -- open drain buffer for one wire (only one)
  
  owr(0) <= rom_data_io;
  owr(1) <= '0';
  rom_data_io <= owr_pwren(0) when (owr_pwren(0) = '1' or owr_en(0) = '1') else 'Z';
  
  -- open drain buffer for SFP i2c
  sfp1_scl_i <= sfp1_mod1_io;
  sfp1_sda_i <= sfp1_mod2_io;
  
  sfp1_det_i <= sfp1_mod0_i;
  sfp1_mod1_io <= '0' when sfp1_scl_o = '0' else 'Z';
  sfp1_mod2_io <= '0' when sfp1_sda_o = '0' else 'Z';
  
  U_WR_CORE : xwr_core
    generic map (
      g_simulation                => 0,
      g_phys_uart                 => true,
      g_virtual_uart              => false,
      g_with_external_clock_input => true,
      g_aux_clks                  => 1,
      g_ep_rxbuf_size             => 1024,
      g_dpram_initf               => "../../../ip_cores/wrpc-sw/wrc.mif",
      g_dpram_size                => 131072/4,
      g_interface_mode            => PIPELINED,
      g_address_granularity       => BYTE,
      g_aux_sdb                   => c_etherbone_sdb)
    port map (
      clk_sys_i  => clk_sys,
      clk_dmtd_i => clk_dmtd,
      clk_ref_i  => clk_ref,
      clk_aux_i  => (others => '0'),
      clk_ext_i  => lemo_i(7),
      pps_ext_i  => lemo_i(8),
      rst_n_i    => rstn_sys,

      dac_hpll_load_p1_o => dac_hpll_load_p1,
      dac_hpll_data_o    => dac_hpll_data,
      dac_dpll_load_p1_o => dac_dpll_load_p1,
      dac_dpll_data_o    => dac_dpll_data,
		
      phy_ref_clk_i      => clk_ref,
      phy_tx_data_o      => phy_tx_data,
      phy_tx_k_o         => phy_tx_k,
      phy_tx_disparity_i => phy_tx_disparity,
      phy_tx_enc_err_i   => phy_tx_enc_err,
      phy_rx_data_i      => phy_rx_data,
      phy_rx_rbclk_i     => phy_rx_rbclk,
      phy_rx_k_i         => phy_rx_k,
      phy_rx_enc_err_i   => phy_rx_enc_err,
      phy_rx_bitslide_i  => phy_rx_bitslide,
      phy_rst_o          => phy_rst,
      phy_loopen_o       => phy_loopen,
      
      led_act_o   => link_act,
      led_link_o  => link_up,
      scl_o       => open,
      scl_i       => '0',
      sda_i       => '0',
      sda_o       => open,
      sfp_scl_i   => sfp1_scl_i,
      sfp_sda_i   => sfp1_sda_i,
      sfp_scl_o   => sfp1_scl_o,
      sfp_sda_o   => sfp1_sda_o,
      sfp_det_i   => sfp1_det_i,
      btn1_i      => '0',
      btn2_i      => '0',

      uart_rxd_i => uart_rx,
      uart_txd_o => uart_tx,
      
      owr_pwren_o => owr_pwren,
      owr_en_o    => owr_en,
      owr_i       => owr,
      
      slave_i => cbar_master_o(0),
      slave_o => cbar_master_i(0),

      wrf_src_o => mb_snk_in,
      wrf_src_i => mb_snk_out,
      wrf_snk_o => mb_src_in,
      wrf_snk_i => mb_src_out,

      aux_master_o => wrc_master_o,
      aux_master_i => wrc_master_i,
 
      tm_link_up_o         => tm_up,
      tm_dac_value_o       => open,
      tm_dac_wr_o          => open,
      tm_clk_aux_lock_en_i => (others => '0'),
      tm_clk_aux_locked_o  => open,
      tm_time_valid_o      => tm_valid,
      tm_tai_o             => tm_tai,
      tm_cycles_o          => tm_cycles,
      pps_p_o              => pps,
      
      dio_o                => open,
      rst_aux_n_o          => open,
      link_ok_o            => open);

  wr_gxb_arria2 : wr_arria2_phy
    port map (
      clk_reconf_i   => clk_reconf,
      clk_pll_i      => clk_ref0,
      clk_cru_i      => sfp_ref_clk_i,
      clk_free_i     => clk_free,
      rst_i          => pll_arst,
      locked_o       => gxb_locked,
      loopen_i       => phy_loopen,
      drop_link_i    => phy_rst,
      tx_clk_i       => clk_ref,
      tx_data_i      => phy_tx_data,
      tx_k_i         => phy_tx_k,
      tx_disparity_o => phy_tx_disparity,
      tx_enc_err_o   => phy_tx_enc_err,
      rx_rbclk_o     => phy_rx_rbclk,
      rx_data_o      => phy_rx_data,
      rx_k_o         => phy_rx_k,
      rx_enc_err_o   => phy_rx_enc_err,
      rx_bitslide_o  => phy_rx_bitslide,
      pad_txp_o      => sfp1_td_o,
      pad_rxp_i      => sfp1_rd_i);

  U_DAC_ARB : spec_serial_dac_arb
    generic map (
      g_invert_sclk    => false,
      g_num_extra_bits => 8) -- AD DACs with 24bit interface
    port map (
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,

      val1_i  => dac_dpll_data,
      load1_i => dac_dpll_load_p1,

      val2_i  => dac_hpll_data,
      load2_i => dac_hpll_load_p1,

      dac_cs_n_o(0) => ndac_cs_o(1),
      dac_cs_n_o(1) => ndac_cs_o(2),
      dac_clr_n_o   => open,
      dac_sclk_o    => dac_sclk_o,
      dac_din_o     => dac_din_o);

  U_Extend_PPS : gc_extend_pulse
    generic map (
      g_width => 10000000)
    port map (
      clk_i      => clk_ref,
      rst_n_i    => rstn_ref,
      pulse_i    => pps,
      extended_o => ext_pps);
  
  U_ebone : eb_ethernet_slave
    generic map(
      g_sdb_address => x"00000000" & c_sdb_address)
    port map(
      clk_i       => clk_sys,
      nRst_i      => rstn_sys,
      snk_i       => mb_snk_in,
      snk_o       => mb_snk_out,
      src_o       => mb_src_out,
      src_i       => mb_src_in,
      cfg_slave_o => wrc_master_i,
      cfg_slave_i => wrc_master_o,
      master_o    => cbar_slave_i(1),
      master_i    => cbar_slave_o(1));
  
  ref2sys : xwb_clock_crossing
    port map(
      slave_clk_i   => clk_ref,
      slave_rst_n_i => rstn_ref,
      slave_i       => eca_master_o,
      slave_o       => eca_master_i,
      master_clk_i  => clk_sys,
      master_rst_n_i=> rstn_sys,
      master_i      => cbar_slave_o(0),
      master_o      => cbar_slave_i(0));
  
  TLU : wb_timestamp_latch
    generic map (
      g_num_triggers => 32,
      g_fifo_depth   => 10)
    port map (
      ref_clk_i                => clk_ref,
      ref_rstn_i               => rstn_ref,
      sys_clk_i                => clk_sys,
      sys_rstn_i               => rstn_sys,
      triggers_i(15 downto  0) => rc_i,
      triggers_i(23 downto 16) => lemo_i,
      triggers_i(31 downto 24) => any_i,
      tm_time_valid_i          => tm_valid,
      tm_tai_i                 => tm_tai,
      tm_cycles_i              => tm_cycles,
      wb_slave_i               => cbar_master_o(1),
      wb_slave_o               => cbar_master_i(1));

  ECA0 : wr_eca
    generic map(
      g_eca_name       => f_name("Exploder2C + DB2"),
      g_channel_names  => (f_name("GPIO: TTL Output (2-7) Side LEDs(9-12) NIM|TTL(16)"), 
                           f_name("GPIO: TRIGGER1+TRIGGER2 simultaneously (1-8)"), 
                           f_name("GPIO: LVDS Output (2-6) ECL Output (9-16)"),
                           f_name("WB:   Top-level bus controller")),
      g_log_table_size => 7,
      g_log_queue_len  => 8,
      g_num_channels   => c_channels,
      g_num_streams    => 1)
    port map(
      e_clk_i  (0)=> clk_sys,
      e_rst_n_i(0)=> rstn_sys,
      e_slave_i(0)=> cbar_master_o(3),
      e_slave_o(0)=> cbar_master_i(3),
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => cbar_master_o(2),
      c_slave_o   => cbar_master_i(2),
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_tai_i     => tm_tai,
      a_cycles_i  => tm_cycles,
      a_channel_o => channels);
  
  C0 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(0),
      gpio_o    => eca_lemo_led);
  
  C1 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(1),
      gpio_o    => eca_trigger);
  
  C2 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(2),
      gpio_o    => eca_lvds_ecl);
  
  C3 : eca_wb_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(3),
      master_o  => eca_master_o,
      master_i  => eca_master_i);
  
  GSI_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves  => c_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_layout,
     g_sdb_addr    => c_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => cbar_slave_i,
     slave_o       => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => cbar_master_i,
     master_o      => cbar_master_o);
  
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

  -- Baseboard LEDs
  hpv_o(0) <= not link_act and link_up; -- red   = traffic/no-link
  hpv_o(1) <= not link_up;              -- blue  = link
  hpv_o(2) <= not tm_valid;             -- green = timing valid
  hpv_o(3) <= not ext_pps;              -- white = PPS
  hpv_o(7 downto 4) <= not eca_lemo_led(11 downto 8); -- ECA controls other LEDs
  
  -- Baseboard logic analyzer (HPLA1)
  hpw_io(15 downto 0) <= (others => 'Z');
  -- 20 is ground
  
  -- Use output LEMOs in TTL mode
  lemo_ttl  <= not eca_lemo_led(15);
  select_o  <= lemo_ttl;
  selectn_o <= not lemo_ttl;
  
  ref_out : process(clk_ref) is
  begin
    if rising_edge(clk_ref) then
      ref_toggle <= not ref_toggle;
    end if;
  end process;
  
  -- LEMO outputs
  ttnim_o(8)          <= pps;
  ttnim_o(7 downto 2) <= eca_lemo_led(6 downto 1);
  ttnim_o(1)          <= ref_toggle;
  
  -- ECA outputs
  lvds_o(6 downto 2) <= eca_lvds_ecl(5 downto 1);
  ecl_o  <= eca_lvds_ecl(15 downto 8);
  de_o   <= eca_trigger(7 downto 0);
  
  lvds_o(8) <= clk_butis;
  lvds_o(7) <= pps;
  lvds_o(1) <= clk_ref;
  
  -- Use TRIGGER ports in type 1 mode.
  fsen1_o <= '0';
  fsen2_o <= '0';
  -- Enable control of the trigger bus
  mde_o   <= '1'; 
  -- Drive a '1' out the trigger bus on enable
  di_o <= (others => '1');
  
  -- RES is unused for now
  res_io(8 downto 1) <= (others => 'Z');
  
  -- LEMO inputs can come from TTL or NIM standard
  lemo_i <= not (ttlin_i or nimin_i);
  
  -- Extend input LEMO pulse to 20Hz on LED
  lemo_leds : for i in 1 to 8 generate
    lemo_led : gc_extend_pulse
      generic map(
        g_width => 125000000/20)
      port map(
        clk_i      => clk_ref,
        rst_n_i    => rstn_ref,
        pulse_i    => lemo_i(i),
        extended_o => led_o(i));
  end generate;
  
  -- Display back light
  -- red=nolink, blue=link+notrack, green=track
  di_bll <= '1';
  bll_o   <= 'Z' when di_bll='1' else '0';
  red_o   <= '0' when (not tm_up)                  = '1' else 'Z';
  blue_o  <= '0' when (    tm_up and not tm_valid) = '1' else 'Z';
  green_o <= '0' when (    tm_up and     tm_valid) = '1' else 'Z';
  
  -- Display
  display : wb_serial_lcd
    generic map(
      g_wait => 1,
      g_hold => 15)
    port map(
      slave_clk_i  => clk_sys,
      slave_rstn_i => rstn_sys,
      slave_i      => cbar_master_o(4),
      slave_o      => cbar_master_i(4),
      di_clk_i     => clk_lcd,
      di_scp_o     => di_scp,
      di_lp_o      => di_lp,
      di_flm_o     => di_flm,
      di_dat_o     => di_dat);
  
  discp_o <= '0' when (di_scp = '0') else 'Z';
  dilp_o  <= '0' when (di_lp  = '0') else 'Z';
  diflm_o <= '0' when (di_flm = '0') else 'Z';
  diin_o  <= '0' when (di_dat = '0') else 'Z';
  
  -- USB micro controller
  pres_o <= '0';      -- reserved pin connected to FPGA by mistake. must be ground.
  
  fd_io <= fd_o when fd_oen='1' else (others => 'Z');
  readyn_io <= 'Z'; -- weak pull-up
  
  EZUSB : ez_usb
    generic map(
      g_sdb_address => c_sdb_address)
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => cbar_slave_o(2),
      master_o  => cbar_slave_i(2),
      uart_o    => uart_rx,
      uart_i    => uart_tx,
      
      rstn_o    => sres_o,
      ebcyc_i   => ebcyc_i,
      speed_i   => speed_i,
      shift_i   => shift_i,
      fifoadr_o => fifoadr_o,
      readyn_i  => readyn_io,
      fulln_i   => fulln_i,
      emptyn_i  => emptyn_i,
      sloen_o   => sloen_o,
      slrdn_o   => slrdn_o,
      slwrn_o   => slwrn_o,
      pktendn_o => pktendn_o,
      fd_i      => fd_io,
      fd_o      => fd_o,
      fd_oen_o  => fd_oen);

end rtl;
