library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.stub_pll_pkg.all;

-- Xilinx-105 FMC Debug Card Mapping and Layout (connected to FMC_A an/or FMC_B)
-- <<PIN ID AT DEBUG CARD>> <<NAME>> <<PIN AT FPGA>> <<BANK OR CPLD>> <<FMC PORT>> <<SPECIAL FUNCTION>>
-- --------------------------------------------------------------------------------------------
--
--
--
-- J1: 1..40 aka FMC-LA[00:19]
-- --------------------------------------------------------------------------------------------
-- 01 FMC_LA00_CC_P => PIN_G14@3H@PORT_A                           02 FMC_LA10_P => PIN_A7@3H@PORT_A
-- 03 FMC_LA00_CC_N => PIN_H14@3H@PORT_A                           04 FMC_LA10_N => PIN_A8@3H@PORT_A
-- 05 FMC_LA01_CC_P                                                06 FMC_LA11_P => PIN_C9@3H@PORT_A
-- 07 FMC_LA01_CC_N                                                08 FMC_LA11_N => PIN_D9@3H@PORT_A
-- 09 FMC_LA02_P => PIN_C13@3H@PORT_A                              10 FMC_LA12_P => PIN_M12@3G@PORT_A
-- 11 FMC_LA02_N => PIN_D13@3H@PORT_A                              12 FMC_LA12_N => PIN_N13@3G@PORT_A
-- 13 FMC_LA03_P => PIN_C14@3H@PORT_A                              14 FMC_LA13_P => PIN_J11@3G@PORT_A
-- 15 FMC_LA03_N => PIN_D14@3H@PORT_A                              16 FMC_LA13_N => PIN_K11@3G@PORT_A
-- 17 FMC_LA04_P => PIN_H12@3H@PORT_A, RZQ_3H                      18 FMC_LA14_P => PIN_J9@3G@PORT_A
-- 19 FMC_LA04_N => PIN_H13@3H@PORT_A                              20 FMC_LA14_N => PIN_J10@3G@PORT_A
-- 21 FMC_LA05_P                                                   22 FMC_LA15_P => PIN_D4@3G@PORT_A
-- 23 FMC_LA05_N                                                   24 FMC_LA15_N => PIN_D5@3G@PORT_A
-- 25 FMC_LA06_P => PIN_A10@3H@PORT_A                              26 FMC_LA16_P => PIN_D6@3G@PORT_A, RZQ_3G
-- 27 FMC_LA06_N => PIN_B10@3H@PORT_A                              28 FMC_LA16_N => PIN_E6@3G@PORT_A
-- 29 FMC_LA07_P => PIN_A9@3H@PORT_A                               30 FMC_LA17_CC_P => PIN_F9@3G@PORT_A
-- 31 FMC_LA07_N => PIN_B9@3H@PORT_A                               32 FMC_LA17_CC_N => PIN_G9@3G@PORT_A
-- 33 FMC_LA08_P => PIN_B11@3H@PORT_A                              34 FMC_LA18_CC_P => PIN_G7@3G@PORT_A
-- 35 FMC_LA08_N => PIN_B12@3H@PORT_A                              36 FMC_LA18_CC_N => PIN_H7@3G@PORT_A
-- 37 FMC_LA09_P => PIN_A12@3H@PORT_A                              38 FMC_LA19_P => PIN_G5@3G@PORT_A
-- 39 FMC_LA09_N => PIN_A13@3H@PORT_A                              40 FMC_LA19_N => PIN_G6@3G@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J2: 1..40 aka FMC_HB[00:19]
-- --------------------------------------------------------------------------------------------
-- 01 FMC_HB00_CC_P => PIN_J4@3F@PORT_A                            02 FMC_HB10_P => PIN_L4@3F@PORT_A
-- 03 FMC_HB00_CC_N => PIN_J5@3F@PORT_A                            04 FMC_HB10_N => PIN_M4@3F@PORT_A
-- 05 FMC_HB01_P => PIN_H1@3F@PORT_A                               06 FMC_HB11_P => PIN_G14@MAXV@PORT_A
-- 07 FMC_HB01_N => PIN_J1@3F@PORT_A                               08 FMC_HB11_N => PIN_F16@MAXV@PORT_A
-- 09 FMC_HB02_P => PIN_H3@3F@PORT_A                               10 FMC_HB12_P => PIN_G13@MAXV@PORT_A
-- 11 FMC_HB02_N => PIN_H4@3F@PORT_A                               12 FMC_HB12_N => PIN_G15@MAXV@PORT_A
-- 13 FMC_HB03_P => PIN_K1@3F@PORT_A                               14 FMC_HB13_P => PIN_N8@3F@PORT_A
-- 15 FMC_HB03_N => PIN_K2@3F@PORT_A                               16 FMC_HB13_N => PIN_M9@3F@PORT_A
-- 17 FMC_HB04_P => PIN_L2@3F@PORT_A, RZQ_3F                       18 FMC_HB14_P => PIN_G12@MAXV@PORT_A
-- 19 FMC_HB04_N => PIN_L3@3F@PORT_A                               20 FMC_HB14_N => PIN_G16@MAXV@PORT_A
-- 21 FMC_HB05_P => PIN_J3@3F@PORT_A                               22 FMC_HB15_P => PIN_H14@MAXV@PORT_A
-- 23 FMC_HB05_N => PIN_K3@3F@PORT_A                               24 FMC_HB15_N => PIN_H15@MAXV@PORT_A
-- 25 FMC_HB06_CC_P => PIN_N6@3F@PORT_A                            26 FMC_HB16_P => PIN_J13@MAXV@PORT_A
-- 27 FMC_HB06_CC_N => PIN_N7@3F@PORT_A                            28 FMC_HB16_N => PIN_J16@MAXV@PORT_A
-- 29 FMC_HB07_P => PIN_K5@3F@PORT_A                               30 FMC_HB17_CC_P => PIN_H13@MAXV@PORT_A
-- 31 FMC_HB07_N => PIN_K6@3F@PORT_A                               32 FMC_HB17_CC_N => PIN_H16@MAXV@PORT_A
-- 33 FMC_HB08_P => PIN_K7@3F@PORT_A                               34 FMC_HB18_P => PIN_E14@MAXV@PORT_A
-- 35 FMC_HB08_N => PIN_L7@3F@PORT_A                               36 FMC_HB18_N => PIN_C14@MAXV@PORT_A
-- 37 FMC_HB09_P => PIN_M6@3F@PORT_A                               38 FMC_HB19_P => PIN_C15@MAXV@PORT_A
-- 39 FMC_HB09_N => PIN_M7@3f@PORT_A                               40 FMC_HB19_N => PIN_E13@MAXV@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J3: 1..40 aka FMC_HA[00:19]
-- --------------------------------------------------------------------------------------------
-- 01 FMC_HA00_CC_P => PIN_P14@3H@PORT_A                           02 FMC_HA10_P => PIN_D10@3H@PORT_A
-- 03 FMC_HA00_CC_N => PIN_P15@3H@PORT_A                           04 FMC_HA10_N => PIN_E10@3H@PORT_A
-- 05 FMC_HA01_CC_P => PIN_M14@3H@PORT_A                           06 FMC_HA11_P => PIN_G11@3H@PORT_A
-- 07 FMC_HA01_CC_N => PIN_N14@3H@PORT_A                           08 FMC_HA11_N => PIN_H11@3H@PORT_A
-- 09 FMC_HA02_P => PIN_J13@3H@PORT_A                              10 FMC_HA12_P => PIN_E8@3G@PORT_A
-- 11 FMC_HA02_N => PIN_J14@3H@PORT_A                              12 FMC_HA12_N => PIN_F8@3G@PORT_A
-- 13 FMC_HA03_P => PIN_L14@3H@PORT_A                              14 FMC_HA13_P => PIN_B7@3G@PORT_A
-- 15 FMC_HA03_N => PIN_L15@3H@PORT_A                              16 FMC_HA13_N => PIN_C7@3G@PORT_A
-- 17 FMC_HA04_P => PIN_L12@3H@PORT_A                              18 FMC_HA14_P => PIN_C8@3G@PORT_A
-- 19 FMC_HA04_N => PIN_L13@3H@PORT_A                              20 FMC_HA14_N => PIN_D8@3G@PORT_A
-- 21 FMC_HA05_P => PIN_K12@3H@PORT_A                              22 FMC_HA15_P => PIN_C6@3G@PORT_A
-- 23 FMC_HA05_N => PIN_K13@3H@PORT_A                              24 FMC_HA15_N => PIN_B6@3G@PORT_A
-- 25 FMC_HA06_P => PIN_C11@3H@PORT_A                              26 FMC_HA16_P => PIN_A5@3G@PORT_A
-- 27 FMC_HA06_N => PIN_C12@3H@PORT_A                              28 FMC_HA16_N => PIN_B5@3G@PORT_A
-- 29 FMC_HA07_P => PIN_D11@3H@PORT_A                              30 FMC_HA17_CC_P => PIN_A4@3G@PORT_A
-- 31 FMC_HA07_N => PIN_E11@3H@PORT_A                              32 FMC_HA17_CC_N => PIN_B4@3G@PORT_A
-- 33 FMC_HA08_P => PIN_F12@3H@PORT_A                              34 FMC_HA18_P => PIN_J18@MAXV@PORT_A
-- 35 FMC_HA08_N => PIN_G12@3H@PORT_A                              36 FMC_HA18_N => PIN_J19@MAXV@PORT_A
-- 37 FMC_HA09_P => PIN_F10@3H@PORT_A                              38 FMC_HA19_P => PIN_F19@MAXV@PORT_A
-- 39 FMC_HA09_N => PIN_G10@3H@PORT_A                              40 FMC_HA19_N => PIN_F20@MAXV@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J6: 1..6
-- --------------------------------------------------------------------------------------------
-- 01 VCC3V                                                        02 VCC3V3
-- 03 HDR_POWER                                                    04 HDR_POWER
-- 05 VADJ                                                         06 VADJ
-- --------------------------------------------------------------------------------------------
--
--
--
-- J16: 1..12 aka FMC_LA [28:31]
-- --------------------------------------------------------------------------------------------
-- 01 HDR_POWER                                                    02 HDR_POWER
-- 03 GND                                                          04 GND
-- 05 FMC_LA28_P => PIN_L5@3F@PORT_A                               06 FMC_LA30_P => PIN_P9@3F@PORT_A
-- 07 FMC_LA28_N => PIN_M5@3F@PORT_A                               08 FMC_LA30_N => PIN_R10@3F@PORT_A
-- 09 FMC_LA29_P => PIN_N9@3F@PORT_A                               10 FMC_LA31_P => PIN_P8@3F@PORT_A
-- 11 FMC_LA29_N => PIN_P10@3F@PORT_A                              12 FMC_LA31_N => PIN_R8@3F@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J15: LEDs
-- --------------------------------------------------------------------------------------------
-- 01 VADJ
-- 02 GND
-- 03 FMC_LA32_P, LED-GRN => PIN_L8@3F@PORT_A
-- 05 FMC_LA32_N, LED-GRN => PIN_L9@3F@PORT_A
-- 07 FMC_LA33_P, LED-GRN => PIN_P11@3F@PORT_A
-- 09 FMC_LA33_N, LED-GRN => PIN_R11@3F@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J23: 1..12
-- --------------------------------------------------------------------------------------------
-- 01 FMC_HB20_P => PIN_E12@MAXV@PORT_B                            02 FMC_HB21_P => PIN_P6@MAXV@PORT_B
-- 03 FMC_HB20_N => PIN_D15@MAXV@PORT_B                            04 FMC_HB21_N => PIN_N6@MAXV@PORT_B
-- 05 FMC_HA20_P => PIN_K10@3G@PORT_A                              06 FMC_HA22_P => PIN_E7@3G@PORT_A
-- 07 FMC_HA20_N => PIN_L10@3G@PORT_A                              08 FMC_HA22_N => PIN_F7@3G@PORT_A
-- 09 FMC_HA21_P => PIN_M10@3G@PORT_A                              10 FMC_HA23_P => PIN_H6@3G@PORT_A
-- 11 FMC_HA21_N => PIN_N11@3G@PORT_A                              12 FMC_HA23_N => PINJ6@3G@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- J20: 1..16 aka FMC_HA [20:27]
-- --------------------------------------------------------------------------------------------
-- 01 FMC_LA20_P => PIN_C3@3G@PORT_A                               02 FMC_LA24_P => PIN_E1@3G@PORT_A
-- 03 FMC LA20_N => PIN_C4@3G@PORT_A                               04 FMC_LA24_N => PIN_E2@3G@PORT_A
-- 05 FMC LA21_P => PIN_C2@3G@PORT_A                               06 FMC_LA25_5 => PIN_E3@3G@PORT_A
-- 07 FMC LA21_N => PIN_D3@3G@PORT_A                               08 FMC_LA25_N => PIN_F3@3G@PORT_A
-- 09 FMC LA22_P => PIN_F4@3F@PORT_A                               10 FMC_LA26_P => PIN_F2@3G@PORT_A
-- 11 FMC LA22_N => PIN_G4@3F@PORT_A                               12 FMC_LA26_N => PIN_G2@3G@PORT_A
-- 13 FMC LA23_P => PIN_C1@3F@PORT_A                               14 FMC_LA27_P => PIN_G1@3G@PORT_A
-- 15 FMC LA23_N => PIN_D1@3F@PORT_A                               16 FMC_LA27_N => PIN_H2@3G@PORT_A
-- --------------------------------------------------------------------------------------------
--
--
--
-- MISC
-- --------------------------------------------------------------------------------------------
-- FA_LA_DEV_CLK_P PIN_E12@3H@PORT_A
-- FA_LA_DEV_CLK_N PIN_E13@3H@PORT_A
-- FA_LA_SYS_REF_P PIN_F13@3H@PORT_A
-- FA_LA_SYS_REF_N PIN_F13@3H@PORT_A
-- FPGA_RCLK_P PIN_J8@3G
-- FPGA_RCLK_N PIN_K8@3G

entity pci_control is
  port(
    --clk_20m_vcxo_i    : in std_logic; -- 20MHz VCXO clock
    --clk_125m_pllref_i : in std_logic; -- 125 MHz PLL reference
    --clk_125m_local_i  : in std_logic; -- local clk from 125Mhz oszillator
    --clk_sfp_i         : in std_logic; -- Reserved
    clk_125m_sfpref_i : in std_logic; -- 125 MHz PLL/SFP reference
    clk_pll_i           : in std_logic; -- Stub PLL source, 100 MHz

    -----------------------------------------
    -- PCI express pins
    -----------------------------------------
    pcie_refclk_i  : in    std_logic;
    pcie_rx_i      : in    std_logic_vector(3 downto 0);
    pcie_tx_o      : out   std_logic_vector(3 downto 0);
    nPCI_RESET     : in    std_logic; -- PCIE1V8_PERSTn
    nPCI_RESET1    : in    std_logic; -- PCIE1V8_PERST1n
    --pe_smdat       : inout std_logic; -- unused (needed for CvP)
    --pe_smclk       : out   std_logic := 'Z';
    --pe_waken       : out   std_logic := 'Z';

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk       : out std_logic;                    -- FMC/XILINX-105 HEADER J20 => PIN 1 => PIN_C3
    wr_dac_din        : out std_logic;                    -- FMC/XILINX-105 HEADER J20 => PIN 3 => PIN_C4
    wr_ndac_cs        : out std_logic_vector(2 downto 1); -- FMC/XILINX-105 HEADER J20 => PIN 5 => PIN_C2 [2]
                                                          -- FMC/XILINX-105 HEADER J20 => PIN 7 => PIN_D3 [1]

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data        : inout std_logic;                    -- FMC/XILINX-105 HEADER J20 => PIN 9 => PIN_F4

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    fpga_res        : in std_logic; -- Normally this comes from CPLD
	                                  -- At CPLD this is also defines as input, => Force it to zero, use DIP switch in final design
	                                  -- At Exploder5, this pin is called "IO_RZQ_5"
    nres            : in std_logic; -- At Exploder5, this pin is called "IO", comes from "RESET2" (which is a voltage watchdog), goes to CPLD mres1
	 	                                -- At CPLD this is also defines as input, => Force it to zero, use DIP switch in final design

    -----------------------------------------------------------------------
    -- LVTTL IOs
    -----------------------------------------------------------------------
    lemo_p_i          : in    std_logic_vector(1 downto 0); -- Bank E3???
    lemo_n_i          : in    std_logic_vector(1 downto 0);
    lemo_p_o          : out   std_logic_vector(1 downto 0);
    lemo_n_o          : out   std_logic_vector(1 downto 0);

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds          : out std_logic_vector(3 downto 0) := (others => '1'); -- FMC/XILINX-105 HEADER J15 / LED => L8[3]
                                                                            -- FMC/XILINX-105 HEADER J15 / LED => L9[2]
                                                                            -- FMC/XILINX-105 HEADER J15 / LED => P11[1]
                                                                            -- FMC/XILINX-105 HEADER J15 / LED => R11[0]
    rt_leds          : out std_logic_vector(3 downto 0) := (others => '1'); -- FMC/XILINX-105 HEADER J16 => L5[3]
                                                                            -- FMC/XILINX-105 HEADER J16 => M5[2]
                                                                            -- FMC/XILINX-105 HEADER J16 => N9[1]
                                                                            -- FMC/XILINX-105 HEADER J16 => P10[0]

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_tx_disable_o : out   std_logic := '0'; -- SFPB_TXDISABLE, connected to MAX V, PIN N7 -- FMC/XILINX-105 @ J1 -> PIN 17 => FMC_LA04_P LEAVE UNCONNECTED
    sfp_tx_fault     : in    std_logic; -- SFPB_TXFAULT, connected to MAX V, PIN T7 -- FMC/XILINX-105 @ J1 -> PIN 19 => FMC_LA04_N, LEAVE UNCONNECTED
    sfp_los          : in    std_logic; -- SFPB_LOS, connected to MAX V, PIN_P8 -- FMC/XILINX-105 @ J1 -> PIN 21 => FMC_LA05_P, LEAVE UNCONNECTED
    sfp_txp_o        : out   std_logic; -- SFPB_TX_P/N, P = AW37, N = AW36
    sfp_rxp_i        : in    std_logic; -- SFPB_RX_P/N, P = AT31, N = AT30
    sfp_mod0         : in    std_logic; -- SFPB_MOD0_PRSNTn (grounded by module, DET), connectd to MAX V, PIN R8, LEAVE UNCONNECTED
    sfp_mod1         : inout std_logic; -- SCL, EXTB_SCL, A10_2L_SCL, to MAX V, PIN P4, to HPS shared IOs, M20 @ FPGA
    sfp_mod2         : inout std_logic); -- SDA, EXTB_SDA, A10_2L_SDA, to MAX V, PIN R1, to HPS shared IOs, L20 @ FPGA

end pci_control;

architecture rtl of pci_control is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_gpio_o       : std_logic_vector(3 downto 0);
  signal s_lvds_p_i     : std_logic_vector(1 downto 0);
  signal s_lvds_n_i     : std_logic_vector(1 downto 0);
  signal s_lvds_p_o     : std_logic_vector(1 downto 0);
  signal s_lvds_n_o     : std_logic_vector(1 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_125m_sfpref_i    : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  signal s_pcie_reset_n         : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 5) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1_BASE_R",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED2_BASE_B",  IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED3_BASE_G",  IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LED4_BASE_W",  IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LVDS_DUMMY1",  IO_NONE,         false,   false,  0,     IO_INOUTPUT, IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDS_DUMMY2",  IO_NONE,         false,   false,  1,     IO_INOUTPUT, IO_LVDS,  false,        false,       IO_LVDS)
  );

  constant c_family        : string := "Arria 10 SX";
  constant c_project       : string := "pci_control";
  constant c_cores         : natural:= 1;
  constant c_initf_name    : string := c_project & "_stub.mif";
  constant c_profile_name  : string := "medium_icache_debug";

begin

  main : monster
    generic map(
      g_family           => c_family,
      g_project          => c_project,
      g_flash_bits       => 25, -- !!! TODO: Check this
      g_gpio_out         => 4,
      g_lvds_inout       => 2,
      g_en_pcie          => true,
      g_en_tlu           => false,
      g_en_usb           => false,
      g_io_table         => io_mapping_table,
      g_en_tempsens      => false,
      g_lm32_cores       => c_cores,
      g_lm32_ramsizes    => c_lm32_ramsizes/4,
      g_lm32_init_files  => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles    => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i     => s_clk_20m_vcxo_i,
      core_clk_125m_pllref_i  => s_clk_125m_pllref_i,
      core_clk_125m_local_i   => s_clk_125m_local_i,
      core_clk_125m_sfpref_i  => clk_125m_sfpref_i,
      wr_onewire_io           => rom_data,
      wr_sfp_sda_io           => sfp_mod2,
      wr_sfp_scl_io           => sfp_mod1,
      wr_sfp_det_i            => sfp_mod0,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_dac_sclk_o           => wr_dac_sclk,
      wr_dac_din_o            => wr_dac_din,
      wr_ndac_cs_o            => wr_ndac_cs,
      sfp_tx_disable_o        => open,
      sfp_tx_fault_i          => sfp_tx_fault,
      sfp_los_i               => sfp_los,
      gpio_o                  => s_gpio_o,
      lvds_p_i                => s_lvds_p_i,
      lvds_n_i                => s_lvds_n_i,
      lvds_p_o                => s_lvds_p_o,
      lvds_n_o                => s_lvds_n_o,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o);

  -- PCIe reset
  p_pcie_reset : process(nPCI_RESET, nPCI_RESET1)
  begin
	 if (nPCI_RESET = '1' and nPCI_RESET1 = '1') then
      s_pcie_reset_n <= '1';
    else
      s_pcie_reset_n <= '0';
    end if;
  end process;

  -- SFP(nPCI_RESET or nPCI_RESET1)
  sfp_tx_disable_o <= '0';

  -- LEDs
  wr_leds(0) <= not (s_led_link_act and s_led_link_up); -- red   = traffic/no-link
  wr_leds(1) <= not s_led_link_up;                      -- blue  = link
  wr_leds(2) <= not s_led_track;                        -- green = timing valid
  wr_leds(3) <= not s_led_pps;                          -- white = PPS
  --led_fpg    <= not s_led_link_up;
  --led_fpr    <= not s_led_link_act;
  rt_leds    <= not s_gpio_o;

  stub_inst : stub_pll_altera_iopll_160_z2kwsvq port map(
    rst        => s_stub_pll_reset,
    refclk     => clk_pll_i,
    locked     => s_stub_pll_locked,
    outclk_0   => s_clk_20m_vcxo_i,
    outclk_1   => s_clk_125m_pllref_i,
    outclk_2   => s_clk_125m_local_i,
    outclk_3   => s_clk_125m_sfpref_i);

  -- PLL reset, don't do this at home
  p_stub_pll_reset : process(clk_pll_i)
  begin
    if (rising_edge(clk_pll_i)) then
      s_stub_pll_locked_prev <= s_stub_pll_locked;
      if (s_stub_pll_reset = '1') then
        s_stub_pll_reset <= '0';
      elsif (s_stub_pll_locked = '0' and s_stub_pll_locked_prev = '1') then
        s_stub_pll_reset <= '1';
      else
        s_stub_pll_reset <= '0';
      end if;
    end if;
  end process;

  -- LEMOs
  lemos : for i in 0 to 1 generate
    s_lvds_p_i(i)      <= lemo_p_i(i);
    s_lvds_n_i(i)      <= lemo_n_i(i);
    lemo_p_o(i)        <= s_lvds_p_o(i);
    lemo_n_o(i)        <= s_lvds_n_o(i);
  end generate;

end rtl;
