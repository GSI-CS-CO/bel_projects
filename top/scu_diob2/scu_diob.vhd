library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.fg_quad_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;



entity scu_diob is
generic (
  CLK_sys_in_Hz : integer := 125000000;
  g_card_type   : string  := "diob"
);

port (
  ------------------------------ Clocks -------------------------------------------------------------------------
  CLK_20MHz_A: in std_logic; -- Clock_A
  CLK_20MHz_B: in std_logic; -- Clock_B
  CLK_20MHz_C: in std_logic; -- Clock_C
  CLK_20MHz_D: in std_logic; -- Clock_D

  --------- parallel scu bus signals ----------------------------------------------------------------------------
  A_A                   : in std_logic_vector(15 downto 0);     -- address bus signals
  A_nADR_EN             : out std_logic := '0';                 -- '0' => external address driver active 
  A_nADR_FROM_SCUB      : out std_logic := '0';                 -- '0' => external address driver direction master => slave
  A_D                   : inout std_logic_vector(15 downto 0);  -- databus signals
  A_nDS                 : in std_logic;                         -- data strobe driven by master
  A_RnW                 : in std_logic;                         -- read/write, driven by master
  A_nSel_Ext_Data_Drv   : out std_logic;                        -- '0' => external data driver is active
  A_Ext_Data_RD         : out std_logic;                        -- '0' => external data driver direction master => slave
                                                                -- (besser default 0, oder Treiber A/B tauschen) -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
  A_nDtack              : out std_logic;                        -- '0' => data acknowlege is active; opendrain driver
  A_nSRQ                : out std_logic;                        -- '0' => service request is active; opendrain driver
  A_nBoardSel           : in std_logic;                         -- '0' => selects this slave 
  A_nEvent_Str          : in std_logic;                         -- '0' => signals a timing cycle
  A_SysClock            : in std_logic;                         -- sys clock driver by the bus master
  A_Spare0              : in std_logic;                         -- spare line; direction master => slave
  A_Spare1              : in std_logic;                         -- spare line; direction master => slave
  A_nReset              : in std_logic;                         -- '0' => reset is active; driven by master

  A_nSEL_Ext_Signal_DRV : out std_logic;                        -- '0' => external signal driver is active
  A_nExt_Signal_in      : out std_logic;                        -- '0' => external signal direction master => slave

  ----------------- OneWire ----------------------------------------------------------------------------------------
  A_OneWire: inout std_logic;                                   -- temperature sensor on the slave board

  ------------ Logic analyser Signals -------------------------------------------------------------------------------
  A_SEL  : in std_logic_vector(3 downto 0);                     -- use to select sources for the logic analyser ports
  A_Tclk : out std_logic;                                       -- Clock  for Logikanalysator Port A
  A_TA   : out std_logic_vector(15 downto 0);                   -- test port a

  ---------------------------------- Diagnose-LED's -----------------------------------------------------------------
  A_nLED_D2 : out std_logic;                                    -- debug led d2 on the base board
  A_nLED_D3 : out std_logic;                                    -- debug led d3 on the base board

  ------------ User I/O zur VG-Connector -------------------------------------------------------------------------------
  A_nUser_EN : out std_logic;                                   -- Enable User-I/O
  UIO        : inout std_logic_vector(15 downto 0);             -- User I/O VG-Connector

  ---------------- User I/O to the user module -----------------------------------------------------------------
  CLK_IO : in std_logic;                                        -- clock from the user module
  PIO    : inout std_logic_vector(150 downto 16)                -- Dig. User I/0 to Piggy
  );
end scu_diob;



architecture scu_diob_arch of scu_diob is


--  +============================================================================================================================+
--  |                                 Firmware_Version/Firmware_Release und Basis-Adressen                                       |
--  +============================================================================================================================+

    CONSTANT c_Firmware_Version:    Integer := 0;      -- Firmware_Version
--  CONSTANT c_Firmware_Release:    Integer := 9;      -- Firmware_Release
--  CONSTANT c_Firmware_Release:    Integer := 10;     -- Firmware_Release Stand 03.08.2015 ('740 nur Standard- kein DAC/FG-Mode)
--  CONSTANT c_Firmware_Release:    Integer := 11;     -- Firmware_Release Stand 04.09.2015 ('Fehler LED-Lemo-Out, IO-Modul-Backplane-Test)
--  CONSTANT c_Firmware_Release:    Integer := 12;     -- Firmware_Release Stand 10.09.2015 ('IO-Modul-Backplane-Test Fortsetzung)
--  CONSTANT c_Firmware_Release:    Integer := 13;     -- Firmware_Release Stand 09.10.2015 ('+ 16In, 16Out, CID SPSIOI1 60 geändert in 68)
--  CONSTANT c_Firmware_Release:    Integer := 14;     -- Firmware_Release Stand 12.10.2015 ( +'731 und Fehlerkorrektur)
--  CONSTANT c_Firmware_Release:    Integer := 15;     -- Firmware_Release Stand 21.10.2015 ( +'731 und Fehlerkorrektur)
--  CONSTANT c_Firmware_Release:    Integer := 16;     -- Firmware_Release Stand 19.11.2015 ( Update Reg.-Belegung '700' u. '710')
--  CONSTANT c_Firmware_Release:    Integer := 17;     -- Firmware_Release Stand 02.12.2015 ( Strobe und Trigger (ECC) auf '710' geändert)
--  CONSTANT c_Firmware_Release:    Integer := 18;     -- Firmware_Release Stand 28.01.2016 ( Error, Tag-Steuerung: überlappende Outputs im gleichen Register)
--  CONSTANT c_Firmware_Release:    Integer := 19;     -- Firmware_Release Stand 08.02.2016 ( + Tag-Steuerung: Level für jedes Bit im Outp.-Register ist einstellbar)
--  CONSTANT c_Firmware_Release:    Integer := 20;     -- Firmware_release Stand 17.02.2016 ( INFO ROM in Housekeeping Modul eingefügt)
--  CONSTANT c_Firmware_Release:    Integer := 21;     -- Firmware_release Stand 29.02.2016 ( PLL nur an lokalem CLK angeschlossen)
--  CONSTANT c_Firmware_Release:    Integer := 22;     -- Firmware_release Stand 29.06.2016 ( +'700 Status/Error "SM" + Lemo-Outp., + ('700+'710) Lemo-Outp., +'751 (DA2)
--  CONSTANT c_Firmware_Release:    Integer := 23;     -- Firmware_release Stand 12.01.2017 ( PLL's wieder am SCU-CLK angeschlossen, Interlock-Logik geändert)
--  CONSTANT c_Firmware_Release:    Integer := 24;     -- Firmware_release Stand 10.05.2017 ( Error, Umschaltung FG: bipolar/unipolar DAC '710
--  CONSTANT c_Firmware_Release:    Integer := 25;     -- Firmware_release Stand 28.08.2017 ( + '760 (ATR1) + FG_901.040 AD1) + FG_901.050 8In8Out1) + Tri-State-Steuerung (PIO+UIO) )
--  CONSTANT c_Firmware_Release:    Integer := 26;     -- Firmware_release Stand 10.10.2017 ( + 'FG_901.010 16Out, OutpReg1 'MF-Funktion' auf die Outputs umschaltbar)
--  CONSTANT c_Firmware_Release:    Integer := 27;     -- Firmware_release Stand 21.11.2017 ( Error, '760 (ATR1): LED-Mux für FG902070_OptoDig_Out1
--  CONSTANT c_Firmware_Release:    Integer := 28;     -- Firmware_release Stand 20.06.2018 ( KK: Umbau ATR Trigger auf kanalweises Triggern und Largepulse Option, KL: SPill-Abort,QD-Test Matrix)
    CONSTANT c_Firmware_Release:    Integer := 29;     -- Firmware_release Stand 19.05.2021 ( + neuer Zwischen-Backplane )
--  CONSTANT c_Firmware_Release:    Integer := 16#FF#; -- Test-Firmware_release


    CONSTANT clk_switch_status_cntrl_addr:       unsigned := x"0030";
    CONSTANT c_lm32_ow_Base_Addr:   unsigned(15 downto 0):=  x"0040";  -- housekeeping/LM32

    CONSTANT c_ADDAC_Base_addr:                  Integer := 16#0200#;  -- ADDAC (DAC = x"0200", ADC = x"0230")
    CONSTANT c_io_port_Base_Addr:   unsigned(15 downto 0):=  x"0220";  -- 4x8 Bit (ADDAC FG900.161)
    CONSTANT c_fg_1_Base_Addr:      unsigned(15 downto 0):=  x"0300";  -- FG1
    CONSTANT c_tmr_Base_Addr:       unsigned(15 downto 0):=  x"0330";  -- Timer
    CONSTANT c_fg_2_Base_Addr:      unsigned(15 downto 0):=  x"0340";  -- FG2
    --
    CONSTANT c_Conf_Sts1_Base_Addr:              Integer := 16#0500#;  -- Status-Config-Register
    CONSTANT c_AW_Port1_Base_Addr:               Integer := 16#0510#;  -- Anwender I/O-Register
    CONSTANT c_INL_xor1_Base_Addr:               Integer := 16#0530#;  -- Interlock-Pegel-Register
    CONSTANT c_INL_msk1_Base_Addr:               Integer := 16#0540#;  -- Interlock-Masken-Register
    CONSTANT c_Tag_Ctrl1_Base_Addr:              Integer := 16#0580#;  -- Tag-Steuerung
    CONSTANT c_AW_ATR_DAC_Base_Addr:             Integer := 16#0600#;  -- DAC-Daten-Register
    CONSTANT c_AW_atr_comp_ctrl_Base_Addr:       Integer := 16#0610#;  -- Cnt_Reg_Comp_Channel
    CONSTANT c_AW_atr_puls_ctrl_Base_Addr:       Integer := 16#0618#;  -- 8Ch:Pulsbreite&Triggerverzögerung für Zündpuls, Timeout der Rückmeldung (Adr:x618..x62F)
--
    CONSTANT c_IOBP_Masken_Base_Addr:            Integer := 16#0630#;  -- IO-Backplane Masken-Register
    CONSTANT c_IOBP_ID_Base_Addr:                Integer := 16#0638#;  -- IO-Backplane Modul-ID-Register
    CONSTANT c_HW_Interlock_Base_Addr:           Integer := 16#0640#;  -- IO-Backplane Spill Abort HW Interlock
    CONSTANT c_IOBP_QD_Base_Addr:                Integer := 16#0650#;  -- IO-Backplane Quench Detection
    CONSTANT c_IOBP_READBACK_Base_Addr:          Integer := 16#0670#;  -- IO-Backplane Output Readback Register



--  +============================================================================================================================+
--  |                                                 CONSTANT                                                                   |
--  +============================================================================================================================+


    CONSTANT c_cid_system:     integer range 0 to 16#FFFF#:= 55;     -- extension card: cid_system, CSCOHW=55


    type ID_CID is record
      ID   : std_logic_vector(7 downto 0);
      CID  : integer range 0 to 16#FFFF#;
    end record;
--                                        +--------------- Piggy-ID(Hardware-Codierung)
--                                        |     +--------- CID(extension card: cid_system)
--                                        |     |
    CONSTANT c_AW_P37IO:      ID_CID:= (x"01", 27);   ---- Piggy-ID(Codierung), B"0000_0001", FG900_700
    CONSTANT c_AW_P25IO:      ID_CID:= (x"02", 28);   ---- Piggy-ID(Codierung), B"0000_0010", FG900_710
    CONSTANT c_AW_OCin:       ID_CID:= (x"03", 29);   ---- Piggy-ID(Codierung), B"0000_0011", FG900_720
    CONSTANT c_AW_OCIO1:      ID_CID:= (x"04", 30);   ---- Piggy-ID(Codierung), B"0000_0100", FG900_730
    CONSTANT c_AW_UIO:        ID_CID:= (x"05", 31);   ---- Piggy-ID(Codierung), B"0000_0101", FG900_740
    CONSTANT c_AW_DA1:        ID_CID:= (x"06", 32);   ---- Piggy-ID(Codierung), B"0000_0110", FG900_750
    CONSTANT c_AW_ATR1:       ID_CID:= (x"07", 73);   ---- Piggy-ID(Codierung), B"0000_0111", FG900_760
    CONSTANT c_AW_SPSIO1:     ID_CID:= (x"08", 33);   ---- Piggy-ID(Codierung), B"0000_1000", FG900_770 -- Ausgänge schalten nach 24V
    CONSTANT c_AW_HFIO:       ID_CID:= (x"09", 34);   ---- Piggy-ID(Codierung), B"0000_1001", FG900_780
    CONSTANT c_AW_SPSIOI1:    ID_CID:= (x"0A", 68);   ---- Piggy-ID(Codierung), B"0000_1010", FG901_770 -- Ausgänge schalten nach GND
    CONSTANT c_AW_INLB12S:    ID_CID:= (x"0B", 67);   ---- Piggy-ID(Codierung), B"0000_1011", FG902_050 -- IO-Modul-Backplane mit 12 Steckplätzen
    CONSTANT c_AW_16Out2:     ID_CID:= (x"0C", 70);   ---- Piggy-ID(Codierung), B"0000_1100", FG901_010 and FG901.011-- Output 16 Bit
    CONSTANT c_AW_16In2:      ID_CID:= (x"0D", 71);   ---- Piggy-ID(Codierung), B"0000_1101", FG901_020 -- Input 16 Bit
    CONSTANT c_AW_OCIO2:      ID_CID:= (x"0E", 61);   ---- Piggy-ID(Codierung), B"0000_1110", FG900_731
    CONSTANT c_AW_DA2:        ID_CID:= (x"0F", 72);   ---- Piggy-ID(Codierung), B"0000_1111", FG900_751
    CONSTANT c_AW_AD1:        ID_CID:= (x"10", 80);   ---- Piggy-ID(Codierung), B"0001_0000", FG901_040 -- analog Input: 2x16Bit ADC
    CONSTANT c_AW_ATR2:       ID_CID:= (x"11", 81);   ---- Piggy-ID(Codierung), B"0001_0001", FG900_761
    CONSTANT c_AW_8In8Out1:   ID_CID:= (x"12", 88);   ---- Piggy-ID(Codierung), B"0001_0010", FG901_050 -- Input 8-Bit + Output 8-
    CONSTANT c_AW_INLB12S1:   ID_CID:= (x"13", 67);   ---- Piggy-ID(Codierung), B"0001_0011", FG902_050-- neue IO-Modul-Backplane mit 12 Steckplätzen (zu überprüfen und zu berichtigen)
---
---
--- Konstantenwerte für die Interface-Module für der aktuellen und neuen Zwischenbackplane:
---
    CONSTANT c_BP_5LWLIO2 :    ID_CID:= (x"01", 65);   ----SUB- Piggy-ID(Codierung), B"0000_0001", FG902.011 -- 5x opt In, 1x opt Out       -> aktuelle und newe Zwischenbackplane
    CONSTANT c_BP_5LEMOIO2 :   ID_CID:= (x"02", 66);   ---- SUB-Piggy-ID(Codierung), B"0000_0010", FG902.011 -- 5xlemo In, 1xlemo Out       -> aktuelle und newe Zwischenbackplane
    CONSTANT c_BP_6LemoI1 :    ID_CID:= (x"03", 74);   ---- SUB-Piggy-ID(Codierung), B"0000_0011", FG902.130 -- 6xlemo In,   -> neue Zwischenbackplane
    CONSTANT c_BP_6LWLI1 :     ID_CID:= (x"04", 75);   ---- SUB-Piggy-ID(Codierung), B"0000_0100", FG902.110 -- 6x opt In,   -> neue Zwischenbackplane
    CONSTANT c_BP_6LWLO1 :     ID_CID:= (x"05", 76);   ---- SUB-Piggy-ID(Codierung), B"0000_0101", FG902.120 -- 6x opt Out,  -> neue Zwischenbackplane
    CONSTANT c_BP_6LEMO1 :     ID_CID:= (x"06", 77);   ---- SUB-Piggy-ID(Codierung), B"0000_0110", FG902.140 -- 6x lemo Out,  -> neue Zwischenbackplane
    CONSTANT c_BP_6DGIN1 :     ID_CID:= (x"07", 78);   ---- SUB-Piggy-ID(Codierung), B"0000_0111", FG902.150 -- 6x lemo In DigIn1 ,  -> neue Zwischenbackplane
    --  CONSTANT c_AW:            ID_CID:= (x"0F", 00);   ---- Piggy-ID(Codierung), B"0000_1111",


    constant  stretch_cnt:    integer := 5;                               -- für LED's


    constant  Clk_in_ns:      integer  :=  1000000000 /  clk_sys_in_Hz;          -- (=8ns,    bei 125MHz)
    CONSTANT  CLK_sys_in_ps:  INTEGER  := (1000000000 / (CLK_sys_in_Hz / 1000));  -- muss eigentlich clk-halbe sein

    constant  C_Strobe_1us:   integer := 1000 / Clk_in_ns;                       -- Anzahl der Clocks für 1us
    constant  C_Strobe_2us:   integer := 2000 / Clk_in_ns;                       -- Anzahl der Clocks für 2us
    constant  C_Strobe_3us:   integer := 003000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   3uS
    constant  C_Strobe_7us:   integer := 007000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   7uS
--  constant  C_Strobe_3us:   integer := 000300 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   300nS (Test)
--  constant  C_Strobe_7us:   integer := 000700 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   700nS (Test)


   TYPE      t_Integer_Array  is array (0 to 7) of integer range 0 to 16383;

  --------------- Array für die Anzahl der Clock's für die B1dddebounce-Zeiten von 1,2,4,8,16,32,64,128 us ---------------


  constant  Wert_2_Hoch_n:   t_Integer_Array := (001000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   1uS
                                                 002000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   2uS
                                                 004000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   4uS
                                                 008000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   8uS
                                                 016000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  16uS
                                                 032000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  32uS
                                                 064000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  64uS
                                                 128000 * 1000 / CLK_sys_in_ps);  -- Anzahl der Clock's für die Debounce-Zeit von 128uS





  CONSTANT C_Strobe_100ns:  integer range 0 to 16383:= (000100 * 1000 / CLK_sys_in_ps);   -- Anzahl der Clock's für den Strobe 100ns

  TYPE   t_Integer_Strobe_Array     is array (0 to 7) of integer range 0 to 65535;
  constant Wert_Strobe_2_Hoch_n : t_Integer_Strobe_Array := (00001, 00002, 00004, 00008, 00016, 00032, 00064, 00128);

  TYPE   t_status_error_update_Array     is array (0 to 7) of integer range 0 to 1023;
--                                                                ( 2^0, 2^1, 2^2, 2^3, 2^4, 2^5, 2^6, 2^7
--                                                                   0    2    4    8   16   32   64  128
  constant Sts_Err_Zeit_2_Hoch_n : t_status_error_update_Array := (005, 010, 020, 040, 080, 160, 320, 640);


--  +============================================================================================================================+
--  |                                                         signal                                                             |
--  +============================================================================================================================+

  signal clk_sys, clk_cal, locked : std_logic;
  signal Debounce_cnt:              integer range 0 to 16383;   -- Clock's für die Entprellzeit

  signal sys_clk_is_bad        : std_logic;
  signal rstn_sys              : std_logic;
  signal pll_locked            : std_logic;
  signal clk_switch_rd_data    : std_logic_vector(15 downto 0);
  signal clk_switch_rd_active  : std_logic;
  signal clk_switch_dtack      : std_logic;
  signal signal_tap_clk_250mhz : std_logic;
  signal clk_update            : std_logic;
  signal clk_flash             : std_logic;
  signal rstn_stc              : std_logic;
  signal rstn_update           : std_logic;
  signal rstn_flash            : std_logic;
  constant c_is_arria5: boolean := false;
  signal local_clk_is_running  : std_logic;
  signal local_clk_is_bad      : std_logic;


  signal SCUB_SRQ:            std_logic;
  signal SCUB_Dtack:          std_logic;
  signal convst:              std_logic;
  signal rst:                 std_logic;

  signal Dtack_to_SCUB:       std_logic;

  signal ADR_from_SCUB_LA:    std_logic_vector(15 downto 0);
  signal Data_from_SCUB_LA:   std_logic_vector(15 downto 0);
  signal Ext_Adr_Val:         std_logic;
  signal Ext_Rd_active:       std_logic;
  signal Ext_Wr_active:       std_logic;
  signal Ext_Wr_fin_ovl:      std_logic;
  signal Ext_RD_fin_ovl:      std_logic;
  signal SCU_Ext_Wr_fin:      std_logic;
  signal nPowerup_Res:        std_logic;
  signal Timing_Pattern_LA:   std_logic_vector(31 downto 0);--  latched timing pattern from SCU_Bus for external user functions
  signal Timing_Pattern_RCV:  std_logic;----------------------  timing pattern received

  signal extension_cid_system:  integer range 0 to 16#FFFF#;  -- in,  extension card: cid_system
  signal extension_cid_group:   integer range 0 to 16#FFFF#;  --in, extension card: cid_group

  signal Max_AWOut_Reg_Nr:      integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
  signal Max_AWIn_Reg_Nr:       integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung

  signal AWIn_Deb_Time:          integer range 0 to 7;           -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time", Wert aus DIOB-Config 1
  signal Min_AWIn_Deb_Time:      integer range 0 to 7;           -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time"


  signal FG_1_dtack:         std_logic;
  signal FG_1_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal FG_1_rd_active:     std_logic;
  signal FG_1_sw:            std_logic_vector(31 downto 0);
  signal FG_1_strobe:        std_logic;
  signal FG_1_dreq:          std_logic;

  signal FG_2_dtack:         std_logic;
  signal FG_2_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal FG_2_rd_active:     std_logic;
  signal FG_2_sw:            std_logic_vector(31 downto 0);
  signal FG_2_strobe:        std_logic;
  signal FG_2_dreq:          std_logic;

  signal fg_start:           std_logic;

  signal tmr_rd_active:      std_logic;
  signal tmr_data_to_SCUB:   std_logic_vector(15 downto 0);
  signal tmr_dtack:          std_logic;
  signal tmr_irq:            std_logic;

  signal led_ena_cnt:        std_logic;

  signal Data_to_SCUB:       std_logic_vector(15 downto 0);

  signal reset_clks :        std_logic_vector(0 downto 0);
  signal reset_rstn :        std_logic_vector(0 downto 0);
  signal clk_sys_rstn :      std_logic;

  signal owr_pwren_o:        std_logic_vector(1 downto 0);
  signal owr_en_o:           std_logic_vector(1 downto 0);
  signal owr_i:              std_logic_vector(1 downto 0);

  signal wb_scu_rd_active:    std_logic;
  signal wb_scu_dtack:        std_logic;
  signal wb_scu_data_to_SCUB: std_logic_vector(15 downto 0);


  signal Powerup_Res:     std_logic;  -- only for modelsim!
  signal Powerup_Done:    std_logic;  -- this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
  signal WRnRD:           std_logic;  -- only for modelsim!

  signal Deb_SCUB_Reset_out:  std_logic;
  signal Standard_Reg_Acc:    std_logic;
  signal Ext_Rd_fin:          std_logic;
--  signal Ext_Wr_fin:          std_logic;

  signal test_out: std_logic_vector(15 downto 0);

  signal Ena_Every_100ns: std_logic;
  signal Ena_Every_166ns: std_logic;
  signal Ena_Every_250ns: std_logic;
  signal Ena_Every_500ns: std_logic;
  signal Ena_Every_10ms:  std_logic;
  signal Ena_Every_20ms:  std_logic;
  signal Ena_Every_1us:   std_logic;
  signal Ena_Every_250ms: std_logic;
  signal Ena_Every_500ms: std_logic;

  signal F_12p5_MHz:      std_logic;

  signal test_port_in_0:  std_logic_vector(15 downto 0);
  signal test_clocks:     std_logic_vector(15 downto 0);

  signal s_nLED_Sel:      std_logic;   -- LED = Sel
  signal s_nLED_Dtack:    std_logic;   -- LED = Dtack
  signal s_nLED_inR:      std_logic;   -- LED = interrupt

  signal s_nLED:          std_logic_vector(7 downto 0); -- LED's
  signal s_nLED_Out:      std_logic_vector(7 downto 0); -- LED's
  signal AW_ID:           std_logic_vector(7 downto 0); -- Anwender_ID

  signal s_nLED_User1_i:  std_logic;
  signal s_nLED_User1_o:  std_logic;
  signal s_nLED_User2_i:  std_logic;
  signal s_nLED_User2_o:  std_logic;
  signal s_nLED_User3_i:  std_logic;
  signal s_nLED_User3_o:  std_logic;
  signal clk_blink:       std_logic;
  signal clk_switch_intr: std_logic;

  signal uart_txd_out:    std_logic;
  signal tag_fg_start:    std_logic;


begin

  A_nADR_EN             <= '0';
  A_nADR_FROM_SCUB      <= '0';
  A_nExt_Signal_in      <= '0';
  A_nSEL_Ext_Signal_DRV <= '0';
  A_nUser_EN            <= '0';


  Powerup_Res <= not nPowerup_Res;  -- only for modelsim!
  WRnRD       <= not A_RnW;         -- only for modelsim!

  diob_clk_switch: slave_clk_switch
    generic map (
      Base_Addr => clk_switch_status_cntrl_addr,
      card_type => g_card_type
    )
    port map(
      local_clk_i             => CLK_20MHz_D,
      sys_clk_i               => A_SysClock,
      nReset                  => rstn_sys,
      master_clk_o            => clk_sys,               -- core clocking
      pll_locked              => pll_locked,
      sys_clk_is_bad          => sys_clk_is_bad,
      Adr_from_SCUB_LA        => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
      Data_from_SCUB_LA       => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
      Ext_Adr_Val             => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active           => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
      Ext_Wr_active           => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
      Rd_Port                 => clk_switch_rd_data,    -- output for all read sources of this macro
      Rd_Activ                => clk_switch_rd_active,  -- this acro has read data available at the Rd_Port.
      Dtack                   => clk_switch_dtack,
      signal_tap_clk_250mhz   => signal_tap_clk_250mhz,
      clk_update              => clk_update,
      clk_flash               => clk_flash,
      clk_encdec              => open
    );

   reset : altera_reset
    generic map (
      g_plls   => 1,
      g_clocks => 4,
      g_areset => f_pick(c_is_arria5, 100, 1)*1024,
      g_stable => f_pick(c_is_arria5, 100, 1)*1024)
    port map (
      clk_free_i    => clk_sys,
      rstn_i        => A_nReset,
      pll_lock_i(0) => pll_locked,
      pll_arst_o    => open,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => signal_tap_clk_250mhz,
      clocks_i(2)   => clk_update,
      clocks_i(3)   => clk_flash,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_stc,
      rstn_o(2)     => rstn_update,
      rstn_o(3)     => rstn_flash
    );


  test_port_in_0 <= x"0000"; --- kein Clock's am Teststecker


  test_clocks <=  X"0"                                                                              -- bit15..12
                -- use only outputs from PLL
              --& '0' & signal_tap_clk_250mhz & A_SysClock & CLK_20MHz_D                          -- bit11..8
              & '0' & '0' & '0' & '0'                                                             -- bit11..8
              & '0' & pll_locked & '0' & '0'                       -- bit7..4
              & '0' & '0' & '0' & '0';     -- bit3..0


  -- open drain buffer for one wire
  owr_i(0) <= A_OneWire;
  A_OneWire <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';

  zeit1 : zeitbasis
  generic map (
    CLK_in_Hz =>  clk_sys_in_Hz,
    diag_on   =>  1
  )
  port map (
    Res               =>  not rstn_sys,
    Clk               =>  clk_sys,
    Ena_every_100ns   =>  Ena_Every_100ns,
    Ena_every_166ns   =>  Ena_Every_166ns,
    Ena_every_250ns   =>  Ena_every_250ns,
    Ena_every_500ns   =>  Ena_every_500ns,
    Ena_every_1us     =>  Ena_every_1us,
    Ena_Every_20ms    =>  Ena_Every_20ms
  );


  p_led_sel: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => (not A_nBoardSel and not A_nDS), nLED => s_nLED_Sel);-- LED: sel Board

  p_led_dtack: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => SCUB_Dtack, nLED => s_nLED_Dtack);-- LED: Dtack to SCU-Bus

  p_led_inr: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => SCUB_SRQ, nLED => s_nLED_inR);-- LED: interrupt

  p_led_user1: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User1_i, nLED => s_nLED_User1_o);-- LED3 = User 1

  p_led_user2: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User2_i, nLED => s_nLED_User2_o);-- LED3 = User 1

  p_led_user3: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User3_i, nLED => s_nLED_User3_o);-- LED3 = User 1


  A_nLED_D2 <=   s_nLED_Sel;    -- Diagnose-LED_D2 = board select 
  A_nLED_D3 <=   s_nLED_Dtack;  -- Diagnose-LED_D3 = dtack


  sel_every_10ms: div_n
  generic map (n => integer(10.0e-3 / 1.0e-6), diag_on => 0)  -- ena nur jede us für einen Takt aktiv, deshalb n = 10000
  port map (
    res   => not rstn_sys,
    clk   => clk_sys,
    ena   => ENA_every_1us,
    div_o => ENA_every_10ms
  );

  sel_every_250ms: div_n
  generic map (n => 12, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 13x20ms = 260ms
  port map (
    res   => not rstn_sys,
    clk   => clk_sys,
    ena   => Ena_Every_20ms,
    div_o => ENA_every_250ms
  );

  sel_every_500ms: div_n
  generic map (n => 25, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 25x20ms = 500ms
  port map (
    res   => not rstn_sys,
    clk   => clk_sys,
    ena   => Ena_Every_20ms,
    div_o => ENA_every_500ms
  );

  p_clk_blink:
  process (clk_sys, rstn_sys, ENA_every_250ms)
  begin
    if rstn_sys = '0' then
      clk_blink <= '0';
    elsif rising_edge(clk_sys) then
      if ENA_every_500ms = '1' then
        clk_blink <= not clk_blink;
      end if;
    end if;
  end process;

  clk_switch_intr <= local_clk_is_running;

  SCU_Slave: SCU_Bus_Slave
  generic map (
      CLK_in_Hz               => clk_sys_in_Hz,
      Firmware_Release        => c_Firmware_Release,  -------------------- important: => Firmware_Release
      Firmware_Version        => c_Firmware_Version,  -------------------- important: => Firmware_Version
      CID_System              => 55, ------------------------------------- important: => CSCOHW
      intr_Enable             => b"0000_0000_0000_0001")
  port map (
      SCUB_Addr               => A_A,                                   -- in, SCU_Bus: address bus
      nSCUB_Timing_Cyc        => A_nEvent_Str,                          -- in, SCU_Bus signal: low active SCU_Bus runs timing cycle
      SCUB_Data               => A_D,                                   -- inout, SCU_Bus: data bus (FPGA tri state buffer)
      nSCUB_Slave_Sel         => A_nBoardSel,                           -- in, SCU_Bus: '0' => SCU master select slave
      nSCUB_DS                => A_nDS,                                 -- in, SCU_Bus: '0' => SCU master activate data strobe
      SCUB_RDnWR              => A_RnW,                                 -- in, SCU_Bus: '1' => SCU master read slave
      clk                     => clk_sys,
      nSCUB_Reset_in          => A_nReset,                              -- in, SCU_Bus-Signal: '0' => 'nSCUB_Reset_in' is active
      Data_to_SCUB            => Data_to_SCUB,                          -- in, connect read sources from external user functions
      Dtack_to_SCUB           => Dtack_to_SCUB,                         -- in, connect Dtack from from external user functions
      intr_in                 => FG_1_dreq & FG_2_dreq & tmr_irq & '0'  -- bit 15..12
                               & '0'& '0' & '0' &'0'                    -- bit 11..8
                               & x"0"                                   -- bit 7..4
                               & '0' & '0' & clk_switch_intr,           -- bit 3..1
      User_Ready              => '1',
      CID_GROUP               => 26,                                    -- important: => "FG900500_SCU_Diob1"
      extension_cid_system    => extension_cid_system,                  -- in, extension card: cid_system
      extension_cid_group     => extension_cid_group,                   -- in, extension card: cid_group
      Data_from_SCUB_LA       => Data_from_SCUB_LA,                     -- out, latched data from SCU_Bus for external user functions
      ADR_from_SCUB_LA        => ADR_from_SCUB_LA,                      -- out, latched address from SCU_Bus for external user functions
      Timing_Pattern_LA       => Timing_Pattern_LA,                     -- out, latched timing pattern from SCU_Bus for external user functions
      Timing_Pattern_RCV      => Timing_Pattern_RCV,                    -- out, timing pattern received
      nSCUB_Dtack_Opdrn       => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                        -- '0' => slave give dtack to SCU master
      SCUB_Dtack              => SCUB_Dtack,                            -- out, for connect via ext. open collector driver
                                                                        -- '1' => slave give dtack to SCU master
      nSCUB_SRQ_Opdrn         => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                        -- '0' => slave service request to SCU ma
      SCUB_SRQ                => SCUB_SRQ,                              -- out, for connect via ext. open collector driver
                                                                        -- '1' => slave service request to SCU master
      nSel_Ext_Data_Drv       => A_nSel_Ext_Data_Drv,                   -- out, '0' => select the external data driver on the SCU_Bus slave
      Ext_Data_Drv_Rd         => A_Ext_Data_RD,                         -- out, '1' => direction of the external data driver on the
                                                                        -- SCU_Bus slave is to the SCU_Bus
      Standard_Reg_Acc        => Standard_Reg_Acc,                      -- out, '1' => mark the access to register of this macro
      Ext_Adr_Val             => Ext_Adr_Val,                           -- out, for external user functions: '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active           => Ext_Rd_active,                         -- out, '1' => Rd-Cycle to external user register is active
      Ext_Rd_fin              => Ext_Rd_fin,                            -- out, marks end of read cycle, active one for one clock period
                                                                        -- of clk past cycle end (no overlap)
      Ext_Rd_Fin_ovl          => Ext_Rd_Fin_ovl,                        -- out, marks end of read cycle, active one for one clock period
                                                                        -- of clk during cycle end (overlap)
      Ext_Wr_active           => Ext_Wr_active,                         -- out, '1' => Wr-Cycle to external user register is active
      Ext_Wr_fin              => SCU_Ext_Wr_fin,                        -- out, marks end of write cycle, active high for one clock period
                                                                        -- of clk past cycle end (no overlap)
      Ext_Wr_fin_ovl          => Ext_Wr_fin_ovl,                        -- out, marks end of write cycle, active high for one clock period
                                                                        -- of clk before write cycle finished (with overlap)
      Deb_SCUB_Reset_out      => Deb_SCUB_Reset_out,                    -- out, the debounced 'nSCUB_Reset_in'-signal, is active high,
                                                                        -- can be used to reset
                                                                        -- external macros, when 'nSCUB_Reset_in' is '0'
      nPowerup_Res            => nPowerup_Res,                          -- out, this macro generates a power up reset
      Powerup_Done            => Powerup_Done                           -- out, this signal is set after powerup. Only the SCUB-Master can clear this bit.
      );

  lm32_ow: housekeeping
  generic map (
    Base_addr => c_lm32_ow_Base_Addr)
  port map (
    clk_sys     => clk_sys,
    clk_update  => clk_update,
    clk_flash   => clk_flash,
    rstn_sys    => rstn_sys,
    rstn_update => rstn_update,
    rstn_flash  => rstn_flash,


    ADR_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => wb_scu_rd_active,
    Data_to_SCUB      => wb_scu_data_to_SCUB,
    Dtack_to_SCUB     => wb_scu_dtack,

    owr_pwren_o       => owr_pwren_o,
    owr_en_o          => owr_en_o,
    owr_i             => owr_i,

    debug_serial_o    => uart_txd_out,
    debug_serial_i    => '0');

  fg_1: fg_quad_scu_bus
    generic map (
      Base_addr => c_fg_1_Base_Addr,
      clk_in_hz => clk_sys_in_Hz,
      diag_on_is_1 => 0, -- if 1 then diagnosic information is generated during compilation
      ACU => false
      )
    port map (

      -- SCUB interface
      Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
      Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
      Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
      Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
      clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
      sysclk            => '0',                   -- in, only used for ACU implementation
      nReset            => rstn_sys,          -- in, '0' => resets the fg_1
      Rd_Port           => FG_1_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
      user_rd_active    => FG_1_rd_active,        -- '1' = read data available at 'Rd_Port'-output
      Dtack             => FG_1_dtack,            -- connect Dtack to SCUB-Macro
      irq               => FG_1_dreq,             -- request of new parameter set
      tag               => Timing_Pattern_LA,     --
      tag_valid         => Timing_Pattern_RCV,    --
      ext_trigger       => Tag_FG_Start,          -- starts the ramping by external signal

      -- fg output
      sw_out            => FG_1_sw,               -- 32bit output from fg
      sw_strobe         => FG_1_strobe            -- signals new output data
    );

  fg_2: fg_quad_scu_bus
    generic map (
      Base_addr => c_fg_2_Base_Addr,
      clk_in_hz => clk_sys_in_Hz,
      diag_on_is_1 => 0, -- if 1 then diagnosic information is generated during compilation
      ACU => false
      )
    port map (

      -- SCUB interface
      Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
      Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
      Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
      Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
      clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
      sysclk            => '0',                   -- in, only used for ACU implementation
      nReset            => rstn_sys,          -- in, '0' => resets the fg_1
      Rd_Port           => FG_2_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
      user_rd_active    => FG_2_rd_active,        -- '1' = read data available at 'Rd_Port'-output
      Dtack             => FG_2_dtack,            -- connect Dtack to SCUB-Macro
      irq               => FG_2_dreq,             -- request of new parameter set
      tag               => Timing_Pattern_LA,     --
      tag_valid         => Timing_Pattern_RCV,    --
      ext_trigger       => Tag_FG_Start,          -- starts the ramping by external signal

      -- fg output
      sw_out            => FG_2_sw,               -- 32bit output from fg
      sw_strobe         => FG_2_strobe            -- signals new output data
    );

  tmr: tmr_scu_bus
  generic map (
    Base_addr     => c_tmr_Base_Addr,
    diag_on_is_1  => 1)
  port map (
    clk           => clk_sys,
    nrst          => rstn_sys,
    tmr_irq       => tmr_irq,

    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => tmr_rd_active,
    Data_to_SCUB      => tmr_data_to_SCUB,
    Dtack_to_SCUB     => tmr_dtack);

  rd_port_mux:  process ( clk_switch_rd_active,     clk_switch_rd_data,
                          wb_scu_rd_active,         wb_scu_data_to_SCUB,
                          FG_1_rd_active,           FG_1_data_to_SCUB,
                          FG_2_rd_active,           FG_2_data_to_SCUB,
                          tmr_rd_active,            tmr_data_to_SCUB
                        )


  variable sel: unsigned(4 downto 0);
  begin
  sel :=  tmr_rd_active & FG_1_rd_active & FG_2_rd_active & wb_scu_rd_active & clk_switch_rd_active;

    case sel is

      when "10000" => Data_to_SCUB <= tmr_data_to_SCUB;
      when "01000" => Data_to_SCUB <= FG_1_data_to_SCUB;
      when "00100" => Data_to_SCUB <= FG_2_data_to_SCUB;
      when "00010" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when "00001" => Data_to_SCUB <= clk_switch_rd_data;

      when others      => Data_to_SCUB <= (others => '0');
    end case;
  end process rd_port_mux;



-------------- Dtack_to_SCUB -----------------------------

    Dtack_to_SCUB <= ( tmr_dtack or FG_1_dtack or FG_2_dtack  or wb_scu_dtack  or clk_switch_dtack );

    A_nDtack <= not(SCUB_Dtack);
    A_nSRQ   <= not(SCUB_SRQ);


end architecture;
