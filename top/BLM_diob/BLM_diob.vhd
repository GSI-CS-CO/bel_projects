library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;
use work.BLM_counter_pkg.all;



--  Base_addr    : DIOB-Config-Register1 (alle Bit können gelesen und geschrieben werden)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 15     | Test-Mode                 | 1 = Testmodus; für Inbetriebnahme und Diagnose, 0 = Normalbetrieb (default)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 14..12 | InReg-Debounce-Time       | Entprellzeit für digitale Eingänge;
--  |            |                           | Entprellzeit in in 2x µs; Vorgabe Exponent (x) für Entprellzeit;
--  |            |                           | Wertebereich 1 ... 128 µs *)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 11     | InReg-Debounce-Enable     | Steuerung (Einschalten/Ausschalten) der Entprelleinheit (debouncing unit) für externe digitale Signale
--  |            |                           | 1 = Entprellung ausgeschaltet
--  |            |                           | 0 = Entprellung eingeschaltet
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 10..8  | MirrorMode-InReg-Sel      | Auswahl des Eingangsregisters für Spiegelung des gewählten Ausgangsregisters (siehe MirrorMode-OutReg-Sel)
--  |            |                           | 0 = inaktiv
--  |            |                           | 1...7 = Eingangsregister 1 bis 7; alle unmaskierten Bits (siehe MirrorMode-OutReg-Mask) des Ausgangsregisters x
--  |            |                           | (x gewählt über MirrorMode-OutReg-Sel) werden auf das hier gewählte Eingangsregister kopiert (gespiegelt)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 7..5   | MirrorMode-OutReg-Sel     | Auswahl des Ausgangsregister für Spiegelung der Bits in Eingangsregister x (x gewählt über MirrorMode-InReg-Sel)
--  |            |                           | 0 = inaktiv
--  |            |                           | 1...7 = Ausgangsregister 1 bis 7
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 4      |--  Reserve                |
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 3      | MirrorMode-OutReg         | Aktiviert die Spiegelung des gewählten Ausgangsregisters (MirrorMode-OutReg-Sel) auf Eingangsregister 1 oder 2;
--  |            |                           | 1 = Spiegelung aktiviert
--  |            |                           | 0 = Spiegelung ausgeschaltet (default)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 2      | Clear-CntUnit-Config      | 1 = löschen aller Konfigurationsregister der Counter-Kanäle (CounterUnit-Config-Reg 1 und 2)
--  |            |                           | Bit wird nach Auswertung automatisch gelöscht; ist nicht rücklesbar
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 1      | Clear-CmpUnit-Config      | 1 = löschen aller Konfigurationsregister der Compare-Kanäle (CmpUnit-Config-Reg 1 und 2)
--  |            |                           | Bit wird nach Auswertung automatisch gelöscht; ist nicht rücklesbar
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 0      | Clear-TAG-Config          | 1 = löschen aller Konfigurationsregister der Event-Steuerung (TAG-Filterung)
--  |                                        | Bit wird nach Auswertung automatisch gelöscht; ist nicht rücklesbar
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--                                                                                                                                --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +1 : DIOB-Config-Register2 (alle Bit können gelesen und geschrieben werden)                                     --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-0 |  frei                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +2 : DIOB-Status-Register1 (die Status-Bit's werden nach dem Lesen glöscht)                                     --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--      5 |  TAG-ID-Config-Error     | zwei oder mehr Ereigniskanäle mit gleicher Maske und gleichem Ausgangsregister             --
--      4 |  OutReg-Select-Error     | in einem oder mehr Ereigniskanälen ist kein Ausgangsregister ausgewählt                    --
--      3 |  TriggerReg-Select-Error | in einem oder mehr Ereigniskanälen ist kein Eingangsregister für Triggersignal ausgewählt  --
--      2 |  Unknown-OutReg-Select   | in einem oder mehr Ereigniskanälen ist ein nicht unterstütztes Ausgangsregister gewählt    --
--      1 |  Unknown-InReg-Select    | in einem oder mehr Ereigniskanälen ist ein nicht unterstütztes Eingangsregister gewählt    --
--      0 |  Trigger-Time-Out        | zulässige Wartezeit auf externes Triggersignal wurde überschritten (Time-Out)              --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +3 : DIOB-Status-Register2 (die Status-Bit's werden nach dem Lesen glöscht)                                     --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-0 |  frei                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--     Base_addr + 4 – Base_addr +6  reserviert für Erweiterung                                                                   --
-----+------------------------------------------------------------------------------------------------------------------
--     Base_addr + 7 Konfigurationregister1 für Interface-Teil: Die Bits im Anwender(Piggy)Config-Register1 haben für jedes Piggy --
--                                                              eine andere Bedeutung                                             --
--                                                                                                                                --
--     ##########################################################################################################                 --
--     ####                                 Anwender-IO: P25IO  -- FG900_710                                  ###                 --
--     ##########################################################################################################                 --
--                                                                                                                                --
--   ----+----------------------------------------------------------------------------------------------------------------------  --
--     1 | Input:  1 = ADC-Daten aus dem Input-Speicher (gespeichert mit EOC)                                                     --
--       |         2 = Daten die am Sub-D Stecker anstehen.                                                                       --
--   ----+----------------------------------------------------------------------------------------------------------------------  --
--     0 | Output: 1 = Output-Daten(DAC-Werte), kommen vom Funktionsgen. und werden mit FG_Strobe uebernommen.                    --
--       |         0 = Output-Daten(DAC-Werte), kommen vom SCU-Bus-Slave.                                                         --
--   ----+----------------------------------------------------------------------------------------------------------------------  --
--                                                                                                                                --
--     ##########################################################################################################                 --
--     ####                                 Anwender-IO: P25IO  -- FG900_740                                  ###                 --
--     ##########################################################################################################                 --
--                                                                                                                                --
--   ----+-----------------------------------------------------------------------                                                 --
--     9 | FG_mode;  1 = Funktiongenerator-Mode, DAC-Werte kommen von FG_Data und                                                 --
--       |           0 = Software-Mode, DAC-Werte, kommen vom SCU-Bus-Slave.                                                      --
--   ----+-----------------------------------------------------------------------                                                 --
--     8 | Output-Polarität Lemo,         1 = Negativ,  0 = Poitiv(Default)                                                      --
--     7 | Output-Polarität Bit [23..0],  1 = Negativ,  0 = Positiv(Default)                                                      --
--   ----+-----------------------------------------------------------------------                                                 --
--     6 | Enable Output-Lemo,            1 = Enable,   0 = Disable(Default)                                                      --
--     5 | Enable Output-Bit [23..20],    1 = Enable,   0 = Disable(Default)                                                      --
--     4 | Enable Output-Bit [19..16],    1 = Enable,   0 = Disable(Default)                                                      --
--     3 | Enable Output-Bit [15..12],    1 = Enable,   0 = Disable(Default)                                                      --
---    2 | Enable Output-Bit [11..8],     1 = Enable,   0 = Disable(Default)                                                      --
--     1 | Enable Output-Bit [7..4],      1 = Enable,   0 = Disable(Default)                                                      --
--     0 | Enable Output-Bit [3..0],      1 = Enable,   0 = Disable(Default)                                                      --
--   ----+-----------------------------------------------------------------------                                                 --
--                                                                                                                                --
------------------------------------------------------------------------------------------------------------------------------------
entity BLM_diob is
generic (
    CLK_sys_in_Hz:      integer := 125000000;
    g_card_type:        string := "diob"
        );

port  (
    ------------------------------ Clocks -------------------------------------------------------------------------
    CLK_20MHz_A: in std_logic; -- Clock_A
    CLK_20MHz_B: in std_logic; -- Clock_B
    CLK_20MHz_C: in std_logic; -- Clock_C
    CLK_20MHz_D: in std_logic; -- Clock_D

    --------- Parallel SCU-Bus-Signale ----------------------------------------------------------------------------
    A_A: in std_logic_vector(15 downto 0); -- SCU-Adressbus
    A_nADR_EN: out std_logic := '0'; -- '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_FROM_SCUB: out std_logic := '0'; -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave
    A_D: inout std_logic_vector(15 downto 0); -- SCU-Datenbus
    A_nDS: in std_logic; -- Data-Strobe vom Master gertieben
    A_RnW: in std_logic; -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv: out std_logic; -- '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD: out std_logic; -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack: out std_logic; -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nSRQ: out std_logic; -- Service-Request null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nBoardSel: in std_logic; -- '0' => Master aktiviert diesen Slave
    A_nEvent_Str: in std_logic; -- '0' => Master sigalisiert Timing-Zyklus
    A_SysClock: in std_logic; -- Clock vom Master getrieben.
    A_Spare0: in std_logic; -- vom Master getrieben
    A_Spare1: in std_logic; -- vom Master getrieben
    A_nReset: in std_logic; -- Reset (aktiv '0'), vom Master getrieben

    A_nSEL_Ext_Signal_DRV: out std_logic; -- '0' => Treiber fr SCU-Bus-Steuer-Signale aktiv
    A_nExt_Signal_in: out std_logic; -- '0' => Treiber fr SCU-Bus-Steuer-Signale-Richtung: SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)

    ----------------- OneWire ----------------------------------------------------------------------------------------
    A_OneWire: inout std_logic; -- Temp.-OneWire auf dem Slave

    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL: in std_logic_vector(3 downto 0); -- use to select sources for the logic analyser ports
    A_Tclk: out std_logic; -- Clock  for Logikanalysator Port A
    A_TA:   out std_logic_vector(15 downto 0); -- test port a

    ---------------------------------- Diagnose-LED's -----------------------------------------------------------------
    A_nLED_D2: out std_logic; -- Diagnose-LED_D2 auf dem Basis-Board
    A_nLED_D3: out std_logic; -- Diagnose-LED_D3 auf dem Basis-Board

    ------------ User I/O zur VG-Leiste -------------------------------------------------------------------------------
    A_nUser_EN: out std_logic; -- Enable User-I/O
    UIO: inout std_logic_vector(15 downto 0); -- User I/O VG-Leiste

    ---------------- bergabestecker fr Anwender-I/O -----------------------------------------------------------------
    CLK_IO: in std_logic; -- Clock vom Anwender_I/0
    PIO: inout std_logic_vector(150 downto 16)  -- Dig. User I/0 to Piggy
    );
end BLM_diob;



architecture BLM_diob_arch of BLM_diob is


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
--  CONSTANT c_Firmware_Release:    Integer := 29;     -- Firmware_release Stand 19.05.2021 ( + neuer Zwischen-Backplane )
   CONSTANT c_Firmware_Release:    Integer := 30;     -- Firmware_release Stand 19.05.2021 ( + neuer Zwischen-Backplane )
--   CONSTANT c_Firmware_Release:    Integer := 16#FF#; -- Test-Firmware_release

    CONSTANT clk_switch_status_cntrl_addr:       unsigned := x"0030";
    CONSTANT c_lm32_ow_Base_Addr:   unsigned(15 downto 0):=  x"0040";  -- housekeeping/LM32

--    CONSTANT c_ADDAC_Base_addr:                  Integer := 16#0200#;  -- ADDAC (DAC = x"0200", ADC = x"0230")
    CONSTANT c_io_port_Base_Addr:   unsigned(15 downto 0):=  x"0220";  -- 4x8 Bit (ADDAC FG900.161)
--    CONSTANT c_fg_1_Base_Addr:      unsigned(15 downto 0):=  x"0300";  -- FG1
    CONSTANT c_tmr_Base_Addr:       unsigned(15 downto 0):=  x"0330";  -- Timer
--    CONSTANT c_fg_2_Base_Addr:      unsigned(15 downto 0):=  x"0340";  -- FG2
    --
    CONSTANT c_Conf_Sts1_Base_Addr:              Integer := 16#0500#;  -- Status-Config-Register
--    CONSTANT c_AW_Port1_Base_Addr:               Integer := 16#0510#;  -- Anwender I/O-Register
    CONSTANT c_INL_xor1_Base_Addr:               Integer := 16#0530#;  -- Interlock-Pegel-Register
    CONSTANT c_INL_msk1_Base_Addr:               Integer := 16#0540#;  -- Interlock-Masken-Register
--    CONSTANT c_Tag_Ctrl1_Base_Addr:              Integer := 16#0580#;  -- Tag-Steuerung
--    CONSTANT c_AW_ATR_DAC_Base_Addr:             Integer := 16#0600#;  -- DAC-Daten-Register
--    CONSTANT c_AW_atr_comp_ctrl_Base_Addr:       Integer := 16#0610#;  -- Cnt_Reg_Comp_Channel
--    CONSTANT c_AW_atr_puls_ctrl_Base_Addr:       Integer := 16#0618#;  -- 8Ch:Pulsbreite&Triggerverzögerung für Zündpuls, Timeout der Rückmeldung (Adr:x618..x62F)
--
    CONSTANT c_IOBP_Masken_Base_Addr:            Integer := 16#0630#;  -- IO-Backplane Masken-Register
    CONSTANT c_IOBP_ID_Base_Addr:                Integer := 16#0638#;  -- IO-Backplane Modul-ID-Register
--    CONSTANT c_HW_Interlock_Base_Addr:           Integer := 16#0640#;  -- IO-Backplane Spill Abort HW Interlock
--    CONSTANT c_IOBP_QD_Base_Addr:                Integer := 16#0650#;  -- IO-Backplane Quench Detection
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
    --  CONSTANT c_AW:            ID_CID:= (x"0F", 00);   ---- Piggy-ID(Codierungvariable sel: unsigned(19 downto 0);), B"0000_1111",


    constant  stretch_cnt:    integer := 5;                               -- für LED's


    constant  Clk_in_ns:      integer  :=  1000000000 /  clk_sys_in_Hz;          -- (=8ns,    bei 125MHz)
    CONSTANT  CLK_sys_in_ps:  INTEGER  := (1000000000 / (CLK_sys_in_Hz / 1000));  -- muss eigentlich clk-halbe sein

    constant  C_Strobe_1us:   integer := 1000 / Clk_in_ns;                       -- Anzahl der Clocks für 1us
    constant  C_Strobe_2us:   integer := 2000 / Clk_in_ns;                       -- Anzahl der Clocks für 2us
    constant  C_Strobe_3us:   integer := 003000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   3uS
    constant  C_Strobe_7us:   integer := 007000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   7uS
--  constant  C_Strobe_3us:   integer := 000300 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   300nS (Test)
--  constant  C_Strobe_7us:   integer := 000700 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   700nS (Test)

    constant SEL_FG902_130_ID  :  std_logic_vector(7 downto 0):="00000011"; -- 6 LEMO Input Modul FG902.130
    constant SEL_FG902_150_ID  :  std_logic_vector(7 downto 0):="00000111"; -- 6 LEMO Input Modul FG902150 in slot x
    constant SEL_FG902_LWL_ID  :  std_logic_vector(7 downto 0):="00000100"; -- 6 LWL Input Modul in slot x
    
    constant SEL_OUTMOD1_ID  :  std_logic_vector(7 downto 0):="00000101";   -- Output Modul
    constant SEL_OUTMOD2_ID  :  std_logic_vector(7 downto 0):="00000110";   -- Output Modul
     
    constant SEL_OUTMOD3_ID  :  std_logic_vector(7 downto 0):="00000001";   -- 5 In/1 Out Modul in slot x
    constant SEL_OUTMOD4_ID  :  std_logic_vector(7 downto 0):="00000010";   -- 5 In/1 Out Modul in slot x

      
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

  signal extension_cid_system:integer range 0 to 16#FFFF#;  -- in,  extension card: cid_system
  signal extension_cid_group: integer range 0 to 16#FFFF#;  --in, extension card: cid_group

  signal Max_AWOut_Reg_Nr:    integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
  signal Max_AWIn_Reg_Nr:     integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung

  signal AWIn_Deb_Time:       integer range 0 to 7;           -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time", Wert aus DIOB-Config 1
  signal Min_AWIn_Deb_Time:   integer range 0 to 7;           -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time"

 -- signal fg_start:           std_logic;

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
--  signal AWIn_Reg_Array:        t_IO_Reg_1_to_7_Array;          -- Copy der AWIn-Register in ein Array
  signal test_port_in_0:  std_logic_vector(15 downto 0);
  signal test_clocks:     std_logic_vector(15 downto 0);

  signal s_nLED_Sel:      std_logic;   -- LED = Sel
  signal s_nLED_Dtack:    std_logic;   -- LED = Dtack
  signal s_nLED_inR:      std_logic;   -- LED = interrupt

  signal s_nLED:          std_logic_vector(7 downto 0); -- LED's
  signal s_nLED_Out:      std_logic_vector(7 downto 0); -- LED's
  signal AW_ID:           std_logic_vector(7 downto 0); -- Anwender_ID


--------------------------- Conf_Sts1 ----------------------------------------------------------------------

  signal DIOB_Config1:           std_logic_vector(15 downto 0);
  signal DIOB_Config2:           std_logic_vector(15 downto 0);
  signal DIOB_Status1:           std_logic_vector(15 downto 0);
  signal DIOB_Status2:           std_logic_vector(15 downto 0);
  signal AW_Config1:             std_logic_vector(15 downto 0);
  signal AW_Config2:             std_logic_vector(15 downto 0);
  signal AW_Status1:             std_logic_vector(15 downto 0);
  signal AW_Status2:             std_logic_vector(15 downto 0);

  signal Diob_Config1_wr:        std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config1
  signal Diob_Config2_wr:        std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config2
  signal AW_Config1_wr:          std_logic;                        -- write-Strobe, Daten-Reg. AW_Config1
  signal AW_Config2_wr:          std_logic;                        -- write-Strobe, Daten-Reg. AW_Config2
  signal Clr_Tag_Config:         std_logic;                        -- clear alle Tag-Masken

  signal Conf_Sts1_rd_active:    std_logic;
  signal Conf_Sts1_Dtack:        std_logic;
  signal Conf_Sts1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal LA_Conf_Sts1:           std_logic_vector(15 downto 0);

--------------------------- AWIn ----------------------------------------------------------------------

  signal SCU_AW_Input_Reg:        t_IO_Reg_1_to_7_Array;  -- Input-Register zum SCU-Bus
  signal AW_Input_Reg:            t_IO_Reg_1_to_7_Array;  -- Input-Register von den Piggy's

--------------------------- AWOut ----------------------------------------------------------------------

  signal SCU_AW_Output_Reg:         t_IO_Reg_1_to_7_Array;  -- Output-Register vom SCU-Bus
  signal AW_Output_Reg:             t_IO_Reg_1_to_7_Array;  -- Output-Register zu den Piggy's
--

  signal sel: std_logic_vector(19 downto 0) := (OTHERS => '0');

  --------------------------- INL_xor ----------------------------------------------------------------------

  signal Interlock: std_logic;

  signal hp_la_o:      std_logic_vector(15 downto 0); -- Output für HP-Logicanalysator

  signal s_nLED_User1_i: std_logic;  -- LED3 = User 1
  signal s_nLED_User2_i: std_logic;  -- LED2 = User 2
  signal s_nLED_User3_i: std_logic;  -- LED1 = User 3
  signal s_nLED_User1_o: std_logic;  -- LED3 = User 1
  signal s_nLED_User2_o: std_logic;  -- LED2 = User 2
  signal s_nLED_User3_o: std_logic;  -- LED1 = User 3

  signal uart_txd_out:  std_logic;


    ------------ IO-Port-Signale --------------------------------------------------------------------------------------

    ------------ Mirror-Mode-Signale -------------------------------------------------------------------------------------

  signal Mirr_OutReg_Maske:     std_logic_vector(15 downto 0);  -- Maskierung für Spiegel-Modus des Ausgangsregisters
  signal Mirr_AWOut_Reg_Nr:     integer range 0 to 7;           -- AWOut-Reg-Nummer
  signal Mirr_AWIn_Reg_Nr:      integer range 0 to 7;           -- AWIn-Reg-Nummer

  TYPE   t_word_array     is array (0 to 15) of std_logic_vector(15 downto 0);
   
  signal Tag_matched_7_0 : STD_LOGIC_VECTOR(7 DOWNTO 0) := (OTHERS => '0');


--  +============================================================================================================================+
--  |    §§§              Übergabe-Signale für Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen                 |
--  +============================================================================================================================+


  signal IOBP_Output: std_logic_vector(18 downto 1); -- Data_Output "Slave-Karten 1-12"

  TYPE   t_input_array      is array (1 to 12) of std_logic_vector(5 downto 1);
  signal IOBP_Input:        t_input_array;    -- Inputs der "Slave-Karten"

  TYPE   t_id_array         is array (1 to 12) of std_logic_vector(7 downto 0);
  signal IOBP_ID:           t_id_array;     -- ID's der "Slave-Karten"
  
  TYPE   t_led_array        is array (1 to 12) of std_logic_vector(6 downto 1);
  signal IOBP_Sel_LED:      t_led_array;    -- Sel-LED's der "Slave-Karten"

  signal IOBP_Aktiv_LED_i:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"
  signal IOBP_Aktiv_LED_o:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"

  signal IOBP_STR_rot_o:    std_logic_vector(12 downto 1);  -- LED-Str Rot  für Slave 12-1
  signal IOBP_STR_gruen_o:  std_logic_vector(12 downto 1);  -- LED-Str Grün für Slave 12-1
  signal IOBP_STR_ID_o:     std_logic_vector(12 downto 1);  -- ID-Str Grün für Slave 12-1
  signal IOBP_LED_o:        std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
  signal IOBP_LED_ID_Bus_o: std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
  signal IOBP_LED_ID_Bus_i: std_logic_vector(7 downto 0);   -- LED_ID_Bus_In
  signal IOBP_LED_En:       std_logic;                      -- Output-Enable für LED- ID-Bus

  signal Slave_Loop_cnt   : integer range 0 to 12;         -- 1-12   -- Loop-Counter

  type   IOBP_LED_state_t is   (IOBP_idle, led_id_wait, led_id_loop, led_str_rot_h, led_str_rot_l, led_gruen,
                                led_str_gruen_h, led_str_gruen_l, iobp_led_dis, iobp_led_z, iobp_id_str_h, iobp_rd_id, iobp_id_str_l, iobp_end);
  signal IOBP_state:   IOBP_LED_state_t:= IOBP_idle;

  --------------------------- IO-Select ----------------------------------------------------------------------
TYPE   t_Mask_word_array     is array (0 to 7) of std_logic_vector(15 downto 0);
signal IOBP_Masken_Reg     : t_Mask_word_array := (OTHERS => (OTHERS => '0'));

signal IOBP_msk_rd_active  : std_logic:='0';
signal IOBP_msk_Dtack      : std_logic:='0';
signal IOBP_msk_data_to_SCUB: std_logic_vector(15 downto 0):= (OTHERS => '0');

signal IOBP_Id_Reg         : t_Mask_word_array := (OTHERS => (OTHERS => '0'));

signal IOBP_id_rd_active   : std_logic:='0';
signal IOBP_id_Dtack       : std_logic :='0';
signal IOBP_id_data_to_SCUB: std_logic_vector(15 downto 0) := (OTHERS => '0');

signal IOBP_hw_il_rd_active: std_logic:='0';
signal IOBP_hw_il_Dtack    : std_logic:='0';
signal IOBP_hw_il_data_to_SCUB: std_logic_vector(15 downto 0) := (OTHERS => '0');

signal IOBP_in_data_to_SCUB: std_logic_vector(15 downto 0) := (OTHERS => '0');
signal IOBP_in_rd_active   : std_logic:='0';
signal IOBP_in_Dtack       : std_logic:='0';

signal Signal60_in         : std_logic_vector(59 downto 0) := (OTHERS => '0');
signal Deb60_out           : std_logic_vector(59 downto 0) := (OTHERS => '0');

signal Syn60               : std_logic_vector(59 downto 0) := (OTHERS => '0');
signal Deb_Sync60          : std_logic_vector(59 downto 0) := (OTHERS => '0');
----------------------------------------------------------------------------------------------------------------------------------
--  +============================================================================================================================+
--  |    §§§              Übergabe-Signale für Anwender-IO: FG902_xxx -- Newe Interlock-Backplane mit 12 Steckplätzen                 |
--  +============================================================================================================================+

TYPE t_IOBP_array  is array (1 to 12) of std_logic_vector(5 downto 0);
TYPE t_IOBP_arrayS is array (0 to 11) of std_logic_vector(5 downto 0);
type t_reg_array   is array (1 to 12) of std_logic_vector(7 downto 0);

--TYPE t_Arr_0_11_5to0_ is array (0 to 11) of std_logic_vector(5 downto 0); --t_IOBP_arrayS
--TYPE t_Arr_1_12_7to0_ is array (1 to 12) of std_logic_vector(7 downto 0); --t_id_array,        --t_reg_array
--TYPE t_Arr_1_12_6to1_ is array (1 to 12) of std_logic_vector(6 downto 1);--t_led_array
--TYPE t_Arr_1_12_5to0_ is array (1 to 12) of std_logic_vector(5 downto 0); --t_IOBP_array

--TYPE t_Arr_1_12_5to1_ is array (1 to 12) of std_logic_vector(5 downto 1); --t_input_array
--TYPE t_Arr_0_7_word  is array (0 to 7) of std_logic_vector(15 downto 0); --t_Mask_word_array, t_IO_Reg_0_to_7_Array
--TYPE t_Arr_1_7_word  is array (1 to 7)  of std_logic_vector(15 downto 0); --t_IO_Reg_1_to_7_Array

signal IOBP_SK_Output      : t_IOBP_array  := (OTHERS =>(OTHERS => '0'));    -- Outputs "Slave-Karten 1-12"  --but I use only 1-2-3 respectiverly for slot 10-11-12
signal IOBP_SK_Input       : t_IOBP_array  := (OTHERS =>(OTHERS => '0'));    -- Inputs "Slave-Karten 1-12"
signal IOBP_SK_Sel_LED     : t_led_array   := (OTHERS =>(OTHERS => '0'));
signal IOBP_SK_Aktiv_LED_i : t_led_array   := (OTHERS =>(OTHERS => '0'));

signal IOBP_Output_Readback: t_IO_Reg_0_to_7_Array:= (OTHERS =>(OTHERS => '0'));

signal Deb72_in            : t_IOBP_arrayS := (OTHERS =>(OTHERS => '0')); --std_logic_vector(71 downto 0);
signal Deb72_out           : t_IOBP_arrayS := (OTHERS =>(OTHERS => '0')); --std_logic_vector(71 downto 0);

signal Syn72               : t_IOBP_arrayS := (OTHERS =>(OTHERS => '0'));
signal Deb_Sync72          : t_IOBP_arrayS := (OTHERS =>(OTHERS => '0'));

signal AW_SK_Input_Reg     : t_IOBP_arrayS := (OTHERS =>(OTHERS => '0'));  -- Input-Register von den Piggy's

signal PIO_ENA_SLOT        : t_IOBP_array := (OTHERS =>(OTHERS => '0')); --1..12 of (5 down to 0)
signal PIO_OUT_SLOT        : t_IOBP_array := (OTHERS =>(OTHERS => '0')); --1..12 of (5 down to 0)

signal conf_reg            : t_reg_array  := (OTHERS =>(OTHERS => '0'));


--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: Out16   -- FG901_010                                   |
--  +============================================================================================================================+

--
  signal Out16_LED_Lemo_In_i:    std_logic;  --  Input  "nLED_Lemo_In"
  signal Out16_nLED_Lemo_In_o:   std_logic;  --  Output "nLED_Lemo_In"
--
  signal Out16_Lemo_deb_i:       std_logic;  -- Input  "UIO_Lemo_in"
  signal Out16_Lemo_deb_o:       std_logic;  -- Output "UIO_Lemo_in"


  --  +============================================================================================================================+

  signal clk_blink: std_logic;

  signal sys_clk_is_bad:          std_logic;
  signal sys_clk_is_bad_led_n:    std_logic;
  signal sys_clk_is_bad_la:       std_logic;
  signal local_clk_is_bad:        std_logic;
  signal local_clk_is_running:    std_logic;
  signal local_clk_runs_led_n:    std_logic;
  signal sys_clk_failed:          std_logic;
  signal sys_clk_deviation:       std_logic;
  signal sys_clk_deviation_la:    std_logic;
  signal sys_clk_deviation_led_n: std_logic;
  signal clk_switch_rd_data:      std_logic_vector(15 downto 0);
  signal clk_switch_rd_active:    std_logic;
  signal clk_switch_dtack:        std_logic;
  signal pll_locked:              std_logic;
  signal clk_switch_intr:         std_logic;

  signal  signal_tap_clk_250mhz:  std_logic;
  signal  clk_update:             std_logic;
  signal  clk_flash:              std_logic;


  signal  rstn_sys:               std_logic;
  signal  rstn_update:            std_logic;
  signal  rstn_flash:             std_logic;
  signal  rstn_stc:               std_logic;

  constant c_is_arria5: boolean := false;


--%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  signal  PIO_SYNC:              STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_SYNC1:             STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_ENA:               STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_ENA_SYNC:          STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_OUT:               STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_OUT_SYNC:          STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%

  signal  UIO_SYNC:              STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_SYNC1:             STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_ENA:               STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_ENA_SYNC:          STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_OUT:               STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_OUT_SYNC:          STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
--%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

component zeitbasis
generic (
    CLK_in_Hz:      integer;
    diag_on:      integer
    );
port  (
    Res:        in  std_logic;
    Clk:        in  std_logic;
    Ena_every_100ns:  out std_logic;
    Ena_every_166ns:  out std_logic;
    Ena_every_250ns:  out std_logic;
    Ena_every_500ns:  out std_logic;
    Ena_every_1us:    out std_logic;
    Ena_Every_20ms:   out std_logic
    );
end component;



----BLM signals
signal triggerarr     : t_arr_Trigout                 := (OTHERS => (OTHERS =>'0'));
signal TriggerOutVec  : t_arr_Trigoutmax              := (OTHERS => (OTHERS =>'0'));
signal PulsCountin    : std_logic_vector(11 downto 0) := (OTHERS => '0');
signal BLM_rd_active  : std_logic                     := '0';
signal BLM_data_to_SCUB: STD_LOGIC_VECTOR(15 DOWNTO 0):= (OTHERS => '0');
signal BLM_Dtack      : std_logic                     := '0';
signal PulseInVec     : STD_LOGIC_VECTOR(29 DOWNTO 0) := (OTHERS => '0');


type   PIN_CONSTANTS is array (0 TO 5) OF INTEGER;
 
 --Generate Mux for INPUT signals
PROCEDURE INREG 
  (
      constant  PINS_A         : PIN_CONSTANTS :=(0,0,0,0,0);
      constant  CARDSEL        : STD_LOGIC_VECTOR(7 downto 0) :="00000000";
      constant  I              : integer := 0
   ) IS
      
   BEGIN               
      
      if CARDSEL = IOBP_ID(I+1) then                  
         if I MOD 2 = 0 then
            AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(5 downto 0));
            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(5 downto 0));   -- Register für Sel-LED's vom Slave x                                                      
            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
         else
            AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(11 downto 6));
            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(11 downto 6));   -- Register für Sel-LED's vom Slave x                                                      
            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
         end if;         
         
         IOBP_SK_Output(I+1)     <= (OTHERS => '0');
         --IOBP_SK_Sel_LED(I+1)    <= not ( IOBP_Masken_Reg(I) );  -- Register für Sel-LED's vom Slave x
         IOBP_SK_Sel_LED(I+1)    <= Deb72_out(I);          -- Signale für Aktiv-LED's                                                                                                                                                         
         IOBP_SK_Input(I+1)      <= PIO_SYNC(PINS_A(0)) &  PIO_SYNC(PINS_A(1)) &  PIO_SYNC(PINS_A(2))&  PIO_SYNC(PINS_A(3)) &  PIO_SYNC(PINS_A(4))&  PIO_SYNC(PINS_A(5));
       end if;              
  
   END INREG;
   


-- PROCEDURE OUTREG 
--  (
--      constant  PINS_A         : PIN_CONSTANTS :=(0,0,0,0,0);
--      constant  CARDSEL        : STD_LOGIC_VECTOR(7 downto 0) :="00000000";
--      constant  I              : integer := 0
--   ) IS
--      
--   BEGIN   
--            
--            
--            
--      
--      if CARDSEL = IOBP_ID(I+1) then 
--
--         if I MOD 2 = 0 then
--            --AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(5 downto 0));
--            IOBP_SK_Output(I+1)(5  downto 0) <= triggerarr(0) AND not IOBP_Masken_Reg(I)(5  downto 0);
--            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(5 downto 0));   -- Register für Sel-LED's vom Slave x                                                      
--            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
--         else
--            AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(11 downto 6));
--            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(11 downto 6));   -- Register für Sel-LED's vom Slave x                                                      
--            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
--         end if;             
--      
--         AW_SK_Input_Reg(I)<=   (OTHERS => '0');
--         --IOBP_SK_Output(6) <= AW_Output_Reg(3)(11 downto  6) AND not IOBP_Masken_Reg(2)(11 downto 6);
--         IOBP_SK_Output(I+1)(5  downto 0) <= triggerarr(0) AND not IOBP_Masken_Reg(2)(5  downto 0);
--         
--          PIO_OUT_SLOT(6) <= IOBP_SK_Output(I+1);
--         PIO_ENA_SLOT(6) <= std_logic_vector'("111111");
--         --IOBP_SK_Aktiv_LED_i(I+1)  <=  IOBP_SK_Output(6);
--         IOBP_SK_Aktiv_LED_i(I+1)  <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
--         IOBP_SK_Input(I+1)  <= (OTHERS => '0');
--         --IOBP_SK_Sel_LED(I+1)   <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
--         IOBP_SK_Sel_LED(I+1)   <= IOBP_SK_Output(I+1);
--                  
--                  
--         if I MOD 2 = 0 then
--            AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(5 downto 0));
--            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(5 downto 0));   -- Register für Sel-LED's vom Slave x                                                      
--            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
--         else
--            AW_SK_Input_Reg(I)      <= (Deb_Sync72(I)   AND not IOBP_Masken_Reg(I)(11 downto 6));
--            IOBP_sK_Aktiv_LED_i(I+1)<= not ( IOBP_Masken_Reg(I)(11 downto 6));   -- Register für Sel-LED's vom Slave x                                                      
--            --IOBP_sK_Aktiv_LED_i(I+1) <= Deb72_out(I);          -- Signale für Aktiv-LED's         
--         end if;         
--         
--         IOBP_SK_Output(I+1)     <= (OTHERS => '0');
--         --IOBP_SK_Sel_LED(I+1)    <= not ( IOBP_Masken_Reg(I) );  -- Register für Sel-LED's vom Slave x
--         IOBP_SK_Sel_LED(I+1)    <= Deb72_out(I);          -- Signale für Aktiv-LED's                                                                                                                                                         
--         IOBP_SK_Input(I+1)      <= PIO_SYNC(PINS_A(0)) &  PIO_SYNC(PINS_A(1)) &  PIO_SYNC(PINS_A(2))&  PIO_SYNC(PINS_A(3)) &  PIO_SYNC(PINS_A(4))&  PIO_SYNC(PINS_A(5));
--       end if;              
--  
--   END OUTREG;
  
   
  
  
--  case conf_reg(6) is
--
--              when SEL_OUTMOD1_ID  | SEL_OUTMOD2_ID => -- Output Modul in slot 6
--                  AW_SK_Input_Reg(5)<=   (OTHERS => '0');
--                  --IOBP_SK_Output(6) <= AW_Output_Reg(3)(11 downto  6) AND not IOBP_Masken_Reg(2)(11 downto 6);
--                  IOBP_SK_Output(6)(5  downto 0) <= triggerarr(0) AND not IOBP_Masken_Reg(2)(5  downto 0);
--                  
--                   PIO_OUT_SLOT(6) <= IOBP_SK_Output(6);
--                  PIO_ENA_SLOT(6); <= std_logic_vector'("111111");
--                  --IOBP_SK_Aktiv_LED_i(6)  <=  IOBP_SK_Output(6);
--                  IOBP_SK_Aktiv_LED_i(6)  <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
--                  IOBP_SK_Input(6)  <= (OTHERS => '0');
--                  --IOBP_SK_Sel_LED(6)   <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
--                  IOBP_SK_Sel_LED(6)   <= IOBP_SK_Output(6);
--
--              when others     =>  NULL;
--         end case;
--  
--  ###############################################################################################################################
--  ###############################################################################################################################
--  #####                                                                                                                     #####
--  #####                                                 BEGIN                                                               #####
--  #####                                                                                                                     #####
--  ###############################################################################################################################
--  ###############################################################################################################################


  begin


  A_nADR_EN             <= '0';
  A_nADR_FROM_SCUB      <= '0';
  A_nExt_Signal_in      <= '0';
  A_nSEL_Ext_Signal_DRV <= '0';
  A_nUser_EN            <= '0';


--  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
--  %%%%%                          I/O-Synch und TriState-Steuerung                                          %%%%%
--  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



  p_in_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      PIO_SYNC   <= (others => '0');
      PIO_SYNC1  <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      PIO_SYNC   <= PIO_SYNC1;
      PIO_SYNC1  <= PIO;
    end if;
  end process p_in_sync;

  p_out_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      PIO_OUT_SYNC   <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      PIO_OUT_SYNC   <= PIO_OUT;
    end if;
  end process p_out_sync;

  p_ena_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      PIO_ENA_SYNC   <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      PIO_ENA_SYNC   <= PIO_ENA;
    end if;
  end process p_ena_sync;


  p_diob_tristates: for I in 16 to 150 generate
  process (PIO, PIO_OUT_SYNC, PIO_ENA_SYNC)
  begin
    if  PIO_ENA_SYNC(I)  = '0' then
        PIO(I)          <= 'Z';
    else
        PIO(I)          <= PIO_OUT_SYNC(I);
    end if;
  end process p_diob_tristates;
  end generate p_diob_tristates;



  u_in_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      UIO_SYNC   <= (others => '0');
      UIO_SYNC1  <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      UIO_SYNC   <= UIO_SYNC1;
      UIO_SYNC1  <= UIO;
    end if;
  end process u_in_sync;

  u_out_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      UIO_OUT_SYNC   <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      UIO_OUT_SYNC   <= UIO_OUT;
    end if;
  end process u_out_sync;

  u_ena_sync:
  process (clk_sys, rstn_sys)
  begin
    if  ( not rstn_sys    = '1') then
      UIO_ENA_SYNC   <= (others => '0');
    elsif (rising_edge(clk_sys)) then
      UIO_ENA_SYNC   <= UIO_ENA;
    end if;
  end process u_ena_sync;


  u_diob_tristates: for I in 0 to 15 generate
  process (UIO, UIO_OUT_SYNC, UIO_ENA_SYNC)
  begin
    if  UIO_ENA_SYNC(I)  = '0' then
        UIO(I)          <= 'Z';
    else
        UIO(I)          <= UIO_OUT_SYNC(I);
    end if;
  end process u_diob_tristates;
  end generate u_diob_tristates;



--  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
--  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


-------------Debouncer
Deb60:  for I in 0 to 59 generate
    DB_I:  diob_debounce
    GENERIC MAP (DB_Tst_Cnt   => 3,
                 Test         => 0)            
              port map(DB_Cnt => Debounce_cnt,  -- Debounce-Zeit in Clock's
                       DB_in  => Signal60_in(I),-- Signal-Input
                       Reset  => not rstn_sys,  -- Powerup-ReIOBP_id_rd_activeset
                       clk    => clk_sys,       -- Sys-Clock
                       DB_Out => Deb60_out(I)); -- Debounce-Signal-Out
end generate Deb60;
-------------


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
    generic map(
      g_plls   => 1,
      g_clocks => 4,
      g_areset => f_pick(c_is_arria5, 100, 1)*1024,
      g_stable => f_pick(c_is_arria5, 100, 1)*1024)
    port map(
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


Conf_Sts1: config_status
generic map(
      CS_Base_addr =>   c_Conf_Sts1_Base_Addr
           )
port map  (

      Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,          -- latched address from SCU_Bus
      Data_from_SCUB_LA   =>  Data_from_SCUB_LA,         -- latched data from SCU_Bus
      Ext_Adr_Val         =>  Ext_Adr_Val,               -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active       =>  Ext_Rd_active,             -- '1' => Rd-Cycle is active
      Ext_Rd_fin          =>  Ext_Rd_fin,                -- marks end of read cycle, active one for one clock period of sys_clk
      Ext_Wr_active       =>  Ext_Wr_active,             -- '1' => Wr-Cycle is active
      Ext_Wr_fin          =>  SCU_Ext_Wr_fin,            -- marks end of write cycle, active one for one clock period of sys_clk
      clk                 =>  clk_sys,                   -- should be the same clk, used by SCU_Bus_Slave
      nReset              =>  rstn_sys,

      Diob_Status1        =>  Diob_Status1,              -- Input-Diob_Status1
      Diob_Status2        =>  Diob_Status2,              -- Input-Diob_Status2
      AW_Status1          =>  AW_Status1,                -- Input-AW_Status1
      AW_Status2          =>  AW_Status2,                -- Input-AW_Status2

      Diob_Config1        =>  Diob_Config1,              -- Daten-Reg_Diob_Config1
      Diob_Config2        =>  Diob_Config2,              -- Daten-Reg_Diob_Config2
      AW_Config1          =>  AW_Config1,                -- Daten-Reg_AW_Config1
      AW_Config2          =>  AW_Config2,                -- Daten-Reg_AW_Config2
      Clr_Tag_Config      =>  Clr_Tag_Config,            -- Clear Tag-Konfigurations-Register

      Diob_Config1_wr     =>  Diob_Config1_wr,           -- write-Strobe, Daten-Reg. AWOut1
      Diob_Config2_wr     =>  Diob_Config2_wr,           -- write-Strobe, Daten-Reg. AWOut2
      AW_Config1_wr       =>  AW_Config1_wr,             -- write-Strobe, Daten-Reg. AWOut3
      AW_Config2_wr       =>  AW_Config2_wr,             -- write-Strobe, Daten-Reg. AWOut4

      Mirr_OutReg_Maske   =>  Mirr_OutReg_Maske,         -- Maskierung für Spiegel-Modus des Ausgangsregisters

      Rd_active           =>  Conf_Sts1_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
      Dtack_to_SCUB       =>  Conf_Sts1_Dtack,           -- connect read sources to SCUB-Macro
      Data_to_SCUB        =>  Conf_Sts1_data_to_SCUB,    -- connect Dtack to SCUB-Macro
      LA                  =>  LA_Conf_Sts1
      );


IOBP_Maske: io_reg
generic map(
      Base_addr =>  c_IOBP_Masken_Base_Addr
      )
port map  (
      Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
      Data_from_SCUB_LA  =>  Data_from_SCUB_LA,
      Ext_Adr_Val        =>  Ext_Adr_Val,
      Ext_Rd_active      =>  Ext_Rd_active,
      Ext_Rd_fin         =>  Ext_Rd_fin,
      Ext_Wr_active      =>  Ext_Wr_active,
      Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
      clk                =>  clk_sys,
      nReset             =>  rstn_sys,
--
      Reg_IO1            =>  IOBP_Masken_Reg(0),      
      Reg_IO2            =>  IOBP_Masken_Reg(1),
      Reg_IO3            =>  IOBP_Masken_Reg(2),
      Reg_IO4            =>  IOBP_Masken_Reg(3),
      Reg_IO5            =>  IOBP_Masken_Reg(4),
      Reg_IO6            =>  IOBP_Masken_Reg(5),
      Reg_IO7            =>  IOBP_Masken_Reg(6),
      Reg_IO8            =>  open,
      Reg_rd_active      =>  IOBP_msk_rd_active,
      Dtack_to_SCUB      =>  IOBP_msk_Dtack,
      Data_to_SCUB       =>  IOBP_msk_data_to_SCUB
    );

    
IOBP_ID_RegZ: in_reg
generic map(
      Base_addr =>  c_IOBP_ID_Base_Addr
      )
port map  (
      Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
      Data_from_SCUB_LA  =>  Data_from_SCUB_LA,
      Ext_Adr_Val        =>  Ext_Adr_Val,
      Ext_Rd_active      =>  Ext_Rd_active,
      Ext_Rd_fin         =>  Ext_Rd_fin,
      Ext_Wr_active      =>  Ext_Wr_active,
      Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
      clk                =>  clk_sys,
      nReset             =>  rstn_sys,
--
      Reg_In1            =>  IOBP_ID_Reg(0),
      Reg_In2            =>  IOBP_ID_Reg(1),
      Reg_In3            =>  IOBP_ID_Reg(2),
      Reg_In4            =>  IOBP_ID_Reg(3),
      Reg_In5            =>  IOBP_ID_Reg(4),
      Reg_In6            =>  IOBP_ID_Reg(5),
      Reg_In7            =>  IOBP_ID_Reg(6),
      Reg_In8            =>  IOBP_ID_Reg(7),
--
      Reg_rd_active      =>  IOBP_id_rd_active,
      Dtack_to_SCUB      =>  IOBP_id_Dtack,
      Data_to_SCUB       =>  IOBP_id_data_to_SCUB
    );

IOBP_Readout_Reg: in_reg
    generic map(
          Base_addr =>  c_IOBP_READBACK_Base_Addr
          )
    port map  (
          Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
          Data_from_SCUB_LA  =>  Data_from_SCUB_LA,
          Ext_Adr_Val        =>  Ext_Adr_Val,
          Ext_Rd_active      =>  Ext_Rd_active,
          Ext_Rd_fin         =>  Ext_Rd_fin,
          Ext_Wr_active      =>  Ext_Wr_active,
          Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
          clk                =>  clk_sys,
          nReset             =>  rstn_sys,
    --
          Reg_In1            =>  IOBP_Output_Readback(0),
          Reg_In2            =>  IOBP_Output_Readback(1),
          Reg_In3            =>  IOBP_Output_Readback(2),
          Reg_In4            =>  IOBP_Output_Readback(3),
          Reg_In5            =>  IOBP_Output_Readback(4),
          Reg_In6            =>  IOBP_Output_Readback(5),
          Reg_In7            =>  IOBP_Output_Readback(6),
          Reg_In8            =>  IOBP_Output_Readback(7),
    --
          Reg_rd_active      =>  IOBP_in_rd_active,
          Dtack_to_SCUB      =>  IOBP_in_Dtack,
          Data_to_SCUB       =>  IOBP_in_data_to_SCUB
        );

PulseInVec<=AW_SK_Input_Reg(4) & AW_SK_Input_Reg(3) & AW_SK_Input_Reg(2) & AW_SK_Input_Reg(1) & AW_SK_Input_Reg(0);

NewBLM:BLM_Head
    generic map (
      BLM_Start_addr =>BLM_Base_Addr
    )
  port map
    (
   Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,          -- latched address from SCU_Bus
   Data_from_SCUB_LA   =>  Data_from_SCUB_LA,         -- latched data from SCU_Bus
   Ext_Adr_Val         =>  Ext_Adr_Val,               -- '1' => "ADR_from_SCUB_LA" is valid
   Ext_Rd_active       =>  Ext_Rd_active,             -- '1' => Rd-Cycle is active
   --   Ext_Rd_fin          =>  Ext_Rd_fin,           -- marks end of read cycle, active one for one clock period of sys_clk
   Ext_Wr_active       =>  Ext_Wr_active,             -- '1' => Wr-Cycle is active
   --Ext_Wr_fin          =>  SCU_Ext_Wr_fin,          -- marks end of write cycle, active one for one clock period of sys_clk
   clk                 =>  clk_sys,                   -- should be the same clk, used by SCU_Bus_Slave
   nReset              =>  rstn_sys,
   Tclk1us             =>  Ena_every_1us,
   -- system IO

   --external IO
   PulsCountin         => PulseInVec(CHAN_NUM-1 downto 0), -- Pulse values to count for each module
   TrigOutreg          => triggerarr,                 -- FBAS trigger out-  is array (0 to CHAN_NUM-1) of std_logic_vector(5 downto 0);

   user_rd_active      =>  BLM_rd_active,             -- '1' = read data avaliable at 'Data_to_SCUB'-output
   Data_to_SCUB        =>  BLM_data_to_SCUB,          --
   Dtack_to_SCUB       =>  BLM_Dtack                  -- connect read sources to SCUB-Macro
);





testport_mux: process (A_SEL
----------------- AW_Config1, AW_Input_Reg, AW_Output_Reg, LA_Tag_Ctrl1,
                 --      LA_AW_Port1, LA_Conf_Sts1, Timing_Pattern_RCV,
                --       Timing_Pattern_LA, test_port_in_0, test_clocks, uart_txd_out,
                --       Ext_Rd_active, Ext_Rd_fin, Ext_Rd_Fin_ovl, Ext_Wr_active, SCU_Ext_Wr_fin, Ext_Wr_fin_ovl
                       )
begin
  case (not A_SEL) is
--    when X"0" => test_out <= AW_Config1;
--    when X"1" => test_out <= AW_Input_Reg(1);
--    when X"2" => test_out <= AW_Input_Reg(2);
--    when X"3" => test_out <= AW_Input_Reg(3);
----
--    when X"4" => test_out <= AW_Output_Reg(1);
--    when X"5" => test_out <= AW_Output_Reg(2);
--    when X"6" => test_out <= AW_Output_Reg(3);
--                                                 +-------------------- '1' drives the external max level shifter
    when X"7" => test_out <= X"000" & '0' & '0' & '1' & uart_txd_out;
--
    --when X"8" => test_out <= LA_Tag_Ctrl1;   -- Logic analyser Signals "LA_Tag_Ctrl1"

    when X"9" => test_out <= LA_Conf_Sts1;
    --when X"A" => test_out <= LA_AW_Port1;
--

    when X"B" => test_out <= X"00"&
                              '0' &
                              '0' &
                              Ext_Rd_active  &  -- out, '1' => Rd-Cycle to external user register is active
                              Ext_Rd_fin     &  -- out, marks end of read cycle, active one for one clock period of clk past cycle end (no overlap)
                              Ext_Rd_Fin_ovl &  -- out, marks end of read cycle, active one for one clock period of clk during cycle end (overlap)
                              Ext_Wr_active  &  -- out, '1' => Wr-Cycle to external user register is active
                              SCU_Ext_Wr_fin &  -- out, marks end of write cycle, active high for one clock period of clk past cycle end (no overlap)
                              Ext_Wr_fin_ovl;   -- out, marks end of write cycle, active high for one clock period of clk before write cycle finished (with overlap)
--
    when X"C" => test_out <= Timing_Pattern_RCV & Timing_Pattern_LA(14 downto 0);-- Timing
   --
    when X"D" =>    test_out <= X"0000";
    when X"E" =>    test_out <= test_clocks;
    when X"F" =>    test_out <= test_port_in_0;
    when others =>  test_out <= (others => '0');
  end case;
end process testport_mux;


hp_la_o <=  x"0000";  --test_out(15 downto 0);


test_port_in_0 <= x"0000"; --- kein Clock's am Teststecker

--test_port_in_0 <= rstn_sys              & clk_sys         & Ena_Every_100ns & Ena_Every_166ns & -- bit15..12
--                  Ext_Wr_active         & SCU_Ext_Wr_fin  & '0'             & FG_1_strobe     & -- bit11..8
--                  signal_tap_clk_250mhz & pll_locked      & A_RnW & A_nDS   &                   -- bit7..4
--                  A_nBoardSel           & FG_1_strobe     & '0'             & SCUB_Dtack      ; -- bit3..0


test_clocks <=  X"0"                                                                              -- bit15..12
              -- use only outputs from PLL
              --& '0' & signal_tap_clk_250mhz & A_SysClock & CLK_20MHz_D                          -- bit11..8
              & '0' & '0' & '0' & '0'                                                             -- bit11..8
              & '0' & pll_locked & sys_clk_deviation & sys_clk_deviation_la                       -- bit7..4
              & local_clk_is_running & local_clk_is_bad & sys_clk_is_bad & sys_clk_is_bad_la;     -- bit3..0


  -- open drain buffer for one wire
owr_i(0) <= A_OneWire;
A_OneWire <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';

zeit1 : zeitbasis
generic map (
      CLK_in_Hz =>  clk_sys_in_Hz,
      diag_on   =>  1
      )
port map  (
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

--p_led_pu: led_n
--  generic map (stretch_cnt => stretch_cnt)
--  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => not (rstn_sys), nLED => s_nLED_PU);-- LED: rstn_syset

p_led_user1: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User1_i, nLED => s_nLED_User1_o);-- LED3 = User 1

p_led_user2: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User2_i, nLED => s_nLED_User2_o);-- LED3 = User 1

p_led_user3: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys, Sig_in => s_nLED_User3_i, nLED => s_nLED_User3_o);-- LED3 = User 1



A_nLED_D2 <=   s_nLED_Sel;    -- Diagnose-LED_D2 = BoardSelekt
A_nLED_D3 <=   s_nLED_Dtack;  -- Diagnose-LED_D3 = Dtack


sel_every_10ms: div_n
  generic map (n => integer(10.0e-3 / 1.0e-6), diag_on => 0)  -- ena nur jede us für einen Takt aktiv, deshalb n = 10000
    port map  ( res => not rstn_sys,
                clk => clk_sys,
                ena => ENA_every_1us,
                div_o => ENA_every_10ms
              );

sel_every_250ms: div_n
  generic map (n => 12, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 13x20ms = 260ms
    port map  ( res => not rstn_sys,
                clk => clk_sys,
                ena => Ena_Every_20ms,
                div_o => ENA_every_250ms
              );

sel_every_500ms: div_n
  generic map (n => 25, diag_on => 0)  -- ena nur alle 20ms fr einen Takt aktiv, deshalb 25x20ms = 500ms
    port map  ( res => not rstn_sys,
                clk => clk_sys,
                ena => Ena_Every_20ms,
                div_o => ENA_every_500ms
              );


p_clk_blink:
process (clk_sys, rstn_sys, ENA_every_250ms)
begin
  if  ( not rstn_sys    = '1') then
      clk_blink   <= '0';
  elsif (rising_edge(clk_sys)) then
    if (ENA_every_500ms = '1') then
      clk_blink <= not clk_blink;
    end if;
  end if;
end process;


clk_switch_intr <= local_clk_is_running or sys_clk_deviation_la;

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
   intr_in                 =>  '0'& '0' & '0' &'0' --FG_1_dreq & FG_2_dreq & tmr_irq & '0'  -- bit 15..12
                             & '0'& '0' & '0' &'0'            -- bit 11..8
                              & x"0"                                  -- bit 7..4
                              & '0' & '0' & clk_switch_intr,          -- bit 3..1
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
  debug_serial_i    => '0'
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

    
sel(7) <= tmr_rd_active;
sel(6) <= BLM_rd_active;
sel(5) <= wb_scu_rd_active;
sel(4) <= clk_switch_rd_active;
sel(3) <= Conf_Sts1_rd_active ;
sel(2) <= IOBP_in_rd_active;
sel(1) <= IOBP_msk_rd_active;
sel(0) <= IOBP_id_rd_active;


rd_port_mux:  process (clk_sys, rstn_sys,sel,
                           clk_switch_rd_active,     clk_switch_rd_data,
                           wb_scu_rd_active,         wb_scu_data_to_SCUB,
                           Conf_Sts1_rd_active,      Conf_Sts1_data_to_SCUB,
                           tmr_rd_active,            tmr_data_to_SCUB,
                           IOBP_msk_rd_active,       IOBP_msk_data_to_SCUB,
                           BLM_rd_active,            BLM_data_to_SCUB,
                           IOBP_in_rd_active,        IOBP_in_data_to_SCUB                                                   
                      )
  begin

     if (rstn_sys = '0') then
         Data_to_SCUB <= (others => '0');
     elsif rising_edge(clk_sys) then

        case sel IS
            when "00000000000010000000" => Data_to_SCUB <= tmr_data_to_SCUB;
            when "00000000000001000000" => Data_to_SCUB <= BLM_data_to_SCUB;
            when "00000000000000100000" => Data_to_SCUB <= wb_scu_data_to_SCUB;
            when "00000000000000010000" => Data_to_SCUB <= clk_switch_rd_data;
            when "00000000000000001000" => Data_to_SCUB <= Conf_Sts1_data_to_SCUB;
            when "00000000000000000100" => Data_to_SCUB <= IOBP_in_data_to_SCUB;
            when "00000000000000000010" => Data_to_SCUB <= IOBP_msk_data_to_SCUB;
            when "00000000000000000001" => Data_to_SCUB <= IOBP_id_data_to_SCUB;

            when others      => Data_to_SCUB <= (others => '0');
        end case;
    end if; --clk
end process rd_port_mux;



-------------- Dtack_to_SCUB -----------------------------

    Dtack_to_SCUB <= (
                      BLM_Dtack
                     or wb_scu_dtack
                     or clk_switch_dtack
                     or Conf_Sts1_Dtack
                     or IOBP_in_Dtack
                     or IOBP_msk_Dtack
                     or IOBP_id_Dtack
                     or tmr_dtack
              );

    A_nDtack <= NOT(SCUB_Dtack);
    A_nSRQ   <= NOT(SCUB_SRQ);

--  +============================================================================================================================+
--  |            §§§                        Anwender-IO: IOBP (INLB12S1)  -- FG902_050                                           |
--  +============================================================================================================================+
Deb72:  for I in 0 to 11 generate
       d2:   for N in 0 to 5 generate
          
         DB_II:  diob_debounce
            GENERIC MAP (
                         DB_Tst_Cnt   => 3,
                         Test         => 0
                         )             
                port map(DB_Cnt => Debounce_cnt,  -- Debounce-Zeit in Clock's
                         DB_in  => Deb72_in(I)(N),-- Signal-Input
                         Reset  => not rstn_sys,  -- Powerup-Reset
                         clk    => clk_sys,       -- Sys-Clock
                         DB_Out => Deb72_out(I)(N)
                         ); -- Debounce-Signal-Out
          end generate d2;          
end generate Deb72;
--
--         =========== Component's für die 72 "aktiv" Led's ===========
--
IOBP_In_LEDn:  for J in 1 to 12 generate
--                ---------------------------------------------------------------------------
   IOBP_In_LEDn_Slave1: for I in 1 to 6 generate
      DB_I: LED_n
               GENERIC MAP (stretch_cnt => stretch_cnt)             --
               port map(ena   => Ena_Every_20ms,          -- Enable-Clock
                       CLK    => clk_sys,                 -- Sys-Clock
                       Sig_in => IOBP_Aktiv_LED_i(J)(I),  -- Signal-Input
                       nLED   => IOBP_Aktiv_LED_o(J)(I)); -- Signal-Out
   end generate IOBP_In_LEDn_Slave1;
--                ---------------------------------------------------------------------------
end generate IOBP_In_LEDn;
--

     -------------------------------------------------------------------------------------------------------
     ------------------------------ Loop für LED_Output's und ID read --------------------------------------
     -------------------------------------------------------------------------------------------------------


P_IOBP_LED_ID_Loop:  process (clk_sys, Ena_Every_250ns, rstn_sys, IOBP_state)

    begin
      if (not rstn_sys = '1') then
        Slave_Loop_cnt       <=   1;                 --  Loop-Counter
        IOBP_LED_En          <=  '0';                --  O-Enable für LED- ID-Bus
        IOBP_STR_rot_o       <=  (others => '0');    --  Led-Strobs 'rot'
        IOBP_STR_gruen_o     <=  (others => '0');    --  Led-Strobs 'grün'
        IOBP_STR_id_o        <=  (others => '0');    --  ID-Strobs


    ELSIF (rising_edge(clk_sys) AND Ena_Every_250ns = '1') THEN
--  ELSIF ((rising_edge(clk_sys)) or Ena_Every_100ns)  then
      case IOBP_state is
        when IOBP_idle   =>  Slave_Loop_cnt       <=  1;      -- Loop-Counter

                            if  ((AW_ID(7 downto 0) = c_AW_INLB12S.ID) or (AW_ID(7 downto 0) = c_AW_INLB12S1.ID)) THEN  IOBP_state  <= led_id_wait;
                                                                       else  IOBP_state  <= IOBP_idle;
                            end if;

        when led_id_wait      =>  IOBP_LED_En          <=  '1';     --  Output-Enable für LED- ID-Bus
                                  IOBP_state  <= led_id_loop;


        when led_id_loop      =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("0" & "0");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  IOBP_Aktiv_LED_o(Slave_Loop_cnt)(6 downto 1);   -- Aktiv-LED für Slave zum LED-Port
                                  IOBP_state  <= led_str_rot_h;

        when led_str_rot_h    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '1';   -- Aktiv LED für Slave (Slave_Loop_cnt) zum LED-Port
                                  IOBP_state  <= led_str_rot_l;

        when led_str_rot_l    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '0';   -- Aktiv LED für Slave (Slave_Loop_cnt) zum LED-Port
                                  IOBP_state  <= led_gruen;

        when led_gruen        =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("0" & "0");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  not IOBP_Sel_LED(Slave_Loop_cnt)(6 downto 1);   -- Sel-LED für Slave zum LED-Port
                                  IOBP_state  <= led_str_gruen_h;

        when led_str_gruen_h  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '1';   -- Sel-LED für Slave (Slave_Loop_cnt) zum LED-Port
                                  IOBP_state  <= led_str_gruen_l;

        when led_str_gruen_l  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '0';   -- Sel-LED für Slave (Slave_Loop_cnt) zum LED-Port
                                  IOBP_state  <= iobp_led_dis;

        when iobp_led_dis     =>  IOBP_LED_En <=  '0';                        --  Disable Output für LED- ID-Bus
                                  IOBP_state  <= iobp_led_z;

        when iobp_led_z       =>  IOBP_state  <= iobp_id_str_l;


        when iobp_id_str_l    =>  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '1';   -- Sel-ID für Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_rd_id;

        when iobp_rd_id       =>  IOBP_ID(Slave_Loop_cnt) <=  IOBP_LED_ID_Bus_i;   -- Sel-ID für Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_id_str_h;

        when iobp_id_str_h    =>  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '0';   -- Sel-ID für Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_end;



        when iobp_end         =>  Slave_Loop_cnt <=  Slave_Loop_cnt + 1;       -- Loop +1

                                  if Slave_Loop_cnt < 13 then
                                    IOBP_state     <= led_id_wait;
                                  else
                                    IOBP_state     <= IOBP_idle;
                                  end if;

        when others           =>  IOBP_state       <= IOBP_idle;

      end case;
    end if;
  end process P_IOBP_LED_ID_Loop;

      
 ID_Front_Board_proc: process (clk_sys, rstn_sys,
                                    IOBP_Masken_Reg,Deb_Sync72,
                                    IOBP_SK_Output,
                                    AW_Output_Reg,
                                    triggerarr,
                                    PIO_SYNC,
                                    IOBP_ID,
                                    Deb_Sync72,
                                    Deb72_out)
  begin

      if (  rstn_sys= '0')    then
            conf_reg  <= (others =>(others =>'0'));
            
            AW_SK_Input_Reg     <= (others =>(others =>'0'));
            IOBP_sK_Aktiv_LED_i <= (others =>(others =>'1'));
            IOBP_SK_Output      <= (others =>(others =>'0'));
            IOBP_SK_Sel_LED     <= (others =>(others =>'0'));
            IOBP_SK_Input       <= (others =>(others =>'0'));

      elsif rising_edge(clk_sys) then
      
            for I in 1 to 12 loop
               conf_reg(I) <= IOBP_ID(I);
            end loop;                    

            --set default input values 
            AW_SK_Input_Reg     <= (others =>(others =>'0'));
            IOBP_sK_Aktiv_LED_i <= (others =>(others =>'1'));
            IOBP_SK_Output      <= (others =>(others =>'0'));
            IOBP_SK_Sel_LED     <= (others =>(others =>'0'));
            IOBP_SK_Input       <= (others =>(others =>'0'));
            
--    constant SEL_FG902_130_ID  :  std_logic_vector(7 downto 0):="00000011"; -- 6 LEMO Input Modul FG902.130
--    constant SEL_FG902_150_ID  :  std_logic_vector(7 downto 0):="00000111"; -- 6 LEMO Input Modul FG902150 in slot x
--    constant SEL_FG902_LWL_ID  :  std_logic_vector(7 downto 0):="00000100"; -- 6 LWL Input Modul in slot x
--    
--    constant SEL_OUTMOD1_ID  :  std_logic_vector(7 downto 0):="00000101";   -- Output Modul
--    constant SEL_OUTMOD2_ID  :  std_logic_vector(7 downto 0):="00000110";   -- Output Modul
--     
--    constant SEL_OUTMOD3_ID  :  std_logic_vector(7 downto 0):="00000001";   -- 5 In/1 Out Modul in slot x
--    constant SEL_OUTMOD4_ID  :  std_logic_vector(7 downto 0):="00000010";   -- 5 In/1 Out Modul in slot x
--    
            
--       when IOBP_slot1=>            conf_reg(1)<= IOBP_ID(1);
             INREG((56,62,54,60,52,58),SEL_FG902_130_ID,0);                                                        
             INREG((56,62,54,60,52,58),SEL_FG902_150_ID,0);                                                                                                                  
             INREG((56,60,62,52,54,58),SEL_FG902_LWL_ID,0);                                                                                                                      

--              when IOBP_slot2=>            conf_reg(2)<= IOBP_ID(2);
             INREG((96,102,94,100,92,98),SEL_FG902_130_ID,1);                                                        
             INREG((96,102,94,100,92,98),SEL_FG902_150_ID,1);                                                                                                                           
             INREG((96,100,102,92,94,98),SEL_FG902_LWL_ID,1);
             
--              when IOBP_slot3=>            conf_reg(3)<= IOBP_ID(3);
             INREG((73,79,71,77,69,75),SEL_FG902_130_ID,2);
             INREG((73,79,71,77,69,75),SEL_FG902_150_ID,2);                                                                                                                  
             INREG((73,77,79,69,71,75),SEL_FG902_LWL_ID,2);                                                                                                                      
                          
--               when IOBP_slot4=>           conf_reg(4)<= IOBP_ID(4);
             INREG((101,93,102,91,105,89),SEL_FG902_130_ID,3);                                                        
             INREG((101,93,102,91,105,89),SEL_FG902_150_ID,3);                                                                                                                  
             INREG((101,91,93,105,103,89),SEL_FG902_LWL_ID,3);                                                                                                                      
               
--              when IOBP_slot5=>            conf_reg(5)<= IOBP_ID(5);
             INREG((53,63,55,61,57,59),SEL_FG902_130_ID,4);                                                        
             INREG((53,63,55,61,57,59),SEL_FG902_150_ID,4);                                                                                                  
             INREG((53,61,63,57,55,59),SEL_FG902_LWL_ID,4);

                                                
--OUTPUT / FBAS-Trigger                                       
--            conf_reg(6) <= IOBP_ID(6);       IOBP_Masken_Reg
          case conf_reg(6) is

              when SEL_OUTMOD1_ID  | SEL_OUTMOD2_ID => -- Output Modul in slot 6
                  AW_SK_Input_Reg(5)<=   (OTHERS => '0');
                  --IOBP_SK_Output(6) <= AW_Output_Reg(3)(11 downto  6) AND not IOBP_Masken_Reg(2)(11 downto 6);
                  IOBP_SK_Output(6)(5  downto 0) <= triggerarr(0) AND not IOBP_Masken_Reg(2)(5  downto 0);
                  
                  PIO_OUT_SLOT(6) <= IOBP_SK_Output(6);
                  PIO_ENA_SLOT(6) <= std_logic_vector'("111111");
                  --IOBP_SK_Aktiv_LED_i(6)  <=  IOBP_SK_Output(6);
                  IOBP_SK_Aktiv_LED_i(6)  <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
                  IOBP_SK_Input(6)  <= (OTHERS => '0');
                  --IOBP_SK_Sel_LED(6)   <=  not ( IOBP_Masken_Reg(2)(11 downto 6) );  -- Register für Sel-LED's vom Slave 6
                  IOBP_SK_Sel_LED(6)   <= IOBP_SK_Output(6);

              when others     =>  NULL;
         end case;

    end if;
   end process ID_Front_Board_proc;



--  ###############################################################################################################################
--  #####                                                                                                                     #####
--  #####                             Input-Muliplexer zum SCU-Bus für den Mirror-Mode                                        #####
--  #####                                                                                                                     #####
--  ###############################################################################################################################

P_AW_SCU_In:  process (rstn_sys, clk_sys, Diob_Config1, Mirr_AWOut_Reg_Nr, SCU_AW_Output_Reg)

  begin
    if rstn_sys = '0' then

      SCU_AW_Input_Reg <= (others => (others => '0'));

    elsif rising_edge(clk_sys) then

      IF  (Diob_Config1(3) = '0')  THEN   -- 0 = Default: kein "Mirror-Mode"
        SCU_AW_Input_Reg  <= AW_Input_Reg; -- Input's bleiben unverändert
      ELSE

      --############################# Mirror-Mode ######################################

      Mirr_AWOut_Reg_Nr      <= to_integer(unsigned(Diob_Config1)( 7 downto 5));      -- Output-Reg. Nr. 1..7
      Mirr_AWIn_Reg_Nr       <= to_integer(unsigned(Diob_Config1)(10 downto 8));      -- Input-Reg. Nr. 1..7

      For REG_Nr in 1 to 7 loop

        IF REG_Nr = Mirr_AWIn_Reg_Nr THEN  -- Maskierte Bits vom Output-Register "Mirr_AWOut_Reg_Nr" --> Input_Register "Mirr_AWIn_Reg_Nr"

           FOR Bit_Nr in 0 to 15 loop
                if  (Mirr_OutReg_Maske(Bit_Nr)) = '1' then
                      SCU_AW_Input_Reg(REG_Nr)(Bit_Nr)   <= SCU_AW_Output_Reg (Mirr_AWOut_Reg_Nr)(Bit_Nr);   -- Copy Output-Bit --> Input-Bit
                else  SCU_AW_Input_Reg(REG_Nr)(Bit_Nr)   <= AW_Input_Reg(REG_Nr)           (Bit_Nr);   -- Input-Bit bleibt unverändert
                end if;
           end loop;

        ELSE
          FOR Bit_Nr in 0 to 15 loop
            SCU_AW_Input_Reg(REG_Nr)(Bit_Nr)   <= AW_Input_Reg(REG_Nr)(Bit_Nr);    -- Input-Bit bleibt unverändert
          end loop;
        END IF; -- Mirror-Mode
      end loop;

    END IF;
  END IF;

  end process P_AW_SCU_In;



--  ###############################################################################################################################
--  ###############################################################################################################################
--  #####                                                                                                                     #####
--  #####               PROCESS: Zuordnung der IO-Signale über den Stecker JPIO1(150pol.) ==> "Piggy-Type"                    #####
--  #####                                                                                                                     #####
--  ###############################################################################################################################
--  ###############################################################################################################################


p_AW_MUX: PROCESS (  clk_sys, rstn_sys, Powerup_Done, AW_ID, s_nLED_Out, signal_tap_clk_250mhz, A_SEL,
                     IOBP_SK_Input,clk_blink,

                     PIO_SYNC, PIO_SYNC1, PIO_ENA, PIO_ENA_SYNC, PIO_OUT, PIO_OUT_SYNC, PIO,
                     CLK_IO,
                     DIOB_Status1, DIOB_Status2, AW_Status1, AW_Status2,
                     AW_Input_Reg,
                     DIOB_Config1,    DIOB_Config2,    AW_Config1,    AW_Config2,
                     DIOB_Config1_wr, DIOB_Config2_wr, AW_Config1_wr, AW_Config2_wr,

                     s_nLED_Sel, s_nLED_Dtack, s_nLED_inR, s_nLED_User1_o, s_nLED_User2_o, s_nLED_User3_o,

                     AW_Output_Reg,
                     IOBP_Output,
                     IOBP_SK_Output, IOBP_SK_Input, Deb72_out, Deb72_in, Syn72, AW_SK_Input_Reg,
                     IOBP_SK_Aktiv_LED_i
                  )


BEGIN

  --############################# Set Defaults ######################################

    PIO_OUT(150 downto 16)      <=  (OTHERS => '0');   -- setze alle Outputs auf '0';
    PIO_ENA(150 downto 16)      <=  (OTHERS => '0');   -- Disable alle Outputs;

    UIO_OUT(15 downto 0)        <=  (OTHERS => '0');   -- setze alle Outputs auf '0';
    UIO_ENA(15 downto 0)        <=  (OTHERS => '0');   -- Disable alle Outputs;

    AW_Input_Reg                <=  (OTHERS => (OTHERS => '0'));  -- AW_Input_Reg's = 0

    AW_ID(7 downto 0)           <=  x"FF";    -- Anwender-Karten ID

    extension_cid_system        <= 0;   -- extension card: cid_system
    extension_cid_group         <= 0;   -- extension card: cid_group

    Max_AWOut_Reg_Nr            <= 0;    -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr             <= 0;    -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn_Deb_Time               <= 0;    -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time", Wert aus DIOB-Config 1
    Min_AWIn_Deb_Time           <= 0;    -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us


    Diob_Status1(15 downto 6)   <= (OTHERS => '0');      -- Reserve
    Diob_Status1(5  downto 0)   <= (OTHERS => '0');--Tag_Sts(5 downto 0);  -- Tag-Ctrl Status
    Diob_Status2(15 downto 8)   <= (OTHERS => '0');      -- Reserve
    Diob_Status2( 7 downto 0)   <= (OTHERS => '0');--Tag_Aktiv;            -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)

    AW_Status1                  <= (OTHERS => '0');      -- Input-Port-AW_Sts1
    AW_Status2                  <= (OTHERS => '0');      -- Input-Port-AW_Sts2

    A_Tclk                      <= '0';  -- Clock  für HP-Logic-Analysator

    s_nLED_User1_i              <= '0';        -- LED3 = User 1, -- frei --
    s_nLED_User2_i              <= '0';        -- LED3 = User 2, -- frei --
    s_nLED_User3_i              <= '0';        -- LED3 = User 3, -- frei --

    Debounce_cnt                <= 0;
    Signal60_in                 <= (OTHERS => '0');
    Syn60                       <= (OTHERS => '0');
    Deb_Sync60                  <= (OTHERS => '0');

    IOBP_Aktiv_LED_i            <=  (OTHERS => (OTHERS => '0'));    -- Inputs für die Aktiv-LED-Monoflops der "Slave-Karte 1-12"
    IOBP_Input                  <=  (OTHERS => (OTHERS => '0'));    -- Data_Input  "Slave-Karte 1-12"
    IOBP_Output                 <=             (OTHERS => '0');     -- Data_Output "Slave-Karte 1-12"
    IOBP_Sel_LED                <=  (OTHERS => (OTHERS => '0'));    -- Inputs für die Aktiv-LED-Monoflops der "Slave-Karte 1-12"
    IOBP_LED_ID_Bus_i           <=             (OTHERS => '1');     -- Data_Output "Slave-Karte 1-12"

    IOBP_ID_Reg                 <=  (OTHERS => (OTHERS => '0'));    -- IO-Backplane_ID_Register
--    IOBP_Id_Reg2                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
--    IOBP_Id_Reg3                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
--    IOBP_Id_Reg4                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
--    IOBP_Id_Reg5                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
--    IOBP_Id_Reg6                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register


    IOBP_Output_Readback        <=  (OTHERS => (OTHERS => '0'));  -- IO-Backplane_ID_Register Readback
    Deb72_in                    <=  (OTHERS => (OTHERS => '0'));
    Syn72                       <=  (OTHERS => (OTHERS => '0'));
    Deb_Sync72                  <=  (OTHERS => (OTHERS => '0'));
    --#################################################################################
    --###                                                                           ###
    --###                    IO-Stecker-Test mit "BrückenStecker                    ###
    --###                                                                           ###
    --############################WHEN   c_AW_INLB12S.ID  =>


    --###################################################################################
    --###################################################################################
    --####                                                                            ###
    --####      Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen     ###
    --####                                                                            ###
    --###################################################################################
    --###################################################################################


    extension_cid_system <= c_cid_system;       -- extension card: CSCOHW
    extension_cid_group  <= c_AW_INLB12S.CID;   -- extension card: cid_group, "FG902_050"

    AW_Status1(15 downto 0)  <=  (OTHERS => '0');                  -- Unbenutzte Status-Bits
     AW_Status2(15 downto 0)  <=  (OTHERS => '0');                 -- Unbenutzte Status-Bits

    Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    --############################# Set Debounce- oder Syn-Time ######################################

      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1

      IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN 
         Debounce_cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
      ELSE 
         Debounce_cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;

    --################################### Set LED's ########################################

    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei --
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei --
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei --

    --========================== Output Register 1 ======================================

    PIO_OUT(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA)
    PIO_ENA(86)   <=  '1';                -- Output Enable

    ---------------------------------------------------------------------------------------------------------------------------------------

    (PIO_OUT(112), PIO_OUT(48), PIO_OUT(130), PIO_OUT(30), PIO_OUT(137), PIO_OUT(35),
     PIO_OUT(119), PIO_OUT(53), PIO_OUT(101), PIO_OUT(73), PIO_OUT(96),  PIO_OUT(56))   <= IOBP_Output(12 downto 1); ----------- Data_Output-Pin's der "Slave-Karten 12-1"

    (PIO_ENA(112), PIO_ENA(48), PIO_ENA(130), PIO_ENA(30), PIO_ENA(137), PIO_ENA(35),
     PIO_ENA(119), PIO_ENA(53), PIO_ENA(101), PIO_ENA(73), PIO_ENA(96),  PIO_ENA(56))   <= std_logic_vector'("111111111111");   -- Output Enable


    IOBP_Input(1)(5 downto 1)  <= ( PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58)  );  -- Input's 5-1 von den der Slave-Karte-1,
    IOBP_Input(2)(5 downto 1)  <= ( PIO_SYNC(102), PIO_SYNC(94),  PIO_SYNC(100), PIO_SYNC(92),  PIO_SYNC(98)  );  -- Input's 5-1 von den der Slave-Karte-2,
    IOBP_Input(3)(5 downto 1)  <= ( PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75)  );  -- Input's 5-1 von den der Slave-Karte-3,
    IOBP_Input(4)(5 downto 1)  <= ( PIO_SYNC(93),  PIO_SYNC(103), PIO_SYNC(91),  PIO_SYNC(105), PIO_SYNC(89)  );  -- Input's 5-1 von den der Slave-Karte-4,
    IOBP_Input(5)(5 downto 1)  <= ( PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59)  );  -- Input's 5-1 von den der Slave-Karte-5,
    IOBP_Input(6)(5 downto 1)  <= ( PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107) );  -- Input's 5-1 von den der Slave-Karte-6,
    IOBP_Input(7)(5 downto 1)  <= ( PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41)  );  -- Input's 5-1 von den der Slave-Karte-7,
    IOBP_Input(8)(5 downto 1)  <= ( PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125) );  -- Input's 5-1 von den der Slave-Karte-8,
    IOBP_Input(9)(5 downto 1)  <= ( PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24)  );  -- Input's 5-1 von den der Slave-Karte-9,
    IOBP_Input(10)(5 downto 1) <= ( PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142) );  -- Input's 5-1 von den der Slave-Karte-10,
    IOBP_Input(11)(5 downto 1) <= ( PIO_SYNC(38),  PIO_SYNC(46),  PIO_SYNC(40),  PIO_SYNC(44),  PIO_SYNC(42)  );  -- Input's 5-1 von den der Slave-Karte-11,
    IOBP_Input(12)(5 downto 1) <= ( PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124) );  -- Input's 5-1 von den der Slave-Karte-12,


    (PIO_OUT(114), PIO_OUT(50), PIO_OUT(132), PIO_OUT(32), PIO_OUT(135), PIO_OUT(33),
     PIO_OUT(117), PIO_OUT(51), PIO_OUT(99),  PIO_OUT(83), PIO_OUT(106), PIO_OUT(66))  <=  IOBP_STR_rot_o;   -- LED-Strobe Rot  für Slave 12-1
    (PIO_ENA(114), PIO_ENA(50), PIO_ENA(132), PIO_ENA(32), PIO_ENA(135), PIO_ENA(33),
     PIO_ENA(117), PIO_ENA(51), PIO_ENA(99),  PIO_ENA(83), PIO_ENA(106), PIO_ENA(66))  <=  std_logic_vector'("111111111111");   -- Output Enable

    (PIO_OUT(116), PIO_OUT(34), PIO_OUT(134), PIO_OUT(16), PIO_OUT(133), PIO_OUT(49),
     PIO_OUT(115), PIO_OUT(67), PIO_OUT(97),  PIO_OUT(81), PIO_OUT(104), PIO_OUT(64))  <=  IOBP_STR_gruen_o; -- LED-Strobe Grün für Slave 12-1
    (PIO_ENA(116), PIO_ENA(34), PIO_ENA(134), PIO_ENA(16), PIO_ENA(133), PIO_ENA(49),
     PIO_ENA(115), PIO_ENA(67), PIO_ENA(97),  PIO_ENA(81), PIO_ENA(104), PIO_ENA(64))  <=  std_logic_vector'("111111111111");   -- Output Enable

    (PIO_OUT(118), PIO_OUT(36), PIO_OUT(136), PIO_OUT(18), PIO_OUT(131), PIO_OUT(47),
     PIO_OUT(113), PIO_OUT(65), PIO_OUT(95),  PIO_OUT(85), PIO_OUT(90),  PIO_OUT(68))  <=  not IOBP_STR_ID_o;    -- ID-Strobe für Slave 12-1 (Enable ist L-Aktiv)
    (PIO_ENA(118), PIO_ENA(36), PIO_ENA(136), PIO_ENA(18), PIO_ENA(131), PIO_ENA(47),
     PIO_ENA(113), PIO_ENA(65), PIO_ENA(95),  PIO_ENA(85), PIO_ENA(90),  PIO_ENA(68))  <=  std_logic_vector'("111111111111");   -- Output Enable



    --###################################################################################
    --###################################################################################
    --####                                                                            ###
    --####      Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen     ###
    --####                                                                            ###
    --###################################################################################
    --###################################################################################


    extension_cid_system <= c_cid_system;       -- extension card: CSCOHW
    extension_cid_group  <= c_AW_INLB12S.CID;   -- extension card: cid_group, "FG902_050"

    AW_Status1(15 downto 0)  <=  (OTHERS => '0');                  -- Unbenutzte Status-Bits
     AW_Status2(15 downto 0)  <=  (OTHERS => '0');                 -- Unbenutzte Status-Bits

    Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    --############################# Set Debounce- oder Syn-Time ######################################

      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1

      IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN Debounce_cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
                                             ELSE Debounce_cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;

    --################################### Set LED's ########################################

    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei --
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei --
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei --


    --========================== Output Register 1 ======================================

    PIO_OUT(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA)
    PIO_ENA(86)   <=  '1';                -- Output Enable


    ---------------------------------------------------------------------------------------------------------------------------------------


    (PIO_OUT(112), PIO_OUT(48), PIO_OUT(130), PIO_OUT(30), PIO_OUT(137), PIO_OUT(35),
     PIO_OUT(119), PIO_OUT(53), PIO_OUT(101), PIO_OUT(73), PIO_OUT(96),  PIO_OUT(56))   <= IOBP_Output(12 downto 1); ----------- Data_Output-Pin's der "Slave-Karten 12-1"

    (PIO_ENA(112), PIO_ENA(48), PIO_ENA(130), PIO_ENA(30), PIO_ENA(137), PIO_ENA(35),
     PIO_ENA(119), PIO_ENA(53), PIO_ENA(101), PIO_ENA(73), PIO_ENA(96),  PIO_ENA(56))   <= std_logic_vector'("111111111111");   -- Output Enable


    IOBP_Input(1)(5 downto 1)  <= ( PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58)  );  -- Input's 5-1 von den der Slave-Karte-1,
    IOBP_Input(2)(5 downto 1)  <= ( PIO_SYNC(102), PIO_SYNC(94),  PIO_SYNC(100), PIO_SYNC(92),  PIO_SYNC(98)  );  -- Input's 5-1 von den der Slave-Karte-2,
    IOBP_Input(3)(5 downto 1)  <= ( PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75)  );  -- Input's 5-1 von den der Slave-Karte-3,
    IOBP_Input(4)(5 downto 1)  <= ( PIO_SYNC(93),  PIO_SYNC(103), PIO_SYNC(91),  PIO_SYNC(105), PIO_SYNC(89)  );  -- Input's 5-1 von den der Slave-Karte-4,
    IOBP_Input(5)(5 downto 1)  <= ( PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59)  );  -- Input's 5-1 von den der Slave-Karte-5,
    IOBP_Input(6)(5 downto 1)  <= ( PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107) );  -- Input's 5-1 von den der Slave-Karte-6,
    IOBP_Input(7)(5 downto 1)  <= ( PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41)  );  -- Input's 5-1 von den der Slave-Karte-7,
    IOBP_Input(8)(5 downto 1)  <= ( PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125) );  -- Input's 5-1 von den der Slave-Karte-8,
    IOBP_Input(9)(5 downto 1)  <= ( PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24)  );  -- Input's 5-1 von den der Slave-Karte-9,
    IOBP_Input(10)(5 downto 1) <= ( PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142) );  -- Input's 5-1 von den der Slave-Karte-10,
    IOBP_Input(11)(5 downto 1) <= ( PIO_SYNC(38),  PIO_SYNC(46),  PIO_SYNC(40),  PIO_SYNC(44),  PIO_SYNC(42)  );  -- Input's 5-1 von den der Slave-Karte-11,
    IOBP_Input(12)(5 downto 1) <= ( PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124) );  -- Input's 5-1 von den der Slave-Karte-12,


    (PIO_OUT(114), PIO_OUT(50), PIO_OUT(132), PIO_OUT(32), PIO_OUT(135), PIO_OUT(33),
     PIO_OUT(117), PIO_OUT(51), PIO_OUT(99),  PIO_OUT(83), PIO_OUT(106), PIO_OUT(66))  <=  IOBP_STR_rot_o;   -- LED-Strobe Rot  für Slave 12-1
    (PIO_ENA(114), PIO_ENA(50), PIO_ENA(132), PIO_ENA(32), PIO_ENA(135), PIO_ENA(33),
     PIO_ENA(117), PIO_ENA(51), PIO_ENA(99),  PIO_ENA(83), PIO_ENA(106), PIO_ENA(66))  <=  std_logic_vector'("111111111111");   -- Output Enable

    (PIO_OUT(116), PIO_OUT(34), PIO_OUT(134), PIO_OUT(16), PIO_OUT(133), PIO_OUT(49),
     PIO_OUT(115), PIO_OUT(67), PIO_OUT(97),  PIO_OUT(81), PIO_OUT(104), PIO_OUT(64))  <=  IOBP_STR_gruen_o; -- LED-Strobe Grün für Slave 12-1
    (PIO_ENA(116), PIO_ENA(34), PIO_ENA(134), PIO_ENA(16), PIO_ENA(133), PIO_ENA(49),
     PIO_ENA(115), PIO_ENA(67), PIO_ENA(97),  PIO_ENA(81), PIO_ENA(104), PIO_ENA(64))  <=  std_logic_vector'("111111111111");   -- Output Enable

    (PIO_OUT(118), PIO_OUT(36), PIO_OUT(136), PIO_OUT(18), PIO_OUT(131), PIO_OUT(47),
     PIO_OUT(113), PIO_OUT(65), PIO_OUT(95),  PIO_OUT(85), PIO_OUT(90),  PIO_OUT(68))  <=  not IOBP_STR_ID_o;    -- ID-Strobe für Slave 12-1 (Enable ist L-Aktiv)
    (PIO_ENA(118), PIO_ENA(36), PIO_ENA(136), PIO_ENA(18), PIO_ENA(131), PIO_ENA(47),
     PIO_ENA(113), PIO_ENA(65), PIO_ENA(95),  PIO_ENA(85), PIO_ENA(90),  PIO_ENA(68))  <=  std_logic_vector'("111111111111");   -- Output Enable


-------------------- Input/Output vom LED_ID_Bus der Zwischenbackplane  ------------

     IOBP_LED_ID_Bus_i <= (PIO_Sync(70), PIO_Sync(72), PIO_Sync(74), PIO_Sync(76), PIO_Sync(78), PIO_Sync(80), PIO_Sync(82), PIO_Sync(84));   ------------------------- Input  LED_ID_Bus
                          (PIO_OUT(70),  PIO_OUT(72),  PIO_OUT(74),  PIO_OUT(76),  PIO_OUT(78),  PIO_OUT(80),  PIO_OUT(82),  PIO_OUT(84))   <=  IOBP_LED_ID_Bus_o;   -- Output LED_ID_Bus


-------------------- Tri-State Steuerung vom LED_ID_Bus der Zwischenbackplane  ------------

    IF IOBP_LED_En = '1' THEN ---------------- LED write Loop
      (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("11111111");  -- Output Enable
    ELSE --------------------------------------ID read Loop
      (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("00000000");  -- Output Disable
    END IF;


-----------------------------------------------------------------------------------------------------------------------------------------


---------------- Output-Register(Maske) für die Iput- und Output Sel-LED's vom Slave 1-12
--
--  IOBP_Sel_LED's sind Low-Aktiv             Bit5 = Output          Bit[4..0]  =  Input[5..1]
--        |                                         |                           |
    IOBP_Sel_LED(1) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 0) & IOBP_Masken_Reg(0)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 1
    IOBP_Sel_LED(2) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 1) & IOBP_Masken_Reg(0)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 2
    IOBP_Sel_LED(3) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 2) & IOBP_Masken_Reg(0)(14 downto 10) );  -- Register für Sel-LED's vom Slave 3
    IOBP_Sel_LED(4) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 3) & IOBP_Masken_Reg(1)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 4
    IOBP_Sel_LED(5) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 4) & IOBP_Masken_Reg(1)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 5
    IOBP_Sel_LED(6) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 5) & IOBP_Masken_Reg(1)(14 downto 10) );  -- Register für Sel-LED's vom Slave 6
    IOBP_Sel_LED(7) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 6) & IOBP_Masken_Reg(2)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 7
    IOBP_Sel_LED(8) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 7) & IOBP_Masken_Reg(2)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 8
    IOBP_Sel_LED(9) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 8) & IOBP_Masken_Reg(2)(14 downto 10) );  -- Register für Sel-LED's vom Slave 9
    IOBP_Sel_LED(10)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 9) & IOBP_Masken_Reg(3)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 10
    IOBP_Sel_LED(11)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(10) & IOBP_Masken_Reg(3)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 11
    IOBP_Sel_LED(12)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(11) & IOBP_Masken_Reg(3)(14 downto 10) );  -- Register für Sel-LED's vom Slave 12


---------------------------------------------------------------------------------------------------------------------------------------

--                    ID-Input-Register für die IO-Module Nr. 1+12

    IOBP_Id_Reg(05)(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
    IOBP_Id_Reg(5)( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
    IOBP_Id_Reg(4)(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
    IOBP_Id_Reg(4)( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
    IOBP_Id_Reg(3)(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
    IOBP_Id_Reg(3)( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
    IOBP_Id_Reg(2)(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
    IOBP_Id_Reg(2)( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
    IOBP_Id_Reg(1)(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
    IOBP_Id_Reg(1)( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
    IOBP_Id_Reg(0)(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
    IOBP_Id_Reg(0)( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1


---------------------------------------------------------------------------------------------------------------------------------------

--################################ Debounce oder Sync Input's  ##################################

--  Signal60_in = H-Aktiv             IOBP_Input = L-Aktiv
--        |                                |
    Signal60_in( 4 DOWNTO  0)   <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Signal60_in( 9 DOWNTO  5)   <=  not IOBP_Input( 2)(5 downto 1);
    Signal60_in(14 DOWNTO 10)   <=  not IOBP_Input( 3)(5 downto 1);
    Signal60_in(19 DOWNTO 15)   <=  not IOBP_Input( 4)(5 downto 1);
    Signal60_in(24 DOWNTO 20)   <=  not IOBP_Input( 5)(5 downto 1);
    Signal60_in(29 DOWNTO 25)   <=  not IOBP_Input( 6)(5 downto 1);
    Signal60_in(34 DOWNTO 30)   <=  not IOBP_Input( 7)(5 downto 1);
    Signal60_in(39 DOWNTO 35)   <=  not IOBP_Input( 8)(5 downto 1);
    Signal60_in(44 DOWNTO 40)   <=  not IOBP_Input( 9)(5 downto 1);
    Signal60_in(49 DOWNTO 45)   <=  not IOBP_Input(10)(5 downto 1);
    Signal60_in(54 DOWNTO 50)   <=  not IOBP_Input(11)(5 downto 1);
    Signal60_in(59 DOWNTO 55)   <=  not IOBP_Input(12)(5 downto 1);


--  Syn60 = H-Aktiv             IOBP_Input = L-Aktiv
--                                      |
    Syn60( 4 DOWNTO  0)      <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Syn60( 9 DOWNTO  5)      <=  not IOBP_Input( 2)(5 downto 1);
    Syn60(14 DOWNTO 10)      <=  not IOBP_Input( 3)(5 downto 1);
    Syn60(19 DOWNTO 15)      <=  not IOBP_Input( 4)(5 downto 1);
    Syn60(24 DOWNTO 20)      <=  not IOBP_Input( 5)(5 downto 1);
    Syn60(29 DOWNTO 25)      <=  not IOBP_Input( 6)(5 downto 1);
    Syn60(34 DOWNTO 30)      <=  not IOBP_Input( 7)(5 downto 1);
    Syn60(39 DOWNTO 35)      <=  not IOBP_Input( 8)(5 downto 1);
    Syn60(44 DOWNTO 40)      <=  not IOBP_Input( 9)(5 downto 1);
    Syn60(49 DOWNTO 45)      <=  not IOBP_Input(10)(5 downto 1);
    Syn60(54 DOWNTO 50)      <=  not IOBP_Input(11)(5 downto 1);
    Syn60(59 DOWNTO 55)      <=  not IOBP_Input(12)(5 downto 1);


    IF  (Diob_Config1(11) = '1')  THEN 
      Deb_Sync60 <=  Syn60;         -- Dobounce = Abgeschaltet ==> nur Synchronisation
    ELSE 
      Deb_Sync60 <=  Deb60_out;     -- Debounce und Synchronisation
    END IF;


--  ################################ Input's AND Maske zu Input-Register ###################

--                  Input-Test, Stecker 1, 2, 3

    AW_Input_Reg(1)( 4 downto  0) <=   (Deb_Sync60( 4 downto  0) AND not IOBP_Masken_Reg(0)( 4 downto  0));  -- Input, IO-Modul Nr. 1
    AW_Input_Reg(1)( 9 downto  5) <=   (Deb_Sync60( 9 downto  5) AND not IOBP_Masken_Reg(0)( 9 downto  5));  -- Input, IO-Modul Nr. 2
    AW_Input_Reg(1)(14 downto 10) <=   (Deb_Sync60(14 downto 10) AND not IOBP_Masken_Reg(0)(14 downto 10));  -- Input, IO-Modul Nr. 3
    AW_Input_Reg(1)(15) <=  '0';

--                  Input-Test, Stecker 4, 5, 6
    AW_Input_Reg(2)( 4 downto  0) <=   (Deb_Sync60(19 DOWNTO 15) AND not IOBP_Masken_Reg(1)( 4 downto  0));  -- Input, IO-Modul Nr. 4
    AW_Input_Reg(2)( 9 downto  5) <=   (Deb_Sync60(24 DOWNTO 20) AND not IOBP_Masken_Reg(1)( 9 downto  5));  -- Input, IO-Modul Nr. 5
    AW_Input_Reg(2)(14 downto 10) <=   (Deb_Sync60(29 DOWNTO 25) AND not IOBP_Masken_Reg(1)(14 downto 10));  -- Input, IO-Modul Nr. 6
    AW_Input_Reg(2)(15) <=  '0';

--                  Input-Test, Stecker 7, 8, 9

    AW_Input_Reg(3)( 4 downto  0) <=   (Deb_Sync60(34 DOWNTO 30) AND not IOBP_Masken_Reg(2)( 4 downto  0));  -- Input, IO-Modul Nr. 7
    AW_Input_Reg(3)( 9 downto  5) <=   (Deb_Sync60(39 DOWNTO 35) AND not IOBP_Masken_Reg(2)( 9 downto  5));  -- Input, IO-Modul Nr. 8
    AW_Input_Reg(3)(14 downto 10) <=   (Deb_Sync60(44 DOWNTO 40) AND not IOBP_Masken_Reg(2)(14 downto 10));  -- Input, IO-Modul Nr. 9
    AW_Input_Reg(3)(15) <=  '0';

--                    Input-Test, Stecker 10, 11, 12

    AW_Input_Reg(4)( 4 downto  0) <=   (Deb_Sync60(49 DOWNTO 45) AND not IOBP_Masken_Reg(3)( 4 downto  0));  -- Input, IO-Modul Nr. 10
    AW_Input_Reg(4)( 9 downto  5) <=   (Deb_Sync60(54 DOWNTO 50) AND not IOBP_Masken_Reg(3)( 9 downto  5));  -- Input, IO-Modul Nr. 11
    AW_Input_Reg(4)(14 downto 10) <=   (Deb_Sync60(59 DOWNTO 55) AND not IOBP_Masken_Reg(3)(14 downto 10));  -- Input, IO-Modul Nr. 12
    AW_Input_Reg(4)(15) <=  '0';



--################################ Outputs AND Maske ##################################
--
   -- case AW_Config2 is

    --  STANDARD OUTPUT OUTREG
--                                                    MaskenBit=0 --> Enable
      IOBP_Output(1)  <= (AW_Output_Reg(1)( 0) AND not IOBP_Masken_Reg(4)( 0));  -- Output von Slave 1
      IOBP_Output(2)  <= (AW_Output_Reg(1)( 1) AND not IOBP_Masken_Reg(4)( 1));  -- Output von Slave 2
      IOBP_Output(3)  <= (AW_Output_Reg(1)( 2) AND not IOBP_Masken_Reg(4)( 2));  -- Output von Slave 3
      IOBP_Output(4)  <= (AW_Output_Reg(1)( 3) AND not IOBP_Masken_Reg(4)( 3));  -- Output von Slave 4
      IOBP_Output(5)  <= (AW_Output_Reg(1)( 4) AND not IOBP_Masken_Reg(4)( 4));  -- Output von Slave 5
      IOBP_Output(6)  <= (AW_Output_Reg(1)( 5) AND not IOBP_Masken_Reg(4)( 5));  -- Output von Slave 6
      IOBP_Output(7)  <= (AW_Output_Reg(1)( 6) AND not IOBP_Masken_Reg(4)( 6));  -- Output von Slave 7
      IOBP_Output(8)  <= (AW_Output_Reg(1)( 7) AND not IOBP_Masken_Reg(4)( 7));  -- Output von Slave 8
      IOBP_Output(9)  <= (AW_Output_Reg(1)( 8) AND not IOBP_Masken_Reg(4)( 8));  -- Output von Slave 9
      IOBP_Output(10) <= (AW_Output_Reg(1)( 9) AND not IOBP_Masken_Reg(4)( 9));  -- Output von Slave 10
      IOBP_Output(11) <= (AW_Output_Reg(1)(10) AND not IOBP_Masken_Reg(4)(10));  -- Output von Slave 11
      IOBP_Output(12) <= (AW_Output_Reg(1)(11) AND not IOBP_Masken_Reg(4)(11));  -- Output von Slave 12
    --end Case


--################################ Aktiv-Led's ##################################
--
--                          maskierte Outputs            entprellte Inputs
--                                   |
    IOBP_Aktiv_LED_i(1)  <=  (IOBP_Output(1)    &  Deb60_out( 4 DOWNTO  0));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(2)  <=  (IOBP_Output(2)    &  Deb60_out( 9 DOWNTO  5));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(3)  <=  (IOBP_Output(3)    &  Deb60_out(14 DOWNTO 10));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(4)  <=  (IOBP_Output(4)    &  Deb60_out(19 DOWNTO 15));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(5)  <=  (IOBP_Output(5)    &  Deb60_out(24 DOWNTO 20));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(6)  <=  (IOBP_Output(6)    &  Deb60_out(29 DOWNTO 25));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(7)  <=  (IOBP_Output(7)    &  Deb60_out(34 DOWNTO 30));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(8)  <=  (IOBP_Output(8)    &  Deb60_out(39 DOWNTO 35));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(9)  <=  (IOBP_Output(9)    &  Deb60_out(44 DOWNTO 40));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(10) <=  (IOBP_Output(10)   &  Deb60_out(49 DOWNTO 45));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(11) <=  (IOBP_Output(11)   &  Deb60_out(54 DOWNTO 50));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(12) <=  (IOBP_Output(12)   &  Deb60_out(59 DOWNTO 55));  -- Signale für Aktiv-LED's
    
  

-------------------- Input/Output vom LED_ID_Bus der Zwischenbackplane  ------------

     IOBP_LED_ID_Bus_i <= (PIO_Sync(70), PIO_Sync(72), PIO_Sync(74), PIO_Sync(76), PIO_Sync(78), PIO_Sync(80), PIO_Sync(82), PIO_Sync(84));   ------------------------- Input  LED_ID_Bus
                          (PIO_OUT(70),  PIO_OUT(72),  PIO_OUT(74),  PIO_OUT(76),  PIO_OUT(78),  PIO_OUT(80),  PIO_OUT(82),  PIO_OUT(84))   <=  IOBP_LED_ID_Bus_o;   -- Output LED_ID_Bus


-------------------- Tri-State Steuerung vom LED_ID_Bus der Zwischenbackplane  ------------

    IF IOBP_LED_En = '1' THEN ---------------- LED write Loop
      (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("11111111");  -- Output Enable
    ELSE --------------------------------------ID read Loop
      (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("00000000");  -- Output Disable
    END IF;


-----------------------------------------------------------------------------------------------------------------------------------------


---------------- Output-Register(Maske) für die Iput- und Output Sel-LED's vom Slave 1-12
--
--  IOBP_Sel_LED's sind Low-Aktiv             Bit5 = Output          Bit[4..0]  =  Input[5..1]
--        |                                         |                           |
    IOBP_Sel_LED(1) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 0) & IOBP_Masken_Reg(0)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 1
    IOBP_Sel_LED(2) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 1) & IOBP_Masken_Reg(0)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 2
    IOBP_Sel_LED(3) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 2) & IOBP_Masken_Reg(0)(14 downto 10) );  -- Register für Sel-LED's vom Slave 3
    IOBP_Sel_LED(4) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 3) & IOBP_Masken_Reg(1)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 4
    IOBP_Sel_LED(5) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 4) & IOBP_Masken_Reg(1)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 5
    IOBP_Sel_LED(6) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 5) & IOBP_Masken_Reg(1)(14 downto 10) );  -- Register für Sel-LED's vom Slave 6
    IOBP_Sel_LED(7) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 6) & IOBP_Masken_Reg(2)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 7
    IOBP_Sel_LED(8) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 7) & IOBP_Masken_Reg(2)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 8
    IOBP_Sel_LED(9) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 8) & IOBP_Masken_Reg(2)(14 downto 10) );  -- Register für Sel-LED's vom Slave 9
    IOBP_Sel_LED(10)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 9) & IOBP_Masken_Reg(3)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 10
    IOBP_Sel_LED(11)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(10) & IOBP_Masken_Reg(3)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 11
    IOBP_Sel_LED(12)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(11) & IOBP_Masken_Reg(3)(14 downto 10) );  -- Register für Sel-LED's vom Slave 12


---------------------------------------------------------------------------------------------------------------------------------------

--                    ID-Input-Register für die IO-Module Nr. 1+12

    IOBP_Id_Reg(5)(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
    IOBP_Id_Reg(5)( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
    IOBP_Id_Reg(4)(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
    IOBP_Id_Reg(4)( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
    IOBP_Id_Reg(3)(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
    IOBP_Id_Reg(3)( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
    IOBP_Id_Reg(2)(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
    IOBP_Id_Reg(2)( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
    IOBP_Id_Reg(1)(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
    IOBP_Id_Reg(1)( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
    IOBP_Id_Reg(0)(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
    IOBP_Id_Reg(0)( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1


---------------------------------------------------------------------------------------------------------------------------------------

--################################ Debounce oder Sync Input's  ##################################

--  Signal60_in = H-Aktiv             IOBP_Input = L-Aktiv
--        |                                |
    Signal60_in( 4 DOWNTO  0)   <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Signal60_in( 9 DOWNTO  5)   <=  not IOBP_Input( 2)(5 downto 1);
    Signal60_in(14 DOWNTO 10)   <=  not IOBP_Input( 3)(5 downto 1);
    Signal60_in(19 DOWNTO 15)   <=  not IOBP_Input( 4)(5 downto 1);
    Signal60_in(24 DOWNTO 20)   <=  not IOBP_Input( 5)(5 downto 1);
    Signal60_in(29 DOWNTO 25)   <=  not IOBP_Input( 6)(5 downto 1);
    Signal60_in(34 DOWNTO 30)   <=  not IOBP_Input( 7)(5 downto 1);
    Signal60_in(39 DOWNTO 35)   <=  not IOBP_Input( 8)(5 downto 1);
    Signal60_in(44 DOWNTO 40)   <=  not IOBP_Input( 9)(5 downto 1);
    Signal60_in(49 DOWNTO 45)   <=  not IOBP_Input(10)(5 downto 1);
    Signal60_in(54 DOWNTO 50)   <=  not IOBP_Input(11)(5 downto 1);
    Signal60_in(59 DOWNTO 55)   <=  not IOBP_Input(12)(5 downto 1);


--  Syn60 = H-Aktiv             IOBP_Input = L-Aktiv
--                                      |
    Syn60( 4 DOWNTO  0)      <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Syn60( 9 DOWNTO  5)      <=  not IOBP_Input( 2)(5 downto 1);
    Syn60(14 DOWNTO 10)      <=  not IOBP_Input( 3)(5 downto 1);
    Syn60(19 DOWNTO 15)      <=  not IOBP_Input( 4)(5 downto 1);
    Syn60(24 DOWNTO 20)      <=  not IOBP_Input( 5)(5 downto 1);
    Syn60(29 DOWNTO 25)      <=  not IOBP_Input( 6)(5 downto 1);
    Syn60(34 DOWNTO 30)      <=  not IOBP_Input( 7)(5 downto 1);
    Syn60(39 DOWNTO 35)      <=  not IOBP_Input( 8)(5 downto 1);
    Syn60(44 DOWNTO 40)      <=  not IOBP_Input( 9)(5 downto 1);
    Syn60(49 DOWNTO 45)      <=  not IOBP_Input(10)(5 downto 1);
    Syn60(54 DOWNTO 50)      <=  not IOBP_Input(11)(5 downto 1);
    Syn60(59 DOWNTO 55)      <=  not IOBP_Input(12)(5 downto 1);


    IF  (Diob_Config1(11) = '1')  THEN 
      Deb_Sync60 <=  Syn60;         -- Dobounce = Abgeschaltet ==> nur Synchronisation
    ELSE 
      Deb_Sync60 <=  Deb60_out;     -- Debounce und Synchronisation
    END IF;


--  ################################ Input's AND Maske zu Input-Register ###################

--                  Input-Test, Stecker 1, 2, 3

    AW_Input_Reg(1)( 4 downto  0) <=   (Deb_Sync60( 4 downto  0) AND not IOBP_Masken_Reg(0)( 4 downto  0));  -- Input, IO-Modul Nr. 1
    AW_Input_Reg(1)( 9 downto  5) <=   (Deb_Sync60( 9 downto  5) AND not IOBP_Masken_Reg(0)( 9 downto  5));  -- Input, IO-Modul Nr. 2
    AW_Input_Reg(1)(14 downto 10) <=   (Deb_Sync60(14 downto 10) AND not IOBP_Masken_Reg(0)(14 downto 10));  -- Input, IO-Modul Nr. 3
    AW_Input_Reg(1)(15) <=  '0';

--                  Input-Test, Stecker 4, 5, 6
    AW_Input_Reg(2)( 4 downto  0) <=   (Deb_Sync60(19 DOWNTO 15) AND not IOBP_Masken_Reg(1)( 4 downto  0));  -- Input, IO-Modul Nr. 4
    AW_Input_Reg(2)( 9 downto  5) <=   (Deb_Sync60(24 DOWNTO 20) AND not IOBP_Masken_Reg(1)( 9 downto  5));  -- Input, IO-Modul Nr. 5
    AW_Input_Reg(2)(14 downto 10) <=   (Deb_Sync60(29 DOWNTO 25) AND not IOBP_Masken_Reg(1)(14 downto 10));  -- Input, IO-Modul Nr. 6
    AW_Input_Reg(2)(15) <=  '0';

--                  Input-Test, Stecker 7, 8, 9

    AW_Input_Reg(3)( 4 downto  0) <=   (Deb_Sync60(34 DOWNTO 30) AND not IOBP_Masken_Reg(2)( 4 downto  0));  -- Input, IO-Modul Nr. 7
    AW_Input_Reg(3)( 9 downto  5) <=   (Deb_Sync60(39 DOWNTO 35) AND not IOBP_Masken_Reg(2)( 9 downto  5));  -- Input, IO-Modul Nr. 8
    AW_Input_Reg(3)(14 downto 10) <=   (Deb_Sync60(44 DOWNTO 40) AND not IOBP_Masken_Reg(2)(14 downto 10));  -- Input, IO-Modul Nr. 9
    AW_Input_Reg(3)(15) <=  '0';

--                    Input-Test, Stecker 10, 11, 12

    AW_Input_Reg(4)( 4 downto  0) <=   (Deb_Sync60(49 DOWNTO 45) AND not IOBP_Masken_Reg(3)( 4 downto  0));  -- Input, IO-Modul Nr. 10
    AW_Input_Reg(4)( 9 downto  5) <=   (Deb_Sync60(54 DOWNTO 50) AND not IOBP_Masken_Reg(3)( 9 downto  5));  -- Input, IO-Modul Nr. 11
    AW_Input_Reg(4)(14 downto 10) <=   (Deb_Sync60(59 DOWNTO 55) AND not IOBP_Masken_Reg(3)(14 downto 10));  -- Input, IO-Modul Nr. 12
    AW_Input_Reg(4)(15) <=  '0';



--################################ Outputs AND Maske ##################################
--
   -- case AW_Config2 is

    --  STANDARD OUTPUT OUTREG
--                                                    MaskenBit=0 --> Enable
      IOBP_Output(1)  <= (AW_Output_Reg(1)( 0) AND not IOBP_Masken_Reg(4)( 0));  -- Output von Slave 1
      IOBP_Output(2)  <= (AW_Output_Reg(1)( 1) AND not IOBP_Masken_Reg(4)( 1));  -- Output von Slave 2
      IOBP_Output(3)  <= (AW_Output_Reg(1)( 2) AND not IOBP_Masken_Reg(4)( 2));  -- Output von Slave 3
      IOBP_Output(4)  <= (AW_Output_Reg(1)( 3) AND not IOBP_Masken_Reg(4)( 3));  -- Output von Slave 4
      IOBP_Output(5)  <= (AW_Output_Reg(1)( 4) AND not IOBP_Masken_Reg(4)( 4));  -- Output von Slave 5
      IOBP_Output(6)  <= (AW_Output_Reg(1)( 5) AND not IOBP_Masken_Reg(4)( 5));  -- Output von Slave 6
      IOBP_Output(7)  <= (AW_Output_Reg(1)( 6) AND not IOBP_Masken_Reg(4)( 6));  -- Output von Slave 7
      IOBP_Output(8)  <= (AW_Output_Reg(1)( 7) AND not IOBP_Masken_Reg(4)( 7));  -- Output von Slave 8
      IOBP_Output(9)  <= (AW_Output_Reg(1)( 8) AND not IOBP_Masken_Reg(4)( 8));  -- Output von Slave 9
      IOBP_Output(10) <= (AW_Output_Reg(1)( 9) AND not IOBP_Masken_Reg(4)( 9));  -- Output von Slave 10
      IOBP_Output(11) <= (AW_Output_Reg(1)(10) AND not IOBP_Masken_Reg(4)(10));  -- Output von Slave 11
      IOBP_Output(12) <= (AW_Output_Reg(1)(11) AND not IOBP_Masken_Reg(4)(11));  -- Output von Slave 12
    --end Case


--################################ Aktiv-Led's ##################################
--
--                          maskierte Outputs            entprellte Inputs
--                                   |
    IOBP_Aktiv_LED_i(1)  <=  (IOBP_Output(1)    &  Deb60_out( 4 DOWNTO  0));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(2)  <=  (IOBP_Output(2)    &  Deb60_out( 9 DOWNTO  5));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(3)  <=  (IOBP_Output(3)    &  Deb60_out(14 DOWNTO 10));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(4)  <=  (IOBP_Output(4)    &  Deb60_out(19 DOWNTO 15));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(5)  <=  (IOBP_Output(5)    &  Deb60_out(24 DOWNTO 20));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(6)  <=  (IOBP_Output(6)    &  Deb60_out(29 DOWNTO 25));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(7)  <=  (IOBP_Output(7)    &  Deb60_out(34 DOWNTO 30));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(8)  <=  (IOBP_Output(8)    &  Deb60_out(39 DOWNTO 35));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(9)  <=  (IOBP_Output(9)    &  Deb60_out(44 DOWNTO 40));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(10) <=  (IOBP_Output(10)   &  Deb60_out(49 DOWNTO 45));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(11) <=  (IOBP_Output(11)   &  Deb60_out(54 DOWNTO 50));  -- Signale für Aktiv-LED's
    IOBP_Aktiv_LED_i(12) <=  (IOBP_Output(12)   &  Deb60_out(59 DOWNTO 55));  -- Signale für Aktiv-LED's
---#####################################################


   IF  DIOB_Config1(15) = '1'  THEN   -- Config-Reg Bit15 = 1  --> Testmode


    --- Test der PIO-Pins ---

     AW_Input_Reg(1)(15 downto 0)  <=  ( CLK_IO,        PIO_SYNC(16),  PIO_SYNC(17),  PIO_SYNC(18),
                                         PIO_SYNC(19),  PIO_SYNC(20),  PIO_SYNC(21),  PIO_SYNC(22),
                                         PIO_SYNC(23),  PIO_SYNC(24),  PIO_SYNC(25),  PIO_SYNC(26),
                                         PIO_SYNC(27),  PIO_SYNC(28),  PIO_SYNC(29),  PIO_SYNC(30) );

          ( PIO_OUT(61),  PIO_OUT(62),  PIO_OUT(59),  PIO_OUT(60),
            PIO_OUT(57),  PIO_OUT(58),  PIO_OUT(55),  PIO_OUT(56),
            PIO_OUT(53),  PIO_OUT(54),  PIO_OUT(51),  PIO_OUT(52),
            PIO_OUT(49),  PIO_OUT(50),  PIO_OUT(47),  PIO_OUT(48)  )  <=  AW_Output_Reg(1)(15 downto 0) ;

            PIO_ENA(62 downto 47)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(2)(15 downto 0)  <=  ( PIO_SYNC(31),  PIO_SYNC(32),  PIO_SYNC(33),  PIO_SYNC(34),
                                          PIO_SYNC(35),  PIO_SYNC(36),  PIO_SYNC(37),  PIO_SYNC(38),
                                          PIO_SYNC(39),  PIO_SYNC(40),  PIO_SYNC(41),  PIO_SYNC(42),
                                          PIO_SYNC(43),  PIO_SYNC(44),  PIO_SYNC(45),  PIO_SYNC(46) );

          ( PIO_OUT(77),  PIO_OUT(78),  PIO_OUT(75),  PIO_OUT(76),
            PIO_OUT(73),  PIO_OUT(74),  PIO_OUT(71),  PIO_OUT(72),
            PIO_OUT(69),  PIO_OUT(70),  PIO_OUT(67),  PIO_OUT(68),
            PIO_OUT(65),  PIO_OUT(66),  PIO_OUT(63),  PIO_OUT(64)   )   <=  AW_Output_Reg(2)(15 downto 0) ;

            PIO_ENA(78 downto 63)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(3)(15 downto 0)  <=  ( PIO_SYNC(79),  PIO_SYNC(80),  PIO_SYNC(81),  PIO_SYNC(82),
                                          PIO_SYNC(83),  PIO_SYNC(84),  PIO_SYNC(85),  PIO_SYNC(86),
                                          PIO_SYNC(87),  PIO_SYNC(88),  PIO_SYNC(89),  PIO_SYNC(90),
                                          PIO_SYNC(91),  PIO_SYNC(92),  PIO_SYNC(93),  PIO_SYNC(94) );

          ( PIO_OUT(125), PIO_OUT(126), PIO_OUT(123), PIO_OUT(124),
            PIO_OUT(121), PIO_OUT(122), PIO_OUT(119), PIO_OUT(120),
            PIO_OUT(117), PIO_OUT(118), PIO_OUT(115), PIO_OUT(116),
            PIO_OUT(113), PIO_OUT(114), PIO_OUT(111), PIO_OUT(112)  )   <=  AW_Output_Reg(3)(15 downto 0) ;

            PIO_ENA(126 downto 111)                                     <= (others => '1'); -- Output-Enable


      AW_Input_Reg(4)(15 downto 0)  <=  ( PIO_SYNC(95),  PIO_SYNC(96),  PIO_SYNC(97),  PIO_SYNC(98),
                                          PIO_SYNC(99),  PIO_SYNC(100), PIO_SYNC(101), PIO_SYNC(102),
                                          PIO_SYNC(103), PIO_SYNC(104), PIO_SYNC(105), PIO_SYNC(106),
                                          PIO_SYNC(107), PIO_SYNC(108), PIO_SYNC(109), PIO_SYNC(110) );

          ( PIO_OUT(141), PIO_OUT(142), PIO_OUT(139), PIO_OUT(140),
            PIO_OUT(137), PIO_OUT(138), PIO_OUT(135), PIO_OUT(136),
            PIO_OUT(133), PIO_OUT(134), PIO_OUT(131), PIO_OUT(132),
            PIO_OUT(129), PIO_OUT(130), PIO_OUT(127), PIO_OUT(128)  )   <=  AW_Output_Reg(4)(15 downto 0) ;

            PIO_ENA(142 downto 127)                                     <= (others => '1'); -- Output-Enable


    AW_Input_Reg(5)(15 downto 4)  <=   AW_Output_Reg(5)(15 downto 4); --+   Input [15..4] = Copy der Output-Bits, da Testprog. nur 16 Bit Vergleich.
    AW_Input_Reg(5)(3  downto 0)  <=  (PIO_SYNC(143), PIO_SYNC(144), PIO_SYNC(149), PIO_SYNC(150));

   --  Beim Test, sind die Pins vom AW_Output_Reg(5)(3 downto 0) mit AW_Input_Reg(5)(3 downto 0) extern verbunden.

           (PIO_OUT(147), PIO_OUT(148), PIO_OUT(145), PIO_OUT(146))     <=  AW_Output_Reg(5)(3 downto 0) ;
            PIO_ENA(148 downto 145)                                     <= (others => '1'); -- Output-Enable


    --- Test der User-Pins zur VG-Leiste und HPLA1 (HP-Logicanalysator) ---

--    UIO(15 downto 0)              <= (OTHERS => 'Z');         -- UIO = Input;
--    AW_Input_Reg(6)(15 downto 0)  <=  UIO(15 downto 0);       -- User-Pins zur VG-Leiste als Input

    UIO_ENA(15 downto 0)          <= (OTHERS => '0');           -- UIO = Input;
    AW_Input_Reg(6)(15 downto 0)  <=  UIO_SYNC(15 downto 0);    -- User-Pins zur VG-Leiste als Input


    A_TA(15 downto 0)             <= AW_Output_Reg(6)(15 downto 0);  -- HPLA1 (HP-Logicanalysator) als Output


    --- Test Codierschalter ---

    AW_Input_Reg(7)(15 downto 4)  <=  (OTHERS => '0');         -- setze alle unbenutzten Bit's = 0
    AW_Input_Reg(7)(3 downto 0)   <=  not A_SEL(3 downto 0);   -- Lese Codierschalter (neg. Logic)


  else

    --#################################################################################
    --#################################################################################
    --###                                                                           ###
    --###                         Stecker Anwender I/O                              ###
    --###                                                                           ###
    --#################################################################################
    --#################################################################################



    --input: Anwender_ID ---
      AW_ID(7 downto 0)         <=  PIO_SYNC(150 downto 143);


    --  --- Output: Anwender-LED's ---

    PIO_OUT(17) <= s_nLED_Sel;                          -- LED7 = sel Board
    PIO_OUT(19) <= s_nLED_Dtack;                        -- LED6 = Dtack
    PIO_OUT(21) <= s_nLED_inR;                          -- LED5 = interrupt
    PIO_OUT(23) <= not Powerup_Done or clk_blink;       -- LED4 = Powerup
    PIO_OUT(25) <= s_nLED_User1_o;                      -- LED3 = User 1
    PIO_OUT(27) <= s_nLED_User2_o;                      -- LED2 = User 2
    PIO_OUT(29) <= s_nLED_User3_o;                      -- LED1 = User 3
    PIO_OUT(31) <= local_clk_is_running and clk_blink;  -- LED0 (User-4) = int. Clock

   (PIO_ENA(17), PIO_ENA(19), PIO_ENA(21), PIO_ENA(23),
    PIO_ENA(25), PIO_ENA(27), PIO_ENA(29), PIO_ENA(31) )  <=  std_logic_vector'("11111111"); --  Output-Enable


    A_TA(15 downto 0) <= hp_la_o(15 downto 0); ----------------- Output für HP-Logic-Analysator

    A_Tclk   <= signal_tap_clk_250mhz;  -- Clock  für HP-Logic-Analysator


--  UIO(0)  <= Interlock; -- Ist kein Interlock-Bit gesetzt ==> UIO(0) = 0

    UIO_OUT(0)  <= Interlock; -- Ist kein Interlock-Bit gesetzt ==> UIO(0) = 0
    UIO_ENA(0)  <= '1';       -- Output-Enable für Interlock-Bit
    AW_Input_Reg(6)   <=  Timing_Pattern_LA(31 downto 16);  -- H-Word vom Timing_Pattern
    AW_Input_Reg(7)   <=  Timing_Pattern_LA(15 downto 0);   -- L-Word vom Timing_Pattern


  CASE AW_ID(7 downto 0) IS


     WHEN   c_AW_INLB12S.ID  =>


       --###################################################################################
       --###################################################################################
       --####                                                                            ###
       --####      Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen     ###
       --####                                                                            ###
       --###################################################################################
       --###################################################################################


       extension_cid_system <= c_cid_system;       -- extension card: CSCOHW
       extension_cid_group  <= c_AW_INLB12S.CID;   -- extension card: cid_group, "FG902_050"

       AW_Status1(15 downto 0)  <=  (OTHERS => '0');                  -- Unbenutzte Status-Bits
        AW_Status2(15 downto 0)  <=  (OTHERS => '0');                 -- Unbenutzte Status-Bits

       Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
       Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
       Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

       --############################# Set Debounce- oder Syn-Time ######################################

         AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1

         IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN Debounce_cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
                                                ELSE Debounce_cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
         END IF;

       --################################### Set LED's ########################################

       s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei --
       s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei --
       s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei --


       --========================== Output Register 1 ======================================

       PIO_OUT(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA)
       PIO_ENA(86)   <=  '1';                -- Output Enable


       ---------------------------------------------------------------------------------------------------------------------------------------


       (PIO_OUT(112), PIO_OUT(48), PIO_OUT(130), PIO_OUT(30), PIO_OUT(137), PIO_OUT(35),
        PIO_OUT(119), PIO_OUT(53), PIO_OUT(101), PIO_OUT(73), PIO_OUT(96),  PIO_OUT(56))   <= IOBP_Output(12 downto 1); ----------- Data_Output-Pin's der "Slave-Karten 12-1"

       (PIO_ENA(112), PIO_ENA(48), PIO_ENA(130), PIO_ENA(30), PIO_ENA(137), PIO_ENA(35),
        PIO_ENA(119), PIO_ENA(53), PIO_ENA(101), PIO_ENA(73), PIO_ENA(96),  PIO_ENA(56))   <= std_logic_vector'("111111111111");   -- Output Enable


       IOBP_Input(1)(5 downto 1)  <= ( PIO_SYNC(62),  PIO_SYNC(54),  PIO_SYNC(60),  PIO_SYNC(52),  PIO_SYNC(58)  );  -- Input's 5-1 von den der Slave-Karte-1,
       IOBP_Input(2)(5 downto 1)  <= ( PIO_SYNC(102), PIO_SYNC(94),  PIO_SYNC(100), PIO_SYNC(92),  PIO_SYNC(98)  );  -- Input's 5-1 von den der Slave-Karte-2,
       IOBP_Input(3)(5 downto 1)  <= ( PIO_SYNC(79),  PIO_SYNC(71),  PIO_SYNC(77),  PIO_SYNC(69),  PIO_SYNC(75)  );  -- Input's 5-1 von den der Slave-Karte-3,
       IOBP_Input(4)(5 downto 1)  <= ( PIO_SYNC(93),  PIO_SYNC(103), PIO_SYNC(91),  PIO_SYNC(105), PIO_SYNC(89)  );  -- Input's 5-1 von den der Slave-Karte-4,
       IOBP_Input(5)(5 downto 1)  <= ( PIO_SYNC(63),  PIO_SYNC(55),  PIO_SYNC(61),  PIO_SYNC(57),  PIO_SYNC(59)  );  -- Input's 5-1 von den der Slave-Karte-5,
       IOBP_Input(6)(5 downto 1)  <= ( PIO_SYNC(111), PIO_SYNC(121), PIO_SYNC(109), PIO_SYNC(123), PIO_SYNC(107) );  -- Input's 5-1 von den der Slave-Karte-6,
       IOBP_Input(7)(5 downto 1)  <= ( PIO_SYNC(45),  PIO_SYNC(37),  PIO_SYNC(43),  PIO_SYNC(39),  PIO_SYNC(41)  );  -- Input's 5-1 von den der Slave-Karte-7,
       IOBP_Input(8)(5 downto 1)  <= ( PIO_SYNC(129), PIO_SYNC(139), PIO_SYNC(127), PIO_SYNC(141), PIO_SYNC(125) );  -- Input's 5-1 von den der Slave-Karte-8,
       IOBP_Input(9)(5 downto 1)  <= ( PIO_SYNC(20),  PIO_SYNC(28),  PIO_SYNC(22),  PIO_SYNC(26),  PIO_SYNC(24)  );  -- Input's 5-1 von den der Slave-Karte-9,
       IOBP_Input(10)(5 downto 1) <= ( PIO_SYNC(138), PIO_SYNC(128), PIO_SYNC(140), PIO_SYNC(126), PIO_SYNC(142) );  -- Input's 5-1 von den der Slave-Karte-10,
       IOBP_Input(11)(5 downto 1) <= ( PIO_SYNC(38),  PIO_SYNC(46),  PIO_SYNC(40),  PIO_SYNC(44),  PIO_SYNC(42)  );  -- Input's 5-1 von den der Slave-Karte-11,
       IOBP_Input(12)(5 downto 1) <= ( PIO_SYNC(120), PIO_SYNC(110), PIO_SYNC(122), PIO_SYNC(108), PIO_SYNC(124) );  -- Input's 5-1 von den der Slave-Karte-12,


       (PIO_OUT(114), PIO_OUT(50), PIO_OUT(132), PIO_OUT(32), PIO_OUT(135), PIO_OUT(33),
        PIO_OUT(117), PIO_OUT(51), PIO_OUT(99),  PIO_OUT(83), PIO_OUT(106), PIO_OUT(66))  <=  IOBP_STR_rot_o;   -- LED-Strobe Rot  für Slave 12-1
       (PIO_ENA(114), PIO_ENA(50), PIO_ENA(132), PIO_ENA(32), PIO_ENA(135), PIO_ENA(33),
        PIO_ENA(117), PIO_ENA(51), PIO_ENA(99),  PIO_ENA(83), PIO_ENA(106), PIO_ENA(66))  <=  std_logic_vector'("111111111111");   -- Output Enable

       (PIO_OUT(116), PIO_OUT(34), PIO_OUT(134), PIO_OUT(16), PIO_OUT(133), PIO_OUT(49),
        PIO_OUT(115), PIO_OUT(67), PIO_OUT(97),  PIO_OUT(81), PIO_OUT(104), PIO_OUT(64))  <=  IOBP_STR_gruen_o; -- LED-Strobe Grün für Slave 12-1
       (PIO_ENA(116), PIO_ENA(34), PIO_ENA(134), PIO_ENA(16), PIO_ENA(133), PIO_ENA(49),
        PIO_ENA(115), PIO_ENA(67), PIO_ENA(97),  PIO_ENA(81), PIO_ENA(104), PIO_ENA(64))  <=  std_logic_vector'("111111111111");   -- Output Enable

       (PIO_OUT(118), PIO_OUT(36), PIO_OUT(136), PIO_OUT(18), PIO_OUT(131), PIO_OUT(47),
        PIO_OUT(113), PIO_OUT(65), PIO_OUT(95),  PIO_OUT(85), PIO_OUT(90),  PIO_OUT(68))  <=  not IOBP_STR_ID_o;    -- ID-Strobe für Slave 12-1 (Enable ist L-Aktiv)
       (PIO_ENA(118), PIO_ENA(36), PIO_ENA(136), PIO_ENA(18), PIO_ENA(131), PIO_ENA(47),
        PIO_ENA(113), PIO_ENA(65), PIO_ENA(95),  PIO_ENA(85), PIO_ENA(90),  PIO_ENA(68))  <=  std_logic_vector'("111111111111");   -- Output Enable


   -------------------- Input/Output vom LED_ID_Bus der Zwischenbackplane  ------------

        IOBP_LED_ID_Bus_i <= (PIO_Sync(70), PIO_Sync(72), PIO_Sync(74), PIO_Sync(76), PIO_Sync(78), PIO_Sync(80), PIO_Sync(82), PIO_Sync(84));   ------------------------- Input  LED_ID_Bus
                             (PIO_OUT(70),  PIO_OUT(72),  PIO_OUT(74),  PIO_OUT(76),  PIO_OUT(78),  PIO_OUT(80),  PIO_OUT(82),  PIO_OUT(84))   <=  IOBP_LED_ID_Bus_o;   -- Output LED_ID_Bus


   -------------------- Tri-State Steuerung vom LED_ID_Bus der Zwischenbackplane  ------------

       IF IOBP_LED_En = '1' THEN ---------------- LED write Loop
         (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("11111111");  -- Output Enable
       ELSE --------------------------------------ID read Loop
         (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("00000000");  -- Output Disable
       END IF;


   -----------------------------------------------------------------------------------------------------------------------------------------


   ---------------- Output-Register(Maske) für die Iput- und Output Sel-LED's vom Slave 1-12
   --
   --  IOBP_Sel_LED's sind Low-Aktiv             Bit5 = Output          Bit[4..0]  =  Input[5..1]
   --        |                                         |                           |
       IOBP_Sel_LED(1) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 0) & IOBP_Masken_Reg(0)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 1
       IOBP_Sel_LED(2) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 1) & IOBP_Masken_Reg(0)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 2
       IOBP_Sel_LED(3) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 2) & IOBP_Masken_Reg(0)(14 downto 10) );  -- Register für Sel-LED's vom Slave 3
       IOBP_Sel_LED(4) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 3) & IOBP_Masken_Reg(1)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 4
       IOBP_Sel_LED(5) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 4) & IOBP_Masken_Reg(1)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 5
       IOBP_Sel_LED(6) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 5) & IOBP_Masken_Reg(1)(14 downto 10) );  -- Register für Sel-LED's vom Slave 6
       IOBP_Sel_LED(7) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 6) & IOBP_Masken_Reg(2)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 7
       IOBP_Sel_LED(8) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 7) & IOBP_Masken_Reg(2)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 8
       IOBP_Sel_LED(9) (6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 8) & IOBP_Masken_Reg(2)(14 downto 10) );  -- Register für Sel-LED's vom Slave 9
       IOBP_Sel_LED(10)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)( 9) & IOBP_Masken_Reg(3)( 4 downto  0) );  -- Register für Sel-LED's vom Slave 10
       IOBP_Sel_LED(11)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(10) & IOBP_Masken_Reg(3)( 9 downto  5) );  -- Register für Sel-LED's vom Slave 11
       IOBP_Sel_LED(12)(6 downto 1)  <=  not ( IOBP_Masken_Reg(4)(11) & IOBP_Masken_Reg(3)(14 downto 10) );  -- Register für Sel-LED's vom Slave 12


   ---------------------------------------------------------------------------------------------------------------------------------------

   --                    ID-Input-Register für die IO-Module Nr. 1+12

       IOBP_Id_Reg(5)(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
       IOBP_Id_Reg(5)( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
       IOBP_Id_Reg(4)(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
       IOBP_Id_Reg(4)( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
       IOBP_Id_Reg(3)(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
       IOBP_Id_Reg(3)( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
       IOBP_Id_Reg(2)(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
       IOBP_Id_Reg(2)( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
       IOBP_Id_Reg(1)(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
       IOBP_Id_Reg(1)( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
       IOBP_Id_Reg(0)(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
       IOBP_Id_Reg(0)( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1


   ---------------------------------------------------------------------------------------------------------------------------------------

   --################################ Debounce oder Sync Input's  ##################################

   --  Signal60_in = H-Aktiv             IOBP_Input = L-Aktiv
   --        |                                |
       Signal60_in( 4 DOWNTO  0)   <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
       Signal60_in( 9 DOWNTO  5)   <=  not IOBP_Input( 2)(5 downto 1);
       Signal60_in(14 DOWNTO 10)   <=  not IOBP_Input( 3)(5 downto 1);
       Signal60_in(19 DOWNTO 15)   <=  not IOBP_Input( 4)(5 downto 1);
       Signal60_in(24 DOWNTO 20)   <=  not IOBP_Input( 5)(5 downto 1);
       Signal60_in(29 DOWNTO 25)   <=  not IOBP_Input( 6)(5 downto 1);
       Signal60_in(34 DOWNTO 30)   <=  not IOBP_Input( 7)(5 downto 1);
       Signal60_in(39 DOWNTO 35)   <=  not IOBP_Input( 8)(5 downto 1);
       Signal60_in(44 DOWNTO 40)   <=  not IOBP_Input( 9)(5 downto 1);
       Signal60_in(49 DOWNTO 45)   <=  not IOBP_Input(10)(5 downto 1);
       Signal60_in(54 DOWNTO 50)   <=  not IOBP_Input(11)(5 downto 1);
       Signal60_in(59 DOWNTO 55)   <=  not IOBP_Input(12)(5 downto 1);


   --  Syn60 = H-Aktiv             IOBP_Input = L-Aktiv
   --                                      |
       Syn60( 4 DOWNTO  0)      <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
       Syn60( 9 DOWNTO  5)      <=  not IOBP_Input( 2)(5 downto 1);
       Syn60(14 DOWNTO 10)      <=  not IOBP_Input( 3)(5 downto 1);
       Syn60(19 DOWNTO 15)      <=  not IOBP_Input( 4)(5 downto 1);
       Syn60(24 DOWNTO 20)      <=  not IOBP_Input( 5)(5 downto 1);
       Syn60(29 DOWNTO 25)      <=  not IOBP_Input( 6)(5 downto 1);
       Syn60(34 DOWNTO 30)      <=  not IOBP_Input( 7)(5 downto 1);
       Syn60(39 DOWNTO 35)      <=  not IOBP_Input( 8)(5 downto 1);
       Syn60(44 DOWNTO 40)      <=  not IOBP_Input( 9)(5 downto 1);
       Syn60(49 DOWNTO 45)      <=  not IOBP_Input(10)(5 downto 1);
       Syn60(54 DOWNTO 50)      <=  not IOBP_Input(11)(5 downto 1);
       Syn60(59 DOWNTO 55)      <=  not IOBP_Input(12)(5 downto 1);


       IF  (Diob_Config1(11) = '1')  THEN 
         Deb_Sync60 <=  Syn60;         -- Dobounce = Abgeschaltet ==> nur Synchronisation
       ELSE 
         Deb_Sync60 <=  Deb60_out;     -- Debounce und Synchronisation
       END IF;


   --  ################################ Input's AND Maske zu Input-Register ###################

   --                  Input-Test, Stecker 1, 2, 3

       AW_Input_Reg(1)( 4 downto  0) <=   (Deb_Sync60( 4 downto  0) AND not IOBP_Masken_Reg(0)( 4 downto  0));  -- Input, IO-Modul Nr. 1
       AW_Input_Reg(1)( 9 downto  5) <=   (Deb_Sync60( 9 downto  5) AND not IOBP_Masken_Reg(0)( 9 downto  5));  -- Input, IO-Modul Nr. 2
       AW_Input_Reg(1)(14 downto 10) <=   (Deb_Sync60(14 downto 10) AND not IOBP_Masken_Reg(0)(14 downto 10));  -- Input, IO-Modul Nr. 3
       AW_Input_Reg(1)(15) <=  '0';

   --                  Input-Test, Stecker 4, 5, 6
       AW_Input_Reg(2)( 4 downto  0) <=   (Deb_Sync60(19 DOWNTO 15) AND not IOBP_Masken_Reg(1)( 4 downto  0));  -- Input, IO-Modul Nr. 4
       AW_Input_Reg(2)( 9 downto  5) <=   (Deb_Sync60(24 DOWNTO 20) AND not IOBP_Masken_Reg(1)( 9 downto  5));  -- Input, IO-Modul Nr. 5
       AW_Input_Reg(2)(14 downto 10) <=   (Deb_Sync60(29 DOWNTO 25) AND not IOBP_Masken_Reg(1)(14 downto 10));  -- Input, IO-Modul Nr. 6
       AW_Input_Reg(2)(15) <=  '0';

   --                  Input-Test, Stecker 7, 8, 9

       AW_Input_Reg(3)( 4 downto  0) <=   (Deb_Sync60(34 DOWNTO 30) AND not IOBP_Masken_Reg(2)( 4 downto  0));  -- Input, IO-Modul Nr. 7
       AW_Input_Reg(3)( 9 downto  5) <=   (Deb_Sync60(39 DOWNTO 35) AND not IOBP_Masken_Reg(2)( 9 downto  5));  -- Input, IO-Modul Nr. 8
       AW_Input_Reg(3)(14 downto 10) <=   (Deb_Sync60(44 DOWNTO 40) AND not IOBP_Masken_Reg(2)(14 downto 10));  -- Input, IO-Modul Nr. 9
       AW_Input_Reg(3)(15) <=  '0';

   --                    Input-Test, Stecker 10, 11, 12

       AW_Input_Reg(4)( 4 downto  0) <=   (Deb_Sync60(49 DOWNTO 45) AND not IOBP_Masken_Reg(3)( 4 downto  0));  -- Input, IO-Modul Nr. 10
       AW_Input_Reg(4)( 9 downto  5) <=   (Deb_Sync60(54 DOWNTO 50) AND not IOBP_Masken_Reg(3)( 9 downto  5));  -- Input, IO-Modul Nr. 11
       AW_Input_Reg(4)(14 downto 10) <=   (Deb_Sync60(59 DOWNTO 55) AND not IOBP_Masken_Reg(3)(14 downto 10));  -- Input, IO-Modul Nr. 12
       AW_Input_Reg(4)(15) <=  '0';



   --################################ Outputs AND Maske ##################################
   --
      -- case AW_Config2 is

       --  STANDARD OUTPUT OUTREG
   --                                                    MaskenBit=0 --> Enable
         IOBP_Output(1)  <= (AW_Output_Reg(1)( 0) AND not IOBP_Masken_Reg(4)( 0));  -- Output von Slave 1
         IOBP_Output(2)  <= (AW_Output_Reg(1)( 1) AND not IOBP_Masken_Reg(4)( 1));  -- Output von Slave 2
         IOBP_Output(3)  <= (AW_Output_Reg(1)( 2) AND not IOBP_Masken_Reg(4)( 2));  -- Output von Slave 3
         IOBP_Output(4)  <= (AW_Output_Reg(1)( 3) AND not IOBP_Masken_Reg(4)( 3));  -- Output von Slave 4
         IOBP_Output(5)  <= (AW_Output_Reg(1)( 4) AND not IOBP_Masken_Reg(4)( 4));  -- Output von Slave 5
         IOBP_Output(6)  <= (AW_Output_Reg(1)( 5) AND not IOBP_Masken_Reg(4)( 5));  -- Output von Slave 6
         IOBP_Output(7)  <= (AW_Output_Reg(1)( 6) AND not IOBP_Masken_Reg(4)( 6));  -- Output von Slave 7
         IOBP_Output(8)  <= (AW_Output_Reg(1)( 7) AND not IOBP_Masken_Reg(4)( 7));  -- Output von Slave 8
         IOBP_Output(9)  <= (AW_Output_Reg(1)( 8) AND not IOBP_Masken_Reg(4)( 8));  -- Output von Slave 9
         IOBP_Output(10) <= (AW_Output_Reg(1)( 9) AND not IOBP_Masken_Reg(4)( 9));  -- Output von Slave 10
         IOBP_Output(11) <= (AW_Output_Reg(1)(10) AND not IOBP_Masken_Reg(4)(10));  -- Output von Slave 11
         IOBP_Output(12) <= (AW_Output_Reg(1)(11) AND not IOBP_Masken_Reg(4)(11));  -- Output von Slave 12
       --end Case


   --################################ Aktiv-Led's ##################################
   --
   --                          maskierte Outputs            entprellte Inputs
   --                                   |
       IOBP_Aktiv_LED_i(1)  <=  (IOBP_Output(1)    &  Deb60_out( 4 DOWNTO  0));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(2)  <=  (IOBP_Output(2)    &  Deb60_out( 9 DOWNTO  5));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(3)  <=  (IOBP_Output(3)    &  Deb60_out(14 DOWNTO 10));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(4)  <=  (IOBP_Output(4)    &  Deb60_out(19 DOWNTO 15));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(5)  <=  (IOBP_Output(5)    &  Deb60_out(24 DOWNTO 20));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(6)  <=  (IOBP_Output(6)    &  Deb60_out(29 DOWNTO 25));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(7)  <=  (IOBP_Output(7)    &  Deb60_out(34 DOWNTO 30));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(8)  <=  (IOBP_Output(8)    &  Deb60_out(39 DOWNTO 35));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(9)  <=  (IOBP_Output(9)    &  Deb60_out(44 DOWNTO 40));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(10) <=  (IOBP_Output(10)   &  Deb60_out(49 DOWNTO 45));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(11) <=  (IOBP_Output(11)   &  Deb60_out(54 DOWNTO 50));  -- Signale für Aktiv-LED's
       IOBP_Aktiv_LED_i(12) <=  (IOBP_Output(12)   &  Deb60_out(59 DOWNTO 55));  -- Signale für Aktiv-LED's




      -----------------------------------------------------------------------------------------------------------------------------------------

   WHEN  c_AW_INLB12S1.ID  =>


         --########################################################################################
         --########################################################################################
         --####                                                                                 ###
         --####      Anwender-IO: FG902_0xx -- Newe Interlock-Backplane mit 12 Steckplätzen     ###
         --####                                                                                 ###
         --########################################################################################
         --########################################################################################

         extension_cid_group  <= c_AW_INLB12S1.CID; -- extension card: cid_group, new Zwischenplane "FG902_xxx"

         extension_cid_system <= c_cid_system;       -- extension card: CSCOHW

         AW_Status1(15 downto 0)  <=  (OTHERS => '0');                   -- Unbenutzte Status-Bits
         AW_Status2(15 downto 0)  <=  (OTHERS => '0');                   -- Unbenutzte Status-Bits

         Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
         Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
         Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

         --############################# Set Debounce- oder Syn-Time ######################################

           AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1

           IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN Debounce_cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
                                                  ELSE Debounce_cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
           END IF;

         --################################### Set LED's ########################################

         s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei --
         s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei --
         s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei --


         --========================== Output Register 1 ======================================

         PIO_OUT(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA)
         PIO_ENA(86)   <=  '1';                -- Output Enable
         ---------------------------------------------------------------------------------------------------------------------------------------

         --========================== Output Register 2 ======================================

         PIO_OUT(88)   <=  '0';  ---------------- Output_Enable OEn2 (nach init vom ALTERA)
         PIO_ENA(88)   <=  '1';                -- Output Enable
         ---------------------------------------------------------------------------------------------------------------------------------------

         --                    ID-Input-Register für die IO-Module Nr. 1+12

         IOBP_Id_Reg(5)(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
         IOBP_Id_Reg(5)( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
         IOBP_Id_Reg(4)(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
         IOBP_Id_Reg(4)( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
         IOBP_Id_Reg(3)(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
         IOBP_Id_Reg(3)( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
         IOBP_Id_Reg(2)(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
         IOBP_Id_Reg(2)( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
         IOBP_Id_Reg(1)(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
         IOBP_Id_Reg(1)( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
         IOBP_Id_Reg(0)(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
         IOBP_Id_Reg(0)( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1

         -----------------------------------------------------------------------------------------------------------------------------------------
         ------------------------- general LED Assigments - intermediate backplane ---------------------------------------------------------------
         -----------------------------------------------------------------------------------------------------------------------------------------

         (PIO_OUT(114), PIO_OUT(50), PIO_OUT(132), PIO_OUT(32), PIO_OUT(135), PIO_OUT(33),
         PIO_OUT(117), PIO_OUT(51), PIO_OUT(99),  PIO_OUT(83), PIO_OUT(106), PIO_OUT(66))  <=  IOBP_STR_rot_o;   -- LED-Strobe Rot  für Slave 12-1
         (PIO_ENA(114), PIO_ENA(50), PIO_ENA(132), PIO_ENA(32), PIO_ENA(135), PIO_ENA(33),
         PIO_ENA(117), PIO_ENA(51), PIO_ENA(99),  PIO_ENA(83), PIO_ENA(106), PIO_ENA(66))  <=  std_logic_vector'("111111111111");   -- Output Enable

         (PIO_OUT(116), PIO_OUT(34), PIO_OUT(134), PIO_OUT(16), PIO_OUT(133), PIO_OUT(49),
         PIO_OUT(115), PIO_OUT(67), PIO_OUT(97),  PIO_OUT(81), PIO_OUT(104), PIO_OUT(64))  <=  IOBP_STR_gruen_o; -- LED-Strobe Grün für Slave 12-1
         (PIO_ENA(116), PIO_ENA(34), PIO_ENA(134), PIO_ENA(16), PIO_ENA(133), PIO_ENA(49),
         PIO_ENA(115), PIO_ENA(67), PIO_ENA(97),  PIO_ENA(81), PIO_ENA(104), PIO_ENA(64))  <=  std_logic_vector'("111111111111");   -- Output Enable

         (PIO_OUT(118), PIO_OUT(36), PIO_OUT(136), PIO_OUT(18), PIO_OUT(131), PIO_OUT(47),
         PIO_OUT(113), PIO_OUT(65), PIO_OUT(95),  PIO_OUT(85), PIO_OUT(90),  PIO_OUT(68))  <=  not IOBP_STR_ID_o;    -- ID-Strobe für Slave 12-1 (Enable ist L-Aktiv)
         (PIO_ENA(118), PIO_ENA(36), PIO_ENA(136), PIO_ENA(18), PIO_ENA(131), PIO_ENA(47),
         PIO_ENA(113), PIO_ENA(65), PIO_ENA(95),  PIO_ENA(85), PIO_ENA(90),  PIO_ENA(68))  <=  std_logic_vector'("111111111111");   -- Output Enable


         -------------------- Input/Output vom LED_ID_Bus der Zwischenbackplane  ------------
         IOBP_LED_ID_Bus_i <= (PIO_Sync(70), PIO_Sync(72), PIO_Sync(74), PIO_Sync(76), PIO_Sync(78), PIO_Sync(80), PIO_Sync(82), PIO_Sync(84));   ------------------------- Input  LED_ID_Bus
                          (PIO_OUT(70),  PIO_OUT(72),  PIO_OUT(74),  PIO_OUT(76),  PIO_OUT(78),  PIO_OUT(80),  PIO_OUT(82),  PIO_OUT(84))   <=  IOBP_LED_ID_Bus_o;   -- Output LED_ID_Bus


         -------------------- Tri-State Steuerung vom LED_ID_Bus der Zwischenbackplane  ------------
         IF IOBP_LED_En = '1' THEN ---------------- LED write Loop
         (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("11111111");  -- Output Enable
         ELSE --------------------------------------ID read Loop
         (PIO_ENA(70), PIO_ENA(72), PIO_ENA(74), PIO_ENA(76), PIO_ENA(78), PIO_ENA(80), PIO_ENA(82), PIO_ENA(84))  <=  std_logic_vector'("00000000");  -- Output Disable
         END IF;


         -----------------------------------------------------------------------------------------------------------------------------------------
         ( PIO_ENA(56),  PIO_ENA(62),  PIO_ENA(54),  PIO_ENA(60),  PIO_ENA(52),  PIO_ENA(58)) <= PIO_ENA_SLOT(1);
         ( PIO_ENA(96),  PIO_ENA(102), PIO_ENA(94),  PIO_ENA(100), PIO_ENA(92),  PIO_ENA(98)) <= PIO_ENA_SLOT(2);
         ( PIO_ENA(73),  PIO_ENA(79),  PIO_ENA(71),  PIO_ENA(77),  PIO_ENA(69),  PIO_ENA(75)) <= PIO_ENA_SLOT(3);
         ( PIO_ENA(101), PIO_ENA(93),  PIO_ENA(103), PIO_ENA(91),  PIO_ENA(105), PIO_ENA(89)) <= PIO_ENA_SLOT(4);
         ( PIO_ENA(53),  PIO_ENA(63),  PIO_ENA(55),  PIO_ENA(61),  PIO_ENA(57),  PIO_ENA(59)) <= PIO_ENA_SLOT(5);
         ( PIO_ENA(119), PIO_ENA(111), PIO_ENA(121), PIO_ENA(109), PIO_ENA(123), PIO_ENA(107))<= PIO_ENA_SLOT(6);
         ( PIO_ENA(35),  PIO_ENA(45),  PIO_ENA(37),  PIO_ENA(43),  PIO_ENA(39),  PIO_ENA(41)) <= PIO_ENA_SLOT(7);
         ( PIO_ENA(137), PIO_ENA(129), PIO_ENA(139), PIO_ENA(127), PIO_ENA(141), PIO_ENA(125))<= PIO_ENA_SLOT(8);
         ( PIO_ENA(30),  PIO_ENA(20),  PIO_ENA(28),  PIO_ENA(22),  PIO_ENA(26),  PIO_ENA(24)) <= PIO_ENA_SLOT(9);
         ( PIO_ENA(130), PIO_ENA(138), PIO_ENA(128), PIO_ENA(140), PIO_ENA(126), PIO_ENA(142))<= PIO_ENA_SLOT(10);
         ( PIO_ENA(48),  PIO_ENA(38),  PIO_ENA(46),  PIO_ENA(40),  PIO_ENA(44),  PIO_ENA(42)) <= PIO_ENA_SLOT(11);
         ( PIO_ENA(112), PIO_ENA(120), PIO_ENA(110), PIO_ENA(122), PIO_ENA(108), PIO_ENA(124))<= PIO_ENA_SLOT(12);


            case AW_Config2 is
               when x"DEDE" => --Quench Detection Development
               
               
               when OTHERS =>

                ( PIO_OUT(56),  PIO_OUT(62),  PIO_OUT(54),  PIO_OUT(60),  PIO_OUT(52),  PIO_OUT(58)) <=  PIO_OUT_SLOT(1);
                ( PIO_OUT(96),  PIO_OUT(102), PIO_OUT(94), PIO_OUT(100),  PIO_OUT(92),  PIO_OUT(98)) <=  PIO_OUT_SLOT(2);
                ( PIO_OUT(73),  PIO_OUT(79),  PIO_OUT(71),  PIO_OUT(77),  PIO_OUT(69),  PIO_OUT(75)) <=  PIO_OUT_SLOT(3);
                ( PIO_OUT(101), PIO_OUT(93),  PIO_OUT(103), PIO_OUT(91),  PIO_OUT(105), PIO_OUT(89)) <=  PIO_OUT_SLOT(4);
                ( PIO_OUT(53),  PIO_OUT(63),  PIO_OUT(55),  PIO_OUT(61),  PIO_OUT(57),  PIO_OUT(59)) <=  PIO_OUT_SLOT(5);
                ( PIO_OUT(119), PIO_OUT(111), PIO_OUT(121), PIO_OUT(109), PIO_OUT(123), PIO_OUT(107))<=  PIO_OUT_SLOT(6);
                ( PIO_OUT(35),  PIO_OUT(45),  PIO_OUT(37),  PIO_OUT(43),  PIO_OUT(39),  PIO_OUT(41)) <=  PIO_OUT_SLOT(7);
                ( PIO_OUT(137), PIO_OUT(129), PIO_OUT(139), PIO_OUT(127), PIO_OUT(141), PIO_OUT(125))<=  PIO_OUT_SLOT(8);
                ( PIO_OUT(30),  PIO_OUT(20),  PIO_OUT(28),  PIO_OUT(22),  PIO_OUT(26),  PIO_OUT(24)) <=  PIO_OUT_SLOT(9);
                ( PIO_OUT(130), PIO_OUT(138), PIO_OUT(128), PIO_OUT(140), PIO_OUT(126), PIO_OUT(142))<=  PIO_OUT_SLOT(10);
                ( PIO_OUT(48),  PIO_OUT(38),  PIO_OUT(46),  PIO_OUT(40),  PIO_OUT(44),  PIO_OUT(42)) <=  PIO_OUT_SLOT(11);
                ( PIO_OUT(112), PIO_OUT(120), PIO_OUT(110), PIO_OUT(122), PIO_OUT(108), PIO_OUT(124))<=  PIO_OUT_SLOT(12);

            end case;

         --( PIO_OUT(56),  PIO_OUT(62),  PIO_OUT(54),  PIO_OUT(60),  PIO_OUT(52),  PIO_OUT(58)) <= PIO_OUT_SLOT(1);
         --( PIO_OUT(96),  PIO_OUT(102), PIO_OUT(94), PIO_OUT(100),  PIO_OUT(92),  PIO_OUT(98)) <= PIO_OUT_SLOT(2);
         --( PIO_OUT(73),  PIO_OUT(79),  PIO_OUT(71),  PIO_OUT(77),  PIO_OUT(69),  PIO_OUT(75)) <= PIO_OUT_SLOT(3);
         --( PIO_OUT(101), PIO_OUT(93),  PIO_OUT(103), PIO_OUT(91),  PIO_OUT(105), PIO_OUT(89)) <= PIO_OUT_SLOT(4);
         --( PIO_OUT(53),  PIO_OUT(63),  PIO_OUT(55),  PIO_OUT(61),  PIO_OUT(57),  PIO_OUT(59)) <= PIO_OUT_SLOT(5);
         --( PIO_OUT(119), PIO_OUT(111), PIO_OUT(121), PIO_OUT(109), PIO_OUT(123), PIO_OUT(107))<= PIO_OUT_SLOT(6);
         --( PIO_OUT(35),  PIO_OUT(45),  PIO_OUT(37),  PIO_OUT(43),  PIO_OUT(39),  PIO_OUT(41)) <= PIO_OUT_SLOT(7);
         --( PIO_OUT(137), PIO_OUT(129), PIO_OUT(139), PIO_OUT(127), PIO_OUT(141), PIO_OUT(125))<= PIO_OUT_SLOT(8);
         --( PIO_OUT(30),  PIO_OUT(20),  PIO_OUT(28),  PIO_OUT(22),  PIO_OUT(26),  PIO_OUT(24)) <= PIO_OUT_SLOT(9);
         --( PIO_OUT(130), PIO_OUT(138), PIO_OUT(128), PIO_OUT(140), PIO_OUT(126), PIO_OUT(142))<= PIO_OUT_SLOT(10;
         --( PIO_OUT(48),  PIO_OUT(38),  PIO_OUT(46),  PIO_OUT(40),  PIO_OUT(44),  PIO_OUT(42)) <= PIO_OUT_SLOT(11);
         --( PIO_OUT(112), PIO_OUT(120), PIO_OUT(110), PIO_OUT(122), PIO_OUT(108), PIO_OUT(124))<= PIO_OUT_SLOT(12);

         dgen: 
         for I in 0 to 3 loop
            AW_Input_Reg(I+1)(5 downto 0)   <= AW_SK_Input_Reg(2*I);
            AW_Input_Reg(I+1)(11 downto 6)  <= AW_SK_Input_Reg((2*I)+1);   
            AW_Input_Reg(I+1)(15 downto 12) <="0000";
         end loop;

         IOBP_Aktiv_LED_i <= IOBP_SK_Aktiv_LED_i;

         ---output readback
         outrd: 
         for I in 0 to 5 loop
            IOBP_Output_Readback(I)(5 downto 0)  <= IOBP_SK_Output((2*I)+1);
            IOBP_Output_Readback(I)(11 downto 6) <= IOBP_SK_Output((2*I)+2);
            IOBP_Output_Readback(I)(15 downto 12)<="0000"; 
         end loop;   

         IOBP_Output_Readback(6) <= (OTHERS => '0');
         IOBP_Output_Readback(7) <= (OTHERS => '0');

         --IOBP_Output_Readback(0) <= "0000" & IOBP_SK_Output(2) & IOBP_SK_Output(1);
         --IOBP_Output_Readback(1) <= "0000" & IOBP_SK_Output(4) & IOBP_SK_Output(3);
         --IOBP_Output_Readback(2) <= "0000" & IOBP_SK_Output(6) & IOBP_SK_Output(5);
         --IOBP_Output_Readback(3) <= "0000" & IOBP_SK_Output(8) & IOBP_SK_Output(7);
         --IOBP_Output_Readback(4) <= "0000" & IOBP_SK_Output(10) & IOBP_SK_Output(9);
         --IOBP_Output_Readback(5) <= "0000" & IOBP_SK_Output(12) & IOBP_SK_Output(11);


         ---------------- Output-Register(Maske) für die Iput- und Output Sel-LED's vom Slave 1-12
         IOBP_Sel_Led <= IOBP_SK_Sel_Led;


         --################################ Debounce oder Sync Input's  ##################################

         --  Deb72_in = H-Aktiv             IOBP_Input = L-Aktiv
         --        |                                |
         Deb72_in(0) <=  not IOBP_SK_Input( 1);  -- Input-Daten
         Deb72_in(1) <=  not IOBP_SK_Input( 2);
         Deb72_in(2) <=  not IOBP_SK_Input( 3);
         Deb72_in(3) <=  not IOBP_SK_Input( 4);
         Deb72_in(4) <=  not IOBP_SK_Input( 5);
         Deb72_in(5) <=  not IOBP_SK_Input( 6);
         Deb72_in(6) <=  not IOBP_SK_Input( 7);
         Deb72_in(7) <=  not IOBP_SK_Input( 8);
         Deb72_in(8) <=  not IOBP_SK_Input( 9);
         Deb72_in(9) <=  not IOBP_SK_Input( 10);
         Deb72_in(10)<=  not IOBP_SK_Input( 11);
         Deb72_in(11)<=  not IOBP_SK_Input( 12);
         --  Syn72 = H-Aktiv             IOBP_Input = L-Aktiv
         --                                      |
         Syn72(0)   <=  not IOBP_SK_Input( 1);  -- Input-Daten
         Syn72(1)   <=  not IOBP_SK_Input( 2);
         Syn72(2)   <=  not IOBP_SK_Input( 3);
         Syn72(3)   <=  not IOBP_SK_Input( 4);
         Syn72(4)   <=  not IOBP_SK_Input( 5);
         Syn72(5)   <=  not IOBP_SK_Input( 6);
         Syn72(6)   <=  not IOBP_SK_Input( 7);
         Syn72(7)   <=  not IOBP_SK_Input( 8);
         Syn72(8)   <=  not IOBP_SK_Input( 9);
         Syn72(9)   <=  not IOBP_SK_Input( 10);
         Syn72(10)  <=  not IOBP_SK_Input( 11);
         Syn72(11)  <=  not IOBP_SK_Input( 12);


         --Debounce ON/OFF
         IF  (Diob_Config1(11) = '1')  THEN 
            Deb_Sync72 <=  Syn72;         -- Dobounce = Abgeschaltet => nur Synchronisation
         ELSE 
            Deb_Sync72 <=  Deb72_out;     -- Debounce und Synchronisation
         END IF;

         -----------------------------------------------------------------------------------------------------------------------------------------



     WHEN OTHERS =>

          extension_cid_system <=  0;  -- extension card: cid_system
          extension_cid_group  <=  0;  -- extension card: cid_group

          Max_AWOut_Reg_Nr     <=  0;  -- Maximale AWOut-Reg-Nummer der Anwendung
          Max_AWIn_Reg_Nr      <=  0;  -- Maximale AWIn-Reg-Nummer der Anwendung
          Min_AWIn_Deb_Time    <=  0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

          s_nLED_User1_i       <= '0';        -- LED3 = User 1, -- frei --
          s_nLED_User2_i       <= '0';        -- LED3 = User 2, -- frei --
          s_nLED_User3_i       <= '0';        -- LED3 = User 3, -- frei --


        -- Output: Anwender-LED's ---

          PIO_OUT(17)   <=  clk_blink; -- LED7
          PIO_OUT(19)   <=  clk_blink; -- LED6
          PIO_OUT(21)   <=  clk_blink; -- LED5
          PIO_OUT(23)   <=  clk_blink; -- LED4
          PIO_OUT(25)   <=  clk_blink; -- LED3
          PIO_OUT(27)   <=  clk_blink; -- LED2
          PIO_OUT(29)   <=  clk_blink; -- LED1
          PIO_OUT(31)   <=  clk_blink; -- LED0

         (PIO_ENA(17), PIO_ENA(19), PIO_ENA(21), PIO_ENA(23),
          PIO_ENA(25), PIO_ENA(27), PIO_ENA(29), PIO_ENA(31) )  <=  std_logic_vector'("11111111"); -- Output Enable

     END CASE;

  END IF;

END PROCESS p_AW_MUX;

end architecture;

--INL_xor1: io_reg
--generic map(
--      Base_addr =>  c_INL_xor1_Base_Addr
--      )
--port map  (
--      Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
--      Data_from_SCUB_LA  =>  Data_from_SCUB_LA,--RegIdxI <= to_integer(RegIdx);
--      Ext_Adr_Val        =>  Ext_Adr_Val,
--      Ext_Rd_active      =>  Ext_Rd_active,
--      Ext_Rd_fin         =>  Ext_Rd_fin,
--      Ext_Wr_active      =>  Ext_Wr_active,
--      Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
--      clk                =>  clk_sys,
--      nReset             =>  rstn_sys,
----
--      Reg_IO1             =>  INL_xor_IO1,
--      Reg_IO2             =>  INL_xor_IO2,
--      Reg_IO3             =>  INL_xor_IO3,
--      Reg_IO4             =>  INL_xor_IO4,
--      Reg_IO5             =>  INL_xor_IO5,
--      Reg_IO6             =>  INL_xor_IO6,
--      Reg_IO7             =>  INL_xor_IO7,
--      Reg_IO8             =>  open,
----
--      Reg_rd_active       =>  INL_xor1_rd_active,
--      Dtack_to_SCUB       =>  INL_xor1_Dtack,
--      Data_to_SCUB        =>  INL_xor1_data_to_SCUB
--    );


--INL_msk1: io_reg
--generic map(
--      Base_addr =>  c_INL_msk1_Base_Addr
--      )
--port map  (
--      Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
--      Data_from_SCUB_LA  =>  Data_from_SCUB_LA,
--      Ext_Adr_Val        =>  Ext_Adr_Val,
--      Ext_Rd_active      =>  Ext_Rd_active,
--      Ext_Rd_fin         =>  Ext_Rd_fin,
--      Ext_Wr_active      =>  Ext_Wr_active,
--      Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
--      clk                =>  clk_sys,
--      nReset             =>  rstn_sys,
----
--      Reg_IO1            =>  INL_msk_IO1,
--      Reg_IO2            =>  INL_msk_IO2,
--      Reg_IO3            =>  INL_msk_IO3,
--      Reg_IO4            =>  INL_msk_IO4,
--      Reg_IO5            =>  INL_msk_IO5,
--      Reg_IO6            =>  INL_msk_IO6,
--      Reg_IO7            =>  INL_msk_IO7,
--      Reg_IO8            =>  open,
----
--      Reg_rd_active      =>  INL_msk1_rd_active,
--      Dtack_to_SCUB      =>  INL_msk1_Dtack,
--      Data_to_SCUB       =>  INL_msk1_data_to_SCUB
--    );

