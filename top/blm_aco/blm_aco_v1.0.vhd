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
--     15-6|  free                                                                                                                --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--      5-0|  Tag-Ctrl Status                                                                                                     --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--                                                                                                                                --
--                                                                                                                                --
--      Base_addr +3 : DIOB-Status-Register2 (the status bits are deleted after reading)                                          --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--   15-8 |  free                                                                                                                 --
--   -----+-------------------------------------------------------------------------------------------------------------------    --
--    7-0 |  Tag_Active           -- Flag: Bit7 = Tag7 (active) --- Bit0 = Tag0 (active)                                          --                                                                                                        --
--   -----+-------------------------------------------------------------------------------------------------------------------    --

--                                                                                                                                --
--                                                                                                                                --
--     Base_addr + 4 – Base_addr +6  reserved for expansion                                                                       --
-----+------------------------------------------------------------------------------------------------------------------
--     Base_addr + 7 Configuration register1 for interface part: The bits in the user (piggy)config register1 have a different    --
--     meaning for each piggy                                                                                                     --
--                                                                                                                                --
------------------------------------------------------------------------------------------------------------------------------------
entity blm_aco is
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
end blm_aco;


architecture blm_aco_arch_for_Beam_Loss_Mon of blm_aco is

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
    CONSTANT c_Status_READBACK_Base_Addr:        Integer := 16#0670#;  -- IO-Backplane Output Readback Register: 24 x 16 bit registers --> +18h 
  --  CONSTANT c_DIOB_DAQ_Base_Addr:               Integer := 16#2000#;  -- DAQ Base Address
    CONSTANT c_BLM_thres_Base_Addr:              Integer := 16#0700#;  -- BLM thresholds for the counter pool: 512 16 bit registers--> + 200h
    CONSTANT c_BLM_in_sel_Base_Addr:             Integer := 16#1000#;   --BLM input mux select registers :      128 16 bit registers -->80h
    CONSTANT c_BLM_out_sel_Base_Addr:            Integer := 16#1100#;   --BLM output mux select registers :      130 16 bit registers -->82h 
    CONSTANT c_BLM_ctrl_Base_Addr:               Integer := 16#1200#;   --BLM control registers: 15 x 16 bit registers --7Fh
    CONSTANT c_BLM_event_readout_Base_Addr:      Integer := 16#1280#;   --BLM event readout registers: 8 x 16 bit registers
    CONSTANT c_BLM_event_ctrl_Base_Addr:         Integer := 16#1288#;   --BLM event control registers: 8 x 16 bit registers
    CONSTANT c_BLM_counter_readout_Base_Addr:    Integer := 16#1290#;   --BLM counters readout registers: 256 x 16 bit registers 
  --  CONSTANT c_BLM_mem_thres_Base_Addr:          Integer := 16#1390#;   --BLM thresholds from local RAM : 512 x 16 bit registers


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
  
    CONSTANT c_BP_6LemoI1 :    ID_CID:= (x"03", 74);   ---- SUB-Piggy-ID(Codierung), B"0000_0011", FG902.130 -- 6xlemo In,  
    CONSTANT c_BP_6LWLI1 :     ID_CID:= (x"04", 75);   ---- SUB-Piggy-ID(Codierung), B"0000_0100", FG902.110 -- 6x opt In,   
    CONSTANT c_BP_6LWLO1 :     ID_CID:= (x"05", 76);   ---- SUB-Piggy-ID(Codierung), B"0000_0101", FG902.120 -- 6x opt Out,  
    CONSTANT c_BP_6LEMO1 :     ID_CID:= (x"06", 77);   ---- SUB-Piggy-ID(Codierung), B"0000_0110", FG902.140 -- 6x opt Out,  
    CONSTANT c_BP_6DigIn1:     ID_CID:= (x"07", 78);   ---- SUB-Piggy-ID(Codierung), B"0000_0111", FG902.150 -- 6xDIgIn1 IN,  

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
end component zeitbasis;


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
component deglitcher 
  generic	(nr_stages : integer := 2   --3
);
port	
(
    clock: in std_logic;          
  reset: in std_logic;          
  degl_in : in std_logic;
  degl_out: out std_logic
);
end component deglitcher; 

component gate_deglitcher 
  generic	(nr_stages : integer := 10   --3
);
port	
(
    clock: in std_logic;          
  reset: in std_logic;          
  degl_in : in std_logic;
  degl_out: out std_logic
);
end component gate_deglitcher; 

 

component Beam_Loss_check is
  generic (
    
    WIDTH        : integer := 30     -- Counter width
       
);
port (
    clk_sys           : in std_logic;      -- Clock
    rstn_sys          : in std_logic;      -- Reset

   -- IN BLM 
    BLM_data_in       : in std_logic_vector(53 downto 0);
    BLM_gate_in       : in std_logic_vector(11 downto 0);
    BLM_tst_ck_sig    : in std_logic_vector (10 downto 0);
    IOBP_LED_nr       : in std_logic_vector(3 downto 0);
    --IN registers
    pos_threshold           : in t_BLM_th_Array; --t_BLM_th_Array is array (0 to 127) of std_logic_vector(31 downto 0);
    neg_threshold           : in t_BLM_th_Array ;
    BLM_wdog_hold_time_Reg  : in std_logic_vector(15 downto 0);
    BLM_wd_reset            : in std_logic_vector(53 downto 0);
    BLM_gate_hold_time_Reg  : in  t_BLM_gate_hold_Time_Array;
    BLM_ctrl_Reg            : in std_logic_vector(15 downto 0); --bit 0 = counter RESET, bit 1 = counter LOAD, bit 2: when 0 the outputs of board in slot 12 are the direct outptuts of the output OR, 
    --   when 1, the outputs in slot 12 are the values of AW_Output_Reg(6),  bit 15..3 free
    BLM_gate_seq_prep_ck_sel_Reg : in std_logic_vector(15 downto 0);
    BLM_gate_recover_Reg : in std_logic_vector(15 downto 0);
  --  BLM_gate_seq_in_ena_Reg : in std_logic_vector(15 downto 0); --"00"& ena for gate board2 &"00" & ena for gate board1 
    BLM_in_sel_Reg          : in t_BLM_reg_Array; --128 x (4 bit for gate ena & 6 bit for up signal ena & 6 for down signal ena)
   BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;    -- 122 x 16 bits = Reg120-0:  "0000" and 6 x (54 watchdog errors  + 12 gate errors + 256 counters overflows outputs) 
                                                    -- + 4 more registers for 6 x 12 input gate (= 72 bits) to be send to the outputs.    
                                                    --=> 126 registers             
                                                    --   REg127             ex Reg121: counter outputs buffering enable (bit 15) and buffered output select (bit 7-0). Bits 14-8 not used     
    -- OUT register
   -- BLM_status_Reg    : out t_IO_Reg_0_to_25_Array ;



   ev_prepare_reg : in std_logic_vector(11 downto 0);
   ev_recover_reg: in std_logic_vector(11 downto 0);
   ev_counter_reset: in std_logic;
   ev_thr_load: in std_logic;
   virt_acc: in std_logic_vector(11 downto 0);


   BLM_status_Reg    : out t_IO_Reg_0_to_29_Array ;

   counter_readout_reg: out t_BLM_th_Array  ;
      -- OUT BLM
      BLM_Out           : out std_logic_vector(5 downto 0) 
);

  end component Beam_Loss_check;
 

component front_board_id is 

  Port 
      ( clk : in STD_LOGIC;
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
      -- nBLM_out_ena      : in std_logic;
       AW_IOBP_Input_Reg     : out t_IO_Reg_1_to_7_Array;
       IOBP_Output     : out std_logic_vector (5 downto 0);     
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
        --Ena_Every_250ns   : in  std_logic; 
        Ena_Every_500ns   : in std_logic;
        AW_ID             : in  std_logic_vector(7 downto 0); -- Application_ID
        IOBP_LED_ID_Bus_i : in  std_logic_vector(7 downto 0);   -- LED_ID_Bus_In
        IOBP_Aktiv_LED_o  : in  t_led_array;    -- Active LEDs of the "Slave-Boards"
        IOBP_Sel_LED      : in  t_led_array;    -- Sel-LED of the "Slave-Boards"
        IOBP_LED_En       : out std_logic; -- Output-Enable for LED -ID-Bus
        IOBP_STR_rot_o    : out std_logic_vector(12 downto 1);  -- LED-Str Red  for Slave 12-1
        IOBP_STR_gruen_o  : out std_logic_vector(12 downto 1);  -- LED-Str Green for Slave 12-1
        IOBP_STR_ID_o     : out std_logic_vector(12 downto 1);  -- ID-Str Green for Slave 12-1
        IOBP_LED_ID_Bus_o : out std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
        IOBP_ID           : out t_id_array ;    -- IDs of the "Slave-Boards"
        IOBP_LED_state_nr : out std_logic_vector(3 downto 0)
        
        );
 end component IOBP_LED_ID_Module;

 component p_connector

  port(
    Powerup_Done		  : in std_logic;
  signal_tap_clk_250mhz  : in std_logic;
  A_SEL                  : in std_logic_vector(3 downto 0);
  PIO_SYNC		           : in STD_LOGIC_VECTOR(150 DOWNTO 16); 
  CLK_IO                 : in std_logic;                      -- Clock for user_I/0
  DIOB_Config1           : in std_logic_vector(15 downto 0);
  AW_Output_Reg          : in t_IO_Reg_1_to_7_Array;          -- Output-Register to the Piggys
  UIO_SYNC		           : in STD_LOGIC_VECTOR(15 DOWNTO 0); 
  hp_la_o                : in std_logic_vector(15 downto 0);
  local_clk_is_running   : in std_logic; 
  clk_blink              : in std_logic;
  s_nLED_Sel             : in std_logic;   -- LED = Sel
  s_nLED_Dtack           : in std_logic;   -- LED = Dtack
  s_nLED_inR             : in std_logic;   -- LED = interrupt
  s_nLED_User1_o         : in std_logic;   -- LED3 = User 1
  s_nLED_User2_o         : in  std_logic;  -- LED2 = User 2
  s_nLED_User3_o         : in std_logic;   -- LED1 = User 3
  Tag_Sts                : in std_logic_vector(15 downto 0);  -- Tag-Status
  Timing_Pattern_LA      : in std_logic_vector(31 downto 0);  --  latched timing pattern from SCU_Bus for external user functions
  Tag_Aktiv              : in std_logic_vector( 7 downto 0);  -- Flag: Bit7 = Tag7 (active) --- Bit0 = Tag0 (active)       
  IOBP_LED_ID_Bus_o      : in std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
  IOBP_ID                : in t_id_array;                     -- IDs of the "Slave-Boards"
  IOBP_LED_En            : in std_logic;                      -- Output-Enable für LED- ID-Bus 
  IOBP_STR_rot_o         : in std_logic_vector(12 downto 1);  -- LED-Str Red for Slave 12-1
  IOBP_STR_gruen_o       : in std_logic_vector(12 downto 1);  -- LED-Str Green for Slave 12-1
  IOBP_STR_ID_o          : in std_logic_vector(12 downto 1);  -- ID-Str Green for Slave 12-1
  IOBP_Output            : in std_logic_vector(5 downto 0);   -- Outputs "Slave-Boards 1-12" 
  IOBP_Input             : in t_IOBP_array;                   -- Inputs "Slave-Boards 1-12"
  Deb66_out              : in std_logic_vector(65 downto 0);
  AW_IOBP_Input_Reg      : in t_IO_Reg_1_to_7_Array;  -- Input-Register of the Piggy's
  PIO_ENA_SLOT_1         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_2         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_3         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_4         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_5         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_6         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_7         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_8         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_9         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_10         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_11         : in std_logic_vector(5 downto 0);
  PIO_ENA_SLOT_12         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_1         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_3          : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_2          : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_4         : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_5       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_6        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_7       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_8        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_9      : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_10        : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_11       : in std_logic_vector(5 downto 0);
  PIO_OUT_SLOT_12       : in std_logic_vector(5 downto 0);

  --------------------------------------------------------------------------------------
  A_TA                   : out std_logic_vector(15 downto 0); -- test port a
  IOBP_LED_ID_Bus_i      : out  std_logic_vector(7 downto 0); 
  PIO_OUT                : out   STD_LOGIC_VECTOR(150 DOWNTO 16); 
  PIO_ENA                : out   STD_LOGIC_VECTOR(150 DOWNTO 16);   
  UIO_OUT                : out   STD_LOGIC_VECTOR(15 DOWNTO 0);
  UIO_ENA                : out   STD_LOGIC_VECTOR(15 DOWNTO 0);
  AW_ID                  : out   std_logic_vector(7 downto 0);
  AWIn_Deb_Time          : out   integer range 0 to 7;           -- Debounce-Time 2 High "AWIn_Deb_Time", value from DIOB-Config 1
  Min_AWIn_Deb_Time      : out   integer range 0 to 7;           -- Minimal Debounce-Time 2 High"Min_AWIn_Deb_Time" 
  Diob_Status1           : out   std_logic_vector(15 downto 0);
  DIOB_Status2           : out   std_logic_vector(15 downto 0);
  IOBP_Id_Reg1           : out std_logic_vector(15 downto 0);
  IOBP_Id_Reg2           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg3           : out std_logic_vector(15 downto 0);
  IOBP_Id_Reg4           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg5           : out  std_logic_vector(15 downto 0);
  IOBP_Id_Reg6           : out    std_logic_vector(15 downto 0);
  IOBP_Id_Reg7           : out   std_logic_vector(15 downto 0);
  IOBP_Id_Reg8           : out    std_logic_vector(15 downto 0);
  Deb66_in               : out     std_logic_vector(65 downto 0);
  Syn66                  : out        std_logic_vector(65 downto 0);
  AW_Input_Reg           : out   t_IO_Reg_1_to_8_Array;
  A_Tclk                 : out std_logic;
  extension_cid_group    : out  integer range 0 to 16#FFFF#;
  extension_cid_system   : out integer range 0 to 16#FFFF#;
  Max_AWOut_Reg_Nr       : out integer range 0 to 7;
  Max_AWIn_Reg_Nr        : out  integer range 0 to 7;
  Debounce_cnt           : out  integer range 0 to 16383;
  s_nLED_User1_i         : out std_logic;  -- LED3 = User 1
  s_nLED_User2_i         : out std_logic;  -- LED2 = User 2
  s_nLED_User3_i         : out std_logic;
  --IOBP_Output_Readback   : out t_IO_Reg_0_to_7_Array;
  --IOBP_Output_Readback   : out std_logic_vector(15 downto 0);
  Deb_Sync66             : out std_logic_vector(65 downto 0)
  

    );
    end component p_connector;

    component blm_24_9_9_9pll
    PORT
    (
      areset		: IN STD_LOGIC  := '0';
      inclk0		: IN STD_LOGIC  := '0';
      c0		: OUT STD_LOGIC ;
      c1		: OUT STD_LOGIC 
    );
    end component blm_24_9_9_9pll;
    


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


component  event_ctrl_el is

  port(
      clk : in std_logic;     -- chip-internal pulsed clk signal
      nRST : in std_logic;   -- general reset signal
      nEvent_Str : in std_logic; -- low active SCU_Bus runs timing cycle
      A_Address: in std_logic_vector(15 downto 0); -- SCU-Adress Bus
      A_Data: in std_logic_vector(15 downto 0); -- SCU-Data Bus (inout)
      BLM_event_key_Reg : in std_logic_vector(15 downto 0); --mask in_data
     BLM_event_ctrl_Reg : in std_logic_vector(15 downto 0); --config signals reg

     prepare_out: out std_logic_vector(11 downto 0);
     recover_out: out std_logic_vector(11 downto 0);
     reset_ctr_out: out std_logic;
     load_thr_out: out std_logic;
     reg_curr_data_set_by_ev: out std_logic_vector(11 downto 0);
    BLM_event_readout_Reg : out t_IO_Reg_0_to_2_Array-- virt acc readed address + value

   
  );
  end component event_ctrl_el;
  
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

  signal AWIn_Deb_Time:          integer range 0 to 7:= 0;           -- Debounce-Time 2 High "AWIn_Deb_Time", value from DIOB-Config 1
  signal Min_AWIn_Deb_Time:      integer range 0 to 7:= 0;           -- Minimal Debounce-Time 2 High"Min_AWIn_Deb_Time"

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
  signal AW_ID:           std_logic_vector(7 downto 0):=x"FF"; -- Application_ID


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
  signal AW_Input_Reg:            t_IO_Reg_1_to_8_Array;  -- Input-Register of the Piggys



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
  signal IOBP_Masken_Reg8:        std_logic_vector(15 downto 0);
  signal IOBP_msk_rd_active:      std_logic;
  signal IOBP_msk_Dtack:          std_logic;
  signal IOBP_msk_data_to_SCUB:   std_logic_vector(15 downto 0);
 --signal BLM_Status_Reg:    t_IO_Reg_0_to_25_Array ;
 signal BLM_Status_Reg:    t_IO_Reg_0_to_29_Array ;

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
  signal IOBP_in_data_to_SCUB:    t_IO_Reg_0_to_3_Array;
  signal IOBP_in_rd_active:       std_logic_vector(3 downto 0);
  signal IOBP_in_Dtack:            std_logic_vector(3 downto 0);
  signal IOBP_in_res_Dtack: std_logic;
  signal IOBP_Sel_LED:      t_led_array;    -- Sel-LED's der "Slave-Karten"
  signal IOBP_ID:           t_id_array;     -- IDs of the "Slave-Boards"
  signal IOBP_Aktiv_LED_i:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"
  signal IOBP_Aktiv_LED_o:  t_led_array;    -- Aktiv-LED's der "Slave-Karten"

  signal Syn66:        std_logic_vector(65 downto 0):= (OTHERS => '0');
 
  
signal Deg_Sync66:   std_logic_vector(65 downto 0);
  signal  Deb66_out:    std_logic_vector(65 downto 0);
signal  Deg66_in:     std_logic_vector(65 downto 0):= (OTHERS => '0');
signal  Deg66_out:    std_logic_vector(65 downto 0);

signal IOBP_STR_rot_o:    std_logic_vector(12 downto 1);  -- LED-Str Rot  für Slave 12-1
signal IOBP_STR_gruen_o:  std_logic_vector(12 downto 1);  -- LED-Str Grün für Slave 12-1
signal IOBP_STR_ID_o:     std_logic_vector(12 downto 1);  -- ID-Str Grün für Slave 12-1
signal IOBP_LED_ID_Bus_o: std_logic_vector(7 downto 0);   -- LED_ID_Bus_Out
signal IOBP_LED_ID_Bus_i: std_logic_vector(7 downto 0) :=    (OTHERS => '1');     -- Data_Output "Slave-Karte 1-12";   -- LED_ID_Bus_In
signal IOBP_LED_En:       std_logic;                      -- Output-Enable für LED- ID-Bus
signal Slave_Loop_cnt:      integer range 0 to 12;         -- 1-12   -- Loop-Counter

signal AW_IOBP_Input_Reg:  t_IO_Reg_1_to_7_Array;  -- Input-Register of the Piggy's
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
  signal  PIO_ENA_SYNC:          STD_LOGIC_VECTOR(150 DOWNTO 16):=(OTHERS => '0'); --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_OUT:               STD_LOGIC_VECTOR(150 DOWNTO 16):=(OTHERS => '0'); --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  PIO_OUT_SYNC:          STD_LOGIC_VECTOR(150 DOWNTO 16);  --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%

  signal  UIO_SYNC:              STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_SYNC1:             STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_ENA:               STD_LOGIC_VECTOR(15 DOWNTO 0):=(OTHERS => '0');     --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_ENA_SYNC:          STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_OUT:               STD_LOGIC_VECTOR(15 DOWNTO 0):=(OTHERS => '0');    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
  signal  UIO_OUT_SYNC:          STD_LOGIC_VECTOR(15 DOWNTO 0);    --  %%%%%  I/O-Synch und TriState-Steuerung   %%%%%
--%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



------
-- signal for test_signals and for gate_seq_clk signals:
signal blm_clk_25MHz, blm_clk_24_9MHz, blm_clk_100MHz,blm_clk_10MHz,blm_clk_1MHz,blm_clk_100kHz,blm_clk_10kHz,blm_clk_1kHz,blm_clk_9_9MHz,blm_clk_0_99MHz,blm_clk_99kHz,blm_clk_9_9kHz ,blm_clk_0_99kHz : std_logic;
---
signal BLM_tst_ck_sig: std_logic_vector (10 downto 0);

signal BLM_data_in: std_logic_vector(53 downto 0);
signal INTL_Output: std_logic_vector(5 downto 0);     -- Output "Slave-Karten 12"  
signal BLM_gate_in : std_logic_vector(11 downto 0);
signal BLM_Out :  std_logic_vector(5 downto 0);
signal BLM_deglitcher_data: std_logic_vector(65 downto 0);
signal BLM_deg_gate_in: std_logic_vector(11 downto 0);


--------------------------------------------------------------------------------------------------------------------------------------
--for thresholds

signal pos_thres_Reg:       t_BLM_th_Array; --128x 2 x 16 bit pos threshold
signal neg_thres_Reg:       t_BLM_th_Array; --128x 2 x 16 bit neg threshold
signal BLM_th_active:       std_logic_vector(63 downto 0);
signal BLM_th_Dtack:        std_logic_vector(63 downto 0);
signal BLM_th_data_to_SCUB: t_BLM_data_Array; 
signal BLM_th_res_Dtack: std_logic;
-------------------------------------------------
-----for BLM in_sel and gate mux enables
signal BLM_in_sel_Reg :             t_BLM_reg_Array; --128 x (4 bit for gate ena & 6 bit for up signal ena & 6 for down signal ena)
signal BLM_in_sel_rd_active :      std_logic_vector(15 downto 0);
signal BLM_in_sel_Dtack :       std_logic_vector(15 downto 0);
signal BLM_in_sel_data_to_SCUB: t_BLM_in_sel_Array;
signal BLM_in_sel_res_Dtack     : std_logic;
----------------------------------------------------------------

----------------------------------------------------------------
-----for hold times, gate enable and clock for gate sequence selection
signal BLM_wdog_hold_time_Reg :  std_logic_vector(15 downto 0);
signal BLM_gate_hold_time_Reg :  t_BLM_gate_hold_Time_Array;
signal BLM_gate_seq_prep_ck_sel_Reg: std_logic_vector(15 downto 0); 
signal BLM_gate_recover_Reg : std_logic_vector(15 downto 0);
--signal BLM_gate_seq_in_ena_Reg :  std_logic_vector(15 downto 0);
signal BLM_wd_reset_Reg: t_IO_Reg_0_to_3_Array;

signal BLM_wd_reset: std_logic_vector(53 downto 0);
signal BLM_ctrl_Reg:  std_logic_vector(15 downto 0); 
signal BLM_ctrl_rd_active:    std_logic_vector(2 downto 0);
signal BLM_ctrl_data_to_SCUB: t_IO_Reg_0_to_2_Array ;-- Data to SCU Bus Macro
signal BLM_ctrl_Dtack:        std_logic_vector(2 downto 0);                  -- Dtack to SCU Bus Macro
---------------------------------------
            -- Dtack to SCU Bus Macro
-----for BLM out_sel 
signal BLM_out_sel_Reg :       t_BLM_out_sel_reg_Array; 

signal BLM_out_sel_rd_active:  std_logic_vector(15 downto 0);
signal BLM_out_sel_Dtack: std_logic_vector(15 downto 0);
signal BLM_out_sel_data_to_SCUB: t_IO_Reg_0_to_15_Array;
signal BLM_out_sel_res_Dtack     : std_logic;

signal IOBP_LED_sm_nr: std_logic_vector(3 downto 0);
---
----------------------------------------------------------
          -- for event and virtual accelerator 
signal BLM_event_readout_Reg :t_IO_Reg_0_to_2_Array;
signal BLM_virt_acc_rd_Reg: std_logic_vector(15 downto 0);
signal BLM_event_v_acc_readout_rd_active: std_logic;
signal BLM_event_v_acc_readout_Dtack: std_logic;
signal BLM_event_v_acc_readout_data_to_SCUB: std_logic_vector(15 downto 0);


signal BLM_event_key_Reg:std_logic_vector(15 downto 0);
signal BLM_event_ctrl_Reg: std_logic_vector(15 downto 0);
signal BLM_mem_cmd_Reg: std_logic_vector(15 downto 0);
signal BLM_new_dataset_Reg: std_logic_vector(15 downto 0);

signal BLM_event_v_acc_ctrl_rd_active: std_logic;
signal BLM_event_v_acc_ctrl_Dtack: std_logic;
signal BLM_event_v_acc_ctrl_data_to_SCUB: std_logic_vector(15 downto 0);
signal ev_cmd_prepare : std_logic_vector(11 downto 0);
signal ev_cmd_recover:  std_logic_vector(11 downto 0);

signal ev_cmd_reset_ctr: std_logic;
signal ev_cmd_load_thr: std_logic;
signal loaded_data_set: std_logic_vector(11 downto 0);
signal counter_readout_Reg: t_BLM_th_Array  ;
signal counter_readout_active: std_logic_vector(31 downto 0);
signal counter_readout_Dtack: std_logic_vector(31 downto 0);
signal counter_readout_data_to_SCUB: t_IO_Reg_0_to_31_Array;
signal counter_readout_res_Dtack: std_logic;

constant ZERO_th: std_logic_vector(BLM_th_Dtack'range) := (others => '0');
constant ZERO_in_sel: std_logic_vector(BLM_in_sel_Dtack'range) := (others => '0');
constant ZERO_status_sel: std_logic_vector(IOBP_in_Dtack'range) := (others => '0');
constant ZERO_out_sel: std_logic_vector(BLM_out_sel_Dtack'range) := (others => '0');
constant ZERO_cnt_readout_sel: std_logic_vector(counter_readout_Dtack'range) := (others => '0');
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
      Reg_IO8            =>  IOBP_Masken_Reg8,
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
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
------------------- BLM Registers -------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
BLM_status_registers_0_23:for i in 0 to 2 generate 


    BLM_Status_READBACK_Reg: in_reg
    generic map(
          Base_addr =>  c_Status_READBACK_Base_Addr +8*i
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
          Reg_In1            =>  BLM_Status_Reg(i*8),
          Reg_In2            =>  BLM_Status_Reg(i*8+1),
          Reg_In3            =>  BLM_Status_Reg(i*8+2),
          Reg_In4            =>  BLM_Status_Reg(i*8+3),
          Reg_In5            =>  BLM_Status_Reg(i*8+4),
          Reg_In6            =>  BLM_Status_Reg(i*8+5),
          Reg_In7            =>  BLM_Status_Reg(i*8+6),
          Reg_In8            =>  BLM_Status_Reg(i*8+7),

    --
          Reg_rd_active      =>  IOBP_in_rd_active(i),
          Dtack_to_SCUB      =>  IOBP_in_Dtack(i),
          Data_to_SCUB       =>  IOBP_in_data_to_SCUB(i)
        );
    end generate BLM_status_registers_0_23;

     


        BLM_Status_READBACK_Reg_24_29: in_reg
        generic map(
              Base_addr =>  c_Status_READBACK_Base_Addr +24
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
              Reg_In1            =>  BLM_Status_Reg(24),
              Reg_In2            =>  BLM_Status_Reg(24+1),
              Reg_In3            =>  BLM_Status_Reg(24+2),
              Reg_In4            =>  BLM_Status_Reg(24+3),
              Reg_In5            =>  BLM_Status_Reg(24+4),
              Reg_In6            =>  BLM_Status_Reg(24+5),
              Reg_In7            =>  (others =>'0'),
              Reg_In8            =>  (others =>'0'),
    
        --
              Reg_rd_active      =>  IOBP_in_rd_active(3),
              Dtack_to_SCUB      =>  IOBP_in_Dtack(3),
              Data_to_SCUB       =>  IOBP_in_data_to_SCUB(3)
            );
     
    

threshold_registers: for i in 0 to 63 generate

BLM_thr_Reg: io_reg
        generic map(
              Base_addr =>  c_BLM_thres_Base_Addr + 8*i
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

              Reg_IO1            =>  pos_thres_Reg(2*i)(15 downto 0),
              Reg_IO2            =>  pos_thres_Reg(2*i)(31 downto 16),
              Reg_IO3            =>  neg_thres_Reg(2*i)(15 downto 0),
              Reg_IO4            =>  neg_thres_Reg(2*i)(31 downto 16),
              Reg_IO5            =>  pos_thres_Reg(2*i+1)(15 downto 0),
              Reg_IO6            =>  pos_thres_Reg(2*i+1)(31 downto 16),
              Reg_IO7            =>  neg_thres_Reg(2*i+1)(15 downto 0),
              Reg_IO8            =>  neg_thres_Reg(2*i+1)(31 downto 16),
        --
              Reg_rd_active      =>  BLM_th_active (i),
              Dtack_to_SCUB      =>  BLM_th_Dtack(i),
              Data_to_SCUB       =>  BLM_th_data_to_SCUB(i)
            );
  end generate threshold_registers;

BLM_in_sel_registers: for i in 0 to 15 generate 

BLM_in_sl_Reg: io_reg
generic map(
      Base_addr =>  c_BLM_in_sel_Base_Addr + 8*i
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
      Reg_IO1            =>  BLM_in_sel_Reg(i*8),
      Reg_IO2            =>  BLM_in_sel_Reg(i*8+1),
      Reg_IO3            =>  BLM_in_sel_Reg(i*8+2),
      Reg_IO4            =>  BLM_in_sel_Reg(i*8+3),
      Reg_IO5            =>  BLM_in_sel_Reg(i*8+4),
      Reg_IO6            =>  BLM_in_sel_Reg(i*8+5),
      Reg_IO7            =>  BLM_in_sel_Reg(i*8+6),
      Reg_IO8            =>  BLM_in_sel_Reg(i*8+7),
--
      Reg_rd_active      =>  BLM_in_sel_rd_active(i),
      Dtack_to_SCUB      =>  BLM_in_sel_Dtack(i),
      Data_to_SCUB       =>  BLM_in_sel_data_to_SCUB(i)
    );
    end generate BLM_in_sel_registers;


BLM_ctrl_Reg_1st_block: io_reg
  generic map(
        Base_addr =>  c_BLM_ctrl_Base_Addr
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
 
  Reg_IO1            =>  BLM_wdog_hold_time_Reg,     -- the same for all
  Reg_IO2            =>  BLM_gate_seq_prep_ck_sel_Reg,    -- bit 15 not used
                                                          -- bit 14-3 for gate_prepare signals
                                                          -- bit 2-0 for the clock gate sel, the same for all

  Reg_IO3            =>  BLM_ctrl_Reg,               --  bit 0 = counter RESET, 
                                    
                                                     --  bit 11..0 Direct Gate-usage, one bit for each gate signal input, 
                                                     --  bit 14 reset from gate: when 1 the corresponding gate signal selecting the counter enable
                                                     --         is used instead to reset it.
                                                     -- bit 15 not used
  Reg_IO4            =>  BLM_gate_recover_Reg, -- bit 15 -12 not used
                                                -- bit 11-0 for gate_recover signals
  Reg_IO5            =>   BLM_gate_hold_time_Reg(0), 
  Reg_IO6            =>   BLM_gate_hold_time_Reg(1),
  Reg_IO7            =>   BLM_gate_hold_time_Reg(2),
  Reg_IO8            =>   BLM_gate_hold_time_Reg(3),

  Reg_rd_active      =>   BLM_ctrl_rd_active(0),
  Dtack_to_SCUB      =>   BLM_ctrl_Dtack(0),
  Data_to_SCUB       =>   BLM_ctrl_data_to_SCUB(0)
      );

  BLM_ctrl_Reg_2nd_block: io_reg
      generic map(
            Base_addr =>  c_BLM_ctrl_Base_Addr + 8
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
     
      Reg_IO1            =>   BLM_gate_hold_time_Reg(4),
      Reg_IO2            =>   BLM_gate_hold_time_Reg(5),
      Reg_IO3            =>   BLM_gate_hold_time_Reg(6),
      Reg_IO4            =>   BLM_gate_hold_time_Reg(7),
      Reg_IO5            =>   BLM_gate_hold_time_Reg(8),
      Reg_IO6            =>   BLM_gate_hold_time_Reg(9),
      Reg_IO7            =>   BLM_gate_hold_time_Reg(10),
      Reg_IO8            =>   BLM_gate_hold_time_Reg(11),

      Reg_rd_active      =>   BLM_ctrl_rd_active(1),
      Dtack_to_SCUB      =>   BLM_ctrl_Dtack(1),
      Data_to_SCUB       =>   BLM_ctrl_data_to_SCUB(1)
          );

      BLM_ctrl_Reg_3rdd_block: io_reg
          generic map(
                Base_addr =>  c_BLM_ctrl_Base_Addr + 16
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
         
          Reg_IO1            =>   BLM_wd_reset_Reg(0),
          Reg_IO2            =>   BLM_wd_reset_Reg(1),
          Reg_IO3            =>   BLM_wd_reset_Reg(2),
          Reg_IO4            =>   BLM_wd_reset_Reg(3),
          Reg_IO5            =>   open,
          Reg_IO6            =>   open,
          Reg_IO7            =>   open,
          Reg_IO8            =>   open,
    
          Reg_rd_active      =>   BLM_ctrl_rd_active(2),
          Dtack_to_SCUB      =>   BLM_ctrl_Dtack(2),
          Data_to_SCUB       =>   BLM_ctrl_data_to_SCUB(2)
              );
          
  BLM_out_sel_registers: for i in 0 to 14 generate 

      BLM_o_sel_Reg: io_reg
      generic map(
            Base_addr =>  c_BLM_out_sel_Base_Addr + 8*i
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
            Reg_IO1            =>  BLM_out_sel_Reg(i*8),
            Reg_IO2            =>  BLM_out_sel_Reg(i*8+1),
            Reg_IO3            =>  BLM_out_sel_Reg(i*8+2),
            Reg_IO4            =>  BLM_out_sel_Reg(i*8+3),
            Reg_IO5            =>  BLM_out_sel_Reg(i*8+4),
            Reg_IO6            =>  BLM_out_sel_Reg(i*8+5),
            Reg_IO7            =>  BLM_out_sel_Reg(i*8+6),
            Reg_IO8            =>  BLM_out_sel_Reg(i*8+7),
      --
            Reg_rd_active      =>  BLM_out_sel_rd_active(i),
            Dtack_to_SCUB      =>  BLM_out_sel_Dtack(i),
            Data_to_SCUB       =>  BLM_out_sel_data_to_SCUB(i)
          );
          end generate BLM_out_sel_registers;



          BLM_o_sel_Reg_120_126: io_reg
          generic map(
                Base_addr =>  c_BLM_out_sel_Base_Addr + 120
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
                Reg_IO1            =>  BLM_out_sel_Reg(120),
                Reg_IO2            =>  BLM_out_sel_Reg(121),
                Reg_IO3            =>  BLM_out_sel_Reg(122),
                Reg_IO4            =>  BLM_out_sel_Reg(123),
                Reg_IO5            =>  BLM_out_sel_Reg(124),
                Reg_IO6            =>  BLM_out_sel_Reg(125),
                Reg_IO7            =>   open,
                Reg_IO8            =>  open,
          --
                Reg_rd_active      =>  BLM_out_sel_rd_active(15),
                Dtack_to_SCUB      =>  BLM_out_sel_Dtack(15),
                Data_to_SCUB       =>  BLM_out_sel_data_to_SCUB(15));


 -------------------------------------------------------------------- 
BLM_event_v_acc_readout_Reg: in_reg
generic map(
      Base_addr => c_BLM_event_readout_Base_Addr
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
      Reg_In1            =>  BLM_event_readout_Reg(0),
      Reg_In2            =>  BLM_event_readout_Reg(1),
      Reg_In3            =>  BLM_event_readout_Reg(2),
      Reg_In4            =>  BLM_virt_acc_rd_Reg,
      Reg_In5            => (others =>'0'),
      Reg_In6            =>  (others =>'0'),
      Reg_In7            =>  (others =>'0'),
      Reg_In8            =>  (others =>'0'),
--
      Reg_rd_active      =>  BLM_event_v_acc_readout_rd_active,
      Dtack_to_SCUB      =>  BLM_event_v_acc_readout_Dtack,
      Data_to_SCUB       =>  BLM_event_v_acc_readout_data_to_SCUB
    );


BLM_event_v_acc_control_Reg: io_reg
generic map(
      Base_addr =>  c_BLM_event_ctrl_Base_Addr
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
   
      Reg_IO1            =>   BLM_event_key_Reg,
      Reg_IO2            =>   BLM_event_ctrl_Reg,
      Reg_IO3            =>   BLM_mem_cmd_Reg,
      Reg_IO4            =>   BLM_new_dataset_Reg,
      Reg_IO5            =>   open,
      Reg_IO6            =>   open,
      Reg_IO7            =>   open,
      Reg_IO8            =>   open,

      Reg_rd_active      =>   BLM_event_v_acc_ctrl_rd_active,
      Dtack_to_SCUB      =>   BLM_event_v_acc_ctrl_Dtack,
      Data_to_SCUB       =>   BLM_event_v_acc_ctrl_data_to_SCUB
        );


  counters_readout_registers:  for i in 0 to 31 generate

          counter_readout_Reg_module: in_reg
                  generic map(
                        Base_addr =>  c_BLM_counter_readout_Base_Addr + 8*i
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
          
                        Reg_In1            =>  counter_readout_Reg(4*i)(15 downto 0),
                        Reg_In2            =>  counter_readout_Reg(4*i)(31 downto 16),
                        Reg_In3            =>  counter_readout_Reg(4*i+1)(15 downto 0),
                        Reg_In4            =>  counter_readout_Reg(4*i+1)(31 downto 16),
                        Reg_In5            =>  counter_readout_Reg(4*i+2)(15 downto 0),
                        Reg_In6            =>  counter_readout_Reg(4*i+2)(31 downto 16),
                        Reg_In7            =>  counter_readout_Reg(4*i+3)(15 downto 0),
                        Reg_In8            =>  counter_readout_Reg(4*i+3)(31 downto 16),
                  --
                        Reg_rd_active      =>   counter_readout_active (i),
                        Dtack_to_SCUB      =>   counter_readout_Dtack(i),
                        Data_to_SCUB       =>    counter_readout_data_to_SCUB(i)
                      );
            end generate counters_readout_registers;

      --  RAM_threshold_registers: for i in 0 to 63 generate

        --  BLM_RAM_thr_Reg: io_reg
        --          generic map(
         --               Base_addr =>  c_BLM_mem_thres_Base_Addr + 8*i
          --              )
          --        port map  (
          --              Adr_from_SCUB_LA   =>  ADR_from_SCUB_LA,
          --              Data_from_SCUB_LA  =>  Data_from_SCUB_LA,
          --              Ext_Adr_Val        =>  Ext_Adr_Val,
          --              Ext_Rd_active      =>  Ext_Rd_active,
          --              Ext_Rd_fin         =>  Ext_Rd_fin,
          --              Ext_Wr_active      =>  Ext_Wr_active,
          --              Ext_Wr_fin         =>  SCU_Ext_Wr_fin,
          --              clk                =>  clk_sys,
          --              nReset             =>  rstn_sys,
          
          --              Reg_IO1            =>  mem_pos_thres_Reg(2*i)(15 downto 0),
          --              Reg_IO2            =>  mem_pos_thres_Reg(2*i)(31 downto 16),
           --             Reg_IO3            =>  mem_neg_thres_Reg(2*i)(15 downto 0),
            --            Reg_IO4            =>  mem_neg_thres_Reg(2*i)(31 downto 16),
          --              Reg_IO5            =>  mem_pos_thres_Reg(2*i+1)(15 downto 0),
           --             Reg_IO6            =>  mem_pos_thres_Reg(2*i+1)(31 downto 16),
           --             Reg_IO7            =>  mem_neg_thres_Reg(2*i+1)(15 downto 0),
           --             Reg_IO8            =>  mem_neg_thres_Reg(2*i+1)(31 downto 16),
                  --
           --             Reg_rd_active      =>  BLM_mem th_active (i),
           --             Dtack_to_SCUB      =>  BLM_mem_th_Dtack(i),
           --             Data_to_SCUB       =>  BLM__memth_data_to_SCUB(i)
           --           );
           -- end generate threshold_registers;
          

      Event_Timestamping_module:  event_ctrl_el 

          port map(
              clk => clk_sys,
              nRST  =>  rstn_sys,
              nEvent_Str =>  A_nEvent_Str,
              A_Address=> A_A,
              A_Data => A_D,
              BLM_event_key_Reg    => BLM_event_key_Reg,
              prepare_out => ev_cmd_prepare,
              recover_out => ev_cmd_recover,
              reset_ctr_out => ev_cmd_reset_ctr,
              load_thr_out => ev_cmd_load_thr,
              reg_curr_data_set_by_ev => loaded_data_set,
              BLM_event_ctrl_Reg    => BLM_event_ctrl_Reg,


              BLM_event_readout_Reg   => BLM_event_readout_Reg
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
   intr_in                 =>"00" & tmr_irq & "00000000000"& clk_switch_intr,   
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

rd_port_mux:  process ( clk_switch_rd_active,              clk_switch_rd_data,
                        wb_scu_rd_active,                  wb_scu_data_to_SCUB,
                        Tag_Ctrl1_rd_active,               Tag_Ctrl1_data_to_SCUB,
                        Conf_Sts1_rd_active,               Conf_Sts1_data_to_SCUB,
                        tmr_rd_active,                     tmr_data_to_SCUB,
                        AW_Port1_rd_active,                AW_Port1_data_to_SCUB,
                        IOBP_msk_rd_active,                IOBP_msk_data_to_SCUB,
                        IOBP_id_rd_active,                 IOBP_id_data_to_SCUB,
                        IOBP_in_rd_active,                 IOBP_in_data_to_SCUB,
                        BLM_ctrl_rd_active,                BLM_ctrl_data_to_SCUB,
                        BLM_th_active,                     BLM_th_data_to_SCUB,
                        BLM_in_sel_rd_active,              BLM_in_sel_data_to_SCUB,
                        BLM_event_v_acc_readout_rd_active, BLM_event_v_acc_readout_data_to_SCUB,
                        BLM_event_v_acc_ctrl_rd_active,    BLM_event_v_acc_ctrl_data_to_SCUB,
                        counter_readout_active,            counter_readout_data_to_SCUB
                      )


  variable sel: unsigned(12 downto 0);
  variable sel_th: unsigned(63 downto 0);
  variable sel_in_sel: unsigned(15 downto 0);
  variable sel_st: unsigned(3 downto 0);
  variable sel_out_sel: unsigned(15 downto 0); 
  variable sel_cnt_readout_sel: unsigned(31 downto 0); 
  begin


    sel_in_sel := unsigned(BLM_in_sel_rd_active);
    sel_th:= unsigned (BLM_th_active);
    sel_st:= unsigned(IOBP_in_rd_active);
    sel_out_sel := unsigned(BLM_out_sel_rd_active);
    sel_cnt_readout_sel := unsigned(counter_readout_active);
    
    sel:=   BLM_event_v_acc_readout_rd_active & BLM_event_v_acc_ctrl_rd_active& 
            BLM_ctrl_rd_active(2) & BLM_ctrl_rd_active(1)&BLM_ctrl_rd_active(0)   & 
            AW_Port1_rd_active & tmr_rd_active &  wb_scu_rd_active & clk_switch_rd_active &
            Conf_Sts1_rd_active & Tag_Ctrl1_rd_active & IOBP_msk_rd_active & IOBP_id_rd_active ;
  
if to_integer(sel(12 downto 0))>0 then
  case sel(12 downto 0) IS
      when "1000000000000" => Data_to_SCUB <=  BLM_event_v_acc_readout_data_to_SCUB; 
      when "0100000000000" => Data_to_SCUB <=  BLM_event_v_acc_ctrl_data_to_SCUB; 
      when "0010000000000" => Data_to_SCUB <= BLM_ctrl_data_to_SCUB(2);
      when "0001000000000" => Data_to_SCUB <= BLM_ctrl_data_to_SCUB(1);
      when "0000100000000" => Data_to_SCUB <= BLM_ctrl_data_to_SCUB(0);  
      when "0000010000000" => Data_to_SCUB <= AW_Port1_data_to_SCUB;  
      when "0000001000000" => Data_to_SCUB <= tmr_data_to_SCUB;
      when "0000000100000" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when "0000000010000" => Data_to_SCUB <= clk_switch_rd_data;
      when "0000000001000" => Data_to_SCUB <= Conf_Sts1_data_to_SCUB;
      when "0000000000100" => Data_to_SCUB <= Tag_Ctrl1_data_to_SCUB;
      when "0000000000010" => Data_to_SCUB <= IOBP_msk_data_to_SCUB;
      when "0000000000001" => Data_to_SCUB <= IOBP_id_data_to_SCUB;

      when others      => Data_to_SCUB <= (others => '0');
    end case;
else 
    if to_integer(sel_th)>0 then
        for i in 0 to 63 loop
          if sel_th(i) = '1' then 
            Data_to_SCUB <= BLM_th_data_to_SCUB(i);
          end if;
        end loop;
        else 
        if to_integer(sel_in_sel) > 0 then  
           for i in 0 to 15 loop
             if sel_in_sel(i) = '1' then 
                Data_to_SCUB <= BLM_in_sel_data_to_SCUB(i);
             end if;
           end loop;
           else
           if to_integer(sel_st) > 0 then  
           for i in 0 to 3 loop
             if sel_st(i) = '1' then 
                Data_to_SCUB <= IOBP_in_data_to_SCUB(i);
             end if;
           end loop;
           else
           if to_integer(sel_out_sel) > 0 then  
           for i in 0 to 15 loop
             if sel_out_sel(i) = '1' then 
                Data_to_SCUB <=  BLM_out_sel_data_to_SCUB(i);
             end if;
           end loop;
           else
            if to_integer(sel_cnt_readout_sel) > 0 then  
              for i in 0 to 31 loop
                if sel_cnt_readout_sel(i) = '1' then 
                   Data_to_SCUB <=  counter_readout_data_to_SCUB(i);
                end if;
              end loop;
           
        else 
           Data_to_SCUB <= (others =>'0');
        end if;
    end if;
    end if;
  end if;
  end if;  
end if;        
  end process rd_port_mux;

  ------------------------------------------------------
  -----Dtack_to_SCUB for gate/wd ena registers
  new_Dtack_sproc: process(BLM_th_Dtack,BLM_in_sel_Dtack)
  begin
  
------------------------------------------------------

  ------------------------------------------------------
  -----Dtack_to_SCUB for threshold registers
 
    if (BLM_th_Dtack = ZERO_th) then BLM_th_res_Dtack <='0';
    else BLM_th_res_Dtack <='1';
    end if;
  ------------------------------------------------------
  -----Dtack_to_SCUB for input and gate ena registers
 
  if (BLM_in_sel_Dtack =ZERO_in_sel) then BLM_in_sel_res_Dtack <='0';
  else BLM_in_sel_res_Dtack <='1';
  end if;
   
 ------------------------------------------------------
-----Dtack_to_SCUB for out_sel registers
 
if (BLM_out_sel_Dtack =ZERO_out_sel) then BLM_out_sel_res_Dtack <='0';
else BLM_out_sel_res_Dtack <='1';
end if;
   

-----Dtack_to_SCUB for status registers
 
if (IOBP_in_Dtack =ZERO_status_sel) then IOBP_in_res_Dtack <='0';
else IOBP_in_res_Dtack <='1';
end if;
 
 
if (counter_readout_Dtack =ZERO_cnt_readout_sel) then counter_readout_res_Dtack <='0';
else counter_readout_res_Dtack <='1';
end if;
   
-------------- Dtack_to_SCUB -----------------------------

    Dtack_to_SCUB <= ( tmr_dtack  or AW_Port1_Dtack   or wb_scu_dtack  or clk_switch_dtack  or Conf_Sts1_Dtack  or Tag_Ctrl1_Dtack  or
                         IOBP_msk_Dtack   or IOBP_id_Dtack    or    IOBP_in_res_Dtack  or 
                         BLM_ctrl_Dtack(2) or BLM_ctrl_Dtack(1) or  BLM_ctrl_Dtack(0) or BLM_th_res_Dtack or BLM_in_sel_res_Dtack or BLM_out_sel_res_Dtack or 
                         BLM_event_v_acc_readout_Dtack or BLM_event_v_acc_ctrl_Dtack or  counter_readout_res_Dtack);

    A_nDtack <= NOT(SCUB_Dtack);
    A_nSRQ   <= NOT(SCUB_SRQ);
  end process;

--  +============================================================================================================================+
--  |            §§§                        Anwender-IO: IOBP (INLB12S1)  -- FG902_050                                           |
--  +============================================================================================================================+
--Deb66:  for I in 0 to 65 generate
--DB_I:  diob_debounce
--GENERIC MAP (DB_Tst_Cnt   => 3,
--             Test         => 0)             --
--          port map(DB_Cnt => Debounce_cnt,     -- Debounce-Zeit in Clock's
--                   DB_in  => Deb66_in(I),   -- Signal-Input
--                   Reset  => not rstn_sys,  -- Powerup-Reset
--                   clk    => clk_sys,       -- Sys-Clock
--                   DB_Out => Deb66_out(I)); -- Debounce-Signal-Out
--end generate Deb66;

Deg_in_signals:  for I in 0 to 53 generate
DB_I: deglitcher 
  generic	map (nr_stages => 1
)
port map 
(
    clock => clk_sys,        
   reset=> not rstn_sys,           
  degl_in => Deg66_in(I), 
  degl_out => Deg66_out(I)
);
  end generate Deg_in_signals;

--Deg_gate_signals: for I in 54 to 65 generate
--DB_I: gate_deglitcher 
--  generic	map (nr_stages => 10
--)
--port map 
--(
  --  clock => clk_sys,        
  -- reset=> not rstn_sys,           
  --degl_in => Deg66_in(I), 
  --degl_out => Deg66_out(I)
--);
  -- end generate Deg_gate_signals;
--  Deg_gate_signals: for I in 0 to 11 generate
--DB_GI: gate_deglitcher 
--  generic	map (nr_stages => 10
--)
--port map 
--(
 --   clock => clk_sys,        
--   reset=> not rstn_sys,           
--  degl_in => Deg66_in(I+54), 
--  degl_out => Deg66_out(I+54)
--);
--  end generate Deg_gate_signals;

  Deb_GI:  for I in 0 to 11 generate
 DB_GI:  diob_debounce
GENERIC MAP (DB_Tst_Cnt   => 20, --3,
            Test         => 0)             --
          port map(DB_Cnt => Debounce_cnt,     -- Debounce-Zeit in Clock's
                   DB_in  => Deg66_in(I+54),   -- Signal-Input
                  Reset  => not rstn_sys,  -- Powerup-Reset
                  clk    => clk_sys,       -- Sys-Clock
                  DB_Out => Deg66_out(I+54)); -- Debounce-Signal-Out
end generate Deb_GI;
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
---Beam Loss Monitor new version




BLM_data_in <= AW_IOBP_Input_Reg(5)(5 downto 0) & AW_IOBP_Input_Reg(4)(11 downto 6) & AW_IOBP_Input_Reg(4)(5 downto 0) &
               AW_IOBP_Input_Reg(3)(11 downto 6) & AW_IOBP_Input_Reg(3)(5 downto 0) & AW_IOBP_Input_Reg(2)(11 downto 6) & 
               AW_IOBP_Input_Reg(2)(5 downto 0) &  AW_IOBP_Input_Reg(1)(11 downto 6) & AW_IOBP_Input_Reg(1)(5 downto 0); 

BLM_gate_in <= AW_IOBP_Input_Reg(5)(11 downto 6) & AW_IOBP_Input_Reg(6)(5 downto 0);
---
BLM_tst_ck_sig <= blm_clk_100MHz & blm_clk_25MHz & blm_clk_24_9MHz & blm_clk_10MHz & blm_clk_1MHz & blm_clk_100kHz & blm_clk_10kHz & blm_clk_1kHz & blm_clk_9_9MHz & blm_clk_0_99MHz & blm_clk_99kHz;-- & blm_clk_9_9kHz& blm_clk_0_99kHz;
BLM_wd_reset <= BLM_wd_reset_Reg(3)(5 downto 0)&BLM_wd_reset_Reg(2) & BLM_wd_reset_Reg(1) &BLM_wd_reset_Reg(0);

--for tests
BLM_deglitcher_data <= Deg66_out;

BLM_Module : Beam_Loss_check 

  generic map (

  WIDTH => 32     -- Counter width
     
)

  port map(
    clk_sys        => clk_sys,    -- Clock
    rstn_sys       => rstn_sys,     -- Reset

 -- IN BLM 
  BLM_data_in      => BLM_data_in,--BLM_deglitcher_data(53 downto 0), --BLM_data_in,
  BLM_gate_in      => BLM_gate_in,
  BLM_tst_ck_sig   => BLM_tst_ck_sig,
  IOBP_LED_nr      => IOBP_LED_sm_nr,
  --IN registers
  pos_threshold            => pos_thres_Reg,
  neg_threshold            => neg_thres_Reg,
  BLM_wdog_hold_time_Reg   => BLM_wdog_hold_time_Reg,
  BLM_wd_reset => BLM_wd_reset,
  BLM_gate_hold_time_Reg   => BLM_gate_hold_time_Reg,
  BLM_ctrl_Reg             => BLM_ctrl_Reg,
  BLM_gate_seq_prep_ck_sel_Reg  => BLM_gate_seq_prep_ck_sel_Reg,
  BLM_gate_recover_Reg => BLM_gate_recover_Reg,
  --BLM_gate_seq_in_ena_Reg  => BLM_gate_seq_in_ena_Reg,
  BLM_in_sel_Reg           => BLM_in_sel_Reg,
  BLM_out_sel_reg          => BLM_out_sel_Reg,

-- event_ctrl_sig


ev_counter_reset=> ev_cmd_reset_ctr,
ev_thr_load => ev_cmd_load_thr,
virt_acc => loaded_data_set,
ev_prepare_reg =>ev_cmd_prepare,
ev_recover_reg =>ev_cmd_recover,




  -- OUT register
  BLM_status_Reg           => BLM_status_Reg,
counter_readout_reg => counter_readout_Reg,
    -- OUT BLM
  BLM_Out                 => BLM_out
);



front_board_id_Module: front_board_id 
port map   
( clk               => clk_sys,
nReset            => rstn_sys,
Deb_Sync          => Deg_Sync66,
Deb_out           => Deg66_out,
IOBP_Masken_Reg1  => IOBP_Masken_Reg1,
IOBP_Masken_Reg2  => IOBP_Masken_Reg2,
IOBP_Masken_Reg3  => IOBP_Masken_Reg3,
IOBP_Masken_Reg4  => IOBP_Masken_Reg4,
IOBP_Masken_Reg5  => IOBP_Masken_Reg5,
IOBP_Masken_Reg6  => IOBP_Masken_Reg6,
PIO_SYNC          => PIO_SYNC(142 DOWNTO 20),
IOBP_ID           => IOBP_ID,
INTL_Output       =>  BLM_out, --INTL_Output,
AW_Output_Reg     => AW_Output_Reg(6)(11 downto  6),
--nBLM_out_ena        => BLM_ctrl_Reg(1), -- 
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
             -- Ena_Every_250ns   => Ena_Every_250ns,
              Ena_Every_500ns => Ena_Every_500ns,

              AW_ID             => AW_ID,
              IOBP_LED_ID_Bus_i => IOBP_LED_ID_Bus_i,
              IOBP_Aktiv_LED_o  => IOBP_Aktiv_LED_o,
              IOBP_Sel_LED      => IOBP_Sel_LED,
              IOBP_LED_En       => IOBP_LED_En,
              IOBP_STR_rot_o    => IOBP_STR_rot_o,
              IOBP_STR_gruen_o  => IOBP_STR_gruen_o,
              IOBP_STR_ID_o     => IOBP_STR_ID_o,
              IOBP_LED_ID_Bus_o => IOBP_LED_ID_Bus_o,
              IOBP_ID           => IOBP_ID,
              IOBP_LED_state_nr =>  IOBP_LED_sm_nr
              
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

    for i in 1 to 7 loop
      SCU_AW_Input_Reg(i)  <= AW_Input_Reg(i); -- Input's bleiben unverändert
     end loop; 
     

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
--  #####               PROCESS: IO Signals assignment via JPIO1(150pol.) ==> "Piggy-Type"                    #####
--  #####                                                                                                                     #####
--  ###############################################################################################################################
--  ###############################################################################################################################

AW_B12s1_connection: p_connector 

  port map
  (
    Powerup_Done		       => Powerup_Done,
    signal_tap_clk_250mhz  => signal_tap_clk_250mhz,
    A_SEL                  => A_SEL,
    PIO_SYNC		           => PIO_SYNC,
    CLK_IO                 => CLK_IO,
    DIOB_Config1           => DIOB_Config1, 
    AW_Output_Reg          => AW_Output_Reg,
    UIO_SYNC		           => UIO_SYNC,
    hp_la_o                => hp_la_o,
    local_clk_is_running   => local_clk_is_running,
    clk_blink              => clk_blink,
    s_nLED_Sel             => s_nLED_Sel,
    s_nLED_Dtack           => s_nLED_Dtack,
    s_nLED_inR             => s_nLED_inR,
    s_nLED_User1_o         => s_nLED_User1_o,
    s_nLED_User2_o         => s_nLED_User2_o,
    s_nLED_User3_o         => s_nLED_User3_o,
    Tag_Sts                => Tag_Sts,
    Timing_Pattern_LA      => Timing_Pattern_LA,
    Tag_Aktiv              => Tag_Aktiv,    
    IOBP_LED_ID_Bus_o      => IOBP_LED_ID_Bus_o,
    IOBP_ID                => IOBP_ID,
    IOBP_LED_En            => IOBP_LED_En,
    IOBP_STR_rot_o         => IOBP_STR_rot_o,
    IOBP_STR_gruen_o       => IOBP_STR_gruen_o,
    IOBP_STR_ID_o          => IOBP_STR_ID_o,
    IOBP_Output            => IOBP_Output,
    IOBP_Input             => IOBP_Input,
    Deb66_out              => Deg66_out,
    AW_IOBP_Input_Reg      => AW_IOBP_Input_Reg,
    A_TA                   => A_TA,
    PIO_ENA_SLOT_1         =>  PIO_ENA_SLOT_1,
    PIO_ENA_SLOT_2          => PIO_ENA_SLOT_2,
    PIO_ENA_SLOT_3        => PIO_ENA_SLOT_3,
    PIO_ENA_SLOT_4         => PIO_ENA_SLOT_4,
    PIO_ENA_SLOT_5        => PIO_ENA_SLOT_5,
    PIO_ENA_SLOT_6         => PIO_ENA_SLOT_6,
    PIO_ENA_SLOT_7        => PIO_ENA_SLOT_7,
    PIO_ENA_SLOT_8         => PIO_ENA_SLOT_8,
    PIO_ENA_SLOT_9      => PIO_ENA_SLOT_9,
    PIO_ENA_SLOT_10         => PIO_ENA_SLOT_10,
    PIO_ENA_SLOT_11         => PIO_ENA_SLOT_11,
    PIO_ENA_SLOT_12         => PIO_ENA_SLOT_12,
    PIO_OUT_SLOT_1         =>  PIO_OUT_SLOT_1,
    PIO_OUT_SLOT_2          => PIO_OUT_SLOT_2,
    PIO_OUT_SLOT_3        => PIO_OUT_SLOT_3,
    PIO_OUT_SLOT_4         => PIO_OUT_SLOT_4,
    PIO_OUT_SLOT_5        => PIO_OUT_SLOT_5,
    PIO_OUT_SLOT_6         => PIO_OUT_SLOT_6,
    PIO_OUT_SLOT_7        => PIO_OUT_SLOT_7,
    PIO_OUT_SLOT_8         => PIO_OUT_SLOT_8,
    PIO_OUT_SLOT_9      => PIO_OUT_SLOT_9,
    PIO_OUT_SLOT_10         => PIO_OUT_SLOT_10,
    PIO_OUT_SLOT_11         => PIO_OUT_SLOT_11,
    PIO_OUT_SLOT_12         => PIO_OUT_SLOT_12,
    ------------------------------
    IOBP_LED_ID_Bus_i      =>  IOBP_LED_ID_Bus_i,
    PIO_OUT                => PIO_OUT,
    PIO_ENA                => PIO_ENA,
    UIO_OUT                => UIO_OUT,
    UIO_ENA                => UIO_ENA,
    AW_ID                  => AW_ID,
    AWIn_Deb_Time          => AWIn_Deb_Time ,
    Min_AWIn_Deb_Time      => Min_AWIn_Deb_Time,
    Diob_Status1           => Diob_Status1,
    DIOB_Status2           => Diob_Status2,
    IOBP_Id_Reg1           => IOBP_Id_Reg1,
    IOBP_Id_Reg2           => IOBP_Id_Reg2,
    IOBP_Id_Reg3           => IOBP_Id_Reg3,
    IOBP_Id_Reg4           => IOBP_Id_Reg4,
    IOBP_Id_Reg5           => IOBP_Id_Reg5,
    IOBP_Id_Reg6           => IOBP_Id_Reg6,
    IOBP_Id_Reg7           => IOBP_Id_Reg7,
    IOBP_Id_Reg8           => IOBP_Id_Reg8,
    Deb66_in               => Deg66_in,
    Syn66                  => Syn66,
    AW_Input_Reg           => AW_Input_Reg,
    A_Tclk                 => A_Tclk,
    extension_cid_group    => extension_cid_group,
    extension_cid_system   => extension_cid_system,
    Max_AWOut_Reg_Nr       => Max_AWOut_Reg_Nr,
    Max_AWIn_Reg_Nr        => Max_AWIn_Reg_Nr ,
    Debounce_cnt           => Debounce_cnt ,
    s_nLED_User1_i         => s_nLED_User1_i,
    s_nLED_User2_i         => s_nLED_User2_i,
    s_nLED_User3_i         => s_nLED_User3_i,
    --IOBP_Output_Readback   =>  BLM_Status_Reg(0),
    Deb_Sync66             => Deg_Sync66
    
    );


    blm_clk_sig24_9_9_9MHz: blm_24_9_9_9pll 
    PORT map
    (
      areset	=> rstn_sys,
      inclk0	=> clk_sys,

      c0	=>  blm_clk_24_9MHz,
      c1		=>  blm_clk_9_9MHz
    );
 

  blm_clk_100MHz <= ENA_every_10ms;
  blm_clk_10MHz  <= Ena_every_100ns;
  blm_clk_1MHz   <= Ena_every_1us;


  comp_25_Mhz_gen: div_n
  generic map (n => 4, diag_on => 0)  
  port map(
                clk => clk_sys,
                ena => blm_clk_100MHz,
                div_o => blm_clk_25MHz
   );             


  comp_0_99_Mhz_gen: div_n
  generic map (n => 10, diag_on => 0)
    port map(
                clk => clk_sys,
                ena => blm_clk_9_9MHz,
                div_o => blm_clk_0_99MHz
     
  );


  comp_100_kHz_gen:  div_n
  generic map (n => 1000, diag_on => 0) 
  port map(  
                clk => clk_sys,
                ena => blm_clk_100MHz,
                div_o => blm_clk_100kHz
  );


  comp_99kHz_gen:  div_n
  generic map (n => 100, diag_on => 0)  
  port map(
                clk => clk_sys,
                ena => blm_clk_9_9MHz,
                div_o => blm_clk_99kHz
     
  );



  comp_10_kHz_gen:  div_n
  generic map (n => 1000, diag_on => 0)  
  port map (
                clk => clk_sys,
                ena => blm_clk_100MHz,
                div_o => blm_clk_10kHz
  );


  comp_1_kHz_gen: div_n
  generic map (n => 10000, diag_on => 0)  
  port map(
                clk => clk_sys,
                ena => blm_clk_100MHz,
                div_o =>  blm_clk_1kHz
  );
  
end architecture;
