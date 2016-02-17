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
--      Base_addr +3 : DIOB-Status-Register1 (die Status-Bit's werden nach dem Lesen glöscht)                                     --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-0 |  frei                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--     Base_addr + 4: Die Bits im Anwender(Piggy)Config-Register1 haben für jedes Piggy eine andere Bedeutung                     --
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
--     8 | Output-Polarität Lemo,         1 = Negativ,  0 = Positiv(Default)                                                      --
--     7 | Output-Polarität Bit [23..0],  1 = Negativ,  0 = Positiv(Default)                                                      --
--   ----+-----------------------------------------------------------------------                                                 --
--     6 | Enable Output-Lemo,            1 = Enable,   0 = Disable(Default)                                                      --
--     5 | Enable Output-Bit [23..20],    1 = Enable,   0 = Disable(Default)                                                      --
--     4 | Enable Output-Bit [19..16],    1 = Enable,   0 = Disable(Default)                                                      --
--     3 | Enable Output-Bit [15..12],    1 = Enable,   0 = Disable(Default)                                                      --
--     2 | Enable Output-Bit [11..8],     1 = Enable,   0 = Disable(Default)                                                      --
--     1 | Enable Output-Bit [7..4],      1 = Enable,   0 = Disable(Default)                                                      --
--     0 | Enable Output-Bit [3..0],      1 = Enable,   0 = Disable(Default)                                                      --
--   ----+-----------------------------------------------------------------------                                                 --
--                                                                                                                                --
------------------------------------------------------------------------------------------------------------------------------------


entity scu_diob is
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
end scu_diob;



architecture scu_diob_arch of scu_diob is


--  +============================================================================================================================+
--  |                                 Firmware_Version/Firmware_Release und Basis-Adressen                                       |
--  +============================================================================================================================+

    CONSTANT c_Firmware_Version:    Integer := 0;   ---- Firmware_Version
--  CONSTANT c_Firmware_Release:    Integer := 9;   ---- Firmware_Release
--  CONSTANT c_Firmware_Release:    Integer := 10;  ---- Firmware_Release Stand 03.08.2015 ('740 nur Standard- kein DAC/FG-Mode)
--  CONSTANT c_Firmware_Release:    Integer := 11;  ---- Firmware_Release Stand 04.09.2015 ('Fehler LED-Lemo-Out, IO-Modul-Backplane-Test)
--  CONSTANT c_Firmware_Release:    Integer := 12;  ---- Firmware_Release Stand 10.09.2015 ('IO-Modul-Backplane-Test Fortsetzung)
--  CONSTANT c_Firmware_Release:    Integer := 13;  ---- Firmware_Release Stand 09.10.2015 ('+ 16In, 16Out, CID SPSIOI1 60 geändert in 68)
--  CONSTANT c_Firmware_Release:    Integer := 14;  ---- Firmware_Release Stand 12.10.2015 ( +'731 und Fehlerkorrektur)
--  CONSTANT c_Firmware_Release:    Integer := 15;  ---- Firmware_Release Stand 21.10.2015 ( +'731 und Fehlerkorrektur)
--  CONSTANT c_Firmware_Release:    Integer := 16;  ---- Firmware_Release Stand 19.11.2015 ( Update Reg.-Belegung '700' u. '710')
--  CONSTANT c_Firmware_Release:    Integer := 17;  ---- Firmware_Release Stand 02.12.2015 ( Strobe und Trigger (ECC) auf '710' geändert)
--  CONSTANT c_Firmware_Release:    Integer := 18;  ---- Firmware_Release Stand 28.01.2016 ( Error, Tag-Steuerung: überlappende Outputs im gleichen Register)
--  CONSTANT c_Firmware_Release:    Integer := 19;  ---- Firmware_Release Stand 08.02.2016 ( + Tag-Steuerung: Level für jedes Bit im Outp.-Register ist einstellbar)
    CONSTANT c_Firmware_Release:    Integer := 20;  ---- Firmware_release Stand 17.02.2016 ( INFO ROM in Housekeeping Modul eingefügt)

    
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
    CONSTANT c_IOBP_Masken_Base_Addr:            Integer := 16#0600#;  -- IO-Backplane Masken-Register
   
  
  
--  +============================================================================================================================+
--  |                                                 CONSTANT                                                                   |
--  +============================================================================================================================+


    CONSTANT c_cid_system:     integer range 0 to 16#FFFF#:= 55;     -- extension card: cid_system, CSCOHW=55


    type ID_CID is record
      ID   : std_logic_vector(7 downto 0);
      CID  : integer range 0 to 16#FFFF#;
    end record;
--                                        +--------------- Piggy-ID(Codierung)
--                                        |     +--------- CID(extension card: cid_system)
--                                        |     |     
    CONSTANT c_AW_P37IO:      ID_CID:= (x"01", 27);   ---- Piggy-ID(Codierung), B"0000_0001", FG900_700
    CONSTANT c_AW_P25IO:      ID_CID:= (x"02", 28);   ---- Piggy-ID(Codierung), B"0000_0010", FG900_710
    CONSTANT c_AW_OCin:       ID_CID:= (x"03", 29);   ---- Piggy-ID(Codierung), B"0000_0011", FG900_720
    CONSTANT c_AW_OCIO1:      ID_CID:= (x"04", 30);   ---- Piggy-ID(Codierung), B"0000_0100", FG900_730
    CONSTANT c_AW_UIO:        ID_CID:= (x"05", 31);   ---- Piggy-ID(Codierung), B"0000_0101", FG900_740
    CONSTANT c_AW_DA:         ID_CID:= (x"06", 32);   ---- Piggy-ID(Codierung), B"0000_0110", FG900_750
    CONSTANT c_AW_Frei:       ID_CID:= (x"07", 00);   ---- Piggy-ID(Codierung), B"0000_0111", FG900_760
    CONSTANT c_AW_SPSIO1:     ID_CID:= (x"08", 33);   ---- Piggy-ID(Codierung), B"0000_1000", FG900_770 -- Ausgänge schalten nach 24V
    CONSTANT c_AW_HFIO:       ID_CID:= (x"09", 34);   ---- Piggy-ID(Codierung), B"0000_1001", FG900_780
    CONSTANT c_AW_SPSIOI1:    ID_CID:= (x"0A", 68);   ---- Piggy-ID(Codierung), B"0000_1010", FG901_770 -- Ausgänge schalten nach GND
    CONSTANT c_AW_INLB12S:    ID_CID:= (x"0B", 67);   ---- Piggy-ID(Codierung), B"0000_1011", FG902_050 -- IO-Modul-Backplane mit 12 Steckplätzen
    CONSTANT c_AW_16Out2:     ID_CID:= (x"0C", 70);   ---- Piggy-ID(Codierung), B"0000_1100", FG901_010 -- Output 16 Bit
    CONSTANT c_AW_16In2:      ID_CID:= (x"0D", 71);   ---- Piggy-ID(Codierung), B"0000_1101", FG901_020 -- Input 16 Bit
    CONSTANT c_AW_OCIO2:      ID_CID:= (x"0E", 61);   ---- Piggy-ID(Codierung), B"0000_1110", FG900_731
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
                                                 
                                                 

                                               
                                             
  CONSTANT C_Cnt_Syn_min:  integer := 3;   -- Anzahl der Clock's, für die Synchonnisation der Eingänge bei abgeschalter Entprellung 
  
  CONSTANT C_Strobe_100ns:  integer range 0 to 16383:= (000100 * 1000 / CLK_sys_in_ps);   -- Anzahl der Clock's für den Strobe 100ns 
  
  TYPE   t_Integer_Strobe_Array     is array (0 to 7) of integer range 0 to 65535;
  
  constant Wert_Strobe_2_Hoch_n : t_Integer_Strobe_Array := (00001, 00002, 00004, 00008, 00016, 00032, 00064, 00128);
            
            
  
--  +============================================================================================================================+
--  |                                                    Component                                                               |
--  +============================================================================================================================+


component config_status
  generic ( CS_Base_addr  : integer );
  port (
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    Diob_Status1:         in   std_logic_vector(15 downto 0);    -- Input-Port 1
    Diob_Status2:         in   std_logic_vector(15 downto 0);    -- Input-Port 2
    AW_Status1:           in   std_logic_vector(15 downto 0);    -- Input-Port 3
    AW_Status2:           in   std_logic_vector(15 downto 0);    -- Input-Port 4
   
    Diob_Config1:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut1
    Diob_Config2:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut2
    AW_Config1:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut3
    AW_Config2:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut4

    Mirr_OutReg_Maske:    out  std_logic_vector(15 downto 0);    -- Maskierung für Spiegel-Modus des Ausgangsregisters

    Diob_Config1_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut1
    Diob_Config2_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut2
    AW_Config1_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut3
    AW_Config2_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut4
    Clr_Tag_Config:        out  std_logic;                        -- Clear Tag-Konfigurations-Register
    
    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    LA:                   out  std_logic_vector(15 downto 0)
      );
end component config_status;


component aw_io_reg
  generic ( AW_Base_addr:   integer;
            CLK_sys_in_Hz:  integer);
  port (
        Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
        Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
        Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
        Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
        Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
        Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
        Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
        clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
        Ena_every_1us:        in   std_logic;                        -- Clock-Enable-Puls alle Mikrosekunde, 1 Clock breit
        nReset:               in   std_logic;
    
        SCU_AW_Input_Reg:     in   t_IO_Reg_1_to_7_Array;            -- Input-Port's  zum SCU-Bus
        SCU_AW_Output_Reg:    out  t_IO_Reg_1_to_7_Array;            -- Output-Port's vom SCU-Bus

        AWOut_Reg1_wr:        out  std_logic;                        -- Daten-Reg. AWOut1
        AWOut_Reg2_wr:        out  std_logic;                        -- Daten-Reg. AWOut2
        AWOut_Reg3_wr:        out  std_logic;                        -- Daten-Reg. AWOut3
        AWOut_Reg4_wr:        out  std_logic;                        -- Daten-Reg. AWOut4
        AWOut_Reg5_wr:        out  std_logic;                        -- Daten-Reg. AWOut5
        AWOut_Reg6_wr:        out  std_logic;                        -- Daten-Reg. AWOut6
        AWOut_Reg7_wr:        out  std_logic;                        -- Daten-Reg. AWOut7
      
        Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
        Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
        Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
        LA:                   out  std_logic_vector(15 downto 0)
      );
end component aw_io_reg;



component tag_ctrl
  generic ( TAG_Base_addr  : integer );
  port (
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    Timing_Pattern_LA:    in   std_logic_vector(31 downto 0);    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:   in   std_logic;                        -- timing pattern received
    Spare0:               in   std_logic;                        -- vom Master getrieben
    Spare1:               in   std_logic;                        -- vom Master getrieben
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    SCU_AW_Input_Reg:         in   t_IO_Reg_1_to_7_Array;            -- Input-Port's  zum SCU-Bus

    Clr_Tag_Config:        in   std_logic;                        -- clear alle Tag-Masken
    Max_AWOut_Reg_Nr:     in   integer range 0 to 7;             -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:      in   integer range 0 to 7;             -- Maximale AWIn-Reg-Nummer der Anwendung
   
    Tag_Maske_Reg:        out  t_IO_Reg_1_to_7_Array;            -- Tag-Output-Maske für Register 1-7
    Tag_Outp_Reg:         out  t_IO_Reg_1_to_7_Array;            -- Tag-Output-Maske für Register 1-7
       
    Tag_FG_Start:         out  std_logic;                        -- Start-Puls für den FG
    Tag_Sts:              out  std_logic_vector(15 downto 0);    -- Tag-Status
   
    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    Tag_Aktiv:            out  std_logic_vector( 7 downto 0);    -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)  
    LA_tag_ctrl:          out  std_logic_vector(15 downto 0)
    );  
end component tag_ctrl;



COMPONENT io_reg
  GENERIC ( Base_addr : INTEGER );
  PORT
  (
    Adr_from_SCUB_LA:    IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Data_from_SCUB_LA:   IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Ext_Adr_Val:         IN  STD_LOGIC;
    Ext_Rd_active:       IN  STD_LOGIC;
    Ext_Rd_fin:          IN  STD_LOGIC;
    Ext_Wr_active:       IN  STD_LOGIC;
    Ext_Wr_fin:          IN  STD_LOGIC;
    clk:                 IN  STD_LOGIC;
    nReset:              IN  STD_LOGIC;
    Reg_IO1:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO2:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO3:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO4:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO5:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO6:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO7:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO8:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_rd_active:       OUT STD_LOGIC;
    Data_to_SCUB:        OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Dtack_to_SCUB:       OUT STD_LOGIC
  );
END COMPONENT io_reg;

component pu_reset
generic (
    PU_Reset_in_clks : integer
    );
port  (
    Clk:      in    std_logic;
    PU_Res:   out   std_logic
    );
end component;



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


COMPONENT addac_reg
 GENERIC ( Base_addr : INTEGER);
 PORT
 (
  Adr_from_SCUB_LA:   IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  Data_from_SCUB_LA:  IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  Ext_Adr_Val:        IN    STD_LOGIC;
  Ext_Rd_active:      IN    STD_LOGIC;
  Ext_Rd_fin:         IN    STD_LOGIC;
  Ext_Wr_active:      IN    STD_LOGIC;
  Ext_Wr_fin:         IN    STD_LOGIC;
  clk:                IN    STD_LOGIC;
  nReset:             IN    STD_LOGIC;
  DAC1_Config:        OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  DAC1_Config_wr:     OUT   STD_LOGIC;
  DAC1_Out:           OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  DAC1_Out_wr:        OUT   STD_LOGIC;
  DAC2_Config:        OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  DAC2_Config_wr:     OUT   STD_LOGIC;
  DAC2_Out:           OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  DAC2_Out_wr:        OUT   STD_LOGIC;
  ADC_Config:         OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In1:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In2:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In3:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In4:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In5:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In6:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In7:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_In8:            IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
  Rd_active:          OUT   STD_LOGIC;
  Data_to_SCUB:       OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
  Dtack_to_SCUB:      OUT   STD_LOGIC;
  LA:                 OUT   STD_LOGIC_VECTOR(15 DOWNTO 0)
 );
END COMPONENT;


component IO_4x8
  generic (
    Base_addr: unsigned(15 downto 0));
  port (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';
    io:                 inout std_logic_vector(31 downto 0);  -- select and set direction only in 8-bit partitions
    io_7_0_tx:          out   std_logic;                      -- '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis:     out   std_logic;                      -- '1' = disable external io(7..0)-buffer.
    io_15_8_tx:         out   std_logic;                      -- '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis:    out   std_logic;                      -- '1' = disable external io(15..8)-buffer.
    io_23_16_tx:        out   std_logic;                      -- '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis:   out   std_logic;                      -- '1' = disable external io(23..16)-buffer.
    io_31_24_tx:        out   std_logic;                      -- '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis:   out   std_logic;                      -- '1' = disable external io(31..24)-buffer.
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
    );
  end component IO_4x8;


 component diob_debounce
  generic (
    DB_Tst_Cnt: integer := 3;
    Test:       integer range 0 TO 1);
  port (
    DB_Cnt:     in  integer range 0 to 16383;    
    DB_In:      in  std_logic;
    Reset:      in  std_logic;
    Clk:        in  std_logic;
    DB_Out:     out std_logic
    );
  end component diob_debounce;
  
  
 component diob_sync
  port (
    Sync_In:    in  std_logic;
    Reset:      in  std_logic;
    Clk:        in  std_logic;
    Sync_Out:   out std_logic
    );
  end component diob_sync;
  
  
  
 COMPONENT outpuls
 PORT
 (
  nReset      :  IN STD_LOGIC;
  clk         :  IN STD_LOGIC;
  Start       :  IN STD_LOGIC;
  cnt_ena     :  IN STD_LOGIC;              -- Enable für die Basis_Verzögerungszeit
  Base_cnt    :  IN INTEGER RANGE 0 TO 15;
  Mult_cnt    :  IN INTEGER RANGE 0 TO 65535;
  Sign_Out    :  OUT STD_LOGIC
 );
END COMPONENT;
  
  

--  +============================================================================================================================+
--  |                                                         signal                                                             |
--  +============================================================================================================================+

  signal clk_sys, clk_cal, locked : std_logic;

  --  signal clk:                       std_logic := '0';

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
   

--------------------------- Conf_Sts1 ----------------------------------------------------------------------
    
  signal DIOB_Config1:          std_logic_vector(15 downto 0);
  signal DIOB_Config2:          std_logic_vector(15 downto 0);
  signal DIOB_Status1:          std_logic_vector(15 downto 0);
  signal DIOB_Status2:          std_logic_vector(15 downto 0);
  signal AW_Config1:            std_logic_vector(15 downto 0);
  signal AW_Config2:            std_logic_vector(15 downto 0);
  signal AW_Status1:            std_logic_vector(15 downto 0);
  signal AW_Status2:            std_logic_vector(15 downto 0);

  signal Diob_Config1_wr:       std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config1
  signal Diob_Config2_wr:       std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config2
  signal AW_Config1_wr:         std_logic;                        -- write-Strobe, Daten-Reg. AW_Config1  
  signal AW_Config2_wr:         std_logic;                        -- write-Strobe, Daten-Reg. AW_Config2  
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

  signal AWOut_Reg1_Wr:         std_logic;
  signal AWOut_Reg2_Wr:         std_logic;
  signal AWOut_Reg3_Wr:         std_logic;
  signal AWOut_Reg4_Wr:         std_logic;
  signal AWOut_Reg5_Wr:         std_logic;
  signal AWOut_Reg6_Wr:         std_logic;
  signal AWOut_Reg7_Wr:         std_logic;
  
  signal AW_Port1_rd_active:    std_logic;
  signal AW_Port1_Dtack:        std_logic;
  signal AW_Port1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal Tag_Reg_Conf_Err:      std_logic;
  signal LA_AW_Port1:           std_logic_vector(15 downto 0);

--------------------------- Ctrl1 ----------------------------------------------------------------------
  
  signal Tag_Maske_Reg:          t_IO_Reg_1_to_7_Array;           -- Tag-Output-Maske für Register 1-7
  signal Tag_Outp_Reg:           t_IO_Reg_1_to_7_Array;           -- Tag-Output-Maske für Register 1-7

  signal Tag_FG_Start:           std_logic;                       -- Start-Puls für den FG
  signal Tag_Sts:                std_logic_vector(15 downto 0);   -- Tag-Status

  signal Tag_Ctrl1_rd_active:    std_logic;                       -- read data available at 'Data_to_SCUB'-Tag_Ctrl1
  signal Tag_Ctrl1_Dtack:        std_logic;                       -- connect read sources to SCUB-Macro         
  signal Tag_Ctrl1_data_to_SCUB: std_logic_vector(15 downto 0);   -- connect Dtack to SCUB-Macro                
  signal Tag_Aktiv:              std_logic_vector( 7 downto 0);   -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)  
  signal LA_Tag_Ctrl1:           std_logic_vector(15 downto 0);  
  

  --------------------------- INL_msk ----------------------------------------------------------------------
  
  signal INL_msk_IO1:           std_logic_vector(15 downto 0);         
  signal INL_msk_IO2:           std_logic_vector(15 downto 0);         
  signal INL_msk_IO3:           std_logic_vector(15 downto 0);       
  signal INL_msk_IO4:           std_logic_vector(15 downto 0);         
  signal INL_msk_IO5:           std_logic_vector(15 downto 0);         
  signal INL_msk_IO6:           std_logic_vector(15 downto 0);         
  signal INL_msk_IO7:           std_logic_vector(15 downto 0);         
  signal INL_msk1_rd_active:    std_logic;   
  signal INL_msk1_Dtack:        std_logic;      
  signal INL_msk1_data_to_SCUB: std_logic_vector(15 downto 0);


  --------------------------- INL_xor ----------------------------------------------------------------------
  
  signal INL_xor_IO1:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO2:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO3:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO4:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO5:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO6:           std_logic_vector(15 downto 0);        
  signal INL_xor_IO7:           std_logic_vector(15 downto 0);        
  signal INL_xor1_rd_active:    std_logic;   
  signal INL_xor1_Dtack:        std_logic;      
  signal INL_xor1_data_to_SCUB: std_logic_vector(15 downto 0);
       

  --------------------------- INL_xor ----------------------------------------------------------------------
       
  signal INL_IO1:   std_logic_vector(15 downto 0);        
  signal INL_IO2:   std_logic_vector(15 downto 0);        
  signal INL_IO3:   std_logic_vector(15 downto 0);        
  signal INL_IO4:   std_logic_vector(15 downto 0);        
  signal INL_IO5:   std_logic_vector(15 downto 0);        
  signal INL_IO6:   std_logic_vector(15 downto 0);        
  signal INL_IO7:   std_logic_vector(15 downto 0);        
  signal Interlock: std_logic;        

  signal hp_la_o:      std_logic_vector(15 downto 0); -- Output für HP-Logicanalysator
   
  signal s_nLED_User1_i: std_logic;  -- LED3 = User 1 
  signal s_nLED_User2_i: std_logic;  -- LED2 = User 2 
  signal s_nLED_User3_i: std_logic;  -- LED1 = User 3 
  signal s_nLED_User1_o: std_logic;  -- LED3 = User 1 
  signal s_nLED_User2_o: std_logic;  -- LED2 = User 2 
  signal s_nLED_User3_o: std_logic;  -- LED1 = User 3 

  signal uart_txd_out:  std_logic;

  
  --------------------------- ADDAC ----------------------------------------------------------------------

  signal DAC1_Config:         STD_LOGIC_VECTOR(15 DOWNTO 0);  -- DAC  Config-Register
  signal DAC1_Config_wr:      STD_LOGIC;                      -- write DAC1 Config-Register
  signal DAC1_Out:            STD_LOGIC_VECTOR(15 DOWNTO 0);  -- DAC1 Output-Register
  signal DAC1_Out_wr:         STD_LOGIC;                      -- write DAC1 Output-Register
  signal DAC2_Config:         STD_LOGIC_VECTOR(15 DOWNTO 0);  -- DAC  Config-Register
  signal DAC2_Config_wr:      STD_LOGIC;                      -- write DAC2 Config-Register
  signal DAC2_Out:            STD_LOGIC_VECTOR(15 DOWNTO 0);  -- DAC1 Output-Register
  signal DAC2_Out_wr:         STD_LOGIC;                      -- write DAC2 Output-Register
  signal ADC_Config:          STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC  Config-Register
  signal ADC_In1:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC1 Input-Register
  signal ADC_In2:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC2 Input-Register 
  signal ADC_In3:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC3 Input-Register 
  signal ADC_In4:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC4 Input-Register 
  signal ADC_In5:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC5 Input-Register
  signal ADC_In6:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC6 Input-Register
  signal ADC_In7:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC7 Input-Register
  signal ADC_In8:             STD_LOGIC_VECTOR(15 DOWNTO 0);  -- ADC8 Input-Register
  signal addac_rd_active:     STD_LOGIC;                      -- read data available at 'Data_to_SCUB'-AWOut
  signal addac_Data_to_SCUB:  STD_LOGIC_VECTOR(15 DOWNTO 0);  -- connect read sources to SCUB-Macro
  signal addac_Dtack:         STD_LOGIC;                      -- connect Dtack to SCUB-Macro
  signal LA_addac:            STD_LOGIC_VECTOR(15 DOWNTO 0);



    ------------ IO-Port-Signale --------------------------------------------------------------------------------------

  signal s_io_7_0_tx:           std_logic;                    -- '1' = external io(7..0)-buffer set to output.
  signal s_io_15_8_tx:          std_logic;                    -- '1' = external io(15..8)-buffer set to output
  signal s_io_23_16_tx:         std_logic;                    -- '1' = external io(23..16)-buffer set to output
  signal s_io_31_24_tx:         std_logic;                    -- '1' = external io(31..24)-buffer set to output
  signal s_ext_io_7_0_dis:      std_logic;                    -- '1' = disable external io(7..0)-buffer.
  signal s_ext_io_15_8_dis:     std_logic;                    -- '1' = disable external io(15..8)-buffer.
  signal s_ext_io_23_16_dis:    std_logic;                    -- '1' = disable external io(23..16)-buffer.
  signal s_ext_io_31_24_dis:    std_logic;                    -- '1' = disable external io(31..24)-buffer.
  signal s_io:                  std_logic_vector(31 downto 0);-- select and set direction only in 8-bit partitions
  signal io_port_rd_active:     std_logic;   
  signal io_port_Dtack:         std_logic;      
  signal io_port_data_to_SCUB:  std_logic_vector(15 downto 0);



    ------------ Mirror-Mode-Signale --------------------------------------------------------------------------------------

  signal AWIn_Reg_Array:        t_IO_Reg_1_to_7_Array;          -- Copy der AWIn-Register in ein Array

  signal Mirr_OutReg_Maske:     std_logic_vector(15 downto 0);  -- Maskierung für Spiegel-Modus des Ausgangsregisters
  signal Mirr_AWOut_Reg_Nr:     integer range 0 to 7;           -- AWOut-Reg-Nummer

  signal Mirr_AWIn_Reg_Nr:      integer range 0 to 7;           -- AWIn-Reg-Nummer
  
  
  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: P37IO  -- FG900_700                                    |
--  +============================================================================================================================+

  signal P37IO_Start_deb_i:    std_logic;    -- input "Start" L-Aktiv
  signal P37IO_Start_deb_o:    std_logic;    -- input "Start" entprellt
  signal P37IO_nLED_Start_o:   std_logic;    -- Output "nLED_Start"
  signal P37IO_Stop_deb_i:     std_logic;    -- input "Stop" L-Aktiv
  signal P37IO_Stop_deb_o:     std_logic;    -- input "Stop" entprellt
  signal P37IO_nLED_Stop_o:    std_logic;    -- Output "nLED_Stop"
  signal P37IO_Reset_deb_i:    std_logic;    -- input "Reset" L-Aktiv
  signal P37IO_Reset_deb_o:    std_logic;    -- input "Rest" entprellt
  signal P37IO_Start_syn_i:    std_logic;    -- input "Start" L-Aktiv
  signal P37IO_Start_syn_o:    std_logic;    -- input "Start" entprellt
  signal P37IO_Stop_syn_i:     std_logic;    -- input "Stop" L-Aktiv
  signal P37IO_Stop_syn_o:     std_logic;    -- input "Stop" entprellt
  signal P37IO_Reset_syn_i:    std_logic;    -- input "Reset" L-Aktiv
  signal P37IO_Reset_syn_o:    std_logic;    -- input "Rest" entprellt
  signal P37IO_BNC_o:          std_logic;    -- Output "BNC"
  signal P37IO_nELD_BNC_o:     std_logic;    -- Output "nLED_BNC"

  signal P37IO_FF_Start:       std_logic;    -- FF-Input
  signal P37IO_FF_Stop:        std_logic;    -- FF-Input
  signal P37IO_FF_Reset:       std_logic;    -- FF-Input
  
  signal P37IO_in_Data:           std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal P37IO_Deb_in_Data_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal P37IO_Deb_in_Data_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal P37IO_Syn_in_Data_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal P37IO_Syn_in_Data_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler

  
  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: P25IO  -- FG900_710                                    |
--  +============================================================================================================================+

  signal P25IO_Start_deb_i:         std_logic;        -- input "Start" L-Aktiv
  signal P25IO_Start_deb_o:         std_logic;        -- input "Start" entprellt
  signal P25IO_Start_syn_i:         std_logic;        -- input "Start" L-Aktiv
  signal P25IO_Start_syn_o:         std_logic;        -- input "Start" entprellt
  signal P25IO_nLED_Start_o:        std_logic;        -- Output "nLED_Start"
  signal P25IO_Stop_deb_i:          std_logic;        -- input "Stop" L-Aktiv
  signal P25IO_Stop_deb_o:          std_logic;        -- input "Stop" entprellt
  signal P25IO_Stop_syn_i:          std_logic;        -- input "Stop" L-Aktiv
  signal P25IO_Stop_syn_o:          std_logic;        -- input "Stop" entprellt
  signal P25IO_nLED_Stop_o:         std_logic;        -- Output "nLED_Stop"
  signal P25IO_Reset_deb_i:         std_logic;        -- input "Reset" L-Aktiv
  signal P25IO_Reset_deb_o:         std_logic;        -- input "Rest" entprellt
  signal P25IO_Reset_syn_i:         std_logic;        -- input "Reset" L-Aktiv
  signal P25IO_Reset_syn_o:         std_logic;        -- input "Rest" entprellt
  signal P25IO_BNC_o:               std_logic;        -- Output "BNC"
  signal P25IO_nELD_BNC_o:          std_logic;        -- Output "nLED_BNC"

  signal P25IO_FF_Start:            std_logic;        -- FF-Input
  signal P25IO_FF_Stop:             std_logic;        -- FF-Input
  signal P25IO_FF_Reset:            std_logic;        -- FF-Input


  signal P25IO_DAC_Mode:             std_logic_vector( 1 downto 0); -- Output-Betriebsart
  signal P25IO_DAC_Out:              std_logic_vector(15 downto 0); -- Data_Output
  signal P25IO_DAC_Data_FG_Out:      std_logic_vector(15 downto 0); -- Data/FG-Output

  signal P25IO_DAC_DAC_Strobe_i:     std_logic;                     -- Input  "DAC-Strobe"
  signal P25IO_DAC_DAC_Strobe_o:     std_logic;                     -- Output "DAC-Strobe"
  signal P25IO_DAC_DAC_Strobe_Expo:  integer range 0 to 7;          -- Anzahl der Counts
  signal P25IO_DAC_Strobe:           std_logic;                     -- Output "DAC-Strobe"
  
  type   P25IO_Holec_state_t is   (holec_idle, holec_puls,  holec_puls_w,  holec_puls_e, holec_pause, holec_pause_w, holec_end);
  signal P25IO_Holec_state:       P25IO_Holec_state_t:= holec_idle;

  signal P25IO_DAC_Str_Puls_i:      std_logic;                  -- 
  signal P25IO_DAC_Str_Puls_o:      std_logic;                  -- 
  signal P25IO_DAC_Str_Pause_i:     std_logic;                  -- 
  signal P25IO_DAC_Str_Pause_o:     std_logic;                  -- 
  signal P25IO_Holec_Str_Cnt:       integer range 0 to 7;          -- Anzahl der Counts
  
  signal P25IO_Holec_Strobe_Start:  std_logic;                  -- Start Holec-Strobe-Sequence
  signal P25IO_Holec_Strobe_Out:    std_logic;                  -- Output, Holec-Strobe-Sequence
  
  signal P25IO_In:                std_logic_vector(15 downto 0); -- Data_Input
  signal P25IO_Strobe_in:         std_logic;                     -- Input  "Strobe"
  
  signal P25IO_LED_Lemo_Out_i:    std_logic;                     -- Input  "nLED_Lemo_In"
  signal P25IO_nLED_Lemo_Out_o:   std_logic;                     -- Output "nLED_Lemo_In"
                      
  signal P25IO_ADC_In:            std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Deb_In_i:      std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Deb_In_o:      std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Syn_In_i:      std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Syn_In_O:      std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Input:         std_logic_vector(15  downto 0);  -- Input Daten
  signal P25IO_ADC_Strobe:        std_logic;                       -- Input Strobe
  signal P25IO_ADC_Strobe_in:     std_logic;                       -- Input Strobe
  
  signal P25IO_ADC_Strobe_i:      std_logic;                       -- input  "Strobe-Signal für den ADC"
  signal P25IO_ADC_Strobe_o:      std_logic;                       -- Output "Strobe-Signal für den ADC (1 CLK breit)"
  signal P25IO_ADC_shift:         std_logic_vector(2  downto 0);   -- Shift-Reg.

  signal P25IO_ADC_Data_FF_i:     std_logic_vector(15  downto 0);  -- input  "Daten ADC-Register"
  signal P25IO_ADC_Data_FF_o:     std_logic_vector(15  downto 0);  -- Output "Daten ADC-Register"
     
  signal P25IO_Ext_Tim_deb_i:     std_logic;        -- input "Start" L-Aktiv
  signal P25IO_Ext_Tim_deb_o:     std_logic;        -- input "Start" entprellt
  signal P25IO_Ext_Tim_syn_i:     std_logic;        -- input "Start" L-Aktiv
  signal P25IO_Ext_Tim_syn_o:     std_logic;        -- input "Start" entprellt
  signal P25IO_nLED_Ext_Tim_o:    std_logic;        --  Output "nLED_Start"
  
  signal P25IO_ECC_Puls_i:        std_logic;                       -- input  "ECC-Signal für den ADC"
  signal P25IO_ECC_Puls_o:        std_logic;                       -- Output "ECC-Signal für den ADC (1 CLK breit)"
  signal P25IO_ECC_Puls_shift:    std_logic_vector(2  downto 0);   -- Shift-Reg.
  
  signal P25IO_ECC_Strobe_i:     std_logic;                     -- Input  "ECC-Strobe"
  signal P25IO_ECC_Strobe_o:     std_logic;                     -- Output "ECC-Strobe"
  signal P25IO_ECC_Strobe_Expo:  integer range 0 to 7;          -- Anzahl der Counts
  
  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: OCIN   -- FG900_720                                    |
--  +============================================================================================================================+


  signal OCIN_Data1_in:          std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Deb_Data1_in_i:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Deb_Data1_in_o:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Syn_Data1_in_i:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Syn_Data1_in_o:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Data2_in:          std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Deb_Data2_in_i:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Deb_Data2_in_o:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Syn_Data2_in_i:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIN_Syn_Data2_in_o:    std_logic_vector(15 downto 0); -- Data_Input über Optokoppler


--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: OCIO   -- FG900_730                                    |
--  +============================================================================================================================+


  signal OCIO_Data1_in:           std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Deb_Data1_in_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Deb_Data1_in_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Syn_Data1_in_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Syn_Data1_in_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Data2_in:           std_logic_vector(15 downto 0); -- entprellter Data_Input
  signal OCIO_Deb_Data2_in_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Deb_Data2_in_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Syn_Data2_in_i:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler
  signal OCIO_Syn_Data2_in_o:     std_logic_vector(15 downto 0); -- Data_Input über Optokoppler

  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: OCIO   -- FG900_740                                    |
--  +============================================================================================================================+


  signal UIO_Mode:                std_logic_vector( 1 downto 0); -- Output-Betriebsart
  signal UIO_Out:                 std_logic_vector(23 downto 0); -- Data_Output
  signal UIO_Data_FG_Out:         std_logic_vector(23 downto 0); -- Data/FG-Output
  signal UIO_HS_In:               std_logic_vector(23 downto 0); -- Input auf GND 
  signal UIO_LS_In:               std_logic_vector(23 downto 0); -- Input Uext
  signal UIO_Deb_In_i:            std_logic_vector(23 downto 0); -- 
  signal UIO_Deb_In_o:            std_logic_vector(23 downto 0); -- 
  signal UIO_Syn_In_i:            std_logic_vector(23 downto 0); -- 
  signal UIO_Syn_In_o:            std_logic_vector(23 downto 0); -- 
  
  signal UIO_LED_Lemo_In_i:       std_logic;  --  Input  "nLED_Lemo_In"
  signal UIO_nLED_Lemo_In_o:      std_logic;  --  Output "nLED_Lemo_In"
  signal UIO_LED_Lemo_Out_i:      std_logic;  --  Input  "nLED_Lemo_Out"
  signal UIO_nLED_Lemo_Out_o:     std_logic;  --  Output "nLED_Lemo_Out"
  
  signal UIO_DAC_Strobe_i:        std_logic;                      -- Input  "DAC-Strobe"
  signal UIO_DAC_Strobe_o:        std_logic;                      -- Output "DAC-Strobe"
  signal UIO_DAC_Strobe_Expo:     integer range 0 to 7;           -- Anzahl der Counts
  signal UIO_Lemo_in:             std_logic;                      -- Input "Lemo" 
  
  signal UIO_Lemo_deb_i:          std_logic;  -- Input  "UIO_Lemo_in"
  signal UIO_Lemo_deb_o:          std_logic;  -- Output "UIO_Lemo_in"
  signal UIO_Lemo_syn_i:          std_logic;  -- Input  "UIO_Lemo_in"
  signal UIO_Lemo_syn_o:          std_logic;  -- Output "UIO_Lemo_in"

  signal UIO_Reg_Enable:          std_logic;                     -- Input   Speicher-Strobe"
  signal UIO_Reg_Out_i:           std_logic_vector(23 downto 0); -- Input:  Output-Register
  signal UIO_Reg_Out_o:           std_logic_vector(23 downto 0); -- Output: Output-Register
  
  signal UIO_in_Data:             std_logic_vector(23 downto 0); -- Data_Input über Optokoppler
  
  

--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: DA    -- FG900_750                                     |
--  +============================================================================================================================+


  signal DA_DAC1_Data:            std_logic_vector(15 downto 0); -- Zwischenspeicher
  signal DA_DAC1_Str:             std_logic;                     -- DAC1-Strobe
  signal DA_DAC1_Out:             std_logic_vector(15 downto 0); -- Zwischenspeicher
  signal DA_DAC1_Str_Out:         std_logic;                     -- DAC1-Output-Strobe

  signal DA_DAC2_Data:            std_logic_vector(15 downto 0); -- Zwischenspeicher
  signal DA_DAC2_Str:             std_logic;                     -- DAC2-Strobe
  signal DA_DAC2_Out:             std_logic_vector(15 downto 0); -- Zwischenspeicher
  signal DA_DAC2_Str_Out:         std_logic;                     -- DAC2-Output-Strobe

  signal DAC_Test_Out:            std_logic_vector(15 downto 0); -- Test-Bitmuster
  signal DAC_Test_Strobe:         std_logic;                     -- Output Test-Strobe
  signal DAC_Wait_cnt:            integer range 0 to 65535;      -- 0-FFFF -- Wait-Counter
 
  type   dac_state_t is   (dac_idle, dac_data1, dac_str1h, dac_str1l, dac_loop1, dac_wait1, dac_data2, dac_str2h, dac_str2l, dac_loop2);
  signal dac_state:        dac_state_t := dac_idle;

  
  signal DAC_tr_Test_Out:         std_logic_vector(15 downto 0); -- Test-Bitmuster
  signal DAC_tr_Test_Strobe:      std_logic;                     -- Output Test-Strobe
  signal DAC_tr_Wait_cnt:         integer range 0 to 65535;      -- 0-FFFF -- Wait-Counter
  signal DAC_tr_Test_Loop_cnt:    integer range 0 to 32;         -- 0-32   -- Loop-Counter
 
  TYPE   t_word_array     is array (0 to 32) of std_logic_vector(15 downto 0);
  signal DAC_tr_Array:    t_word_array;                                       --  DAC-Input  "Test-Treppen-Mode"

  type   dac_tr_state_t is   (dac_tr_idle, dac_tr_data, dac_tr_strh, dac_tr_strl, dac_tr_loop1, dac_tr_end);
  signal dac_tr_state:        dac_tr_state_t := dac_tr_idle;


  
  signal DA_LED_Ext_Trig1_i:       std_logic;      --  Input  "nLED_Ext_Trig1"
  signal DA_LED_Ext_Trig1_o:       std_logic;      --  Output "nLED_Ext_Trig1"
  signal DA_LED_Ext_Trig2_i:       std_logic;      --  Input  "nLED_Ext_Trig2"
  signal DA_LED_Ext_Trig2_o:       std_logic;      --  Output "nLED_Ext_Trig2"
       
  signal DA_LED_Trig_Out1_i:       std_logic;      --  Input  "nLED_Trig_Out1"
  signal DA_LED_Trig_Out1_o:       std_logic;      --  Output "nLED_Trig_Out1"
  signal DA_LED_Trig_Out2_i:       std_logic;      --  Input  "nLED_Trig_Out2"
  signal DA_LED_Trig_Out2_o:       std_logic;      --  Output "nLED_Trig_Out2"
     
  signal DA_Trig1_i:               std_logic;      -- Input  "DAC-Strobe1"
  signal DA_Trig1_1us_o:           std_logic;      -- Output "Trig1_1µs" breit
  signal DA_Trig2_i:               std_logic;      -- Input  "DAC-Strobe2"
  signal DA_Trig2_1us_o:           std_logic;      -- Output "Trig2_1µs" breit

  signal DA_Trig1_deb_i:           std_logic;      -- input "Trigger1"
  signal DA_Trig1_deb_o:           std_logic;      -- input "Trigger1" entprellt
  signal DA_Trig2_deb_i:           std_logic;      -- input "Trigger2"
  signal DA_Trig2_deb_o:           std_logic;      -- input "Trigger2" entprellt
  signal DA_Trig1_syn_i:           std_logic;      -- input "Trigger1"
  signal DA_Trig1_syn_o:           std_logic;      -- input "Trigger1" entprellt
  signal DA_Trig2_syn_i:           std_logic;      -- input "Trigger2"
  signal DA_Trig2_syn_o:           std_logic;      -- input "Trigger2" entprellt
   
  signal DA_Trig1_Strobe_i:        std_logic;                      -- input  "Start-Signal für ext. Trigger 1"
  signal DA_Trig1_Strobe_o:        std_logic;                      -- Output "Start-Puls   für ext. Trigger 1 (1 CLK breit)"
  signal DA_Trig1_shift:           std_logic_vector(2  downto 0);  -- Shift-Reg.
   
  signal DA_Trig2_Strobe_i:        std_logic;                      -- input  "Start-Signal für ext. Trigger 2"
  signal DA_Trig2_Strobe_o:        std_logic;                      -- Output "Start-Puls   für ext. Trigger 2 (1 CLK breit)"
  signal DA_Trig2_shift:           std_logic_vector(2  downto 0);  -- Shift-Reg.

  signal DA_DAC1_Str_Puls_i:       std_logic;                      -- input  "Start-Signal für DAC1_Reset"
  signal DA_DAC1_Str_Puls_o:       std_logic;                      -- Output "Start-Puls   für DAC1_Reset (1 CLK breit)"
  signal DA_DAC1_Str_Puls_shift:   std_logic_vector(2  downto 0);  -- Shift-Reg.
  
  signal DA_DAC2_Str_Puls_i:       std_logic;                      -- input  "Start-Signal für DAC2_Reset"
  signal DA_DAC2_Str_Puls_o:       std_logic;                      -- Output "Start-Puls   für DAC2_Reset (1 CLK breit)"
  signal DA_DAC2_Str_Puls_shift:   std_logic_vector(2  downto 0);  -- Shift-Reg.
   
  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: SPSIO  -- FG900.770                                    |
--  +============================================================================================================================+

  signal SPSIO_Data_in:             std_logic_vector(23 downto 0); -- Data_Input über Optokoppler
  signal SPSIO_Deb_Data_in_i:       std_logic_vector(23 downto 0);
  signal SPSIO_Deb_Data_in_o:       std_logic_vector(23 downto 0);
  signal SPSIO_Syn_Data_in_i:       std_logic_vector(23 downto 0);
  signal SPSIO_Syn_Data_in_o:       std_logic_vector(23 downto 0);


--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: HFIO  -- FG900.780                                     |
--  +============================================================================================================================+

  signal HFIO_Tastpuls_i:                 std_logic;  --  Input  "nLED_Tastpuls"
  signal HFIO_nLED_Tastpuls_o:            std_logic;  --  Output "nLED_Tastpuls"
  signal HFIO_Sample_Puls_inv_i:          std_logic;  --  Input  "nLED_Sample_Puls_inv"
  signal HFIO_nLED_Sample_Puls_inv_o:     std_logic;  --  Output "nLED_Sample_Puls_inv"
  signal HFIO_Aux_i:                      std_logic;  --  Input  "nLED_Aux"
  signal HFIO_nLED_Aux_o:                 std_logic;  --  Output "nLED_Aux"
  signal HFIO_Sample_Puls_Display_i:      std_logic;  --  Input  "nLED_Sample_Puls_Display"
  signal HFIO_nLED_Sample_Puls_Display_o: std_logic;  --  Output "nLED_Sample_Puls_Display"

  signal HFIO_in_AMP_FEHLER_Deb_i:        std_logic;  -- Input  "AMP_FEHLER"
  signal HFIO_in_AMP_FEHLER_Deb_o:        std_logic;  -- Output "AMP_FEHLER"
  signal HFIO_in_AMP_FEHLER_syn_i:        std_logic;  -- Input  "AMP_FEHLER"
  signal HFIO_in_AMP_FEHLER_syn_o:        std_logic;  -- Output "AMP_FEHLER"
  signal HFIO_in_PHASE_FEHLER_Deb_i:      std_logic;  -- Input  "PHASE_FEHLER"
  signal HFIO_in_PHASE_FEHLER_Deb_o:      std_logic;  -- Output  PHASE_FEHLER"
  signal HFIO_in_PHASE_FEHLER_syn_i:      std_logic;  -- Input  "PHASE_FEHLER"
  signal HFIO_in_PHASE_FEHLER_syn_o:      std_logic;  -- Output  PHASE_FEHLER"
  
  
--  +============================================================================================================================+
--  |    §§§              Übergabe-Signale für Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen                 |
--  +============================================================================================================================+


  signal IOBP_Output: std_logic_vector(12 downto 1); -- Data_Output "Slave-Karten 1-12"
  
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

  signal Slave_Loop_cnt:      integer range 0 to 12;         -- 1-12   -- Loop-Counter
  
  type   IOBP_LED_state_t is   (IOBP_idle, led_id_wait, led_id_loop, led_str_rot_h, led_str_rot_l, led_gruen,
                                led_str_gruen_h, led_str_gruen_l, iobp_led_dis, iobp_led_z, iobp_id_str_h, iobp_rd_id, iobp_id_str_l, iobp_end);
  signal IOBP_state:   IOBP_LED_state_t:= IOBP_idle;
 
  
  --------------------------- IO-Select ----------------------------------------------------------------------
  
  signal IOBP_Masken_Reg1:        std_logic_vector(15 downto 0);         
  signal IOBP_Masken_Reg2:        std_logic_vector(15 downto 0);         
  signal IOBP_Masken_Reg3:        std_logic_vector(15 downto 0);       
  signal IOBP_Masken_Reg4:        std_logic_vector(15 downto 0);         
  signal IOBP_Masken_Reg5:        std_logic_vector(15 downto 0);         
  signal IOBP_sel1_rd_active:     std_logic;   
  signal IOBP_sel1_Dtack:         std_logic;      
  signal IOBP_sel1_data_to_SCUB:  std_logic_vector(15 downto 0);

  
--  +============================================================================================================================+

  signal    Deb60_Cnt:    integer range 0 to 16383;  
  signal    Deb60_in:     std_logic_vector(59 downto 0);
  signal    Deb60_out:    std_logic_vector(59 downto 0);

  signal    Syn60_in:     std_logic_vector(59 downto 0);  
  signal    Syn60_out:    std_logic_vector(59 downto 0);  
  
  signal    Deb_Sync60:   std_logic_vector(59 downto 0);  
  
  
  
--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: Out16   -- FG901_010                                   |
--  +============================================================================================================================+


  signal Out16_Mode:             std_logic_vector( 1 downto 0); -- Output-Betriebsart
  signal Out16_Out:              std_logic_vector(15 downto 0); -- Data_Output
  signal Out16_Data_FG_Out:      std_logic_vector(15 downto 0); -- Data/FG-Output

  signal Out16_DAC_Strobe_i:     std_logic;                      -- Input  "DAC-Strobe"
  signal Out16_DAC_Strobe_o:     std_logic;                      -- Output "DAC-Strobe"
  signal Out16_DAC_Strobe_Expo:  integer range 0 to 7;           -- Anzahl der Counts
  signal Out16_Strobe:           std_logic;                      -- Output "DAC-Strobe"

  signal Out16_Lemo_in:          std_logic;                      -- Input "Lemo" 

  signal Out16_LED_Lemo_In_i:    std_logic;  --  Input  "nLED_Lemo_In"
  signal Out16_nLED_Lemo_In_o:   std_logic;  --  Output "nLED_Lemo_In"
  
  signal Out16_Lemo_deb_i:       std_logic;  -- Input  "UIO_Lemo_in"
  signal Out16_Lemo_deb_o:       std_logic;  -- Output "UIO_Lemo_in"
  signal Out16_Lemo_syn_i:       std_logic;  -- Input  "UIO_Lemo_in"
  signal Out16_Lemo_syn_o:       std_logic;  -- Output "UIO_Lemo_in"
  
  

--  +============================================================================================================================+
--  |                                   Übergabe-Signale für Anwender-IO: In16   -- FG901_020                                    |
--  +============================================================================================================================+


  signal In16_In:                std_logic_vector(15 downto 0); -- Data_Input
  signal In16_Strobe_in:         std_logic;                     -- Input  "Strobe"
  
  signal In16_LED_Lemo_Out_i:    std_logic;                     -- Input  "nLED_Lemo_In"
  signal In16_nLED_Lemo_Out_o:   std_logic;                     -- Output "nLED_Lemo_In"
                      
  signal In16_Input:             std_logic_vector(15  downto 0);  -- Input Daten
  signal In16_Deb_Input_in:      std_logic_vector(15  downto 0);  -- Input Daten
  signal In16_Deb_Input_out:     std_logic_vector(15  downto 0);  -- Input Daten
  signal In16_Strobe:            std_logic;                       -- Input Strobe
  signal In16_Deb_Strobe_in:     std_logic;                       -- Input Strobe
  signal In16_Deb_Strobe_out:    std_logic;                       -- Input Strobe
  
  signal In16_ADC_Strobe_i:      std_logic;                       -- input  "Strobe-Signal für den ADC"
  signal In16_ADC_Strobe_o:      std_logic;                       -- Output "Strobe-Signal für den ADC (1 CLK breit)"
  signal In16_ADC_shift:         std_logic_vector(2  downto 0);   -- Shift-Reg.

  signal In16_ADC_Data_FF_i:     std_logic_vector(15  downto 0);  -- input  "Daten ADC-Register"
  signal In16_ADC_Data_FF_o:     std_logic_vector(15  downto 0);  -- Output "Daten ADC-Register"
  
  
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
  

  
  Deb60:  for I in 0 to 59 generate
    DB_I:  diob_debounce
    GENERIC MAP (DB_Tst_Cnt   => 3,
                 Test         => 0)             --     
              port map(DB_Cnt => Deb60_Cnt,     -- Debounce-Zeit in Clock's
                       DB_in  => Deb60_in(I),   -- Signal-Input
                       Reset  => Powerup_Res,   -- Powerup-Reset
                       clk    => clk_sys,       -- Sys-Clock
                       DB_Out => Deb60_out(I)); -- Debounce-Signal-Out
    end generate Deb60;


  Syn60:  for I in 0 to 59 generate
    Sy_I:  diob_sync
              port map(Sync_in  => Syn60_in(I),    -- Signal-Input
                       Reset    => Powerup_Res,    -- Powerup-Reset
                       clk      => clk_sys,        -- Sys-Clock
                       Sync_out => Syn60_out(I));  -- Sync-Signal-Out
    end generate Syn60;
  


  
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
      nReset                  => nPowerup_Res,
      master_clk_o            => clk_sys,               -- core clocking
      pll_locked              => pll_locked,
      sys_clk_is_bad          => sys_clk_is_bad,
      sys_clk_is_bad_la       => sys_clk_is_bad_la,
      local_clk_is_bad        => local_clk_is_bad,
      local_clk_is_running    => local_clk_is_running,
      sys_clk_deviation       => sys_clk_deviation,
      sys_clk_deviation_la    => sys_clk_deviation_la,
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
      rstn_o(3)     => rstn_flash);
    
      
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

      Mirr_OutReg_Maske   =>  Mirr_OutReg_Maske,   --  Maskierung für Spiegel-Modus des Ausgangsregisters
     
      Rd_active           =>  Conf_Sts1_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
      Dtack_to_SCUB       =>  Conf_Sts1_Dtack,           -- connect read sources to SCUB-Macro
      Data_to_SCUB        =>  Conf_Sts1_data_to_SCUB,    -- connect Dtack to SCUB-Macro
      LA                  =>  LA_Conf_Sts1            
      );
  
      
AW_Port1: aw_io_reg     
generic map(
      CLK_sys_in_Hz =>  125000000,
      AW_Base_addr =>   c_AW_Port1_Base_Addr
           )
port map  (     

      Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,    -- latched address from SCU_Bus
      Data_from_SCUB_LA   =>  Data_from_SCUB_LA,   -- latched data from SCU_Bus 
      Ext_Adr_Val         =>  Ext_Adr_Val,         -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active       =>  Ext_Rd_active,       -- '1' => Rd-Cycle is active
      Ext_Rd_fin          =>  Ext_Rd_fin,          -- marks end of read cycle, active one for one clock period of sys_clk
      Ext_Wr_active       =>  Ext_Wr_active,       -- '1' => Wr-Cycle is active
      Ext_Wr_fin          =>  SCU_Ext_Wr_fin,      -- marks end of write cycle, active one for one clock period of sys_clk
      clk                 =>  clk_sys,             -- should be the same clk, used by SCU_Bus_Slave
      Ena_every_1us       =>  Ena_every_1us,       -- Clock-Enable-Puls alle Mikrosekunde, 1 Clock breit
      nReset              =>  rstn_sys,      

      SCU_AW_Input_Reg    =>  SCU_AW_Input_Reg,    -- Input-Port's  zum SCU-Bus
      SCU_AW_Output_Reg   =>  SCU_AW_Output_Reg,   -- Output-Port's vom SCU-Bus 

      AWOut_Reg1_wr       =>  AWOut_Reg1_wr,       -- Daten-Reg. AWOut1
      AWOut_Reg2_wr       =>  AWOut_Reg2_wr,       -- Daten-Reg. AWOut2
      AWOut_Reg3_wr       =>  AWOut_Reg3_wr,       -- Daten-Reg. AWOut3
      AWOut_Reg4_wr       =>  AWOut_Reg4_wr,       -- Daten-Reg. AWOut4
      AWOut_Reg5_wr       =>  AWOut_Reg5_wr,       -- Daten-Reg. AWOut5
      AWOut_Reg6_wr       =>  AWOut_Reg6_wr,       -- Daten-Reg. AWOut6
      AWOut_Reg7_wr       =>  AWOut_Reg7_wr,       -- Daten-Reg. AWOut7
      
      Rd_active           =>  AW_Port1_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
      Dtack_to_SCUB       =>  AW_Port1_Dtack,           -- connect read sources to SCUB-Macro
      Data_to_SCUB        =>  AW_Port1_data_to_SCUB,    -- connect Dtack to SCUB-Macro
      LA                  =>  LA_AW_Port1            
      );



Tag_Ctrl1: tag_ctrl     
generic map(
      TAG_Base_addr =>   c_Tag_Ctrl1_Base_Addr
           )
port map  (     

      Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,          -- latched address from SCU_Bus
      Data_from_SCUB_LA   =>  Data_from_SCUB_LA,         -- latched data from SCU_Bus 
      Ext_Adr_Val         =>  Ext_Adr_Val,               -- '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active       =>  Ext_Rd_active,             -- '1' => Rd-Cycle is active
      Ext_Rd_fin          =>  Ext_Rd_fin,                -- marks end of read cycle, active one for one clock period of sys_clk
      Ext_Wr_active       =>  Ext_Wr_active,             -- '1' => Wr-Cycle is active
      Ext_Wr_fin          =>  SCU_Ext_Wr_fin,            -- marks end of write cycle, active one for one clock period of sys_clk
      
      Timing_Pattern_LA   =>  Timing_Pattern_LA,         -- latched timing pattern from SCU_Bus for external user functions
      Timing_Pattern_RCV  =>  Timing_Pattern_RCV,        -- timing pattern received
      Spare0              =>  A_Spare0,                  -- vom Master getrieben
      Spare1              =>  A_Spare1,                  -- vom Master getrieben
      clk                 =>  clk_sys,                   -- should be the same clk, used by SCU_Bus_Slave
      nReset              =>  rstn_sys,              

      SCU_AW_Input_Reg    =>  SCU_AW_Input_Reg,          -- die gleichen Input-Port's wie zum SCU-Bus
      Clr_Tag_Config      =>  Clr_Tag_Config,            -- Clear Tag-Konfigurations-Register
      
      Max_AWOut_Reg_Nr    =>  Max_AWOut_Reg_Nr,          -- Maximale AWOut-Reg-Nummer der Anwendung
      Max_AWIn_Reg_Nr     =>  Max_AWIn_Reg_Nr,           -- Maximale AWIn-Reg-Nummer der Anwendung

      Tag_Maske_Reg       =>  Tag_Maske_Reg,             -- Tag-Output-Maske für Register 1-7
      Tag_Outp_Reg        =>  Tag_Outp_Reg,              -- Tag-Output-Maske für Register 1-7

      Tag_FG_Start        =>  Tag_FG_Start,              -- Start-Puls für den FG
      Tag_Sts             =>  Tag_Sts,                   -- Tag-Status

      Rd_active           =>  Tag_Ctrl1_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
      Data_to_SCUB        =>  Tag_Ctrl1_Data_to_SCUB,    -- connect read sources to SCUB-Macro
      Dtack_to_SCUB       =>  Tag_Ctrl1_Dtack,           -- connect Dtack to SCUB-Macro
      Tag_Aktiv           =>  Tag_Aktiv,                 -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)  
      LA_Tag_Ctrl         =>  LA_Tag_Ctrl1
      );  

      
addac:  addac_reg
  generic map (
    Base_addr => c_ADDAC_Base_addr)
  port map (

    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,  -- latched address from SCU_Bus
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA, -- latched data from SCU_Bus 
    Ext_Adr_Val         =>  Ext_Adr_Val,       -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,     -- '1' => Rd-Cycle is active
    Ext_Rd_fin          =>  Ext_Rd_fin,        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active       =>  Ext_Wr_active,     -- '1' => Wr-Cycle is active
    Ext_Wr_fin          =>  SCU_Ext_Wr_fin,    -- marks end of write cycle, active one for one clock period of sys_clk
    clk                 =>  clk_sys,           -- should be the same clk, used by SCU_Bus_Slave
    nReset              =>  rstn_sys,      

    DAC1_Config         =>  DAC1_Config,       -- DAC1 Config-Register
    DAC1_Config_wr      =>  DAC1_Config_wr,    -- DAC1 Output-Register
    DAC1_Out            =>  DAC1_Out,          -- DAC1 Output-Register
    DAC1_Out_wr         =>  DAC1_Out_wr,       -- DAC1 Output-Register
    DAC2_Config         =>  DAC2_Config,       -- DAC2 Config-Register
    DAC2_Config_wr      =>  DAC2_Config_wr,    -- DAC2 Output-Register
    DAC2_Out            =>  DAC2_Out,          -- DAC2 Output-Register
    DAC2_Out_wr         =>  DAC2_Out_wr,       -- DAC2 Output-Register
    ADC_Config          =>  ADC_Config,        -- ADC  Config-Register
    ADC_In1             =>  ADC_In1,          -- ADC1 Input-Register
    ADC_In2             =>  ADC_In2,          -- ADC2 Input-Register 
    ADC_In3             =>  ADC_In3,          -- ADC3 Input-Register 
    ADC_In4             =>  ADC_In4,          -- ADC4 Input-Register 
    ADC_In5             =>  ADC_In5,          -- ADC5 Input-Register
    ADC_In6             =>  ADC_In6,          -- ADC6 Input-Register
    ADC_In7             =>  ADC_In7,          -- ADC7 Input-Register
    ADC_In8             =>  ADC_In8,          -- ADC8 Input-Register

    Rd_active           =>  addac_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB        =>  addac_Data_to_SCUB,    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB       =>  addac_Dtack,           -- connect Dtack to SCUB-Macro
    LA                  =>  LA_addac
    );  
      
      

io_port: IO_4x8
  generic map (
    Base_addr => c_io_port_Base_Addr)
  port map (
    Adr_from_SCUB_LA    => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA   => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val         => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active       => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk                 => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset              => rstn_sys,          -- in, '0' => resets the IO_4x8
    io                  => s_io,                  -- inout, select and set direction only in 8-bit partitions
    io_7_0_tx           => s_io_7_0_tx,           -- out, '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis      => s_ext_io_7_0_dis,      -- out, '1' = disable external io(7..0)-buffer.
    io_15_8_tx          => s_io_15_8_tx,          -- out, '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis     => s_ext_io_15_8_dis,     -- out, '1' = disable external io(15..8)-buffer.
    io_23_16_tx         => s_io_23_16_tx,         -- out, '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis    => s_ext_io_23_16_dis,    -- out, '1' = disable external io(23..16)-buffer.
    io_31_24_tx         => s_io_31_24_tx,         -- out, '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis    => s_ext_io_31_24_dis,    -- out, '1' = disable external io(31..24)-buffer.
    user_rd_active      => io_port_rd_active,     -- out, '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB        => io_port_data_to_SCUB,  -- out, connect read sources to SCUB-Macro
    Dtack_to_SCUB       => io_port_Dtack);        -- out, connect Dtack to SCUB-Macro

  
--------- AW-Output Mux zu den "Piggys" --------------------

p_AW_Out_Mux:  PROCESS (Tag_Maske_Reg, Tag_Outp_Reg, SCU_AW_Output_Reg) 
    BEGin
    for i in 0 to 15 loop

------ Masken-Reg. aus Tag-Ctrl         Daten => Piggy             User-Output-Reg.                Daten => Piggy        Tag aus Tag-Ctrl
------           |                              |                         |                              |                       |
      IF Tag_Maske_Reg(1)(i)  = '0' then AW_Output_Reg(1)(i)  <= SCU_AW_Output_Reg(1)(i);  else  AW_Output_Reg(1)(i)  <= Tag_Outp_Reg(1)(i);  end if;    -- Daten-Reg. AWOut1
      IF Tag_Maske_Reg(2)(i)  = '0' then AW_Output_Reg(2)(i)  <= SCU_AW_Output_Reg(2)(i);  else  AW_Output_Reg(2)(i)  <= Tag_Outp_Reg(2)(i);  end if;    -- Daten-Reg. AWOut2
      IF Tag_Maske_Reg(3)(i)  = '0' then AW_Output_Reg(3)(i)  <= SCU_AW_Output_Reg(3)(i);  else  AW_Output_Reg(3)(i)  <= Tag_Outp_Reg(3)(i);  end if;    -- Daten-Reg. AWOut3
      IF Tag_Maske_Reg(4)(i)  = '0' then AW_Output_Reg(4)(i)  <= SCU_AW_Output_Reg(4)(i);  else  AW_Output_Reg(4)(i)  <= Tag_Outp_Reg(4)(i);  end if;    -- Daten-Reg. AWOut4
      IF Tag_Maske_Reg(5)(i)  = '0' then AW_Output_Reg(5)(i)  <= SCU_AW_Output_Reg(5)(i);  else  AW_Output_Reg(5)(i)  <= Tag_Outp_Reg(5)(i);  end if;    -- Daten-Reg. AWOut5
      IF Tag_Maske_Reg(6)(i)  = '0' then AW_Output_Reg(6)(i)  <= SCU_AW_Output_Reg(6)(i);  else  AW_Output_Reg(6)(i)  <= Tag_Outp_Reg(6)(i);  end if;    -- Daten-Reg. AWOut6
      IF Tag_Maske_Reg(7)(i)  = '0' then AW_Output_Reg(7)(i)  <= SCU_AW_Output_Reg(7)(i);  else  AW_Output_Reg(7)(i)  <= Tag_Outp_Reg(7)(i);  end if;    -- Daten-Reg. AWOut7
    end loop;  
  END PROCESS p_AW_Out_Mux;


  
INL_xor1: io_reg     
generic map(
      Base_addr =>  c_INL_xor1_Base_Addr
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
      Reg_IO1             =>  INL_xor_IO1,
      Reg_IO2             =>  INL_xor_IO2,    
      Reg_IO3             =>  INL_xor_IO3,    
      Reg_IO4             =>  INL_xor_IO4,    
      Reg_IO5             =>  INL_xor_IO5,    
      Reg_IO6             =>  INL_xor_IO6,  
      Reg_IO7             =>  INL_xor_IO7,    
      Reg_IO8             =>  open,    
--                     
      Reg_rd_active       =>  INL_xor1_rd_active,
      Dtack_to_SCUB       =>  INL_xor1_Dtack,
      Data_to_SCUB        =>  INL_xor1_data_to_SCUB
    );
    
    
INL_msk1: io_reg     
generic map(
      Base_addr =>  c_INL_msk1_Base_Addr
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
      Reg_IO1            =>  INL_msk_IO1,
      Reg_IO2            =>  INL_msk_IO2,   
      Reg_IO3            =>  INL_msk_IO3,  
      Reg_IO4            =>  INL_msk_IO4,  
      Reg_IO5            =>  INL_msk_IO5,  
      Reg_IO6            =>  INL_msk_IO6,
      Reg_IO7            =>  INL_msk_IO7,
      Reg_IO8            =>  open,    
--
      Reg_rd_active      =>  INL_msk1_rd_active,
      Dtack_to_SCUB      =>  INL_msk1_Dtack,
      Data_to_SCUB       =>  INL_msk1_data_to_SCUB
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
      Reg_IO1            =>  IOBP_Masken_Reg1,
      Reg_IO2            =>  IOBP_Masken_Reg2,   
      Reg_IO3            =>  IOBP_Masken_Reg3,  
      Reg_IO4            =>  IOBP_Masken_Reg4,  
      Reg_IO5            =>  IOBP_Masken_Reg5,  
      Reg_IO6            =>  open,
      Reg_IO7            =>  open,
      Reg_IO8            =>  open,    
--
      Reg_rd_active      =>  IOBP_sel1_rd_active,
      Dtack_to_SCUB      =>  IOBP_sel1_Dtack,
      Data_to_SCUB       =>  IOBP_sel1_data_to_SCUB
    );
  
    
 
    
testport_mux: process (A_SEL, AW_Config1, AW_Input_Reg, AW_Output_Reg, LA_Tag_Ctrl1, 
                       LA_AW_Port1, LA_Conf_Sts1, Timing_Pattern_RCV,
                       Timing_Pattern_LA, test_port_in_0, test_clocks, uart_txd_out,
                       Ext_Rd_active, Ext_Rd_fin, Ext_Rd_Fin_ovl, Ext_Wr_active, SCU_Ext_Wr_fin, Ext_Wr_fin_ovl
                       )
begin
  case (not A_SEL) is
    when X"0" => test_out <= AW_Config1;
    when X"1" => test_out <= AW_Input_Reg(1);
    when X"2" => test_out <= AW_Input_Reg(2);
    when X"3" => test_out <= AW_Input_Reg(3);
--
    when X"4" => test_out <= AW_Output_Reg(1);
    when X"5" => test_out <= AW_Output_Reg(2);
    when X"6" => test_out <= AW_Output_Reg(3);
--                                                 +-------------------- '1' drives the external max level shifter
    when X"7" => test_out <= X"000" & '0' & '0' & '1' & uart_txd_out;
--
    when X"8" => test_out <= LA_Tag_Ctrl1;   -- Logic analyser Signals "LA_Tag_Ctrl1"       

    when X"9" => test_out <= LA_Conf_Sts1;
    when X"A" => test_out <= LA_AW_Port1;
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


hp_la_o <= test_out(15 downto 0);





test_port_in_0 <= rstn_sys              & clk_sys         & Ena_Every_100ns & Ena_Every_166ns & -- bit15..12
                  Ext_Wr_active         & SCU_Ext_Wr_fin  & '0'             & FG_1_strobe     & -- bit11..8
                  signal_tap_clk_250mhz & pll_locked      & A_RnW & A_nDS   &                   -- bit7..4
                  A_nBoardSel           & FG_1_strobe     & '0'             & SCUB_Dtack      ; -- bit3..0

            
test_clocks <=  X"0"                                                                              -- bit15..12
              -- use only outputs from PLL
              --& '0' & signal_tap_clk_250mhz & A_SysClock & CLK_20MHz_D                          -- bit11..8
              & '0' & signal_tap_clk_250mhz & '0' & '0'                                           -- bit11..8
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
    intr_in                 => FG_1_dreq & FG_2_dreq & tmr_irq & '0'  -- bit 15..12
                              & x"0"                                  -- bit 11..8
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
  debug_serial_i    => '0');
    
fg_1: fg_quad_scu_bus
  generic map (
    Base_addr => c_fg_1_Base_Addr,
    clk_in_hz => clk_sys_in_Hz,
    diag_on_is_1 => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
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
    diag_on_is_1 => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
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

rd_port_mux:  process ( clk_switch_rd_active, clk_switch_rd_data,
                        wb_scu_rd_active,     wb_scu_data_to_SCUB,
                        FG_1_rd_active,       FG_1_data_to_SCUB,
                        FG_2_rd_active,       FG_2_data_to_SCUB,
                        AW_Port1_rd_active,   AW_Port1_data_to_SCUB,
                        Tag_Ctrl1_rd_active,  Tag_Ctrl1_data_to_SCUB,
                        Conf_Sts1_rd_active,  Conf_Sts1_data_to_SCUB,
                        INL_msk1_rd_active,   INL_msk1_data_to_SCUB,
                        INL_xor1_rd_active,   INL_xor1_data_to_SCUB,
                        tmr_rd_active,        tmr_data_to_SCUB,
                        addac_rd_active,      addac_Data_to_SCUB,
                        io_port_rd_active,    io_port_data_to_SCUB,
                        IOBP_sel1_rd_active,  IOBP_sel1_data_to_SCUB
                      )

  variable sel: unsigned(12 downto 0);
  begin
    sel :=  tmr_rd_active   & INL_xor1_rd_active  & INL_msk1_rd_active    &  AW_Port1_rd_active &  FG_1_rd_active &
            FG_2_rd_active  & wb_scu_rd_active    & clk_switch_rd_active  & Conf_Sts1_rd_active &  Tag_Ctrl1_rd_active &
            addac_rd_active & io_port_rd_active   & IOBP_sel1_rd_active;
    
  case sel IS
      when "1000000000000" => Data_to_SCUB <= tmr_data_to_SCUB;
      when "0100000000000" => Data_to_SCUB <= INL_xor1_data_to_SCUB;
      when "0010000000000" => Data_to_SCUB <= INL_msk1_data_to_SCUB;
      when "0001000000000" => Data_to_SCUB <= AW_Port1_data_to_SCUB;
      when "0000100000000" => Data_to_SCUB <= FG_1_data_to_SCUB;
      when "0000010000000" => Data_to_SCUB <= FG_2_data_to_SCUB;
      when "0000001000000" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when "0000000100000" => Data_to_SCUB <= clk_switch_rd_data;
      when "0000000010000" => Data_to_SCUB <= Conf_Sts1_data_to_SCUB;
      when "0000000001000" => Data_to_SCUB <= Tag_Ctrl1_data_to_SCUB;
      when "0000000000100" => Data_to_SCUB <= addac_Data_to_SCUB;
      when "0000000000010" => Data_to_SCUB <= io_port_data_to_SCUB;
      when "0000000000001" => Data_to_SCUB <= IOBP_sel1_data_to_SCUB;


      when others      => Data_to_SCUB <= (others => '0');
    end case;
  end process rd_port_mux;


  
-------------- Dtack_to_SCUB -----------------------------   

    Dtack_to_SCUB <= ( tmr_dtack   or INL_xor1_Dtack  or INL_msk1_Dtack   or AW_Port1_Dtack  or FG_1_dtack  or 
                       FG_2_dtack  or wb_scu_dtack    or clk_switch_dtack or Conf_Sts1_Dtack or Tag_Ctrl1_Dtack or
                       addac_Dtack or io_port_Dtack   or IOBP_sel1_Dtack);
                     

    A_nDtack <= NOT(SCUB_Dtack);
    A_nSRQ   <= NOT(SCUB_SRQ);


p_interlock:  
process (AW_Input_Reg, Max_AWIn_Reg_Nr,                                                               -- Input-Register
      INL_xor_IO1, INL_xor_IO2, INL_xor_IO3, INL_xor_IO4, INL_xor_IO5, INL_xor_IO6, INL_xor_IO7,  -- Pegel(xor)-Register (default = 0)
      INL_msk_IO1, INL_msk_IO2, INL_msk_IO3, INL_msk_IO4, INL_msk_IO5, INL_msk_IO6, INL_msk_IO7,  -- Maskenregister      (default = 0)
      INL_IO1,    INL_IO2,    INL_IO3,    INL_IO4,     INL_IO5,    INL_IO6,     INL_IO7)          -- Zwischenergebnis: Interlock-Bits xor, Maske
begin
--
  INL_IO1  <=  ((AW_Input_Reg(1) xor INL_xor_IO1) and not INL_msk_IO1);
  INL_IO2  <=  ((AW_Input_Reg(2) xor INL_xor_IO2) and not INL_msk_IO2);
  INL_IO3  <=  ((AW_Input_Reg(3) xor INL_xor_IO3) and not INL_msk_IO3);
  INL_IO4  <=  ((AW_Input_Reg(4) xor INL_xor_IO4) and not INL_msk_IO4);
  INL_IO5  <=  ((AW_Input_Reg(5) xor INL_xor_IO5) and not INL_msk_IO5);
--INL_IO6  <=  ((AW_Input_Reg(6) xor INL_xor_IO6) and not INL_msk_IO6);   -- Aus der Interlock Erzeugung entfernt, wg. der Rücklesmöglichkeit vom H-Word des Timing_Pattern
--INL_IO7  <=  ((AW_Input_Reg(7) xor INL_xor_IO7) and not INL_msk_IO7);   -- Aus der Interlock Erzeugung entfernt, wg. der Rücklesmöglichkeit vom L-Word des Timing_Pattern

-- "Oder-Veknüpfung aller Statusbits (nach Aktiv-Pegel-Selekt und Maske) ---  

case Max_AWIn_Reg_Nr is
  when 1 =>
        if   (INL_IO1 = x"0000")  then
          interlock <= '0'; -- alle Bits = 0 ==> kein Interloc
        else
          interlock <= '1'; -- Interlock aktiv
        end if;
  when 2 =>
        if  ((INL_IO1 = x"0000") and (INL_IO2 = x"0000")) then
          interlock <= '0'; -- alle Bits = 0 ==> kein Interloc
        else
          interlock <= '1'; -- Interlock aktiv
        end if;
  when others =>
        if  ((INL_IO1 = x"0000") and (INL_IO2 = x"0000") and (INL_IO3 = x"0000") and 
             (INL_IO4 = x"0000") and (INL_IO5 = x"0000")) then
        
          interlock <= '0'; -- alle Bits = 0 ==> kein Interloc
        else
          interlock <= '1'; -- Interlock aktiv
        end if;
end case;
  
end process;
  
  
--  +============================================================================================================================+
--  |                                          Anwender-IO: P37IO  -- FG900_700                                                 |
--  +============================================================================================================================+


  
P37_Deb:  for I in 0 to 15 generate
    DB_I:  diob_debounce
    GENERIC MAP (DB_Tst_Cnt => 3, Test => 0)     
    port map(DB_Cnt => Deb60_Cnt, DB_in  => P37IO_Deb_in_Data_i(I), Reset => not rstn_sys, clk => clk_sys, DB_Out => P37IO_Deb_in_Data_o(I));
    end generate P37_Deb;

P37_Syn:  for I in 0 to 15 generate
    Sy_I:  diob_sync
    port map(Sync_in => P37IO_Syn_in_Data_i(I), Reset => not rstn_sys, clk => clk_sys, Sync_out => P37IO_Syn_in_Data_o(I));  -- Sync-Signal
    end generate P37_Syn;


P37IO_in_Start_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)   --     
  port map(DB_Cnt => Deb60_Cnt,               -- Debounce-Zeit in Clock's
           DB_in  => P37IO_Start_deb_i,       -- Signal-Input
           Reset  => not rstn_sys,             -- Powerup-Reset
           clk    => clk_sys,                 -- Sys-Clock
           DB_Out => P37IO_Start_deb_o);      -- Debounce-Signal-Out

P37IO_in_Start_Syn:  diob_sync
  port map(Sync_in  => P37IO_Start_syn_i,     -- Signal-Input
           Reset    => not rstn_sys,           -- Powerup-Reset
           clk      => clk_sys,               -- Sys-Clock
           Sync_out => P37IO_Start_syn_o);    -- Sync-Signal-Out
        
      
P37IO_Out_Led_Start: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P37IO_Start_deb_o,    nLED => P37IO_nLED_Start_o);
  
P37IO_in_Stop_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P37IO_Stop_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P37IO_Stop_deb_o);  

P37IO_in_Stop_Syn:  diob_sync
  port map(Sync_in  => P37IO_Stop_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P37IO_Stop_syn_o);
  
   
P37IO_Out_Led_Stop: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P37IO_Stop_deb_o,    nLED => P37IO_nLED_Stop_o);
 

P37IO_in_Reset_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P37IO_Reset_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P37IO_Reset_deb_o);  
 
P37IO_in_Reset_Syn:  diob_sync
  port map(Sync_in  => P37IO_Reset_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P37IO_Reset_syn_o);

  
p_P37IO_dff: 
process (clk_sys, P37IO_FF_Start, P37IO_FF_Stop, P37IO_FF_Reset, rstn_sys)
begin
  -- Reset whenever the reset signal goes low, regardless of the clock
  -- or the clock enable
  if  ( not rstn_sys    = '1') then
      P37IO_BNC_o  <= '0';
  elsif ( (P37IO_FF_Stop or P37IO_FF_Reset)  = '1') then
      P37IO_BNC_o  <= '0';

      -- If not resetting, and the clock signal is enabled on this register, 
  -- update the register output on the clock's rising edge
  elsif (rising_edge(clk_sys)) then
    if (P37IO_FF_Start = '1') then
      P37IO_BNC_o <= '1';
    end if;
  end if;
end process;


P37IO_Out_Led_BNC: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P37IO_BNC_o,    nLED => P37IO_nELD_BNC_o);

              
  

--  +============================================================================================================================+
--  |                                          Anwender-IO: P25IO  -- FG900_710                                                  |
--  +============================================================================================================================+
  

P25IO_in_Start_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P25IO_Start_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P25IO_Start_deb_o);  

P25IO_in_Start_Syn:  diob_sync
  port map(Sync_in  => P25IO_Start_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P25IO_Start_syn_o);
  
P25IO_Out_Led_Start: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P25IO_Start_deb_o,    nLED => P25IO_nLED_Start_o);

P25IO_in_Stop_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P25IO_Stop_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P25IO_Stop_deb_o);  

P25IO_in_Stop_Syn:  diob_sync
  port map(Sync_in  => P25IO_Stop_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P25IO_Stop_syn_o);
  
P25IO_Out_Led_Stop: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P25IO_Stop_deb_o,    nLED => P25IO_nLED_Stop_o);
 

P25IO_in_Reset_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P25IO_Reset_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P25IO_Reset_deb_o);  

P25IO_in_Reset_Syn:  diob_sync
  port map(Sync_in  => P25IO_Reset_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P25IO_Reset_syn_o);
  
  
  
p_P25IO_dff: 
process (clk_sys, P25IO_FF_Start, P25IO_FF_Stop, P25IO_FF_Reset, rstn_sys)
begin
  -- Reset whenever the reset signal goes low, regardless of the clock
  -- or the clock enable
  if  ( not rstn_sys    = '1') then
      P25IO_BNC_o  <= '0';
  elsif ( (P25IO_FF_Stop or P25IO_FF_Reset)  = '1') then
      P25IO_BNC_o  <= '0';

      -- If not resetting, and the clock signal is enabled on this register, 
  -- update the register output on the clock's rising edge
  elsif (rising_edge(clk_sys)) then
    if (P25IO_FF_Start = '1') then
      P25IO_BNC_o <= '1';
    end if;
  end if;
end process;


P25IO_Out_Led_BNC: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P25IO_BNC_o,    nLED => P25IO_nELD_BNC_o);


  
  --------- DAC_Out-Strobe --------------------

P25IO_DAC_DAC_Strobe: outpuls port map( nReset   => rstn_sys,
                                        CLK      => clk_sys,
                                        Cnt_ena  => '1',
                                        Start    => (P25IO_DAC_DAC_Strobe_i),
                                        Base_cnt => C_Strobe_100ns, 
                                        Mult_cnt => Wert_Strobe_2_Hoch_n(P25IO_DAC_DAC_Strobe_Expo),
                                        Sign_Out => P25IO_DAC_DAC_Strobe_o);


                                  
                             
     -------------------------------------------------------------------------------------------------------
     ---------------------------    Erzeugung des "Holec_DAC_Strobes"   ------------------------------------
     -------------------------------------------------------------------------------------------------------
--
--              <-- 10us --|
--              +--+       +--+       +--+       +--+
--              | 3|  7us  | 3|  7us  | 3|  7us  | 3|
--        ------+  +-------+  +-------+  +-------+  +-------
   
  --------- DAC_Strobe_Puls_1-3 --------------------
P25IO_DAC_Puls: led_n
  generic map (stretch_cnt => C_Strobe_3us) -- = 3us
  port map      (ena => '1', CLK => clk_sys,   Sig_in => P25IO_DAC_Str_Puls_i,   nLED => P25IO_DAC_Str_Puls_o);-- 

  --------- DAC_Strobe_Pause_1-3 --------------------
P25IO_DAC_Pause: led_n
  generic map (stretch_cnt => C_Strobe_7us) -- = 7us
  port map      (ena => '1', CLK => clk_sys,   Sig_in => P25IO_DAC_Str_Pause_i,   nLED => P25IO_DAC_Str_Pause_o);-- 
   

   
P_P25IO_Holec_Strobe:  process (clk_sys, rstn_sys, P25IO_Holec_Strobe_Start, P25IO_DAC_Str_Puls_o, P25IO_DAC_Str_Pause_o)

    begin
      if (rstn_sys = '0') then
        P25IO_Holec_state        <= holec_idle;
        P25IO_DAC_Str_Puls_i     <= '0';          -- Input-Signal für Pulsbreite
        P25IO_DAC_Str_Pause_i    <= '0';          -- Input-Signal für Pausenbreite
        P25IO_Holec_Strobe_Out   <= '0';          -- Output-Signal
        P25IO_Holec_Str_Cnt      <=  0 ;          -- Anzahl der Strobs

        
    ELSIF rising_edge(clk_sys) then
      case P25IO_Holec_state is
        when holec_idle   =>    if  (P25IO_Holec_Strobe_Start  = '1')  THEN
                                     P25IO_Holec_Str_Cnt      <=  4 ;               -- Anzahl der Strobs
                                     P25IO_Holec_state        <= holec_puls;
                                else          
                                     P25IO_Holec_state        <= holec_idle;
                                end if;
                               
        when holec_puls   =>    P25IO_DAC_Str_Puls_i          <= '1';               -- Start Puls-Breiten-Counter
                                P25IO_Holec_state             <= holec_puls_w;
        
        when holec_puls_w =>    P25IO_DAC_Str_Puls_i          <= '0';               -- Stop Puls-Breiten-Counter
                                P25IO_Holec_Strobe_Out        <= '1';               -- Set Output-Strobe (Puls-n)
                                if  (P25IO_DAC_Str_Puls_o      = '0')  THEN         -- Output ist Low-Akiv
                                  P25IO_Holec_state           <= holec_puls_w;
                                else
                                  P25IO_Holec_Str_Cnt         <= P25IO_Holec_Str_Cnt-1 ;    -- Anzahl der Strobs
                                  P25IO_Holec_state           <= holec_puls_e;
                                end if;

        when holec_puls_e =>    P25IO_Holec_Strobe_Out        <= '0';                   -- Reset Output-Strobe (Puls-n)
                                IF  P25IO_Holec_Str_Cnt       <   1 THEN
                                    P25IO_Holec_state         <= holec_end;
                                else  
                                    P25IO_Holec_state         <= holec_pause;
                                end if; 
                  
        when holec_pause  =>    P25IO_DAC_Str_Pause_i         <= '1';                   -- Start Pause-Breiten-Counter
                                P25IO_Holec_state             <= holec_pause_w;
  
        when holec_pause_w =>   P25IO_DAC_Str_Pause_i         <= '0';                   -- Stop Pause-Breiten-Counter
                                if  (P25IO_DAC_Str_Pause_o     = '0')  THEN             -- Output ist Low-Akiv
                                    P25IO_Holec_state         <= holec_pause_w;
                                else
                                    P25IO_Holec_state         <= holec_puls;
                                end if;
                                
        when holec_end     =>   P25IO_Holec_state           <= holec_idle;

        when others =>          P25IO_Holec_state           <= holec_idle;

      end case;
    end if;
  end process P_P25IO_Holec_Strobe;
  






--------- Puls als Strobe (1 Clock breit) --------------------

p_P25IO_ADC_Strobe_Start:  PROCESS (clk_sys, rstn_sys, P25IO_ADC_Strobe_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      P25IO_ADC_shift  <= (OTHERS => '0');
      P25IO_ADC_Strobe_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      P25IO_ADC_shift <= (P25IO_ADC_shift(P25IO_ADC_shift'high-1 downto 0) & (P25IO_ADC_Strobe_i));

      IF P25IO_ADC_shift(P25IO_ADC_shift'high) = '0' AND P25IO_ADC_shift(P25IO_ADC_shift'high-1) = '1' THEN
        P25IO_ADC_Strobe_o <= '1';
      ELSE
        P25IO_ADC_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_P25IO_ADC_Strobe_Start;
  
  
p_P25IO_ADC_FF: 
process (clk_sys, P25IO_ADC_Strobe_o, P25IO_ADC_Data_FF_i, rstn_sys)
begin
  if  ( not rstn_sys      = '1') then   P25IO_ADC_Data_FF_o  <= (OTHERS => '0');
  elsif (rising_edge(clk_sys)) then
    if (P25IO_ADC_Strobe_o = '1') then  P25IO_ADC_Data_FF_o  <= P25IO_ADC_Data_FF_i;
    end if;
  end if;
end process;
  

  

P25IO_in_Ext_Tim_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => P25IO_Ext_Tim_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => P25IO_Ext_Tim_deb_o);  

P25IO_in_Ext_Tim_Syn:  diob_sync
  port map(Sync_in  => P25IO_Ext_Tim_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => P25IO_Ext_Tim_syn_o);
  
P25IO_Out_Led_Ext_Tim: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => P25IO_Ext_Tim_syn_o,    nLED => P25IO_nLED_Ext_Tim_o);

   
--------- Puls als Strobe (1 Clock breit) --------------------

p_P25IO_ECC_Puls_Start:  PROCESS (clk_sys, rstn_sys, P25IO_ECC_Puls_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      P25IO_ECC_Puls_shift  <= (OTHERS => '0');
      P25IO_ECC_Puls_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      P25IO_ECC_Puls_shift <= (P25IO_ECC_Puls_shift(P25IO_ECC_Puls_shift'high-1 downto 0) & (P25IO_ECC_Puls_i));

      IF P25IO_ECC_Puls_shift(P25IO_ECC_Puls_shift'high) = '0' AND P25IO_ECC_Puls_shift(P25IO_ECC_Puls_shift'high-1) = '1' THEN
        P25IO_ECC_Puls_o <= '1';
      ELSE
        P25IO_ECC_Puls_o <= '0';
      END IF;
    END IF;
  END PROCESS p_P25IO_ECC_Puls_Start;
  


  --------- ECC_Out-Strobe --------------------

P25IO_ECC_Strobe: outpuls port map( nReset   => rstn_sys,
                                    CLK      => clk_sys,
                                    Cnt_ena  => '1',
                                    Start    => (P25IO_ECC_Strobe_i),
                                    Base_cnt => C_Strobe_100ns,                  --+-> Outpuls = 3,2ys
                                    Mult_cnt => Wert_Strobe_2_Hoch_n(5),         --+
                                    Sign_Out => P25IO_ECC_Strobe_o);

  
  
--  +============================================================================================================================+
--  |                                           Anwender-IO: OCIN  -- FG900_720                                                  |
--  +============================================================================================================================+


--  +============================================================================================================================+
--  |                                           Anwender-IO: OCIO  -- FG900_730                                                  |
--  +============================================================================================================================+
--

--  +============================================================================================================================+
--  |                                           Anwender-IO: UIO  -- FG900_740                                                   |
--  +============================================================================================================================+

UIO_Out_Led_Lemo_In: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => UIO_LED_Lemo_In_i,    nLED => UIO_nLED_Lemo_In_o);

UIO_Out_Led_Lemo_Out: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => UIO_LED_Lemo_Out_i,    nLED => UIO_nLED_Lemo_Out_o);

  
UIO_in_Start_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => UIO_Lemo_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => UIO_Lemo_deb_o);  

UIO_in_Start_Syn:  diob_sync
  port map(Sync_in  => UIO_Lemo_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => UIO_Lemo_syn_o);

  
  UIO_DAC_Strobe: outpuls port map(nReset   => rstn_sys,
                                   CLK      => clk_sys,
                                   Cnt_ena  => '1',
                                   Start    => (UIO_DAC_Strobe_i),
                                   Base_cnt => C_Strobe_100ns, 
                                   Mult_cnt => Wert_Strobe_2_Hoch_n(UIO_DAC_Strobe_Expo),
                                   Sign_Out => UIO_DAC_Strobe_o);
               
  
  
p_UIO_Out_dff: 
process (clk_sys, UIO_Reg_Enable, rstn_sys)
begin
  if (not rstn_sys = '1') then UIO_Reg_Out_o <= (others => '0');
  elsif (rising_edge(clk_sys)) then
    if (UIO_Reg_Enable = '1') then
       UIO_Reg_Out_o <= UIO_Reg_Out_i;
    end if;
  end if;
end process;
               
               
               
--  +============================================================================================================================+
--  |                                          Anwender-IO: DA  -- FG900_750                                                     |
--  +============================================================================================================================+


DA_in_DA_Trig1_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => DA_Trig1_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => DA_Trig1_deb_o);  

DA_in_DA_Trig1_Syn:  diob_sync
  port map(Sync_in  => DA_Trig1_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => DA_Trig1_syn_o);

  
DA_in_DA_Trig2_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => DA_Trig2_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => DA_Trig2_deb_o);  

DA_in_DA_Trig2_Syn:  diob_sync
  port map(Sync_in  => DA_Trig2_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => DA_Trig2_syn_o);

  
------------------------------------------------------------------------------------------------------------------
-------- DAC_Test_Loop:                                                                         
-------- AW_Output_Reg(3) = 1. DAC-Wert, AW_Output_Reg(4) = 2. DAC-Wert, AW_Output_Reg(5) = Verzögerungszeit in Taktperioden (8ns)                                                                       
------------------------------------------------------------------------------------------------------------------

  
P_Dac_Test_Loop:  process (clk_sys, rstn_sys, dac_state, AW_Output_Reg)

    begin
      if (not rstn_sys = '1') then
        dac_state         <= dac_idle;
        DAC_Test_Out      <= (others => '0');    -- Test-Bitmuster
        DAC_Test_Strobe   <=  '0';               -- Output Test-Strobe
        DAC_Wait_cnt      <= 0;                  -- Wait-Counter

        
    ELSIF rising_edge(clk_sys) then
      case dac_state is
        when dac_idle   =>     if  (AW_Config1(15) = '1')  THEN
                                   dac_state          <= dac_data1;
                               else
                                   dac_state          <= dac_idle;
                               end if;
                               
--------------------------------- DAC-Daten und Strobe für Datenwert 1 ------------------------------------------

        when dac_data1    =>    DAC_Test_Out       <= AW_Output_Reg(3);         -- 1. Dac-Sollwert
                                 dac_state          <= dac_str1h;     
          
        when dac_str1h    =>    DAC_Test_Strobe    <=  '1';               -- Output Test-Strobe
                                dac_state          <= dac_str1l;      
          
        when dac_str1l    =>    DAC_Test_Strobe    <=  '0';               -- Output Test-Strobe
                                DAC_Wait_cnt       <=  to_integer(unsigned(AW_Output_Reg(5)));   -- Wait-Counter 
                                dac_state          <= dac_loop1;
    
        when dac_loop1    =>   if (DAC_Wait_cnt  > 0) then 
                                   DAC_Wait_cnt <= DAC_Wait_cnt-1;    
                                   dac_state    <= dac_loop1;
                               elsE
                                   dac_state    <= dac_wait1;
                               end if;

------------------------------ DAC-Daten und Strobe für Datenwert 2 ------------------------------------------
 
        when dac_wait1    =>    dac_state          <= dac_data2;          -- Laufzeitausgleich    


 
        when dac_data2    =>    DAC_Test_Out       <= AW_Output_Reg(4);         -- 2. Dac-Sollwert
                                dac_state          <= dac_str2h;     
              
        when dac_str2h    =>    DAC_Test_Strobe    <=  '1';               -- Output Test-Strobe
                                dac_state          <= dac_str2l;      
          
        when dac_str2l    =>    DAC_Test_Strobe    <=  '0';               -- Output Test-Strobe
                                DAC_Wait_cnt       <=  to_integer(unsigned(AW_Output_Reg(5)));   -- Wait-Counter 
                                dac_state          <= dac_loop2;
    
        when dac_loop2    =>   if (DAC_Wait_cnt  > 0) then 
                                      DAC_Wait_cnt <= DAC_Wait_cnt-1;    
                                      dac_state    <= dac_loop2;
                                  elsE
                                      dac_state    <= dac_idle;
                                  end if;

        when others =>          dac_state    <= dac_idle;

      end case;
    end if;
  end process P_Dac_Test_Loop;
  



------------------------------------------------------------------------------------------------------------------
-------- DAC_Test_Loop, Treppen-Mode:   AW_Output_Reg(5) = Verzögerungszeit in Taktperioden (8ns)
------------------------------------------------------------------------------------------------------------------

  
P_Dac_tr_Test_Loop:  process (clk_sys, rstn_sys, dac_state, AW_Output_Reg)

    begin
      if (not rstn_sys = '1') then
        dac_tr_state         <= dac_tr_idle;
        DAC_tr_Test_Out      <= (others => '0');    -- Test-Bitmuster
        DAC_tr_Test_Strobe   <= '0';                -- Output Test-Strobe
        DAC_tr_Wait_cnt      <=  0;                 -- Wait-Counter
        DAC_tr_Test_Loop_cnt <=  0;                 -- Loop-Counter
        DAC_tr_Array         <= (others => (others => '0'));    --  DAC-Input  "Test-Treppen-Mode"

  
    ELSIF rising_edge(clk_sys) then
      case dac_tr_state is
        when dac_tr_idle   =>     
                                DAC_tr_Test_Strobe   <= '0';                -- Output Test-Strobe
                                DAC_tr_Wait_cnt      <=  0;                 -- Wait-Counter
                                DAC_tr_Test_Loop_cnt <=  0;                 -- Loop-Counter

                                DAC_tr_Array         <= (x"8000", x"8800", x"9000", x"9800", x"A000", x"A800", x"B000", x"B800", 
                                                         x"C000", x"C800", x"D000", x"D800", x"E000", x"E800", x"F000", x"F800", 
                                                         x"0000", x"0800", x"1000", x"1800", x"2000", x"2800", x"3000", x"3800", 
                                                         x"4000", x"4800", x"5000", x"5800", x"6000", x"6800", x"7000", x"7800", x"7fff");    --  DAC-Input  "Test-Treppen-Mode"
                                              
                       
                                if  (AW_Config1(15) = '1')  THEN
                                  dac_tr_state          <= dac_tr_data;
                                else
                                  dac_tr_state          <= dac_tr_idle;
                                end if;
                               
--------------------------------- DAC-Daten und Strobe für Datenwert 1 ------------------------------------------

        when dac_tr_data    =>  DAC_tr_Test_Out    <= DAC_tr_Array(DAC_tr_Test_Loop_cnt);
                                dac_tr_state       <= dac_tr_strh;      
        
        when dac_tr_strh    =>  DAC_tr_Test_Strobe <=  '1';               -- Output Test-Strobe
                                dac_tr_state       <= dac_tr_strl;      
          
        when dac_tr_strl    =>  DAC_tr_Test_Strobe <=  '0';               -- Output Test-Strobe
                                DAC_tr_Wait_cnt    <=  to_integer(unsigned(AW_Output_Reg(5)));   -- Wait-Counter 
                                dac_tr_state       <= dac_tr_loop1;
    
        when dac_tr_loop1    =>   if (DAC_tr_Wait_cnt  > 0) then 
                                    DAC_tr_Wait_cnt <= DAC_tr_Wait_cnt-1;    
                                    dac_tr_state    <= dac_tr_loop1;
                                  elsE
                                    dac_tr_state    <= dac_tr_end;
                                  end if;

        when dac_tr_end      =>  DAC_tr_Test_Loop_cnt <=  DAC_tr_Test_Loop_cnt + 1;               -- Output Test-Strobe

                                 if DAC_tr_Test_Loop_cnt < 32 then
                                   dac_tr_state     <= dac_tr_data;
                                 else
                                   dac_tr_state     <= dac_tr_idle;
                                 end if;

        when others =>          dac_tr_state    <= dac_tr_idle;

      end case;
    end if;
  end process P_Dac_tr_Test_Loop;
  
  
  
 ------------------------------ LED'S und Strobe's ------------------------------------------


  DA_LED_Ext_Trig1: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => DA_LED_Ext_Trig1_i,    nLED => DA_LED_Ext_Trig1_o);

  DA_LED_Ext_Trig2: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => DA_LED_Ext_Trig2_i,    nLED => DA_LED_Ext_Trig2_o);

  DA_LED_Trig_Out1: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => DA_LED_Trig_Out1_i,    nLED => DA_LED_Trig_Out1_o);

  DA_LED_Trig_Out2: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => DA_LED_Trig_Out2_i,    nLED => DA_LED_Trig_Out2_o);


  
  --------- DAC1_Out-Strobe --------------------
  DA_Trig1: led_n
  generic map (stretch_cnt => C_Strobe_1us) -- = 1us
  port map      (ena => '1', CLK => clk_sys,   Sig_in => DA_Trig1_i,    nLED => DA_Trig1_1us_o);-- 

  --------- DAC2_Out-Strobe --------------------
  DA_Trig2: led_n
  generic map (stretch_cnt => C_Strobe_1us) -- = 1us
  port map      (ena => '1', CLK => clk_sys,   Sig_in => DA_Trig2_i,    nLED => DA_Trig2_1us_o);-- 




 --------- Ext. Trigger1 (Puls aus Signal (1 Clock breit)) --------------------

p_DA_Trig1_Strobe:  PROCESS (clk_sys, rstn_sys, DA_Trig1_Strobe_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      DA_Trig1_shift  <= (OTHERS => '0');
      DA_Trig1_Strobe_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      DA_Trig1_shift <= (DA_Trig1_shift(DA_Trig1_shift'high-1 downto 0) & (DA_Trig1_Strobe_i));

      IF DA_Trig1_shift(DA_Trig1_shift'high) = '0' AND DA_Trig1_shift(DA_Trig1_shift'high-1) = '1' THEN
        DA_Trig1_Strobe_o <= '1';
      ELSE
        DA_Trig1_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_DA_Trig1_Strobe;
 

 --------- Ext. Trigger2 (Puls aus Signal (1 Clock breit)) --------------------

p_DA_Trig2_Strobe:  PROCESS (clk_sys, rstn_sys, DA_Trig2_Strobe_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      DA_Trig2_shift  <= (OTHERS => '0');
      DA_Trig2_Strobe_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      DA_Trig2_shift <= (DA_Trig2_shift(DA_Trig2_shift'high-1 downto 0) & (DA_Trig2_Strobe_i));

      IF DA_Trig2_shift(DA_Trig2_shift'high) = '0' AND DA_Trig2_shift(DA_Trig2_shift'high-1) = '1' THEN
        DA_Trig2_Strobe_o <= '1';
      ELSE
        DA_Trig2_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_DA_Trig2_Strobe;

  
 --------- Strobe für DAC1 (Puls aus Signal (1 Clock breit)) --------------------

p_DA_DAC1_Str_Puls:  PROCESS (clk_sys, rstn_sys, DA_DAC1_Str_Puls_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      DA_DAC1_Str_Puls_shift  <= (OTHERS => '0');
      DA_DAC1_Str_Puls_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      DA_DAC1_Str_Puls_shift <= (DA_DAC1_Str_Puls_shift(DA_DAC1_Str_Puls_shift'high-1 downto 0) & (DA_DAC1_Str_Puls_i));

      IF DA_DAC1_Str_Puls_shift(DA_DAC1_Str_Puls_shift'high) = '0' AND DA_DAC1_Str_Puls_shift(DA_DAC1_Str_Puls_shift'high-1) = '1' THEN
        DA_DAC1_Str_Puls_o <= '1';
      ELSE
        DA_DAC1_Str_Puls_o <= '0';
      END IF;
    END IF;
  END PROCESS p_DA_DAC1_Str_Puls;
  

  --------- Strobe für DAC2 (Puls aus Signal (1 Clock breit)) --------------------

p_DA_DAC2_Str_Puls:  PROCESS (clk_sys, rstn_sys, DA_DAC2_Str_Puls_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      DA_DAC2_Str_Puls_shift  <= (OTHERS => '0');
      DA_DAC2_Str_Puls_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      DA_DAC2_Str_Puls_shift <= (DA_DAC2_Str_Puls_shift(DA_DAC2_Str_Puls_shift'high-1 downto 0) & (DA_DAC2_Str_Puls_i));

      IF DA_DAC2_Str_Puls_shift(DA_DAC2_Str_Puls_shift'high) = '0' AND DA_DAC2_Str_Puls_shift(DA_DAC2_Str_Puls_shift'high-1) = '1' THEN
        DA_DAC2_Str_Puls_o <= '1';
      ELSE
        DA_DAC2_Str_Puls_o <= '0';
      END IF;
    END IF;
  END PROCESS p_DA_DAC2_Str_Puls;
 
  
 
 
--  +===========================================================================================================================+
--  |                                          Anwender-IO:  SPSIO  --  FG900.770 ---                                           |
--  +===========================================================================================================================+
--
--
--  +==========================================================================================================================+
--  |                                          Anwender-IO:  HFIO  --  FG900.780 ---                                           |
--  +==========================================================================================================================+


HFIO_nLED_Tastpuls: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => HFIO_Tastpuls_i,    nLED => HFIO_nLED_Tastpuls_o);

HFIO_nLED_Sample_Puls_inv: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => HFIO_Sample_Puls_inv_i,    nLED => HFIO_nLED_Sample_Puls_inv_o);

HFIO_nLED_Aux: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => HFIO_Aux_i,    nLED => HFIO_nLED_Aux_o);

HFIO_nLED_Sample_Puls_Display: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => HFIO_Sample_Puls_Display_i,    nLED => HFIO_nLED_Sample_Puls_Display_o);


HFIO_in_AMP_FEHLER_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => HFIO_in_AMP_FEHLER_Deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => HFIO_in_AMP_FEHLER_Deb_o);  

HFIO_in_AMP_FEHLER_Syn:  diob_sync
  port map(Sync_in  => HFIO_in_AMP_FEHLER_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => HFIO_in_AMP_FEHLER_syn_o);

 
HFIO_in_PHASE_FEHLER_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => HFIO_in_PHASE_FEHLER_Deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => HFIO_in_PHASE_FEHLER_Deb_o);  

HFIO_in_PHASE_FEHLER_Syn:  diob_sync
  port map(Sync_in  => HFIO_in_PHASE_FEHLER_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => HFIO_in_PHASE_FEHLER_syn_o);
  

--  +============================================================================================================================+
--  |            §§§                        Anwender-IO: IOBP (INLB12S1)  -- FG902_050                                           |
--  +============================================================================================================================+
--
--
--         =========== Component's für die 72 "aktiv" Led's =========== 
--
IOBP_In_LEDn:  for J in 1 to 12 generate
--                ---------------------------------------------------------------------------
                  IOBP_In_LEDn_Slave1:  for I in 1 to 6 generate
                    DB_I:  LED_n
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
        IOBP_LED_En          <=  '0';                --  Output-Enable für LED- ID-Bus
        IOBP_STR_rot_o       <=  (others => '0');    --  Led-Strobs 'rot'
        IOBP_STR_gruen_o     <=  (others => '0');    --  Led-Strobs 'grün'
        IOBP_STR_id_o        <=  (others => '1');    --  ID-Strobs

        
    ELSIF (clk_sys'EVENT AND clk_sys = '1' AND Ena_Every_250ns = '1') THEN
--  ELSIF ((rising_edge(clk_sys)) or Ena_Every_100ns)  then
      case IOBP_state is
        when IOBP_idle   =>  Slave_Loop_cnt       <=  1;                 -- Loop-Counter
                       
                            if  (AW_ID(7 downto 0) = c_AW_INLB12S.ID)  THEN  IOBP_state  <= led_id_wait;
                                                                       else  IOBP_state  <= IOBP_idle;
                            end if;
                           
        when led_id_wait      =>  IOBP_LED_En          <=  '1';                --  Output-Enable für LED- ID-Bus
                                  IOBP_state  <= led_id_loop;

        
        when led_id_loop      =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("1" & "1");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  IOBP_Aktiv_LED_o(Slave_Loop_cnt)(6 downto 1);   -- Aktiv-LED für Slave zum LED-Port
                                  IOBP_state  <= led_str_rot_h;
    
        when led_str_rot_h    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '1';   -- Aktiv LED für Slave (Slave_Loop_cnt) zum LED-Port                       
                                  IOBP_state  <= led_str_rot_l;
    
        when led_str_rot_l    =>  IOBP_STR_rot_o(Slave_Loop_cnt) <=  '0';   -- Aktiv LED für Slave (Slave_Loop_cnt) zum LED-Port                       
                                  IOBP_state  <= led_gruen;

        when led_gruen        =>  IOBP_LED_ID_Bus_o(7 downto 6)  <=  ("1" & "1");
                                  IOBP_LED_ID_Bus_o(5 downto 0)  <=  not IOBP_Sel_LED(Slave_Loop_cnt)(6 downto 1);   -- Sel-LED für Slave zum LED-Port
                                  IOBP_state  <= led_str_gruen_h;
                                  
        when led_str_gruen_h  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '1';   -- Sel-LED für Slave (Slave_Loop_cnt) zum LED-Port                       
                                  IOBP_state  <= led_str_gruen_l;

        when led_str_gruen_l  =>  IOBP_STR_gruen_o(Slave_Loop_cnt) <=  '0';   -- Sel-LED für Slave (Slave_Loop_cnt) zum LED-Port                       
                                  IOBP_state  <= iobp_led_dis;

        when iobp_led_dis     =>  IOBP_LED_En <=  '0';                        --  Disable Output für LED- ID-Bus              
                                  IOBP_state  <= iobp_led_z;

        when iobp_led_z       =>  IOBP_state  <= iobp_id_str_l;

          
        when iobp_id_str_l    =>  --IOBP_STR_ID_o(Slave_Loop_cnt) <=  '0';   -- Sel-ID für Slave (Slave_Loop_cnt)                       
                                  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '1';   -- Sel-ID für Slave (Slave_Loop_cnt)                       
                                  IOBP_state  <= iobp_rd_id;

        when iobp_rd_id       =>  
                                --IOBP_ID(Slave_Loop_cnt) <=  IOBP_LED_ID_Bus_i;   -- Sel-ID für Slave (Slave_Loop_cnt)
                                  IOBP_state  <= iobp_id_str_h;

        when iobp_id_str_h    =>  IOBP_STR_ID_o(Slave_Loop_cnt) <=  '1';   -- Sel-ID für Slave (Slave_Loop_cnt)                       
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
  
  
--  +============================================================================================================================+
--  |                                          Anwender-IO: Out16  -- FG901_010                                                  |
--  +============================================================================================================================+

Out16_Out_Led_Lemo_In: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => Out16_LED_Lemo_In_i,    nLED => Out16_nLED_Lemo_In_o);
  
Out16_in_Start_Deb:  diob_debounce
  GENERIC MAP (DB_Tst_Cnt => 3, Test  => 0)
  port map(DB_Cnt => Deb60_Cnt, DB_in => Out16_Lemo_deb_i, Reset => not rstn_sys, clk => clk_sys, DB_Out => Out16_Lemo_deb_o);  

Out16_in_Start_syn:  diob_sync
  port map(Sync_in  => Out16_Lemo_syn_i, Reset => not rstn_sys, clk  => clk_sys, Sync_out => Out16_Lemo_syn_o);
  
  
Out16_DAC_Strobe: outpuls port map(nReset   => rstn_sys,
                                   CLK      => clk_sys,
                                   Cnt_ena  => '1',
                                   Start    => (Out16_DAC_Strobe_i),
                                   Base_cnt => C_Strobe_100ns, 
                                   Mult_cnt => Wert_Strobe_2_Hoch_n(Out16_DAC_Strobe_Expo),
                                   Sign_Out => Out16_DAC_Strobe_o);
               
  
--  +============================================================================================================================+
--  |   §§§                                    Anwender-IO: In16  -- FG901_020                                                   |
--  +============================================================================================================================+


In16_LED_Lemo_Out: led_n
  generic map (stretch_cnt => stretch_cnt)
  port map      (ena => Ena_Every_20ms, CLK => clk_sys,   Sig_in => In16_LED_Lemo_Out_i,    nLED => In16_nLED_Lemo_Out_o);
  
  
--------- Puls als Strobe (1 Clock breit) --------------------

p_In16_ADC_Strobe_Start:  PROCESS (clk_sys, rstn_sys, In16_ADC_Strobe_i)
  BEGin
    IF not rstn_sys  = '1' THEN
      In16_ADC_shift  <= (OTHERS => '0');
      In16_ADC_Strobe_o    <= '0';

    ELSIF rising_edge(clk_sys) THEN
      In16_ADC_shift <= (In16_ADC_shift(In16_ADC_shift'high-1 downto 0) & (In16_ADC_Strobe_i));

      IF In16_ADC_shift(In16_ADC_shift'high) = '0' AND In16_ADC_shift(In16_ADC_shift'high-1) = '1' THEN
        In16_ADC_Strobe_o <= '1';
      ELSE
        In16_ADC_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_In16_ADC_Strobe_Start;
  
  
p_In16_ADC_FF: 
process (clk_sys, In16_ADC_Strobe_o, In16_ADC_Data_FF_i, rstn_sys)
begin
  if  ( not rstn_sys      = '1') then   In16_ADC_Data_FF_o  <= (OTHERS => '0');
  elsif (rising_edge(clk_sys)) then
    if (In16_ADC_Strobe_o = '1') then  In16_ADC_Data_FF_o  <= In16_ADC_Data_FF_i;
    end if;
  end if;
end process;
  
  
           
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
  
-- §§PROZESS  

p_AW_MUX: PROCESS (clk_sys, rstn_sys, Powerup_Done, AW_ID, s_nLED_Out, PIO, A_SEL, signal_tap_clk_250mhz,
             FG_1_sw, FG_1_strobe, FG_2_sw, FG_2_strobe, P25IO_DAC_Out,
             CLK_IO,
             AWIn_Deb_Time, Min_AWIn_Deb_Time, Deb60_out, Deb60_in, Syn60_in, Syn60_out, Deb_Sync60,
             DIOB_Status1, DIOB_Status2, AW_Status1, AW_Status2, 
             AW_Input_Reg, 
             DIOB_Config1,    DIOB_Config2,    AW_Config1,    AW_Config2, 
             DIOB_Config1_wr, DIOB_Config2_wr, AW_Config1_wr, AW_Config2_wr, 
             AW_Output_Reg, 
             Interlock, UIO, hp_la_o, local_clk_is_running, clk_blink,
             s_nLED_Sel, s_nLED_Dtack, s_nLED_inR, s_nLED_User1_o, s_nLED_User2_o, s_nLED_User3_o, 

             P37IO_FF_Start, P37IO_FF_Stop, P37IO_FF_Reset,
             P37IO_Deb_in_Data_i, P37IO_Deb_in_Data_o, P37IO_Syn_in_Data_i, P37IO_Syn_in_Data_o,
             P37IO_nLED_Start_o, P37IO_nLED_Stop_o, P37IO_BNC_o, P37IO_nELD_BNC_o, P37IO_in_Data, P37IO_Start_deb_i, P37IO_Stop_deb_i, P37IO_Reset_deb_i, 
             P37IO_Start_deb_o, P37IO_Stop_deb_o, P37IO_Reset_deb_o, P37IO_Start_syn_i, P37IO_Stop_syn_i, P37IO_Reset_syn_i,
             P37IO_Start_syn_o, P37IO_Stop_syn_o, P37IO_Reset_syn_o,

             P25IO_FF_Start, P25IO_FF_Stop, P25IO_FF_Reset, P25IO_Start_deb_i, P25IO_Stop_deb_i, P25IO_Reset_deb_i,
             P25IO_Start_deb_o, P25IO_Stop_deb_o, P25IO_Reset_deb_o, P25IO_Start_syn_i, P25IO_Stop_syn_i, P25IO_Reset_syn_i,
             P25IO_Start_syn_o, P25IO_Stop_syn_o, P25IO_Reset_syn_o,
             P25IO_DAC_Mode, P25IO_DAC_DAC_Strobe_o, P25IO_nLED_Start_o, P25IO_nLED_Stop_o, P25IO_BNC_o, P25IO_nELD_BNC_o,
             P25IO_DAC_Data_FG_Out, P25IO_DAC_Strobe, P25IO_ADC_In, P25IO_ADC_Strobe_in, P25IO_ADC_Strobe, P25IO_ADC_Input,
             P25IO_ADC_Data_FF_o, P25IO_ADC_Strobe_o, 
             P25IO_Ext_Tim_deb_o, P25IO_nLED_Ext_Tim_o, P25IO_Ext_Tim_syn_o,
             P25IO_Holec_Strobe_Out, P25IO_ECC_Puls_o, P25IO_ECC_Strobe_o,
             
             OCIN_Data1_in, OCIN_Data2_in,
             OCIO_Data1_in, OCIO_Data2_in,
             SPSIO_Data_in,
             HFIO_nLED_Aux_o, HFIO_nLED_Tastpuls_o, HFIO_nLED_Sample_Puls_Display_o,
             HFIO_in_AMP_FEHLER_Deb_i, HFIO_in_PHASE_FEHLER_Deb_i,
             HFIO_in_AMP_FEHLER_Deb_o, HFIO_in_PHASE_FEHLER_Deb_o,
             HFIO_in_AMP_FEHLER_syn_i, HFIO_in_PHASE_FEHLER_syn_i,
             HFIO_in_AMP_FEHLER_syn_o, HFIO_in_PHASE_FEHLER_syn_o, HFIO_nLED_Sample_Puls_inv_o,
             UIO_Mode, UIO_Reg_Out_o, SCU_Ext_Wr_fin,
             UIO_DAC_Strobe_o, UIO_HS_IN, UIO_LS_IN,
             UIO_Out, UIO_Data_FG_Out, UIO_in_Data, UIO_Lemo_in,
             UIO_nLED_Lemo_In_o, UIO_nLED_Lemo_Out_o, UIO_Lemo_deb_i, UIO_Lemo_deb_o, UIO_Lemo_syn_i, UIO_Lemo_syn_o,
             Tag_Sts, 
             DA_DAC1_Data, DA_DAC1_Out, DA_DAC1_Str, DA_DAC1_Str_Out,
             DA_DAC2_Data, DA_DAC2_Out, DA_DAC2_Str, DA_DAC2_Str_Out,
             DA_Trig1_Strobe_o, DA_Trig2_Strobe_o, DA_DAC1_Str_Puls_o, DA_DAC2_Str_Puls_o,
             DAC_Test_Out,    DAC_Test_Strobe, DAC_tr_Test_Out, DAC_tr_Test_Strobe, 
             DA_LED_Ext_Trig1_o, DA_LED_Ext_Trig2_o,
             DA_Trig1_deb_o, DA_Trig2_deb_o, DA_Trig1_syn_o, DA_Trig2_syn_o,
             DA_Trig1_1us_o,  DA_Trig2_1us_o,
             DA_LED_Trig_Out1_o, DA_LED_Trig_Out2_o,
             Timing_Pattern_LA, Tag_Aktiv,
             DAC1_Config, DAC1_Config_wr, DAC1_Out, DAC1_Out_wr,   
             DAC2_Config, DAC2_Config_wr, DAC2_Out, DAC2_Out_wr,   
             ADC_Config, ADC_In1, ADC_In2, ADC_In3, ADC_In4, ADC_In5, ADC_In6, ADC_In7, ADC_In8,
             AWOut_Reg1_wr, AWOut_Reg2_wr,
             IOBP_Masken_Reg1, IOBP_Masken_Reg2, IOBP_Masken_Reg3, IOBP_Masken_Reg4, IOBP_Masken_Reg5, IOBP_Output,
             IOBP_Input, IOBP_LED_ID_Bus_o, IOBP_ID, IOBP_LED_En, IOBP_STR_rot_o, IOBP_STR_gruen_o, IOBP_STR_ID_o,
             Out16_Strobe, Out16_Mode, Out16_DAC_Strobe_o, Out16_Out, Out16_Data_FG_Out, Out16_Lemo_in,
             Out16_nLED_Lemo_In_o, Out16_Lemo_deb_i, Out16_Lemo_deb_o, Out16_Lemo_syn_i, Out16_Lemo_syn_o,
             In16_In, In16_Strobe_In, In16_nLED_Lemo_Out_o, In16_Strobe, In16_Input, IN16_ADC_Data_FF_o, IN16_ADC_Strobe_o
             )
             
           
--  +============================================================================================================================+
--  |                  Übergabe-Signale für Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen                    |
--  +============================================================================================================================+


 
BEGIN

  --############################# Set Defaults ######################################

    PIO(150 downto 16)   <=  (OTHERS => 'Z');   -- setze alle IO-Pins auf input;
    UIO(15 downto 0)     <=  (OTHERS => 'Z');   -- UIO = Input;

    AW_Input_Reg         <=  (OTHERS => (OTHERS => '0'));  -- AW_Input_Reg's = 0

    AW_ID(7 downto 0)    <=  x"FF";    -- Anwender-Karten ID
    
    extension_cid_system <= 0;   -- extension card: cid_system
    extension_cid_group  <= 0;   -- extension card: cid_group
    
    Max_AWOut_Reg_Nr     <= 0;    -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 0;    -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn_Deb_Time        <= 0;    -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time", Wert aus DIOB-Config 1
    Min_AWIn_Deb_Time    <= 0;    -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us
  

    Diob_Status1(15 downto 6) <= (OTHERS => '0');       -- Reserve
    Diob_Status1(5 downto 0)  <= Tag_Sts(5 downto 0);   -- Tag-Ctrl Status

    Diob_Status2    <= (x"00" &      -- Input-Port-Diob_Status2
                        Tag_Aktiv);  -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)  

    AW_Status1      <= x"123C";  -- "Dummy" Input-Port-AW_Sts    
    AW_Status2      <= x"123D";  -- "Dummy" Input-Port-AW_Sts2
     
    A_Tclk               <= '0';  -- Clock  für HP-Logic-Analysator

    
    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 

    Deb60_cnt                <= 0;
    Deb60_in                 <= (OTHERS => '0');
    Syn60_in                <= (OTHERS => '0');
    Deb_Sync60               <= (OTHERS => '0');
   
   
    P37IO_Start_deb_i        <= '0';
    P37IO_Stop_deb_i         <= '0';
    P37IO_Reset_deb_i        <= '0';
    P37IO_Start_syn_i        <= '0';
    P37IO_Stop_syn_i         <= '0';
    P37IO_Reset_syn_i        <= '0';
    P37IO_FF_Start           <= '0';    -- Input mit Entprellung
    P37IO_FF_Stop            <= '0';    -- Input mit Entprellung
    P37IO_FF_Reset           <= '0';    -- Input mit Entprellung

    P37IO_in_Data            <= (OTHERS => '0');
    P37IO_Deb_in_Data_i      <= (OTHERS => '0');
    P37IO_Syn_in_Data_i      <= (OTHERS => '0');


    P25IO_Start_deb_i        <= '0';
    P25IO_Stop_deb_i         <= '0';
    P25IO_Reset_deb_i        <= '0';
    P25IO_Start_syn_i        <= '0';
    P25IO_Stop_syn_i         <= '0';
    P25IO_Reset_syn_i        <= '0';
    P25IO_FF_Start           <= '0';    -- Input mit Entprellung
    P25IO_FF_Stop            <= '0';    -- Input mit Entprellung
    P25IO_FF_Reset           <= '0';    -- Input mit Entprellung

    P25IO_DAC_Mode            <=  (OTHERS => '0');  -- P25IO_DAC_Mode 
    P25IO_DAC_Data_FG_Out     <=  (OTHERS => '0');  -- Data/FG-Output 
    P25IO_DAC_Out             <=  (OTHERS => '0');  -- Daten zum DAC
    P25IO_DAC_Strobe          <=  '0';              -- Output "nStrobe"               
    P25IO_DAC_DAC_Strobe_Expo <=   0;               -- Exponent für die Breite des Strobs
    P25IO_DAC_DAC_Strobe_i    <=  '0';              -- Input-Strobe
    P25IO_Holec_Strobe_Start  <=  '0';              -- Start Holec Strobe-Sequence
    
    P25IO_ADC_In              <=  (OTHERS => '0');  -- Input Daten   
    P25IO_ADC_Input           <=  (OTHERS => '0');  -- Input Daten   
    P25IO_ADC_Strobe_in       <= '0';               -- Input Strobe
    P25IO_ADC_Strobe          <= '0';               -- Input Strobe
    P25IO_ADC_Strobe_i        <= '0';               -- Input Strobe  
    P25IO_ADC_Data_FF_i       <=  (OTHERS => '0');  -- input  "Daten ADC-Register"
    P25IO_Ext_Tim_deb_i       <= '0';
    P25IO_Ext_Tim_syn_i       <= '0';
    P25IO_ECC_Puls_i          <= '0';
    P25IO_ECC_Strobe_i        <= '0';

    
    OCIN_Data1_in             <=  (OTHERS => '0');
    OCIN_Data2_in             <=  (OTHERS => '0');
    OCIO_Data1_in             <=  (OTHERS => '0');
    OCIO_Data2_in             <=  (OTHERS => '0');
    SPSIO_Data_in             <=  (OTHERS => '0');
    
    HFIO_Aux_i                  <= '0';
    HFIO_Tastpuls_i             <= '0';
    HFIO_Sample_Puls_Display_i  <= '0';
    HFIO_Sample_Puls_inv_i      <= '0';
    HFIO_in_AMP_FEHLER_Deb_i    <= '0';       
    HFIO_in_PHASE_FEHLER_Deb_i  <= '0';
    HFIO_in_AMP_FEHLER_syn_i    <= '0';       
    HFIO_in_PHASE_FEHLER_syn_i  <= '0';

    UIO_Mode                    <= (OTHERS => '0');     -- UIO ('740) Mode
    UIO_Data_FG_Out             <= (OTHERS => '0');     -- Data/FG-Output
    UIO_Out                     <= (OTHERS => '0');     -- Data_Output
    UIO_HS_In                   <= (OTHERS => '0');     -- Input auf GND 
    UIO_LS_In                   <= (OTHERS => '0');     -- Input Uext
    UIO_in_Data                 <= (OTHERS => '0');     -- Data_Input über Optokoppler
    UIO_LED_Lemo_In_i           <=  '0';                -- Input  "nLED_Lemo_In"
    UIO_LED_Lemo_Out_i          <=  '0';                -- Input  "nLED_Lemo_Out"
    UIO_Lemo_in                 <=  '0';                -- Input "Lemo" 
    UIO_Lemo_deb_i              <=  '0';                -- Debounce: Input "Lemo" 
    UIO_Lemo_syn_i              <=  '0';                -- Debounce: Input "Lemo" 
    UIO_DAC_Strobe_i            <=  '0';                -- Input: UIO-Strobe
    UIO_DAC_Strobe_Expo         <=   0;                 -- Exponent für die Breite des Strobs
    UIO_Reg_Out_i               <= (OTHERS => '0');     -- Input vom Daten-OutputReg. 
    UIO_Reg_Enable              <=  '0';                -- Enable-Input vom Daten-OutputReg.
    
    
    
    DA_DAC1_Str                 <=  '0';                -- DAC1-Strobe
    DA_DAC1_Str_Out             <=  '0';                -- DAC1-Output-Strobe
    DA_DAC1_Data                <=   (OTHERS => '0');   -- DAC1-Data   Bit-15
    DA_DAC1_Out                 <=   (OTHERS => '0');   -- DAC1-Output Bit-15

    DA_DAC2_Str                 <=  '0';                -- DAC2-Strobe
    DA_DAC2_Str_Out             <=  '0';                -- DAC2-Output-Strobe
    DA_DAC2_Data                <=   (OTHERS => '0');   -- DAC2-Data   Bit-15
    DA_DAC2_Out                 <=   (OTHERS => '0');   -- DAC2-Output Bit-15

    DA_Trig1_deb_i              <= '0';
    DA_Trig2_deb_i              <= '0';
    DA_Trig1_syn_i              <= '0';
    DA_Trig2_syn_i              <= '0';

    DA_Trig1_i                  <= '0';
    DA_Trig2_i                  <= '0';
    DA_Trig1_Strobe_i           <= '0';
    DA_Trig2_Strobe_i           <= '0';
    DA_LED_Trig_Out1_i          <= '0';
    DA_LED_Trig_Out2_i          <= '0';
    DA_LED_Ext_Trig1_i          <= '0';
    DA_LED_Ext_Trig2_i          <= '0';
    DA_DAC1_Str_Puls_i          <= '0';
    DA_DAC2_Str_Puls_i          <= '0';
    
    ADC_In1(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In2(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In3(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In4(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In5(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In6(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In7(15 downto 0)  <=  x"0000";  -- ADC Input-Register
    ADC_In8(15 downto 0)  <=  x"0000";  -- ADC Input-Register
 
    IOBP_Aktiv_LED_i  <=  (OTHERS => (OTHERS => '0'));  -- Inputs für die Aktiv-LED-Monoflops der "Slave-Karte 1-12"
    IOBP_Input        <=  (OTHERS => (OTHERS => '0'));  -- Data_Input  "Slave-Karte 1-12"
    IOBP_Output       <=             (OTHERS => '0');   -- Data_Output "Slave-Karte 1-12"
    IOBP_Sel_LED      <=  (OTHERS => (OTHERS => '0'));  -- Inputs für die Aktiv-LED-Monoflops der "Slave-Karte 1-12"
    IOBP_ID           <=  (OTHERS => (OTHERS => '1'));  -- Inputs für die Aktiv-LED-Monoflops der "Slave-Karte 1-12"
    IOBP_LED_ID_Bus_i <=             (OTHERS => '1');   -- Data_Output "Slave-Karte 1-12"
    
    Out16_Mode                    <= (OTHERS => '0');     -- Out16 ('901.010') Mode
    Out16_Data_FG_Out             <= (OTHERS => '0');     -- Data/FG-Output
    Out16_Out                     <= (OTHERS => '0');     -- Data_Output
    Out16_Strobe                  <=  '0';                -- Output "nStrobe"
    Out16_LED_Lemo_In_i           <=  '0';                -- Input  "nLED_Lemo_In"
    Out16_Lemo_in                 <=  '0';                -- Input "Lemo" 
    Out16_Lemo_deb_i              <=  '0';                -- Debounce: Input "Lemo" 
    Out16_Lemo_syn_i              <=  '0';                -- Debounce: Input "Lemo" 
    Out16_DAC_Strobe_i            <=  '0';                -- Input: UIO-Strobe
    Out16_DAC_Strobe_Expo         <=   0;                 -- Exponent für die Breite des Strobs
    
    In16_In                      <= (OTHERS => '0');     -- Data_Input
    In16_Strobe_in               <=  '0';                -- Input  "Strobe"
    In16_Input                   <= (OTHERS => '0');     -- Data_Input
    In16_Strobe                  <=  '0';                -- Input  "Strobe"
    In16_LED_Lemo_Out_i          <=  '0';                -- Input  "nLED_Lemo_Out"
    In16_ADC_Data_FF_i           <=  (OTHERS => '0');    -- Input-Register
    In16_ADC_Strobe_i            <= '0';                 -- Input-Register-Strobe
    
    
    
    
    
    --#################################################################################
    --###                                                                           ###
    --###                    IO-Stecker-Test mit "BrückenStecker                    ###
    --###                                                                           ###
    --#################################################################################
    
    
    IF  DIOB_Config1(15) = '1'  THEN   -- Config-Reg Bit15 = 1  --> Testmode 


    --- Test der PIO-Pins ---

      AW_Input_Reg(1)(15 downto 0)  <=  ( CLK_IO,   PIO(16),  PIO(17),  PIO(18),  PIO(19),  PIO(20),  PIO(21),  PIO(22),
                                      PIO(23),  PIO(24),  PIO(25),  PIO(26),  PIO(27),  PIO(28),  PIO(29),  PIO(30)   );

          ( PIO(61),  PIO(62),  PIO(59),  PIO(60),  PIO(57),  PIO(58),  PIO(55),  PIO(56),                      
            PIO(53),  PIO(54),  PIO(51),  PIO(52),  PIO(49),  PIO(50),  PIO(47),  PIO(48)   )   <=  AW_Output_Reg(1)(15 downto 0) ;         
  

      AW_Input_Reg(2)(15 downto 0)  <=  ( PIO(31),  PIO(32),  PIO(33),  PIO(34),  PIO(35),  PIO(36),  PIO(37),  PIO(38),
                                      PIO(39),  PIO(40),  PIO(41),  PIO(42),  PIO(43),  PIO(44),  PIO(45),  PIO(46)   );

          ( PIO(77),  PIO(78),  PIO(75),  PIO(76),  PIO(73),  PIO(74),  PIO(71),  PIO(72),
            PIO(69),  PIO(70),  PIO(67),  PIO(68),  PIO(65),  PIO(66),  PIO(63),  PIO(64)   )   <=  AW_Output_Reg(2)(15 downto 0) ;


      AW_Input_Reg(3)(15 downto 0)  <=  ( PIO(79),  PIO(80),  PIO(81),  PIO(82),  PIO(83),  PIO(84),  PIO(85),  PIO(86),
                                      PIO(87),  PIO(88),  PIO(89),  PIO(90),  PIO(91),  PIO(92),  PIO(93),  PIO(94)   );
              
          ( PIO(125), PIO(126), PIO(123), PIO(124), PIO(121), PIO(122), PIO(119), PIO(120),
            PIO(117), PIO(118), PIO(115), PIO(116), PIO(113), PIO(114), PIO(111), PIO(112)  )   <=  AW_Output_Reg(3)(15 downto 0) ;

            
      AW_Input_Reg(4)(15 downto 0)  <=  ( PIO(95),  PIO(96),  PIO(97),  PIO(98),  PIO(99),  PIO(100), PIO(101), PIO(102),
                                      PIO(103), PIO(104), PIO(105), PIO(106), PIO(107), PIO(108), PIO(109), PIO(110)  );
                          
          ( PIO(141), PIO(142), PIO(139), PIO(140), PIO(137), PIO(138), PIO(135), PIO(136),               
            PIO(133), PIO(134), PIO(131), PIO(132), PIO(129), PIO(130), PIO(127), PIO(128)  )   <=  AW_Output_Reg(4)(15 downto 0) ;



    AW_Input_Reg(5)(15 downto 4)  <=   AW_Output_Reg(5)(15 downto 4); --+   Input [15..4] = Copy der Output-Bits, da Testprog. nur 16 Bit Vergleich.
    AW_Input_Reg(5)(3  downto 0)  <=  (PIO(143), PIO(144), PIO(149), PIO(150));

   --  Beim Test, sind die Pins vom AW_Output_Reg(5)(3 downto 0) mit AW_Input_Reg(5)(3 downto 0) extern verbunden.
    
                    (PIO(147), PIO(148), PIO(145), PIO(146))    <=  AW_Output_Reg(5)(3 downto 0) ;


    --- Test der User-Pins zur VG-Leiste und HPLA1 (HP-Logicanalysator) ---

    UIO(15 downto 0)          <= (OTHERS => 'Z');         -- UIO = Input;
    AW_Input_Reg(6)(15 downto 0)  <=  UIO(15 downto 0);       -- User-Pins zur VG-Leiste als Input
    A_TA(15 downto 0)         <= AW_Output_Reg(6)(15 downto 0);  -- HPLA1 (HP-Logicanalysator) als Output


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
      AW_ID(7 downto 0)         <=  PIO(150 downto 143);

  
    --  --- Output: Anwender-LED's ---

    PIO(17) <= s_nLED_Sel;                          -- LED7 = sel Board 
    PIO(19) <= s_nLED_Dtack;                        -- LED6 = Dtack 
    PIO(21) <= s_nLED_inR;                          -- LED5 = interrupt
    PIO(23) <= not Powerup_Done or clk_blink;       -- LED4 = Powerup 
    PIO(25) <= s_nLED_User1_o;                      -- LED3 = User 1
    PIO(27) <= s_nLED_User2_o;                      -- LED2 = User 2  
    PIO(29) <= s_nLED_User3_o;                      -- LED1 = User 3 
    PIO(31) <= local_clk_is_running and clk_blink;  -- LED0 (User-4) = int. Clock 
    

    A_TA(15 downto 0) <= hp_la_o(15 downto 0); ----------------- Output für HP-Logic-Analysator

    A_Tclk   <= signal_tap_clk_250mhz;  -- Clock  für HP-Logic-Analysator


    UIO(0)  <= not Interlock; -- Ist kein Interlock-Bit gesetzt ==> UIO(0) = 1 (low-aktiv)

    AW_Input_Reg(6)   <=  Timing_Pattern_LA(31 downto 16);  -- H-Word vom Timing_Pattern
    AW_Input_Reg(7)   <=  Timing_Pattern_LA(15 downto 0);   -- L-Word vom Timing_Pattern

    
  CASE AW_ID(7 downto 0) IS
  

  WHEN  c_AW_P37IO.ID =>

    --#################################################################################
    --####                  Anwender-IO: P37IO  -- FG900_700                        ###
    --#################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-9  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--       8   | Output-Polarität Lemo,   1 = Negativ,  0 = Positiv(Default)               --
--       7   | Output-Polarität Daten,  1 = Negativ,  0 = Positiv(Default)               --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --

      extension_cid_system <= c_cid_system;    -- extension card: CSCOHW
      extension_cid_group  <= c_AW_P37IO.CID;  -- extension card
    
      Max_AWOut_Reg_Nr     <= 2;      -- Maximale AWOut-Reg-Nummer der Anwendung
      Max_AWIn_Reg_Nr      <= 1;      -- Maximale AWIn-Reg-Nummer der Anwendung
      Min_AWIn_Deb_Time    <= 1;      -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

      s_nLED_User1_i       <= '0';    -- LED3 = User 1, -- frei -- 
      s_nLED_User2_i       <= '0';    -- LED3 = User 2, -- frei -- 
      s_nLED_User3_i       <= '0';    -- LED3 = User 3, -- frei -- 
      

    --############################# Set Debounce- oder Syn-Time ######################################
      
      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

      IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
          Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
      ELSE
          Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;
      
      
    --############################# Start/Stop FF ######################################

      P37IO_Start_deb_i   <=  not PIO(139);         -- input "LemoBuchse-Start" H-Aktiv, nach dem Optokoppler aber L-Aktiv
      PIO(33)             <=  P37IO_nLED_Start_o;   --  Output "nLED_Start"
      P37IO_Stop_deb_i    <=  not PIO(141);         -- input "LemoBuchse-Stop" L-Aktiv, nach dem Optokoppler aber L-Aktiv
      PIO(35)             <=  P37IO_nLED_Stop_o;    --  Output "nLED_Stop"
      P37IO_Reset_deb_i   <=  not PIO(133);         -- input "Rest-Taster" L-Aktiv

      PIO(37)          <=  P37IO_nELD_BNC_o;        -- Output "nLED_BNC"

      
   --###################### Input's ==> FF ########################
      
      IF  (Diob_Config1(11) = '0')  THEN            -- 0 = Entprellung "Eingeschaltet" 
          P37IO_FF_Start  <=  P37IO_Start_deb_o;    -- Input mit Entprellung
          P37IO_FF_Stop   <=  P37IO_Stop_deb_o;     -- Input mit Entprellung
          P37IO_FF_Reset  <=  P37IO_Reset_deb_o;    -- Input mit Entprellung
      ELSE
          P37IO_FF_Start  <=  P37IO_Start_deb_i;    -- Input ohne Entprellung
          P37IO_FF_Stop   <=  P37IO_Stop_deb_i;     -- Input ohne Entprellung
          P37IO_FF_Reset  <=  P37IO_Reset_deb_i;    -- Input ohne Entprellung
      END IF;
    
      AW_Input_Reg(1)( 8)  <=  P37IO_FF_Start;      --  Start     -+-- Signale am FF
      AW_Input_Reg(1)( 9)  <=  P37IO_FF_Stop;       --  Stop       |
      AW_Input_Reg(1)(10)  <=  P37IO_FF_Reset;      --  Reset      |
      AW_Input_Reg(1)(11)  <=  P37IO_BNC_o;         --  Output FF -+
      
      
   --###################### FF ==> Output ########################

      IF  (AW_Config1(8) = '0')  THEN
        PIO(51)     <=      P37IO_BNC_o;          -- Output "BNC" positiv
      Else
        PIO(51)     <=  not P37IO_BNC_o;          -- Output "BNC" negativ  
      END IF; 
      
      PIO(37)          <=  P37IO_nELD_BNC_o;     -- Output "nLED_BNC"


   --###################### AWOut ==> Output ########################

      PIO(39) <=  '0';  -------+------------------- Output_Enable (nach init vom ALTERA)
      PIO(41) <=  '0';  -------+
      PIO(43) <=  '0';  -------+


      IF  (AW_Config1(7) = '0')  THEN
        (PIO(65), PIO(69), PIO(61), PIO(67), PIO(63), PIO(71), PIO(55), PIO(53)) <=  not AW_Output_Reg(1)(7 downto 0);  --  Output "CO_D[7..0]" positiv
         PIO(57)                                                                 <=  not AW_Output_Reg(1)(8);           --  Output "CO_FAULT"
         PIO(59)                                                                 <=  not AW_Output_Reg(1)(9);           --  Output "CO_STAT"
      Else                                                                                                                              
        (PIO(65), PIO(69), PIO(61), PIO(67), PIO(63), PIO(71), PIO(55), PIO(53)) <=      AW_Output_Reg(1)(7 downto 0);  --  Output "CO_D[7..0]" negativ
         PIO(57)                                                                 <=      AW_Output_Reg(1)(8);           --  Output "CO_FAULT"
         PIO(59)                                                                 <=      AW_Output_Reg(1)(9);           --  Output "CO_STAT"
      END IF;  
      

   --##################################### Debounce, Input's ##########################################

      P37IO_in_Data(15 downto 8) <=  not (PIO(131), PIO(129), PIO(127), PIO(125), PIO(123), PIO(121), PIO(119), PIO(117)); -- Input "HI[7..0]" 
      P37IO_in_Data(7  downto 0) <=  not (PIO(115), PIO(113), PIO(111), PIO(109), PIO(107), PIO(105), PIO(103), PIO(101)); -- Input "LO[7..0]" 
      
--      Deb60_in(15 downto 0)   <= P37IO_in_Data(15 downto 0);   -- Debounce-Inputs         
--      Syn60_in(15 downto 0)   <= P37IO_in_Data(15 downto 0);   -- Sync-Inputs         

      P37IO_Deb_in_Data_i(15 downto 0)   <= P37IO_in_Data(15 downto 0);   -- Debounce-Inputs         
      P37IO_Syn_in_Data_i(15 downto 0)   <= P37IO_in_Data(15 downto 0);   -- Sync-Inputs         
      
      
   --##################################### Input's ==> AW_Input_Reg(2) #########################################
      
      IF  (Diob_Config1(11) = '0')  THEN
              AW_Input_Reg(2)(15 downto 0)  <=  P37IO_Deb_in_Data_i(15 downto 0);   -- Input mit Entprellung
      ELSE    
              AW_Input_Reg(2)(15 downto 0)  <=  P37IO_Syn_in_Data_i(15 downto 0);   -- Input ohne Entprellung
      END IF;



  WHEN   c_AW_P25IO.ID =>
  
    --#################################################################################
    --####                    Anwender-IO: P25IO  -- FG900_710                      ###
    --#################################################################################
    
--           +======================================================================+
--           |            User-Config-Register 1 (AW_Config1)                       |
--           +======================================================================+
--           |                           DAC/ADC-Mode                               |
--           +=======================================================================

    
      extension_cid_system <= c_cid_system;     -- extension card: CSCOHW
      extension_cid_group  <= c_AW_P25IO.CID;   -- extension card: cid_group, "FG900710_P25IO1" = 28
  
      Max_AWOut_Reg_Nr     <= 1;  -- Maximale AWOut-Reg-Nummer der Anwendung
      Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
      Min_AWIn_Deb_Time    <= 1;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us
  
      
      --############################# Set Debounce-Time ######################################
        
      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        
  
      IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
          Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
      ELSE
          Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;

      
      --################################### Set LED's ########################################
    

      IF (AW_Config1(0) = '1') AND (AW_Config1(1) = '1') THEN  s_nLED_User1_i <= '1';    -- LED3 = User 1, DAC-Data vom FG
                                                         ELSE  s_nLED_User1_i <= '0';    -- LED3 = User 1, DAC-Data vom AW_Outreg
      END IF;
      
      s_nLED_User2_i <= P25IO_ECC_Puls_o;          -- LED2 = User 2, ECC (Enable-Conv-CMD)
      s_nLED_User3_i <= P25IO_ADC_Strobe_in;       -- LED1 = User 3, EOC vom ADC 
    
    
 
    --############################# Start/Stop FF ######################################

      P25IO_Start_deb_i   <=  not PIO(71);          -- input "LemoBuchse-Start" H-Aktiv, nach dem Optokoppler aber L-Aktiv
      PIO(87)             <=  P25IO_nLED_Start_o;   -- Output "nLED_Start"
      P25IO_Stop_deb_i    <=  not PIO(75);          -- input "LemoBuchse-Stop" L-Aktiv, nach dem Optokoppler aber L-Aktiv
      PIO(89)             <=  P25IO_nLED_Stop_o;    -- Output "nLED_Stop"
      P25IO_Reset_deb_i   <=  not PIO(67);          -- input "Rest-Taster" L-Aktiv

      
   --###################### Input's ==> FF ########################

      IF  (Diob_Config1(11) = '0')  THEN            -- 0 = Entprellung "Eingeschaltet" 
          P25IO_FF_Start  <=  P25IO_Start_deb_o;    -- Input mit Entprellung
          P25IO_FF_Stop   <=  P25IO_Stop_deb_o;     -- Input mit Entprellung
          P25IO_FF_Reset  <=  P25IO_Reset_deb_o;    -- Input mit Entprellung
      ELSE
          P25IO_FF_Start  <=  P25IO_Start_deb_i;    -- Input ohne Entprellung
          P25IO_FF_Stop   <=  P25IO_Stop_deb_i;     -- Input ohne Entprellung
          P25IO_FF_Reset  <=  P25IO_Reset_deb_i;    -- Input ohne Entprellung
      END IF;

      AW_Input_Reg(1)( 8)  <=  P25IO_FF_Start;       --  Start     -+-- Signale am FF
      AW_Input_Reg(1)( 9)  <=  P25IO_FF_Stop;        --  Stop       |
      AW_Input_Reg(1)(10)  <=  P25IO_FF_Reset;       --  Reset      |
      AW_Input_Reg(1)(11)  <=  P25IO_BNC_o;          --  Output FF -+
      AW_Input_Reg(1)(12)  <=  P25IO_Ext_Tim_deb_o;  --  Input "P25IO_Ext_Tim_deb_o" 
      
   --###################### FF ==> Output ########################


      IF  (AW_Config1(11) = '0')  THEN
        PIO(103)     <=      P25IO_BNC_o;          -- Output "BNC" positiv
      Else
        PIO(103)     <=  not P25IO_BNC_o;          -- Output "BNC" negativ  
      END IF; 

      PIO(91)             <=  P25IO_nELD_BNC_o;     -- Output "nLED_BNC"

      
--           +======================================================================+    --
--           |         User-Config-Register 1 (AW_Config1)                          |    --
--           +======================================================================+    --
--           |                              DAC-Mode                                |
--     ------+======================================================================+    --
--       7   | Output-Polarität Bit [15..0],  1 = Negativ,  0 = Positiv(Default)    |    --
--     ------+----------------------------------------------------------------------|    --
--       6   | Strobe-Polarität,         1 = Negativ,  0 = Positiv(Default)         |    --
--     ------+----------------------------------------------------------------------|    --
--    [5..3] | Strobe-Puls-Breite, Entprellzeit in in 2x 100ns;                     |    --
--           | Vorgabe Exponent (x) für Pulsbreite: Wertebereich 100ns..12,8 µs *)  |    --
--     ------+----------------------------------------------------------------------|    --
--       2   | DAC-Strobe:   | 0 = Holec_Mode (4x) (Default), 1 = Strobe (1x)       |    --
--     ------+----------------------------------------------------------------------|    --
--           |  Output-Mode:                                                        |    --
--           |    "11" =  FG  mit Strobe                                            |    --
--    [1..0] |    "10" =  frei                                                      |     --
--           |    "01" =  16 Bit Output, Strobe = wr auf AW_Output_Reg1(0)          |    --
--           |    "00" =  16 Bit-Dac,    Strobe = wr auf AW_Output_Reg2 (Default)   |    --
--     ------+----------------------------------------------------------------------+    --
      
      
--    ##################################################################################################      
--    #                                                                                                #
--    #                                      DAC                                                       #
--    #                 Output-Daten von den AWOut_Registern oder vom FG mit Strobe                    #
--    #                                                                                                #
--    ##################################################################################################      

--                 +-----------------------------------------------+-----------------------------------------------+
--                 |                 AW_Output_Reg. 2              |                 AW_Output_Reg. 1              |
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|  16 Bit DAC 
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--              +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+    
--              |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|    Output-Daten vom FG (unipol.)
--              +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+    
--                 |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|  Output-Daten vom FG (bipol.)  
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                                               |
--    25pol. SubD  |                   DAC 16Bit                   |                                 
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+                    
--  |     Pin #    |22| 9|21| 8|20| 7|19| 6|18| 5|17| 4|16| 3|15| 2|   24   |
--  +---------------==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+                        
--  |  Daten-Bit#  |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| Strobe | <-+-- AW_Output_Reg1(0)   (Default-Mode)
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+   |-- Strobe write 16-Bit (DAC-Mode bipol./unipol.) 
--                                                                              +-- FG-Str.             (FG-Mode)      
-- P25IO_Holec_Strobe_Out    
-- P25IO_Holec_Strobe_Start
  
      P25IO_DAC_Mode  <=  AW_Config1(1) & AW_Config1(0);         -- Output-Betriebsart

      case P25IO_DAC_Mode is

  
        when "00"  =>  -- DAC16-Mode
              
              P25IO_DAC_Data_FG_Out(15 DOWNTO 0) <=  AW_Output_Reg(2)(15 DOWNTO 0); ------------------------- Bipolar
    
              P25IO_DAC_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config1)(5 downto 3)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n

              P25IO_Holec_Strobe_Start   <=  (AWOut_Reg2_wr AND SCU_Ext_Wr_fin);              -- Software-Strobe   
              P25IO_DAC_DAC_Strobe_i     <=  (AWOut_Reg2_wr AND SCU_Ext_Wr_fin);              -- Software-Strobe   

              IF  (AW_Config1(2) = '0')  THEN  P25IO_DAC_Strobe  <=  NOT P25IO_Holec_Strobe_Out; -- Strobe positiv
                                         Else  

                                           IF  (AW_Config1(6) = '0')  THEN  P25IO_DAC_Strobe  <=  NOT P25IO_DAC_DAC_Strobe_o; -- Strobe positiv
                                                                      Else  P25IO_DAC_Strobe  <=      P25IO_DAC_DAC_Strobe_o; -- Strobe negativ
                                           END IF; 
              END IF; 

              
        when "01"  =>  -- Output-Mode(Default):
        
              P25IO_DAC_Data_FG_Out(15 DOWNTO 0) <=  AW_Output_Reg(2)(15 DOWNTO 0);
              P25IO_DAC_Strobe                   <=  NOT AW_Output_Reg(1)(0);
              
                
        when "10"  =>  -- frei:

              P25IO_DAC_Data_FG_Out(15 downto 0) <= (OTHERS => '0');  
              P25IO_DAC_Strobe                   <= '0';
 

        when "11"  =>  -- FG-Mode:
              
              IF  (AW_Config1(2) = '0')  THEN  P25IO_DAC_Data_FG_Out(15 DOWNTO 0) <=  FG_1_sw(31 downto 16); -- Bipolar
                                         Else  P25IO_DAC_Data_FG_Out(15 DOWNTO 0) <=  FG_1_sw(30 downto 15); -- Unipolar
              END IF; 
    
              P25IO_DAC_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config1)(5 downto 3)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n


              P25IO_Holec_Strobe_Start   <=  FG_1_strobe;                   -- FG_1_strobe (vom Funktionsgen) 
              P25IO_DAC_DAC_Strobe_i     <=  FG_1_strobe;                   -- FG_1_strobe (vom Funktionsgen)

              IF  (AW_Config1(2) = '0')  THEN  P25IO_DAC_Strobe  <=  NOT P25IO_Holec_Strobe_Out; -- Strobe positiv
                                         Else  

                                           IF  (AW_Config1(6) = '0')  THEN  P25IO_DAC_Strobe  <=  NOT P25IO_DAC_DAC_Strobe_o; -- Strobe positiv
                                                                      Else  P25IO_DAC_Strobe  <=      P25IO_DAC_DAC_Strobe_o; -- Strobe negativ
                                           END IF; 
              END IF; 

      end case;


   
    --############################ Einstellen der Output-Polarität ##################################

    IF  (AW_Config1(7) = '0')  THEN  P25IO_DAC_Out(15 DOWNTO 0) <=  NOT P25IO_DAC_Data_FG_Out(15 downto 0); -- Output negativ 
                               Else  P25IO_DAC_Out(15 DOWNTO 0) <=      P25IO_DAC_Data_FG_Out(15 downto 0); -- Output positiv 
    END IF; 


    --    Output DAC-Daten

      PIO(77) <=  '0';  -----+------------------------- Output_Enable (nach init vom ALTERA)
      PIO(79) <=  '0';  -----+
      PIO(81) <=  '0';  -----+
      PIO(83) <=  '0';  -----+
      PIO(95) <=  '0';  -----+

     (PIO(107), PIO(109), PIO(111), PIO(113), PIO(115), PIO(117), PIO(119), PIO(121))  <=  P25IO_DAC_Out(15 downto 8);  -- Output Bit-[15..8]  
     (PIO(123), PIO(125), PIO(127), PIO(129), PIO(131), PIO(133), PIO(135), PIO(137))  <=  P25IO_DAC_Out( 7 downto 0);  -- Output Bit-[ 8..0] 
                                                                            PIO(105)   <=  P25IO_DAC_Strobe;            -- Strobe-Output

                                                                            
--           +======================================================================+   
--           |         User-Config-Register 1 (AW_Config1)                          |   
--           +======================================================================+   
--           |                              ADC-Mode                                |
--     ------+=======================================================================   
--     15-12 | frei                                                                 |   
--     ------+----------------------------------------------------------------------|   
--      11   | Output-Polarität Lemo,   1 = Negativ,  0 = Positiv(Default)          |   
--     ------+----------------------------------------------------------------------|   
--      10   | Triggerflanke: | 1 = neg. Flanke ist Trigger                         |
--           |                | 0 = pos. Flanke ist Trigger                         |
--     ------+----------------+-----------------------------------------------------|   
--       9   | ECC_Start:     | 0 = write auf AW_Output_Reg.(1)(1) (Default)        |   
--           |                | 1 = Input ext. Timing                               |   
--     ------+----------------------------------------------------------------------|   
--       8   | Input-Mode:    | 0 = Input, 1 = Input mit Strobe                     |
--     ------+----------------------------------------------------------------------+   


--    ##################################################################################################      
--    #                                                                                                #
--    #                                      ADC-Input                                                 #
--    #                                                                                                #
--    ##################################################################################################      


--                 +-----------------------------------------------+-----------------------------------------------+
--                 |                  AW_Input_Reg. 2              |                  AW_Input_Reg. 1              |
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                                               |                                             
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+                                       
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| 16 Bit ADC                                       
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+                                    
--                 |                                               |                       +----------------------------*               
--    25pol. SubD  |                  ADC 16 Bit                   |                       |    AW_Input_Reg.(1)(0)     |                  
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=======================+============================+                       
--  |     Pin #    |12|24|11|23|10|22| 9|21| 8|20| 7|19| 6|18| 5|17|             15        |              4             |                       
--  +---------------==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=======================+============================+                       
--  |  Daten-Bit#  |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| ECC (Enable-Conv-CMD) | Strobe: EOC (End-of-Conv.) |                       
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=======================+============================+                       
--                                                                 |  AW_Output_Reg.(1)(1) |                                            
--                                                                 +-----------------------+                                                  
--                                                                                                                    


    --############################# Signal-Input-Daten über Piggy-Stecker JPIO1 ###################################

    
    P25IO_ADC_IN(15 downto 8)  <=  not (PIO(41), PIO(43), PIO(37), PIO(39), PIO(33), PIO(35), PIO(49), PIO(51));  -- Input "HI[7..0]" 
    P25IO_ADC_IN(7  downto 0)  <=  not (PIO(53), PIO(55), PIO(45), PIO(47), PIO(57), PIO(59), PIO(61), PIO(63));  -- Input "Lo[7..0]"   
    
    P25IO_ADC_Strobe_in    <=    not PIO(65); -- EOC (Input Strobe für ADC-Daten)
    

   --################################ Input: Debounce oder Synchr.  ##################################

    Deb60_in(15 DOWNTO 0)              <=  P25IO_ADC_IN(15 downto 0);  -- Entprellung der Input-Daten
    Deb60_in(16)                       <=  P25IO_ADC_Strobe_in;        -- Entprellung des Input "Strobe" 

    Syn60_in(15 DOWNTO 0)              <=  P25IO_ADC_IN(15 downto 0);  -- Synchronisation der Input-Daten
    Syn60_in(16)                       <=  P25IO_ADC_Strobe_in;        -- Synchronisation des Input "Strobe" 


    IF  (Diob_Config1(11) = '0')  THEN                               -- 0 = Entprellung "Eingeschaltet" 
      P25IO_ADC_Input(15 DOWNTO 0)   <=  Deb60_out(15 DOWNTO 0);     -- Debounce-Input-Daten   (Dafault)
      P25IO_ADC_Strobe               <=  Deb60_out(16);              -- Debounce-Input-Strobe  (Dafault)
    ELSE    
      P25IO_ADC_Input(15 DOWNTO 0)   <=  Syn60_out(15 DOWNTO 0);     -- Syn-Input-Daten
      P25IO_ADC_Strobe               <=  Syn60_out(16);              -- Syn-Input-Strobe
    END IF;

   --################################   Daten FF mit Strobe      ##################################

    P25IO_ADC_Data_FF_i(15 DOWNTO 0)    <=  P25IO_ADC_Input(15 downto 0);  -- Input zum Daten-Input_FF

    IF  (AW_Config1(10) = '0')  THEN  P25IO_ADC_Strobe_i <=  NOT P25IO_ADC_Strobe; -- pos. Flanke vom Strobe (Default)
                                Else  P25IO_ADC_Strobe_i <=      P25IO_ADC_Strobe; -- neg. Flanke vom Strobe  
    END IF; 
  
  
 --################################      Input oder ADC-Input-Mode     ##################################
        
    IF  (AW_Config1(8) = '0')  THEN                                            -- 0 = Input-Mode
        AW_Input_Reg(2)(15 DOWNTO 0)    <=  P25IO_ADC_Input(15 DOWNTO 0);      -- Daten-Input  Deb/Syn
        AW_Input_Reg(1)(0)              <=  P25IO_ADC_Strobe;                  -- Deb/Syn ADC-Strobe-Input
      ELSE    
        AW_Input_Reg(1)(0)              <=  P25IO_ADC_Strobe_o;                -- Daten-Strobe für die Input-Daten
        AW_Input_Reg(2)(15 DOWNTO 0)    <=  P25IO_ADC_Data_FF_o(15 DOWNTO 0);  -- Input-Register-Daten
    END IF;
   

   --################################  Input ECC (Enable_Convert_Command)  ##################################
   
    P25IO_Ext_Tim_deb_i     <=  not PIO(85);            -- input(Debounce) "LemoBuchse-Ext_Timing" H-Aktiv, nach dem Optokoppler aber L-Aktiv
    P25IO_Ext_Tim_syn_i     <=  not PIO(85);            -- input(Synch.)   "LemoBuchse-Ext_Timing" H-Aktiv, nach dem Optokoppler aber L-Aktiv
    PIO(93)                 <=  P25IO_nLED_Ext_Tim_o;   -- Output "nLED_Ext_Timing"
   

   --################################  ECC Pulsbreite (Enable_Convert_Command)  ##################################

   IF  (AW_Config1(9) = '0')  THEN                                            -- 0 = Ecc-Intern
        P25IO_ECC_Puls_i      <=  (AWOut_Reg1_wr AND AW_Output_Reg(1)(1) AND SCU_Ext_Wr_fin);    -- Software-Strobe   
      ELSE    
        P25IO_ECC_Puls_i      <=  P25IO_Ext_Tim_syn_o;              -- Ext. Trigger  
   END IF;

   P25IO_ECC_Strobe_i         <=  P25IO_ECC_Puls_o;
   
   PIO(101)  <=  not P25IO_ECC_Strobe_o;   -- ECC (Enable-Convert-CMD) 
   

    
    --#################################################################################
    --#################################################################################


 
  
  WHEN   c_AW_OCin.ID =>

    --#################################################################################
    --####                    Anwender-IO: OCin -- FG900_720                        ###
    --#################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-8  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--       7   | Output-Polarität Daten,  1 = Negativ,  0 = Positiv(Default)               --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --


      extension_cid_system <= c_cid_system;    -- extension card: CSCOHW
      extension_cid_group  <= c_AW_OCin.CID;   -- extension card: cid_group, "FG900720_OCin1" = 29

      Max_AWOut_Reg_Nr     <= 1;  -- Maximale AWOut-Reg-Nummer der Anwendung
      Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
      Min_AWIn_Deb_Time    <= 2;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    
      --############################# Set Debounce-Time ######################################
        
      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        
  
      IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
          Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
      ELSE
          Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;

      
      --################################### Set LED's ########################################

      s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
      s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
      s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 
    
    
    --################################ Debounce, Input's ####################################
    
      OCIN_Data1_in(15 downto 14) <=      ('0', '0'); ------------------------------------------------------------------------ Frei
      OCIN_Data1_in(13 downto  8) <=  not (PIO(73),  PIO(75),  PIO(93),  PIO(95),  PIO(97),  PIO(99)); ----------------------- Input "B[5..0]"
      OCIN_Data1_in(7  downto  0) <=  not (PIO(117), PIO(119), PIO(121), PIO(123), PIO(133), PIO(135), PIO(137), PIO(139)); -- Input "A[7..0]"
    
      OCIN_Data2_in(15 downto 8)  <=  not (PIO(81),  PIO(79),  PIO(77),  PIO(83),  PIO(85),  PIO(87),  PIO(89),  PIO(91));   -- Input "D[7..0]"
      OCIN_Data2_in(7  downto 0)  <=  not (PIO(109), PIO(111), PIO(113), PIO(115), PIO(125), PIO(127), PIO(129), PIO(131));  -- Input "C[7..0]"

      Deb60_in(15 downto 0)   <= OCIN_Data1_in(15 downto 0);   -- Bebounce-Inputs         
      Deb60_in(31 downto 16)  <= OCIN_Data2_in(15 downto 0);   -- Bebounce-Inputs         

      Syn60_in(15 downto 0)   <= OCIN_Data1_in(15 downto 0);   -- Bebounce-Inputs         
      Syn60_in(31 downto 16)  <= OCIN_Data2_in(15 downto 0);   -- Bebounce-Inputs         
      
      
   --###################### Input's ==> AW_Input_Reg(1)/AW_Input_Reg(2) ########################
      
      IF  (Diob_Config1(11) = '0')  THEN                        -- 0 = Entprellung "Eingeschaltet" 
          AW_Input_Reg(1)(15 downto 0)  <=  Deb60_out(15 downto 0);    -- Input mit Entprellung
          AW_Input_Reg(2)(15 downto 0)  <=  Deb60_out(31 downto 16);   -- Input mit Entprellung
      ELSE
          AW_Input_Reg(1)(15 downto 0)  <=  Syn60_out(15 downto 0);    -- Input ohne Entprellung
          AW_Input_Reg(2)(15 downto 0)  <=  Syn60_out(31 downto 16);   -- Input ohne Entprellung
      END IF;
    
      
   --####################### Output: AW_Output_Reg(1) ############################
    
      PIO(39)   <=  '0';  ------------------------------- Output_Enable (nach init vom ALTERA)

      IF  (AW_Config1(7) = '0')  THEN -- positiv
        PIO(49) <=  not  AW_Output_Reg(1)(3);  --  Output "2CB2", '0' am Output --> Optokoppler EIN
        PIO(47) <=  not  AW_Output_Reg(1)(2);  --  Output "2CA2"
        PIO(45) <=  not  AW_Output_Reg(1)(1);  --  Output "1CB2"
        PIO(43) <=  not  AW_Output_Reg(1)(0);  --  Output "1CA2"
      Else                                                                                                                              
        PIO(49) <=       AW_Output_Reg(1)(3);  --  Output "2CB2"
        PIO(47) <=       AW_Output_Reg(1)(2);  --  Output "2CA2"
        PIO(45) <=       AW_Output_Reg(1)(1);  --  Output "1CB2"
        PIO(43) <=       AW_Output_Reg(1)(0);  --  Output "1CA2"
      END IF;  


    
  WHEN   c_AW_OCIO1.ID | c_AW_OCIO2.ID  =>    --- OCIO1 oder OCIO2=>
  
    --#################################################################################
    --####                      Anwender-IO: OCIO -- FG900_730                      ###
    --#################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-8  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--       7   | Output-Polarität Daten,  1 = Negativ,  0 = Positiv(Default)               --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --


      extension_cid_system <= c_cid_system;    -- extension card: CSCOHW

    if  ( AW_ID(7 downto 0) = c_AW_OCIO1.ID) then
      extension_cid_group  <= c_AW_OCIO1.CID; -- extension card: cid_group, "FG900730_OCIO1"
    else
      extension_cid_group  <= c_AW_OCIO2.CID; -- extension card: cid_group, "FG900731_OCIO2"
    end if;
      
      Max_AWOut_Reg_Nr     <= 1;  -- Maximale AWOut-Reg-Nummer der Anwendung
      Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
      Min_AWIn_Deb_Time    <= 2;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

      
      --############################# Set Debounce-Time ######################################
        
      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        
  
      IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
          Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
      ELSE
          Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;

      
      --################################### Set LED's ########################################
    
      s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
      s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
      s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 
    

    --####################### Debounce, Input's AW_Input_Reg(1)/AW_Input_Reg(2) ############################
    
      OCIO_Data1_in(15 downto 0) <=  not (PIO(45),  PIO(47),  PIO(51),  PIO(53),  PIO(37), PIO(39), PIO(41), PIO(43),   -- input "C[7..0]"
                                          PIO(127), PIO(125), PIO(123), PIO(121), PIO(99), PIO(97), PIO(95), PIO(93));  -- input "A[7..0]"
               
      OCIO_Data2_in(7 downto 0)  <=  not (PIO(89), PIO(91), PIO(119), PIO(117),  --  input "D[7..4]"
                                          PIO(87), PIO(85), PIO(83),  PIO(81));  --  input "D[3..0]"

                                          
      Deb60_in(15 downto 0)   <= OCIO_Data1_in(15 downto 0);    -- Bebounce-Inputs         
      Deb60_in(23 downto 16)  <= OCIO_Data2_in( 7 downto 0);    -- Bebounce-Inputs         

      Syn60_in(15 downto 0)   <= OCIO_Data1_in(15 downto 0);    -- Bebounce-Inputs         
      Syn60_in(23 downto 16)  <= OCIO_Data2_in( 7 downto 0);    -- Bebounce-Inputs         

      
   --###################### Input's ==> AW_Input_Reg(1)/AW_Input_Reg(2) ########################
      
      IF  (Diob_Config1(11) = '0')  THEN                        -- 0 = Entprellung "Eingeschaltet" 
          AW_Input_Reg(1)(15 downto 0)  <=  Deb60_out(15 downto 0);      -- Input mit Entprellung
          AW_Input_Reg(2)(15 downto 8)  <=  (OTHERS => '0');             -- Inputs = 0; ;      -- Input mit Entprellung
          AW_Input_Reg(2)( 7 downto 0)  <=  Deb60_out(23 downto 16);     -- Input mit Entprellung
      ELSE
          AW_Input_Reg(1)(15 downto 0)  <=  Syn60_out(15 downto 0);      -- Input ohne Entprellung
          AW_Input_Reg(2)(15 downto 8)  <=  (OTHERS => '0');             -- Inputs = 0;
          AW_Input_Reg(2)( 7 downto 0)  <=  Syn60_out(23 downto 16);     -- Input ohne Entprellung
      END IF;

    
    
    --####################### Output: AW_Output_Reg(1) ############################

      PIO(77)   <=  '0';  ------------------------------- Output_Enable (nach init vom ALTERA) -- FG 900.730
      PIO(79)   <=  '0';  ------------------------------- Output_Enable (nach init vom ALTERA) -- FG 900.731
      
      IF  (AW_Config1(7) = '0')  THEN -- positiv
      PIO(105)  <=  not AW_Output_Reg(1)(11); ----------------  Output "CD2"
      PIO(61)   <=  not AW_Output_Reg(1)(10); ----------------  Output "CC2"
      PIO(107)  <=  not AW_Output_Reg(1)(9);  ----------------  Output "CB2"
      PIO(115)  <=  not AW_Output_Reg(1)(8);  ----------------  Output "CA2"

      (PIO(109), PIO(111), PIO(113), PIO(101), PIO(103), PIO(59), PIO(57), PIO(55))  <=  not AW_Output_Reg(1)(7 downto 0);  --  Output "B[7..0]"  

      Else                                                                                                                              
      PIO(105)  <=      AW_Output_Reg(1)(11); ----------------  Output "CD2"
      PIO(61)   <=      AW_Output_Reg(1)(10); ----------------  Output "CC2"
      PIO(107)  <=      AW_Output_Reg(1)(9);  ----------------  Output "CB2"
      PIO(115)  <=      AW_Output_Reg(1)(8);  ----------------  Output "CA2"

      (PIO(109), PIO(111), PIO(113), PIO(101), PIO(103), PIO(59), PIO(57), PIO(55))  <=      AW_Output_Reg(1)(7 downto 0);  --  Output "B[7..0]"  

      END IF;  

      
  WHEN   c_AW_UIO.ID =>
  
    --#####################################################################################
    --####                       Anwender-IO: UIO  -- FG900_740                         ###
    --#####################################################################################
    
    
--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-11 | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      10   | Input-Mode;  1 = High-Side Input-Mode                                     --
--           |              0 = Low-Side  Input-Mode(Default)                            --
--     ------+-----------------------------------------------------------------------    --
--      9    | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      8    | Output-Polarität Lemo,         1 = Negativ,  0 = Positiv(Default)         --
--      7    | Output-Polarität Bit [23..0],  1 = Negativ,  0 = Positiv(Default)         --
--     ------+-----------------------------------------------------------------------    --
--      6    | Enable Output-Lemo,            1 = Enable,   0 = Disable(Default)         --
--      5    | Enable Output-Bit [23..20],    1 = Enable,   0 = Disable(Default)         --
--      4    | Enable Output-Bit [19..16],    1 = Enable,   0 = Disable(Default)         --
--      3    | Enable Output-Bit [15..12],    1 = Enable,   0 = Disable(Default)         --
--      2    | Enable Output-Bit [11..8],     1 = Enable,   0 = Disable(Default)         --
--      1    | Enable Output-Bit [7..4],      1 = Enable,   0 = Disable(Default)         --
--      0    | Enable Output-Bit [3..0],      1 = Enable,   0 = Disable(Default)         --
--     ------+-----------------------------------------------------------------------    --
    
    
    
--           +=======================================================================    --
--           |         User-Config-Register 2 (AW_Config2)                               --
--     ------+=======================================================================    --
--     15-6  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      5    | Strobe-Polarität,         1 = Negativ,  0 = Positiv(Default)              --
--     ------+-----------------------------------------------------------------------    --
--    [4..2] | Strobe-Puls-Breite, Entprellzeit in in 2x 100ns;                          --
--           | Vorgabe Exponent (x) für Pulsbreite: Wertebereich 100ns..12,8 µs *)       --
--     ------+-----------------------------------------------------------------------    --
--      1    |  Output-Mode:                                                             --
--      0    | "00" = Output, "01" = 16 Bit-Dac, "10" = 32 Bit-Dac,  "11" = FG           --
--     ------+-----------------------------------------------------------------------    --
  
  
    extension_cid_system <= c_cid_system;    -- extension card: CSCOHW
    extension_cid_group  <= c_AW_UIO.CID;    -- extension card: cid_group, "FG900740_UIO1"

    Max_AWOut_Reg_Nr     <= 2;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us


    --############################# Set Debounce-Time ######################################
        
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################

    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 

    
    --################### Output-Daten von den AWOut_Registern oder dem FG mit Strobe ##################


--                 +-----------------------------------------------+-----------------------------------------------+
--                 |                 AW_Output_Reg. 2              |                 AW_Output_Reg. 1              |
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|    
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                                               |        +--------+  |                         |  
--                 |  +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ 18 Bit |  |                         |  
--                 |  |17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|   DAC  |  |                         +--> Lemo_Buchse
--                 |  +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ unipol.|  |
--    37pol. SubD  |                                               |        +--------+  |                          
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+               
--  |     Pin #    |30|29|28|27|26|25|24|23|22|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3|
--  +--------------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+               
--  |  Daten-Bit#  |23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|<-+-- AW_Output_Reg1(8)   (Default-Mode)   
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+  +-- Strobe write 16-Bit (DAC-Mode 16) 
--                 |                                                                    |     +-- Strobe write 32-Bit (DAC-Mode 32) 
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+     +-- FG-Str.             (FG-Mode)        
--                 |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|9 |                                                    
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                         Output-Daten vom FG                        | 
--                 +--------------------------------------------------------------------+
  
  
      UIO_Mode  <=  AW_Config2(1)& AW_Config2(0);         -- Output-Betriebsart

      case UIO_Mode is

        when "00"  =>  -- Output-Mode(Default): Out[23..8] = AW_Output_Reg2[15..0], Out[7..0] = AW_Output_Reg1[15..8] 
              
              UIO_Data_FG_Out(23 DOWNTO 0) <=  (AW_Output_Reg(2)(15 DOWNTO 0) & AW_Output_Reg(1)(15 DOWNTO 8));
  
  
        when "01"  =>  -- DAC16-Mode:  Output-Mode: Out[23..8] = AW_Output_Reg2[15..0], Out[7..1] = AW_Output_Reg1[15..7], Out[0] = DAC-Strobe 
              
              UIO_Reg_Out_i(23 DOWNTO 8)    <=  AW_Output_Reg(2)(15 DOWNTO 0);          --  Daten zum Register
              UIO_Reg_Out_i( 7 DOWNTO 1)    <=  (others => '0');                        --  Daten zum Register
              UIO_Reg_Enable                <=  AWOut_Reg2_wr;                          --  Enable für Daten Register
              UIO_Data_FG_Out(23 DOWNTO 1)  <=  UIO_Reg_Out_o(23 DOWNTO 1);             --  Register-Daten zum Ausgang
    
              UIO_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config2)(4 downto 2)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n
              UIO_DAC_Strobe_i     <=  (AWOut_Reg2_wr AND SCU_Ext_Wr_fin);              -- Software-Strobe   

              IF  (AW_Config2(5) = '0')  THEN  UIO_Data_FG_Out(0) <=      UIO_DAC_Strobe_o; -- Strobe positiv
                                         Else  UIO_Data_FG_Out(0) <=  not UIO_DAC_Strobe_o; -- Strobe negativ
              END IF; 
              
        when "10"  =>  -- DAC32-Mode:  Output-Mode: Out[23..8] = AW_Output_Reg2[15..0], Out[7..1] = AW_Output_Reg1[15..7], Out[0] = DAC-Strobe 
              
              UIO_Reg_Out_i(23 DOWNTO 8)    <=  AW_Output_Reg(2)(15 DOWNTO 0);          --  Daten zum Register
              UIO_Reg_Out_i( 7 DOWNTO 1)    <=  AW_Output_Reg(1)(15 DOWNTO 9);          --  Daten zum Register
              UIO_Reg_Enable                <=  AWOut_Reg2_wr;                          --  Enable für Daten Register
              UIO_Data_FG_Out(23 DOWNTO 1)  <=  UIO_Reg_Out_o(23 DOWNTO 1);             --  Register-Daten zum Ausgang
    
              UIO_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config2)(4 downto 2)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n
              UIO_DAC_Strobe_i     <=  (AWOut_Reg2_wr AND SCU_Ext_Wr_fin);              -- Software-Strobe   

              IF  (AW_Config2(5) = '0')  THEN  UIO_Data_FG_Out(0) <=      UIO_DAC_Strobe_o; -- Strobe positiv
                                         Else  UIO_Data_FG_Out(0) <=  not UIO_DAC_Strobe_o; -- Strobe negativ
              END IF; 
              
  
        when "11"  =>  -- FG-Mode:     Output-Mode: Out[23..1] = FG_Daten[31..9], Out[0] = FG_DAC-Strobe 
              
              UIO_Data_FG_Out(23 downto 1) <= FG_1_sw(31 downto 9);                     -- gespeicherte FG-Daten zum Ausgang
  
              UIO_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config2)(4 downto 2)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n
              UIO_DAC_Strobe_i     <=  FG_1_strobe;                                     -- FG_1_strobe (vom Funktionsgen)

              IF  (AW_Config2(5) = '0')  THEN  UIO_Data_FG_Out(0) <=      UIO_DAC_Strobe_o; -- Strobe positiv
                                         Else  UIO_Data_FG_Out(0) <=  not UIO_DAC_Strobe_o; -- Strobe negativ
              END IF; 
                
      end case;

    
    --############################ Einstellen der Output-Polarität ##################################

    IF  (AW_Config1(7) = '1')  THEN  UIO_Out(23 DOWNTO 0) <=  Not UIO_Data_FG_Out(23 downto 0); -- Output negativ 
                               Else  UIO_Out(23 DOWNTO 0) <=      UIO_Data_FG_Out(23 downto 0); -- Output positiv 
    END IF; 

    --################################### Output-Enable ##################################

    PIO(132) <=  not AW_Config1(6);  ---- Output_Enable (Lemo)
    PIO(24)  <=  not AW_Config1(5);  ---- Output_Enable (IO_20-23)
    PIO(26)  <=  not AW_Config1(4);  ---- Output_Enable (IO_16-19)
    PIO(28)  <=  not AW_Config1(3);  ---- Output_Enable (IO_12-15)
    PIO(30)  <=  not AW_Config1(2);  ---- Output_Enable (IO_8-11)
    PIO(22)  <=  not AW_Config1(1);  ---- Output_Enable (IO_4-7)
    PIO(20)  <=  not AW_Config1(0);  ---- Output_Enable (IO_3-0)
      
      
    --########################## Daten zum Piggy-Stecker JPIO1 ###########################


    (PIO(69), PIO(71), PIO(73), PIO(75),  PIO(61), PIO(63), PIO(65), PIO(67)) <=  UIO_Out(23 downto 16); --- Output-Pins zum Piggy [23..16]
    (PIO(62), PIO(66), PIO(70), PIO(74),  PIO(78), PIO(82), PIO(86), PIO(90)) <=  UIO_Out(15 downto 8);  --- Output-Pins zum Piggy [15.. 8]
    (PIO(77), PIO(79), PIO(81), PIO(83),  PIO(85), PIO(87), PIO(89), PIO(91)) <=  UIO_Out(7  downto 0);  --- Output-Pins zum Piggy [ 7.. 0]


    --####################### Lemo: Output, Polarität und LED ############################

    IF  (AW_Config1(8) = '1')  THEN
            PIO(133)    <=  Not AW_Output_Reg(1)(0);   -- Lemo-Output negativ 
        Else
            PIO(133)    <=      AW_Output_Reg(1)(0);   -- Lemo-Output positiv 
    END IF; 

    UIO_LED_Lemo_Out_i  <=      AW_Output_Reg(1)(0);   --  Input  "nLED_Lemo_Out"
    PIO(141)            <=      UIO_nLED_Lemo_Out_o;   --  Output "nLED_Lemo_Out"


    
    --############################# High-Signal-Input-Daten vom Piggy-Stecker JPIO1 ###################################

    UIO_HS_IN(23 downto 0)  <= not (PIO(103), PIO(99),  PIO(95),  PIO(57),  PIO(53),  PIO(49),  PIO(45),  PIO(41),     
                                    PIO(37),  PIO(115), PIO(111), PIO(107), PIO(114), PIO(110), PIO(106), PIO(102),     
                                    PIO(98),  PIO(94),  PIO(56),  PIO(52),  PIO(48),  PIO(44),  PIO(40),  PIO(36));    
    

    --############################# Low-Signal-Input-Daten vom Piggy-Stecker JPIO1 #####################################
    
    UIO_LS_IN(23 downto 0)  <= not (PIO(101), PIO(97),  PIO(93),  PIO(55),  PIO(51),  PIO(47),  PIO(43),  PIO(39),    
                                    PIO(35),  PIO(113), PIO(109), PIO(105), PIO(116), PIO(112), PIO(108), PIO(104),    
                                    PIO(100), PIO(96),  PIO(58),  PIO(54),  PIO(50),  PIO(46),  PIO(42),  PIO(38));    

    --############################################### Lemo-Input #######################################################

    UIO_Lemo_in      <=  NOT PIO(127);        --  Input "Lemo-Buchse" 

    

   --################################ Debounce, Input 24-Bit und Lemo  ##################################

    
    IF  (AW_Config1(9) = '0')  THEN
        Syn60_in(23 DOWNTO 0)   <=  UIO_LS_IN(23 DOWNTO 0);  -- Low-Side  Syn-Input-Daten
        Deb60_in(23 DOWNTO 0)   <=  UIO_LS_IN(23 DOWNTO 0);  -- Low-Side  Debounce-Input-Daten
    Else  
        Syn60_in(23 DOWNTO 0)   <=  UIO_HS_IN(23 DOWNTO 0);  -- High-Side Syn-Input-Daten
        Deb60_in(23 DOWNTO 0)   <=  UIO_HS_IN(23 DOWNTO 0);  -- High-Side Debounce-Input-Daten  
    END IF; 


    UIO_Lemo_deb_i            <=  UIO_Lemo_in;                  --  Bebounce: Input "Lemo-Buchse" 
    UIO_LED_Lemo_In_i         <=  UIO_Lemo_deb_o;               --  Input  "nLED_Lemo_IN"
    PIO(139)                  <=  UIO_nLED_Lemo_In_o;           --  Output "nLED_Lemo_IN"
    
   --########################### Input's 24-Bit und Lemo ==> AW_Input_Reg(1)/AW_Input_Reg(2) #################################
        
    IF  (Diob_Config1(11) = '0')  THEN                                -- 0 = Entprellung "Eingeschaltet" 
        AW_Input_Reg(2)(15 DOWNTO 0)  <=  Deb60_out(23 DOWNTO 8);     -- Debounce-Output zum AW_Input_Reg(2)-Register (mit Entprellung)
        AW_Input_Reg(1)(15 DOWNTO 8)  <=  Deb60_out( 7 DOWNTO 0);     -- Debounce-Output zum AW_Input_Reg(1)-Register (mit Entprellung)
        AW_Input_Reg(1)(0)            <=  UIO_Lemo_deb_o;             -- Input "Lemo-Buchse" ===== entprellt =====
    ELSE    
        AW_Input_Reg(2)(15 DOWNTO 0)  <=  Syn60_out(23 DOWNTO 8);     -- Input zum AW_Input_Reg(2)-Register (ohne Entprellung)
        AW_Input_Reg(1)(15 DOWNTO 8)  <=  Syn60_out( 7 DOWNTO 0);     -- Input zum AW_Input_Reg(1)-Register (ohne Entprellung)
        AW_Input_Reg(1)(0)            <=  UIO_Lemo_in;                --  Input "Lemo-Buchse" ===== nicht entprellt =====
    END IF;


    
  WHEN   c_AW_DA.ID =>
      
    --###################################################################################
    --####                  Anwender-IO: DA(DAC/ADC)  -- FG900_750                    ###
    --###################################################################################
--
--                                                                                
--   ----+----------------+-------------------------------------------------------------------- 
--    15 | Test_Mode:     |   0 = Normalbetrieb      
--       |                |   1 = Testbetrieb :  AW_Output_Reg(3) = 1. DAC-Wert
--       |                |                      AW_Output_Reg(4) = 2. DAC-Wert
--       |                |                      AW_Output_Reg(5) = Verzögerungszeit in Taktperioden (8ns)                                                                       
--   ----+----------------+-------------------------------------------------------------------- 
--    14 | frei     
--     | |   | 
--     9 | frei     
--   ----+----------------+------------------------------------------------------------------- 
--     8 | Trigger_Outp_  | 1 = negative Logik (active low)
--       | _Polarität:    | 0 = positive Logik (active high), default      
--   ----+----------------+------------------------------------------------------------------- 
--     7 | frei           |
--   ----+----------------+------------------------------------------------------------------- 
--     6 | frei           |
--   ----+----------------+------------------------------------------------------------------- 
--     5 | Trigger_Output:| 1 = freigegeben (enable), DAC-Strobe-Signal wird auf 
--       |                |     Lemo-Ausgang geschaltet; Puls verlängert auf 1 µs
--       |                | 0 = gesperrt (disable), default; Lemo-Ausgang ist inaktiv    
--   ----+----------------+------------------------------------------------------------------- 
--     4 | FG_mode:       | 1 = Funktiongenerator-Mode, DAC-Werte kommen von FG_Data und 
--       |                |     werden mit FG_Strobe uebernommen. Kein externer Trigger! 
--       |                | 0 = Software-Mode, DAC-Werte, kommen vom SCU-Bus-Slave.      
--   ----+----------------+------------------------------------------------------------------- 
--     3 | Triggerflanke: | 1 = neg. Flanke ist Trigger, wenn ext. Trig. selekt.
--       |                | 0 = pos. Flanke ist Trigger, wenn ext. Trigger gewählt.  
--   ----+----------------+------------------------------------------------------------------- 
--     2 | Triggerquelle: | 1 = extern, externer Trigger ist selektiert
--       |                | 0 = intern, direkt nach der Daten-Übertragung vom SCU-Bus,
--       |                |     wird der DAC-Wert übernommen und am Ausgang eingestellt. 
--   ----+----------------+------------------------------------------------------------------- 
--     1 | Reset-DAC,     | 1 = ein Reset des DACs wird ausgeführt. DAC-Wert wird auf Null gestellt.
--       | CLR_DAC:       |                 Das Bit wird anschließend zurückgesetzt.  
--   ----+----------------+------------------------------------------------------------------- 
--     0 | reserviert für Erweiterung 
--    ---+------------------------------------------------------------------------------------ 


    extension_cid_system <= c_cid_system;   -- extension card: CSCOHW
    extension_cid_group  <= c_AW_DA.CID;    -- extension card: cid_group, "FG900750_DA"

    Max_AWOut_Reg_Nr     <= 2;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 2;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us
    

    Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung

    
    --############################# Set Debounce-Time ######################################
      
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################
    
    s_nLED_User1_i       <= DAC1_Config(4);    -- LED3 = User 1, DAC-Data vom FG1
    s_nLED_User2_i       <= DAC2_Config(4);    -- LED2 = User 2, DAC-Data vom FG2 
    s_nLED_User3_i       <= '0';              -- LED1 = User 3, -- frei --

      
   --################################ Debounce, DA_Trig1 und DA_Trig2  ##################################

    DA_Trig1_deb_i    <=  PIO(43);          -- Lemo: Input Ext-Trigger1   
    DA_Trig2_deb_i    <=  PIO(45);          -- Lemo: Input Ext-Trigger2   
    AW_Input_Reg(1)(0)    <=  DA_Trig1_deb_o;   -- Copy vom Input Ext-Trigger1 auf das Inputregister1(0)
    AW_Input_Reg(1)(1)    <=  DA_Trig2_deb_o;   -- Copy vom Input Ext-Trigger1 auf das Inputregister1(1)

    
  -------------------------------- Testmode für DAC1 ---------------------------------------
    
    IF  (DAC1_Config(15) = '1')  THEN   
    
      DA_DAC1_Str     <=  DAC_tr_Test_Strobe;        -- Output Strobe
      DA_DAC1_Data    <=  DAC_tr_Test_Out;           -- Test-Bitmuster

  --------------------------------- ext. Trigger für DAC1 ---------------------------------------

      elsif  (DAC1_Config(2) = '1')  THEN 

        if   (DAC1_Config(3) = '1')  THEN 
              DA_Trig1_Strobe_i   <=     DA_Trig1_deb_o;           -- Lemo: Debounce-Input Ext-Trigger1 (neg.)
        else    
              DA_Trig1_Strobe_i   <= not DA_Trig1_deb_o;           -- Lemo: Debounce-Input Ext-Trigger1 (pos.)
        end if;

      DA_LED_Ext_Trig1_i         <= DA_Trig1_Strobe_o;      -- LED:  Input Ext-Trigger1 
      PIO(51)                    <= DA_LED_Ext_Trig1_o;     -- LED:  Extrern_Trigger1
      
     
      DA_DAC1_Data(15 downto 0)  <=  DAC1_Out;              -- Output Daten
      DA_DAC1_Str                <=  DA_Trig1_Strobe_o;     -- FG_1_strobe (vom Funktionsgen)

  --------------------------------- FG-Mode für DAC1 ---------------------------------------

      elsif  (DAC1_Config(4) = '1')  THEN 

      DA_DAC1_Data(15 downto 0)  <=  FG_1_sw(31 downto 16);    -- FG1-Output
      DA_DAC1_Str                <=  FG_1_strobe;              -- FG_1_strobe (vom Funktionsgen)

  ----------------------------- SCU-Bus-Daten für DAC1 -------------------------------------
      else

      DA_DAC1_Data       <=   DAC1_Out;                            -- Output Daten

      DA_DAC1_Str_Puls_i <=   DAC1_Out_wr;                         -- DAC1-Output-Strobe
      DA_DAC1_Str        <=  (DA_DAC1_Str_Puls_o or (not rstn_sys));  -- Output Strobe für SCU-Bus Daten und Einschalt-Reset
    
    END IF; 

    
    
  -------------------------------- Testmode für DAC2 ---------------------------------------
    
    IF  (DAC2_Config(15) = '1')  THEN   
    
      DA_DAC2_Str    <=  DAC_Test_Strobe;        -- Output Strobe
      DA_DAC2_Data   <=  DAC_Test_Out;           -- Test-Bitmuster

  --------------------------------- ext. Trigger für DAC2 ---------------------------------------

      elsif  (DAC2_Config(2) = '1')  THEN 

        if   (DAC2_Config(3) = '1')  THEN 
              DA_Trig2_Strobe_i   <=     DA_Trig2_deb_o;         -- Lemo: Debounce-Input Ext-Trigger2 (neg.)
        else    
              DA_Trig2_Strobe_i   <= not DA_Trig2_deb_o;         -- Lemo: Debounce-Input Ext-Trigger2 (pos.)
        end if;   
    
      DA_LED_Ext_Trig2_i  <= DA_Trig2_Strobe_o;           -- LED:  Input Ext-Trigger1 
      PIO(53)             <= DA_LED_Ext_Trig2_o;          -- LED:  Extrern_Trigger1
      
     
      DA_DAC2_Data(15 downto 0)  <=  DAC2_Out;          -- Output Daten
      DA_DAC2_Str                <=  DA_Trig2_Strobe_o;   -- FG_2_strobe (vom Funktionsgen)

  --------------------------------- FG-Mode für DAC2 ---------------------------------------

      elsif  (DAC2_Config(4) = '1')  THEN 

      DA_DAC2_Data(15 downto 0) <=  FG_2_sw(31 downto 16);    -- FG_2-Output
      DA_DAC2_Str               <=  FG_2_strobe;              -- FG_2_Strobe (vom Funktionsgen)

  ----------------------------- SCU-Bus-Daten für DAC2 -------------------------------------
      else

      DA_DAC2_Data       <=   DAC2_Out;                            -- Output Daten
      DA_DAC2_Str_Puls_i <=   DAC2_Out_wr;                         -- DAC2-Output-Strobe
      DA_DAC2_Str        <=  (DA_DAC2_Str_Puls_o or (not rstn_sys));  -- Output Strobe für SCU-Bus Daten und Einschalt-Reset
   
    END IF; 

    
    
 --############## Multiplexer: Reset- oder SCU/FG-Output Daten  ############################################
 
 
    if  (DAC1_Config(1) = '1') and (DAC1_Config_wr = '1')  THEN

       DA_DAC1_Out        <= (OTHERS => '0');     -- Zwischenspeicher
       DA_DAC1_Str_Puls_i <= '1';                 -- 
       DA_DAC1_Str_Out    <= DA_DAC1_Str_Puls_o;  -- Resetpuls --> DAC1-Output-Strobe 
    else
       DA_DAC1_Out        <= DA_DAC1_Data;        -- Zwischenspeicher
       DA_DAC1_Str_Out    <= DA_DAC1_Str;         -- DAC1-Output-Strobe 
    end if;
 
    if  (DAC2_Config(1) = '1') and (DAC2_Config_wr = '1')  THEN

       DA_DAC2_Out        <= (OTHERS => '0');     -- Zwischenspeicher
       DA_DAC2_Str_Puls_i <= '1';                 -- 
       DA_DAC2_Str_Out    <=  DA_DAC2_Str_Puls_o; -- Resetpuls --> DAC2-Output-Strobe 
     else
       DA_DAC2_Out        <= DA_DAC2_Data;        -- Zwischenspeicher
       DA_DAC2_Str_Out    <= DA_DAC2_Str;         -- DAC2-Output-Strobe 
    end if;
 


 --################## DAC-Daten und Strobe zum DIOB-Output-Stecker ######################################
 
 
      PIO(99)   <=  not DA_DAC1_STR_Out;  -- Output Strobe
      PIO(133)  <=  not DA_DAC2_STR_Out;  -- Output Strobe
    

      PIO(97) <= not  DA_DAC1_Out(15); PIO(95)  <= DA_DAC1_Out(14); PIO(93)  <= DA_DAC1_Out(13); PIO(91)  <= DA_DAC1_Out(12);
      PIO(89) <=      DA_DAC1_Out(11); PIO(87)  <= DA_DAC1_Out(10); PIO(85)  <= DA_DAC1_Out(9);  PIO(83)  <= DA_DAC1_Out(8); 
      PIO(81) <=      DA_DAC1_Out(7);  PIO(79)  <= DA_DAC1_Out(6);  PIO(77)  <= DA_DAC1_Out(5);  PIO(75)  <= DA_DAC1_Out(4); 
      PIO(73) <=      DA_DAC1_Out(3);  PIO(71)  <= DA_DAC1_Out(2);  PIO(69)  <= DA_DAC1_Out(1);  PIO(67)  <= DA_DAC1_Out(0); 
    
      PIO(131) <= not DA_DAC2_Out(15); PIO(129) <= DA_DAC2_Out(14); PIO(127) <= DA_DAC2_Out(13); PIO(125) <= DA_DAC2_Out(12);
      PIO(123) <=     DA_DAC2_Out(11); PIO(121) <= DA_DAC2_Out(10); PIO(119) <= DA_DAC2_Out(9);  PIO(117) <= DA_DAC2_Out(8); 
      PIO(115) <=     DA_DAC2_Out(7);  PIO(113) <= DA_DAC2_Out(6);  PIO(111) <= DA_DAC2_Out(5);  PIO(109) <= DA_DAC2_Out(4); 
      PIO(107) <=     DA_DAC2_Out(3);  PIO(105) <= DA_DAC2_Out(2);  PIO(103) <= DA_DAC2_Out(1);  PIO(101) <= DA_DAC2_Out(0); 
    


  ------------------ DAC1_Out-Strobe --------------------


    IF  (DAC1_Config(5)   = '1')  THEN               -- DAC1_Out-Strobe Enable   
      DA_Trig1_i            <=  DA_DAC1_Str_Out;   
      PIO(55)               <=  DA_LED_Trig_Out1_o; -- LED: Trigger DAC1      

      IF  (DAC1_Config(8) = '1')  THEN               -- DAC1_Out-Strobe negativ Enable   
        DA_LED_Trig_Out1_i  <=  DA_DAC1_Str_Out;
        PIO(49)             <=  DA_Trig1_1us_o;     -- Lemo: Trigger_Out1 = neg.
      Else
        DA_LED_Trig_Out1_i  <=  DA_DAC1_Str_Out;
        PIO(49)             <=  not DA_Trig1_1us_o; -- Lemo: Trigger_Out1 = pos.
      end if;

    Else
      DA_LED_Trig_Out1_i    <=  AW_Output_Reg(1)(0);
      PIO(55)               <=  DA_LED_Trig_Out1_o; -- LED: Trigger DAC1      
      PIO(49)               <=  AW_Output_Reg(1)(0);          -- Output-Register1(0) auf "Lemo: Trigger_Out1"
    end if;


  ------------------ DAC2_Out-Strobe --------------------


    IF  (DAC2_Config(5)   = '1')  THEN              -- DAC2_Out-Strobe Enable   
      DA_LED_Trig_Out2_i    <=  DA_DAC2_Str_Out;
      PIO(57)               <=  DA_LED_Trig_Out2_o; -- LED: Trigger DAC2      

      IF  (DAC2_Config(8) = '1')  THEN              -- DAC2_Out-Strobe negativ Enable   
        DA_Trig2_i          <=  DA_DAC2_Str_Out;   
        PIO(47)             <=  DA_Trig2_1us_o;     -- Lemo: Trigger_Out2 = neg.
      Else
        DA_Trig2_i          <=  DA_DAC2_Str_Out;   
        PIO(47)             <=  not DA_Trig2_1us_o; -- Lemo: Trigger_Out2 = pos.
      end if;

    Else
      DA_LED_Trig_Out2_i    <=  AW_Output_Reg(1)(1);
      PIO(57)               <=  DA_LED_Trig_Out2_o; -- LED: Trigger DAC2      
      PIO(47)               <=  AW_Output_Reg(1)(1);      -- Output-Register1(1) auf "Lemo: Trigger_Out2"
    end if;
      

  WHEN   c_AW_Frei.ID =>
      
    --###################################################################################
    --####                  Anwender-IO: DA(DAC/ADC)  -- FG900_760                    ###
    --###################################################################################



  WHEN   c_AW_SPSIO1.ID | c_AW_SPSIOI1.ID  =>    --- SPSIO1 oder SPSIOI1
      
    --###################################################################################
    --####                Anwender-IO: SPSIO1-FG900_770/SPSIOI1-FG901_770             ###
    --###################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-8  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--       7   | Output-Polarität Daten,  1 = Negativ,  0 = Positiv(Default)               --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --


    extension_cid_system <= c_cid_system;    -- extension card: CSCOHW

    if  ( AW_ID(7 downto 0) = c_AW_SPSIO1.ID) then
      extension_cid_group  <= c_AW_SPSIO1.CID;    -- extension card: cid_group, "FG900770_SPSIO1"
    else
      extension_cid_group  <= c_AW_SPSIOI1.CID;   -- extension card: cid_group, "FG901770_SPSIOI1" = (SPSIO-invers)
    end if;
    
    Max_AWOut_Reg_Nr     <= 1;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 2;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    
    --############################# Set Debounce-Time ######################################
      
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################
    
    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 



    --########################### Debounce, Input's  ############################

    SPSIO_Data_in(23 downto 0)  <=  not (PIO(35), PIO(37), PIO(39), PIO(41), PIO(43), PIO(45), PIO(47), PIO(49),    --  Input "23-16"
                                         PIO(51), PIO(53), PIO(55), PIO(57), PIO(59), PIO(61), PIO(63), PIO(65),    --  Input "15-8"
                                         PIO(67), PIO(69), PIO(71), PIO(73), PIO(75), PIO(77), PIO(79), PIO(81));   --  Input "7-0"

    Deb60_in(23 downto 0)  <= SPSIO_Data_in(23 downto 0);    -- Bebounce-Inputs         
    Syn60_in(23 downto 0)  <= SPSIO_Data_in(23 downto 0);    -- Syn-Inputs         

    
   --###################### Input's ==> AW_Input_Reg(1)/AW_Input_Reg(2) ########################
      
      IF  (Diob_Config1(11) = '0')  THEN                                  -- 0 = Entprellung "Eingeschaltet" 
          AW_Input_Reg(2)(15 downto 8)  <=  (OTHERS => '0');              -- Input's = 0; 
          AW_Input_Reg(2)(7  downto 0)  <=  Deb60_out(23 downto 16);      -- Debounce-Output "23-16"
          AW_Input_Reg(1)(15 downto 0)  <=  Deb60_out(15 downto 0);       -- Debounce-Output "15-0"
      ELSE
          AW_Input_Reg(2)(15 downto 8)  <=  (OTHERS => '0');              -- Input's = 0; 
          AW_Input_Reg(2)(7  downto 0)  <=  Syn60_out(23 downto 16);      -- Input "23-16"
          AW_Input_Reg(1)(15 downto 0)  <=  Syn60_out(15 downto 0);       -- Input "15-0"
      END IF;


  --========================== Output Register 1 ======================================
    
    PIO(101)   <=  '0';  ------------------------------- Output_Enable (nach init vom ALTERA)
    PIO(119)   <=  '0';  ------------------------------- Output_Enable (nach init vom ALTERA)


    IF  (AW_Config1(7) = '0')  THEN -- positiv
      (PIO(111), PIO(113), PIO(115), PIO(117), PIO(103), PIO(105), PIO(107), PIO(109))  <=      AW_Output_Reg(1)(7 downto 0);  --  Output "[7..0]"  
    Else                                                                                                                              
      (PIO(111), PIO(113), PIO(115), PIO(117), PIO(103), PIO(105), PIO(107), PIO(109))  <=  not AW_Output_Reg(1)(7 downto 0);  --  Output "[7..0]"  
    END IF;  

      

  WHEN   c_AW_HFIO.ID =>
      
    --###################################################################################
    --####                  Anwender-IO: DA(DAC/ADC)  -- FG900_780                    ###
    --###################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-9  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--       8   | Output-Polarität Lemo,   1 = Negativ,  0 = Positiv(Default)               --
--       7   | Output-Polarität Daten,  1 = Negativ,  0 = Positiv(Default)               --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --

  
    extension_cid_system <= c_cid_system;    -- extension card: CSCOHW
    extension_cid_group  <= c_AW_HFIO.CID;   -- extension card: cid_group, "FG900780_HFIO1"

    Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 1;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    
    --############################# Set Debounce-Time ######################################
      
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################

    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 


    --========================== Output Register 1 ======================================
    
    PIO(133)   <=  '0';  ---------------- Output_Enable (nach init vom ALTERA) für Tastpuls,Sample_Puls_Display und Reserve
    

---------------------------------------------------------------------------------------------------------------------------------------
    HFIO_Aux_i                  <= AW_Output_Reg(1)(7);                      --  Input  "nLED_Aux"
    PIO(35)                     <= HFIO_nLED_Aux_o;                          --  Output "nLED_Aux"

    IF  (AW_Config1(8) = '0')  THEN   -- Output positiv
      PIO(113)                    <=     AW_Output_Reg(1)(7);                --  --- LEMO2 ---  (Frei, J7),  "Aux "
    Else
      PIO(113)                    <= not AW_Output_Reg(1)(7);                --  --- LEMO2 ---  (Frei, J7),  "Aux "
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    IF  (AW_Config1(7) = '0')  THEN   -- Output positiv
      PIO(59)                     <= not AW_Output_Reg(1)(6);                --  D-Sub37-F(J5)_Pin-Nr. 32, OC11, "Flattop-Puls"
    Else
      PIO(59)                     <=     AW_Output_Reg(1)(6);                --  D-Sub37-F(J5)_Pin-Nr. 32, OC11, "Flattop-Puls"
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    HFIO_Tastpuls_i             <=     AW_Output_Reg(1)(5);                  --  Input  "nLED_Tastpuls"
    PIO(39)                     <= HFIO_nLED_Tastpuls_o;                     --  Output "nLED_Tastpuls"

    IF  (AW_Config1(8) = '0')  THEN   -- Output positiv
      PIO(111)                    <= not AW_Output_Reg(1)(5);                --  --- Lemo4 ---  (Tastpuls),          OE15, "Tastpuls(Gating) "
    Else
      PIO(111)                    <=     AW_Output_Reg(1)(5);                --  --- Lemo4 ---  (Tastpuls),          OE15, "Tastpuls(Gating) "
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    IF  (AW_Config1(7) = '0')  THEN   -- Output positiv
      PIO(57)                     <= not AW_Output_Reg(1)(4);                --  D-Sub37-F(J5)_Pin-Nr. 33, OC12, "Tast-Puls inv."
      PIO(55)                     <= not AW_Output_Reg(1)(3);                --  D-Sub37-F(J5)_Pin-Nr. 34, OC13, "Tast-Puls"
    Else
      PIO(57)                     <=     AW_Output_Reg(1)(4);                --  D-Sub37-F(J5)_Pin-Nr. 33, OC12, "Tast-Puls inv."
      PIO(55)                     <=     AW_Output_Reg(1)(3);                --  D-Sub37-F(J5)_Pin-Nr. 34, OC13, "Tast-Puls"
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    HFIO_Sample_Puls_Display_i  <= AW_Output_Reg(1)(2);                      --  Input  "nLED_Sample-Puls-Display"
    PIO(33)                     <= HFIO_nLED_Sample_Puls_Display_o;          --  Output "nLED_Sample_Puls_Display"
   
    IF  (AW_Config1(8) = '0')  THEN   -- Output positiv  
      PIO(115)                    <=     AW_Output_Reg(1)(2);                --  --- LEMO1 ---  (Sample_Puls_Display, J8),       "Sample_Puls_Display"
    Else   
      PIO(115)                    <= not AW_Output_Reg(1)(2);                --  --- LEMO1 ---  (Sample_Puls_Display, J8),       "Sample_Puls_Display"
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    HFIO_Sample_Puls_inv_i      <= AW_Output_Reg(1)(1);                      --  Input  "nLED_Sample_Puls_inv"
    PIO(37)                     <= HFIO_nLED_Sample_Puls_inv_o;              --  Output "nLED_Sample_Puls_inv"

    IF  (AW_Config1(8) = '0')  THEN   -- Output positiv 
      PIO(51)                     <=     AW_Output_Reg(1)(1);                --  --- LEMO3 ---  (Sample_Puls_inv, J6),     OC15, "Sample_Puls_inv. "
    Else  
      PIO(51)                     <= not AW_Output_Reg(1)(1);                --  --- LEMO3 ---  (Sample_Puls_inv, J6),     OC15, "Sample_Puls_inv. "
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------
    IF  (AW_Config1(7) = '0')  THEN   -- Output positiv
      PIO(53)                     <= not AW_Output_Reg(1)(0);                --  D-Sub37-F(J5)_Pin-Nr. 35, OC14, "Sample-Puls "
    Else
      PIO(53)                     <=     AW_Output_Reg(1)(0);                --  D-Sub37-F(J5)_Pin-Nr. 35, OC14, "Sample-Puls "
    END IF; 
---------------------------------------------------------------------------------------------------------------------------------------

                                                
    --========================== Output Register 2 ======================================

    IF  (AW_Config1(7) = '0')  THEN   --  positiv
    
                                      -- +-- Pin-Nr. D-Sub37-F(J5)
                                      -- |
      PIO(61)   <=  not AW_Output_Reg(2)(10); --  31, OC10, "Strobe Phase"
      PIO(63)   <=  not AW_Output_Reg(2)(9);  --  30, OC9,  "Phase-Bit9"
      PIO(65)   <=  not AW_Output_Reg(2)(8);  --  29, OC8,  "Phase-Bit8"
      PIO(67)   <=  not AW_Output_Reg(2)(7);  --  28, OC7,  "Phase-Bit7"
      PIO(69)   <=  not AW_Output_Reg(2)(6);  --  27, OC6,  "Phase-Bit6"
      PIO(71)   <=  not AW_Output_Reg(2)(5);  --  26, OC5,  "Phase-Bit5"
      PIO(73)   <=  not AW_Output_Reg(2)(4);  --  25, OC4,  "Phase-Bit4"
      PIO(75)   <=  not AW_Output_Reg(2)(3);  --  24, OC3,  "Phase-Bit3"
      PIO(77)   <=  not AW_Output_Reg(2)(2);  --  23, OC2,  "Phase-Bit2"
      PIO(79)   <=  not AW_Output_Reg(2)(1);  --  22, OC1,  "Phase-Bit1"
      PIO(81)   <=  not AW_Output_Reg(2)(0);  --  21, OC0,  "Phase-Bit0"

    else
                                      -- +-- Pin-Nr. D-Sub37-F(J5)
                                      -- |
      PIO(61)   <=      AW_Output_Reg(2)(10); --  31, OC10, "Strobe Phase"
      PIO(63)   <=      AW_Output_Reg(2)(9);  --  30, OC9,  "Phase-Bit9"
      PIO(65)   <=      AW_Output_Reg(2)(8);  --  29, OC8,  "Phase-Bit8"
      PIO(67)   <=      AW_Output_Reg(2)(7);  --  28, OC7,  "Phase-Bit7"
      PIO(69)   <=      AW_Output_Reg(2)(6);  --  27, OC6,  "Phase-Bit6"
      PIO(71)   <=      AW_Output_Reg(2)(5);  --  26, OC5,  "Phase-Bit5"
      PIO(73)   <=      AW_Output_Reg(2)(4);  --  25, OC4,  "Phase-Bit4"
      PIO(75)   <=      AW_Output_Reg(2)(3);  --  24, OC3,  "Phase-Bit3"
      PIO(77)   <=      AW_Output_Reg(2)(2);  --  23, OC2,  "Phase-Bit2"
      PIO(79)   <=      AW_Output_Reg(2)(1);  --  22, OC1,  "Phase-Bit1"
      PIO(81)   <=      AW_Output_Reg(2)(0);  --  21, OC0,  "Phase-Bit0"
    
    END IF;  

    
   --========================== Output Register 3 ======================================

    IF  (AW_Config1(7) = '0')  THEN     --  positiv

                                        --   +-- Pin-Nr. D-Sub37-F(J5)
                                        --   |
      PIO(83)   <=  not AW_Output_Reg(3)(12); --  14, OE12, "Strobe Amplitude"
      PIO(85)   <=  not AW_Output_Reg(3)(11); --  13, OE11, "Amplitude-Bit11"
      PIO(87)   <=  not AW_Output_Reg(3)(10); --  12, OE10, "Amplitude-Bit10"
      PIO(89)   <=  not AW_Output_Reg(3)(9);  --  11, OE9,  "Amplitude-Bit9"
      PIO(91)   <=  not AW_Output_Reg(3)(8);  --  10, OE8,  "Amplitude-Bit8"
      PIO(93)   <=  not AW_Output_Reg(3)(7);  --  9,  OE7,  "Amplitude-Bit7"
      PIO(95)   <=  not AW_Output_Reg(3)(6);  --  8,  OE6,  "Amplitude-Bit6"
      PIO(97)   <=  not AW_Output_Reg(3)(5);  --  7,  OE5,  "Amplitude-Bit5"
      PIO(99)   <=  not AW_Output_Reg(3)(4);  --  6,  OE4,  "Amplitude-Bit4"
      PIO(101)  <=  not AW_Output_Reg(3)(3);  --  5,  OE3,  "Amplitude-Bit3"
      PIO(103)  <=  not AW_Output_Reg(3)(2);  --  4,  OE2,  "Amplitude-Bit2"
      PIO(105)  <=  not AW_Output_Reg(3)(1);  --  3,  OE1,  "Amplitude-Bit1"
      PIO(107)  <=  not AW_Output_Reg(3)(0);  --  2,  OE0,  "Amplitude-Bit0"

    Else                                                                                                                              
                                        --   +-- Pin-Nr. D-Sub37-F(J5)
                                        --   |
      PIO(83)   <=      AW_Output_Reg(3)(12); --  14, OE12, "Strobe Amplitude"
      PIO(85)   <=      AW_Output_Reg(3)(11); --  13, OE11, "Amplitude-Bit11"
      PIO(87)   <=      AW_Output_Reg(3)(10); --  12, OE10, "Amplitude-Bit10"
      PIO(89)   <=      AW_Output_Reg(3)(9);  --  11, OE9,  "Amplitude-Bit9"
      PIO(91)   <=      AW_Output_Reg(3)(8);  --  10, OE8,  "Amplitude-Bit8"
      PIO(93)   <=      AW_Output_Reg(3)(7);  --  9,  OE7,  "Amplitude-Bit7"
      PIO(95)   <=      AW_Output_Reg(3)(6);  --  8,  OE6,  "Amplitude-Bit6"
      PIO(97)   <=      AW_Output_Reg(3)(5);  --  7,  OE5,  "Amplitude-Bit5"
      PIO(99)   <=      AW_Output_Reg(3)(4);  --  6,  OE4,  "Amplitude-Bit4"
      PIO(101)  <=      AW_Output_Reg(3)(3);  --  5,  OE3,  "Amplitude-Bit3"
      PIO(103)  <=      AW_Output_Reg(3)(2);  --  4,  OE2,  "Amplitude-Bit2"
      PIO(105)  <=      AW_Output_Reg(3)(1);  --  3,  OE1,  "Amplitude-Bit1"
      PIO(107)  <=      AW_Output_Reg(3)(0);  --  2,  OE0,  "Amplitude-Bit0"

    END IF;  
                                                

    --==========================   Debounce Input's   ======================================

    HFIO_in_AMP_FEHLER_Deb_i    <=  not PIO(43);                    --  input "AMP_FEHLER"
    HFIO_in_PHASE_FEHLER_Deb_i  <=  not PIO(41);                    --  input "PHASE_FEHLER"


    
   --###################### Input's ==> AW_Input_Reg(1) ########################

      IF  (Diob_Config1(11) = '0')  THEN                -- 0 = Entprellung "Eingeschaltet" 
        AW_Input_Reg(1)(1)   <=  HFIO_in_AMP_FEHLER_Deb_o;        -- Entprellung "eingeschaltet"
        AW_Input_Reg(1)(0)   <=  HFIO_in_PHASE_FEHLER_Deb_o;      -- Entprellung "eingeschaltet"
      ELSE
        AW_Input_Reg(1)(1)   <=  HFIO_in_AMP_FEHLER_Deb_i;        -- Entprellung "ausgeschaltet"
        AW_Input_Reg(1)(0)   <=  HFIO_in_PHASE_FEHLER_Deb_i;      -- Entprellung "ausgeschaltet"
      END IF;




  WHEN   c_AW_INLB12S.ID =>
      
    --###################################################################################
    --###################################################################################
    --####                                                                            ###
    --####      Anwender-IO: FG902_050 -- Interlock-Backplane mit 12 Steckplätzen     ###
    --####                                                                            ###
    --###################################################################################
    --###################################################################################

      
    extension_cid_system <= c_cid_system;       -- extension card: CSCOHW
    extension_cid_group  <= c_AW_INLB12S.CID;   -- extension card: cid_group, "FG902_050"

    Max_AWOut_Reg_Nr     <= 3;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us
    
    
    --############################# Set Debounce- oder Syn-Time ######################################
      
      AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

      IF (AWIn_Deb_Time < Min_AWIn_Deb_Time) THEN Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
                                             ELSE Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
      END IF;
       
    --################################### Set LED's ########################################

    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 


    --========================== Output Register 1 ======================================
    
    PIO(86)   <=  '0';  ---------------- Output_Enable OEn1 (nach init vom ALTERA) 
    

    ---------------------------------------------------------------------------------------------------------------------------------------


    (PIO(112), PIO(48), PIO(130), PIO(30), PIO(137), PIO(35),
     PIO(119), PIO(53), PIO(101), PIO(73), PIO(96),  PIO(56))   <= IOBP_Output; ----------- Data_Output-Pin's der "Slave-Karten 12-1"
  

    IOBP_Input(1)(5 downto 1)  <= ( PIO(62),  PIO(54),  PIO(60),  PIO(52),  PIO(58)  );  -- Input's 5-1 von den der Slave-Karte-1,  
    IOBP_Input(2)(5 downto 1)  <= ( PIO(102), PIO(94),  PIO(100), PIO(92),  PIO(98)  );  -- Input's 5-1 von den der Slave-Karte-2,  
    IOBP_Input(3)(5 downto 1)  <= ( PIO(79),  PIO(71),  PIO(77),  PIO(69),  PIO(75)  );  -- Input's 5-1 von den der Slave-Karte-3,  
    IOBP_Input(4)(5 downto 1)  <= ( PIO(93),  PIO(103), PIO(91),  PIO(105), PIO(89)  );  -- Input's 5-1 von den der Slave-Karte-4,  
    IOBP_Input(5)(5 downto 1)  <= ( PIO(63),  PIO(55),  PIO(61),  PIO(57),  PIO(59)  );  -- Input's 5-1 von den der Slave-Karte-5,  
    IOBP_Input(6)(5 downto 1)  <= ( PIO(111), PIO(121), PIO(109), PIO(123), PIO(107) );  -- Input's 5-1 von den der Slave-Karte-6,  
    IOBP_Input(7)(5 downto 1)  <= ( PIO(45),  PIO(37),  PIO(43),  PIO(39),  PIO(41)  );  -- Input's 5-1 von den der Slave-Karte-7,  
    IOBP_Input(8)(5 downto 1)  <= ( PIO(129), PIO(139), PIO(127), PIO(141), PIO(125) );  -- Input's 5-1 von den der Slave-Karte-8,  
    IOBP_Input(9)(5 downto 1)  <= ( PIO(20),  PIO(28),  PIO(22),  PIO(26),  PIO(24)  );  -- Input's 5-1 von den der Slave-Karte-9,  
    IOBP_Input(10)(5 downto 1) <= ( PIO(138), PIO(128), PIO(140), PIO(126), PIO(142) );  -- Input's 5-1 von den der Slave-Karte-10, 
    IOBP_Input(11)(5 downto 1) <= ( PIO(38),  PIO(46),  PIO(40),  PIO(44),  PIO(42)  );  -- Input's 5-1 von den der Slave-Karte-11, 
    IOBP_Input(12)(5 downto 1) <= ( PIO(120), PIO(110), PIO(122), PIO(108), PIO(124) );  -- Input's 5-1 von den der Slave-Karte-12, 


    (PIO(70), PIO(72), PIO(74), PIO(76), PIO(78), PIO(80), PIO(82), PIO(84))  <=  IOBP_LED_ID_Bus_o;   ---- LED_ID_Bus_Out


    (PIO(114), PIO(50), PIO(132), PIO(32), PIO(135), PIO(33), PIO(117), PIO(51), PIO(99), PIO(83), PIO(106), PIO(66))  <=  IOBP_STR_rot_o;   -- LED-Str Rot  für Slave 12-1
    (PIO(116), PIO(34), PIO(134), PIO(16), PIO(133), PIO(49), PIO(115), PIO(67), PIO(97), PIO(81), PIO(104), PIO(64))  <=  IOBP_STR_gruen_o; -- LED-Str Grün für Slave 12-1
    (PIO(118), PIO(36), PIO(136), PIO(18), PIO(131), PIO(47), PIO(113), PIO(65), PIO(95), PIO(85), PIO(90),  PIO(68))  <=  IOBP_STR_ID_o;    -- ID-Str für Slave 12-1
     
      
-----------------------------------------------------------------------------------------------------------------------------------------


---------------- Output-Register(Maske) für die Iput- und Output Sel-LED's vom Slave 1-12
--
--  IOBP_Sel_LED's sind Low-Aktiv             Bit5 = Output          Bit[4..0]  =  Input[5..1]
--        |                                         |                           |
    IOBP_Sel_LED(1) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 0) & IOBP_Masken_Reg1( 4 downto  0) );  -- Register für Sel-LED's vom Slave 1 
    IOBP_Sel_LED(2) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 1) & IOBP_Masken_Reg1( 9 downto  5) );  -- Register für Sel-LED's vom Slave 2
    IOBP_Sel_LED(3) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 2) & IOBP_Masken_Reg1(14 downto 10) );  -- Register für Sel-LED's vom Slave 3 
    IOBP_Sel_LED(4) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 3) & IOBP_Masken_Reg2( 4 downto  0) );  -- Register für Sel-LED's vom Slave 4
    IOBP_Sel_LED(5) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 4) & IOBP_Masken_Reg2( 9 downto  5) );  -- Register für Sel-LED's vom Slave 5
    IOBP_Sel_LED(6) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 5) & IOBP_Masken_Reg2(14 downto 10) );  -- Register für Sel-LED's vom Slave 6
    IOBP_Sel_LED(7) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 6) & IOBP_Masken_Reg3( 4 downto  0) );  -- Register für Sel-LED's vom Slave 7
    IOBP_Sel_LED(8) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 7) & IOBP_Masken_Reg3( 9 downto  5) );  -- Register für Sel-LED's vom Slave 8
    IOBP_Sel_LED(9) (6 downto 1)  <=  not ( IOBP_Masken_Reg5( 8) & IOBP_Masken_Reg3(14 downto 10) );  -- Register für Sel-LED's vom Slave 9
    IOBP_Sel_LED(10)(6 downto 1)  <=  not ( IOBP_Masken_Reg5( 9) & IOBP_Masken_Reg4( 4 downto  0) );  -- Register für Sel-LED's vom Slave 10
    IOBP_Sel_LED(11)(6 downto 1)  <=  not ( IOBP_Masken_Reg5(10) & IOBP_Masken_Reg4( 9 downto  5) );  -- Register für Sel-LED's vom Slave 11
    IOBP_Sel_LED(12)(6 downto 1)  <=  not ( IOBP_Masken_Reg5(11) & IOBP_Masken_Reg4(14 downto 10) );  -- Register für Sel-LED's vom Slave 12


    ---------------------------------------------------------------------------------------------------------------------------------------

--                    ID-Input's z.Z. nicht möglich (keine Tri-State)    

--      AW_Input_Reg(5)(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2         
--      AW_Input_Reg(5)( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1     

   
---------------------------------------------------------------------------------------------------------------------------------------



--################################ Debounce oder Sync Input's  ##################################

--  Deb60_in = H-Aktiv             IOBP_Input = L-Aktiv  
--        |                                |
    Deb60_in( 4 DOWNTO  0)   <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Deb60_in( 9 DOWNTO  5)   <=  not IOBP_Input( 2)(5 downto 1);
    Deb60_in(14 DOWNTO 10)   <=  not IOBP_Input( 3)(5 downto 1);
    Deb60_in(19 DOWNTO 15)   <=  not IOBP_Input( 4)(5 downto 1);
    Deb60_in(24 DOWNTO 20)   <=  not IOBP_Input( 5)(5 downto 1);
    Deb60_in(29 DOWNTO 25)   <=  not IOBP_Input( 6)(5 downto 1);
    Deb60_in(34 DOWNTO 30)   <=  not IOBP_Input( 7)(5 downto 1);
    Deb60_in(39 DOWNTO 35)   <=  not IOBP_Input( 8)(5 downto 1);
    Deb60_in(44 DOWNTO 40)   <=  not IOBP_Input( 9)(5 downto 1);
    Deb60_in(49 DOWNTO 45)   <=  not IOBP_Input(10)(5 downto 1);
    Deb60_in(54 DOWNTO 50)   <=  not IOBP_Input(11)(5 downto 1);
    Deb60_in(59 DOWNTO 55)   <=  not IOBP_Input(12)(5 downto 1);
    
    
--  Syn60_in = H-Aktiv             IOBP_Input = L-Aktiv  
--        |                                |
    Syn60_in( 4 DOWNTO  0)   <=  not IOBP_Input( 1)(5 downto 1);  -- Input-Daten
    Syn60_in( 9 DOWNTO  5)   <=  not IOBP_Input( 2)(5 downto 1);
    Syn60_in(14 DOWNTO 10)   <=  not IOBP_Input( 3)(5 downto 1);
    Syn60_in(19 DOWNTO 15)   <=  not IOBP_Input( 4)(5 downto 1);
    Syn60_in(24 DOWNTO 20)   <=  not IOBP_Input( 5)(5 downto 1);
    Syn60_in(29 DOWNTO 25)   <=  not IOBP_Input( 6)(5 downto 1);
    Syn60_in(34 DOWNTO 30)   <=  not IOBP_Input( 7)(5 downto 1);
    Syn60_in(39 DOWNTO 35)   <=  not IOBP_Input( 8)(5 downto 1);
    Syn60_in(44 DOWNTO 40)   <=  not IOBP_Input( 9)(5 downto 1);
    Syn60_in(49 DOWNTO 45)   <=  not IOBP_Input(10)(5 downto 1);
    Syn60_in(54 DOWNTO 50)   <=  not IOBP_Input(11)(5 downto 1);
    Syn60_in(59 DOWNTO 55)   <=  not IOBP_Input(12)(5 downto 1);
    
    
    IF  (Diob_Config1(11) = '1')  THEN Deb_Sync60 <=  Syn60_out;    -- Dobounce = Abgeschaltet ==> nur Synchronisation
                                  ELSE Deb_Sync60 <=  Deb60_out;     -- Debounce und Synchronisation
    END IF;  
     
    
--  ################################ Input's AND Maske zu Input-Register ##################################
      
--                  Input-Test, Stecker 1, 2, 3    

    AW_Input_Reg(1)( 4 downto  0) <=   (Deb_Sync60( 4 downto  0) AND not IOBP_Masken_Reg1( 4 downto  0));  -- Input, IO-Modul Nr. 1
    AW_Input_Reg(1)( 9 downto  5) <=   (Deb_Sync60( 9 downto  5) AND not IOBP_Masken_Reg1( 9 downto  5));  -- Input, IO-Modul Nr. 2
    AW_Input_Reg(1)(14 downto 10) <=   (Deb_Sync60(14 downto 10) AND not IOBP_Masken_Reg1(14 downto 10));  -- Input, IO-Modul Nr. 3         
    AW_Input_Reg(1)(15) <=  '0';        
    
--                  Input-Test, Stecker 4, 5, 6    
    AW_Input_Reg(2)( 4 downto  0) <=   (Deb_Sync60(19 DOWNTO 15) AND not IOBP_Masken_Reg2( 4 downto  0));  -- Input, IO-Modul Nr. 4
    AW_Input_Reg(2)( 9 downto  5) <=   (Deb_Sync60(24 DOWNTO 20) AND not IOBP_Masken_Reg2( 9 downto  5));  -- Input, IO-Modul Nr. 5
    AW_Input_Reg(2)(14 downto 10) <=   (Deb_Sync60(29 DOWNTO 25) AND not IOBP_Masken_Reg2(14 downto 10));  -- Input, IO-Modul Nr. 6         
    AW_Input_Reg(2)(15) <=  '0';        

--                  Input-Test, Stecker 7, 8, 9    

    AW_Input_Reg(3)( 4 downto  0) <=   (Deb_Sync60(34 DOWNTO 30) AND not IOBP_Masken_Reg3( 4 downto  0));  -- Input, IO-Modul Nr. 7
    AW_Input_Reg(3)( 9 downto  5) <=   (Deb_Sync60(39 DOWNTO 35) AND not IOBP_Masken_Reg3( 9 downto  5));  -- Input, IO-Modul Nr. 8
    AW_Input_Reg(3)(14 downto 10) <=   (Deb_Sync60(44 DOWNTO 40) AND not IOBP_Masken_Reg3(14 downto 10));  -- Input, IO-Modul Nr. 9         
    AW_Input_Reg(3)(15) <=  '0';        
      
--                    Input-Test, Stecker 10, 11, 12    

    AW_Input_Reg(4)( 4 downto  0) <=   (Deb_Sync60(49 DOWNTO 45) AND not IOBP_Masken_Reg4( 4 downto  0));  -- Input, IO-Modul Nr. 10
    AW_Input_Reg(4)( 9 downto  5) <=   (Deb_Sync60(54 DOWNTO 50) AND not IOBP_Masken_Reg4( 9 downto  5));  -- Input, IO-Modul Nr. 11
    AW_Input_Reg(4)(14 downto 10) <=   (Deb_Sync60(59 DOWNTO 55) AND not IOBP_Masken_Reg4(14 downto 10));  -- Input, IO-Modul Nr. 12         
    AW_Input_Reg(4)(15) <=  '0';        
    

    
--################################ Outputs AND Maske ##################################
--
--                                                    MaskenBit=0 --> Enable 
    IOBP_Output(1)  <= (AW_Output_Reg(1)( 0) AND not IOBP_Masken_Reg5( 0));  -- Output von Slave 1 
    IOBP_Output(2)  <= (AW_Output_Reg(1)( 1) AND not IOBP_Masken_Reg5( 1));  -- Output von Slave 2
    IOBP_Output(3)  <= (AW_Output_Reg(1)( 2) AND not IOBP_Masken_Reg5( 2));  -- Output von Slave 3 
    IOBP_Output(4)  <= (AW_Output_Reg(1)( 3) AND not IOBP_Masken_Reg5( 3));  -- Output von Slave 4
    IOBP_Output(5)  <= (AW_Output_Reg(1)( 4) AND not IOBP_Masken_Reg5( 4));  -- Output von Slave 5
    IOBP_Output(6)  <= (AW_Output_Reg(1)( 5) AND not IOBP_Masken_Reg5( 5));  -- Output von Slave 6
    IOBP_Output(7)  <= (AW_Output_Reg(1)( 6) AND not IOBP_Masken_Reg5( 6));  -- Output von Slave 7
    IOBP_Output(8)  <= (AW_Output_Reg(1)( 7) AND not IOBP_Masken_Reg5( 7));  -- Output von Slave 8
    IOBP_Output(9)  <= (AW_Output_Reg(1)( 8) AND not IOBP_Masken_Reg5( 8));  -- Output von Slave 9
    IOBP_Output(10) <= (AW_Output_Reg(1)( 9) AND not IOBP_Masken_Reg5( 9));  -- Output von Slave 10
    IOBP_Output(11) <= (AW_Output_Reg(1)(10) AND not IOBP_Masken_Reg5(10));  -- Output von Slave 11
    IOBP_Output(12) <= (AW_Output_Reg(1)(11) AND not IOBP_Masken_Reg5(11));  -- Output von Slave 12


    
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


  WHEN   c_AW_16Out2.ID  =>    

  
    --###################################################################################
    --####                         Anwender-IO: 16Out-FG901_010                       ###
    --###################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-10 | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      8    | Output-Polarität Lemo,         1 = Negativ,  0 = Positiv(Default)         --
--      7    | Output-Polarität Bit [15..0],  1 = Negativ,  0 = Positiv(Default)         --
--     ------+-----------------------------------------------------------------------    --
--      6-0  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
    
    
--           +=======================================================================    --
--           |         User-Config-Register 2 (AW_Config2)                               --
--     ------+=======================================================================    --
--     15-6  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      5    | Strobe-Polarität,         1 = Negativ,  0 = Positiv(Default)              --
--     ------+-----------------------------------------------------------------------    --
--    [4..2] | Strobe-Puls-Breite, Entprellzeit in in 2x 100ns;                          --
--           | Vorgabe Exponent (x) für Pulsbreite: Wertebereich 100ns..12,8 µs *)       --
--     ------+-----------------------------------------------------------------------    --
--           |  Output-Mode:                                                             --
--    [1..0] | "00" = Output, "01" = 16 Bit-Dac, "10" = frei,  "11" = FG                 --
--     ------+-----------------------------------------------------------------------    --
    


    extension_cid_system <= c_cid_system;     -- extension card: CSCOHW
    extension_cid_group  <= c_AW_16Out2.CID;  -- extension card: cid_group, "FG901010_16Out"
    
    Max_AWOut_Reg_Nr     <= 2;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 1;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    
    --############################# Set Debounce-Time ######################################
      
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################
    
    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 


    
    --################### Output-Daten von den AWOut_Registern oder dem FG mit Strobe ##################


--                 +-----------------------------------------------+-----------------------------------------------+
--                 |                 AW_Output_Reg. 2              |                 AW_Output_Reg. 1              |
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|    
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                                               |                                 
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ 16 Bit        
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|  DAC          
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ bipol.
--    25pol. SubD  |                                               |                                 
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+           
--  |     Pin #    |22|21|20|19|18|17|16|15| 9| 8| 7| 6| 5| 4| 3| 2|   11   |
--  +---------------==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+             
--  |  Daten-Bit#  |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| Strobe | <-+-- AW_Output_Reg1(0)   (Default-Mode)
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+========+   |-- Strobe write 16-Bit (DAC-Mode 16) 
--                 |                                                            +-- FG-Str.             (FG-Mode)       
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+    
--                 |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|     
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |               Output-Daten vom FG             | 
--                 +---------------------------------------------- +

  
      Out16_Mode  <=  AW_Config2(1)& AW_Config2(0);         -- Output-Betriebsart

      case Out16_Mode is

        when "00"  =>  -- Output-Mode(Default):
        
              Out16_Data_FG_Out(15 DOWNTO 0) <=  AW_Output_Reg(2)(15 DOWNTO 0);
              Out16_Strobe                   <=  AW_Output_Reg(1)(0);
  
  
        when "01"  =>  -- DAC16-Mode:
              
              Out16_Data_FG_Out(15 DOWNTO 0) <=  AW_Output_Reg(2)(15 DOWNTO 0);
    
              Out16_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config2)(4 downto 2)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n
              Out16_DAC_Strobe_i     <=  (AWOut_Reg2_wr AND SCU_Ext_Wr_fin);              -- Software-Strobe   

                IF  (AW_Config2(5) = '0')  THEN  Out16_Strobe  <=      Out16_DAC_Strobe_o; -- Strobe positiv
                                           Else  Out16_Strobe  <=  not Out16_DAC_Strobe_o; -- Strobe negativ
                END IF; 
              
              
        when "10"  =>  -- frei-Mode:  
              
              Out16_Data_FG_Out(15 DOWNTO 0) <=  (others => '0');
              Out16_Strobe                   <=  '0';
  
        when "11"  =>  -- FG-Mode:     Output-Mode: Out[23..1] = FG_Daten[31..9], Out[0] = FG_DAC-Strobe 
              
              Out16_Data_FG_Out(15 downto 0) <= FG_1_sw(31 downto 16);                    -- gespeicherte FG-Daten zum Ausgang
  
              Out16_DAC_Strobe_Expo  <=  (to_integer(unsigned(AW_Config2)(4 downto 2)));  -- Multiplikationswert für 100ns aus Wertetabelle 2^n
              Out16_DAC_Strobe_i     <=  FG_1_strobe;                                     -- FG_1_strobe (vom Funktionsgen)

                IF  (AW_Config2(5) = '0')  THEN  Out16_Strobe  <=      Out16_DAC_Strobe_o; -- Strobe positiv
                                           Else  Out16_Strobe  <=  not Out16_DAC_Strobe_o; -- Strobe negativ
                END IF; 

      end case;

   
    --############################ Einstellen der Output-Polarität ##################################

    IF  (AW_Config1(7) = '1')  THEN  Out16_Out(15 DOWNTO 0) <=  Not Out16_Data_FG_Out(15 downto 0); -- Output negativ 
                               Else  Out16_Out(15 DOWNTO 0) <=      Out16_Data_FG_Out(15 downto 0); -- Output positiv 
    END IF; 

    
    --################################### Output-Enable ##################################

    PIO(39)    <=  '0';  -- OEnStrobe,             Output_Enable (nach init vom ALTERA)
    PIO(129)   <=  '0';  -- OEn1  (OEn0  & OEn8),  Output_Enable (nach init vom ALTERA)
    PIO(121)   <=  '0';  -- OEn2  (OEn1  & OEn9),  Output_Enable (nach init vom ALTERA)
    PIO(113)   <=  '0';  -- OEn3  (OEn2  & OEn10), Output_Enable (nach init vom ALTERA)
    PIO(105)   <=  '0';  -- OEn4  (OEn11 & OEn13), Output_Enable (nach init vom ALTERA)
    PIO(97)    <=  '0';  -- OEn5  (OEn4  & OEn12), Output_Enable (nach init vom ALTERA)
    PIO(83)    <=  '0';  -- OEn6  (OEn5  & OEn13), Output_Enable (nach init vom ALTERA)
    PIO(69)    <=  '0';  -- OEn7  (OEn6  & OEn14), Output_Enable (nach init vom ALTERA)
    PIO(55)    <=  '0';  -- OEn8  (OEn7  & OEn15), Output_Enable (nach init vom ALTERA)
      
      
    --########################## Daten zum Piggy-Stecker JPIO1 ###########################

    (PIO(51), PIO(65), PIO(79), PIO(93),  PIO(101), PIO(109), PIO(117), PIO(125)) <=  Out16_Out(15 downto 8);  --- Output-Pins zum Piggy [15.. 8]
    (PIO(53), PIO(67), PIO(81), PIO(95),  PIO(103), PIO(111), PIO(119), PIO(127)) <=  Out16_Out(7  downto 0);  --- Output-Pins zum Piggy [ 7.. 0]

     PIO(37)    <=  Out16_Strobe;  -- Output-Strobe

    
    --########################### Debounce, Input und LED   ############################

    Out16_Lemo_deb_i    <= not PIO(135);          -- Debounce-Lemo Input (Low-Aktiv)        


    IF  (Diob_Config1(11) = '0')  THEN  AW_Input_Reg(1)(0)  <=  Out16_Lemo_deb_o;      -- Debounce-Output "Lemo" 
                                  ELSE  AW_Input_Reg(1)(0)  <=  Out16_Lemo_deb_i;      -- Input "Lemo"
    END IF;

    Out16_LED_Lemo_In_i     <=  Out16_Lemo_deb_i;      --  Input  "nLED_Lemo_IN"
    PIO(139)                <=  Out16_nLED_Lemo_In_o;  --  Output "nLED_Lemo_IN"
    
    
      -----------------------------------------------------------------------------------------------------------------------------------------


  WHEN   c_AW_16In2.ID  =>    

  
    --###################################################################################
    --####                         Anwender-IO: 16In-FG901_020                        ###
    --###################################################################################

--           +=======================================================================    --
--           |         User-Config-Register 1 (AW_Config1)                               --
--     ------+=======================================================================    --
--     15-9  | frei                                                                      --
--     ------+-----------------------------------------------------------------------    --
--      8    | Output-Polarität Lemo,         1 = Negativ,  0 = Positiv(Default)         --
--     ------+-----------------------------------------------------------------------    --
--      7-2  | frei                                                                      --
--     ------+----------------+------------------------------------------------------    --
--      1    | Triggerflanke: | 1 = neg. Flanke ist Trigger
--           |                | 0 = pos. Flanke ist Trigger  
--     ------+----------------+------------------------------------------------------    --
--      0    | Input-Mode:    | 0 = Input, 1 = Input mit Strobe
--     ------+-----------------------------------------------------------------------    --
    

    extension_cid_system <= c_cid_system;     -- extension card: CSCOHW
    extension_cid_group  <= c_AW_16In2.CID;   -- extension card: cid_group, "FG901020_16In"
    
    Max_AWOut_Reg_Nr     <= 1;  -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr      <= 2;  -- Maximale AWIn-Reg-Nummer der Anwendung
    Min_AWIn_Deb_Time    <= 0;  -- Minimale Debounce-Zeit 2 Hoch "Min_AWIn_Deb_Time" in us

    
    --############################# Set Debounce-Time ######################################
      
    AWIn_Deb_Time   <= to_integer(unsigned(Diob_Config1)(14 downto 12)); -- -- Debounce-Zeit 2 Hoch "AWIn_Deb_Time" in us, Wert aus DIOB-Config 1        

    IF  (AWIn_Deb_Time < Min_AWIn_Deb_Time)  THEN         
        Deb60_Cnt <= Wert_2_Hoch_n(Min_AWIn_Deb_Time);   -- Debounce-Zeit = Min_AWIn_Deb_Time
    ELSE
        Deb60_Cnt <= Wert_2_Hoch_n(AWIn_Deb_Time);       -- Debounce-Zeit = AWIn_Deb_Time
    END IF;

    
    --################################### Set LED's ########################################
    
    s_nLED_User1_i <= '0';        -- LED3 = User 1, -- frei -- 
    s_nLED_User2_i <= '0';        -- LED3 = User 2, -- frei -- 
    s_nLED_User3_i <= '0';        -- LED3 = User 3, -- frei -- 


    
    --########################### Input-Daten zu den AWIn_Registern ########################


--                 +-----------------------------------------------+-----------------------------------------------+
--                 |                  AW_Input_Reg. 2              |                  AW_Input_Reg. 1              |
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+
--                 |                                               |                                             
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ 16 Bit                                      
--                 |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|  ADC                                        
--                 +==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+ bipol.                                      
--    25pol. SubD  |                                               |               +-<----- AW_Input_Reg.(1)(0)                        
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=========+=====+=====+                       
--  |     Pin #    |22|21|20|19|18|17|16|15| 9| 8| 7| 6| 5| 4| 3| 2|    24   |     11    |                       
--  +---------------==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=========+===========+                       
--  |  Daten-Bit#  |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| Req_Out | Strobe_In |                       
--  +--------------+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+==+=====+==+============+                       
--                                                                       +-<--------------- AW_Output_Reg.(1)(1)                                             
--                                                                                                                    
--                                                                                                                    
    --############################# Signal-Input-Daten über Piggy-Stecker JPIO1 ###################################

    In16_IN(15 downto 0)  <=  not (PIO(97), PIO(89), PIO(81), PIO(73), PIO(65), PIO(57), PIO(49), PIO(41),     
                                   PIO(95), PIO(87), PIO(79), PIO(71), PIO(63), PIO(55), PIO(47), PIO(39));

    In16_Strobe_in        <=  NOT PIO(103);        --  Input "Strobe" 
    

   --################################ Input: Debounce oder Synchr.  ##################################

    Deb60_in(15 DOWNTO 0)              <=  In16_IN(15 downto 0);  -- Entprellung der Input-Daten
    Deb60_in(16)                       <=  In16_Strobe_in;        -- Entprellung des Input "Strobe" 

    Syn60_in(15 DOWNTO 0)             <=  In16_IN(15 downto 0);  -- Synchronisation der Input-Daten
    Syn60_in(16)                      <=  In16_Strobe_in;        -- Synchronisation des Input "Strobe" 


    IF  (Diob_Config1(11) = '0')  THEN                          -- 0 = Entprellung "Eingeschaltet" 
      In16_Input(15 DOWNTO 0)   <=  Deb60_out(15 DOWNTO 0);     -- Debounce-Input-Daten   (Dafault)
      In16_Strobe               <=  Deb60_out(16);              -- Debounce-Input-Strobe  (Dafault)
    ELSE    
      In16_Input(15 DOWNTO 0)   <=  Syn60_out(15 DOWNTO 0);     -- Syn-Input-Daten
      In16_Strobe               <=  Syn60_out(16);              -- Syn-Input-Strobe
    END IF;

   --################################   Daten FF mit Strobe      ##################################

    In16_ADC_Data_FF_i(15 DOWNTO 0)    <=  In16_Input(15 downto 0);  -- Input zum Daten-Input_FF

    IF  (AW_Config1(1) = '0')  THEN  In16_ADC_Strobe_i <=  NOT In16_Strobe; -- pos. Flanke vom Strobe (Default)
                               Else  In16_ADC_Strobe_i <=      In16_Strobe; -- neg. Flanke vom Strobe  
    END IF; 
  
  
 --################################         Input-Mode     ##################################
        
    IF  (AW_Config1(0) = '0')  THEN                                       -- 0 = Input-Mode
      AW_Input_Reg(2)(15 DOWNTO 0)  <=  In16_Input(15 DOWNTO 0);          -- Daten-Input  Deb/Syn
      AW_Input_Reg(1)(0)            <=  In16_Strobe;                      -- Strobe-Input Deb/Syn
    ELSE    
      AW_Input_Reg(2)(15 DOWNTO 0)  <=  In16_ADC_Data_FF_o(15 DOWNTO 0);  -- Daten aus dem Input-Register
      AW_Input_Reg(1)(0)            <=  In16_ADC_Strobe_o;                -- Daten-Strobe für die Input-Daten
    END IF;
   
    
    --################################### Output-Enable ##################################

    PIO(121)   <=  '0';  -- OEnRequest,    Output_Enable (nach init vom ALTERA)
    PIO(125)   <=  '0';  -- OEnLemo_out    Output_Enable (nach init vom ALTERA)
      
      
    --############################ Lemo und Request Output ##################################

    IF  (AW_Config1(8) = '0')  THEN  PIO(113) <=      AW_Output_Reg(1)(0); -- Output positiv (Default)
                               Else  PIO(113) <=  Not AW_Output_Reg(1)(0); -- Output negativ 
    END IF; 

    In16_LED_Lemo_Out_i       <=  AW_Output_Reg(1)(0);           --  Input  "nLED_Lemo_Out"
    PIO(131)                  <=  In16_nLED_Lemo_Out_o;          --  Output "nLED_Lemo_Out"
    
    PIO(117) <=    AW_Output_Reg(1)(1); -- Request-Output
      
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

    PIO(17)   <=    AW_ID(7) AND clk_blink; -- LED7
    PIO(19)   <=    AW_ID(6) AND clk_blink; -- LED6   
    PIO(21)   <=    AW_ID(5) AND clk_blink; -- LED5     
    PIO(23)   <=    AW_ID(4) AND clk_blink; -- LED4   
    PIO(25)   <=    AW_ID(3) AND clk_blink; -- LED3   
    PIO(27)   <=    AW_ID(2) AND clk_blink; -- LED2      
    PIO(29)   <=    AW_ID(1) AND clk_blink; -- LED1      
    PIO(31)   <=    AW_ID(0) AND clk_blink; -- LED0   
           
  END CASE;


  END IF;

  
END PROCESS p_AW_MUX;


end architecture;









