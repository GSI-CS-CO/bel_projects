library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;
use work.daq_pkg.all;


--  Base_addr    : DIOB-Config-Register1 (all bits can be read and written)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 15     | Test-Mode                 | 1 = Testmodus; for commissioning and diagnostics, 0 = Normal mode (default)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 14..12 | InReg-Debounce-Time       | Debounce time for digital inputs ;
--  |            |                           | Debounce time  in in 2x µs; parameter Exponent (x) for Debounce time;
--  |            |                           | Wertebereich 1 ... 128 µs *)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 11     | InReg-Debounce-Enable     | Control (Switching on/off) of the Debounce time (debouncing unit) for external digital signals
--  |            |                           | 1 = Debouncing switched off
--  |            |                           | 0 = Debouncing switched on
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 10..8  | MirrorMode-InReg-Sel      | Selection of the input register for mirroring of the selected output register(see MirrorMode-OutReg-Sel)
--  |            |                           | 0 = inactive
--  |            |                           | 1...7 = Inputregisters 1 to 7; all unmasked bits (see MirrorMode-OutReg-Mask) of the output register x
--  |            |                           | (x selected through MirrorMode-OutReg-Sel) are copied to the here selectedt input register (mirrored)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 7..5   | MirrorMode-OutReg-Sel     | Selection of the output register for mirroring of the bits in input register x (x selected through MirrorMode-InReg-Sel)
--  |            |                           | 0 = inactive
--  |            |                           | 1...7 = Outputregisters 1 to 7
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 4      |--  Reserve                |
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 3      | MirrorMode-OutReg         | Enables mirroring of the selected output register (MirrorMode-OutReg-Sel) to input register 1 or 2;
--  |            |                           | 1 = Mirroring enabled
--  |            |                           | 0 = Mirroring deactivated (default)
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 2      | Clear-CntUnit-Config      | 1 = clearing all configuration registers of the counter channels(CounterUnit-Config-Reg 1 und 2)
--  |            |                           | Bit is automatically deleted after evaluation; cannot be read back
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 1      | Clear-CmpUnit-Config      | 1 = delete all configuration registers of the compare channels (CmpUnit-Config-Reg 1 und 2)
--  |            |                           | Bit is automatically deleted after evaluation; cannot be read back
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--  | Bit 0      | Clear-TAG-Config          | 1 = delete all configuration registers of the event control(TAG-Filtering)
--  |                                        | Bit is automatically deleted after evaluation; cannot be read back
--  +------------+---------------------------+------------------------------------------------------------------------------------------------------------------
--                                                                                                                                --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +1 : DIOB-Config-Register2 (all bits can be read and written)                                                   --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-0 |  free                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +2 : DIOB-Status-Register1 (the status bits are deleted after reading)                                          --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--      5 |  TAG-ID-Config-Error     | two or more event channels with the same mask and the same output register                 --
--      4 |  OutReg-Select-Error     | in one or more event channels no output register is selected                               --
--      3 |  TriggerReg-Select-Error | in one or more event channels no input register for trigger signal is selected             --
--      2 |  Unknown-OutReg-Select   | in one or more event channels an unsupported output register is selected                   --
--      1 |  Unknown-InReg-Select    | in one or more event channels an unsupported input register is selected                    --
--      0 |  Trigger-Time-Out        | allowed waiting time for external trigger signal has been exceeded (Time-Out)              --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +3 : DIOB-Status-Register2 (the status bits are deleted after reading)                                          --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-0 |  free                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--     Base_addr + 4 – Base_addr +6  reserved for expansion                                                                       --
-----+------------------------------------------------------------------------------------------------------------------
--     Base_addr + 7 Configuration register1 for interface part: The bits in the user (piggy)config register1 have a different    --
--     meaning for each piggy                                                                                                     --
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

    --------- Parallel SCU-Bus-Signals ----------------------------------------------------------------------------
    A_A: in std_logic_vector(15 downto 0); -- SCU-Adress Bus
    A_nADR_EN: out std_logic := '0'; -- '0' => external address driver of the slave active
    A_nADR_FROM_SCUB: out std_logic := '0'; -- '0' => external address driver direction: SCU bus to slave
    A_D: inout std_logic_vector(15 downto 0); -- SCU-Data Bus
    A_nDS: in std_logic; -- Data strobe driven by master
    A_RnW: in std_logic; -- Write/Read signal driven by master, '0' => read
    A_nSel_Ext_Data_Drv: out std_logic; -- '0' => external data driver of the slave active
    A_Ext_Data_RD: out std_logic; -- '0' => External data driver direction: SCU bus to slave (better default 0, or swap driver A/B)                                                             
    A_nDtack: out std_logic; -- Data-Acknowlege zero active, '0' => enables external open drain driver
    A_nSRQ: out std_logic; -- Service-Request zero active,   '0' => enables external open drain driver
    A_nBoardSel: in std_logic; -- '0' => Master activates this slave
    A_nEvent_Str: in std_logic; -- '0' => Master signals timing cycle
    A_SysClock: in std_logic; -- Clock driven by master
    A_Spare0: in std_logic; -- driven by master
    A_Spare1: in std_logic; -- driven by master
    A_nReset: in std_logic; -- Reset (active '0'), driven by master

    A_nSEL_Ext_Signal_DRV: out std_logic; -- '0' => Driver for SCU bus control signals active
    A_nExt_Signal_in: out std_logic; -- '0' => Driver for SCU bus control signals direction: SCU bus to slave (better default 0, or swap driver A/B)

    ----------------- OneWire ----------------------------------------------------------------------------------------
    A_OneWire: inout std_logic; -- Temp.-OneWire on the Slave

    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL: in std_logic_vector(3 downto 0); -- use to select sources for the logic analyser ports
    A_Tclk: out std_logic; -- Clock  for logic analiser Port A
    A_TA:   out std_logic_vector(15 downto 0); -- test port a

    ---------------------------------- Diagnose-LED's -----------------------------------------------------------------
    A_nLED_D2: out std_logic; -- Diagnosis-LED_D2 on the Base-Board
    A_nLED_D3: out std_logic; -- Diagnosis-LED_D3 on the Base-Board

    ------------ User I/O to VG-Bar -------------------------------------------------------------------------------
    A_nUser_EN: out std_logic; -- Enable User-I/O
    UIO: inout std_logic_vector(15 downto 0); -- User I/O VG-Bar

    ---------------- Transfer connector for user I/O-----------------------------------------------------------------
    CLK_IO: in std_logic; -- Clock for user_I/0
    PIO: inout std_logic_vector(150 downto 16)  -- Dig. User I/0 to Piggy
    );
end scu_diob;


architecture scu_diob_arch_for_Beam_Loss_Mon of scu_diob is

--  +============================================================================================================================+
--  |                                 Firmware_Version/Firmware_Release and Base-Addresses                                      |
--  +============================================================================================================================+

    CONSTANT c_Firmware_Version:    Integer := 0;      -- Firmware_Version
    CONSTANT c_Firmware_Release:    Integer := 29;     -- Firmware_release Stand 19.05.2021 ( + neuer Zwischen-Backplane )
--  CONSTANT c_Firmware_Release:    Integer := 16#FF#; -- Test-Firmware_release
    CONSTANT clk_switch_status_cntrl_addr:       unsigned := x"0030";
    CONSTANT c_lm32_ow_Base_Addr:   unsigned(15 downto 0):=  x"0040";  -- housekeeping/LM32
    CONSTANT c_tmr_Base_Addr:       unsigned(15 downto 0):=  x"0330";  -- Timer
    CONSTANT c_Conf_Sts1_Base_Addr:              Integer := 16#0500#;  -- Status-Config-Register 
    CONSTANT c_AW_Port1_Base_Addr:               Integer := 16#0510#;  -- Anwender I/O-Register
    CONSTANT c_Tag_Ctrl1_Base_Addr:              Integer := 16#0580#;  -- Tag-Control
    CONSTANT c_IOBP_Masken_Base_Addr:            Integer := 16#0630#;  -- IO-Backplane Maske-Register
    CONSTANT c_IOBP_ID_Base_Addr:                Integer := 16#0638#;  -- IO-Backplane Modul-ID-Register
    CONSTANT c_IOBP_READBACK_Base_Addr:          Integer := 16#0670#;  -- IO-Backplane Output Readback Register
    CONSTANT c_DIOB_DAQ_Base_Addr:               Integer := 16#2000#;  -- DAQ Base Address
    CONSTANT pos_threshold:                         Integer := 16#40000#;   -- positive threshold for Beam Loss Check
    CONSTANT neg_threshold:                         Integer := 16#40000#;   -- -negative threshold for Beam Loss Check
--  +============================================================================================================================+
--  |                                                 CONSTANT                                                                   |
--  +============================================================================================================================+

    CONSTANT c_cid_system:     integer range 0 to 16#FFFF#:= 55;     -- extension card: cid_system, CSCOHW=55

    type ID_CID is record
      ID   : std_logic_vector(7 downto 0);
      CID  : integer range 0 to 16#FFFF#;
    end record;
--                                        +--------------- Piggy-ID(Hardware-coding)
--                                        |     +--------- CID(extension card: cid_system)
    CONSTANT c_AW_INLB12S1:   ID_CID:= (x"13", 67);   ---- Piggy-ID(coding), B"0001_0011", FG902_050        -- IO-Modul-Backplane with 12 slots
    CONSTANT c_BP_6LWLI1 :     ID_CID:= (x"04", 75);   ---- SUB-Piggy-ID(coding), B"0000_0100", FG902.110   -- 6x opt In, 
    CONSTANT c_BP_6LWLO1 :     ID_CID:= (x"05", 76);   ---- SUB-Piggy-ID(coding), B"0000_0101", FG902.120   -- 6x opt Out,  
    CONSTANT c_BP_6LEMO1 :     ID_CID:= (x"06", 77);   ---- SUB-Piggy-ID(coding), B"0000_0110", FG902.140   -- 6x lemo Out,  
    constant  stretch_cnt:    integer := 5;                               -- für LED's
    constant  Clk_in_ns:      integer  :=  1000000000 /  clk_sys_in_Hz;          -- (=8ns,    @ 125MHz)
    CONSTANT  CLK_sys_in_ps:  INTEGER  := (1000000000 / (CLK_sys_in_Hz / 1000));  --must actually be half-clk
    constant  C_Strobe_1us:   integer := 1000 / Clk_in_ns;                       -- Number of clocks for 1us
    constant  C_Strobe_2us:   integer := 2000 / Clk_in_ns;                       -- Number of clocks for 2us
    constant  C_Strobe_3us:   integer := 003000 * 1000 / CLK_sys_in_ps;          -- Number of clocks for the Debounce Time of  3uS
    constant  C_Strobe_7us:   integer := 007000 * 1000 / CLK_sys_in_ps;          -- Number of clocks for the Debounce Time of  7uS

   TYPE      t_Integer_Array  is array (0 to 7) of integer range 0 to 16383;

  --------------- Array für die Anzahl der Clock's für die B1dddebounce-Zeiten von 1,2,4,8,16,32,64,128 us ---------------


  constant  Wert_2_Hoch_n:   t_Integer_Array := (001000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   1uS
                                                 002000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   2uS
                                                 004000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   4uS
                                                 008000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of   8uS
                                                 016000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  16uS
                                                 032000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  32uS
                                                 064000 * 1000 / CLK_sys_in_ps,   -- Number of clocks for the Debounce Time of  64uS
                                                 128000 * 1000 / CLK_sys_in_ps);  -- Number of clocks for the Debounce Time of 128uS

  CONSTANT C_Strobe_100ns:  integer range 0 to 16383:= (000100 * 1000 / CLK_sys_in_ps);   -- Number of clocks for the Strobe 100ns

  TYPE   t_Integer_Strobe_Array     is array (0 to 7) of integer range 0 to 65535;
  constant Wert_Strobe_2_Hoch_n : t_Integer_Strobe_Array := (00001, 00002, 00004, 00008, 00016, 00032, 00064, 00128);

  TYPE   t_status_error_update_Array     is array (0 to 7) of integer range 0 to 1023;
--                                                                ( 2^0, 2^1, 2^2, 2^3, 2^4, 2^5, 2^6, 2^7
--                                                                   0    2    4    8   16   32   64  128
  constant Sts_Err_Zeit_2_Hoch_n : t_status_error_update_Array := (005, 010, 020, 040, 080, 160, 320, 640);

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
    Diob_Config1:         out  std_logic_vector(15 downto 0);    -- Data-Reg. AWOut1
    Diob_Config2:         out  std_logic_vector(15 downto 0);    -- Data-Reg. AWOut2
    AW_Config1:           out  std_logic_vector(15 downto 0);    -- Data-Reg. AWOut3
    AW_Config2:           out  std_logic_vector(15 downto 0);    -- Data-Reg. AWOut4
    Mirr_OutReg_Maske:    out  std_logic_vector(15 downto 0);    -- Masking for mirror mode of the source register
    Diob_Config1_wr:      out  std_logic;                        -- write-Strobe, Data-Reg. AWOut1
    Diob_Config2_wr:      out  std_logic;                        -- write-Strobe, Data-Reg. AWOut2
    AW_Config1_wr:        out  std_logic;                        -- write-Strobe, Data-Reg. AWOut3
    AW_Config2_wr:        out  std_logic;                        -- write-Strobe, Data-Reg. AWOut4
    Clr_Tag_Config:       out  std_logic;                        -- Clear Tag-Configurations-Register
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
    Spare0:               in   std_logic;                        -- driven by Master
    Spare1:               in   std_logic;                        -- driven by Master
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    SCU_AW_Input_Reg:     in   t_IO_Reg_1_to_7_Array;            -- Input-Port's  zum SCU-Bus

    Clr_Tag_Config:       in   std_logic;                        -- clear all Tag-Masks
    Max_AWOut_Reg_Nr:     in   integer range 0 to 7;             -- Maximum AWOut Reg number of the application
    Max_AWIn_Reg_Nr:      in   integer range 0 to 7;             -- Maximum AWIn-Reg-Nummenumber of the application
    Tag_matched_7_0:      out  std_logic_vector(7 downto 0);     -- Active on matched Tags for one clock period after matching, one bit for each tag unit

    Tag_Maske_Reg:        out  t_IO_Reg_1_to_7_Array;            -- Tag-Output-Mask for Register 1-7
    Tag_Outp_Reg:         out  t_IO_Reg_1_to_7_Array;            -- Tag-Output-Mask for Register 1-7

    Tag_FG_Start:         out  std_logic;                        -- Start-Puls for the FG
    Tag_Sts:              out  std_logic_vector(15 downto 0);    -- Tag-Status

    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    Tag_Aktiv:            out  std_logic_vector( 7 downto 0);    -- Flag: Bit7 = Tag7 (active) --- Bit0 = Tag0 (active)
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

COMPONENT in_reg
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
    Reg_In1:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In2:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In3:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In4:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In5:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In6:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In7:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In8:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_rd_active:       OUT STD_LOGIC;
    Data_to_SCUB:        OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Dtack_to_SCUB:       OUT STD_LOGIC
  );
END COMPONENT in_reg;


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

  COMPONENT daq 
  generic (
      Base_addr:          unsigned(15 downto 0);
      CLK_sys_in_Hz:      integer := 125_000_000;               
      ch_num:             integer := 16                      
          );
  
  port  (
        Adr_from_SCUB_LA:    in    std_logic_vector(15 downto 0);
        Data_from_SCUB_LA:   in    std_logic_vector(15 downto 0);
        Ext_Adr_Val:         in    std_logic;                    
        Ext_Rd_active:       in    std_logic;                   
        Ext_Wr_active:       in    std_logic;                   
        clk_i:               in    std_logic;                    
        nReset:              in    std_logic;
  
        diob_extension_id:   in    std_logic_vector(15 downto 0);
  
        user_rd_active:      out   std_logic;
        Rd_Port:             out   std_logic_vector(15 downto 0);
        Dtack:               out   std_logic;                    
        daq_srq:             out   std_logic;                    
        HiRes_srq:           out   std_logic;                    
        Timing_Pattern_LA:   in    std_logic_vector(31 downto 0);
        Timing_Pattern_RCV:  in    std_logic;                    
  
        --daq input channels
        daq_dat_i:           in    t_daq_dat (1 to ch_num);    
        daq_ext_trig:        in    t_daq_ctl (1 to ch_num)       
      );
      END COMPONENT daq;


 component Beam_Loss_check is
  generic (
      n            : integer range 0 to 110 :=62;        -- counter pool inputs:  hardware inputs plus test signals      
      m            : integer := 8;       -- gate input width
      WIDTH        : integer := 20;      -- Counter width
      pos_threshold: integer:= pos_threshold;
      neg_threshold: integer:= -neg_threshold      
  );
  port (
      clk_sys       : in std_logic;      -- Clock
      rstn_sys      : in std_logic;      -- Reset
      CLEAR         : in std_logic;      -- Clear counter register
      LOAD          : in std_logic_vector(n-2 downto 0);     -- Load counter register
      watchdog_ena  : in std_logic_vector( 8 downto 0);      
      gate_in_ena   : in std_logic;
      Gate_Mtx      : in std_logic_vector (m-1 downto 0);
      In_Mtx       : in t_IO_Reg_0_to_7_Array; 
      test_sig_sel : std_logic_vector(2 downto 0);
      out_mux_sel  : in std_logic_vector(12 downto 0);
      INTL_Output   : out std_logic_vector(5 downto 0) 
  
  );
  end component Beam_Loss_check;
 

  component front_board_id is 

Port ( clk : in STD_LOGIC;
       nReset : in STD_LOGIC;
       Deb_Sync : in STD_LOGIC_VECTOR(65 downto 0);
       Deb_out   :in STD_LOGIC_VECTOR(65 downto 0);
       IOBP_Masken_Reg1 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg2 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg3 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg4 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg5 : in STD_LOGIC_VECTOR(15 downto 0);
       IOBP_Masken_Reg6 : in STD_LOGIC_VECTOR(15 downto 0);
       PIO_SYNC         : in STD_LOGIC_VECTOR(142 DOWNTO 20);
       IOBP_ID          : in t_id_array;
       INTL_Output      : in std_logic_vector(5 downto 0);
       AW_Output_Reg    : in std_logic_vector(5 downto 0);
       config           : in std_logic_vector(1 downto 0);
       AW_IOBP_Input_Reg     : out t_IO_Reg_1_to_7_Array;
       IOBP_Output     : out std_logic_vector(5 downto 0);     
       IOBP_Input     : out t_IOBP_array;
       IOBP_Aktiv_LED_i   : out t_led_array;
       OUT_SLOT         : out std_logic_vector(5 downto 0);
       ENA_SLOT         : out std_logic_vector(5 downto 0);
       IOBP_Sel_LED     : out t_led_array
);
end component front_board_id;

component IOBP_LED_ID_Module 

port (
        clk_sys           : in  std_logic;      
        rstn_sys          : in  std_logic;      
        Ena_Every_250ns   : in  std_logic; 
        AW_ID             : in  std_logic_vector(7 downto 0); -- Application_ID
        IOBP_LED_ID_Bus_i : in  std_logic_vector(7 downto 0);   -- LED_ID_Bus_In
        IOBP_Aktiv_LED_o  : in  t_led_array;    -- Active LEDs of the "Slave-Boards"
        IOBP_Sel_LED      : in  t_led_array;    -- Sel-LED of the "Slave-Boards"
        IOBP_LED_En       : out std_logic; -- Output-Enable for LED -ID-Bus
        IOBP_STR_rot_o    : out std_logic_vector(12 downto 1);  -- LED-Str Red  for Slave 12-1
        IOBP_STR_gruen_o  : out std_logic_vector(12 downto 1);  -- LED-Str Green for Slave 12-1
        IOBP_STR_ID_o     : out std_logic_vector(12 downto 1);  -- ID-Str Green for Slave 12-1
        IOBP_LED_ID_Bus_o : out std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
        IOBP_ID           : out t_id_array     -- IDs of the "Slave-Boards"
        
        );
 end component IOBP_LED_ID_Module;

--  +============================================================================================================================+
--  |                                                         signal                                                             |
--  +============================================================================================================================+

  signal clk_sys, clk_cal, locked : std_logic;
  signal Debounce_cnt:              integer range 0 to 16383;   -- Clock's for the Debouncing Time

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

  signal Max_AWOut_Reg_Nr:      integer range 0 to 7;           -- Maximal AWOut-Reg-Number of the application
  signal Max_AWIn_Reg_Nr:       integer range 0 to 7;           -- Maximale AWIn-Reg-Number of the application

  signal AWIn_Deb_Time:          integer range 0 to 7;           -- Debounce-Time 2 High "AWIn_Deb_Time", value from DIOB-Config 1
  signal Min_AWIn_Deb_Time:      integer range 0 to 7;           -- Minimal Debounce-Time 2 High"Min_AWIn_Deb_Time"

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
  signal AW_ID:           std_logic_vector(7 downto 0); -- Application_ID


--------------------------- Conf_Sts1 ----------------------------------------------------------------------

  signal DIOB_Config1:           std_logic_vector(15 downto 0);
  signal DIOB_Config2:           std_logic_vector(15 downto 0);
  signal DIOB_Status1:           std_logic_vector(15 downto 0);
  signal DIOB_Status2:           std_logic_vector(15 downto 0);
  signal AW_Config1:             std_logic_vector(15 downto 0);
  signal AW_Config2:             std_logic_vector(15 downto 0);
  signal AW_Status1:             std_logic_vector(15 downto 0);
  signal AW_Status2:             std_logic_vector(15 downto 0);

  signal Diob_Config1_wr:        std_logic;                        -- write-Strobe, Data-Reg. Diob_Config1
  signal Diob_Config2_wr:        std_logic;                        -- write-Strobe, Data-Reg. Diob_Config2
  signal AW_Config1_wr:          std_logic;                        -- write-Strobe, Data-Reg. AW_Config1
  signal AW_Config2_wr:          std_logic;                        -- write-Strobe, Data-Reg. AW_Config2
  signal Clr_Tag_Config:         std_logic;                        -- clear alle Tag-Mask
  signal Conf_Sts1_rd_active:    std_logic;
  signal Conf_Sts1_Dtack:        std_logic;
  signal Conf_Sts1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal LA_Conf_Sts1:           std_logic_vector(15 downto 0);


--------------------------- AWIn ----------------------------------------------------------------------

  signal SCU_AW_Input_Reg:        t_IO_Reg_1_to_7_Array;  -- Input-Register to SCU-Bus
  signal AW_Input_Reg:            t_IO_Reg_1_to_7_Array;  -- Input-Register of the Piggys



--------------------------- AWOut ----------------------------------------------------------------------

  signal SCU_AW_Output_Reg:         t_IO_Reg_1_to_7_Array;  -- Output-Register from SCU-Bus
  signal AW_Output_Reg:             t_IO_Reg_1_to_7_Array;  -- Output-Register to the Piggys

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

  signal Tag_Maske_Reg:          t_IO_Reg_1_to_7_Array;           -- Tag-Output-Mask for Register 1-7
  signal Tag_Outp_Reg:           t_IO_Reg_1_to_7_Array;           -- Tag-Output-Mask for Register 1-7
  signal Tag_Sts:                std_logic_vector(15 downto 0);   -- Tag-Status
  signal Tag_Ctrl1_rd_active:    std_logic;                       -- read data available at 'Data_to_SCUB'-Tag_Ctrl1
  signal Tag_Ctrl1_Dtack:        std_logic;                       -- connect read sources to SCUB-Macro
  signal Tag_Ctrl1_data_to_SCUB: std_logic_vector(15 downto 0);   -- connect Dtack to SCUB-Macro
  signal Tag_Aktiv:              std_logic_vector( 7 downto 0);   -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)
  signal LA_Tag_Ctrl1:           std_logic_vector(15 downto 0);
  signal  Tag_matched_7_0:              STD_LOGIC_VECTOR(7 DOWNTO 0);
  signal hp_la_o:      std_logic_vector(15 downto 0); -- Output für HP-Logicanalysator

  signal s_nLED_User1_i: std_logic;  -- LED3 = User 1
  signal s_nLED_User2_i: std_logic;  -- LED2 = User 2
  signal s_nLED_User3_i: std_logic;  -- LED1 = User 3
  signal s_nLED_User1_o: std_logic;  -- LED3 = User 1
  signal s_nLED_User2_o: std_logic;  -- LED2 = User 2
  signal s_nLED_User3_o: std_logic;  -- LED1 = User 3

  signal uart_txd_out:  std_logic;


    ------------ Mirror-Mode-Signale --------------------------------------------------------------------------------------

  signal AWIn_Reg_Array:        t_IO_Reg_1_to_7_Array;          -- Copy of AWIn-Register in an Array

  signal Mirr_OutReg_Maske:     std_logic_vector(15 downto 0);  -- Masking for Mirror-Modus of the output registers
  signal Mirr_AWOut_Reg_Nr:     integer range 0 to 7;           -- AWOut-Reg-Number

  signal Mirr_AWIn_Reg_Nr:      integer range 0 to 7;           -- AWIn-Reg-Number

  ------------ I/O Front Boards Signals--------------------------------------------------------------------------------------

  signal IOBP_Masken_Reg1:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg2:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg3:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg4:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg5:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg6:        std_logic_vector(15 downto 0);
  signal IOBP_Masken_Reg7:        std_logic_vector(15 downto 0);
  signal IOBP_msk_rd_active:      std_logic;
  signal IOBP_msk_Dtack:          std_logic;
  signal IOBP_msk_data_to_SCUB:   std_logic_vector(15 downto 0);
  signal IOBP_Output_Readback:    t_IO_Reg_0_to_7_Array;
signal IOBP_Output: std_logic_vector(5 downto 0);     -- Outputs "Slave-Karten 1-12"  --but I use only 1-2-3 respectiverly for slot 10-11-12

signal IOBP_Input:  t_IOBP_array;    -- Inputs "Slave-Karten 1-12"
  signal IOBP_Id_Reg1:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg2:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg3:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg4:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg5:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg6:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg7:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_Id_Reg8:            std_logic_vector(15 downto 0) := (OTHERS => '0');
  signal IOBP_id_rd_active:       std_logic;
  signal IOBP_id_Dtack:           std_logic;
  signal IOBP_id_data_to_SCUB:    std_logic_vector(15 downto 0);
  signal IOBP_in_data_to_SCUB:    std_logic_vector(15 downto 0);
  signal IOBP_in_rd_active:       std_logic;
  signal IOBP_in_Dtack:           std_logic;
  signal IOBP_Sel_LED:      t_led_array;    -- Sel-LED's der "Slave-Karten"
  signal IOBP_ID:           t_id_array;     -- IDs of the "Slave-Boards"
  signal IOBP_Aktiv_LED_i:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"
  signal IOBP_Aktiv_LED_o:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"

signal Syn66:        std_logic_vector(65 downto 0);
signal Deb_Sync66:   std_logic_vector(65 downto 0);
signal  Deb66_in:     std_logic_vector(65 downto 0);
signal  Deb66_out:    std_logic_vector(65 downto 0);

signal IOBP_STR_rot_o:    std_logic_vector(12 downto 1);  -- LED-Str Rot  für Slave 12-1
signal IOBP_STR_gruen_o:  std_logic_vector(12 downto 1);  -- LED-Str Grün für Slave 12-1
signal IOBP_STR_ID_o:     std_logic_vector(12 downto 1);  -- ID-Str Grün für Slave 12-1
signal IOBP_LED_ID_Bus_o: std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
signal IOBP_LED_ID_Bus_i: std_logic_vector(7 downto 0);   -- LED_ID_Bus_In
signal IOBP_LED_En:       std_logic;                      -- Output-Enable für LED- ID-Bus
signal Slave_Loop_cnt:      integer range 0 to 12;         -- 1-12   -- Loop-Counter

type   IOBP_LED_state_t is   (IOBP_idle, led_id_wait, led_id_loop, led_str_rot_h, led_str_rot_l, led_gruen,
                              led_str_gruen_h, led_str_gruen_l, iobp_led_dis, iobp_led_z, iobp_id_str_h, iobp_rd_id, iobp_id_str_l, iobp_end);
signal IOBP_state:   IOBP_LED_state_t:= IOBP_idle;
type   IOBP_slot_state_t is   (IOBP_slot_idle, IOBP_slot1, IOBP_slot2,IOBP_slot3,IOBP_slot4,IOBP_slot5,IOBP_slot6,IOBP_slot7,IOBP_slot8,IOBP_slot9,IOBP_slot10,IOBP_slot11,IOBP_slot12);
signal IOBP_slot_state:   IOBP_slot_state_t:= IOBP_slot_idle;
type   t_reg_array         is array (1 to 12) of std_logic_vector(7 downto 0);
signal conf_reg:           t_reg_array;
signal AW_IOBP_Input_Reg:            t_IO_Reg_1_to_7_Array;  -- Input-Register of the Piggy's
signal PIO_ENA_SLOT_1: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_2: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_3: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_4: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_5: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_6: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_7: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_8: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_9: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_10: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_11: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_ENA_SLOT_12: std_logic_vector(5 downto 0):= (OTHERS => '0');

signal PIO_OUT_SLOT_1: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_2: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_3: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_4: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_5: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_6: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_7: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_8: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_9: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_10: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_11: std_logic_vector(5 downto 0):= (OTHERS => '0');
signal PIO_OUT_SLOT_12: std_logic_vector(5 downto 0):= (OTHERS => '0');

--------------------------------------------------------------------------------------

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


signal    In_Mtx:             t_IO_Reg_0_to_7_Array;  -- outputs of the input stage logic
signal    Gate_Mtx:           std_logic_vector (11 downto 0):= (OTHERS => '0');  -- gate signals 
signal    watchdog_ena:       std_logic_vector( 8 downto 0):= (OTHERS => '0');
signal    gate_in_ena:        std_logic :='0';
signal    Gate_In_Mtx:        std_logic_vector (7 downto 0):= (OTHERS => '0');  -- gate outputs from the gate timing sequence control #

type Test_DATA is array (0 to 5)
        of std_logic_vector(7 downto 0);
  constant Test_In_Mtx : Test_DATA :=
            (
             "00100000",
             "00010000",
             "00001000",
             "00000100",
             "00000010",
             "00000001");
signal INTL_Output: std_logic_vector(5 downto 0);     -- Output "Slave-Karten 12"  
signal count_LOAD: std_logic_vector(60 downto 0);
-----------------DAQ-Signale---------------------------------------------------------------------------------------------------

constant daq_ch_num: integer := 16;
signal daq_diob_ID: std_logic_vector(15 downto 0):=x"00FF"; --hard-coded ID Value for DAQ Diob implementation of which bits 3-0 are used 

signal daq_user_rd_active:    std_logic;
signal daq_data_to_SCUB:           std_logic_vector(15 downto 0);-- Data to SCU Bus Macro
signal daq_Dtack:             std_logic;                    -- Dtack to SCU Bus Macro
signal daq_srq:               std_logic;                    -- consolidated irq lines from n daq channels for "channel fifo full"
signal daq_HiRes_srq:         std_logic;                    -- consolidated irq lines from n HiRes channels for "HiRes Daq finished"
--daq input channels signals
signal daq_dat:             t_daq_dat (1 to daq_ch_num) := (others => dummy_daq_dat_in);
signal daq_ext_trig:          t_daq_ctl (1 to daq_ch_num) := (others => dummy_daq_ctl_in);
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
      SCU_AW_Input_Reg    =>  SCU_AW_Input_Reg,          -- the same Input-Port's as for SCU-Bus
      Clr_Tag_Config      =>  Clr_Tag_Config,            -- Clear Tag-Configurations-Register
      Tag_matched_7_0     =>  Tag_matched_7_0,           -- Active on matched Tags for one clock period, one bit for each tag unit
      Max_AWOut_Reg_Nr    =>  Max_AWOut_Reg_Nr,          -- Maximal AWOut-Reg-Number of the application
      Max_AWIn_Reg_Nr     =>  Max_AWIn_Reg_Nr,           -- Maximal AWIn-Reg-Number of the application
      Tag_Maske_Reg       =>  Tag_Maske_Reg,             -- Tag-Output-Mask for Register 1-7
      Tag_Outp_Reg        =>  Tag_Outp_Reg,              -- Tag-Output-Mask for Register 1-7
      Tag_FG_Start        =>  open,              -- Start-Puls for the FG
      Tag_Sts             =>  Tag_Sts,                   -- Tag-Status
      Rd_active           =>  Tag_Ctrl1_rd_active,       -- read data available at 'Data_to_SCUB'-AWOut
      Data_to_SCUB        =>  Tag_Ctrl1_Data_to_SCUB,    -- connect read sources to SCUB-Macro
      Dtack_to_SCUB       =>  Tag_Ctrl1_Dtack,           -- connect Dtack to SCUB-Macro
      Tag_Aktiv           =>  Tag_Aktiv,                 -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)
      LA_Tag_Ctrl         =>  LA_Tag_Ctrl1
      );



--------- AW-Output Mux zu den "Piggys" --------------------

p_AW_Out_Mux:  PROCESS (Tag_Maske_Reg, Tag_Outp_Reg, SCU_AW_Output_Reg)
    BEGin

    for i in 0 to 15 loop

------ Masken-Reg. aus Tag-Ctrl         Daten => Piggy             User-Output-Reg.                Daten => Piggy        Tag aus Tag-Ctrl
------        |                              |                         |                              |                       |
      IF Tag_Maske_Reg(1)(i)  = '0' then AW_Output_Reg(1)(i)  <= SCU_AW_Output_Reg(1)(i);  else  AW_Output_Reg(1)(i)  <= Tag_Outp_Reg(1)(i);  end if;    -- Daten-Reg. AWOut1
      IF Tag_Maske_Reg(2)(i)  = '0' then AW_Output_Reg(2)(i)  <= SCU_AW_Output_Reg(2)(i);  else  AW_Output_Reg(2)(i)  <= Tag_Outp_Reg(2)(i);  end if;    -- Daten-Reg. AWOut2
      IF Tag_Maske_Reg(3)(i)  = '0' then AW_Output_Reg(3)(i)  <= SCU_AW_Output_Reg(3)(i);  else  AW_Output_Reg(3)(i)  <= Tag_Outp_Reg(3)(i);  end if;    -- Daten-Reg. AWOut3
      IF Tag_Maske_Reg(4)(i)  = '0' then AW_Output_Reg(4)(i)  <= SCU_AW_Output_Reg(4)(i);  else  AW_Output_Reg(4)(i)  <= Tag_Outp_Reg(4)(i);  end if;    -- Daten-Reg. AWOut4
      IF Tag_Maske_Reg(5)(i)  = '0' then AW_Output_Reg(5)(i)  <= SCU_AW_Output_Reg(5)(i);  else  AW_Output_Reg(5)(i)  <= Tag_Outp_Reg(5)(i);  end if;    -- Daten-Reg. AWOut5
      IF Tag_Maske_Reg(6)(i)  = '0' then AW_Output_Reg(6)(i)  <= SCU_AW_Output_Reg(6)(i);  else  AW_Output_Reg(6)(i)  <= Tag_Outp_Reg(6)(i);  end if;    -- Daten-Reg. AWOut6
      IF Tag_Maske_Reg(7)(i)  = '0' then AW_Output_Reg(7)(i)  <= SCU_AW_Output_Reg(7)(i);  else  AW_Output_Reg(7)(i)  <= Tag_Outp_Reg(7)(i);  end if;    -- Daten-Reg. AWOut7
    end loop;
  END PROCESS p_AW_Out_Mux;



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
      Reg_IO6            =>  IOBP_Masken_Reg6,
      Reg_IO7            =>  IOBP_Masken_Reg7,
      Reg_IO8            =>  open,
      Reg_rd_active      =>  IOBP_msk_rd_active,
      Dtack_to_SCUB      =>  IOBP_msk_Dtack,
      Data_to_SCUB       =>  IOBP_msk_data_to_SCUB
    );

IOBP_ID_Reg: in_reg
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
      Reg_In1            =>  IOBP_ID_Reg1,
      Reg_In2            =>  IOBP_ID_Reg2,
      Reg_In3            =>  IOBP_ID_Reg3,
      Reg_In4            =>  IOBP_ID_Reg4,
      Reg_In5            =>  IOBP_ID_Reg5,
      Reg_In6            =>  IOBP_ID_Reg6,
      Reg_In7            =>  IOBP_ID_Reg7,
      Reg_In8            =>  IOBP_ID_Reg8,
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
    
        DAQ_modul: daq
        GENERIC MAP(
          Base_addr           =>  to_unsigned(c_DIOB_DAQ_Base_Addr, 16),
          CLK_sys_in_Hz       => 125000000,        
          ch_num => daq_ch_num                  
              )
        
        PORT MAP  (
        
            Adr_from_SCUB_LA      => ADR_from_SCUB_LA,
            Data_from_SCUB_LA     => Data_from_SCUB_LA,
            Ext_Adr_Val           => Ext_Adr_Val,
            Ext_Rd_active         => Ext_Rd_active,
            Ext_Wr_active         => Ext_Wr_active,
            clk_i                 => clk_sys,
            nReset                => rstn_sys,
        
            diob_extension_id     =>  daq_diob_ID,
            user_rd_active        =>  daq_user_rd_active,   
            Rd_Port               =>  daq_data_to_SCUB,  
            Dtack                 =>  daq_Dtack,
            daq_srq               =>  daq_srq,
            HiRes_srq             =>  daq_HiRes_srq,   
            Timing_Pattern_LA     => Timing_Pattern_LA,
            Timing_Pattern_RCV    => Timing_Pattern_RCV, 
        
            --daq input channels
            daq_dat_i             => daq_dat,
            daq_ext_trig          =>  daq_ext_trig
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

hp_la_o <=  x"0000";  --test_out(15 downto 0);
test_port_in_0 <= x"0000"; --- kein Clock's am Teststecker

test_clocks <=  X"0"                                                                              -- bit15..12            
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
   intr_in                 => '0'& '0' & tmr_irq &  daq_srq           -- bit 15..12
                               & daq_HiRes_srq & '0' & '0' &'0'       -- bit 11..8

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
                        AW_Port1_rd_active,       AW_Port1_data_to_SCUB,
                        Tag_Ctrl1_rd_active,      Tag_Ctrl1_data_to_SCUB,
                        Conf_Sts1_rd_active,      Conf_Sts1_data_to_SCUB,
                        tmr_rd_active,            tmr_data_to_SCUB,
                        IOBP_msk_rd_active,       IOBP_msk_data_to_SCUB,
                        IOBP_id_rd_active,        IOBP_id_data_to_SCUB,
                        IOBP_in_rd_active,        IOBP_in_data_to_SCUB,
                        daq_user_rd_active,       daq_data_to_SCUB
                      )


  variable sel: unsigned(8 downto 0);

  begin
    sel :=  daq_user_rd_active & 
            IOBP_in_rd_active  & tmr_rd_active &  wb_scu_rd_active & clk_switch_rd_active &
            Conf_Sts1_rd_active & Tag_Ctrl1_rd_active & IOBP_msk_rd_active & IOBP_id_rd_active ;

  case sel IS
      when "100000000" => Data_to_SCUB <= daq_data_to_SCUB;
      when "010000000" => Data_to_SCUB <= IOBP_in_data_to_SCUB;
      when "001000000" => Data_to_SCUB <= tmr_data_to_SCUB;
      when "000100000" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when "000010000" => Data_to_SCUB <= clk_switch_rd_data;
      when "000001000" => Data_to_SCUB <= Conf_Sts1_data_to_SCUB;
      when "000000100" => Data_to_SCUB <= Tag_Ctrl1_data_to_SCUB;
      when "000000010" => Data_to_SCUB <= IOBP_msk_data_to_SCUB;
      when "000000001" => Data_to_SCUB <= IOBP_id_data_to_SCUB;

      when others      => Data_to_SCUB <= (others => '0');
    end case;
  end process rd_port_mux;

-------------- Dtack_to_SCUB -----------------------------

    Dtack_to_SCUB <= ( tmr_dtack  or AW_Port1_Dtack   or wb_scu_dtack  or clk_switch_dtack  or Conf_Sts1_Dtack  or Tag_Ctrl1_Dtack  or
                         IOBP_msk_Dtack   or IOBP_id_Dtack    or    IOBP_in_Dtack or daq_Dtack );

    A_nDtack <= NOT(SCUB_Dtack);
    A_nSRQ   <= NOT(SCUB_SRQ);

--  +============================================================================================================================+
--  |            §§§                        Anwender-IO: IOBP (INLB12S1)  -- FG902_050                                           |
--  +============================================================================================================================+
Deb66:  for I in 0 to 65 generate
DB_I:  diob_debounce
GENERIC MAP (DB_Tst_Cnt   => 3,
             Test         => 0)             --
          port map(DB_Cnt => Debounce_cnt,     -- Debounce-Zeit in Clock's
                   DB_in  => Deb66_in(I),   -- Signal-Input
                   Reset  => not rstn_sys,  -- Powerup-Reset
                   clk    => clk_sys,       -- Sys-Clock
                   DB_Out => Deb66_out(I)); -- Debounce-Signal-Out
end generate Deb66;
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
count_load_ass_proc: process(clk_sys,  rstn_sys)

    begin
      if (not rstn_sys = '1') then
        count_LOAD <= (others =>'0');
      ELSIF (clk_sys'EVENT AND clk_sys = '1' ) THEN
        for i in 1 to (54) loop
           count_LOAD(i) <='1';
        end loop;
      end if;
    end process;

BLM_Module: Beam_Loss_check 
  generic map (
      n => 62,
      m => 8,       -- gate input width
      WIDTH => 20,      -- Counter width
      pos_threshold => pos_threshold,
      neg_threshold => -neg_threshold 
  )
  port map(
      clk_sys      => clk_sys,    -- Clock
      rstn_sys     => rstn_sys,     -- Reset
      CLEAR        => AW_Config1(4),    --not used yet until now
      LOAD         =>  count_LOAD,-- Load counter register
      watchdog_ena => watchdog_ena,
      gate_in_ena  => gate_in_ena, 
      Gate_Mtx     => gate_Mtx(7 downto 0), 
      In_Mtx       => In_Mtx,
      test_sig_sel => AW_Config2( 2 downto 0), --AW_Config2 not used yet, now it selects a test signal for the counter_matrix
      out_mux_sel => AW_Config2(15 downto 3),   --
      INTL_Output  => INTL_Output
  
  );

front_board_id_Module: front_board_id 
port map ( clk               => clk_sys,
           nReset            => rstn_sys,
           Deb_Sync          => Deb_Sync66,
           Deb_out           => Deb66_out,
           IOBP_Masken_Reg1  => IOBP_Masken_Reg1,
           IOBP_Masken_Reg2  => IOBP_Masken_Reg2,
           IOBP_Masken_Reg3  => IOBP_Masken_Reg3,
           IOBP_Masken_Reg4  => IOBP_Masken_Reg4,
           IOBP_Masken_Reg5  => IOBP_Masken_Reg5,
           IOBP_Masken_Reg6  => IOBP_Masken_Reg6,
           PIO_SYNC          => PIO_SYNC(142 DOWNTO 20),
           IOBP_ID           => IOBP_ID,
           INTL_Output       => INTL_Output,
           AW_Output_Reg     => AW_Output_Reg(6)(11 downto  6),
           config            => AW_Config2(1 downto 0),
           AW_IOBP_Input_Reg => AW_IOBP_Input_Reg,
           IOBP_Output       => IOBP_Output,
           IOBP_Input        => IOBP_Input,
           IOBP_Aktiv_LED_i    => IOBP_Aktiv_LED_i,
           OUT_SLOT          => PIO_OUT_SLOT_12,
           ENA_SLOT          => PIO_ENA_SLOT_12, 
           IOBP_Sel_LED      => IOBP_Sel_LED
);

     -------------------------------------------------------------------------------------------------------
     ------------------------------ Loop für LED_Output's und ID read --------------------------------------
     -------------------------------------------------------------------------------------------------------

     P_IOBP_LED_ID_Loop_module: IOBP_LED_ID_Module 

      port map (
              clk_sys           => clk_sys,      
              rstn_sys          => rstn_sys,    
              Ena_Every_250ns   => Ena_Every_250ns,
              AW_ID             => AW_ID,
              IOBP_LED_ID_Bus_i => IOBP_LED_ID_Bus_i,
              IOBP_Aktiv_LED_o  => IOBP_Aktiv_LED_o,
              IOBP_Sel_LED      => IOBP_Sel_LED,
              IOBP_LED_En       => IOBP_LED_En,
              IOBP_STR_rot_o    => IOBP_STR_rot_o,
              IOBP_STR_gruen_o  => IOBP_STR_gruen_o,
              IOBP_STR_ID_o     => IOBP_STR_ID_o,
              IOBP_LED_ID_Bus_o => IOBP_LED_ID_Bus_o,
              IOBP_ID           => IOBP_ID
              
              );



  --  ###############################################################################################################################
--  #####                                                                                                                     #####
--  #####                             Input-Muliplexer to SCU-Bus for the Mirror-Mode                                         #####
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

    --############################# Mirror-Mode ##################################

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

p_stecker: PROCESS (clk_sys, rstn_sys, Powerup_Done, AW_ID,signal_tap_clk_250mhz, A_SEL,
            PIO_SYNC, PIO_SYNC1, PIO_ENA, PIO_ENA_SYNC, PIO_OUT, PIO_OUT_SYNC, PIO, CLK_IO,
            AWIn_Deb_Time, Min_AWIn_Deb_Time, 
            DIOB_Status1, DIOB_Status2, AW_Status1, AW_Status2,
            AW_Input_Reg,
            DIOB_Config1,    DIOB_Config2,    AW_Config1,    AW_Config2,
            DIOB_Config1_wr, DIOB_Config2_wr, AW_Config1_wr, AW_Config2_wr,
            AW_Output_Reg,
            UIO_SYNC, UIO_SYNC1, UIO_ENA, UIO_ENA_SYNC, UIO_OUT, UIO_OUT_SYNC, UIO,
            hp_la_o, local_clk_is_running, clk_blink,
            s_nLED_Sel, s_nLED_Dtack, s_nLED_inR, s_nLED_User1_o, s_nLED_User2_o, s_nLED_User3_o,
            Tag_Sts, Timing_Pattern_LA, Tag_Aktiv,  
            AWOut_Reg1_wr, AWOut_Reg2_wr,         
            IOBP_Masken_Reg1, IOBP_Masken_Reg2, IOBP_Masken_Reg3, IOBP_Masken_Reg4, IOBP_Masken_Reg5, IOBP_Masken_Reg6, IOBP_Masken_Reg7, 
            IOBP_LED_ID_Bus_i, IOBP_LED_ID_Bus_o, IOBP_ID, IOBP_LED_En, IOBP_STR_rot_o, IOBP_STR_gruen_o, IOBP_STR_ID_o,
            IOBP_Id_Reg1, IOBP_Id_Reg2, IOBP_Id_Reg3, IOBP_Id_Reg4, IOBP_Id_Reg5, IOBP_Id_Reg6,           
            IOBP_Output, IOBP_Input, Deb66_out, Deb66_in, Syn66, AW_IOBP_Input_Reg, IOBP_Aktiv_LED_i
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
    Diob_Status1(5  downto 0)   <= Tag_Sts(5 downto 0);  -- Tag-Ctrl Status
    Diob_Status2(15 downto 8)   <= (OTHERS => '0');      -- Reserve
    Diob_Status2( 7 downto 0)   <= Tag_Aktiv;            -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)

    AW_Status1                  <= (OTHERS => '0');      -- Input-Port-AW_Sts1
    AW_Status2                  <= (OTHERS => '0');      -- Input-Port-AW_Sts2

    A_Tclk                      <= '0';  -- Clock  für HP-Logic-Analysator

    s_nLED_User1_i              <= '0';        -- LED3 = User 1, -- frei --
    s_nLED_User2_i              <= '0';        -- LED3 = User 2, -- frei --
    s_nLED_User3_i              <= '0';        -- LED3 = User 3, -- frei --

    IOBP_LED_ID_Bus_i           <=             (OTHERS => '1');     -- Data_Output "Slave-Karte 1-12"

    IOBP_Id_Reg1                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
    IOBP_Id_Reg2                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
    IOBP_Id_Reg3                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
    IOBP_Id_Reg4                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
    IOBP_Id_Reg5                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register
    IOBP_Id_Reg6                <=  (OTHERS => '0');    -- IO-Backplane_ID_Register

    IOBP_Output_Readback        <=  (OTHERS => (OTHERS => '0'));  -- IO-Backplane_ID_Register Readback
    Deb66_in                    <= (OTHERS => '0');
    Syn66                       <= (OTHERS => '0');
    Deb_Sync66                  <= (OTHERS => '0');
    --#################################################################################
    --###                                                                           ###
    --###                    IO-Stecker-Test mit "BrückenStecker                    ###
    --###                                                                           ###
    --#################################################################################


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

    UIO_OUT(0)  <= '0';
    UIO_ENA(0)  <= '1';       -- Output-Enable für Interlock-Bit
    AW_Input_Reg(6)   <=  Timing_Pattern_LA(31 downto 16);  -- H-Word vom Timing_Pattern
    AW_Input_Reg(7)   <=  Timing_Pattern_LA(15 downto 0);   -- L-Word vom Timing_Pattern


 if AW_ID(7 downto 0) =  c_AW_INLB12S1.ID  then

--########################################################################################

extension_cid_group  <= c_AW_INLB12S1.CID; -- extension card: cid_group, new Zwischenplane "FG902_xxx"

extension_cid_system <= c_cid_system;       -- extension card: CSCOHW

AW_Status1(15 downto 0)  <=  (OTHERS => '0');					    -- Unbenutzte Status-Bits
AW_Status2(15 downto 0)  <=  (OTHERS => '0');					    -- Unbenutzte Status-Bits

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

IOBP_Id_Reg6(15 downto 8) <=  IOBP_ID(12);  -- ID-Input vom  IO-Modul Nr. 12
IOBP_Id_Reg6( 7 downto 0) <=  IOBP_ID(11);  -- ID-Input vom  IO-Modul Nr. 11
IOBP_Id_Reg5(15 downto 8) <=  IOBP_ID(10);  -- ID-Input vom  IO-Modul Nr. 10
IOBP_Id_Reg5( 7 downto 0) <=  IOBP_ID(9);   -- ID-Input vom  IO-Modul Nr. 9
IOBP_Id_Reg4(15 downto 8) <=  IOBP_ID(8);   -- ID-Input vom  IO-Modul Nr. 8
IOBP_Id_Reg4( 7 downto 0) <=  IOBP_ID(7);   -- ID-Input vom  IO-Modul Nr. 7
IOBP_Id_Reg3(15 downto 8) <=  IOBP_ID(6);   -- ID-Input vom  IO-Modul Nr. 6
IOBP_Id_Reg3( 7 downto 0) <=  IOBP_ID(5);   -- ID-Input vom  IO-Modul Nr. 5
IOBP_Id_Reg2(15 downto 8) <=  IOBP_ID(4);   -- ID-Input vom  IO-Modul Nr. 4
IOBP_Id_Reg2( 7 downto 0) <=  IOBP_ID(3);   -- ID-Input vom  IO-Modul Nr. 3
IOBP_Id_Reg1(15 downto 8) <=  IOBP_ID(2);   -- ID-Input vom  IO-Modul Nr. 2
IOBP_Id_Reg1( 7 downto 0) <=  IOBP_ID(1);   -- ID-Input vom  IO-Modul Nr. 1

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
( PIO_ENA(56),  PIO_ENA(62),  PIO_ENA(54),  PIO_ENA(60),  PIO_ENA(52),  PIO_ENA(58)) <= PIO_ENA_SLOT_1;
( PIO_ENA(96),  PIO_ENA(102), PIO_ENA(94),  PIO_ENA(100), PIO_ENA(92),  PIO_ENA(98)) <= PIO_ENA_SLOT_2;
( PIO_ENA(73),  PIO_ENA(79),  PIO_ENA(71),  PIO_ENA(77),  PIO_ENA(69),  PIO_ENA(75)) <= PIO_ENA_SLOT_3;
( PIO_ENA(101), PIO_ENA(93),  PIO_ENA(103), PIO_ENA(91),  PIO_ENA(105), PIO_ENA(89)) <= PIO_ENA_SLOT_4;
( PIO_ENA(53),  PIO_ENA(63),  PIO_ENA(55),  PIO_ENA(61),  PIO_ENA(57),  PIO_ENA(59)) <= PIO_ENA_SLOT_5;
( PIO_ENA(119), PIO_ENA(111), PIO_ENA(121), PIO_ENA(109), PIO_ENA(123), PIO_ENA(107))<= PIO_ENA_SLOT_6;
( PIO_ENA(35),  PIO_ENA(45),  PIO_ENA(37),  PIO_ENA(43),  PIO_ENA(39),  PIO_ENA(41)) <= PIO_ENA_SLOT_7;
( PIO_ENA(137), PIO_ENA(129), PIO_ENA(139), PIO_ENA(127), PIO_ENA(141), PIO_ENA(125))<= PIO_ENA_SLOT_8;
( PIO_ENA(30),  PIO_ENA(20),  PIO_ENA(28),  PIO_ENA(22),  PIO_ENA(26),  PIO_ENA(24)) <= PIO_ENA_SLOT_9;
( PIO_ENA(130), PIO_ENA(138), PIO_ENA(128), PIO_ENA(140), PIO_ENA(126), PIO_ENA(142))<= PIO_ENA_SLOT_10;
( PIO_ENA(48),  PIO_ENA(38),  PIO_ENA(46),  PIO_ENA(40),  PIO_ENA(44),  PIO_ENA(42)) <= PIO_ENA_SLOT_11;
( PIO_ENA(112), PIO_ENA(120), PIO_ENA(110), PIO_ENA(122), PIO_ENA(108), PIO_ENA(124))<= PIO_ENA_SLOT_12;

( PIO_OUT(56),  PIO_OUT(62),  PIO_OUT(54),  PIO_OUT(60),  PIO_OUT(52),  PIO_OUT(58)) <= PIO_OUT_SLOT_1;
( PIO_OUT(96),  PIO_OUT(102), PIO_OUT(94), PIO_OUT(100),  PIO_OUT(92),  PIO_OUT(98)) <= PIO_OUT_SLOT_2;
( PIO_OUT(73),  PIO_OUT(79),  PIO_OUT(71),  PIO_OUT(77),  PIO_OUT(69),  PIO_OUT(75)) <= PIO_OUT_SLOT_3;
( PIO_OUT(101), PIO_OUT(93),  PIO_OUT(103), PIO_OUT(91),  PIO_OUT(105), PIO_OUT(89)) <= PIO_OUT_SLOT_4;
( PIO_OUT(53),  PIO_OUT(63),  PIO_OUT(55),  PIO_OUT(61),  PIO_OUT(57),  PIO_OUT(59)) <= PIO_OUT_SLOT_5;
( PIO_OUT(119), PIO_OUT(111), PIO_OUT(121), PIO_OUT(109), PIO_OUT(123), PIO_OUT(107))<= PIO_OUT_SLOT_6;
( PIO_OUT(35),  PIO_OUT(45),  PIO_OUT(37),  PIO_OUT(43),  PIO_OUT(39),  PIO_OUT(41)) <= PIO_OUT_SLOT_7;
( PIO_OUT(137), PIO_OUT(129), PIO_OUT(139), PIO_OUT(127), PIO_OUT(141), PIO_OUT(125))<= PIO_OUT_SLOT_8;
( PIO_OUT(30),  PIO_OUT(20),  PIO_OUT(28),  PIO_OUT(22),  PIO_OUT(26),  PIO_OUT(24)) <= PIO_OUT_SLOT_9;
( PIO_OUT(130), PIO_OUT(138), PIO_OUT(128), PIO_OUT(140), PIO_OUT(126), PIO_OUT(142))<= PIO_OUT_SLOT_10;
( PIO_OUT(48),  PIO_OUT(38),  PIO_OUT(46),  PIO_OUT(40),  PIO_OUT(44),  PIO_OUT(42)) <= PIO_OUT_SLOT_11;
( PIO_OUT(112), PIO_OUT(120), PIO_OUT(110), PIO_OUT(122), PIO_OUT(108), PIO_OUT(124))<= PIO_OUT_SLOT_12;

AW_Input_Reg<= AW_IOBP_Input_Reg;
In_Mtx (0) <= AW_IOBP_Input_Reg(1);
In_Mtx (1) <= AW_IOBP_Input_Reg(2);
In_Mtx (2) <= AW_IOBP_Input_Reg(3);
In_Mtx (3) <= AW_IOBP_Input_Reg(4);
In_Mtx (4) <= "00"& Test_In_Mtx(0) & AW_IOBP_Input_Reg(5)(5 downto 0);
In_Mtx (5) <= Test_In_Mtx(2) & Test_In_Mtx(1);
In_Mtx (6) <= Test_In_Mtx(4) & Test_In_Mtx(3);
In_Mtx (7) <= "00000000" & Test_In_Mtx(5);

Gate_Mtx(5 downto 0) <= AW_IOBP_Input_Reg(5)(11 downto 6);
Gate_Mtx(11 downto 6) <= AW_IOBP_Input_Reg(6)(5 downto 0);

---output readback
IOBP_Output_Readback(0) <= "0000000000" & IOBP_Output;
IOBP_Output_Readback(1) <= (OTHERS => '0');
IOBP_Output_Readback(2) <= (OTHERS => '0');
IOBP_Output_Readback(3) <= (OTHERS => '0');
IOBP_Output_Readback(4) <= (OTHERS => '0');
IOBP_Output_Readback(5) <= (OTHERS => '0');
IOBP_Output_Readback(6) <= (OTHERS => '0');
IOBP_Output_Readback(7) <= (OTHERS => '0');

--################################ Debounce oder Sync Input's  ##################################

--  Deb66_in = H-Aktiv             IOBP_Input = L-Aktiv
--        |                                |
Deb66_in( 5 DOWNTO  0)   <=  not IOBP_Input( 1);  -- Input-Daten
Deb66_in(11 DOWNTO  6)   <=  not IOBP_Input( 2);
Deb66_in(17 DOWNTO 12)   <=  not IOBP_Input( 3);
Deb66_in(23 DOWNTO 18)   <=  not IOBP_Input( 4);
Deb66_in(29 DOWNTO 24)   <=  not IOBP_Input( 5);
Deb66_in(35 DOWNTO 30)   <=  not IOBP_Input( 6);
Deb66_in(41 DOWNTO 36)   <=  not IOBP_Input( 7);
Deb66_in(47 DOWNTO 42)   <=  not IOBP_Input( 8);
Deb66_in(53 DOWNTO 48)   <=  not IOBP_Input( 9);
Deb66_in(59 DOWNTO 54)   <=   not IOBP_Input( 10);
Deb66_in(65 DOWNTO 60)   <=   not IOBP_Input( 11);

--  Syn66 = H-Aktiv             IOBP_Input = L-Aktiv
--                                      |
Syn66 ( 5 DOWNTO  0)   <=  not IOBP_Input( 1);  -- Input-Daten
Syn66(11 DOWNTO  6)   <=  not IOBP_Input( 2);
Syn66(17 DOWNTO 12)   <=  not IOBP_Input( 3);
Syn66(23 DOWNTO 18)   <=  not IOBP_Input( 4);
Syn66(29 DOWNTO 24)   <=  not IOBP_Input( 5);
Syn66(35 DOWNTO 30)   <=  not IOBP_Input( 6);
Syn66(41 DOWNTO 36)   <=  not IOBP_Input( 7);
Syn66(47 DOWNTO 42)   <=  not IOBP_Input( 8);
Syn66(53 DOWNTO 48)   <=  not IOBP_Input( 9);
Syn66(59 DOWNTO 54)   <=  not IOBP_Input( 10);
Syn66(65 DOWNTO 60)   <=  not IOBP_Input( 11);

IF  (Diob_Config1(11) = '1')  THEN Deb_Sync66 <=  Syn66;         -- Dobounce = Abgeschaltet ==> nur Synchronisation
                         ELSE Deb_Sync66 <=  Deb66_out;     -- Debounce und Synchronisation
END IF;

---------------------------------------------------------------------------------------------------------
  --################################      daq_channels assignments     ##################################
    daq_dat(1) <= "0000"& AW_IOBP_Input_Reg(1)(11 downto 0);
    daq_dat(2) <= "0000"& AW_IOBP_Input_Reg(2)(11 downto 0);
    daq_dat(3) <= "0000"& AW_IOBP_Input_Reg(3)(11 downto 0);
    daq_dat(4) <= "0000"& AW_IOBP_Input_Reg(4)(11 downto 0);
    daq_dat(5) <= "0000"& AW_IOBP_Input_Reg(5)(11 downto 0);
    daq_dat(6)(5 downto 0) <= AW_IOBP_Input_Reg(6)(5 downto 0);
    daq_dat(7)(5 downto 0) <= IOBP_Output;
    daq_diob_ID(15 downto 0)<= "00000000"& c_AW_INLB12S1.ID;
--############################################################################################################


else
   
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

  END if;

  END IF;

END PROCESS p_stecker;

end architecture;
