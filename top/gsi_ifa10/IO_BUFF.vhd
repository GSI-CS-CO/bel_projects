----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IO_BUFF.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices:
-- Tool versions:
-- Description:
-- Generiert die I/O Zustände in Abhängigkeit der ausgewählten Hardware
-- SCU_Mode,MB_Mode,IFA_Mode und TEST_Mode
--
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

library work;
use work.ifk10_pkg.all;

ENTITY IO_BUFF IS

  PORT
  (
    sys_clk       : IN STD_LOGIC;
    sys_reset     : IN STD_LOGIC;

    HW_SEL_Mode   : in STD_LOGIC_VECTOR(3 DOWNTO 0);

    FC            : IN STD_LOGIC_VECTOR(7 DOWNTO 0); -- Funktionscode
    FC_Str        : IN STD_LOGIC; -- Strobe Funktionscode
    SD_me_mm      : IN STD_LOGIC; -- Send data von MIL-SD ist aktiv
    FG_DDS_STR    : IN STD_LOGIC; -- FG DDS Strobe - new data

    nIRQ          : IN STD_LOGIC;
    nDRQ          : IN STD_LOGIC;
    CLK_6MHZ      : IN STD_LOGIC;
    Pu_Reset      : IN STD_LOGIC;
    PowerUP_FlagR : IN STD_LOGIC; --active '0', set by external device to enable IFA221 outputs , only IFA Mode
    MB_A          : IN STD_LOGIC_VECTOR(4 DOWNTO 0);
    MB_SubA       : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    MB_DOut       : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    MB_SEL        : IN STD_LOGIC;
    MB_R_nW       : IN STD_LOGIC;
    MB_nDS        : IN STD_LOGIC;
    MB_SEL_WR     : IN STD_LOGIC;
    MB_CLK        : IN STD_LOGIC;
    MB_nReset     : IN STD_LOGIC;
    FB_A          : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    FB_nAdr_Sel   : IN STD_LOGIC;
    FB_nBSel      : IN STD_LOGIC_VECTOR(12 DOWNTO 1);
    FB_DOut       : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    FB_R_nW       : IN STD_LOGIC;
    FB_nDS        : IN STD_LOGIC;
    FB_Ena_Tri_Buff: IN STD_LOGIC;
    FB_nSel_Ext_Data_Drv: IN STD_LOGIC;
    FB_Ext_Data_Drv_WRnRd: IN STD_LOGIC;
    FB_nReset     : IN STD_LOGIC;
    FB_nEVT_Str   : IN STD_LOGIC;

    V_Ain         : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    V_Bin         : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    V_Cin         : in STD_LOGIC_VECTOR(32 DOWNTO 1);

    ENA_Every_20ms: IN STD_LOGIC;
    Ena_Every_100ns:IN STD_LOGIC;

    SW_in         : in  STD_LOGIC_VECTOR(15 DOWNTO 0);  --node
    SWF_in        : in  STD_LOGIC_VECTOR(7 DOWNTO 0);  --node

    --112, 122
    FG1x2_Funct   : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    FG112_DAC     : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    FG112_nACK    : in STD_LOGIC;
    FG112_nSTB    : in STD_LOGIC;

    FG1x2_Res     : in STD_LOGIC;
    FG122_addr    : in STD_LOGIC_VECTOR(7 DOWNTO 0);  -- adr-Data from VG port
    FG122_Data_out: in STD_LOGIC_VECTOR(15 DOWNTO 0);
    FG122_DBRD    : in STD_LOGIC;
    FG122_DBWR    : in STD_LOGIC;

    FG122_Res     : in STD_LOGIC;
    FG122_Funct   : in STD_LOGIC_VECTOR(7 DOWNTO 0);

    FG112_nDRDY   : in std_logic;

    SCU_Sel_Err   : in std_logic;
    MB_Sel_Err    : in std_logic;
    FB_nINL       : in std_logic;
    FB_nDRQ       : in std_logic;
    FB_nDRDY      : in std_logic;
    
       --203
    FG203_Mode    : in std_logic;      -- card 203 selected
    FG203B_Mode   : in std_logic;      -- card 203 selected with junper

   --211
   -- FG211_Mode    : in std_logic;      -- card 211 selected

    FB_nSRQ_SL    : OUT STD_LOGIC_VECTOR(12 DOWNTO 1);
    FB_DIN        : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    FB_nDTACK     : OUT STD_LOGIC;

    MB_nDTACK     : OUT STD_LOGIC;
    MB_DIN        : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    nSend_Ena     : OUT STD_LOGIC;
    nSend_Str     : OUT STD_LOGIC;

    Sel_MB        : OUT STD_LOGIC;
    Sel_FB        : OUT STD_LOGIC;

   --debounced IRQ Meldungen, DRQ, INL u. DRDY
    nDRQ_HW       : OUT STD_LOGIC;
    nDRDY_HW      : OUT STD_LOGIC;
    nINL_HW       : OUT STD_LOGIC;
    Sel_Err       : OUT STD_LOGIC;

   --
    nIACK         : OUT STD_LOGIC;
    IFA_Adr       : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    STS           : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    DIW           : OUT STD_LOGIC_VECTOR(15 DOWNTO 0); --IFA in read

    VG_nSwitch_Ena: OUT STD_LOGIC;  --switch enable bidir SWT puffer array

    VG_A_Out      : OUT STD_LOGIC;
    VG_A_nEn      : OUT STD_LOGIC;
    VG_B_Out      : OUT STD_LOGIC;
    VG_B_nEn      : OUT STD_LOGIC;
    VG_C_Out      : OUT STD_LOGIC;
    VG_C_nEn      : OUT STD_LOGIC;
    VG_D_Out      : OUT STD_LOGIC;
    VG_D_nEn      : OUT STD_LOGIC;
    VG_E_Out      : OUT STD_LOGIC;
    VG_E_nEn      : OUT STD_LOGIC;
    VG_F_Out      : OUT STD_LOGIC;
    VG_F_nEn      : OUT STD_LOGIC;
    VG_G_Out      : OUT STD_LOGIC;
    VG_G_nEn      : OUT STD_LOGIC;
    VG_I_Out      : out STD_LOGIC;
    VG_I_nEn      : out STD_LOGIC;
    VG_K_Out      : out STD_LOGIC;
    VG_K_nEn      : out STD_LOGIC;
    VG_L_Out      : out STD_LOGIC;
    VG_L_nEn      : out STD_LOGIC;
    VG_M_Out      : out STD_LOGIC;
    VG_M_nEn      : out STD_LOGIC;
    VG_N_Out      : out STD_LOGIC;
    VG_N_nEn      : out STD_LOGIC;

    X_SelDirO     : out STD_LOGIC_VECTOR(12 downto 1);
    X_EnIO        : out STD_LOGIC_VECTOR(12 downto 1);

    Testout       : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);

    A_out         : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
    B_out         : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
    C_out         : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);

    --FPGA output/input control
    A_out_en      : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
    B_out_en      : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);
    C_out_en      : OUT STD_LOGIC_VECTOR(32 DOWNTO 1);

    AP_IO         : inout STD_LOGIC_VECTOR(45 DOWNTO 0);

    n_enInputs    : OUT STD_LOGIC;
    n_enOutputs   : OUT STD_LOGIC;

    FG112_ADC     : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    FG112_nLH     : OUT STD_LOGIC;
    FG112_nOBF    : OUT STD_LOGIC;
    FG112_LnH     : OUT STD_LOGIC;
    FG112_IBF     : OUT STD_LOGIC;
    FG112_nALARM  : OUT STD_LOGIC;

    FG122_Data_in : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)

);

END IO_BUFF;



ARCHITECTURE IO_BUFF OF IO_BUFF IS

----------------------------------------------------------------

--signal Mode_Select   : STD_LOGIC_VECTOR(3 DOWNTO 0) := (others =>'0'); -- Hardwareauswahl

signal SW            : STD_LOGIC_VECTOR(15 DOWNTO 0):= (others =>'0');
signal SWF           : STD_LOGIC_VECTOR(7 DOWNTO 0):= (others =>'0');
signal IFA_In        : STD_LOGIC_VECTOR(7 DOWNTO 0) := (others =>'0'); -- Adresszwischenspeicher

signal nDBSend_Ena_in: STD_LOGIC :='1';  -- Ena send debouncer in
signal nDBSend_Str_in: STD_LOGIC :='1';  -- Ena send STR debouncer in

signal nDBSend_Ena_out: STD_LOGIC :='1';  -- Ena send debouncer in
signal nDBSend_Str_out: STD_LOGIC :='1';  -- Ena send STR debouncer in

signal Sel_MB_in     : STD_LOGIC :='0';
signal Sel_FB_in     : STD_LOGIC :='0';

signal FB_SEL        : STD_LOGIC :='1';     --war fest inifk8:ipar_a auf default='1'  --sonst Kollision in Gruppe G mit 18C

--------------
--Kontrolliertes Freigabe-Timing
signal enableshift   : STD_LOGIC_VECTOR(5 DOWNTO 0) := (others =>'0');
signal en1           : STD_LOGIC :='0';
signal en2           : STD_LOGIC :='0';

signal n_enIn        : STD_LOGIC :='1';     --enable all IOs
signal n_enOutD      : STD_LOGIC :='1';     --enable für output mit delay
--signal nSwitch_Ena   : STD_LOGIC :='1';   --enable für bidir switches mit delay

signal nPowerUP_Flag : STD_LOGIC :='1';


--debouncing IRQ+Err Signale
signal nINL_HWd      : std_logic := '1';
signal nDRQ_HWd      : std_logic := '1';
signal nDRDY_HWd     : std_logic := '1';
signal Sel_Errd      : std_logic := '0';



BEGIN

n_enInputs  <= n_enIn;  -- alle Eingänge: 0-freigabe 1- sperren
n_enOutputs <= n_enOutD;



--nIACK <= nIACK;

--Mode_Select <= HW_SEL_Mode;

SW  <= SW_in;
SWF <= SWF_in;


glb:process(sys_clk,sys_reset,HW_SEL_Mode,V_Ain,V_Bin,V_Cin,n_enOutD,nIRQ,n_enIn,FC,MB_CLK,MB_nReset,
             MB_nDS,FB_nSel_Ext_Data_Drv,FB_Ext_Data_Drv_WRnRd,MB_R_nW,MB_A,MB_SUBA,
             FB_SEL,FB_nBSel,FB_nAdr_Sel,FB_A,FB_Ena_Tri_Buff,FB_Dout,
             Pu_Reset,nDRQ,SW,SWF,PowerUP_FlagR,FC_Str,FB_R_nW,FB_nDS,FB_nReset,FB_nEVT_Str,
             SD_me_mm,MB_DOut, MB_SEL,MB_SEL_WR,FG_DDS_STR,nDBSend_Ena_out,FG122_Funct,FG122_addr,FG112_DAC,FG122_DBWR,
             FG122_Data_out,FG1x2_Funct,nPowerUP_Flag,CLK_6MHZ,FG203_Mode,FG203B_Mode,FG112_nDRDY
             )

begin


  if sys_reset ='1' then

    --default internal FPGA output Z-Buf alle Ausgänge  abgeschaltet
         A_out_en(32 downto 1)   <= (others => '0');
         B_out_en(32 downto 1)   <= (others => '0');
         C_out_en(32 downto 1)   <= (others => '0');

         --default internal IO Buf Ausgabezustände default =0
         A_out(32 downto 1)      <= (others => '0'); --alles Eingänge mit Ausgang =0
         B_out(32 downto 1)      <= (others => '0');
         C_out(32 downto 1)      <= (others => '0');

         nPowerUP_Flag <= '1';

         --nc WIRES
--A_out(2 downto 1) <= (others => '0');
--A_out(32)  <= '0';
--C_out(2)   <= '0';
--C_out(32)  <= '0';

  --default
         STS(0) <='0';--V_Ain(7);
         STS(2) <='0';--V_Ain(8);
         STS(4) <='0';--V_Ain(9);
         STS(6) <='0';--V_Ain(10);

         STS(1) <= '0'; --VG_Cin(7);
         STS(3) <= '0'; --VG_Cin(8);
         STS(5) <= '0'; --VG_Cin(9);
         STS(7) <= '0'; --VG_Cin(9);

         VG_nSwitch_Ena  <= '1'; --swt abgeschaltet

         nDBSend_Ena_in <='1';
         nDBSend_Str_in <='1';

         nSend_Ena      <='1';
         nSend_Str      <='1';

         nDRQ_HWd        <= '1';
         nINL_HWd        <= '1';
         nDRDY_HWd       <= '1';
         Sel_Errd        <= '0';
   --else
   elsif rising_edge(sys_clk) then
         --default external Drv alle als Input definiert
         VG_A_Out  <= '0'; --input
         VG_B_Out  <= '0';
         VG_C_Out  <= '0';
         VG_D_Out  <= '0';
         VG_E_Out  <= '0';
         VG_F_Out  <= '0';
         VG_G_Out  <= '0';
         VG_I_Out  <= '0';
         VG_K_Out  <= '0';
         VG_L_Out  <= '0';
         VG_M_Out  <= '0';
         VG_N_Out  <= '0';
         X_SelDirO  <= (others => '0'); --default external Drv alle abgeschaltet

         --default external Drv alle abgeschaltet
         VG_A_nEn<='1';  -- disable
         VG_B_nEn<='1';  -- disable
         VG_C_nEn<='1';  -- disable
         VG_D_nEn<='1';  -- disable
         VG_E_nEn<='1';  -- disable
         VG_F_nEn<='1';  -- disable
         VG_G_nEn<='1';  -- disable
         VG_I_nEn<='1';  -- disable
         VG_K_nEn<='1';  -- disable
         VG_L_nEn<='1';  -- disable
         VG_M_nEn<='1';  -- disable
         VG_N_nEn<='1';  -- disable
         X_EnIO     <= (others => '1'); --Disable all

         --default internal FPGA output Z-Buf alle Ausgänge  abgeschaltet
         A_out_en(32 downto 1)   <= (others => '0');
         B_out_en(32 downto 1)   <= (others => '0');
         C_out_en(32 downto 1)   <= (others => '0');

         --default internal IO Buf Ausgabezustände default =0
         A_out(32 downto 1)      <= (others => '0'); --alles Eingänge mit Ausgang =0
         B_out(32 downto 1)      <= (others => '0');
         C_out(32 downto 1)      <= (others => '0');

         --Ausnahmen hier noch egänzen

         --IFA Adresse lesen     --swt3
         IFA_In(0) <= V_Ain(3);  -- alles Eingänge
         IFA_In(2) <= V_Ain(4);
         IFA_In(4) <= V_Ain(5);
         IFA_In(6) <= V_Ain(6);

         --swt5
         IFA_In(1) <= V_Cin(3);
         IFA_In(3) <= V_Cin(4);
         IFA_In(5) <= V_Cin(5);
         IFA_In(7) <= V_Cin(6);

--    -- Feste IOs nicht verwendet/ NC
--         A_out(16)   <= '0';
--         A_out_en(16)   <= '0';
--         B_out(16)   <= '0';
--         B_out_en(16)   <= '0';
--         C_out(16)   <= '0';
--         C_out_en(16)   <= '0';
--
         VG_nSwitch_Ena  <='0';-- nSwitch_Ena; --swts einschalten

         --IRQ-default -Werte
         nDRQ_HWd        <= '1';
         nINL_HWd        <= '1';
         nDRDY_HWd       <= '1';
         Sel_Errd        <= '0';



      --===================  Hardware-SEL der Betriebsarten ================================

      --if FG203_Mode='1' or FG203B_Mode='1' then -- bei 203 gibt es noch kein MB und FB /SCU select
       --  Sel_MB_in      <= '0';
      --else
         Sel_MB_in      <= not V_Bin(7);  --SWT4  -- Achtung: Kollision mit .203 B7 als NINL
      --end if;

         X_EnIO(5)      <= '0'; -- enable Input external Drv immer an
         X_SelDirO(5)   <= '0'; -- Input

    --  if FG203_Mode='1' or FG203B_Mode='1'  then
    --     Sel_FB_in      <= '0';
      --else
         Sel_FB_in      <= not V_Bin(8);
      --end if;
            --nc WIRES
         A_out(2 downto 1) <= (others => '0');
         A_out(32)  <= '0';
         C_out(2)   <= '0';
         C_out(32)  <= '0';

         nDBSend_Ena_in <='1';
         nDBSend_Str_in <='1';

         nSend_Ena      <='1';
         nSend_Str      <='1';


    case HW_SEL_Mode is

         when C_HW_MB_MODE =>

--    (T_B[6..3],    T_B[6..3].oe)  = GND;-- unbenutzte Pins
--    (T_B[14..9],   T_B[14..9].oe) = GND;-- unbenutzte Pins
--    (T_B[32..25],  T_B[32..25].oe)   = GND;-- unbenutzte Pins
--    (T_C[16],      T_C[16].oe)    = GND;-- unbenutzte Pins
--    (T_C[17],      T_C[17].oe)    = GND;-- unbenutzte Pins
--    (T_B[26],      T_B[26].oe)    = GND;-- unbenutzte Pins
--    (T_B[27],      T_B[27].oe)    = GND;-- unbenutzte Pins
--    (T_B[32],      T_B[32].oe)    = GND;-- unbenutzte Pins
--    (T_A19S,    T_A19S.oe)     = GND;-- unbenutzte Pins
--    (T_C19S,    T_C19S.oe)     = GND;-- unbenutzte Pins

            STS(0)         <= V_Ain(7);
            STS(2)         <= V_Ain(8);
            STS(4)         <= V_Ain(9);
            STS(6)         <= V_Ain(10);

            STS(1)         <= V_Cin(7);
            STS(3)         <= V_Cin(8);
            STS(5)         <= V_Cin(9);
            STS(7)         <= V_Cin(10);

            X_EnIO(1)      <= n_enOutD;-- enable  11A
            X_SelDirO(1)   <= '1';     -- output ext. drv
            A_out(11)      <= nIRQ;
            A_out_en(11)   <= '1';     -- output

            VG_L_nEn       <= n_enIn;  -- enable
            VG_L_Out       <= '0';     -- Input ext. drv
            nIACK          <= V_Cin(11);

            VG_K_nEn       <= n_enOutD;-- enable
            VG_K_Out       <= '1';     -- output ext. drv

            A_out(12)      <= FC(0);
            A_out_en(12)   <= '1';     -- output
            A_out(14)      <= FC(4);
            A_out_en(14)   <= '1';     -- output

            C_out(12)      <= FC(1);
            C_out_en(12)   <= '1'; --output
            C_out(14)      <= FC(5);
            C_out_en(14)   <= '1'; --output
            C_out(15)      <= FC(7);
            C_out_en(15)   <= '1'; --output

            VG_I_nEn       <=  n_enOutD; -- enable
            VG_I_Out       <= '1'; -- output ext. drv
            A_out(13)      <=  FC(2);  -- ?? -> NUC ?
            A_out_en(13)   <= '1'; --output
            A_out(15)      <=  FC(6);  -- ?? -> NUC ?
            A_out_en(15)   <= '1'; --output
            C_out(13)      <=  FC(3);  -- ?? -> NUC ?
            C_out_en(13)   <= '1'; --output

            VG_G_nEn       <= n_enIn; --enable
            VG_G_Out       <= '0'; --Input
            nDRQ_HWd       <= V_Cin(18); -- nSRQ1
            nINL_HWd       <= V_Cin(20); -- MB_nINL
            MB_nDTACK      <= V_Cin(21);

            --       ??  (T_A19S,      T_A19S.oe)     = GND;-- unbenutzte Pins
            --(T_C19S,     T_C19S.oe)     = GND;-- unbenutzte Pins

            X_EnIO(3)      <= n_enIn; -- enable
            X_SelDirO(3)   <= '0'; --input
            nDRDY_HWd      <= V_Ain(17); -- nSRQ2

            VG_F_nEn       <= n_enOutD; -- enable
            VG_F_Out       <= '1';      -- Output
            C_out(19)      <= MB_CLK;
            C_out_en(19)   <= '1';      -- Output

            VG_E_nEn       <= (NOT MB_SEL) or n_enOutD;-- enable mit MB_SEL
            --VG_E_Out     <= MB_Mode;-- when MB_Mode= '1' else    'Z'; ??
            VG_E_Out       <= '1'; --vk ist immer 1 da MB_Mode hier auch immer 1
            A_out_en(21 downto 20) <= (others => '1'); --output
            A_out_en(18)   <='1'; --output
            A_out_en(19)   <='1'; --output
            --if MB_Mode='1' then --MB Mode ist hier immer 1 da if raus --issue Vergleich mit ifa8 definition
            A_out(18)      <= MB_nReset;
            A_out(19)      <= MB_nDS;
            A_out(20)      <= MB_R_nW;
            A_out(21)      <= MB_A(4);

            X_EnIO(7)      <= n_enOutD; -- enable
            X_SelDirO(7)   <= '1'; --output
            B_out_en(15)   <= '1'; --output
            B_out(15)      <= '0';  --GND explizit nochmals angegeben

            VG_C_nEn       <= (NOT MB_SEL) or n_enOutD;-- enable mit MB_SEL
            VG_C_Out       <= '1'; -- Output
            C_out_en(25 downto 22) <= (others => '1');      -- Output
            A_out_en(25 downto 22) <= (others => '1');      -- Output

            --(VG_C_Out,  T_A[22..25].oe, T_C[22..25].oe) = VCC;-- Output
            A_out(22)      <= MB_A(2);
            A_out(23)      <= MB_A(0);
            A_out(24)      <= MB_SUBA(4); --(6)? () ist in IO_BUF.tdf aufgelistet??
            A_out(25)      <= MB_SUBA(4);

            C_out(22)      <= MB_A(3);
            C_out(23)      <= MB_A(1);
            C_out(24)      <= MB_SUBA(7);
            C_out(25)      <= MB_SUBA(5);

            VG_B_nEn       <=  (NOT MB_SEL) or n_enOutD;-- enable mit MB_SEL
            VG_B_Out       <= '1'; -- Output
            C_out_en(27 downto 26) <= (others => '1');      -- Output
            A_out_en(27 downto 26) <= (others => '1');      -- Output
            A_out(26)      <= MB_SUBA(2);
            A_out(27)      <= MB_SUBA(0);
            C_out(26)      <= MB_SUBA(3);
            C_out(27)      <= MB_SUBA(1);

            VG_A_nEn       <= (NOT MB_SEL) or n_enOutD;-- enable mit MB_SEL
            VG_A_Out       <= MB_SEL_WR; -- R/W
            A_out_en(31 downto 28) <= (others => MB_SEL_WR);
            C_out_en(31 downto 28) <= (others => MB_SEL_WR);

            A_out(28)      <= MB_Dout(6);
            A_out(29)      <= MB_Dout(4);
            A_out(30)      <= MB_Dout(2);
            A_out(31)      <= MB_Dout(0);

            C_out(28)      <= MB_Dout(7);
            C_out(29)      <= MB_Dout(5);
            C_out(30)      <= MB_Dout(3);
            C_out(31)      <= MB_Dout(1);
            MB_Din(6)      <= V_Ain(28);  -- Input Daten
            MB_Din(4)      <= V_Ain(29);
            MB_Din(2)      <= V_Ain(30);
            MB_Din(0)      <= V_Ain(31);

            MB_Din(7)      <= V_Cin(28);  -- Input Daten
            MB_Din(5)      <= V_Cin(29);
            MB_Din(3)      <= V_Cin(30);
            MB_Din(1)      <= V_Cin(31);

            VG_D_nEn       <= '1';  -- disable --18B.. 24B
            VG_D_Out       <= '0'; -- input

            VG_M_nEn       <= '1'; -- disable  25B, 14B
            VG_M_Out       <= '0'; -- input  --NC

            VG_N_nEn       <= '1'; -- Disable 2B, 4B, 12B
            VG_N_Out       <= '0'; -- input  --NC

            X_EnIO(2)      <= '1'; -- disable 16A
            X_SelDirO(2)   <= '0'; --input

            X_EnIO(4)      <= '1'; -- disable 1B
            X_SelDirO(4)   <= '0';

            X_EnIO(6)      <= '1'; -- disable  13B
            X_SelDirO(6)   <= '0'; -- input

            X_EnIO(8)      <= '1'; -- disable 16B
            X_SelDirO(8)   <= '0'; --input

            --  NC
            X_EnIO(9)      <= '1';    -- disable 17B
            X_SelDirO(9)   <= '0';    --  input
            --B_out_en(17)   <= '0';    -- disable  s.o.

            X_EnIO(10)        <= '1'; -- disable 9B
            X_SelDirO(10)     <= '0'; --input

            X_EnIO(11)        <= '1'; -- disable  16C
            X_SelDirO(11)     <= '0'; --input

            X_EnIO(12)        <= '1'; -- disable 3B
            X_SelDirO(12)     <= '0'; -- Input

            Sel_Errd          <= MB_Sel_Err;


--###################################################################################
--############################ START SCU_Mode ##########################################
--###################################################################################
          when C_HW_SCU_MODE =>


--    (T_B[4],    T_B[4].oe)     = GND;-- unbenutzter Pin, FB_Master4
--    (T_B[5],    T_B[5].oe)     = GND;-- unbenutzter Pin, FB_Master7
--    (T_B[6],    T_B[6].oe)     = GND;-- unbenutzter Pin, FB_Master10
--    (T_A[7],    T_A[7].oe)     = GND;-- unbenutzter Pin, FB_Master12
--    (T_C[7],    T_C[7].oe)     = GND;-- unbenutzter Pin, FB_Master14
--    (T_A[8],    T_A[8].oe)     = GND;-- unbenutzter Pin, FB_Master15
--    (T_C[8],    T_C[8].oe)     = GND;-- unbenutzter Pin, FB_Master17
--    (T_A[9],    T_A[9].oe)     = GND;-- unbenutzter Pin, FB_Master18
--    (T_B[9],    T_B[9].oe)     = GND;-- unbenutzter Pin, FB_Master19
--
--    (T_C[9],    T_C[9].oe)     = GND;-- unbenutzter Pin, One-Wire
--
--     T_B[3]   = GND; T_B[3].oe    = VCC;-- GND vom ALTERA als Output auf die Backplane
--     T_A[14] = GND; T_A[14].oe    = VCC;-- GND vom ALTERA als Output auf die Backplane
--     T_B[14] = GND; T_B[14].oe    = VCC;-- GND vom ALTERA als Output auf die Backplane
--     T_C[14] = GND; T_C[14].oe    = VCC;-- GND vom ALTERA als Output auf die Backplane

                                       --input swt
            FB_nSRQ_SL(2)     <= V_Bin(10);  -- swt4
            FB_nSRQ_SL(10)    <= V_Bin(27);  -- swt4
            FB_nSRQ_SL(11)    <= V_Bin(29);  -- swt4
            FB_nSRQ_SL(12)    <= V_Bin(31);  -- swt4
            FB_nSRQ_SL(3)     <= V_Cin(10);  -- swt6

            FB_nSRQ_SL(1)     <= V_Ain(10);  -- swt3

            VG_F_nEn          <= n_enIn;     -- enable
            VG_F_Out          <= '0';        -- input
            FB_nSRQ_SL(8)     <= V_Cin(19);

            VG_K_nEn          <= n_enIn;     -- enable
            VG_K_Out          <= '0'; -- in
            FB_nSRQ_SL(4)     <= V_Ain(12);
            FB_nSRQ_SL(6)     <= V_Cin(12);
            FB_nSRQ_SL(7)     <= V_Cin(15);

            C_out_en(17)      <= '0';        -- Eingang
            FB_nDTACK         <= V_Cin(17);  -- swt6

            --C_out_en(14) <= '1'; C_out(14)<='0'; -- GND vom ALTERA als Output auf die Backplane K
            --A_out_en(14) <= '1'; A_out(14)<='0'; -- GND vom ALTERA als Output auf die Backplane K
            --K kann nur eingelesen, Defaults aus tdf werden nicht berücksichtigt

            VG_M_nEn          <= n_enIn; --enable
            VG_M_Out          <= '0'; -- input
            FB_nSRQ_SL(9)     <= V_Bin(25);
            --B_out_en(14) <= '1'; B_out(14)<='0'; -- GND vom ALTERA als Output auf die Backplane geht nicht in Gruppe M
            -- B14 kann nur eingelesen werden, default GND Outputs aus tdf werden nicht berücksichtigt

            VG_L_nEn          <= '0'; -- enable  11B, 26B, 28B, 30B, 32B, 11C
            VG_L_Out          <= '1'; -- Output
            B_out_en(11)      <= FB_SEL;
            B_out_en(26)      <= FB_SEL;
            B_out_en(28)      <= FB_SEL;
            B_out_en(30)      <= FB_SEL;
            B_out_en(32)      <= FB_SEL;
            C_out_en(11)      <= FB_SEL;
            B_out(11)         <= FB_nBSel(2);
            B_out(26)         <= FB_nBSel(9);
            B_out(28)         <= FB_nBSel(10);
            B_out(30)         <= FB_nBSel(11);
            B_out(32)         <= FB_nBSel(12);
            C_out(11)         <= FB_nBSel(3);

            X_EnIO(1)         <= '0';  -- enable
            X_SelDirO(1)      <= '1';     -- output
            A_out(11)         <= FB_nBSel(1);
            A_out_en(11)      <= FB_SEL;

            A_out_en(9 downto 7) <= (others => '0'); --swt3
            B_out_en(6 downto 4) <= (others => '0');  --input SWt4, N
            C_out_en(9 downto 7) <= (others => '0'); --swt5 9C 1Wire

            X_EnIO(12)        <= '0'; -- enable 3B
            X_SelDirO(12)     <= '1'; --
            B_out_en(3)       <= '1';
            B_out(3)          <= '0'; -- GND vom ALTERA als Output auf die Backplane --X12

            X_EnIO(9)         <= FB_nSel_Ext_Data_Drv or n_enOutD; -- Enable EXT_Data_Driver
            X_SelDirO(9)      <= FB_Ext_Data_Drv_WRnRd;  -- WR/nRD EXT_Data_Driver
            B_out(17)         <= FB_Dout(15);
            B_out_en(17)      <= FB_Ext_Data_Drv_WRnRd;
            FB_Din(15)        <= V_Bin(17);
            B_out(17)         <= FB_Dout(15);

            X_EnIO(3)         <= '0';-- Immer Enable bei FB_Mode
            X_SelDirO(3)      <= '1';
            A_out(17)         <= FB_nEVT_Str;
            A_out_en(17)      <= '1';

         -- VG_nC20_21_Ena =  FB_nAdr_Sel;   -- Enable bei FB_Mode
         -- Problem G-Gruppe hat auch 18C!  -OE ist abhängig von FB_SEL
            VG_G_nEn          <= '0';  -- EnnIRQable bei FB_Mode?
            VG_G_Out          <= '1'; -- Output
            --                C_out(18) <='0'; -- raus da FB_SEL statisch='1'
            C_out(20)         <= FB_A(15);
            C_out(21)         <= FB_A(13);
            C_out_en(20)      <= '1';
            C_out_en(21)      <= '1';
             --if FB_SEL= '1' then -- raus da FB_SEL statisch='1' s. Hinweise bei Signal FB_SEL
            C_out(18)         <= FB_nBSel(7);
            C_out_en(18)      <= '1';

            VG_F_nEn          <= '1'; -- disable - 19C
            VG_F_Out          <= '0'; -- input

            X_EnIO(2)         <= '1'; -- disable  --16A
            X_SelDirO(2)      <= '0'; -- input

            X_EnIO(4)         <= '1'; -- disable  1B
            X_SelDirO(4)      <= '0';

            X_EnIO(6)         <= '0'; -- enable
            X_SelDirO(6)      <= '1'; -- output
            B_out(13)         <= FB_nBSel(5);
            B_out_en(13)      <= FB_SEL;

            X_EnIO(8)         <= n_enIn;  -- enable  16B
            X_SelDirO(8)      <= '0';     -- input

            X_EnIO(10)        <= n_enIn; -- enable -9B(T_B[4],    T_B[4].oe)     = GND;-- unbenutzter Pin, FB_Master4
            X_SelDirO(10)     <= '0'; --input

            X_EnIO(11)        <= '1'; -- disable  16C
            X_SelDirO(11)     <= '0'; --input

            VG_E_nEn          <= FB_nAdr_Sel or n_enOutD; -- Enable bei FB_Mode
            VG_E_Out          <= '1'; --output
            --if FB_SEL= '1' then  -- raus da FB_SEL statisch='1'
            A_out(18)         <= FB_nReset;
            A_out_en(18)      <= '1';
            A_out(19)         <= FB_nBSel(8); -- Kollidiert mit FB-SEL! -- TRIstate out
            A_out_en(19)      <= '1';         -- (war als 'IN' in Tabelle bezeichnet -> Korrektur)
            A_out(20)         <= FB_A(14);
            A_out_en(20)      <= '1';
            A_out(21)         <= FB_A(12);
            A_out_en(21)      <= '1';

            X_EnIO(7)         <= FB_nAdr_Sel; -- Enable bei FB_Mode
            X_SelDirO(7)      <= '1'; --output
            B_out(15)         <= FB_nDS;
            B_out_en(15)      <= '1';

            VG_I_nEn          <= FB_nAdr_Sel; --I-Kollision mit bei FB_sel(13A,13C)! //FB_select erwartet immer einen Ausgang
            VG_I_Out          <= '1'; --Output  -- Enable bei FB_Mode
            C_out(13)         <= FB_nBSel(6);-- VCC;-- Output
            C_out_en(13)      <= '1';
            A_out(13)         <= FB_nBSel(4);-- VCC;-- Output
            A_out_en(13)      <= '1';
            A_out(15)         <= FB_R_nW;    --Kollision mit FB_sel()!! --nur bei  Enable bei FB_Mode braucht hier TRI-out
            A_out_en(15)      <= '1';

            VG_D_nEn          <= FB_nSel_Ext_Data_Drv;
            VG_D_Out          <= FB_Ext_Data_Drv_WRnRd; -- output
            B_out_en(24 downto 18) <= (others => FB_Ena_Tri_Buff);
            B_out(24)         <= FB_Dout(8);   -- Output Daten
            B_out(23)         <= FB_Dout(9);   -- Output Daten
            B_out(22)         <= FB_Dout(10);  -- Output Daten
            B_out(21)         <= FB_Dout(11);  -- Output Daten
            B_out(20)         <= FB_Dout(12);  -- Output Daten
            B_out(19)         <= FB_Dout(13);  -- Output Daten
            B_out(18)         <= FB_Dout(14);  -- Output Daten
            FB_DIN(14)        <= V_Bin(18);
            FB_DIN(13)        <= V_Bin(19);
            FB_DIN(12)        <= V_Bin(20);
            FB_DIN(11)        <= V_Bin(21);
            FB_DIN(10)        <= V_Bin(22);
            FB_DIN(9)         <= V_Bin(23);
            FB_DIN(8)         <= V_Bin(24);
            --FB_DIN(14 downto 8)<= V_Bin(24 downto 18); -- Input Daten

            VG_C_nEn          <= FB_nAdr_Sel; -- Enable bei FB_Mode
            VG_C_Out          <= '1';
            A_out_en(25 downto 22)  <= (others =>'1');
            C_out_en(25 downto 22)  <= (others =>'1');
            A_out(25 downto 22) <= FB_A(4) & FB_A(6) & FB_A(8) & FB_A(10);
            C_out(25 downto 22) <= FB_A(5) & FB_A(7) & FB_A(9) & FB_A(11);

            VG_B_nEn          <= FB_nAdr_Sel;-- Enable bei FB_Mode
            VG_B_Out          <= '1';
            A_out_en(27 downto 26)  <= (others =>'1');
            C_out_en(27 downto 26)  <= (others =>'1');
            A_out(27 downto 26) <= FB_A(0) & FB_A(2);
            C_out(27 downto 26) <= FB_A(1) & FB_A(3);

            VG_A_nEn          <=  FB_nSel_Ext_Data_Drv;
            VG_A_Out          <=  FB_Ext_Data_Drv_WRnRd;
            A_out_en(31 downto 28) <= (others => FB_Ena_Tri_Buff);
            C_out_en(31 downto 28) <= (others => FB_Ena_Tri_Buff);

            A_out(31 downto 28) <=  FB_Dout(0) & FB_Dout(2) & FB_Dout(4) & FB_Dout(6);  --Output Daten
            C_out(31 downto 28) <=  FB_Dout(1) & FB_Dout(3) & FB_Dout(5) & FB_Dout(7);  --Output Daten
            FB_DIN(0)         <= V_Ain(31);
            FB_DIN(2)         <= V_Ain(30);
            FB_DIN(4)         <= V_Ain(29);
            FB_DIN(6)         <= V_Ain(28);

            FB_DIN(1)         <= V_Cin(31);
            FB_DIN(3)         <= V_Cin(30);
            FB_DIN(5)         <= V_Cin(29);
            FB_DIN(7)         <= V_Cin(28);

            VG_N_nEn          <= n_enIn; -- Enable  --12B - in --4B  - in --2B  - 'Z'
            VG_N_Out          <= '0'; -- Input

            B_out(2)          <= '0';   -- falls Treiberbaustein "N" auf Ausgang, dann hier zwingend GND wegen AGND
            B_out_en(2)       <= '0';

            --B_out(4); --Master-4
            FB_nSRQ_SL(5)     <= V_Bin(12);


            Sel_Errd    <= SCU_Sel_Err;
            nINL_HWd    <= FB_nINL;
            nDRQ_HWd    <= FB_nDRQ;
            nDRDY_HWd   <= FB_nDRDY;

         --###################################################################################
         --########################## START IFA_Mode #########################################
         --###################################################################################

      when C_HW_IFA_MODE =>


                     if FG203_Mode ='1' or FG203B_Mode ='1'  then
                        nPowerUP_Flag <= '0'; --immer sofort an in 203, 211 Mode
                     else
                        nPowerUP_Flag <= PowerUP_FlagR;
                     end if;

                     nINL_HWd          <= V_Cin(1); --Interlock nINL_IFA

                     STS(0)            <= V_Ain(7);
                     STS(2)            <= V_Ain(8);
                     STS(4)            <= V_Ain(9);
                     STS(6)            <= V_Ain(10);

                     STS(1)            <= V_Cin(7);
                     STS(3)            <= V_Cin(8);
                     STS(5)            <= V_Cin(9);
                     STS(7)            <= V_Cin(10);

                     X_EnIO(1)         <= n_enOutD;-- enable
                     X_SelDirO(1)      <= '1';     -- output
                     A_out_en(11)      <= '1';
                     A_out(11)         <= nIRQ;

                     VG_L_nEn          <= n_enIn; --enable
                     VG_L_Out          <= '0'; --Input

                     nDRDY_HWd        <= V_Bin(11);--nDRDY_IFA
                     nIACK            <= V_Cin(11);

                     DIW(0)            <= V_Bin(28);
                     DIW(2)            <= V_Bin(30);

                     VG_K_nEn          <= n_enOutD; --enable
                     VG_K_Out          <= '1'; -- Output

                     A_out(12)         <= FC(0);
                     A_out_en(12)      <= '1';

                     C_out(12)         <= FC(1);
                     C_out_en(12)      <= '1';

                     A_out(14)         <= FC(4);
                     A_out_en(14)      <= '1';

                     C_out(14)         <= FC(5);
                     C_out_en(14)      <= '1';

                     C_out(15)         <= FC(7);
                     C_out_en(15)      <= '1';

                     VG_I_nEn          <= n_enOutD;-- enable
                     VG_I_Out          <= '1';     -- Output
                     A_out(15)         <= FC(6);
                     A_out_en(15)      <= '1';
                     A_out(13)         <= FC(2);
                     A_out_en(13)      <= '1';
                     C_out(13)         <= FC(3);
                     C_out_en(13)      <= '1';

                     VG_N_nEn          <= n_enOutD; -- enable
                     VG_N_Out          <= '1'; -- output

                     B_out(2)          <= '0'; --ser out+ (I2C)ENTITY IO_BUFF IS ??
                     B_out_en(2)       <= '1';
                     --Fehler im tdf- file T_B[4]                  = (FC_StR_Piggy);          T_B[3].oe                 = VCC;-- Output
                     --OE ist Bit 3? aber Ausgang bit 4 !
                     B_out(4)          <= FC_Str;--FC_StR_Piggy; --Piggy brauchts nicht mehr - FC_Str wird dafür genommen
                     B_out_en(4)       <= '1';

                     B_out(12)         <= not Pu_Reset; -- eigentlich sys_reset wird von locked PLL generiert
                     B_out_en(12)      <= '1';


                     nDBSend_Ena_in    <= V_Bin(5);   -- swt4
                     nSend_Ena         <= nDBSend_Ena_out;

                     nDBSend_Str_in    <= V_Bin(6);   -- swt4
                     nSend_Str         <= nDBSend_Str_out;

                     X_EnIO(10)        <= n_enOutD;   -- enable
                     X_SelDirO(10)     <= '1';        -- output
                     B_out(9)          <= SD_me_mm;
                     B_out_en(9)       <= '1';

                     nDRQ_HWd          <= V_Bin(10);  -- swt4 nDRQ_IFA

                     X_EnIO(6)         <= n_enOutD;   -- enable
                     X_SelDirO(6)      <= '1';        -- output
                     B_out(13)         <= FC_Str;     -- Strobe Funktionscode
                     B_out_en(13)      <= '1';

                     DIW(1)            <= V_Bin(29);  -- swt 4
                     DIW(3)            <= V_Bin(31);

                     X_EnIO(12)        <= n_enOutD or nPowerUP_Flag;  -- enable
                     X_SelDirO(12)     <= '1';        -- output
                     B_out_en(3)       <= '1';
                     IF nPowerUP_Flag = '1' THEN
                        B_out(3) <= '0';              -- Daten = "0" wenn nPowerUP_Flag noch gesetzt ist
                     ELSE
                        B_out(3) <= FG_DDS_STR;       -- Stobe FG_DDS out
                     end if;

                     VG_M_nEn          <= n_enOutD or nPowerUP_Flag;  -- enable -25B, 14B
                     VG_M_Out          <= '1';        -- Output
                     B_out_en(25)      <= '1';

                     IF nPowerUP_Flag = '1' THEN
                        B_out(25)  <= '0';            -- Daten = "0" wenn nPowerUP_Flag noch gesetzt ist
                     ELSE
                        B_out(25)  <=  SWF(7);
                     END IF;

                     B_out(14)         <= CLK_6MHZ;
                     B_out_en(14)      <= '1';

                     X_EnIO(11)        <= '1'; -- disable --16C
                     X_SelDirO(11)     <= '0'; --input

                     X_EnIO(3)         <= '1'; -- disable 17A
                     X_SelDirO(3)      <= '1'; -- output

                     X_EnIO(9)         <= '1'; -- disable 17B
                     X_SelDirO(9)      <= '0'; -- input
                     --wohin ??

                     VG_G_nEn          <= n_enOutD or nPowerUP_Flag; --enable  --NOT v_IFA_Mode;
                     VG_G_Out          <= '1'; --out
                     C_out_en(21  downto 20) <=  (others => '1');
                     C_out_en(18)            <= '1';

                     IF nPowerUP_Flag = '1' THEN
                        C_out(21  downto 20) <=  (others => '0'); -- Daten = "0" wenn nPowerUP_Flag gesetzt
                        C_out(18)      <= '0';
                     ELSE
                        C_out(21  downto 20) <= SW(7) & SW(5);
                        C_out(18)      <= SW(1);
                     END IF;

                     X_EnIO(7)         <= n_enOutD; -- enable
                     X_SelDirO(7)      <= '1'; --output
                     B_out_en(15)      <= '1';
                     B_out(15)         <= not nDRQ;

                     VG_F_nEn          <= n_enOutD or nPowerUP_Flag; -- enable
                     VG_F_Out          <= '1'; --OUTPUT
                     C_out_en(19)      <= '1';

                     IF nPowerUP_Flag = '1' THEN
                       C_out(19)  <= '0'; -- Daten = "0" wenn nPowerUP_Flag gesetzt
                     ELSE
                       C_out(19)  <= SW(3);
                     END IF;

                     VG_E_nEn          <=  n_enOutD or nPowerUP_Flag;  -- enable bei IFA_Mode und (NOT v_IFA_Mode) --issue vk Test mit ifk8
                     --VG_E_Out    <= v_IFA_Mode; --vk hier eigentlich 1 , da bei HW_SEL_Mode immer v_IFA_Mode=1
                     VG_E_Out          <= '1'; --ist immer 1
                     --Vergleich mit tdf? nochmal
                     A_out_en(21  downto 18) <=  (others => '1');

                     IF nPowerUP_Flag = '1' THEN
                        A_out(21 downto 18)  <=  (others => '0'); -- Daten = "0" wenn nPowerUP_Flag gesetzt
                     ELSE
                        A_out(18)  <= SW(0);
                        A_out(19)  <= SW(2);
                        A_out(21 downto 20)  <=  SW(6) & SW(4);
                     END IF;

                     VG_D_nEn          <= n_enOutD or nPowerUP_Flag; -- enable
                     VG_D_Out          <= '1';   --output
                     B_out_en(24  downto 18) <=  (others => '1');

                     IF nPowerUP_Flag = '1' THEN
                        B_out(24 downto 18)    <=  (others => '0'); -- Daten = "0" wenn nPowerUP_Flag gesetzt
                     ELSE
                        B_out(24 downto 18)    <=  SWF(6 downto 0); --??
                     END IF;

                     VG_C_nEn          <= n_enOutD or nPowerUP_Flag; -- enable
                     VG_C_Out          <= '1'; --output
                     A_out_en(25  downto 22) <=  (others => '1');
                     C_out_en(25  downto 22) <=  (others => '1');

                     IF nPowerUP_Flag = '1' THEN
                        A_out(25 downto 22) <= (others => '0');    -- Daten = "0" wenn nPowerUP_Flag gesetzt
                        C_out(25 downto 22) <= (others => '0');
                     ELSE
                        A_out(25 downto 22) <= SW(14) & SW(12) &  SW(10) & SW(8);
                        C_out(25 downto 22) <= SW(15) & SW(13) &  SW(11) &  SW(9);
                     END IF;

                     VG_B_nEn          <= n_enIn;-- enable
                     VG_B_Out          <= '0'; --input

                     DIW(4)            <= V_Ain(26);
                     DIW(6)            <= V_Ain(27);
                     DIW(5)            <= V_Cin(26);
                     DIW(7)            <= V_Cin(27);

                     VG_A_nEn          <= n_enIn;  --13A ..28A, 31C .. 28C
                     VG_A_Out          <= '0';  --IN
                     A_out_en(31  downto 28) <=  (others => '0');
                     C_out_en(31  downto 28) <=  (others => '0');
                     DIW(8)            <= V_Ain(28);
                     DIW(10)           <= V_Ain(29);
                     DIW(12)           <= V_Ain(30);
                     DIW(14)           <= V_Ain(31);
                     DIW(9)            <= V_Cin(28);
                     DIW(11)           <= V_Cin(29);
                     DIW(13)           <= V_Cin(30);
                     DIW(15)           <= V_Cin(31);

                     X_EnIO(8)         <= n_enIn; -- enable
                     X_SelDirO(8)      <= '0'; --input
                     B_out(16)         <= '0';


   when C_HW_112_MODE =>

                     nDRDY_HWd         <= FG112_nDRDY;
                     nINL_HWd          <= V_Cin(1); -- in fixed nINL_IFA

                     STS(0)            <= V_Ain(7);
                     FG112_IBF         <= V_Ain(7);  -- _IBF für Verarbeitung der IO-Register
                     nDRQ_HWd          <= V_Ain(7);  -- _nDRQ für Meldung nach extern

                     STS(2)            <= V_Ain(8);
                     STS(4)            <= V_Ain(9);
                     STS(6)            <= V_Ain(10);

                     STS(1)            <= V_Cin(7);
                     STS(3)            <= V_Cin(8);
                     STS(5)            <= V_Cin(9);
                     STS(7)            <= V_Cin(10);

                     --A11 NC--
                     --B11 C11 NC

                     VG_K_nEn          <= n_enOutD; --enable
                     VG_K_Out          <= '1'; -- Output

                     A_out(12)         <= FG1x2_Funct(0);
                     A_out_en(12)      <= '1';

                     C_out(12)         <= FG1x2_Funct(1);
                     C_out_en(12)      <= '1';

                     A_out(14)         <= FG1x2_Funct(4);
                     A_out_en(14)      <= '1';

                     C_out(14)         <= FG1x2_Funct(5);
                     C_out_en(14)      <= '1';

                     C_out(15)         <= FG1x2_Funct(7);  --FG1x2_RST
                     C_out_en(15)      <= '1';

                     VG_I_nEn          <= n_enOutD;-- enable
                     VG_I_Out          <= '1';     -- Output
                     A_out(15)         <= FG1x2_Funct(6);
                     A_out_en(15)      <= '1';
                     A_out(13)         <= FG1x2_Funct(2);
                     A_out_en(13)      <= '1';
                     C_out(13)         <= FG1x2_Funct(3);
                     C_out_en(13)      <= '1';

                     VG_M_nEn          <= '1';-- disable --25B, 14B
                     VG_M_Out          <= '0';-- input
                     B_out_en(25)      <= '0';
                     B_out_en(14)      <= '0';

                     X_EnIO(11)        <= '0'; -- enable --16C
                     X_SelDirO(11)     <= '1'; -- output
                     C_out(16)         <= FG112_nACK;
                     C_out_en(16)      <= '1';

                     X_EnIO(3)         <= '0'; -- enable 17A
                     X_SelDirO(3)      <= '1'; -- output
                     A_out(17)         <= FG112_nSTB;
                     A_out_en(17)      <= '1';

                     X_EnIO(8)         <= n_enIn; -- enable 16B
                     X_SelDirO(8)      <= '0'; --input
                     FG112_nOBF        <= V_Bin(16);
                     B_out(16)         <= '0';
                     B_out_en(16)      <= '0';

                     X_EnIO(9)         <= n_enIn; -- enable 17B
                     X_SelDirO(9)      <= '0'; -- input
                     FG112_LnH         <= V_Bin(17);
                     B_out(17)         <= '0';
                     B_out_en(17)      <= '0';

                     VG_E_nEn          <=  n_enOutD;
                     VG_E_Out          <= '1'; --ist immer 1
                     A_out_en(21  downto 18) <=  (others => '1');

                     A_out(18)         <= FG112_DAC(0);
                     A_out(19)         <= FG112_DAC(2);
                     A_out(21 downto 20)  <=  FG112_DAC(6) & FG112_DAC(4);

                     VG_G_nEn          <= n_enOutD; --enable
                     VG_G_Out          <= '1'; --out
                     C_out_en(21  downto 20) <=  (others => '1');
                     C_out_en(18)      <= '1';

                     C_out(21  downto 20) <= FG112_DAC(7) & FG112_DAC(5);
                     C_out(18)         <= FG112_DAC(1);

                     VG_F_nEn          <= n_enOutD; -- enable
                     VG_F_Out          <= '1'; --OUTPUT
                     C_out_en(19)      <= '1';
                     C_out(19)         <= FG112_DAC(3);

                     VG_B_nEn          <= n_enIn;-- enable
                     VG_B_Out          <= '0'; --input

                     FG112_ADC(0)      <= V_Ain(26);
                     FG112_ADC(2)      <= V_Ain(27);
                     FG112_ADC(1)      <= V_Cin(26);
                     FG112_ADC(3)      <= V_Cin(27);

                     VG_A_nEn          <= n_enIn;  --13A ..28A, 31C .. 28C
                     VG_A_Out          <= '0';  --IN
                     A_out_en(31  downto 28) <=  (others => '0');
                     C_out_en(31  downto 28) <=  (others => '0');
                     FG112_ADC(4)      <= V_Ain(28);
                     FG112_ADC(6)      <= V_Ain(29);
                     FG112_ADC(5)      <= V_Cin(28);
                     FG112_ADC(7)      <= V_Cin(29);

                     --16A
                     X_EnIO(2)         <= n_enIn; -- enable  --16A
                     X_SelDirO(2)      <= '0'; -- input
                     FG112_nLH         <= V_Ain(16);
                     A_out(16)         <= '0';
                     A_out_en(16)      <= '0';

                     --13B
                     X_EnIO(6)         <= '0'; -- enable  --13B
                     X_SelDirO(6)      <= '1'; -- input
                     FG112_nALARM      <= V_Bin(13);
                     B_out(13)         <= '0';
                     B_out_en(13)      <= '0';


        when C_HW_122_MODE =>

                     nINL_HWd          <= V_Cin(1);  --nINL_IFA

                     STS(0)            <= V_Ain(7);
                     --FG112_IBF         <= V_Ain(7);
                     STS(2)            <= V_Ain(8);
                     STS(4)            <= V_Ain(9);
                     STS(6)            <= V_Ain(10);

                     STS(1)            <= V_Cin(7);
                     STS(3)            <= V_Cin(8);
                     STS(5)            <= V_Cin(9);
                     STS(7)            <= V_Cin(10);

                     --A11 NC--
                     --B11 C11 NC

                     VG_K_nEn          <= n_enOutD; --enable
                     VG_K_Out          <= '1'; -- Output

                     A_out(12)         <= FG122_Funct(0);
                     A_out_en(12)      <= '1';

                     C_out(12)         <= FG122_Funct(1);
                     C_out_en(12)      <= '1';

                     A_out(14)         <= FG122_Funct(4);
                     A_out_en(14)      <= '1';

                     C_out(14)         <= FG122_Funct(5);
                     C_out_en(14)      <= '1';

                     C_out(15)         <= FG122_Funct(7);  --FG1x2_RST
                     C_out_en(15)      <= '1';

                     VG_I_nEn          <= n_enOutD;-- enable
                     VG_I_Out          <= '1';     -- Output
                     A_out(15)         <= FG122_Funct(6);
                     A_out_en(15)      <= '1';
                     A_out(13)         <= FG122_Funct(2);
                     A_out_en(13)      <= '1';
                     C_out(13)         <= FG122_Funct(3);
                     C_out_en(13)      <= '1';

                     VG_D_nEn          <= n_enOutD; --enable
                     VG_D_Out          <= '1'; -- output
                     B_out_en(24 downto 18)  <= (others => '1');
                     B_out(24)         <= FG122_addr(6);   -- Output addressbus
                     B_out(23)         <= FG122_addr(5);
                     B_out(22)         <= FG122_addr(4);
                     B_out(21)         <= FG122_addr(3);
                     B_out(20)         <= FG122_addr(2);
                     B_out(19)         <= FG122_addr(1);
                     B_out(18)         <= FG122_addr(0);

                     VG_M_nEn          <= n_enOutD;-- or nPowerUP_Flag;  -- enable -25B, 14B
                     VG_M_Out          <= '1';        -- Output
                     B_out_en(25)      <= '1';
                     B_out_en(14)      <= '1';
                     B_out(25)         <= FG122_addr(7);
                     B_out(14)         <= not FG122_DBRD;      --nRD Puls

                     X_EnIO(11)        <= '1'; -- disable --16C
                     X_SelDirO(11)     <= '0'; -- input
                     C_out_en(16)      <= '0';

                     X_EnIO(3)         <= '1'; -- disable 17A
                     X_SelDirO(3)      <= '0'; -- input
                     A_out_en(17)      <= '0';

                     X_EnIO(8)         <= n_enOutD;-- enable 16B
                     X_SelDirO(8)      <= '1';     -- output
                     B_out_en(16)      <= '1';
                     B_out(16)         <= not FG122_Res; --nDB-Reset

                     X_EnIO(9)         <= '1';     -- disable 17B
                     X_SelDirO(9)      <= '0';     -- input
                     B_out_en(17)      <= '0';

                     VG_A_nEn          <= '1';     -- disable 31A ..28A , 31C.. 28C
                     VG_A_Out          <= '0';     -- input
                     A_out_en(31 downto 28) <= (others => '0');
                     C_out_en(31 downto 28) <= (others => '0');

                     -- Data IO
                     VG_E_nEn          <= n_enOutD or n_enIn;  --enable
                     VG_E_Out          <= FG122_DBWR;
                     A_out_en(21  downto 18) <=  (others => FG122_DBWR);

                     A_out(18)         <= FG122_Data_out(0);
                     A_out(19)         <= FG122_Data_out(2);
                     A_out(21 downto 20)  <=  FG122_Data_out(6) & FG122_Data_out(4);


                     VG_C_nEn          <= n_enOutD or n_enIn;
                     VG_C_Out          <= FG122_DBWR;
                     A_out_en(25 downto 22)  <= (others =>FG122_DBWR);
                     C_out_en(25 downto 22)  <= (others =>FG122_DBWR);

                     A_out(25 downto 22) <= FG122_Data_out(14) & FG122_Data_out(12) & FG122_Data_out(10) & FG122_Data_out(8);
                     C_out(25 downto 22) <= FG122_Data_out(15) & FG122_Data_out(13) & FG122_Data_out(11) & FG122_Data_out(9);

                     VG_G_nEn          <= n_enOutD or n_enIn; --enable
                     VG_G_Out          <= FG122_DBWR;
                     C_out_en(21  downto 20) <=  (others => FG122_DBWR);
                     C_out_en(18)      <= FG122_DBWR;
                     C_out(21  downto 20) <= FG122_Data_out(7) & FG122_Data_out(5);
                     C_out(18)            <= FG122_Data_out(1);

                     VG_F_nEn          <= n_enOutD or n_enIn; -- enable
                     VG_F_Out          <= FG122_DBWR; --OUTPUT
                     C_out_en(19)      <= FG122_DBWR;
                     C_out(19)         <= FG122_Data_out(3);

                     FG122_Data_in(0)  <= V_Ain(18);
                     FG122_Data_in(1)  <= V_Cin(18);
                     FG122_Data_in(2)  <= V_Ain(19);
                     FG122_Data_in(3)  <= V_Cin(19);
                     FG122_Data_in(4)  <= V_Ain(20);
                     FG122_Data_in(5)  <= V_Cin(20);
                     FG122_Data_in(6)  <= V_Ain(21);
                     FG122_Data_in(7)  <= V_Cin(21);
                     FG122_Data_in(8)  <= V_Ain(22);
                     FG122_Data_in(9)  <= V_Cin(22);
                     FG122_Data_in(10) <= V_Ain(23);
                     FG122_Data_in(11) <= V_Cin(23);
                     FG122_Data_in(12) <= V_Ain(24);
                     FG122_Data_in(13) <= V_Cin(24);
                     FG122_Data_in(14) <= V_Ain(25);
                     FG122_Data_in(15) <= V_Cin(25);

                     --/Data IO

                     --nWR Puls
                     X_EnIO(7)         <= n_enOutD; -- enable 15B
                     X_SelDirO(7)      <= '1'; -- output
                     B_out_en(15)      <= '1';
                     B_out(15)         <= not FG122_DBWR;


                     -- NC 27A,26A,27C,26C
                     VG_B_nEn          <= '1';-- disable
                     VG_B_Out          <= '0'; --input
                     A_out_en(27 downto 26)  <= (others =>'0');
                     C_out_en(27 downto 26)  <= (others =>'0');

                     --NC 16A
                     X_EnIO(2)         <= '1'; -- disable  --16A
                     X_SelDirO(2)      <= '0'; -- input
                     A_out_en(16)      <= '0';

                     --NC 13B
                     X_EnIO(6)         <= '1'; -- disable  --13B
                     X_SelDirO(6)      <= '0'; -- input
                     B_out_en(13)      <= '0';

                     --26B, 28B,30B,32B,11B,11C -- nur 26B wird weiter verwendet
                     VG_L_nEn          <= '0'; --enable
                     VG_L_Out          <= '0'; --Input
                     B_out_en(32)      <= '0';
                     B_out_en(30)      <= '0';
                     B_out_en(28)      <= '0';
                     B_out_en(26)      <= '0';
                     B_out_en(11)      <= '0';
                     C_out_en(11)      <= '0';
                     nDRQ_HWd          <= V_Bin(26); --FG122_nSRQ1

                     --SWT4
                     nDRDY_HWd         <= V_Bin(27); --FG122_nSRQ2


         when C_HW_TEST_MODE  =>       --alles auf Eingang schalten

                     --default external drv alle eingeschaltet
                     VG_A_nEn<='0';
                     VG_B_nEn<='0';
                     VG_C_nEn<='0';
                     VG_D_nEn<='0';
                     VG_E_nEn<='0';
                     VG_F_nEn<='0';
                     VG_G_nEn<='0';
                     VG_I_nEn<='0';
                     VG_K_nEn<='0';
                     VG_L_nEn<='0';
                     VG_M_nEn<='0';
                     VG_N_nEn<='0';
                     X_EnIO     <= (others => '0');

                     --default external drv alle als Input definiert
                     VG_A_Out  <= '0'; --input
                     VG_B_Out  <= '0';
                     VG_C_Out  <= '0';
                     VG_D_Out  <= '0';
                     VG_E_Out  <= '0';
                     VG_F_Out  <= '0';
                     VG_G_Out  <= '0';
                     VG_I_Out  <= '0';
                     VG_K_Out  <= '0';
                     VG_L_Out  <= '0';
                     VG_M_Out  <= '0';
                     VG_N_Out  <= '0';
                     X_SelDirO  <= (others => '0');

                     --default internal IO Buf Ausgabezustände default =0
                     A_out(32 downto 1)      <= (others => '0'); --alles Eingänge mit Ausgang =0
                     B_out(32 downto 1)      <= (others => '0');
                     C_out(32 downto 1)      <= (others => '0');


         when C_HW_TEST_MODEZ  =>

                     --default external drv alle abgeschaltet
                     VG_A_nEn<='1';
                     VG_B_nEn<='1';
                     VG_C_nEn<='1';
                     VG_D_nEn<='1';
                     VG_E_nEn<='1';
                     VG_F_nEn<='1';
                     VG_G_nEn<='1';
                     VG_I_nEn<='1';
                     VG_K_nEn<='1';
                     VG_L_nEn<='1';
                     VG_M_nEn<='1';
                     VG_N_nEn<='1';
                     X_EnIO <= (others => '0');

                     --default internal FPGA output Z-Buf Ausgänge abgeschaltet
                     A_out_en(32 downto 1)   <= (others => '0');
                     B_out_en(32 downto 1)   <= (others => '0');
                     C_out_en(32 downto 1)   <= (others => '0');

         when others =>
                  --s.o. Default-Werte
      end case;

   end if; -- else If sys-clk

end process;


Testout<=(others =>'0');

--------------------------------------------
-- debounce hw select
-- replace of module 2xdebounce in ifpar_a.bdf

DbMB: Debounce
GENERIC MAP(cnt => 4
         )
  Port MAP (
      sig      => Sel_MB_in,
      sel      =>'1',
      cnt_en   => ENA_Every_20ms,
      clk      => sys_clk,
      res      => sys_reset,
      sig_deb  => Sel_MB
   );


DbFB: Debounce
GENERIC MAP(cnt => 4
         )
  Port MAP (
      sig      => Sel_FB_in,
      sel      =>'1',
      cnt_en   => ENA_Every_20ms,
      clk      => sys_clk,
      res      => sys_reset,
      sig_deb  => Sel_FB
   );

-----------------------
--debounce HW-IRQs
-- extener schaltplan nach hier umgezogen un zu VHDL
b2v_20 : debounce
GENERIC MAP(cnt => 4
         )
PORT MAP(sig => nINL_HWd,
       cnt_en => '1',
        sel => '1',
       clk => sys_clk,
       res => sys_reset,
       sig_deb => nINL_HW);


b2v_21 : debounce
GENERIC MAP(cnt => 4
         )
PORT MAP(sig => nDRQ_HWd,
       cnt_en => '1',
       sel => '1',
       clk => sys_clk,
       res => sys_reset,
       sig_deb => nDRQ_HW);


b2v_22 : debounce
GENERIC MAP(cnt => 4
         )
PORT MAP(sig => nDRDY_HWd,
       cnt_en => '1',
       sel => '1',
       clk => sys_clk,
       res => sys_reset,
       sig_deb => nDRDY_HW);



b2v_23 : debounce
GENERIC MAP(cnt => 4
         )
PORT MAP(sig => Sel_Errd,
       cnt_en => '1',
       sel => '1',
       clk => sys_clk,
       res => sys_reset,
       sig_deb => Sel_Err);

--------------------------------------------
-- debounce send and enable str
-- replace of module bm_synch in ifa8

DbSendStr: Debounce
GENERIC MAP(cnt => 4
         )
  Port MAP (
      sig      => nDBSend_Str_in,
      sel      =>'1',
      cnt_en   => Ena_Every_100ns,
      clk      => sys_clk,
      res      => sys_reset,
      sig_deb  => nDBSend_Str_out
   );


DbnSendEna: Debounce
GENERIC MAP(cnt => 4
         )
  Port MAP (
      sig      => nDBSend_Ena_in,
      sel      =>'1',
      cnt_en   => Ena_Every_100ns,
      clk      => sys_clk,
      res      => sys_reset,
      sig_deb  => nDBSend_Ena_out
   );

   -------------------------------------------

-- IFK address Deboucer
Debgen:
for I in 0 to 7 generate
 Debonc:   Debounce
         GENERIC MAP(cnt => 4 )
         port map(IFA_In(I), '1', ENA_Every_20ms,sys_clk,sys_reset, IFA_Adr(I));
 end generate Debgen;

---------------------------------------------

-- moved mux to here from external btrf


---------------------------------------------

--Schieberegister schaltet nacheinander die generellen IO-Freigaben nach Reset ein
-- Reihenfolge
--1. bidir shift Register - ist meistens Input
--2. alle diskreten Inputs
--3. alle diskreten Outputs
enableon_que:process(sys_clk,sys_reset,enableshift,en1,en2,Ena_Every_100ns,HW_SEL_Mode)

 begin

   if sys_reset = '1' then
      enableshift <= (others => '0');
      en1         <= '0';
      en2         <= '0';
      --nSwitch_Ena <= '1';
      n_enIn      <= '1';
      n_enOutD    <= '1';
   elsif rising_edge(sys_clk) then

      en1<=Ena_Every_100ns;
      en2<=en1;
      if en1='1' and en2='0' then -- Flanke -> dann shift
         enableshift<= enableshift(4 DOWNTO 0) & '1';
        -- nSwitch_Ena<= not enableshift(1); --als erstes bidir Switches
         n_enIn     <= not enableshift(2);  --dann weitere Inputs freischalten
         if HW_SEL_Mode=C_HW_TEST_MODE then
            n_enOutD   <= '1'; --Outputs bleiben aus
         else
            n_enOutD   <= not enableshift(3);  --dann Outputs freischalten
         end if;
      end if;
   end if;

 end process;

-------------------------------------------


--Dummy Piggy bisher nicht vorgesehen
AP_IO <=   (others => 'Z') ;

END IO_BUFF;
