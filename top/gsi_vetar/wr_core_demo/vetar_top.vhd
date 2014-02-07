library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
--use work.gencores_pkg.all; -- pulse extend LEDs

entity vetar_top is
  port(
    clk_20m_vcxo_i    : in std_logic; -- N3          --  20 MHz WR VCXO clock
    clk_125m_pllref_i : in std_logic; -- AE15p AF15n -- 125 MHz WR PLL reference
    clk_125m_local_i  : in std_logic; -- D14p  C14n  -- 125 MHz local oscillator (CKPLL_125P)
    clk_pll_o         : out std_logic;-- V24p  U24p  -- clock pll output
        
    -----------------------------------------------------------------------
    -- OneWire 3.3V
    -----------------------------------------------------------------------
    rom_data_io : inout std_logic; -- E3
    
    -----------------------------------------
    -- Timing SFPs 3.3v
    -----------------------------------------
    sfp_ref_clk_i   : in    std_logic; -- AE27p AH23n
    
    sfp_td_o       : out   std_logic; -- AE24p AF24n
    sfp_rd_i       : in    std_logic; -- AG23p AH23n
    sfp_tx_fault_i : in    std_logic; -- J4 
    sfp_los_i      : in    std_logic; -- J5
    sfp_tx_dis_o   : out   std_logic; -- K9
    sfp_mod0_i     : in    std_logic; -- K7
    sfp_mod1_io    : inout std_logic; -- J8
    sfp_mod2_io    : inout std_logic; -- K8
    
    ------------------------------------------------------------------------
    -- WR DAC signals 3.3V
    ------------------------------------------------------------------------
    dac_sclk_o      : out std_logic; -- T1
    dac_din_o       : out std_logic; -- P3
    ndac_cs_o       : out std_logic_vector(2 downto 1);
    -- ndac_cs_o(1) -- AG1
    -- ndac_cs_o(2) -- AF1
    
    -----------------------------------------
    -- Logic analyzer HPLA1 2.5V
    -----------------------------------------
    hpw_io : inout std_logic_vector(15 downto 0);
    -- hpw_io( 0) -- AB16 
    -- hpw_io( 1) -- AC17
    -- hpw_io( 2) -- AC16
    -- hpw_io( 3) -- AE16
    -- hpw_io( 4) -- AF17
    -- hpw_io( 5) -- AB17
    -- hpw_io( 6) -- AD18
    -- hpw_io( 7) -- AF19
    -- hpw_io( 8) -- AF20
    -- hpw_io( 9) -- AE19
    -- hpw_io(10) -- AE20
    -- hpw_io(11) -- AE21
    -- hpw_io(12) -- AC22
    -- hpw_io(13) -- AE22
    -- hpw_io(14) -- AC23
    -- hpw_io(15) -- AC18
    
    -----------------------------------------
    -- LEDs on baseboard 2.5V
    -----------------------------------------
    -- CHANGE FOR LED_O
    leds_o : out std_logic_vector(15 downto 0);
    -- leds_o(0) --      -- leds_o(1) --
    -- leds_o(2) --      -- leds_o(3) --
    -- leds_o(4) --      -- leds_o(5) --
    -- leds_o(6) --      -- leds_o(7) --
    -- leds_o(8) --      -- leds_o(9) --
    -- leds_o(10)--      -- leds_o(11)--
    -- leds_o(12)--      -- leds_o(13)--
    -- leds_o(14)--      -- leds_o(15)--

    -----------------------------------------
    -- USB micro controller 3.3V
    -----------------------------------------
    --pres_o  is '0', it is by design 
    sres_o    : out   std_logic; -- AB8     active low reset#
    slrdn_o   : out   std_logic; -- AC10   read strobe
    slwrn_o   : out   std_logic; -- AB9    write strobe
    speed_i   : in    std_logic; -- PA0 = AF8
    shift_i   : in    std_logic; -- PA1 = AE8
    sloen_o   : out   std_logic; -- PA2 = W11
    ebcyc_i   : in    std_logic; -- PA3 = W12
    fifoadr_o : out   std_logic_vector(1 downto 0); -- 0=PA4=AC12, 1=PA5=W13
    pktendn_o : out   std_logic; -- PA6 = Y12
    readyn_io : inout std_logic; -- PA7 = AD12
    fulln_i   : in    std_logic; -- CTL1 = AA9
    emptyn_i  : in    std_logic; -- CTL2 = AB10
    fd_io     : inout std_logic_vector(7 downto 0); -- FIFO bus
                                                    -- AH2,AA10,AC6,AH3,
                                                    -- Y10,AD6,W10,Y1
	 -----------------------------------------------------------------------
    -- RAM
    -----------------------------------------------------------------------
--    ram_gw	   :	in		std_logic;					      -- Synchronous Global Write Enable
--    ram_bwe	   :	out	std_logic;							-- Synchronous Byte Write Enable
--    ram_bwx	   :	out	std_logic_vector(3 downto 0);	-- Synchronous Byte Write Enable
--    ram_oe	   :	out 	std_logic;					      -- Output Enable
--    ram_ce	   :	out	std_logic_vector(1 downto 0);	-- Synchronous Chip Enable
--    ram_adv	   :	out	std_logic;							-- Synchronous Burst Write Enable
--    ram_adsc   :	out	std_logic;							-- Synchronous Controller Address Status
--    ram_adsp   :	out	std_logic;							-- Synchronous Processor Address Status
--    ram_address:	in		std_logic_vector(18 downto 0);
--    ram_data	:	inout	std_logic_vector(31 downto 0);
--    ram_clk    :  out   std_logic;

	 -----------------------------------------------------------------------
    -- Display
    -----------------------------------------------------------------------	
 	 di_o    : out std_logic_vector(3 downto 0);
    -- di[0] AH7-DIS0 DIN  shift register in ?????
    -- di[0] Y14-DIS0 DOUT shift register out?????
    -- di[1] AD7-DIS1 LP   latch pulse (end-of-40-bit-row)
    -- di[2] AH8-DIS2 FLM  first-line marker
    -- di[3] AC7-DIS3 SCP  clock

 	 --di_i    : in std_logic;
    -- di_i  AH7-DIS0 DIN  shift register in????

	 color_o : out std_logic_vector(2 downto 0);
    -- color[0] AH4-BLU   Blue DON'T drive this pin fast, close to pll!!!
    -- color[1] AH5-DIS4  Green
    -- color[2] AH13-DIS5 Red
 	 
    -----------------------------------------------------------------------
     -- VME bus
	 -----------------------------------------------------------------------
    vme_as_n_i          : in    std_logic;   -- M3
    vme_rst_n_i         : in    std_logic;   -- J1
    vme_write_n_i       : in    std_logic;   -- M4
    vme_am_i            : in    std_logic_vector(5 downto 0); 
    vme_ds_n_i          : in    std_logic_vector(1 downto 0);
    vme_ga_i            : in    std_logic_vector(3 downto 0);
    vme_addr_data_b     : inout std_logic_vector(31 downto 0);
    vme_iackin_n_i      : in    std_logic;   -- H1
    vme_iackout_n_o     : out   std_logic;   -- E1
    vme_iack_n_i        : in    std_logic;   -- K3
    vme_irq_n_o         : out   std_logic_vector(6 downto 0);
    vme_berr_o          : out   std_logic;   -- D1
    vme_dtack_oe_o      : out   std_logic;   -- L4
	 vme_buffer_latch_o  : out   std_logic_vector(3 downto 0);
    vme_data_oe_ab_o    : out   std_logic;   -- L7
    vme_data_oe_ba_o    : out   std_logic;   -- J3
    vme_addr_oe_ab_o    : out   std_logic;   -- E4
    vme_addr_oe_ba_o    : out   std_logic;   -- L1
    
    -----------------------------------------
    -- LEMO on front panel NIM/TTL
    -----------------------------------------
    lemo_i           	: in std_logic;   -- K4
    lemo_o           	: out std_logic;  -- H4
    lemo_o_en_o      	: out std_logic;  -- H3

    -----------------------------------------
    -- VETAR1DB1 ADD-ON Board 
    -----------------------------------------
	 
    -- LVDS
	 lvds_in_i				: in  std_logic_vector(1 downto 0);
    -- leds_in_i[0]  AH16  PG1P12-73
    -- leds_in_i[1]  AH17  PG1N12-75
	 lvds_out_o				: out std_logic_vector(1 downto 0);
    -- leds_out_o[0] AE18 PG1P13-79 
    -- leds_out_o[1] AF18 PG1N13-81

	 leds_lvds_out_o	   : out std_logic_vector(1 downto 0);
    -- leds_lvds_out_o[0] D6p PG2P12-73  
    -- DON'T DRIVE FAST, CLOSE TO PLL!!
    -- leds_lvds_out_o[1] C5 PG2N12-75 
    -- DON'T DRIVE FAST, CLOSE TO PLL!!

	 -- HDMI
	 hdmi_o					: out std_logic_vector(1 downto 0);
    -- hdmi_o[0] F10p PG2P15-91  E10n PG2N15-93
    -- hdmi_o[1] F9p  PG2P16-99  F8n  PG2N16-97
	 hdmi_i					: in  std_logic;
	 -- hdmi_i    E9p  PG1P4-23   D9n PG1N4-25

	 -- Output LEMOs and LEDS
	 lemo_addOn_o			: out std_logic_vector(5 downto 0);
    -- lemo_addOn_o[0] Y16  lemo_addOn_o[1] AA16 lemo_addOn_o[2] AC15
    -- lemo_addOn_o[3] AD15 lemo_addOn_o[4] AA15 lemo_addOn_o[5] AB15
    -- lemo_addOn_o[4] DON'T DRIVE FAST, CLOSE TO PLL!!

	 leds_lemo_addOn_o	: out std_logic_vector(5 downto 0);
	 -- lemo_lemo_addOn_o[0] E7  lemo_lemo_addOn_o[1] D7 lemo_lemo_addOn_o[2] C7
    -- lemo_lemo_addOn_o[3] C6  lemo_lemo_addOn_o[4] B6 lemo_lemo_addOn_o[5] A6
    -- lemo_lemo_addOn_o[3 downto 0] DON'T DRIVE FAST, CLOSE TO PLL!!
  
    -- NIM/TTL LEMOs
	 lemo_nim_ttl_i		: in  std_logic_vector(1  downto 0));
    -- lemo_nim_ttl_i[0] E6-PG1P1 DON'T DRIVE FAST, CLOSE TO PLL!!
    -- lemo_nim_ttl_i[1] D5-PG1N1 DON'T DRIVE FAST, CLOSE TO PLL!!
end vetar_top;

architecture rtl of vetar_top is
  
  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;
  
  signal s_clk_ref      : std_logic;
  signal s_clk_butis    : std_logic;
  signal s_dedicated_out: std_logic;

  signal s_lemo_addOn   : std_logic_vector(5 downto 0);
  signal s_lvds_out     : std_logic_vector(1 downto 0);
  
  signal s_di_scp       : std_logic;
  signal s_di_flm       : std_logic;
  signal s_di_lp        : std_logic; 
  signal s_di_dat       : std_logic;
 
  constant c_black      : std_logic_vector 	:= "111";
  constant c_red        : std_logic_vector   := "101";
  constant c_green      : std_logic_vector 	:= "110";
  constant c_blue       : std_logic_vector 	:= "011";
  
begin

  main : monster
    generic map(
      g_family     => "Arria II",
      g_project    => "vetar_top",
      g_gpio_in    => 4,
      g_gpio_out   => 8,
      g_flash_bits => 24,
      g_en_vme     => true,
      g_en_usb     => true,
      g_en_lcd     => true)
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_clk_wr_ref_o      => s_clk_ref,
      core_clk_butis_o       => s_clk_butis,
      -- gpio
      gpio_o( 5 downto  0)   => s_lemo_addOn(5 downto 0),
      gpio_o( 7 downto  6)   => s_lvds_out(1 downto 0),

      gpio_i( 1 downto  0)   => lvds_in_i(1 downto 0),
      gpio_i( 2 )            => hdmi_i,
      --gpio_i( 2 )            => open,
      gpio_i( 3 )            => lemo_i,
     -- wr core
      wr_onewire_io          => rom_data_io,
      wr_sfp_sda_io          => sfp_mod2_io,
      wr_sfp_scl_io          => sfp_mod1_io,
      wr_sfp_det_i           => sfp_mod0_i,
      wr_sfp_tx_o            => sfp_td_o,
      wr_sfp_rx_i            => sfp_rd_i,
      wr_dac_sclk_o          => dac_sclk_o,
      wr_dac_din_o           => dac_din_o,
      wr_ndac_cs_o           => ndac_cs_o,
      wr_ext_clk_i           => lemo_nim_ttl_i(1),
      wr_ext_pps_i           => lemo_nim_ttl_i(0),
      led_link_up_o          => s_led_link_up,
      led_link_act_o         => s_led_link_act,
      led_track_o            => s_led_track,
      led_pps_o              => s_led_pps,
      -- vme
      vme_as_n_i             => vme_as_n_i,
      vme_rst_n_i            => vme_rst_n_i,
      vme_write_n_i          => vme_write_n_i,
      vme_am_i               => vme_am_i,
      vme_ds_n_i             => vme_ds_n_i,
      vme_ga_i               => vme_ga_i,
      vme_addr_data_b        => vme_addr_data_b,
      vme_iack_n_i           => vme_iack_n_i,
      vme_iackin_n_i         => vme_iackin_n_i,
      vme_iackout_n_o        => vme_iackout_n_o,
      vme_irq_n_o            => vme_irq_n_o,
      vme_berr_o             => vme_berr_o,
      vme_dtack_oe_o         => vme_dtack_oe_o,
      vme_buffer_latch_o     => vme_buffer_latch_o,
      vme_data_oe_ab_o       => vme_data_oe_ab_o,
      vme_data_oe_ba_o       => vme_data_oe_ba_o,
      vme_addr_oe_ab_o       => vme_addr_oe_ab_o,
      vme_addr_oe_ba_o       => vme_addr_oe_ba_o,
        -- usb
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
      -- lcd
      lcd_scp_o              => s_di_scp,
      lcd_lp_o               => s_di_lp,
      lcd_flm_o              => s_di_flm,
      lcd_in_o               => s_di_dat);

  -- SFP
  ----------------
  sfp_tx_dis_o <= '0'; -- enable SFP
  
  -- Baseboard logic analyzer
  ------------------------------
  hpw_io(15 downto 0) <= (others => 'Z');

  -- Display
  ----------------
  di_o(3) <= '0' when (s_di_scp = '0') else 'Z'; -- clock (run at 2MHz)                            
  di_o(2) <= '0' when (s_di_flm = '0') else 'Z'; -- first-line marker
  di_o(1) <= '0' when (s_di_lp  = '0') else 'Z'; -- latch pulse (end-of-40-bit-row)  
  di_o(0) <= '0' when (s_di_dat = '0') else 'Z'; -- shift register in
  
  -- red=nolink, blue=link+notrack, green=track
  color_o <= c_red 	   when (not s_led_link_up) 			               else
			    c_blue  	when (    s_led_link_up and not s_led_track)  	else
			    c_green    when (    s_led_link_up and     s_led_track)    else
			    c_black;          

  -- On board leds
  -----------------
   -- Link Activity
	--!!! DON'T USE leds_o(0) leds_o(1) are too close to
	-- a clock and they can be harmfull
  leds_o(15)		<= not (s_led_link_act and s_led_link_up); -- Link active
  leds_o(14)		<= not s_led_link_up;						    -- Link up
  leds_o(13)		<=	not s_led_track;						       -- Timing Valid
  leds_o(12)	   <= not s_led_pps;
  
  -- not assigned leds
  leds_o(11 downto 0)	<= (others => '1'); -- power off
  
  -- On board lemo
  ---------------- 
  -- PPS output
  lemo_o_en_o 	<= '0';
  lemo_o		  	<= s_led_pps;

  -- VETAR1DB1 ADD-ON Board
  -------------------------
   -- LEMO outputs GPIO0-5
  lemo_addOn_o       <= s_lemo_addOn;
  leds_lemo_addOn_o  <= not s_lemo_addOn;
   
  -- LVDS outputs GPIO6-8
  lvds_out_o        <= s_lvds_out;
  leds_lvds_out_o   <= not s_lvds_out;

  -- HDMI
  hdmi_o(0) <= s_clk_butis;
  hdmi_o(1) <= s_clk_ref;
  clk_pll_o <= s_clk_butis;
  
end rtl;
