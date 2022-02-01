----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:    20:41:09 10.04.2019
-- Design Name:
-- Module Name:    IFA10.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices:
-- Tool versions:
-- Description:
--
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:

--inputs
--  pmoda_io(0); -- ESC ENCODER SHIFT CLOCK Bitclock from MIL device
--  pmoda_io(1); -- SDO SERIAL DATA OUT from MIL device
--  pmoda_io(2); -- VW VALID WORD
--  pmoda_io(3); -- CDS COMMAND/DATA SYNC
--  pmoda_io(4); -- BOI
--  pmoda_io(5); -- TD TAKE DATA
--  pmoda_io(6); -- SD If HIGH 16 tx-bits should be txmited to SDI
--  pmoda_io(7); -- DSC DECODER SHIFT CLOCK Bitclock from MIL device

--  pmodb_io(7) --  BZI

--
--outputs
--  pmodb_io(0) -- 75472 Driver enable
--  pmodb_io(1) -- nIN   1-disable read input, 0- enable read input schmitttrigger device 7B3T3245
--  pmodb_io(2) -- SS    data type sync flag setup
--  pmodb_io(3) -- SDI   COMMAND/DATA to TX
--  pmodb_io(4) -- nMil_Out_Pos
--  pmodb_io(5) -- nMil_Out_Neg --continously clk out, (if available)
--  pmodb_io(6) -- EE    start encoding



-- user_dipsw   -- IFK Adresse

----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

--LIBRARY altera_mf;
--USE altera_mf.altera_mf_components.all;

library work;
use work.ifk10_pkg.all;


entity ifa10 is

  Port (

    A_CLK_24MHz     : in   STD_LOGIC;  --externe CLK Quellen
    A_CLK_50MHz     : in   STD_LOGIC;
    A_EXT_CLK       : in   STD_LOGIC;

    -----------------------------
    ---MIL-Baustein

    A_ME_12MHZ      : out  STD_LOGIC;  --Clk für MIL-Baustein 12MHz
    A_ME_UDI        : out  STD_LOGIC;  --Configuration single wire oder differentiell wire am MIL-Baustein

    --Decodersteuerung
    A_ME_DSC        : in   STD_LOGIC;  --Data-Shift-Clock RX ca 1MHz
    A_ME_SDO        : in   STD_LOGIC;  --RX serielle Daten MIL-Wort
    A_ME_VW         : in   STD_LOGIC;  --Daten gueltig
    A_ME_CDS        : in   STD_LOGIC;  --Datenwort oder Controlwort empfangen
    A_ME_TD         : in   STD_LOGIC;  --Take Data

    A_ME_BZI        : out  STD_LOGIC;  --Zum MIL- Decoder
    A_ME_BOI        : out  STD_LOGIC;  --Zum MIL- Decoder

    --Encodersteuerung
    A_ME_ESC        : in   STD_LOGIC;  --Encoder-Shift-Clock ca. 1MHz -> Gibt Encoder vor
    A_ME_SDI        : out  STD_LOGIC;  --TX MIL-Datenwort
    A_ME_EE         : out  STD_LOGIC;  --Encoder enable
    A_ME_SS         : out  STD_LOGIC;  --sync type select 0-Data/1-Control
    A_ME_SD         : in   STD_LOGIC;  --Force send data
    A_ME_nBOO       : in   STD_LOGIC;  --weiterleiten zum MIL-BUs
    A_ME_nBZO       : in   STD_LOGIC;  --weiterleiten zum MIL-BUs

   --- MIL-BUS
   --Eingang
    A_MIL1_BOI      : in   STD_LOGIC;  --Eingang vom MIL-BUs
    A_MIL1_BZI      : in   STD_LOGIC;  --Eingang vom MIL-BUs
    A_MIL1_nIN_Ena  : out  STD_LOGIC;  --MIL-Eingang freischalten

    --Ausgang Treiber MIL-Bus
    A_MIL1_nBZO     : out  STD_LOGIC;  --zu Treiber MIL-BUS
    A_MIL1_nBOO     : out  STD_LOGIC;  --zu Treiber MIL-BUS
    A_MIL1_OUT_Ena  : out  STD_LOGIC;  --Treiber einschalten

    ---------------------------------------------
    A_UMIL5V        : in   STD_LOGIC;  --5V am MIL-Treiber
    A_UMIL15V       : in   STD_LOGIC;  --15V am MIL-Treiber

    A_nSEL_6408     : in   STD_LOGIC;  -- Select VHDL- oder Hardware-MIL-Encoder/decoder: 0-6408  1-VHDL
                                       -- set Jumper for use of 6408
    --Ansteuerung Optokoppler
    A_nOPK_INL      : out  STD_LOGIC;  --
    A_nOPK_DRQ      : out  STD_LOGIC;  -- Data Request
    A_nOPK_DRDY     : out  STD_LOGIC;  -- Data Ready

    --Ansteuerung externer serieller Speicher
    FS_DATA         : in   STD_LOGIC;  --
    FS_DCLK         : out  STD_LOGIC;
    FS_nCS          : out  STD_LOGIC;
    FS_ASDI         : out  STD_LOGIC;

   --Piggy Anschlüsse
    A_P             : inout STD_LOGIC_VECTOR (45 downto 0);

    --Offene Testpunkte
    SPARE           : out  STD_LOGIC_VECTOR (11 downto 1);

    -- Zum Testport
    A_Test          : out  STD_LOGIC_VECTOR (15 downto 0);
    OE_Test         : out  STD_LOGIC;  --Leitung nicht angeschlossen ehemals freigabe der testpuffer am testport default nun ON

   --vom Hex-Auswahlschalter
    A_SEL_B         : in   STD_LOGIC_VECTOR (3 downto 0);

    -- Einlesen der MIL-BUS Control
    IN_NINL         : in   STD_LOGIC;
    IN_NRDY         : in   STD_LOGIC;
    IN_NDRQ         : in   STD_LOGIC;

    --- Externe Anschlüsse
    LEMO_OUT        : out  STD_LOGIC;
    LEMO_IN         : in   STD_LOGIC;

    --LED Ansteuerungen
      --LED Frontseite
    A_NLED_EVT_INR   : out  STD_LOGIC;
    A_nLED_EXTCLK    : out  STD_LOGIC;

    A_nLED           : out STD_LOGIC_VECTOR (22 downto 0);
    nUSER_LD_LED     : out  STD_LOGIC;
    nFAILSAVE_LD_LED : out  STD_LOGIC;

    --LED on Board
    SHOW_CONFIG      : OUT STD_LOGIC;

------------------------------------------
    --VGA IOs
    V_A              : inout STD_LOGIC_VECTOR (31 downto 3);
    V_B              : inout STD_LOGIC_VECTOR (32 downto 1);
    V_C              : inout STD_LOGIC_VECTOR (31 downto 1);

    --IO enable/Output controls
    A_nSWITCH_Ena    : out STD_LOGIC; -- enable bidir devices

    A_AC28_AC31_Out  : out STD_LOGIC; --A
    A_nAC28_AC31_Ena : out STD_LOGIC;

    A_AC26_AC27_Out  : out STD_LOGIC; --B
    A_nAC26_AC27_Ena : out STD_LOGIC;

    A_AC22_AC25_Out  : out STD_LOGIC;  --C
    A_nAC22_AC25_Ena : out STD_LOGIC;

    A_B18_B24_Out    : out STD_LOGIC;  --D
    A_nB18_B24_Ena   : out STD_LOGIC;

    A_IO1_Out        : out STD_LOGIC;  --E
    A_nIO1_Ena       : out STD_LOGIC;

    A_AC19_Out       : out STD_LOGIC;  --F
    A_nAC19_Ena      : out STD_LOGIC;

    A_C20_21_Out     : out STD_LOGIC; --G
    A_nC20_21_Ena    : out STD_LOGIC;

    A_I_Out          : out STD_LOGIC;
    A_I_Ena          : out STD_LOGIC;

    A_K_Out          : out STD_LOGIC;
    A_K_Ena          : out STD_LOGIC;

    A_L_Out          : out STD_LOGIC;
    A_L_Ena          : out STD_LOGIC;

    A_M_Out          : out STD_LOGIC;
    A_M_Ena          : out STD_LOGIC;

    A_N_Out          : out STD_LOGIC;
    A_N_Ena          : out STD_LOGIC;

    A_X1_Out         : out  STD_LOGIC;
    A_X1_Ena         : out  STD_LOGIC;

    A_X2_Out         : out  STD_LOGIC;
    A_X2_Ena         : out  STD_LOGIC;

    A_X3_Out         : out  STD_LOGIC;
    A_X3_Ena         : out  STD_LOGIC;

    A_X4_Out         : out  STD_LOGIC;
    A_X4_Ena         : out  STD_LOGIC;

    A_X5_Out         : out  STD_LOGIC;
    A_X5_Ena         : out  STD_LOGIC;

    A_X6_Out         : out  STD_LOGIC;
    A_X6_Ena         : out  STD_LOGIC;

    A_X7_Out         : out  STD_LOGIC;
    A_X7_Ena         : out  STD_LOGIC;

    A_X8_Out         : out  STD_LOGIC;
    A_X8_Ena         : out  STD_LOGIC;

    A_X9_Out         : out  STD_LOGIC;
    A_X9_Ena         : out  STD_LOGIC;

    A_X10_Out        : out  STD_LOGIC;
    A_X10_Ena        : out  STD_LOGIC;

    A_X11_Out        : out  STD_LOGIC;
    A_X11_Ena        : out  STD_LOGIC;

    A_X12_Out        : out  STD_LOGIC;
    A_X12_Ena        : out  STD_LOGIC;

------------------------------------------
    -- Addressbus der MCU
    -- TI-EPIOS
    EPIOS_28         : in STD_LOGIC; -- WR
    EPIOS_29         : in STD_LOGIC; -- RD
    EPIOS_30         : in STD_LOGIC; -- Frame
    EPIOS_31         : in STD_LOGIC; -- clock

    EPIOS_32         : in STD_LOGIC; -- TBD IRDY
    EPIOS_33         : in STD_LOGIC; -- TBD CS3
    EPIOS_34         : in STD_LOGIC; -- TBD CS2
    EPIOS_35         : in STD_LOGIC; -- TBD CRE

    EPIOS_ADR        : in STD_LOGIC_VECTOR (11 downto 0);       --Adresse
    EPIOS            : inout STD_LOGIC_VECTOR (15 downto 0);    -- DataIO

    ------------------------------------------
    A_WAKE           : in  STD_LOGIC;     --Leitung vom MCU-WAKE Eingang - Bisher keine Verwendung
    MCU_RESET_FILTER : in  STD_LOGIC;     --Reset Leitung vom MCU-Resettaster -> Später bidir - Bisher keine Verwendung
    MCU_NMI          : in  STD_LOGIC;     --NMI von der MCU - Bisher keine Verwendung

    ------------------------------------------
    A_nMANUAL_RES    : in  STD_LOGIC;     -- Steuerleitung vom reset generator/ Taster S3
   -- RESET_N        : in  STD_LOGIC;     -- Resetleitung vom VCC Control / Wird auch über S3 gesteuert
    ------------------------------------------

   -- MAX10_JTAG_EN  : in  STD_LOGIC;

    --MAX10_JTAG_TMS : in  STD_LOGIC;
    --MAX10_JTAG_TCK : in  STD_LOGIC;
    --MAX10_JTAG_TDI : in  STD_LOGIC;
   -- MAX10_JTAG_TDO : out  STD_LOGIC;

   -- CONFIG_SEL     : in  STD_LOGIC;
   -- NCONFIG        : in  STD_LOGIC;
  --  MAX10_CRC_CONF_ERR  : out  STD_LOGIC;
  --  MAX10_NSTATUS       : out  STD_LOGIC;
  --  MAX10_CONF_DONE     : out  STD_LOGIC;

      RUP               : in  STD_LOGIC;
      RDN               : in  STD_LOGIC


 );
end ifa10;


--------------------------------------------------

architecture Behavioral of ifa10 is

----------------------------------------------------------------
-- Timing und Clock generation

signal   sys_clk              : STD_LOGIC := '0';  --global system clock

signal   clk_50MHz            : STD_LOGIC := '0';
signal   clk_25MHz            : STD_LOGIC := '0';

signal   clk_24MHz            : STD_LOGIC := '0';
signal   clk_12MHZ            : STD_LOGIC := '0';

signal   clk_6MHz             : STD_LOGIC := '0';  --no PLL
--signal   ext_clk              : STD_LOGIC := '0';  --no PLL

signal   locked_sig           : STD_LOGIC := '0';  --PLL status- wird als global system reset verwendet- RST Freigabe erst, wenn PLL stabil

signal   Ena_every_100ns      : STD_LOGIC := '0';
signal   Ena_every_166ns      : STD_LOGIC := '0';
signal   Ena_every_250ns      : STD_LOGIC := '0';
signal   Ena_every_500ns      : STD_LOGIC := '0';
signal   Ena_every_us         : STD_LOGIC := '0';
--signal   Ena_every_ms         : STD_LOGIC := '0';
signal   Ena_every_20ms       : STD_LOGIC := '0';
signal   Ena_every_1sec       : STD_LOGIC := '0';
signal   Ena_every_2_5s       : STD_LOGIC := '0';

----------------------------------------------------------------

signal sys_reset     : STD_LOGIC := '0';  --Global Reset

----------------------------------------------------------------

signal ME_BZI        : std_logic := '0';
signal ME_BOI        : std_logic := '0';
signal ME_UDI        : std_logic := '0';
signal ME_SDI        : std_logic := '0';
signal ME_EE         : std_logic := '0';
signal ME_SS         : std_logic := '0';
signal ME_MIL1_nBZO  : std_logic := '0';
signal ME_MIL1_nBOO  : std_logic := '0';

--input
signal ME_DSC        : std_logic := '0';
signal ME_SDO        : std_logic := '0';
signal ME_VW         : std_logic := '0';
signal ME_CDS        : std_logic := '0';
signal ME_TD         : std_logic := '0';
signal ME_ESC        : std_logic := '0';
signal ME_SD         : std_logic := '0';
signal ME_nBOO       : std_logic := '0';
signal ME_nBZO       : std_logic := '0';
signal MIL1_BOI      : std_logic := '0';
signal MIL1_BZI      : std_logic := '0';
signal UMIL5V        : std_logic := '0';
signal UMIL15V       : std_logic := '0';
signal nSEL_6408     : std_logic := '0';

signal MIL1_OUT_Ena  : std_logic := '0';
signal MIL1_nIN_Ena  : std_logic := '0';
signal FS_ASDI_out   : std_logic := '0';

signal FS_DATA_in    : std_logic := '0';
signal nMANUAL_RES   : std_logic := '1';

signal SEL_B         : std_logic_vector(3 downto 0) := ( others =>'0');

signal IN_NINL_in    : std_logic := '1';
signal IN_NRDY_in    : std_logic := '1';
signal IN_NDRQ_in    : std_logic := '1';
signal LEMO_IN_s     : std_logic := '0';

signal WAKE          : std_logic := '0';
signal MCU_RST_F_in  : std_logic := '0';
signal MCU_NMI_in    : std_logic := '0';


---------------

signal L_Test              : STD_LOGIC_VECTOR (15 downto 0) := ( others =>'0');

signal A_OUT               : STD_LOGIC_VECTOR (32 downto 1) := ( others =>'0');
signal B_OUT               : STD_LOGIC_VECTOR (32 downto 1) := ( others =>'0');
signal C_OUT               : STD_LOGIC_VECTOR (32 downto 1) := ( others =>'0');

signal A_A_Out_CRTL        : std_logic := '0';
signal A_A_Ena_CRTL        : std_logic := '1';
signal A_B_Out_CRTL        : std_logic := '0';
signal A_B_Ena_CRTL        : std_logic := '1';
signal A_C_Out_CRTL        : std_logic := '0';
signal A_C_Ena_CRTL        : std_logic := '1';
signal A_D_Out_CRTL        : std_logic := '0';
signal A_D_Ena_CRTL        : std_logic := '1';
signal A_E_Out_CRTL        : std_logic := '0';
signal A_E_Ena_CRTL        : std_logic := '1';
signal A_F_Out_CRTL        : std_logic := '0';
signal A_F_Ena_CRTL        : std_logic := '1';
signal A_G_Out_CRTL        : std_logic := '0';
signal A_G_Ena_CRTL        : std_logic := '1';
signal A_I_Out_CRTL        : std_logic := '0';
signal A_I_Ena_CRTL        : std_logic := '1';
signal A_K_Out_CRTL        : std_logic := '0';
signal A_K_Ena_CRTL        : std_logic := '1';
signal A_L_Out_CRTL        : std_logic := '0';
signal A_L_Ena_CRTL        : std_logic := '1';
signal A_M_Out_CRTL        : std_logic := '0';
signal A_M_Ena_CRTL        : std_logic := '1';
signal A_N_Out_CRTL        : std_logic := '0';
signal A_N_Ena_CRTL        : std_logic := '1';

signal A_X1_Out_CRTL       : std_logic := '0';
signal A_X1_Ena_CRTL       : std_logic := '1';
signal A_X2_Out_CRTL       : std_logic := '0';
signal A_X2_Ena_CRTL       : std_logic := '1';
signal A_X3_Out_CRTL       : std_logic := '0';
signal A_X3_Ena_CRTL       : std_logic := '1';
signal A_X4_Out_CRTL       : std_logic := '0';
signal A_X4_Ena_CRTL       : std_logic := '1';
signal A_X5_Out_CRTL       : std_logic := '0';
signal A_X5_Ena_CRTL       : std_logic := '1';
signal A_X6_Out_CRTL       : std_logic := '0';
signal A_X6_Ena_CRTL       : std_logic := '1';

signal A_X7_Out_CRTL       : std_logic := '0';
signal A_X7_Ena_CRTL       : std_logic := '1';

signal A_X8_Out_CRTL       : std_logic := '0';
signal A_X8_Ena_CRTL       : std_logic := '1';

signal A_X9_Out_CRTL       : std_logic := '0';
signal A_X9_Ena_CRTL       : std_logic := '1';

signal A_X10_Out_CRTL      : std_logic := '0';
signal A_X10_Ena_CRTL      : std_logic := '1';

signal A_X11_Out_CRTL      : std_logic := '0';
signal A_X11_Ena_CRTL      : std_logic := '1';

signal A_X12_Out_CRTL      : std_logic := '0';
signal A_X12_Ena_CRTL      : std_logic := '1';

signal nLED                : STD_LOGIC_VECTOR(27 downto 0);

signal SHOW_CONFIGn        : std_logic := '1';
signal nFAILSAVE_LD_LEDn   : std_logic := '1';
signal A_nLED_EXTCLKn      : std_logic := '1';
signal A_NLED_EVT_INRn     : std_logic := '1';

signal V_Ain              : STD_LOGIC_VECTOR (32 downto 1);
signal V_Bin              : STD_LOGIC_VECTOR (32 downto 1);
signal V_Cin              : STD_LOGIC_VECTOR (32 downto 1);

--------------------------------------------
-- Signale für MCU-Datenbus
--------------------------------------------
signal EPIOS_WR         : std_logic := '0'; --WR
signal EPIOS_RD         : std_logic := '0'; --RD
signal EPIOS_Frame      : std_logic := '0'; --Frame
signal EPIOS_CLK        : std_logic := '0'; --CLK
signal EPIOS_CLK1       : std_logic := '0'; --CLK-Speicher
signal EPIOS_ALE        : std_logic := '0'; --ALE
signal EPIOS_CSn        : std_logic := '0'; --Chip-Select

signal EPIOS_Dataout    : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0');
signal EPIOS_Datain     : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0');
signal EPIOS_Datain_reg : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); --speichert Eingangsdaten
signal EPIOS_Adr_in     : STD_LOGIC_VECTOR(11 Downto 0) := ( others =>'0');
signal EPIOS_Adr_reg    : STD_LOGIC_VECTOR(11 Downto 0) := ( others =>'0'); --speichert Eingangsadresse

signal EPIOS_Dataout_sel: STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0');

signal rdout            : STD_LOGIC := '0';
signal Wr_Puls          : STD_LOGIC := '0';  -- beim Schreiben eines Registers
signal RD_Puls          : STD_LOGIC := '0';  -- beim Lesen auf 1

signal EPIO_Register    : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Speicher für EPIOS Kommunikation

signal ADRin_DB         : STD_LOGIC_VECTOR (7 downto 0) := ( others =>'0');  --Address debounced

--------------------------------------------
-------------------------------------------------------------------------------

--EPIO BUS READ STM
type read_states is (
  RD_0,
  RD_1,
  RD_2,
  RD_3,
  RD_4
);

signal   read_state : read_states := RD_0;

signal in_EPIOS_32   : std_logic := '0'; -- TBD IRDY
signal in_EPIOS_33   : std_logic := '0'; -- TBD CS3
signal in_EPIOS_34   : std_logic := '0'; -- TBD CS2
signal in_EPIOS_35   : std_logic := '0'; -- TBD CRE

----------------------------------------------------------------


signal A_OUT_en     : std_logic_vector (32 DOWNTO 1):= ( others =>'0');
signal B_OUT_en     : std_logic_vector (32 DOWNTO 1):= ( others =>'0');
signal C_OUT_en     : std_logic_vector (32 DOWNTO 1):= ( others =>'0');

signal X_SelDirO    : std_logic_vector (12 DOWNTO 1):= ( others =>'0');
signal X_EnIO       : std_logic_vector (12 DOWNTO 1):= ( others =>'0');



----------------------------------------------------------------
----------------------------------------------------------------

Begin

A_ME_BZI    <= ME_BZI;
A_ME_BOI    <= ME_BOI;
A_ME_UDI    <= ME_UDI;
A_ME_SDI    <= ME_SDI;
A_ME_EE     <= ME_EE;
A_ME_SS     <= ME_SS;
A_MIL1_nBZO <= ME_MIL1_nBZO;
A_MIL1_nBOO <= ME_MIL1_nBOO;

A_MIL1_OUT_Ena <= MIL1_OUT_Ena;
A_MIL1_nIN_Ena <= MIL1_nIN_Ena;
FS_ASDI        <= FS_ASDI_out;

A_Test         <= L_Test;

SPARE          <= (others=>'Z');

LEMO_OUT       <= '0';


--Eingangssignale eintakten

ReadinBuf: process(sys_clk,sys_reset,V_A,V_B,V_C,A_ME_DSC,A_ME_VW,A_ME_CDS,A_ME_ESC,A_ME_TD,A_ME_SD,A_ME_nBOO,
                         A_ME_nBZO,A_MIL1_BOI,A_MIL1_BZI,A_UMIL5V,A_UMIL15V,A_nSEL_6408,IN_NINL,IN_NRDY,
                         IN_NDRQ,FS_DATA,A_SEL_B,LEMO_IN_s)

 begin
  if sys_reset='1'then
      V_Ain       <= (others=>'0');
      V_Bin       <= (others=>'0');
      V_Cin       <= (others=>'0');
      ME_DSC      <='0';
      ME_SDO      <='0';
      ME_VW       <='0';
      ME_CDS      <='0';
      ME_TD       <='0';
      ME_ESC      <='0';
      ME_SD       <='0';
      ME_nBOO     <='0';
      ME_nBZO     <='0';
      MIL1_BOI    <='0';
      MIL1_BZI    <='0';
      UMIL5V      <='0';
      UMIL15V     <='0';
      nSEL_6408   <='0';
      FS_DATA_in  <='0';
      nMANUAL_RES <='1';
      SEL_B       <= (others=>'0');
      IN_NINL_in  <='1';
      IN_NRDY_in  <='1';
      IN_NDRQ_in  <='1';
      LEMO_IN_s   <='0';
      WAKE        <='0';
      MCU_RST_F_in<='0';
      MCU_NMI_in  <='1';

    elsif rising_edge(sys_clk) then

      V_Ain <= "0" & V_A & "00";
      --V_Ain(1)  =>  '0',
      --V_Ain(2)  =>  '0',
      --V_Ain(32) =>  '0',

      V_Bin <= V_B;
      --  V_Cin(2)  =>  '0',
      --  V_Cin(32) =>  '0',
      V_Cin <= "0" & V_C(31 downto 3) & "0" & V_C(1);

      ME_DSC      <=A_ME_DSC;
      ME_SDO      <=A_ME_SDO;
      ME_VW       <=A_ME_VW;
      ME_CDS      <=A_ME_CDS;
      ME_TD       <=A_ME_TD;
      ME_ESC      <=A_ME_ESC;
      ME_SD       <=A_ME_SD;
      ME_nBOO     <=A_ME_nBOO;
      ME_nBZO     <=A_ME_nBZO;
      MIL1_BOI    <=A_MIL1_BOI;
      MIL1_BZI    <=A_MIL1_BZI;
      UMIL5V      <=A_UMIL5V;
      UMIL15V     <=A_UMIL15V;
      nSEL_6408   <=A_nSEL_6408;
      IN_NINL_in  <=IN_NINL;
      IN_NRDY_in  <=IN_NRDY;
      IN_NDRQ_in  <=IN_NDRQ;
      FS_DATA_in  <=FS_DATA;
      nMANUAL_RES <=A_nMANUAL_RES;
      SEL_B       <=A_SEL_B;
      LEMO_IN_s   <=LEMO_IN_s;

      WAKE        <=A_WAKE;
      MCU_RST_F_in<=MCU_RESET_FILTER;
      MCU_NMI_in  <=MCU_NMI;
---      outputs

  end if;

end process;


--Modul: ifa8_X
Modul: ifa8_X
  Port MAP (

   sys_clk       => sys_clk,
   sys_reset     => sys_reset,
   clk_50MHz     => clk_50MHz,

   --system controls
   A_Timing     => LEMO_IN_s,-- von Lemo
   A_EXT_CLK    => A_EXT_CLK, --von Lemo
   
   IN_NINL         =>IN_NINL_in,
   IN_NRDY        =>IN_NRDY_in,
   IN_NDRQ        =>IN_NDRQ_in,

   CPU_RESET      =>'0',
   Manual_RES     =>'0',

   clk_24MHz      => clk_24MHz,
   clk_6MHz       => clk_6MHz,

   A_SEL_B        => not SEL_B,
   A_CLK_FG       => clk_25MHz,--clk_50MHz?

   Ena_every100ns => Ena_every_100ns,
   Ena_every166ns => Ena_every_166ns,
   Ena_every250ns => Ena_every_250ns,
   Ena_every500ns => Ena_every_500ns,
   Ena_every1us   => Ena_every_us,
   Ena_every20ms  => Ena_every_20ms,
   Ena_every1sec  => Ena_every_1sec,
   Ena_every2_5s  => Ena_every_2_5s,

   A_ME_DSC    => ME_DSC,   --decoder clk 1MHz
   A_ME_ESC    => ME_ESC,   --encoder clk 1MHz
   A_ME_SDO    => ME_SDO,
   A_ME_VW     => ME_VW,
   A_ME_CDS    => ME_CDS,
   A_ME_TD     => ME_TD,
   A_ME_SD     => ME_SD,
   A_ME_nBOO   => ME_nBOO,
   A_ME_nBZO   => ME_nBZO,
   A_MIL1_BOI  => MIL1_BOI,
   A_MIL1_BZI  => MIL1_BZI,
   A_UMIL5V    => UMIL5V,
   A_UMIL15V   => UMIL15V,

   A_ME_BZI    => ME_BZI,
   A_ME_BOI    => ME_BOI,
   A_ME_UDI    => ME_UDI,
   A_ME_SDI    => ME_SDI,
   A_ME_EE        => ME_EE,
   A_ME_SS        => ME_SS,
   A_nSEL_6408    => nSEL_6408,

   A_nOPK_INL     => A_nOPK_INL,
   A_nOPK_DRQ     => A_nOPK_DRQ,
   A_nOPK_DRDY    => A_nOPK_DRDY,

   A_MIL1_nBZO    => ME_MIL1_nBZO,
   A_MIL1_nBOO => ME_MIL1_nBOO,
   A_MIL1_OUT_Ena => MIL1_OUT_Ena,
   A_MIL1_nIN_Ena => MIL1_nIN_Ena,

   FS_DATA  => FS_DATA_in,
   FS_DCLK  => FS_DCLK,
   FS_nCS   => FS_nCS,
   FS_ASDI  => FS_ASDI_out,

   A_nLED          => nLED,

   A_nSWITCH_Ena => A_nSWITCH_Ena,

   V_Ain    => V_Ain,
   V_Bin    => V_Bin,
   V_Cin    => V_Cin,

   A_A_Out  =>  A_A_Out_CRTL,
   A_A_Ena  =>  A_A_Ena_CRTL,

   A_B_Out  => A_B_Out_CRTL,
   A_B_Ena  => A_B_Ena_CRTL,

   A_C_Out  => A_C_Out_CRTL,
   A_C_Ena  => A_C_Ena_CRTL,

   A_D_Out  => A_D_Out_CRTL,
   A_D_Ena  => A_D_Ena_CRTL,

   A_E_Out  => A_E_Out_CRTL,
   A_E_Ena  => A_E_Ena_CRTL,

   A_F_Out  => A_F_Out_CRTL,
   A_F_Ena  => A_F_Ena_CRTL,

   A_G_Out  => A_G_Out_CRTL,
   A_G_Ena  => A_G_Ena_CRTL,

   A_I_Out  => A_I_Out_CRTL,
   A_I_Ena  => A_I_Ena_CRTL,

   A_K_Out  => A_K_Out_CRTL,
   A_K_Ena  => A_K_Ena_CRTL,

   A_L_Out  => A_L_Out_CRTL,
   A_L_Ena  => A_L_Ena_CRTL,

   A_M_Out  => A_M_Out_CRTL,
   A_M_Ena  => A_M_Ena_CRTL,

   A_N_Out  => A_N_Out_CRTL,
   A_N_Ena  => A_N_Ena_CRTL,

   X_SelDirO=> X_SelDirO,
   X_EnIO   => X_EnIO,

   NOE_TST  => OE_Test,

   AP_IO    => A_P,

   A_Test   => L_Test,

   A_Aout   => A_OUT,
   A_Bout   => B_OUT,
   A_Cout   => C_OUT,

   A_out_en => A_OUT_en,
   B_out_en => B_OUT_en,
   C_out_en => C_OUT_en,

   nEmpf_en     =>'1', --  RCV Register darf im LAN-Modul beschrieben werden

   EPIOS_CSn    => EPIOS_CSn,
   EPIOS_Adr    => EPIOS_Adr_reg, --stabile Adresse
   EPIOS_WR     => Wr_Puls,
   EPIOS_RD     => RD_Puls,
   EPIOS_Frame  => EPIOS_Frame,
   EPIOS_CLK    => EPIOS_CLK,
   EPIOS_Datain => EPIOS_Datain_reg,
   EPIOS_Dataout=> EPIOS_Dataout_sel

);


  A_AC28_AC31_Out    <= A_A_Out_CRTL;
  A_nAC28_AC31_Ena   <= A_A_Ena_CRTL;

  A_AC26_AC27_Out    <= A_B_Out_CRTL;
  A_nAC26_AC27_Ena   <= A_B_Ena_CRTL;

  A_AC22_AC25_Out    <= A_C_Out_CRTL;
  A_nAC22_AC25_Ena   <= A_C_Ena_CRTL;

  A_B18_B24_Out      <= A_D_Out_CRTL;
  A_nB18_B24_Ena     <= A_D_Ena_CRTL;

  A_IO1_Out          <= A_E_Out_CRTL;
  A_nIO1_Ena         <= A_E_Ena_CRTL;

  A_AC19_Out         <= A_F_Out_CRTL;
  A_nAC19_Ena        <= A_F_Ena_CRTL;

  A_C20_21_Out       <= A_G_Out_CRTL;
  A_nC20_21_Ena      <= A_G_Ena_CRTL;

  A_I_Out  <= A_I_Out_CRTL;
  A_I_Ena  <= A_I_Ena_CRTL;

  A_K_Out  <=A_K_Out_CRTL;
  A_K_Ena  <=A_K_Ena_CRTL;

  A_L_Out  <=A_L_Out_CRTL;
  A_L_Ena  <=A_L_Ena_CRTL;

  A_M_Out  <=A_M_Out_CRTL;
  A_M_Ena  <=A_M_Ena_CRTL;

  A_N_Out  <=A_N_Out_CRTL;
  A_N_Ena  <=A_N_Ena_CRTL;

  A_X1_Out  <=X_SelDirO(1);
  A_X1_Ena  <=X_EnIO(1);

  A_X2_Out  <=X_SelDirO(2);
  A_X2_Ena  <=X_EnIO(2);

  A_X3_Out  <=X_SelDirO(3);
  A_X3_Ena  <=X_EnIO(3);

  A_X4_Out  <=X_SelDirO(4);
  A_X4_Ena  <=X_EnIO(4);

  A_X5_Out  <=X_SelDirO(5);
  A_X5_Ena  <=X_EnIO(5);

  A_X6_Out  <=X_SelDirO(6);
  A_X6_Ena  <=X_EnIO(6);

  A_X7_Out  <=X_SelDirO(7);
  A_X7_Ena  <=X_EnIO(7);
  A_X8_Out  <=X_SelDirO(8);
  A_X8_Ena  <=X_EnIO(8);

  A_X9_Out  <=X_SelDirO(9);
  A_X9_Ena  <=X_EnIO(9);
  A_X10_Out <=X_SelDirO(10);
  A_X10_Ena <=X_EnIO(10);

  A_X11_Out <=X_SelDirO(11);
  A_X11_Ena <=X_EnIO(11);

  A_X12_Out <=X_SelDirO(12);
  A_X12_Ena <=X_EnIO(12);



Aoengen:
  for I in 3 to 31 generate
         V_A(I)<= A_OUT(I)  when A_OUT_en(I)='1' else 'Z';
         end generate Aoengen;

Boengen:
  for I in 1 to 32 generate
         V_B(I)<= B_OUT(I)  when B_OUT_en(I)='1' else 'Z';
         end generate Boengen;

Coengen:
  for I in 1 to 31 generate
         V_C(I)<= C_OUT(I)  when C_OUT_en(I)='1' else 'Z';
         end generate Coengen;


---------------------------------------------
---------------------------------------------
--LED Reihenfolge wie auf PCB

--D84 PowerLed -- nicht dimmbar -> FET control?

A_NLED_EVT_INR    <= nLED(24);--A_NLED_EVT_INRn    when LEDdimmer='1' else '1';

A_nLED_EXTCLK     <= nLED(23);--A_nLED_EXTCLKn     when LEDdimmer='1' else '1';

nUSER_LD_LED      <= nLED(25);

nFAILSAVE_LD_LED  <= nLED(26);

SHOW_CONFIG       <= nLED(27);

A_nLED            <= nLED(22 downto 0);


----------------------------------
----------------------------------
--Clock und Timing bereitstellen

A_ME_12MHZ  <= clk_12MHz when locked_sig ='1' else 'Z';

ClkTimer: Clock_Timing
   PORT MAP
   (
      PLL_reset         => '0',
      clk_24MHz_in      => A_CLK_24MHz,
      clk_50MHz_in      => A_CLK_50MHz,
      clk_ext_in        => A_EXT_CLK,

      sys_clk           => sys_clk,
      locked_clk        => locked_sig,

      clk_24MHz_out     => clk_24MHz,
      clk_12MHZ_out     => clk_12MHz,
      clk_50MHZ_out      =>clk_50MHz,
      clk_25MHz_out     => clk_25MHz,
      ext_clk_out       => open,  --no PLL
      clk_6MHZ_out      => clk_6MHz, --no PLL

      Ena_every_100ns   => Ena_every_100ns,
      Ena_every_166ns   => Ena_every_166ns,
      Ena_every_250ns   => Ena_every_250ns,
      Ena_every_500ns   => Ena_every_500ns,
      Ena_every_us      => Ena_every_us,
      Ena_every_ms      => open,
      Ena_every_20ms    => Ena_every_20ms,
      Ena_every_1sec    => Ena_every_1sec,
      Ena_every_2_5s    => Ena_every_2_5s
   );

---------------------------------------------
-- Systemreset über PLL-Lock und externe Leitung
sys_reset<= (not locked_sig);-- and N_RESET;


---------------------------------------------------------------------
--
--Aufbereitung des TI EPIOS-Datenbus für MAX10-Register zum Lesen und Schreiben
--TI MCU-Datenbus hat 10MHz clk
---------------------------------------------------------------------
--TI-MCU control

--signal EPIOS_WR     : std_logic := '0'; --WR
--signal EPIOS_RD     : std_logic := '0'; --RD
--signal EPIOS_Frame  : std_logic := '0'; --Frame
--signal EPIOS_CLK    : std_logic := '0'; --CLK
--signal EPIOS_CLK1   : std_logic := '0'; --CLK speicher
--signal EPIOS_ALE    : std_logic := '0'; --ALE
--signal EPIOS_CSn    : std_logic := '0'; --Chip select
--
--signal EPIOS_Datain : STD_LOGIC_VECTOR(15 downto 0); --Data (15 Downto0)
--signal EPIOS_adr    : STD_LOGIC_VECTOR(11 Downto 0);


--BUS-Takt einlesen und flankendetector generieren
epiosclk: process(sys_clk,sys_reset,EPIOS_CLK)

 begin
  if sys_reset='1'then
      EPIOS_CLK1 <='0';
   elsif rising_edge(sys_clk) then
      EPIOS_CLK1 <= EPIOS_CLK;
  end if;

end process;


--Bei Frame-Signal = 1 -> Eingangsdaten und Adresse merken
--Schreibpuls setzen
epiosadr: process(sys_clk,sys_reset,EPIOS_Adr_in,EPIOS_CLK,EPIOS_CLK1,EPIOS_Frame,EPIOS_WR,EPIOS_Datain)

 begin
   if sys_reset='1' then
        Wr_Puls          <='0';   --Schreibpuls für nachfolgende Register setzen
        EPIOS_Datain_reg <= ( others =>'0');
        EPIOS_Adr_reg    <= ( others =>'0');
    elsif rising_edge(sys_clk) then
     Wr_Puls <='0';                      -- für einen sysclock auf H
    if EPIOS_Frame='1' then
       if EPIOS_CLK='1' and EPIOS_CLK1='0' then
         EPIOS_Adr_reg      <= EPIOS_Adr_in;  -- Adresse sichern
         if EPIOS_WR='1' then
           Wr_Puls <='1';      -- nur hier 1x Schreibpuls 1x setzen, ev. Puls 1x sysclk später
           EPIOS_Datain_reg <= EPIOS_Datain;
         else
           EPIOS_Datain_reg <= ( others =>'0');-- x"fafa"; -- dummy Data, um Registerfehler anzuzeigen
         end if;
       end if;
     end if;
   end if;

end process;

--State-Machine für Lesen
--Die Read-Daten müssen bis zum Nächsten LH-CLK für Ausgabe auf dem BUS bereitstehen .
epiosrds: process(sys_clk,sys_reset,EPIOS_CLK,EPIOS_CLK1,EPIOS_Frame,EPIOS_RD)

 begin

    if sys_reset='1' then
     RD_Puls     <= '0';
     read_state  <= RD_0;
   elsif rising_edge(sys_clk) then

     RD_Puls <= '0';
     rdout   <= '0';      --Lesen abschalten
     case read_state is

          when RD_0 =>
                    if EPIOS_Frame = '1' and EPIOS_RD   = '1' then
                      if EPIOS_CLK = '1' and EPIOS_CLK1 = '0' then
                          if EPIOS_WR='0' then --es muss wr == 0 sein sonst kein Lesen
                        -- EPIOS_Adr_reg
                        -- EPIOS_Dataout <= EPIO_Register;
                        -- Wert abholen -- ab hier ist die interne Adresse stabil
                          RD_Puls <= '1';
                          read_state <= RD_1;                  -- lesen wird angemeldet
                          end if;
                      end if;
                    end if;

         when RD_1 =>
                     if EPIOS_CLK = '0' and EPIOS_CLK1 = '1' then
                       read_state <= RD_2;                  -- auf nächsten EPIOS_CLK HL warten
                    rdout <= '1';                        -- Signal zur Freigabe des Ausgebewortes
                     end if;

        when RD_2 =>
                     rdout <= '1';                          -- Ausgabe freigeben
                     --read_state <= RD_3;                  --RD_3 redundant s.u.
                     if EPIOS_CLK = '0' and EPIOS_CLK1 = '1' then  --warten auf HL dann
                       read_state <= RD_0;
                     end if;

        when others =>
                     read_state <= RD_0;
     end case;
   end if;

end process;


--Tristate-Switch - Ausgabe /lesen zur MCU nur, wenn Wr==0
EPIOS(15 downto 0) <= EPIOS_Dataout_sel when rdout = '1' and  EPIOS_WR='0' else (others => 'Z');

--Entprellen/einlesen vom MCU-Datenbus
debounceEPIO: process (sys_clk,sys_reset,EPIOS,EPIOS_ADR,EPIOS_28,EPIOS_29,EPIOS_30,EPIOS_31,EPIOS_32,EPIOS_33,EPIOS_34,EPIOS_35)

  begin
   if sys_reset='1'then
      EPIOS_Datain    <= EPIOS(15 downto 0);
      EPIOS_Adr_in    <= EPIOS_ADR(11 downto 0); --EPIOS(27 Downto 16)

      EPIOS_WR       <= '0'; --WR
      EPIOS_RD       <= '0'; --RD
      EPIOS_Frame    <= '0'; --Frame/ALE
      EPIOS_CLK      <= '0'; --clock

      in_EPIOS_32    <= '0';
      in_EPIOS_33    <= '0';
      in_EPIOS_34    <= '0';
      in_EPIOS_35    <= '0';

    elsif rising_edge(sys_clk) then

      EPIOS_Datain   <= EPIOS(15 downto 0);
      EPIOS_Adr_in   <= EPIOS_ADR(11 downto 0); --EPIOS(27 Downto 16)

      EPIOS_WR       <= EPIOS_28; --WR
      EPIOS_RD       <= EPIOS_29; --RD
      EPIOS_Frame    <= EPIOS_30; --Frame/ALE
      EPIOS_CLK      <= EPIOS_31; --clock

      in_EPIOS_32    <= EPIOS_32;   -- TBD IRDY
      in_EPIOS_33    <= EPIOS_33;   -- TBD CS2
      in_EPIOS_34    <= EPIOS_34;   -- TBD CS3
      in_EPIOS_35    <= EPIOS_35;   -- TBD CS4

    end if;
end process;

--configuration EPIOSMODE (D16, A12)

--   EPIOS_ALE <= EPIOS_27; --ALE/CS1
--   EPIOS_CSn <= EPIOS_26; --CSn
--   bsel0
--   bsel1

--EPIOS(15 downto 0)<= EPIOS_Dataout ; --Data (15 Downto0)



--??
--testausgabe/funktioniert EPIOS datenbus
--epioswr: process(sys_clk,sys_resetEPIOS_adr_in,EPIOS_Datain,EPIOS_CLK,EPIOS_CLK1,EPIOS_WR,EPIOS_Frame)
--
-- begin
-- if sys_reset='1'then
-- wrtttest <='0';
--    elsif rising_edge(sys_clk) then
--      if EPIOS_Frame='1'  and EPIOS_WR='1' then
--        if EPIOS_CLK = '1' and EPIOS_CLK1 = '0' then
--          if EPIOS_adr_in(3 downto 0) = "0010" then
--
--  --          A_nLEDnswap(18) <= wrtttest;
--            wrtttest <= not wrtttest;
--          end if;
--        end if;
--      else
--      end if;
-- end if;
--  end if;
--
-- end process;




-------------------------------------------------
-------------------------------------------------

--------------------------------------------------------------
--------------------------------------------------------------

----LED-Dimmer
--dimLed: process (clk_24MHz,counter24,A_SEL_B,Ledoutshift) -- shiftet
--  begin
--
--    if rising_edge(clk_24MHz) then
--        if counter24(16) = '0' then -- erst mal immer 50% mindestens auf OFF
--          LEDdimmer<='0'; --0
--        else
--          if counter24(15 downto 12) =  (Ledoutshift'range => '0') then
--            LEDdimmer<='0';--0
--          else
--            if counter24(15 downto 12) = A_SEL_B then -- ab hier alle LEDs aus
--              LEDdimmer<='1';          --erst ab diesem Wert wieder an
--            end if;
--          end if;
--        end if;
--    end if;
--end process;
--
--
--signalactive24MHZ: process (clk_24MHz,Ledoutshift,counter24)
--  begin
--
--    if rising_edge(clk_24MHz) then
--        counter24<=counter24+1;
--        if (clkold /= counter24(24)) then
--          if Ledoutshift =  (Ledoutshift'range => '0') then
--            Ledoutshift(0)<='1';
--          else
--            Ledoutshift<= Ledoutshift(21 downto 0) & '0';
--          end if;
--        end if;
--        clkold<=counter24(24);
--    end if;
--end process;


 --A_nLED(20 downto 0)<=  Ledoutshift (20 downto 0);
 --A_nLED(16 downto 1)<= (others=>'1');
 --A_nLED(20 downto 17) <= A_SEL_B(3 downto 0);
 --A_nLED(22)<=counter(24);
 --A_nLED(21)<=counter24(24);
 --nFAILSAVE_LD_LED<=counter24(24);
 --A_NLED_EVT_INR<=counter24(24);
 --A_nLED_EXTCLK <=counter24(24);
 --A_nLED(0)<='0';


end Behavioral;

    --  V_C(28) <= C_OUT(28) when A_A_Out_CRTL='1' else 'Z';
--  V_A(28) <= A_OUT(28) when A_A_Out_CRTL='1' else 'Z';
--  V_C(29) <= C_OUT(29) when A_A_Out_CRTL='1' else 'Z';
--  V_A(29) <= A_OUT(29) when A_A_Out_CRTL='1' else 'Z';
--  V_C(30) <= C_OUT(30) when A_A_Out_CRTL='1' else 'Z';
--  V_A(30) <= A_OUT(30) when A_A_Out_CRTL='1' else 'Z';
--  V_C(31) <= C_OUT(31) when A_A_Out_CRTL='1' else 'Z';
--  V_A(31) <= A_OUT(31) when A_A_Out_CRTL='1' else 'Z';

--  V_C(26) <= C_OUT(26) when A_B_Out_CRTL='1' else 'Z';
--  V_A(26) <= A_OUT(26) when A_B_Out_CRTL='1' else 'Z';
--  V_C(27) <= C_OUT(27) when A_B_Out_CRTL='1' else 'Z';
--  V_A(27) <= A_OUT(27) when A_B_Out_CRTL='1' else 'Z';

--  V_C(22) <= C_OUT(22) when A_C_Out_CRTL='1' else 'Z';
--  V_A(22) <= A_OUT(22) when A_C_Out_CRTL='1' else 'Z';
--  V_C(23) <= C_OUT(23) when A_C_Out_CRTL='1' else 'Z';
--  V_A(23) <= A_OUT(23) when A_C_Out_CRTL='1' else 'Z';
--  V_C(24) <= C_OUT(24) when A_C_Out_CRTL='1' else 'Z';
--  V_A(24) <= A_OUT(24) when A_C_Out_CRTL='1' else 'Z';
--  V_C(25) <= C_OUT(25) when A_C_Out_CRTL='1' else 'Z';
--  V_A(25) <= A_OUT(25) when A_C_Out_CRTL='1' else 'Z';

--  V_B(18) <= B_OUT(18) when A_D_Out_CRTL='1' else 'Z';
--  V_B(19) <= B_OUT(19) when A_D_Out_CRTL='1' else 'Z';
--  V_B(20) <= B_OUT(20) when A_D_Out_CRTL='1' else 'Z';
--  V_B(21) <= B_OUT(21) when A_D_Out_CRTL='1' else 'Z';
--  V_B(22) <= B_OUT(22) when A_D_Out_CRTL='1' else 'Z';
--  V_B(23) <= B_OUT(23) when A_D_Out_CRTL='1' else 'Z';
--  V_B(24) <= B_OUT(24) when A_D_Out_CRTL='1' else 'Z';

--  V_A(18) <= A_OUT(18) when A_E_Out_CRTL='1' else 'Z';
--  V_A(19) <= A_OUT(19) when A_E_Out_CRTL='1' else 'Z';
--  V_A(20) <= A_OUT(20) when A_E_Out_CRTL='1' else 'Z';
--  V_A(21) <= A_OUT(21) when A_E_Out_CRTL='1' else 'Z';

-- V_C(19) <= C_OUT(19) when A_F_Out_CRTL='1' else 'Z';

--  V_C(20) <= C_OUT(20) when A_G_Out_CRTL='1' else 'Z';
--  V_C(21) <= C_OUT(21) when A_G_Out_CRTL='1' else 'Z';
--  V_C(18) <= C_OUT(18) when A_G_Out_CRTL='1' else 'Z';
--
      --  V_C(13) <= C_OUT(13) when A_I_Out_CRTL='1' else 'Z';
--  V_A(13) <= A_OUT(13) when A_I_Out_CRTL='1' else 'Z';
--  V_A(15) <= A_OUT(15) when A_I_Out_CRTL='1' else 'Z';

--  V_A(12)<= A_OUT(12)  when A_K_Out_CRTL='1' else 'Z';
--  V_A(14)<= A_OUT(14)  when A_K_Out_CRTL='1' else 'Z';
--  V_C(12)<= C_OUT(12)  when A_K_Out_CRTL='1' else 'Z';
--  V_C(14)<= C_OUT(14)  when A_K_Out_CRTL='1' else 'Z';
--  V_C(15)<= C_OUT(15)  when A_K_Out_CRTL='1' else 'Z';

--  V_B(11)<=B_OUT(11) when A_L_Out_CRTL='1' else 'Z';
--  V_B(26)<=B_OUT(26) when A_L_Out_CRTL='1' else 'Z';
--  V_B(28)<=B_OUT(28) when A_L_Out_CRTL='1' else 'Z';
--  V_B(30)<=B_OUT(30) when A_L_Out_CRTL='1' else 'Z';
--  V_B(32)<=B_OUT(32) when A_L_Out_CRTL='1' else 'Z';
--  V_C(11)<=C_OUT(11) when A_L_Out_CRTL='1' else 'Z';

--  V_B(14)<= B_OUT(14)  when A_M_Out_CRTL='1' else 'Z';
--  V_B(25)<= B_OUT(25)  when A_M_Out_CRTL='1' else 'Z';

--  V_B(2) <= B_OUT(2)  when A_N_Out_CRTL='1' else 'Z';
--  V_B(4) <= B_OUT(4)  when A_N_Out_CRTL='1' else 'Z';
--  V_B(12)<= B_OUT(12) when A_N_Out_CRTL='1' else 'Z';

--V_A(11)<=  A_OUT(11) when A_X1_Out_CRTL='1' else 'Z';

-- V_A(16)<= A_OUT(16)  when A_X2_Out_CRTL='1' else 'Z';
--  V_A(17)<= A_OUT(17)  when A_X3_Out_CRTL='1' else 'Z';
-- V_B(1)<= B_OUT(1)  when A_X4_Out_CRTL='1' else 'Z';
-- V_B(8)<= B_OUT(8)  when A_X5_Out_CRTL='1' else 'Z';
-- bisher direkter input
-- V_B(13)<= B_OUT(13)  when A_X6_Out_CRTL='1' else 'Z';
--  V_B(15)<= B_OUT(15)  when A_X7_Out_CRTL='1' else 'Z';
-- V_B(16)<= B_OUT(16)  when A_X8_Out_CRTL='1' else 'Z';
--  V_B(17)<= B_OUT(17)  when A_X9_Out_CRTL='1' else 'Z';
-- V_B(9)<= B_OUT(9)  when A_X10_Out_CRTL='1' else 'Z';--check
--  V_C(16)<= C_OUT(16)  when A_X11_Out_CRTL='1' else 'Z';
--  V_B(3)<= B_OUT(3)  when A_X12_Out_CRTL='1' else 'Z';

