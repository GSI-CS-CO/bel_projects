--TITLE "'aw_io_reg' Autor: R.Hartmann, Stand: 19.12.2014, Vers: V02 ";

-- Version 2, W.Panschow, d. 23.11.2012
--  Ausgang 'AWOut_Reg_Rd_active' hinzugefügt. Kennzeichnet, dass das Macro Daten zum Lesen aum Ausgang 'Data_to_SCUB' bereithält. 'AWOut_Reg_Rd_active' kann übergeordnet zur Steuerung des
--  am 'SCU_Bus_Slave' vorgeschalteten Multiplexers verendet werden. Dieser ist nötig, wenn verschiedene Makros Leseregister zum 'SCU_Bus_Slave'-Eingang 'Data_to_SCUB' anlegen müssen.
--
library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

ENTITY aw_io_reg IS
  generic
      (
  AW_Base_addr:    INTEGER := 16#0200#;
  TAG_Base_addr:  INTEGER := 16#0280#
    );
    
  port(
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    Timing_Pattern_LA:    in   std_logic_vector(31 downto 0);   -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:   in   std_logic;                        -- timing pattern received
    Spare0:               in   std_logic;                          -- vom Master getrieben
    Spare1:               in   std_logic;                          -- vom Master getrieben
    clk:                  in   std_logic;                            -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;
    AWIn1:                in   std_logic_vector(15 downto 0);  -- Input-Port 1
    AWIn2:                in   std_logic_vector(15 downto 0);  -- Input-Port 2
    AWIn3:                in   std_logic_vector(15 downto 0);  -- Input-Port 3
    AWIn4:                in   std_logic_vector(15 downto 0);  -- Input-Port 4
    AWIn5:                in   std_logic_vector(15 downto 0);  -- Input-Port 5
    AWIn6:                in   std_logic_vector(15 downto 0);  -- Input-Port 6
    AWIn7:                in   std_logic_vector(15 downto 0);  -- Input-Port 7
    Max_AWOut_Reg_Nr:     in   integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:      in   integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung
    
    AW_Sts1:              in   std_logic_vector(15 downto 0);  -- Input-Port-AW_Sts1
    AW_Sts2:              in   std_logic_vector(15 downto 0);  -- Input-Port-AW_Sts2

    DIOB_Sts1_Rd:         out  std_logic;                      -- Read Input-Port-DIOB_Sts1
    DIOB_Sts2_Rd:         out  std_logic;                      -- Read Input-Port-DIOB_Sts2
    AW_Sts1_Rd:           out  std_logic;                      -- Read Input-Port-AW_Sts1
    AW_Sts2_Rd:           out  std_logic;                      -- Read Input-Port-AW_Sts2

    DIOB_Config1:         out  std_logic_vector(15 downto 0);  -- Diob-Config1-Register
    DIOB_Config2:         out  std_logic_vector(15 downto 0);  -- Diob-Config2-Register
    AW_Config1:           out  std_logic_vector(15 downto 0);  -- Anwender-Config1-Register
    AW_Config2:           out  std_logic_vector(15 downto 0);  -- Anwender-Config2-Register

    AWOut_Reg1:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut1
    AWOut_Reg2:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut2
    AWOut_Reg3:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut3
    AWOut_Reg4:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut4
    AWOut_Reg5:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut5
    AWOut_Reg6:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut6
    AWOut_Reg7:           out  std_logic_vector(15 downto 0);  -- Daten-Reg. AWOut7

    AWOut_Reg1_wr:        out  std_logic;                      -- Daten-Reg. AWOut1
    AWOut_Reg2_wr:        out  std_logic;                      -- Daten-Reg. AWOut2
    AWOut_Reg3_wr:        out  std_logic;                      -- Daten-Reg. AWOut3
    AWOut_Reg4_wr:        out  std_logic;                      -- Daten-Reg. AWOut4
    AWOut_Reg5_wr:        out  std_logic;                      -- Daten-Reg. AWOut5
    AWOut_Reg6_wr:        out  std_logic;                      -- Daten-Reg. AWOut6
    AWOut_Reg7_wr:        out  std_logic;                      -- Daten-Reg. AWOut7

    AWOut_Reg_rd_active:  out  std_logic;                    -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                      -- connect Dtack to SCUB-Macro
    Tag_Reg_Conf_Err:     out  std_logic;                      -- Config-Error im TAG_Array
    LA_aw_io_reg:         out  std_logic_vector(15 downto 0)
    );  
  end aw_io_reg;


ARCHITECTURE Arch_aw_io_reg OF aw_io_reg IS



--  +============================================================================================================================+
--  |                                                Anfang: Component                                                           |
--  +============================================================================================================================+


COMPONENT tag_n
  PORT
  (  clk:                   in    std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:                 in    std_logic;
    Timing_Pattern_LA:      in    std_logic_vector(31 downto 0);    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:     in    std_logic;                        -- timing pattern received
    Tag_n_hw:               in    std_logic_vector(15 downto 0);    -- 
    Tag_n_lw:               in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Maske:            in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Lev_Reg:          in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Delay_Cnt:        in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Puls_Width:       in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Prescale:         in    std_logic_vector(15 downto 0);    -- 
    Tag_n_Trigger:          in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_1:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_2:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_3:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_4:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_5:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_6:            in    std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_7:            in    std_logic_vector(15 downto 0);    -- 
    Max_AWOut_Reg_Nr:       in    integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:        in    integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn1:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn2:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn3:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn4:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn5:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn6:                  in    std_logic_vector(15 downto 0);    -- 
    AWIn7:                  in    std_logic_vector(15 downto 0);    -- 
    Spare0_Strobe:          in    std_logic;                        -- 
    Spare1_Strobe:          in    std_logic;                        -- 
      
    Tag_n_Reg_Nr:           out   integer range 0 to 7;             -- AWOut-Reg-Pointer
    Tag_n_New_AWOut_Data:   out   boolean;                          -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data:         out   std_logic_vector(15 downto 0);    -- Copy der AWOut-Register 
    Tag_n_Reg_Err:          out   std_logic;                        -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err:      out   std_logic;                        -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err:         out   std_logic;                        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err:     out   std_logic;                        -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout:          out   std_logic;                        -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe:  out   std_logic;                        -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start:         out  std_logic;                       -- Funktionsgenerator Start
    Tag_n_LA:               out   std_logic_vector(15 downto 0)
  );  
  END COMPONENT tag_n;


--  +============================================================================================================================+
--  |                                                    Ende: Component                                                         |
--  +============================================================================================================================+


constant  addr_width:                INTEGER := Adr_from_SCUB_LA'length;
--
constant  DIOB_Config1_addr_offset:  INTEGER := 0;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DIOB_Config1-Registers
constant  DIOB_Config2_addr_offset:  INTEGER := 1;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DIOB_Config2-Registers
constant  DIOB_STS1_addr_offset:     INTEGER := 2;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DIOB_Sts1-Registers
constant  DIOB_STS2_addr_offset:     INTEGER := 3;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DIOB_Sts2-Registers
constant  AW_Config1_addr_offset:    INTEGER := 4;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Config1-Registers
constant  AW_Config2_addr_offset:    INTEGER := 5;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Config2-Registers
constant  AW_STS1_addr_offset:       INTEGER := 6;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Sts1-Registers
constant  AW_STS2_addr_offset:       INTEGER := 7;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Sts2-Registers
--
constant  AWOut_Reg_1_addr_offset:   INTEGER := 16;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant  AWOut_Reg_2_addr_offset:   INTEGER := 17;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant  AWOut_Reg_3_addr_offset:   INTEGER := 18;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant  AWOut_Reg_4_addr_offset:   INTEGER := 19;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant  AWOut_Reg_5_addr_offset:   INTEGER := 20;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_5 Registers
constant  AWOut_Reg_6_addr_offset:   INTEGER := 21;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_6 Registers
constant  AWOut_Reg_7_addr_offset:   INTEGER := 22;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_7 Registers
--
constant  AWIn_1_addr_offset:        INTEGER := 32;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port1
constant  AWIn_2_addr_offset:        INTEGER := 33;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port2
constant  AWIn_3_addr_offset:        INTEGER := 34;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port3
constant  AWIn_4_addr_offset:        INTEGER := 35;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port4
constant  AWIn_5_addr_offset:        INTEGER := 36;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port5
constant  AWIn_6_addr_offset:        INTEGER := 37;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port6
constant  AWIn_7_addr_offset:        INTEGER := 38;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port7

constant  C_DIOB_Config1_addr:  unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + DIOB_Config1_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant  C_DIOB_Config2_addr:  unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + DIOB_Config2_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant  C_DIOB_STS1_addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + DIOB_STS1_addr_offset   ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant  C_DIOB_STS2_addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + DIOB_STS2_addr_offset   ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant  C_AW_Config1_addr:    unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Config1_addr_offset  ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_5 Registers
constant  C_AW_Config2_addr:    unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Config2_addr_offset  ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_6 Registers
constant  C_AW_STS1_addr:       unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_STS1_addr_offset     ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_7 Registers
constant  C_AW_STS2_addr:       unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_STS2_addr_offset     ), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_7 Registers

constant  C_AWOut_Reg_1_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_1_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant  C_AWOut_Reg_2_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_2_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant  C_AWOut_Reg_3_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_3_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant  C_AWOut_Reg_4_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_4_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant  C_AWOut_Reg_5_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_5_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_5 Registers
constant  C_AWOut_Reg_6_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_6_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_6 Registers
constant  C_AWOut_Reg_7_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_7_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AWOut_Reg_7 Registers

constant  C_AWIN_1_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_1_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn1
constant  C_AWIN_2_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_2_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn2
constant  C_AWIN_3_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_3_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn3
constant  C_AWIN_4_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_4_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn4
constant  C_AWIN_5_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_5_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn5
constant  C_AWIN_6_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_6_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn6
constant  C_AWIN_7_Addr:     unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_7_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn7



signal    S_DIOB_Config1:     std_logic_vector(15 downto 0);
signal    S_DIOB_Config1_Rd:  std_logic;
signal    S_DIOB_Config1_Wr:  std_logic;

signal    S_DIOB_Config2:     std_logic_vector(15 downto 0);
signal    S_DIOB_Config2_Rd:  std_logic;
signal    S_DIOB_Config2_Wr:  std_logic;

signal    S_AW_Config1:       std_logic_vector(15 downto 0);
signal    S_AW_Config1_Rd:    std_logic;
signal    S_AW_Config1_Wr:    std_logic;
  
signal    S_AW_Config2:       std_logic_vector(15 downto 0);
signal    S_AW_Config2_Rd:    std_logic;
signal    S_AW_Config2_Wr:    std_logic;

signal    S_DIOB_STS1_Rd:     std_logic;
signal    S_DIOB_STS2_Rd:     std_logic;
signal    S_AW_STS1_Rd:       std_logic;
signal    S_AW_STS2_Rd:       std_logic;


signal    S_AWOut_Reg_1:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_1_Rd:  std_logic;
signal    S_AWOut_Reg_1_Wr:  std_logic;

signal    S_AWOut_Reg_2:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_2_Rd:  std_logic;
signal    S_AWOut_Reg_2_Wr:  std_logic;

signal    S_AWOut_Reg_3:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_3_Rd:  std_logic;
signal    S_AWOut_Reg_3_Wr:  std_logic;

signal    S_AWOut_Reg_4:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_4_Rd:  std_logic;
signal    S_AWOut_Reg_4_Wr:  std_logic;

signal    S_AWOut_Reg_5:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_5_Rd:  std_logic;
signal    S_AWOut_Reg_5_Wr:  std_logic;

signal    S_AWOut_Reg_6:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_6_Rd:  std_logic;
signal    S_AWOut_Reg_6_Wr:  std_logic;

signal    S_AWOut_Reg_7:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_7_Rd:  std_logic;
signal    S_AWOut_Reg_7_Wr:  std_logic;

signal    S_AWIn1_Rd:      std_logic;
signal    S_AWIn2_Rd:      std_logic;
signal    S_AWIn3_Rd:      std_logic;
signal    S_AWIn4_Rd:      std_logic;
signal    S_AWIn5_Rd:      std_logic;
signal    S_AWIn6_Rd:      std_logic;
signal    S_AWIn7_Rd:      std_logic;


constant  Tag_Base_0_addr_offset:  INTEGER := 00; -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  Tag_Base_1_addr_offset:  INTEGER := 16;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-1 Datensatzes
constant  Tag_Base_2_addr_offset:  INTEGER := 32;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-2 Datensatzes
constant  Tag_Base_3_addr_offset:  INTEGER := 48;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-3 Datensatzes
constant  Tag_Base_4_addr_offset:  INTEGER := 64;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-4 Datensatzes
constant  Tag_Base_5_addr_offset:  INTEGER := 80;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-5 Datensatzes
constant  Tag_Base_6_addr_offset:  INTEGER := 96;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-6 Datensatzes
constant  Tag_Base_7_addr_offset:  INTEGER := 112;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-7 Datensatzes

constant  C_Tag_Base_0_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_0_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_1_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_1_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_2_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_2_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_3_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_3_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_4_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_4_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_5_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_5_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_6_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_6_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  C_Tag_Base_7_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_7_addr_offset), addr_width);  -- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes


signal    S_Tag_Base_Addr_Wr:    std_logic;
signal    S_Tag_Base_0_Addr_Rd:  std_logic;
signal    S_Tag_Base_0_Addr_Wr:  std_logic;
signal    S_Tag_Base_1_Addr_Rd:  std_logic;
signal    S_Tag_Base_1_Addr_Wr:  std_logic;
signal    S_Tag_Base_2_Addr_Rd:  std_logic;
signal    S_Tag_Base_2_Addr_Wr:  std_logic;
signal    S_Tag_Base_3_Addr_Rd:  std_logic;
signal    S_Tag_Base_3_Addr_Wr:  std_logic;
signal    S_Tag_Base_4_Addr_Rd:  std_logic;
signal    S_Tag_Base_4_Addr_Wr:  std_logic;
signal    S_Tag_Base_5_Addr_Rd:  std_logic;
signal    S_Tag_Base_5_Addr_Wr:  std_logic;
signal    S_Tag_Base_6_Addr_Rd:  std_logic;
signal    S_Tag_Base_6_Addr_Wr:  std_logic;
signal    S_Tag_Base_7_Addr_Rd:  std_logic;
signal    S_Tag_Base_7_Addr_Wr:  std_logic;

signal    S_Dtack:        std_logic;
signal    S_Read_Port:    std_logic_vector(Data_to_SCUB'range);


constant  Timeout_Trigger:    INTEGER := 2500;   -- Counter Timeout (2500 x 8ns = 20us)

constant  i_tag_hw:           INTEGER := 0; -- Index Tag-Data: High-Word
constant  i_tag_lw:           INTEGER := 1; -- Index Tag-Data: Low-Word
constant  i_Tag_Maske:        INTEGER := 2; -- Index Tag-Level und Tag-Maske
constant  i_Tag_Lev_Reg:      INTEGER := 3; -- Index Tag-Data: High-Byte
constant  i_Tag_Delay_Cnt:    INTEGER := 4; -- Index Tag_Array: Verzögerungszeit in Clock's
constant  i_Tag_Puls_Width:   INTEGER := 5; -- Index Tag_Array: "Monoflop"-Pulsbreite in Clock's
constant  i_Tag_Prescale:     INTEGER := 6; -- Index Tag_Array: Vorteiler für: D[15..8] = Verzögerungszeit, D[15..8] = Pulsbreite
constant  i_Tag_Trigger:      INTEGER := 7; -- Index Tag_Array: Input-Trigger-Sel für:  D[11..8] = Input-Reg-Nr.,

constant  i_Tag0:             INTEGER := 0; -- Index Tag_Array: Tag-Nr0
constant  i_Tag1:             INTEGER := 1; -- Index Tag_Array: Tag-Nr1
constant  i_Tag2:             INTEGER := 2; -- Index Tag_Array: Tag-Nr2
constant  i_Tag3:             INTEGER := 3; -- Index Tag_Array: Tag-Nr3
constant  i_Tag4:             INTEGER := 4; -- Index Tag_Array: Tag-Nr4
constant  i_Tag5:             INTEGER := 5; -- Index Tag_Array: Tag-Nr5
constant  i_Tag6:             INTEGER := 6; -- Index Tag_Array: Tag-Nr6
constant  i_Tag7:             INTEGER := 7; -- Index Tag_Array: Tag-Nr7


TYPE   t_Tag_Element    is array (0 to 15) of std_logic_vector(15 downto 0);
TYPE   t_Tag_Array      is array (0 to 7)  of t_Tag_Element;
TYPE   t_Word_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
TYPE   t_Byte_Array     is array (0 to 7)  of std_logic;
TYPE   t_Boolean_Array  is array (1 to 7)  of boolean;
Type   t_Count          is array (0 to 7)  of integer range 0 to 65535;  -- 0-FFFF
Type   t_Prescale       is array (0 to 7)  of integer range 0 to 255;    -- 0-FF
 
Type   t_Int_0_to_7     is array (0 to 7)  of integer range 0 to 7; 
Type   t_Int_0_to_15    is array (0 to 7)  of integer range 0 to 15;
Type   t_Int_0_to_31    is array (0 to 7)  of integer range 0 to 31;




---------------------- Output-Signale von "Tag_n" für Tag0-T-------------------------------

signal  Tag_Array:              t_Tag_Array;                    -- Init über den SCU-Bus
signal  AWOut_Reg_Array:        t_Word_Array;                   -- Copy der AWOut-Register
signal  Tag_New_AWOut_Data:     t_Boolean_Array;                -- Flag's für New Data von Register 1-7 

signal  Tag0_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag0_New_AWOut_Data:    boolean; 
signal  Tag0_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag0_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag0_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag0_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag0_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag0_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag0_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag0_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag0_LA:                std_logic_vector(15 downto 0); 

signal  Tag1_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag1_New_AWOut_Data:    boolean; 
signal  Tag1_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag1_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag1_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag1_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag1_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag1_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag1_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag1_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag1_LA:                std_logic_vector(15 downto 0); 

signal  Tag2_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag2_New_AWOut_Data:    boolean; 
signal  Tag2_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag2_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag2_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag2_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag2_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag2_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag2_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag2_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag2_LA:                std_logic_vector(15 downto 0); 

signal  Tag3_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag3_New_AWOut_Data:    boolean; 
signal  Tag3_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag3_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag3_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag3_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag3_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag3_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag3_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag3_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag3_LA:                std_logic_vector(15 downto 0); 

signal  Tag4_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag4_New_AWOut_Data:    boolean; 
signal  Tag4_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag4_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag4_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag4_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag4_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag4_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag4_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag4_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag4_LA:                std_logic_vector(15 downto 0); 

signal  Tag5_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag5_New_AWOut_Data:    boolean; 
signal  Tag5_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag5_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag5_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag5_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag5_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag5_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag5_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag5_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag5_LA:                std_logic_vector(15 downto 0); 

signal  Tag6_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag6_New_AWOut_Data:    boolean; 
signal  Tag6_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag6_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag6_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag6_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag6_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag6_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag6_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag6_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag6_LA:                std_logic_vector(15 downto 0); 

signal  Tag7_Reg_Nr:            integer range 0 to 7;           -- AWOut-Reg-Nummer 
signal  Tag7_New_AWOut_Data:    boolean; 
signal  Tag7_New_Data:          std_logic_vector(15 downto 0); 
--signal  Tag7_Reg_Err:           std_logic;                       -- Config-Error: TAG-Reg-Nr
--signal  Tag7_Reg_Max_Err:       std_logic;                       -- Config-Error: TAG-Max_Reg-Nr
--signal  Tag7_Trig_Err:          std_logic;                       -- Config-Error: Trig-Reg
--signal  Tag7_Trig_Max_Err:      std_logic;                       -- Config-Error: Trig-Max_Reg-Nr
--signal  Tag7_Timeout:           std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
--signal  Tag7_ext_Trig_Strobe:   std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
--signal  Tag7_FG_Start:          std_logic;                       -- Funktionsgenerator Start
signal  Tag7_LA:                std_logic_vector(15 downto 0); 

signal  DIOB_Sts1:              std_logic_vector(15 downto 0); 
signal  DIOB_Sts2:              std_logic_vector(15 downto 0); 

signal  Tag_n_FG_Start:         std_logic_vector(7 downto 0);     -- Summe alles Signale: Fg-Start
signal  Tag_n_Reg_Err:          std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: TAG-Reg-Nr
signal  Tag_n_Reg_max_Err:      std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: TAG-Reg-Nr-Max
signal  Tag_n_Trig_Err:         std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: Trig-Reg
signal  Tag_n_Trig_max_Err:     std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: Trig-Reg-Max
signal  Tag_n_Timeout:          std_logic_vector(7 downto 0);     -- Summe alles Signale: Timeout-Error ext. Trigger, Spare0/Spare1
signal  Tag_n_ext_Trig_Strobe:  std_logic_vector(7 downto 0);     -- Summe alles Signale: ext. Trigger vom Input-Register

signal  Tag_FG_Start:           std_logic;     -- Fg-Start
signal  Tag_Reg_Err:            std_logic;     -- Config-Error: TAG-Reg-Nr
signal  Tag_Reg_max_Err:        std_logic;     -- Config-Error: TAG-Reg-Nr-Max
signal  Tag_Trig_Err:           std_logic;     -- Config-Error: Trig-Reg
signal  Tag_Trig_max_Err:       std_logic;     -- Config-Error: Trig-Reg-Max
signal  Tag_Timeout:            std_logic;     -- Timeout-Error ext. Trigger, Spare0/Spare1
signal  Tag_ext_Trig_Strobe:    std_logic;     -- ext. Trigger vom Input-Register

-------------------------------------------------------------------------------------------



attribute   keep: boolean;
--attribute   keep of Tag_cnt: signal is true;
--attribute   keep of Tag_Level: signal is true;


signal  Tag_Loop:           integer range 0 to 3 := 0;      -- Loop-Counter
signal  Tag_Reg_Error:      std_logic_vector(7 downto 0);   -- Tag Auswerte-Loop


signal  Tag_Code_Base:      std_logic_vector(31 downto 0);  -- Basis Tag-Code
signal  Tag_Code_Compare:   std_logic_vector(31 downto 0);  -- Compare Tag-Code
signal  Tag_Reg_Base:       std_logic_vector(3 downto 0);   -- Basis Tag_Register
signal  Tag_Reg_Compare:    std_logic_vector(3 downto 0);   -- Compare Tag_Register



signal Tag_Base_Wr_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Tag_Base_Wr_Strobe_i:  std_logic;        -- input  
signal Tag_Base_Wr_Strobe_o:  std_logic;        -- Output 
signal Tag_Base_Wr_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.

signal DIOB_Sts1_rd_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal DIOB_Sts1_rd_Strobe_o:  std_logic;        -- Output 
signal DIOB_Sts1_rd_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.

signal Spare0_Out:          std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Spare0_Strobe:       std_logic;        -- Output 
signal Spare0_shift:        std_logic_vector(2  downto 0); -- Shift-Reg.
    
signal Spare1_Out:          std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Spare1_Strobe:       std_logic;        -- Output 
signal Spare1_shift:        std_logic_vector(2  downto 0); -- Shift-Reg.

signal Tag_Conf_Err_Save:   std_logic := '0'; -- Tag Auswerte-Loop
signal Tag_Conf_Err_Comp:   std_logic := '0'; -- Tag Auswerte-Loop
signal Tag_Test_Wait:       integer range 0 to 100 := 0; -- Loop-Counter

signal Ext_Adr_Val_Out:     std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Ext_Adr_Val_Strobe:  std_logic;        -- Output 
signal Ext_Adr_Val_shift:   std_logic_vector(2  downto 0); -- Shift-Reg.


type state_t is   (idle,  Test_Wait,  TReg_Loop,  Error,  Weiter);
signal state:      state_t := idle;


type type_t0 is   (ST0_idle,   ST0_trigger, ST0_trig_S0, ST0_trig_S1, ST0_trig_Reg,
                   ST0_delay, ST0_pre_vz,  ST0_rd1,  ST0_wr1,  ST0_mono,  ST0_pre_mono,  ST0_rd2,  ST0_wr2,  ST0_err_end,  ST0_end);
signal ST0_state:  type_t0 := ST0_idle;

type type_t1 is   (ST1_idle,   ST1_trigger, ST1_trig_S0, ST1_trig_S1, ST1_trig_Reg,
                   ST1_delay, ST1_pre_vz,  ST1_rd1,  ST1_wr1,  ST1_mono,  ST1_pre_mono,  ST1_rd2,  ST1_wr2,  ST1_err_end,  ST1_end);
signal ST1_state:  type_t1 := ST1_idle;



signal TC_Cnt   : integer range 0 to 7 := 0;
signal TReg_Cnt : integer range 0 to 7 := 0;
signal TErr_Cnt : integer range 0 to 7 := 0;

signal  Tag_Conf_Err:       std_logic := '0'; -- Tag Auswerte-Loop
signal  s_Tag_Reg_Conf_Err: std_logic := '0'; -- Tag Auswerte-Loop 


begin


-- 
----------- Puls aus Ext_Adr_Val (1 Clock breit) --------------------
--
--p_Ext_Adr_Val:  PROCESS (clk, nReset)
--  BEGin
--    IF  nReset                = '0' then
--        Ext_Adr_Val_shift  <= (OTHERS => '0');
--        Ext_Adr_Val_Strobe <= '0';
--
--    ELSIF rising_edge(clk) THEN
--
--      Ext_Adr_Val_shift <= (Ext_Adr_Val_shift(Ext_Adr_Val_shift'high-1 downto 0) & (Ext_Adr_Val)); -- Ext_Adr_Val_Strobe = Puls, nach der neg. Flanke von Ext_Adr_Val 
--
--      IF Ext_Adr_Val_shift(Ext_Adr_Val_shift'high) = '0' AND Ext_Adr_Val_shift(Ext_Adr_Val_shift'high-1) = '1' THEN
--        Ext_Adr_Val_Strobe <= '1';
--      ELSE
--        Ext_Adr_Val_Strobe <= '0';
--      END IF;
--    END IF;
--  END PROCESS p_Ext_Adr_Val;6



P_Adr_Deco:  process (nReset, clk)
  begin
    if nReset = '0' then

      S_DIOB_Config1_Rd <= '0';    S_DIOB_Config1_Wr <= '0';
      S_DIOB_Config2_Rd <= '0';    S_DIOB_Config2_Wr <= '0';
      S_AW_Config1_Rd   <= '0';    S_AW_Config1_Wr <= '0';
      S_AW_Config2_Rd   <= '0';    S_AW_Config2_Wr <= '0';

      S_DIOB_Sts1_Rd <= '0';  S_DIOB_Sts2_Rd <= '0';
      S_AW_Sts1_Rd   <= '0';  S_AW_Sts2_Rd   <= '0';

      
      S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
      S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
      S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
      S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
      S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
      S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
      S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

      S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
      S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';  
      
      S_Tag_Base_0_Addr_Rd <= '0';  S_Tag_Base_0_Addr_Wr <= '0';
      S_Tag_Base_1_Addr_Rd <= '0';  S_Tag_Base_1_Addr_Wr <= '0';
      S_Tag_Base_2_Addr_Rd <= '0';  S_Tag_Base_2_Addr_Wr <= '0';
      S_Tag_Base_3_Addr_Rd <= '0';  S_Tag_Base_3_Addr_Wr <= '0';
      S_Tag_Base_4_Addr_Rd <= '0';  S_Tag_Base_4_Addr_Wr <= '0';
      S_Tag_Base_5_Addr_Rd <= '0';  S_Tag_Base_5_Addr_Wr <= '0';
      S_Tag_Base_6_Addr_Rd <= '0';  S_Tag_Base_6_Addr_Wr <= '0';
      S_Tag_Base_7_Addr_Rd <= '0';  S_Tag_Base_7_Addr_Wr <= '0';

      S_Dtack <= '0';
      AWOut_Reg_rd_active <= '0';
    
    elsif rising_edge(clk) then

      S_DIOB_Config1_Rd <= '0';    S_DIOB_Config1_Wr <= '0';
      S_DIOB_Config2_Rd <= '0';    S_DIOB_Config2_Wr <= '0';
      S_AW_Config1_Rd   <= '0';    S_AW_Config1_Wr <= '0';
      S_AW_Config2_Rd   <= '0';    S_AW_Config2_Wr <= '0';

      S_DIOB_Sts1_Rd <= '0';  S_DIOB_Sts2_Rd <= '0';
      S_AW_Sts1_Rd   <= '0';  S_AW_Sts2_Rd   <= '0';

      
      S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
      S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
      S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
      S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
      S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
      S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
      S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

      S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
      S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';

      S_Tag_Base_0_Addr_Rd <= '0';  S_Tag_Base_0_Addr_Wr <= '0';
      S_Tag_Base_1_Addr_Rd <= '0';  S_Tag_Base_1_Addr_Wr <= '0';
      S_Tag_Base_2_Addr_Rd <= '0';  S_Tag_Base_2_Addr_Wr <= '0';
      S_Tag_Base_3_Addr_Rd <= '0';  S_Tag_Base_3_Addr_Wr <= '0';
      S_Tag_Base_4_Addr_Rd <= '0';  S_Tag_Base_4_Addr_Wr <= '0';
      S_Tag_Base_5_Addr_Rd <= '0';  S_Tag_Base_5_Addr_Wr <= '0';
      S_Tag_Base_6_Addr_Rd <= '0';  S_Tag_Base_6_Addr_Wr <= '0';
      S_Tag_Base_7_Addr_Rd <= '0';  S_Tag_Base_7_Addr_Wr <= '0';

      S_Dtack <= '0';
      AWOut_Reg_rd_active <= '0';
      
      if Ext_Adr_Val = '1' then

        CASE unsigned(ADR_from_SCUB_LA) IS
        
          when C_DIOB_Config1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DIOB_Config1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DIOB_Config1_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_DIOB_Config2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DIOB_Config2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DIOB_Config2_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AW_Config1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Config1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Config1_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AW_Config2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Config2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Config2_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;
       
            when C_DIOB_STS1_addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_DIOB_Sts1_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;
        
            when C_DIOB_STS2_addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_DIOB_Sts2_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AW_STS1_addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AW_Sts1_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;
        
            when C_AW_STS2_addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AW_Sts2_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;
            
            
            
          when C_AWOut_Reg_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_1_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

          when C_AWOut_Reg_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_2_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

          when C_AWOut_Reg_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_3_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_3_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

          when C_AWOut_Reg_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_4_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_4_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWOut_Reg_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_5_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_5_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWOut_Reg_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_6_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_6_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWOut_Reg_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_7_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_7_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            
            when C_AWIN_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn1_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn2_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn3_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn4_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn5_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn6_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_AWIN_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn7_Rd   <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            
            
          when others => 

            S_DIOB_Config1_Rd <= '0';    S_Diob_Config1_Wr <= '0';
            S_DIOB_Config2_Rd <= '0';    S_Diob_Config2_Wr <= '0';
            S_AW_Config1_Rd <= '0';      S_AW_Config1_Wr <= '0';
            S_AW_Config2_Rd <= '0';      S_AW_Config2_Wr <= '0';

            S_DIOB_Sts1_Rd <= '0';       S_DIOB_Sts2_Rd <= '0';
            S_AW_Sts1_Rd   <= '0';       S_AW_Sts2_Rd   <= '0';

            
            S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
            S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
            S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
            S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
            S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
            S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
            S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

            S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
            S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';
                      
--            S_Dtack <= '0';
--            AWOut_Reg_rd_active <= '0';

        end CASE;

        
        
        CASE unsigned(ADR_from_SCUB_LA(15 downto 4)) IS

            when C_Tag_Base_0_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_0_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_0_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_1_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_1_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_1_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_2_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_2_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_2_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_3_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_3_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_3_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_4_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_4_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_4_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_5_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_5_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_5_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_6_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_6_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_6_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;

            when C_Tag_Base_7_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_7_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_7_Addr_Rd <= '1';
              AWOut_Reg_rd_active <= '1';
            end if;
            
          when others => 

            S_Tag_Base_0_Addr_Rd <= '0';  S_Tag_Base_0_Addr_Wr <= '0';
            S_Tag_Base_1_Addr_Rd <= '0';  S_Tag_Base_1_Addr_Wr <= '0';
            S_Tag_Base_2_Addr_Rd <= '0';  S_Tag_Base_2_Addr_Wr <= '0';
            S_Tag_Base_3_Addr_Rd <= '0';  S_Tag_Base_3_Addr_Wr <= '0';
            S_Tag_Base_4_Addr_Rd <= '0';  S_Tag_Base_4_Addr_Wr <= '0';
            S_Tag_Base_5_Addr_Rd <= '0';  S_Tag_Base_5_Addr_Wr <= '0';
            S_Tag_Base_6_Addr_Rd <= '0';  S_Tag_Base_6_Addr_Wr <= '0';
            S_Tag_Base_7_Addr_Rd <= '0';  S_Tag_Base_7_Addr_Wr <= '0';
            
--            S_Dtack <= '0';
--            AWOut_Reg_rd_active <= '0';

        end CASE;

  end if;
    end if;
  
  end process P_Adr_Deco;


  
--------- Puls als Signal (1 Clock breit) --------------------

p_Tag_Base_Wr_Strobe:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then
        Tag_Base_Wr_shift    <= (OTHERS => '0');
        Tag_Base_Wr_Strobe_o <= '0';

    ELSIF rising_edge(clk) THEN

      S_Tag_Base_Addr_Wr <= (S_Tag_Base_0_Addr_Wr or S_Tag_Base_1_Addr_Wr or  --+
                             S_Tag_Base_2_Addr_Wr or S_Tag_Base_3_Addr_Wr or  --+--> wurden Daten ist Tag_Array geschrieben ?
                             S_Tag_Base_4_Addr_Wr or S_Tag_Base_5_Addr_Wr or  --+
                             S_Tag_Base_6_Addr_Wr or S_Tag_Base_7_Addr_Wr);   --+

      Tag_Base_Wr_shift <= (Tag_Base_Wr_shift(Tag_Base_Wr_shift'high-1 downto 0) & (S_Tag_Base_Addr_Wr));

      IF Tag_Base_Wr_shift(Tag_Base_Wr_shift'high) = '0' AND Tag_Base_Wr_shift(Tag_Base_Wr_shift'high-1) = '1' THEN
        Tag_Base_Wr_Strobe_o <= '1';
      ELSE
        Tag_Base_Wr_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_Tag_Base_Wr_Strobe;
  
  

P_Tag_Config_ok:  process (clk, nReset)

    begin
      if (nReset = '0') then
        state   <= idle;
        TC_Cnt            <= 0;   -- Tag_Code_Counter
        TReg_Cnt          <= 0;   -- Tag_Register_Counter
        TErr_Cnt          <= 0;   -- Tag_Error_Counter
        Tag_Test_Wait     <= 0;   -- Tag_Test_Wait_Counter
        Tag_Conf_Err      <=  '0'; -- Error-Flag: Registernummer
        Tag_Code_Base     <= (others => '0'); 
        Tag_Code_Compare  <= (others => '0');
        Tag_Reg_Base      <= (others => '0');
        Tag_Reg_Compare   <= (others => '0');
      ELSIF rising_edge(clk) then
      case state is
         when idle  =>  TC_Cnt            <= 0;   -- Tag_Code_Counter
                        TReg_Cnt          <= 0;   -- Tag_Register_Counter
                        TErr_Cnt          <= 0;   -- Tag_Error_Counter
--                      Tag_Conf_Err      <=  '0'; -- Error-Flag: Registernummer
                        Tag_Test_Wait     <= 0;   -- Tag_Test_Wait_Counter
                        Tag_Code_Base     <= (others => '0'); 
                        Tag_Code_Compare  <= (others => '0');
                        Tag_Reg_Base      <= (others => '0');
                        Tag_Reg_Compare   <= (others => '0');
--                      if (Tag_Base_Wr_Strobe_o ='1') then
--                        state <= TReg_Loop;
--                      end if;
                        state <= Test_Wait;

         when Test_Wait   =>   if (Tag_Test_Wait  < 50) then 
                                   Tag_Test_Wait  <= Tag_Test_Wait+1;    
                                  state           <= Test_Wait;
                                elsE
                                  state     <= TReg_Loop;
                                end if;
                            
         when TReg_Loop  => if (TReg_Cnt < 8) then      -- solange kleiner 8 (0-7)

                              Tag_Code_Base    <= (Tag_Array(TC_Cnt)  (i_tag_hw) & Tag_Array(TC_Cnt)  (i_tag_lw)); -- merke Tag-Code(0-7), gegen den alle anderen Tag-Codes getestet werden sollen
                              Tag_Code_Compare <= (Tag_Array(TReg_Cnt)(i_tag_hw) & Tag_Array(TReg_Cnt)(i_tag_lw)); -- Tag-Code der anderen Tags
                              Tag_Reg_Base     <=  Tag_Array(TC_Cnt)  (i_Tag_Lev_Reg)(3 downto 0); -- Register-Nr. des Tags, gegen den alle anderen Reg.-Nr. getestet werden sollen
                              Tag_Reg_Compare  <=  Tag_Array(TReg_Cnt)(i_Tag_Lev_Reg)(3 downto 0); -- Register-Nr. der anderen Tags

                              IF (Tag_Code_Base = (31 downto 0=>'0')) THEN     -- Tag-Code = 0
                                 state <= Weiter;
                              Else
                                IF (Tag_Code_Base = Tag_Code_Compare  AND (Tag_Reg_Base = Tag_Reg_Compare)) THEN  -- Gleicher Tag-Code und Reg.-Nr.
                                  state <= Error;
                                else
                                  state <= Weiter;
                                end if;
                              end if;
                            end if;
         
         when Error    =>   TErr_Cnt  <= TErr_Cnt+1;  -- Tag_Error_Counter +1 
                            state <= Weiter;
                            
         when Weiter    =>  TReg_Cnt <= TReg_Cnt+1; 
                            IF (TC_Cnt = 7) and (TReg_Cnt = 7) THEN
                                state <= idle;
                            ELSE
                              IF (TReg_Cnt = 7) THEN
                                TC_Cnt    <= TC_Cnt+1;    -- Nächster Basis_Tag_Code
                                TReg_Cnt  <= 0;           -- 
                                TErr_Cnt  <= 0;           -- Tag_Error_Counter auf 0 setzen
                                Tag_Conf_Err  <=  '0'; -- Reset Error-Flag: Registernummer
                                state <= TReg_Loop;
                                  IF (TErr_Cnt >= 2 ) THEN          -- (größer oder gleich) beim Reg._Loop von 0-7: ein Fehler = der eigene (Tag_Code + Reg.-Nr.)
                                    Tag_Conf_Err   <=  '1'; -- Error-Flag: Registernummer
                                  ELSE
                                    Tag_Conf_Err   <=  '0'; -- Error-Flag: Registernummer
                                  END IF;
                            end if;
                              state <= TReg_Loop;
                            end if;
      end case;
    end if;
  end process P_Tag_Config_ok;
  

 
--------- Puls als Spare0 (1 Clock breit) --------------------

p_Spare0:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then
        Spare0_shift  <= (OTHERS => '0');
        Spare0_Strobe <= '0';

    ELSIF rising_edge(clk) THEN

      Spare0_shift <= (Spare0_shift(Spare0_shift'high-1 downto 0) & (Not Spare0)); -- Spare0_Strobe = Puls, nach der neg. Flanke von Spare0 

      IF Spare0_shift(Spare0_shift'high) = '0' AND Spare0_shift(Spare0_shift'high-1) = '1' THEN
        Spare0_Strobe <= '1';
      ELSE
        Spare0_Strobe <= '0';
      END IF;
    END IF;
  END PROCESS p_Spare0;
  

--------- Puls als Spare1 (1 Clock breit) --------------------

p_Spare1:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then
        Spare1_shift  <= (OTHERS => '0');
        Spare1_Strobe <= '0';

    ELSIF rising_edge(clk) THEN

      Spare1_shift <= (Spare1_shift(Spare1_shift'high-1 downto 0) & (Not Spare1)); -- Spare1_Strobe = Puls, nach der neg. Flanke von Spare1 

      IF Spare1_shift(Spare1_shift'high) = '0' AND Spare1_shift(Spare1_shift'high-1) = '1' THEN
        Spare1_Strobe <= '1';
      ELSE
        Spare1_Strobe <= '0';
      END IF;
    END IF;
  END PROCESS p_Spare1;

  

--  +============================================================================================================================+
--  |                                   Anfang: Daten I/O für Tag0-Tag7 (Component(Tag_n)                                        |
--  +============================================================================================================================+
tag0: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag0)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag0)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag0)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag0)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag0)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag0)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag0)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag0)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag0_Reg_Nr,              -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag0_New_AWOut_Data,      -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag0_New_Data,            -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(0),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(0),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(0),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(0),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(0),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(0), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(0),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag0_LA      
  );  

tag1: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag1)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag1)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag1)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag1)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag1)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag1)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag1)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag1)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag1_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag1_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag1_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(1),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(1),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(1),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(1),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(1),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(1), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(1),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag1_LA      
  );  

tag2: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag2)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag2)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag2)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag2)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag2)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag2)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag2)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag2)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag2_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag2_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag2_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(2),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(2),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(2),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(2),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(2),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(2), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(2),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag2_LA      
  );  

tag3: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag3)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag3)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag3)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag3)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag3)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag3)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag3)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag3)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag3_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag3_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag3_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(3),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(3),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(3),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(3),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(3),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(3), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(3),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag3_LA      
  );  

tag4: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag4)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag4)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag4)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag4)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag4)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag4)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag4)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag4)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag4_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag4_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag4_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(4),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(4),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(4),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(4),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(4),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(4), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(4),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag4_LA      
  );  

tag5: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag5)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag5)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag5)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag5)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag5)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag5)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag5)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag5)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag5_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag5_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag5_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(5),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(5),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(5),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(5),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(5),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(5), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(5),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag5_LA      
  );  

tag6: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag6)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag6)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag6)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag6)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag6)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag6)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag6)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag6)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag6_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag6_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag6_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(6),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(6),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(6),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(6),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(6),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(6), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(6),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag6_LA      
  );  

tag7: tag_n     
port map  (     
    clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave        
    nReset                =>    nReset,   
    Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
    Tag_n_hw              =>   (Tag_Array(i_Tag7)(i_tag_hw)),          --+ 
    Tag_n_lw              =>   (Tag_Array(i_Tag7)(i_tag_lw)),          --| 
    Tag_n_Maske           =>   (Tag_Array(i_Tag7)(i_Tag_Maske)),       --| 
    Tag_n_Lev_Reg         =>   (Tag_Array(i_Tag7)(i_Tag_Lev_Reg)),     --+-----> Tag-Array 
    Tag_n_Delay_Cnt       =>   (Tag_Array(i_Tag7)(i_Tag_Delay_Cnt)),   --| 
    Tag_n_Puls_Width      =>   (Tag_Array(i_Tag7)(i_Tag_Puls_Width)),  --| 
    Tag_n_Prescale        =>   (Tag_Array(i_Tag7)(i_Tag_Prescale)),    --| 
    Tag_n_Trigger         =>   (Tag_Array(i_Tag7)(i_Tag_Trigger)),    --+ 
    AWOut_Reg_1           =>    s_AWOut_Reg_1,         -- Daten-Reg. AWOut1  
    AWOut_Reg_2           =>    s_AWOut_Reg_2,         -- Daten-Reg. AWOut2  
    AWOut_Reg_3           =>    s_AWOut_Reg_3,         -- Daten-Reg. AWOut3  
    AWOut_Reg_4           =>    s_AWOut_Reg_4,         -- Daten-Reg. AWOut4  
    AWOut_Reg_5           =>    s_AWOut_Reg_5,         -- Daten-Reg. AWOut5  
    AWOut_Reg_6           =>    s_AWOut_Reg_6,         -- Daten-Reg. AWOut6  
    AWOut_Reg_7           =>    s_AWOut_Reg_7,         -- Daten-Reg. AWOut7  
    AWIn1                 =>    AWIn1,                 -- Input-Reg. AWIn1  
    AWIn2                 =>    AWIn2,                 -- Input-Reg. AWIn2  
    AWIn3                 =>    AWIn3,                 -- Input-Reg. AWIn3  
    AWIn4                 =>    AWIn4,                 -- Input-Reg. AWIn4  
    AWIn5                 =>    AWIn5,                 -- Input-Reg. AWIn5  
    AWIn6                 =>    AWIn6,                 -- Input-Reg. AWIn6  
    AWIn7                 =>    AWIn7,                 -- Input-Reg. AWIn7  
    Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,      -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,       -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe         =>    Spare0_Strobe,         -- 
    Spare1_Strobe         =>    Spare1_Strobe,         -- 
      
    Tag_n_Reg_Nr          =>    Tag7_Reg_Nr,          -- AWOut-Reg-Pointer 
    Tag_n_New_AWOut_Data  =>    Tag7_New_AWOut_Data,  -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data        =>    Tag7_New_Data,        -- Copy der AWOut-Register  
    Tag_n_Reg_Err         =>    Tag_n_Reg_Err(7),         -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(7),     -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err        =>    Tag_n_Trig_Err(7),        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(7),    -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout         =>    Tag_n_Timeout(7),         -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(7), -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start        =>    Tag_n_FG_Start(7),        -- Funktionsgenerator Start
    Tag_n_LA              =>    Tag7_LA      
  );  

--  +============================================================================================================================+
--  |                                     Ende: Daten I/O für Tag0-Tag7 (Component(Tag_n)                                        |
--  +============================================================================================================================+



--  +============================================================================================================================+
--  |                           Übernahme der Daten aus Component Tag_n, für Tag0- Tag7 und Eintrag in                           |
--  |                        AWOut_Reg_Array und Tag_New_AWOut_Data, zum überschreiben der Output-Register.                      |
--  +============================================================================================================================+

P_AWOut_Array:  process (nReset, clk)
  begin
    if nReset = '0' then
     
      AWOut_Reg_Array     <= (others => (others => '0'));   --
      Tag_New_AWOut_Data  <= (others => false);             --  
      
      elsif rising_edge(clk) then

        if Tag0_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag0_Reg_Nr) <= Tag0_New_Data;       -- die Daten aus "Tag0" werden in das Register mit der "Tag0_Reg_Nr" geschrieben.
          Tag_New_AWOut_Data(Tag0_Reg_Nr) <= Tag0_New_AWOut_Data; -- das Flag aus "Tag0" zeigt an: die Daten im Output-Register, wurden geändert.
  
        elsif Tag1_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag1_Reg_Nr) <= Tag1_New_Data;
          Tag_New_AWOut_Data(Tag1_Reg_Nr) <= Tag1_New_AWOut_Data;
  
        elsif Tag2_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag2_Reg_Nr) <= Tag2_New_Data;
          Tag_New_AWOut_Data(Tag2_Reg_Nr) <= Tag2_New_AWOut_Data;
  
        elsif Tag3_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag3_Reg_Nr) <= Tag3_New_Data;
          Tag_New_AWOut_Data(Tag3_Reg_Nr) <= Tag3_New_AWOut_Data;
  
        elsif Tag4_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag4_Reg_Nr) <= Tag4_New_Data;
          Tag_New_AWOut_Data(Tag4_Reg_Nr) <= Tag4_New_AWOut_Data;
  
        elsif Tag5_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag5_Reg_Nr) <= Tag5_New_Data;
          Tag_New_AWOut_Data(Tag5_Reg_Nr) <= Tag5_New_AWOut_Data;
  
        elsif Tag6_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag6_Reg_Nr) <= Tag6_New_Data;
          Tag_New_AWOut_Data(Tag6_Reg_Nr) <= Tag6_New_AWOut_Data;

        elsif Tag7_New_AWOut_Data = true   then
          AWOut_Reg_Array   (Tag7_Reg_Nr) <= Tag7_New_Data;
          Tag_New_AWOut_Data(Tag7_Reg_Nr) <= Tag7_New_AWOut_Data;

        else
          Tag_New_AWOut_Data  <= (others => false);
        end if;
    end if;
  end process P_AWOut_Array;
  

--  +============================================================================================================================+
--  |                      1. Die Output-Daten vom SCU-Bus werden in die jeweiligen Register geschrieben.                        |
--  |                      2. Die von "Tag-Steuerung" geänderten Daten überschreiben die Output-Register.                        |
--  +============================================================================================================================+
  
  
P_AWOut_Reg:  process (nReset, clk)
  begin
    if nReset = '0' then
      S_DIOB_Config1  <= (others => '0');
      S_DIOB_Config2  <= (others => '0');
      S_AW_Config1    <= (others => '0');
      S_AW_Config2    <= (others => '0');
      S_AWOut_Reg_1   <= (others => '0');
      S_AWOut_Reg_2   <= (others => '0');
      S_AWOut_Reg_3   <= (others => '0');
      S_AWOut_Reg_4   <= (others => '0');
      S_AWOut_Reg_5   <= (others => '0');
      S_AWOut_Reg_6   <= (others => '0');
      S_AWOut_Reg_7   <= (others => '0');
    
    elsif rising_edge(clk) then
      if S_DIOB_Config1_Wr = '1'   then  S_DIOB_Config1 <= Data_from_SCUB_LA;
      end if;
      if S_DIOB_Config2_Wr = '1'   then  S_DIOB_Config2 <= Data_from_SCUB_LA;
      end if;
      if S_AW_Config1_Wr = '1'     then  S_AW_Config1 <= Data_from_SCUB_LA;
      end if;  
      if S_AW_Config2_Wr = '1'     then  S_AW_Config2 <= Data_from_SCUB_LA;
      end if;
      
      
      if S_AWOut_Reg_1_Wr = '1'   then  S_AWOut_Reg_1 <= Data_from_SCUB_LA;   -- Output-Daten vom SCU-Bus
      elsif Tag_New_AWOut_Data(1) then  S_AWOut_Reg_1 <= AWOut_Reg_Array(1);  -- Output-Daten werden mit "Tag-Array-Daten" überschrieben
      end if;

      if S_AWOut_Reg_2_Wr = '1'   then  S_AWOut_Reg_2 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(2) then  S_AWOut_Reg_2 <= AWOut_Reg_Array(2);
      end if;

      if S_AWOut_Reg_3_Wr = '1'   then  S_AWOut_Reg_3 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(3) then  S_AWOut_Reg_3 <= AWOut_Reg_Array(3);
      end if;

      if S_AWOut_Reg_4_Wr = '1'   then  S_AWOut_Reg_4 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(4) then  S_AWOut_Reg_4 <= AWOut_Reg_Array(4);
      end if;

      if S_AWOut_Reg_5_Wr = '1'   then  S_AWOut_Reg_5 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(5) then  S_AWOut_Reg_5 <= AWOut_Reg_Array(5);
      end if;

      if S_AWOut_Reg_6_Wr = '1'   then  S_AWOut_Reg_6 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(6) then  S_AWOut_Reg_6 <= AWOut_Reg_Array(6);
      end if;

      if S_AWOut_Reg_7_Wr = '1'   then  S_AWOut_Reg_7 <= Data_from_SCUB_LA;
      elsif Tag_New_AWOut_Data(7) then  S_AWOut_Reg_7 <= AWOut_Reg_Array(7);
      end if;


                                                       
      if S_Tag_Base_0_Addr_Wr = '1' then               
--              +--- Zeilen-Nr. im Array
--              |  +-------------------- Adresse der "Wordposition" -+
--              |  |                                                 |
      Tag_Array(0)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;

      if S_Tag_Base_1_Addr_Wr = '1' then  
      Tag_Array(1)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_2_Addr_Wr = '1' then  
      Tag_Array(2)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_3_Addr_Wr = '1' then  
      Tag_Array(3)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_4_Addr_Wr = '1' then  
      Tag_Array(4)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_5_Addr_Wr = '1' then  
      Tag_Array(5)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_6_Addr_Wr = '1' then  
      Tag_Array(6)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;
      if S_Tag_Base_7_Addr_Wr = '1' then  
      Tag_Array(7)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
      end if;

  end if;
  end process P_AWOut_Reg;
  

  

  P_read_mux:  process (S_DIOB_Config1_Rd,  S_DIOB_Config1,
                        S_DIOB_Config2_Rd,  S_DIOB_Config2,
                        S_AW_Config1_Rd,    S_AW_Config1,
                        S_AW_Config2_Rd,    S_AW_Config2,
                        S_DIOB_Sts1_Rd,     DIOB_Sts1,
                        S_DIOB_Sts2_Rd,     DIOB_Sts2,
                        S_AW_Sts1_Rd,       AW_Sts1,
                        S_AW_Sts2_Rd,       AW_Sts2,

                       S_AWOut_Reg_1_Rd,  S_AWOut_Reg_1,
                       S_AWOut_Reg_2_Rd,  S_AWOut_Reg_2,
                       S_AWOut_Reg_3_Rd,  S_AWOut_Reg_3,
                       S_AWOut_Reg_4_Rd,  S_AWOut_Reg_4,
                       S_AWOut_Reg_5_Rd,  S_AWOut_Reg_5,
                       S_AWOut_Reg_6_Rd,  S_AWOut_Reg_6,
                       S_AWOut_Reg_7_Rd,  S_AWOut_Reg_7,
                       S_AWIn1_Rd,        AWIn1,
                       S_AWIn2_Rd,        AWIn2,
                       S_AWIn3_Rd,        AWIn3,
                       S_AWIn4_Rd,        AWIn4,
                       S_AWIn5_Rd,        AWIn5,
                       S_AWIn6_Rd,        AWIn6,
                       S_AWIn7_Rd,        AWIn7,
                       S_Tag_Base_0_Addr_Rd, S_Tag_Base_1_Addr_Rd,
                       S_Tag_Base_2_Addr_Rd, S_Tag_Base_3_Addr_Rd,
                       S_Tag_Base_4_Addr_Rd, S_Tag_Base_5_Addr_Rd,
                       S_Tag_Base_6_Addr_Rd, S_Tag_Base_7_Addr_Rd,
                       Tag_Array, Adr_from_SCUB_LA)

  begin
    if    S_DIOB_Config1_Rd    = '1' then  S_Read_port <= S_DIOB_Config1;
    elsif S_DIOB_Config2_Rd    = '1' then  S_Read_port <= S_DIOB_Config2;
    elsif S_AW_Config1_Rd      = '1' then  S_Read_port <= S_AW_Config1;
    elsif S_AW_Config2_Rd      = '1' then  S_Read_port <= S_AW_Config2;

    elsif S_DIOB_Sts1_Rd    = '1' then  S_Read_port <= DIOB_Sts1;
    elsif S_DIOB_Sts2_Rd    = '1' then  S_Read_port <= DIOB_Sts2;
    elsif S_AW_Sts1_Rd      = '1' then  S_Read_port <= AW_Sts1;
    elsif S_AW_Sts2_Rd      = '1' then  S_Read_port <= AW_Sts2;


    elsif S_AWOut_Reg_1_Rd = '1' then  S_Read_port <= S_AWOut_Reg_1;
    elsif S_AWOut_Reg_2_Rd = '1' then  S_Read_port <= S_AWOut_Reg_2;
    elsif S_AWOut_Reg_3_Rd = '1' then  S_Read_port <= S_AWOut_Reg_3;
    elsif S_AWOut_Reg_4_Rd = '1' then  S_Read_port <= S_AWOut_Reg_4;
    elsif S_AWOut_Reg_5_Rd = '1' then  S_Read_port <= S_AWOut_Reg_5;
    elsif S_AWOut_Reg_6_Rd = '1' then  S_Read_port <= S_AWOut_Reg_6;
    elsif S_AWOut_Reg_7_Rd = '1' then  S_Read_port <= S_AWOut_Reg_7;

    elsif S_AWIn1_Rd = '1' then  S_Read_port <= AWIn1;    -- read Input-Port1
    elsif S_AWIn2_Rd = '1' then  S_Read_port <= AWIn2;
    elsif S_AWIn3_Rd = '1' then  S_Read_port <= AWIn3;
    elsif S_AWIn4_Rd = '1' then  S_Read_port <= AWIn4;
    elsif S_AWIn5_Rd = '1' then  S_Read_port <= AWIn5;
    elsif S_AWIn6_Rd = '1' then  S_Read_port <= AWIn6;
    elsif S_AWIn7_Rd = '1' then  S_Read_port <= AWIn7;

--                                                                    +--- Zeilen-Nr. im Array
--                                                                    |  +---- Adresse der "Wordposition" in der Zeile ----+
--                                                                    |  |                                                 |
    elsif S_Tag_Base_0_Addr_Rd = '1' then  S_Read_port <= Tag_Array(0)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_1_Addr_Rd = '1' then  S_Read_port <= Tag_Array(1)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_2_Addr_Rd = '1' then  S_Read_port <= Tag_Array(2)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_3_Addr_Rd = '1' then  S_Read_port <= Tag_Array(3)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_4_Addr_Rd = '1' then  S_Read_port <= Tag_Array(4)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_5_Addr_Rd = '1' then  S_Read_port <= Tag_Array(5)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_6_Addr_Rd = '1' then  S_Read_port <= Tag_Array(6)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
    elsif S_Tag_Base_7_Addr_Rd = '1' then  S_Read_port <= Tag_Array(7)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));



  else
      S_Read_Port <= (others => '-');
    end if;
  end process P_Read_mux;



  --------- Summenbildung: Fg_Start und Fehlermeldungen  --------------------

p_summe:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then

      Tag_FG_Start          <= '0';       
      Tag_Reg_Err           <= '0';         
      Tag_Reg_max_Err       <= '0';     
      Tag_Trig_Err          <= '0';        
      Tag_Trig_max_Err      <= '0';   
      Tag_Timeout           <= '0';         
      Tag_ext_Trig_Strobe   <= '0';

    ELSIF rising_edge(clk) THEN

      if  Tag_n_FG_Start          = x"00" then   -- Fg-Start
        Tag_FG_Start             <= '0';       
      else      
        Tag_FG_Start             <= '1';       
      end if;

      if  Tag_n_Reg_Err           = x"00" then   -- Config-Error: TAG-Reg-Nr 
        Tag_Reg_Err              <= '0';       
      else        
        Tag_Reg_Err              <= '1';       
      end if;

      if  Tag_n_Reg_max_Err       = x"00" then   -- Config-Error: TAG-Reg-Nr 
        Tag_Reg_max_Err          <= '0';       
      else    
        Tag_Reg_max_Err          <= '1';       
      end if;

      if  Tag_n_Trig_Err          = x"00" then   -- Config-Error: TAG-Trig-Nr 
        Tag_Trig_Err             <= '0';       
      else      
        Tag_Trig_Err             <= '1';       
      end if;

      if  Tag_n_Trig_max_Err      = x"00" then   -- Config-Error: TAG-Trig-Nr 
        Tag_Trig_max_Err         <= '0';       
      else  
        Tag_Trig_max_Err         <= '1';       
      end if;

      if  Tag_n_Timeout           = x"00" then   -- Config-Error: Timeout 
        Tag_Timeout              <= '0';       
      else        
        Tag_Timeout              <= '1';       
      end if;

      if  Tag_n_ext_Trig_Strobe   = x"00" then   -- Tag ext. Trigger vom Input-Register
        Tag_ext_Trig_Strobe      <= '0';       
      else  
        Tag_ext_Trig_Strobe      <= '1';       
      end if;

    END IF;
  END PROCESS p_Summe;


  
--------- Reset DIOB-Status1 nach read (1 Clock breit) --------------------

p_AW_Config1_Rd:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then
        DIOB_Sts1_rd_shift    <= (OTHERS => '0');
        DIOB_Sts1_rd_Strobe_o <= '0';

    ELSIF rising_edge(clk) THEN

      DIOB_Sts1_rd_shift <= (DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high-1 downto 0) & (Not S_DIOB_Config1_Rd)); -- DIOB_Sts1_rd_Strobe_o = Puls, nach der neg. Flanke von S_AW_Config_Rd 

      IF DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high) = '0' AND DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high-1) = '1' THEN
        DIOB_Sts1_rd_Strobe_o <= '1';
      ELSE
        DIOB_Sts1_rd_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_AW_Config1_Rd;


  
--------- DIOB-Staus-1 --------------------

P_Tag_Reg_Conf_Err:  process (clk, nReset)
  begin
    IF  nReset             = '0' then
        DIOB_Sts1         <= (OTHERS => '0');

    elsif rising_edge(clk) then
      if (DIOB_Sts1_rd_Strobe_o = '1')  then
        DIOB_Sts1         <= (OTHERS => '0');   -- reset Status-Register nach dem Read
      end if;
     
        if (Tag_Conf_Err = '1') then
          DIOB_Sts1(5)  <= '1';
        end if;
     
        if (Tag_Reg_Err  = '1') then
          DIOB_Sts1(4)  <= '1';
        end if;
     
        if (Tag_Trig_Err = '1') then
          DIOB_Sts1(3)  <= '1';
        end if;
     
        if (Tag_Reg_max_Err = '1') then
          DIOB_Sts1(2)     <= '1';
        end if;
     
        if (Tag_Trig_max_Err = '1') then
          DIOB_Sts1(1)      <= '1';
        end if;
     
        if (Tag_Timeout  = '1') then
          DIOB_Sts1(0)  <= '1';
        end if;

     end if;

  end process P_Tag_Reg_Conf_Err;
  
                
  
  
LA_aw_io_reg  <=      (Timing_Pattern_RCV    &    Spare1_Strobe    &    Spare0_Strobe  &  Tag_ext_Trig_Strobe     &   -- Testport für Logic-Analysator
                       Tag_FG_Start          & '0'                 & '0'               & '0'                      &        
                       Tag_Reg_Err           &  Tag_Reg_max_Err    &  Tag_Trig_Err     &  Tag_Trig_max_Err        &
                       Tag_Timeout           & '0'                 & '0'               & '0'                        );   

                       
Tag_Reg_Conf_Err <= s_Tag_Reg_Conf_Err;
  
Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB <= S_Read_Port;

DIOB_Config1      <= S_DIOB_Config1;      -- Configurations-Reg.
DIOB_Config2      <= S_DIOB_Config2;      -- Configurations-Reg.
AW_Config1        <= S_AW_Config1;        -- Configurations-Reg.
AW_Config2        <= S_AW_Config2;        -- Configurations-Reg.

DIOB_Sts1_Rd      <= S_DIOB_Sts1_Rd;      -- Read Input-Port-DIOB_Sts1
DIOB_Sts2_Rd      <= S_DIOB_Sts2_RD;      -- Read Input-Port-DIOB_Sts2
AW_Sts1_Rd        <= S_AW_Sts1_Rd;        -- Read Input-Port-AW_Sts1
AW_Sts2_Rd        <= S_AW_Sts2_Rd;        -- Read Input-Port-AW_Sts2

AWOut_Reg1        <= S_AWOut_Reg_1;    -- Daten-Reg. AWOut1
AWOut_Reg2        <= S_AWOut_Reg_2;    -- Daten-Reg. AWOut2
AWOut_Reg3        <= S_AWOut_Reg_3;    -- Daten-Reg. AWOut3
AWOut_Reg4        <= S_AWOut_Reg_4;    -- Daten-Reg. AWOut4
AWOut_Reg5        <= S_AWOut_Reg_5;    -- Daten-Reg. AWOut5
AWOut_Reg6        <= S_AWOut_Reg_6;    -- Daten-Reg. AWOut6
AWOut_Reg7        <= S_AWOut_Reg_7;    -- Daten-Reg. AWOut7

AWOut_Reg1_wr     <= S_AWOut_Reg_1_wr;  -- Daten-Reg. AWOut1
AWOut_Reg2_wr     <= S_AWOut_Reg_2_wr;  -- Daten-Reg. AWOut2
AWOut_Reg3_wr     <= S_AWOut_Reg_3_wr;  -- Daten-Reg. AWOut3
AWOut_Reg4_wr     <= S_AWOut_Reg_4_wr;  -- Daten-Reg. AWOut4
AWOut_Reg5_wr     <= S_AWOut_Reg_5_wr;  -- Daten-Reg. AWOut5
AWOut_Reg6_wr     <= S_AWOut_Reg_6_wr;  -- Daten-Reg. AWOut6
AWOut_Reg7_wr     <= S_AWOut_Reg_7_wr;  -- Daten-Reg. AWOut7

DIOB_Sts2         <= x"123B";  -- Input-Port-DIOB_Sts2


end Arch_AW_IO_Reg;