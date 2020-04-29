--TITLE "'tag_ctrl' Autor: R.Hartmann, Stand: 30.05.2017 ";
--
library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

library work;
use work.scu_diob_pkg.all;


ENTITY tag_ctrl IS
  generic
      (
  TAG_Base_addr:  INTEGER := 16#0280#
    );

  port(
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:          in   std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                      -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                      -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                      -- marks end of write cycle, active one for one clock period of sys_clk
    Timing_Pattern_LA:    in   std_logic_vector(31 downto 0);  -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:   in   std_logic;                      -- timing pattern received
    Spare0:               in   std_logic;                      -- vom Master getrieben
    Spare1:               in   std_logic;                      -- vom Master getrieben
    clk:                  in   std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    SCU_AW_Input_Reg:     in   t_IO_Reg_1_to_7_Array;          -- Input-Port's wie zum SCU-Bus

    Clr_Tag_Config:       in   std_logic;                      -- Clear Tag-Konfigurations-Register
    Max_AWOut_Reg_Nr:     in   integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:      in   integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung

    Tag_matched_7_0:      out  std_logic_vector(7 downto 0);   -- Active on matched Tags for one clock period after match, one bit for each tag unit

    Tag_Maske_Reg:        out  t_IO_Reg_1_to_7_Array;          -- Tag-Output-Maske für Register 1-7
    Tag_Outp_Reg:         out  t_IO_Reg_1_to_7_Array;          -- Tag-Output-Maske für Register 1-7

    Tag_FG_Start:         out  std_logic;                      -- Start-Puls für den FG
    Tag_Sts:              out  std_logic_vector(15 downto 0);  -- Tag-Status

    Rd_active:            out  std_logic;                      -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                      -- connect Dtack to SCUB-Macro
    Tag_Aktiv:            out  std_logic_vector( 7 downto 0);  -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)
    LA_tag_ctrl:          out  std_logic_vector(15 downto 0)
    );
  end tag_ctrl;


ARCHITECTURE Arch_tag_ctrl OF tag_ctrl IS



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
    Tag_n_Register:         in    std_logic_vector(15 downto 0);    --
    Tag_n_Level:            in    std_logic_vector(15 downto 0);    --
    Tag_n_Delay_Cnt:        in    std_logic_vector(15 downto 0);    --
    Tag_n_Puls_Width:       in    std_logic_vector(15 downto 0);    --
    Tag_n_Prescale:         in    std_logic_vector(15 downto 0);    --
    Tag_n_Trigger:          in    std_logic_vector(15 downto 0);    --
    Max_AWOut_Reg_Nr:       in    integer range 0 to 7;             -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:        in    integer range 0 to 7;             -- Maximale AWIn-Reg-Nummer der Anwendung
    SCU_AW_Input_Reg:       in    t_IO_Reg_1_to_7_Array;            --
    Spare0_Strobe:          in    std_logic;                        --
    Spare1_Strobe:          in    std_logic;                        --

    Tag_matched:            out  std_logic;                         -- Tag_matched is high for one clock pulse on matching Tags

    Tag_n_Reg_Nr:           out   integer range 0 to 7;             -- AWOut-Reg-Pointer
    Tag_n_New_AWOut_Data:   out   boolean;                          -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_Maske_Hi_Bits:    out  std_logic_vector(15 downto 0);   -- Maske für "High-Aktive" Bits im Ausgangs-Register
    Tag_n_Maske_Lo_Bits:    out  std_logic_vector(15 downto 0);   -- Maske für "Low-Aktive"  Bits im Ausgangs-Register
    Tag_n_Reg_Err:          out   std_logic;                        -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err:      out   std_logic;                        -- Config-Error: TAG_Max_Reg_Nr
    Tag_n_Trig_Err:         out   std_logic;                        -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err:     out   std_logic;                        -- Config-Error: Trig_Max_Reg_Nr
    Tag_n_Timeout:          out   std_logic;                        -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe:  out   std_logic;                        -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start:         out   std_logic;                        -- Funktionsgenerator Start
    Tag_n_LA:               out   std_logic_vector(15 downto 0)
  );
  END COMPONENT tag_n;


--  +============================================================================================================================+
--  |                                                    Ende: Component                                                         |
--  +============================================================================================================================+


constant  addr_width:                INTEGER := Adr_from_SCUB_LA'length;
--
constant  Tag_Base_0_addr_offset:  INTEGER := 00;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant  Tag_Base_1_addr_offset:  INTEGER := 16;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-1 Datensatzes
constant  Tag_Base_2_addr_offset:  INTEGER := 32;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-2 Datensatzes
constant  Tag_Base_3_addr_offset:  INTEGER := 48;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-3 Datensatzes
constant  Tag_Base_4_addr_offset:  INTEGER := 64;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-4 Datensatzes
constant  Tag_Base_5_addr_offset:  INTEGER := 80;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-5 Datensatzes
constant  Tag_Base_6_addr_offset:  INTEGER := 96;    -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-6 Datensatzes
constant  Tag_Base_7_addr_offset:  INTEGER := 112;   -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-7 Datensatzes

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
constant  i_Tag_Maske:        INTEGER := 2; -- Index Tag-Maske
constant  i_Tag_Register:     INTEGER := 3; -- Index Tag-Register
constant  i_Tag_Delay_Cnt:    INTEGER := 4; -- Index Tag_Array: Verzögerungszeit in Clock's
constant  i_Tag_Puls_Width:   INTEGER := 5; -- Index Tag_Array: "Monoflop"-Pulsbreite in Clock's
constant  i_Tag_Prescale:     INTEGER := 6; -- Index Tag_Array: Vorteiler für: D[15..8] = Verzögerungszeit, D[15..8] = Pulsbreite
constant  i_Tag_Trigger:      INTEGER := 7; -- Index Tag_Array: Input-Trigger-Sel für:  D[11..8] = Input-Reg-Nr.,
constant  i_Tag_Level:        INTEGER := 8; -- Index Tag-Register


TYPE   t_Tag_Element    is array (0 to 15) of std_logic_vector(15 downto 0);
TYPE   t_Tag_Array      is array (0 to 7)  of t_Tag_Element;
TYPE   t_Word_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
TYPE   t_Byte_Array     is array (0 to 7)  of std_logic;
TYPE   t_Boolean_Array  is array (0 to 7)  of boolean;
Type   t_Count          is array (0 to 7)  of integer range 0 to 65535;  -- 0-FFFF
Type   t_Prescale       is array (0 to 7)  of integer range 0 to 255;    -- 0-FF

Type   t_Int_0_to_7     is array (0 to 7)  of integer range 0 to 7;
Type   t_Int_0_to_15    is array (0 to 7)  of integer range 0 to 15;
Type   t_Int_0_to_31    is array (0 to 7)  of integer range 0 to 31;




---------------------- Output-Signale von "Tag_n" für Tag0-T-------------------------------

signal  Tag_Array:              t_Tag_Array;                    -- Init über den SCU-Bus
signal  Tag_Out_Reg_Array:      t_IO_Reg_1_to_7_Array;          -- Copy der AWOut-Register

signal  Reg_Nr_Tag:            t_Int_0_to_7;           -- AWOut-Reg-Nummer
signal  New_AWOut_Data_Tag:    t_Boolean_Array;
signal  Maske_Hi_Bits_Tag:     t_Word_Array;   -- Maske für "High-Aktive" Bits im Ausgangs-Register
signal  Maske_Lo_Bits_Tag:     t_Word_Array;   -- Maske für "Low-Aktive"  Bits im Ausgangs-Register
signal  LA_Tag:                t_Word_Array;


signal  DIOB_Sts1:              std_logic_vector(15 downto 0);
signal  DIOB_Sts2:              std_logic_vector(15 downto 0);

signal  Tag_n_FG_Start:         std_logic_vector(7 downto 0);     -- Summe alles Signale: Fg-Start
signal  Tag_n_Reg_Err:          std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: TAG-Reg-Nr
signal  Tag_n_Reg_max_Err:      std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: TAG-Reg-Nr-Max
signal  Tag_n_Trig_Err:         std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: Trig-Reg
signal  Tag_n_Trig_max_Err:     std_logic_vector(7 downto 0);     -- Summe alles Signale: Config-Error: Trig-Reg-Max
signal  Tag_n_Timeout:          std_logic_vector(7 downto 0);     -- Summe alles Signale: Timeout-Error ext. Trigger, Spare0/Spare1
signal  Tag_n_ext_Trig_Strobe:  std_logic_vector(7 downto 0);     -- Summe alles Signale: ext. Trigger vom Input-Register

signal  s_Tag_FG_Start:         std_logic;     -- Fg-Start
signal  Tag_Reg_Err:            std_logic;     -- Config-Error: TAG-Reg-Nr
signal  Tag_Reg_max_Err:        std_logic;     -- Config-Error: TAG-Reg-Nr-Max
signal  Tag_Trig_Err:           std_logic;     -- Config-Error: Trig-Reg
signal  Tag_Trig_max_Err:       std_logic;     -- Config-Error: Trig-Reg-Max
signal  Tag_Timeout:            std_logic;     -- Timeout-Error ext. Trigger, Spare0/Spare1
signal  Tag_ext_Trig_Strobe:    std_logic;     -- ext. Trigger vom Input-Register

signal  Tag_LA_Dummy:           std_logic;     -- Dummy Tag_LA-Ports

-------------------------------------------------------------------------------------------



attribute   keep: boolean;
--attribute   keep of Tag_cnt: signal is true;
--attribute   keep of Tag_Level: signal is true;


signal  Tag_Loop:             integer range 0 to 3 := 0;      -- Loop-Counter
signal  Tag_Reg_Error:        std_logic_vector(7 downto 0);   -- Tag Auswerte-Loop


signal  Tag_Code_Base:        std_logic_vector(31 downto 0);  -- Basis Tag-Code
signal  Tag_Code_Compare:     std_logic_vector(31 downto 0);  -- Compare Tag-Code
signal  Tag_Reg_Base:         std_logic_vector(3 downto 0);   -- Basis Tag_Register
signal  Tag_Reg_Compare:      std_logic_vector(3 downto 0);   -- Compare Tag_Register



signal Tag_Base_Wr_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Tag_Base_Wr_Strobe_i:  std_logic;        -- input
signal Tag_Base_Wr_Strobe_o:  std_logic;        -- Output
signal Tag_Base_Wr_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.

signal Spare0_Out:            std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Spare0_Strobe:         std_logic;        -- Output
signal Spare0_shift:          std_logic_vector(2  downto 0); -- Shift-Reg.

signal Spare1_Out:            std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Spare1_Strobe:         std_logic;        -- Output
signal Spare1_shift:          std_logic_vector(2  downto 0); -- Shift-Reg.

signal Ext_Adr_Val_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Ext_Adr_Val_Strobe:    std_logic;        -- Output
signal Ext_Adr_Val_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.

signal Tag_Test_Wait:         integer range 0 to 100 := 0; -- Loop-Counter

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

signal Sum_Reg_MSK:   t_IO_Reg_1_to_7_Array;

signal s_Tag_Aktiv:   std_logic_vector(7 downto 0); -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)

signal Register_Nr  : integer range 0 to 7  := 0;


begin

P_Adr_Deco:  process (nReset, clk)
  begin
    if nReset = '0' then

      S_Tag_Base_0_Addr_Rd <= '0';  S_Tag_Base_0_Addr_Wr <= '0';
      S_Tag_Base_1_Addr_Rd <= '0';  S_Tag_Base_1_Addr_Wr <= '0';
      S_Tag_Base_2_Addr_Rd <= '0';  S_Tag_Base_2_Addr_Wr <= '0';
      S_Tag_Base_3_Addr_Rd <= '0';  S_Tag_Base_3_Addr_Wr <= '0';
      S_Tag_Base_4_Addr_Rd <= '0';  S_Tag_Base_4_Addr_Wr <= '0';
      S_Tag_Base_5_Addr_Rd <= '0';  S_Tag_Base_5_Addr_Wr <= '0';
      S_Tag_Base_6_Addr_Rd <= '0';  S_Tag_Base_6_Addr_Wr <= '0';
      S_Tag_Base_7_Addr_Rd <= '0';  S_Tag_Base_7_Addr_Wr <= '0';

      S_Dtack <= '0';
      Rd_active <= '0';

    elsif rising_edge(clk) then

      S_Tag_Base_0_Addr_Rd <= '0';  S_Tag_Base_0_Addr_Wr <= '0';
      S_Tag_Base_1_Addr_Rd <= '0';  S_Tag_Base_1_Addr_Wr <= '0';
      S_Tag_Base_2_Addr_Rd <= '0';  S_Tag_Base_2_Addr_Wr <= '0';
      S_Tag_Base_3_Addr_Rd <= '0';  S_Tag_Base_3_Addr_Wr <= '0';
      S_Tag_Base_4_Addr_Rd <= '0';  S_Tag_Base_4_Addr_Wr <= '0';
      S_Tag_Base_5_Addr_Rd <= '0';  S_Tag_Base_5_Addr_Wr <= '0';
      S_Tag_Base_6_Addr_Rd <= '0';  S_Tag_Base_6_Addr_Wr <= '0';
      S_Tag_Base_7_Addr_Rd <= '0';  S_Tag_Base_7_Addr_Wr <= '0';

      S_Dtack <= '0';
      Rd_active <= '0';

      if Ext_Adr_Val = '1' then

        CASE unsigned(ADR_from_SCUB_LA(15 downto 4)) IS

            when C_Tag_Base_0_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_0_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_0_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_1_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_1_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_1_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_2_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_2_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_2_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_3_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_3_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_3_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_4_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_4_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_4_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_5_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_5_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_5_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_6_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_6_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_6_Addr_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_Tag_Base_7_Addr(15 downto 4) =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_7_Addr_WR <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Tag_Base_7_Addr_Rd <= '1';
              Rd_active <= '1';
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

            S_Dtack <= '0';
            Rd_active <= '0';

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
                              Tag_Reg_Base     <=  Tag_Array(TC_Cnt)  (i_Tag_Register)(3 downto 0); -- Register-Nr. des Tags, gegen den alle anderen Reg.-Nr. getestet werden sollen
                              Tag_Reg_Compare  <=  Tag_Array(TReg_Cnt)(i_Tag_Register)(3 downto 0); -- Register-Nr. der anderen Tags

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

p_Spare0:  PROCESS (clk, nReset, Spare0)
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

p_Spare1:  PROCESS (clk, nReset, Spare1)
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


Tag:  for N in 0 to 7 generate
    I:  tag_n

    port map  (
        clk                   =>    clk,                   -- should be the same clk, used by SCU_Bus_Slave
        nReset                =>    nReset,
        Timing_Pattern_LA     =>    Timing_Pattern_LA,     -- latched timing pattern from SCU_Bus for external user functions
        Timing_Pattern_RCV    =>    Timing_Pattern_RCV,    -- timing pattern received
        Tag_n_hw              =>   (Tag_Array(N)(i_tag_hw)),           --+
        Tag_n_lw              =>   (Tag_Array(N)(i_tag_lw)),           --|
        Tag_n_Maske           =>   (Tag_Array(N)(i_Tag_Maske)),        --|
        Tag_n_Register        =>   (Tag_Array(N)(i_Tag_Register)),     --+-----> Tag-Array
        Tag_n_Level           =>   (Tag_Array(N)(i_Tag_Level)),        --|
        Tag_n_Delay_Cnt       =>   (Tag_Array(N)(i_Tag_Delay_Cnt)),    --|
        Tag_n_Puls_Width      =>   (Tag_Array(N)(i_Tag_Puls_Width)),   --|
        Tag_n_Prescale        =>   (Tag_Array(N)(i_Tag_Prescale)),     --|
        Tag_n_Trigger         =>   (Tag_Array(N)(i_Tag_Trigger)),      --+
        SCU_AW_Input_Reg      =>    SCU_AW_Input_Reg,             -- Input-Reg. SCU_AW_Input_Reg
        Max_AWOut_Reg_Nr      =>    Max_AWOut_Reg_Nr,         -- Maximale AWOut-Reg-Nummer der Anwendung
        Max_AWIn_Reg_Nr       =>    Max_AWIn_Reg_Nr,          -- Maximale AWIn-Reg-Nummer der Anwendung
        Spare0_Strobe         =>    Spare0_Strobe,            --
        Spare1_Strobe         =>    Spare1_Strobe,            --

        Tag_matched           =>    Tag_matched_7_0(N),


        Tag_n_Reg_Nr          =>    Reg_Nr_Tag(N),            -- AWOut-Reg-Pointer
        Tag_n_New_AWOut_Data  =>    New_AWOut_Data_Tag(N),    -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
        Tag_n_Maske_Hi_Bits   =>    Maske_Hi_Bits_Tag(N),     -- Maske für "High-Aktive" Bits im Ausgangs-Register
        Tag_n_Maske_Lo_Bits   =>    Maske_Lo_Bits_Tag(N),     -- Maske für "Low-Aktive"  Bits im Ausgangs-Register
        Tag_n_Reg_Err         =>    Tag_n_Reg_Err(N),         -- Config-Error: TAG-Reg-Nr
        Tag_n_Reg_max_Err     =>    Tag_n_Reg_Max_Err(N),     -- Config-Error: TAG_Max_Reg_Nr
        Tag_n_Trig_Err        =>    Tag_n_Trig_Err(N),        -- Config-Error: Trig-Reg
        Tag_n_Trig_max_Err    =>    Tag_n_Trig_Max_Err(N),    -- Config-Error: Trig_Max_Reg_Nr
        Tag_n_Timeout         =>    Tag_n_Timeout(N),         -- Timeout-Error ext. Trigger, Spare0/Spare1
        Tag_n_ext_Trig_Strobe =>    Tag_n_ext_Trig_Strobe(N), -- ext. Trigger-Eingang, aus Input-Register-Bit
        Tag_n_FG_Start        =>    Tag_n_FG_Start(N),        -- Funktionsgenerator Start
        Tag_n_LA              =>    LA_Tag(N)
      );
    end generate Tag;


--  +============================================================================================================================+
--  |                                     Ende: Daten I/O für Tag0-Tag7 (Component(Tag_n)                                        |
--  +============================================================================================================================+



--  +============================================================================================================================+
--  |       Übernahme der Daten aus Component Tag_n, für Tag0- Tag7 und Eintrag in Tag_Out_Reg_Array und Tag_New_AWOut_Data      |
--  +============================================================================================================================+
--  | Problem:                                                                                                                   |
--  | 1. Mehrere TAGs können auf das gleiche Outputregister zugreifen (aber nicht auf das gleiche Bit).                          |
--  | 2. zB.: TAG1 kann im Outputreg1 Bit 13 setzen, TAG3 aber Bit 13 zurücksetzen.                                              |
--  | 3. Wenn TAGx und TAGy das gleiche Bit setzen/rücksetzen wollen --> Fehler.                                                 |
--  | 4. New_AWOut_Data_TAGn ist ein Synclock aktiv und zeigt die Änderung an (Maske, Level high/low)                            |
--  | 5. Alle TAGs könennen als Ziel das gleiche Outputregister haben.                                                           |
--  |                                                                                                                            |
--  | Lösung:                                                                                                                    |
--  | Abfrage aller Änderungen (aus den TAGs) auf die Bits in den Outputregistern.                                               |
--  | Die "erste" gültige IF Abfrage für jedes Bit ist wirksam, die Anderen gültigen gehen verloren.            "K.Kaiser"       |
--  |                                                                                                                            |
--  +============================================================================================================================+


P_TAG_AWOut_REG:
  for Register_Nr in 1 to 7 generate  -- für Outputregister 1 bis 7

--  +====================================================================================================================+
--  |     Process zum setzen/rücksetzen Bit(0-15) und setzen der Maske für "ein" Register, ensprechend TAG(0-7)          |
--  +====================================================================================================================+


  P_TAG_REG:  process (nReset, clk, New_AWOut_Data_Tag, Maske_Hi_Bits_Tag, Maske_Lo_Bits_Tag, Tag_Array, Clr_Tag_Config)
    begin
      if nReset = '0' then

        Tag_Out_Reg_Array(Register_Nr)   <= (others => '0');  -- Daten für Output-Register
        Sum_Reg_MSK(Register_Nr)         <= (others => '0');  -- Clear Summen-Maske für das Output-Register
      elsif rising_edge(clk) then

      if Clr_Tag_Config = '1'   then
         Tag_Out_Reg_Array(Register_Nr)   <= (others => '0');  -- Daten für Output-Register
         Sum_Reg_MSK(Register_Nr)         <= (others => '0');  -- Clear Summen-Maske für das Output-Register
      end if;


--        FOR Bit_Nr in 0 to 15 loop
--  +====================================== Register n, Bit 0 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if     ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(0)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(0) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(0)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(0) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(0) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(0)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 1 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bit"
         else                                            Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(1) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(1)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(1) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(1) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(1)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 2 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if     ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(2) = '1'))  then
        if      Maske_Hi_Bits_Tag(0)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(2) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(2)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(2) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(2) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(2)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 3 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(3) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(3)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(3) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(3) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(3)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 4 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(4) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(4)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(4) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(4) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(4)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 5 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(5) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(5)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(5) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(5) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(5)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 6 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(6) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(6)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(6) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(6) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(6)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 7 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(2)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(3)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(4)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';         -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(5)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(6)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(7) = '1'))  then
        if       Maske_Hi_Bits_Tag(7)(7)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(7) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(7) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(7)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 8 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(8) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(8)    = '1'  then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(8) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(8)    = '1'  then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bit"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(2) = true) AND   (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(2)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(3) = true) AND   (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(3)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(4) = true) AND   (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(4)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(5) = true) AND   (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(5)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(6) = true) AND   (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(6)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)       <= '1'; -- set Masken-Bit

      elsif ((New_AWOut_Data_Tag(7) = true) AND   (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(8) = '1'))  then
        if     Maske_Hi_Bits_Tag(7)(8)     = '1'   then  Tag_Out_Reg_Array(Register_Nr)(8) <= '1';    -- Set "H-Bits"
        else                                             Tag_Out_Reg_Array(Register_Nr)(8) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(8)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 9 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if     ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(0)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(1)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)       <= '1'; -- set Masken-Bit

      elsif  ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(9) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(9)    = '1'   then Tag_Out_Reg_Array(Register_Nr)(9) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(9) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(9)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 10 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(10) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(10)    = '1' then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(10) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(10)    = '1' then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(10) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(10)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(10) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(10) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(10)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 11 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(11) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(11)    = '1' then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(11) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(11)    = '1' then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bits"
        else                                                Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(11) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(11)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(11) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(11) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(11)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 12 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(12) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(12)    = '1' then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(12) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(12)    = '1' then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bit"
        else                                                Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bits"
        else                                                Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(12) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(12)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(12) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(12) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(12)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 13 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(13) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(13)    = '1' then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(13) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(13)    = '1' then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(13) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(13)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(13) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(13) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(13)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 14 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(14) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(14)    = '1' then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(14) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(14)    = '1' then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(14) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(14)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(14) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(14) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(14)     <= '1'; -- set Masken-Bit
      end if;


--  +====================================== Register n, Bit 15 ===========================================================+

       --    (---------Tag_0 = aktiv------)---------( Register=Register_Nr)-----------  -(---Bit-0 der Maske = 1)--------
      if      ((New_AWOut_Data_Tag(0) = true) AND (Reg_Nr_Tag(0) = Register_Nr) AND (Tag_Array(0)(i_Tag_Maske)(15) = '1'))  then
        if       Maske_Hi_Bits_Tag(0)(15)    = '1' then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(1) = true) AND (Reg_Nr_Tag(1) = Register_Nr) AND (Tag_Array(1)(i_Tag_Maske)(15) = '1'))  then
        if       Maske_Hi_Bits_Tag(1)(15)    = '1' then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bit"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(2) = true) AND (Reg_Nr_Tag(2) = Register_Nr) AND (Tag_Array(2)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(2)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(3) = true) AND (Reg_Nr_Tag(3) = Register_Nr) AND (Tag_Array(3)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(3)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(4) = true) AND (Reg_Nr_Tag(4) = Register_Nr) AND (Tag_Array(4)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(4)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';         -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';         -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(5) = true) AND (Reg_Nr_Tag(5) = Register_Nr) AND (Tag_Array(5)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(5)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(6) = true) AND (Reg_Nr_Tag(6) = Register_Nr) AND (Tag_Array(6)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(6)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)       <= '1'; -- set Masken-Bit

      elsif   ((New_AWOut_Data_Tag(7) = true) AND (Reg_Nr_Tag(7) = Register_Nr) AND (Tag_Array(7)(i_Tag_Maske)(15) = '1'))  then
        if      Maske_Hi_Bits_Tag(7)(15)    = '1'  then Tag_Out_Reg_Array(Register_Nr)(15) <= '1';    -- Set "H-Bits"
        else                                            Tag_Out_Reg_Array(Register_Nr)(15) <= '0';    -- Set "L-Bit"
        end if;
        Sum_Reg_MSK(Register_Nr)(15)     <= '1'; -- set Masken-Bit
      end if;

        --   END: FOR Bit_Nr in 0 to 15 loop
--  +====================================== Register n, Bit 0 ===========================================================+

      else
--    s_Tag_Aktiv         <= (others => '0'  );

    end if;
  end process P_TAG_REG;

END GENERATE;





--  +============================================================================================================================+
--  |                         Die Output-Daten vom SCU-Bus werden in die jeweiligen Register geschrieben.                        |
--  +============================================================================================================================+


P_AWOut_Reg:  process (clk, nReset, Clr_Tag_Config,
                       S_Tag_Base_0_Addr_Wr, S_Tag_Base_1_Addr_Wr, S_Tag_Base_2_Addr_Wr, S_Tag_Base_3_Addr_Wr,
                       S_Tag_Base_4_Addr_Wr, S_Tag_Base_5_Addr_Wr, S_Tag_Base_6_Addr_Wr, S_Tag_Base_7_Addr_Wr,
                       Data_from_SCUB_LA )
  begin
    if nReset = '0' then
      Tag_Array   <= (others => (others => (others => '0')));

    elsif rising_edge(clk) then

      if Clr_Tag_Config = '1' then  Tag_Array   <= (others => (others => (others => '0'))); -- Clear Tag-Konfigurations-Register
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




  P_read_mux:  process (S_Tag_Base_0_Addr_Rd, S_Tag_Base_1_Addr_Rd,
                        S_Tag_Base_2_Addr_Rd, S_Tag_Base_3_Addr_Rd,
                        S_Tag_Base_4_Addr_Rd, S_Tag_Base_5_Addr_Rd,
                        S_Tag_Base_6_Addr_Rd, S_Tag_Base_7_Addr_Rd,
                        Tag_Array, Adr_from_SCUB_LA)

  begin

--                                                                  +--- Zeilen-Nr. im Array
--                                                                  |  +---- Adresse der "Wordposition" in der Zeile ----+
--                                                                  |  |                                                 |
    if    S_Tag_Base_0_Addr_Rd = '1' then  S_Read_port <= Tag_Array(0)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_1_Addr_Rd = '1' then  S_Read_port <= Tag_Array(1)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_2_Addr_Rd = '1' then  S_Read_port <= Tag_Array(2)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_3_Addr_Rd = '1' then  S_Read_port <= Tag_Array(3)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_4_Addr_Rd = '1' then  S_Read_port <= Tag_Array(4)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_5_Addr_Rd = '1' then  S_Read_port <= Tag_Array(5)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_6_Addr_Rd = '1' then  S_Read_port <= Tag_Array(6)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));
    elsif S_Tag_Base_7_Addr_Rd = '1' then  S_Read_port <= Tag_Array(7)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0))));



  else
      S_Read_Port <= (others => '-');
    end if;
  end process P_Read_mux;



  --------- Summenbildung: Fg_Start, Fehlermeldungen und LA-Dummy  --------------------

p_summe:  PROCESS (clk, nReset, Tag_n_FG_Start, Tag_n_Reg_Err, Tag_n_Reg_max_Err,
                   Tag_n_Trig_Err, Tag_n_Trig_max_Err, Tag_n_Timeout, Tag_n_ext_Trig_Strobe,
                   Tag_LA_Dummy, LA_Tag)
  BEGin
    IF  nReset               = '0' then

      s_Tag_FG_Start        <= '0';
      Tag_Reg_Err           <= '0';
      Tag_Reg_max_Err       <= '0';
      Tag_Trig_Err          <= '0';
      Tag_Trig_max_Err      <= '0';
      Tag_Timeout           <= '0';
      Tag_ext_Trig_Strobe   <= '0';
      Tag_LA_Dummy          <= '0';

    ELSIF rising_edge(clk) THEN

      if  Tag_n_FG_Start          = x"00" then   -- Fg-Start
        s_Tag_FG_Start           <= '0';
      else
        s_Tag_FG_Start           <= '1';
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

      if ((LA_Tag(0) = x"0000") and (LA_Tag(1) = x"0000") and (LA_Tag(2) = x"0000") and (LA_Tag(3) = x"0000") and
          (LA_Tag(4) = x"0000") and (LA_Tag(5) = x"0000") and (LA_Tag(6) = x"0000") and (LA_Tag(7) = x"0000")) then   -- alle LA-Outputs = 0
        Tag_LA_Dummy             <= '0';
      else
        Tag_LA_Dummy             <= '1';
      end if;

    END IF;
  END PROCESS p_Summe;



LA_tag_ctrl  <=      (Timing_Pattern_RCV     &    Spare1_Strobe    &    Spare0_Strobe  &  Tag_ext_Trig_Strobe     &   -- Testport für Logic-Analysator
                       s_Tag_FG_Start        & '0'                 & '0'               & '0'                      &
                       Tag_Reg_Err           &  Tag_Reg_max_Err    &  Tag_Trig_Err     &  Tag_Trig_max_Err        &
                       Tag_Timeout           & '0'                 & '0'               &  Tag_LA_Dummy            );



--------- Tag-Staus --------------------

Tag_Sts      <= (x"00"              &
                  '0'               &
                  '0'               &
                  Tag_Conf_Err      &
                  Tag_Reg_Err       &
                  Tag_Trig_Err      &
                  Tag_Reg_max_Err   &
                  Tag_Trig_max_Err  &
                  Tag_Timeout
                );



Tag_Aktiv        <=    s_Tag_Aktiv;       -- Flag: Bit7 = Tag7 (aktiv) --- Bit0 = Tag0 (aktiv)
Tag_Maske_Reg    <=    Sum_Reg_MSK;       -- Tag-Output-Maske für Register 1-7
Tag_Outp_Reg     <=    Tag_Out_Reg_Array; -- Tag-Output-Register 1-7
Tag_FG_Start     <=    s_Tag_FG_Start;    -- Start-Puls für den FG

Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB <= S_Read_Port;


end Arch_tag_ctrl;
