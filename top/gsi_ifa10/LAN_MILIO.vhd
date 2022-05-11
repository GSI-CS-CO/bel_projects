----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:    2.08.2019
-- Design Name:
-- Module Name:    LAN_MILIO - Behavioral
-- Project Name:   ifa10
-- Target Devices:  MAX10
-- Tool versions:  Quartus 16 +

-- Description:
-- Schnittstelle zum LAN
-- Schaltet den MIL-BUS vom MIL.decoder/encoder aus falls ein registerbit über LAN hier gesetzt wurde


-- Dependencies:


----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;


entity LAN_MILIO is
   generic(
      ADDR_WIDTH  : integer := 12;
      ADDR_WIDTHS : integer := 8
   );

  Port (

    sys_clk       : in  STD_LOGIC; --global system clock
    sys_reset     : in  std_logic; --global reset

    EPIOS_CSn     : in  STD_LOGIC;
    EPIOS_Adr     : in  STD_LOGIC_VECTOR(ADDR_WIDTH-1 Downto 0); --addr (11 Downto 0)
    EPIOS_WR      : in  STD_LOGIC;  --WR
    EPIOS_RD      : in  STD_LOGIC;  --RD
    EPIOS_Frame   : in  STD_LOGIC;  --Frame
    EPIOS_CLK     : in  STD_LOGIC;  --clock

    EPIOS_Dataout : out STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); --Datenbus (15 Downto 0)
    EPIOS_Datain  : in  STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); --Datenbus (15 Downto 0)

    -- Ausgabe für LAN empfangene Manchester Words an IFA-Firmware
    MIL_RCV       : out STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); --Data/Command (15 Downto 0)
    CMD_RCV       : out STD_LOGIC;  -- es eist ein Command-Word
    RCV_RDY       : out STD_LOGIC;  -- Neue daten anliegend
    RCV_Err       : out STD_LOGIC;  -- Data ist ungültig
    RCV_SELLAN    : out STD_LOGIC;  -- Zeigt LAN Priorität an
    nEmpf_en      : in  STD_LOGIC;  -- show that rcv reg can be written

    -- Manchester Words von IFA-Firmware an LAN

    RCV_IRQ       : out  STD_LOGIC;     -- Meldung an die TI-MCU, dass etwas abgeholt werden kann , bisher noch NC

    WR_MIL        : in  STD_LOGIC;      -- Write output data to LAN-Register
    SEND_DATA     : in  STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); --Data to send
    DAT_TYP       : in  STD_LOGIC;      -- 0-Data, 1-CMD type default ist Data, da IFK nur mit Daten antwortet
    MIL_BUSY_RX   : in  STD_LOGIC;      -- Zeigt an ob der MIL-BUS belegt ist, 1- Daten werden gerade empfangen
    MIL_BUSY_TX   : in  STD_LOGIC;      -- Zeigt an ob der MIL-BUS belegt ist, 1- Daten werden gerade gesendet
    MIL_BUSY      : in  STD_LOGIC;      -- Zeigt an ob der MIL-BUS belegt ist, 1- Daten io, 0- keine Busaktivität
    Mute_MILBus   : out STD_LOGIC;      -- 1 - Schaltet den MIL_BUS von der Baustein Ausgabe ab, Werte ist dann nur über LAN sichtbar/Änderbar --Kollisionsvermeidung

    MIL_RCV_in    : in  STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0');
    CMD_RCV_in    : in  STD_LOGIC;      -- Zeigt an ob der MIL-BUS belegt ist, 1- Daten werden gerade empfangen
    RCV_RDY_in    : in  STD_LOGIC;
    RCV_Error_in  : in  STD_LOGIC;

    nLED          : in  STD_LOGIC_VECTOR(27 DOWNTO 0);
    V_Ain         : in  STD_LOGIC_VECTOR(32 DOWNTO 1);
    V_Bin         : in  STD_LOGIC_VECTOR(32 DOWNTO 1);
    V_Cin         : in  STD_LOGIC_VECTOR(32 DOWNTO 1);

    IFA_Adr       : in  STD_LOGIC_VECTOR(7 DOWNTO 0);
    IF_Mode       : in  STD_LOGIC_VECTOR(15 DOWNTO 0);
    VG_Control    : in  STD_LOGIC_VECTOR(23 DOWNTO 0);
    VG_Control_E  : in  STD_LOGIC_VECTOR(24 DOWNTO 0);

    nINL          : in  STD_LOGIC;
    nDRDY         : in  STD_LOGIC;
    nDRQ          : in  STD_LOGIC;

    IN_NINL       : in  STD_LOGIC;
    IN_NRDY       : in  STD_LOGIC;
    IN_NDRQ       : in  STD_LOGIC;

    A_SEL_B       : in  STD_LOGIC_VECTOR(3 DOWNTO 0);

    v_A_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    v_B_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    v_C_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);

    f_A_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    f_B_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);
    f_C_IO        : in STD_LOGIC_VECTOR(32 DOWNTO 1);

    n_enInputs    : in STD_LOGIC;
    n_enOutputs   : in STD_LOGIC;
    HW_SEL_Mode   : in STD_LOGIC_VECTOR(3 DOWNTO 0); --selected mode
    
    IFA_ID        : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    IFA_VERS      : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    
    Sweep_VERS    : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    MB_VERS       : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    FG_VERS       : in STD_LOGIC_VECTOR(7 DOWNTO 0);
    FB_VERS       : in STD_LOGIC_VECTOR(7 DOWNTO 0)

 );

end LAN_MILIO;
--1000 EPIO_Reg_STAT
--1001 EPIO_Reg_RCV_D --Speicher für Data-Word empfangen
--1010 EPIO_Reg_RCV_C --Speicher für Command-Word empfangen
--1011 EPIO_Reg_CRTL

architecture Behavioral of LAN_MILIO is



--local MIL_ modul adresses
constant STATUS_ADR   : STD_LOGIC_VECTOR(7 downto 0) := x"00"; -- Read get MIL status
constant CONTROL_ADR  : STD_LOGIC_VECTOR(7 downto 0) := x"01"; -- R/W  set MIL controls / sys_reset

--MIL encoder/decoder values
constant RCV_C_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"02"; -- Ersatz für Manchester Cmd  in -- Wird local an die IFK übergeben
constant RCV_D_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"03"; -- Ersatz für Manchester Data in -- Wird local an die IFK übergeben

constant MIL_C_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"04";
constant MIL_D_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"05";

constant TX_LAN_ADR   : STD_LOGIC_VECTOR(7 downto 0) := x"06";

constant RDC_ADR      : STD_LOGIC_VECTOR(7 downto 0) := x"07";

signal EPIO_Reg_STAT  : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Speicher für Status
--Status-Register Bits EPIO_Reg_STAT
-- 0  -- 1- Wert zum Abholen in EPIO_Reg_TX_LAN, 0 - Nothing
-- 1  -- 1- Wert zum Abholen in EPIO_Reg_RCV_C, 0 - Nothing
-- 2  -- 1- Wert zum Abholen in EPIO_Reg_RCV_D, 0 - Nothing

-- 3  -- 1- Wort wird gerade empfangen
-- 4  -- 1- Wort wird  gerade gesendet



signal EPIO_Reg_CRTL  : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Speicher für control
-- Bit 0 -- enable/Disable ReceiveCmds/Words by LAN
-- Bit 1 -- 1- MIL output only to LAN register 0- send Output to LAN an MIL-Bus too

signal EPIO_Reg_RCV_C : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Speicher für Manchester empfangen - Command
                                                                          -- tut so, als ob Command vom MIL-Decoder gekommen sind
signal EPIO_Reg_RCV_D : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Speicher für Manchester empfangen - Data
                                                                          -- tut so, als ob Data vom MIL-Decoder gekommen sind

signal EPIO_Reg_MIL_C : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- CommandWert zum MIL-BUS vermitteln
                                                                          -- Dieser Wert wird an den MIL-Bus übergeben

signal EPIO_Reg_MIL_D : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- DatenWert zum MIL-BUS vermitteln
                                                                          -- Dieser Wert wird an den MIL-Bus übergeben

signal EPIO_Reg_TX_LAN: STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0'); -- Wert zum LAN vermitteln
                                                                          -- Die Ausgaben an den MIL-BUS werden hier zum LAN gespiegelt -- vorerst nur Data-Typ


--local modul addresses

CONSTANT C_WR_VG1       : STD_LOGIC_VECTOR(7 downto 0) := x"30";
CONSTANT C_RD_VG1       : STD_LOGIC_VECTOR(7 downto 0) := x"A0";
CONSTANT C_WR_VG2       : STD_LOGIC_VECTOR(7 downto 0) := x"31";
CONSTANT C_RD_VG2       : STD_LOGIC_VECTOR(7 downto 0) := x"A1";
CONSTANT C_WR_VG3       : STD_LOGIC_VECTOR(7 downto 0) := x"32";
CONSTANT C_RD_VG3       : STD_LOGIC_VECTOR(7 downto 0) := x"A2";
CONSTANT C_WR_VG4       : STD_LOGIC_VECTOR(7 downto 0) := x"33";
CONSTANT C_RD_VG4       : STD_LOGIC_VECTOR(7 downto 0) := x"A3";
CONSTANT C_WR_VG5       : STD_LOGIC_VECTOR(7 downto 0) := x"34";
CONSTANT C_RD_VG5       : STD_LOGIC_VECTOR(7 downto 0) := x"A4";
CONSTANT C_WR_VG6       : STD_LOGIC_VECTOR(7 downto 0) := x"35";
CONSTANT C_RD_VG6       : STD_LOGIC_VECTOR(7 downto 0) := x"A5";
--
CONSTANT C_WR_Piggy1    : STD_LOGIC_VECTOR(7 downto 0) := x"36";
CONSTANT C_RD_Piggy1    : STD_LOGIC_VECTOR(7 downto 0) := x"A6";
CONSTANT C_WR_Piggy2    : STD_LOGIC_VECTOR(7 downto 0) := x"37";
CONSTANT C_RD_Piggy2    : STD_LOGIC_VECTOR(7 downto 0) := x"A7";
CONSTANT C_WR_Piggy3    : STD_LOGIC_VECTOR(7 downto 0) := x"38";
CONSTANT C_RD_Piggy3    : STD_LOGIC_VECTOR(7 downto 0) := x"A8";

constant C_RD_IFA_ADR   : STD_LOGIC_VECTOR(7 downto 0) := x"A9";
constant C_RD_MODE      : STD_LOGIC_VECTOR(7 downto 0) := x"AA";

constant C_RD_VGDIR0    : STD_LOGIC_VECTOR(7 downto 0) := x"AB";
constant C_RD_VGDIR1    : STD_LOGIC_VECTOR(7 downto 0) := x"AC";
constant C_RD_VGOE0     : STD_LOGIC_VECTOR(7 downto 0) := x"AD";
constant C_RD_VGOE1     : STD_LOGIC_VECTOR(7 downto 0) := x"AE";

constant C_RD_NIRQ      : STD_LOGIC_VECTOR(7 downto 0) := x"AF"; --nINL,nDRDY,nDRQ

CONSTANT C_RD_LED0      : STD_LOGIC_VECTOR(7 downto 0) := x"B0";
CONSTANT C_RD_LED1      : STD_LOGIC_VECTOR(7 downto 0) := x"B1";

--
CONSTANT C_RD_Out_Reg   : STD_LOGIC_VECTOR(7 downto 0) := x"B2";
CONSTANT C_WR_Out_Reg   : STD_LOGIC_VECTOR(7 downto 0) := x"42";

CONSTANT C_RD_Ti_Data   : STD_LOGIC_VECTOR(7 downto 0) := x"B3";
CONSTANT C_WR_Ti_Data   : STD_LOGIC_VECTOR(7 downto 0) := x"43";  --Timing-Data Register
CONSTANT C_RD_Inp_Reg   : STD_LOGIC_VECTOR(7 downto 0) := x"B4";

CONSTANT C_RD_AIO1      : STD_LOGIC_VECTOR(7 downto 0) := x"B5"; -- Zustand an den IO-Engängen der FPGA-Puffer
CONSTANT C_RD_AIO2      : STD_LOGIC_VECTOR(7 downto 0) := x"B6";
CONSTANT C_RD_BIO1      : STD_LOGIC_VECTOR(7 downto 0) := x"B7";
CONSTANT C_RD_BIO2      : STD_LOGIC_VECTOR(7 downto 0) := x"B8";
CONSTANT C_RD_CIO1      : STD_LOGIC_VECTOR(7 downto 0) := x"B9";
CONSTANT C_RD_CIO2      : STD_LOGIC_VECTOR(7 downto 0) := x"BA";

CONSTANT C_RD_FAIO1     : STD_LOGIC_VECTOR(7 downto 0) := x"D0"; --read IO auswahl der FPGA Puffer 1- Ausgang , 0-Eingang
CONSTANT C_RD_FAIO2     : STD_LOGIC_VECTOR(7 downto 0) := x"D1";
CONSTANT C_RD_FBIO1     : STD_LOGIC_VECTOR(7 downto 0) := x"D2";
CONSTANT C_RD_FBIO2     : STD_LOGIC_VECTOR(7 downto 0) := x"D3";
CONSTANT C_RD_FCIO1     : STD_LOGIC_VECTOR(7 downto 0) := x"D4";
CONSTANT C_RD_FCIO2     : STD_LOGIC_VECTOR(7 downto 0) := x"D5";


CONSTANT C_RD_FPGAIDVERS : STD_LOGIC_VECTOR(7 downto 0) := x"C0";  -- Get FPGA_Version / IFA _ID

CONSTANT C_RD_BUSIQ_SEL  : STD_LOGIC_VECTOR(7 downto 0) := x"C1";  -- Get BusIRQ_status and SEL_Value

CONSTANT C_RD_SWEEPFGVERS: STD_LOGIC_VECTOR(7 downto 0) := x"C2";  -- Get Sweep_VERS / FG_VERS

CONSTANT C_RD_FBMBVERS   : STD_LOGIC_VECTOR(7 downto 0) := x"C3";  -- Get FB_Version /MB_VERS
--
--CONSTANT C_WR_OE        : STD_LOGIC_VECTOR(7 downto 0) := x"39";
--CONSTANT C_RD_OE        : STD_LOGIC_VECTOR(7 downto 0) := x"A9";

--CONSTANT C_WR_DIR       : STD_LOGIC_VECTOR(7 downto 0) := x"3A";
--CONSTANT C_RD_DIR       : STD_LOGIC_VECTOR(7 downto 0) := x"AA";


--CONSTANT C_WR_OE1       : STD_LOGIC_VECTOR(7 downto 0) := x"3E";
--CONSTANT C_RD_OE1       : STD_LOGIC_VECTOR(7 downto 0) := x"AE";

--CONSTANT C_WR_DIR1      : STD_LOGIC_VECTOR(7 downto 0) := x"3F";
--CONSTANT C_RD_DIR1      : STD_LOGIC_VECTOR(7 downto 0) := x"AF";

--signal RCV_LAN_Accept : std_logic := '0'; --


--signal EPIOS_CLK1     : std_logic := '0'; -- CLK Speicher

signal Start_Man_out  : std_logic := '0'; -- Ausgabe aus Mandecoder starten
--signal Start_Man_out1 : std_logic := '0'; -- Ausgabe aus Mandecoder starten puffer
--signal ItsaDWORD      : std_logic := '0'; -- Datenwort aus LAN empfangen und
--signal ItsaCWORD      : std_logic := '0'; -- Commandwort aus LAN empfangen und

signal Rcvrdy1        : std_logic := '0'; -- Strobe für RDY ausgabe
signal Rcvrdy2        : std_logic := '0'; -- Strobe für RDY ausgabe

signal EPIOS_WR1      : std_logic := '0'; -- speichert wr Pulse --explicit! issue
signal EPIOS_RD1      : std_logic := '0'; -- speichert RD-Pulse --explicit! issue

signal sel_LANorMIL   : std_logic := '0'; -- -0- MIL 1-LAN

signal MIL_RCV_L      : STD_LOGIC_VECTOR(15 downto 0) := ( others =>'0');
signal CMD_RCV_L      : std_logic := '0';
signal RCV_RDY_L      : std_logic := '0';
signal RCV_Err_L      : std_logic := '0';

--type read_states is (
--  RD_0,
--  RD_1,
--  RD_2,
--  RD_3
--);
--
--signal   read_state : read_states := RD_0;
--
--
--type LAN_instates is (
--  LD_0,
--  LD_1,
--  LD_2,
--  LD_3
--);
--
--signal   LAN_instate : LAN_instates := LD_0;



Begin
---------------------------------

-- Select der MIL-Quelle  zwischen LAN und MIL6408 umschalten
-- Weiterleitung der MIL-Device signale, falls LAN nicht Priorität haben soll
MIL_RCV <=  MIL_RCV_in   when sel_LANorMIL ='0' else MIL_RCV_L;
CMD_RCV <=  CMD_RCV_in   when sel_LANorMIL ='0' else CMD_RCV_L;
RCV_RDY <=  RCV_RDY_in   when sel_LANorMIL ='0' else RCV_RDY_L;
RCV_Err <=  RCV_Error_in when sel_LANorMIL ='0' else RCV_Err_L;

RCV_IRQ <= '0';-- bisher keine Meldung an die MCU

--epiosclk: process(sys_clk,sys_reset,EPIOS_CLK)
--
-- begin
--  if rising_edge(sys_clk) then
--     if sys_reset = '1' then
--        EPIOS_CLK1 <= '0';
--     else
--        EPIOS_CLK1 <= EPIOS_CLK;
--     end if;
--  end if;
--
--end process;


--constant STATUS_ADR   : STD_LOGIC_VECTOR(7 downto 0) := x"00"; -- Read get MIL status
--constant CONTROL_ADR  : STD_LOGIC_VECTOR(7 downto 0) := x"01"; -- R/W  set MIL controls / sys_reset
--
--MIL encoder/decoder values:
--constant CMD_ADR      : STD_LOGIC_VECTOR(7 downto 0) := x"02"; -- Wird Local an die IFK übergeben
--constant DAT_ADR      : STD_LOGIC_VECTOR(7 downto 0) := x"03"; -- Wird Local an die IFK übergeben
--
--constant TX_LAN_ADR   : STD_LOGIC_VECTOR(7 downto 0) := x"04";
--constant MIL_D_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"05";
--constant MIL_C_ADR    : STD_LOGIC_VECTOR(7 downto 0) := x"06";
--
--constant RDC_ADR      : STD_LOGIC_VECTOR(7 downto 0) := x"07";


--Liest MIL-Zustand und die 96 VGA IOs und LED -Zustände
epiosrd_MIL_IOREG: process(sys_clk,sys_reset,EPIOS_RD1,EPIOS_RD,EPIOS_adr,EPIO_Reg_STAT,EPIO_Reg_CRTL,
                        EPIO_Reg_RCV_C,EPIO_Reg_RCV_D,EPIO_Reg_TX_LAN,IN_NINL,IN_NRDY,IN_NDRQ,A_SEL_B    )

 begin

    if rising_edge(sys_clk) then
      if sys_reset = '1' then
         EPIOS_RD1<='0';
     --   rdout <='0';
      else

        if EPIOS_RD1 = '0' and EPIOS_RD = '1' then --LH-Flanke an RD-Leitung?
        --  rdout <='1';
          EPIOS_Dataout<= ( others =>'0');  --Default value
          -- Wordpuffer-Daten setzen
          case EPIOS_adr(ADDR_WIDTHS-1 downto 0) is
            when STATUS_ADR =>
                            EPIOS_Dataout <= EPIO_Reg_STAT;
                            --rdout <='1';

            when CONTROL_ADR =>
                            EPIOS_Dataout <= EPIO_Reg_CRTL;       -- Wordpuffer-Daten setzen
                            --rdout <='1';                        -- zeigt z.B. an, dass Wort empfangen und gelesen werden kann

            when RCV_C_ADR =>
                            EPIOS_Dataout <= EPIO_Reg_RCV_C;

            when RCV_D_ADR =>
                            EPIOS_Dataout <= EPIO_Reg_RCV_D;

            when TX_LAN_ADR =>
                            EPIOS_Dataout <= EPIO_Reg_TX_LAN;
-----------------------------
-- Ab hier VG Inputs und LEDs lesen
            when C_RD_VG1 =>
                              EPIOS_Dataout<= V_Ain(16 downto 1);   -- Mux VG1 ..
            when C_RD_VG2 =>
                              EPIOS_Dataout<= V_Ain(32 downto 17);
            when C_RD_VG3 =>
                              EPIOS_Dataout<= V_Bin(16 downto 1);
            when C_RD_VG4 =>
                              EPIOS_Dataout<= V_Bin(32 downto 17);
            when C_RD_VG5 =>
                              EPIOS_Dataout<= V_Cin(16 downto 1);
            when C_RD_VG6 =>
                              EPIOS_Dataout<= V_Cin(32 downto 17);

            when C_RD_LED0=>
                              EPIOS_Dataout<= not nLED(15 downto 0);

            when C_RD_LED1=>
                              EPIOS_Dataout(11 downto 0) <= not nLED(27 downto 16);

            when C_RD_VGDIR0 =>
                              EPIOS_Dataout  <= VG_Control(15 downto 0);

            when C_RD_VGDIR1 =>
                              EPIOS_Dataout(7 downto 0) <= VG_Control(23 downto 16);

            when C_RD_VGOE0 =>
                              EPIOS_Dataout<= VG_Control_E(15 downto 0);

            when C_RD_VGOE1 =>
                              EPIOS_Dataout(8 downto 0) <= VG_Control_E(24 downto 16);
                              EPIOS_Dataout(9)  <= n_enInputs;
                              EPIOS_Dataout(10) <= n_enOutputs;

            when C_RD_MODE=>
                              EPIOS_Dataout <= IF_Mode;

            when C_RD_IFA_ADR=>
                              EPIOS_Dataout(7 downto 0) <=  IFA_Adr;

            when C_RD_NIRQ =>
                              EPIOS_Dataout(2 downto 0) <= nINL & nDRDY & nDRQ;

            when C_RD_FPGAIDVERS =>
                              EPIOS_Dataout <= IFA_VERS & IFA_ID;

            when C_RD_SWEEPFGVERS =>
                              EPIOS_Dataout <= Sweep_VERS & FG_VERS;

            when C_RD_FBMBVERS => 
                              EPIOS_Dataout <= MB_VERS & FB_VERS;
            
            when C_RD_BUSIQ_SEL  =>
                              EPIOS_Dataout <= "0000" & HW_SEL_Mode &"0" & IN_NINL & IN_NRDY & IN_NDRQ & A_SEL_B;

            when C_RD_AIO1       =>
                              EPIOS_Dataout <= v_A_IO(16 downto 1);

            when C_RD_AIO2       =>
                              EPIOS_Dataout <= v_A_IO(32 downto 17);

            when C_RD_BIO1       =>
                              EPIOS_Dataout <= v_B_IO(16 downto 1);

            when C_RD_BIO2       =>
                              EPIOS_Dataout <= v_B_IO(32 downto 17);

            when C_RD_CIO1       =>
                              EPIOS_Dataout <= v_C_IO(16 downto 1);

            when C_RD_CIO2       =>
                              EPIOS_Dataout <= v_C_IO(32 downto 17);

            when C_RD_FAIO1      =>
                              EPIOS_Dataout <= f_A_IO(16 downto 1);

            when C_RD_FAIO2      =>
                              EPIOS_Dataout <= f_A_IO(32 downto 17);

            when C_RD_FBIO1      =>
                              EPIOS_Dataout <= f_B_IO(16 downto 1);

            when C_RD_FBIO2      =>
                              EPIOS_Dataout <= f_B_IO(32 downto 17);

            when C_RD_FCIO1      =>
                              EPIOS_Dataout <= f_C_IO(16 downto 1);

            when C_RD_FCIO2      =>
                              EPIOS_Dataout <= f_C_IO(32 downto 17);

            when others =>
                              EPIOS_Dataout <= ( others =>'0');

                             -- rdout <='0';
          end case;
       end if; --if EPIOS_RD1 = '0' and EPIOS_RD = '1' then
    end if; --sys_reset
  end if; --rising_edge(sys_clk)
end process;


--Register setzen -- für MIL-Empfang
--Simuliert ein empfangenes Word aus dem MIL-Decoder, wird über LAN-Port gesetzt
epioswr2: process(sys_clk,sys_reset,EPIOS_adr,EPIOS_Datain,EPIOS_CLK,EPIOS_WR,EPIOS_WR1,Start_Man_out,Rcvrdy2) --Rcvrdy3 EPIOS_CLK1,

 begin

    if rising_edge(sys_clk) then

      if sys_reset = '1' then
        --ItsaDWORD       <= '0';
        --ItsaCWORD       <= '0';
        EPIO_Reg_RCV_C  <= ( others => '0');
        EPIO_Reg_RCV_D  <= ( others => '0');
        EPIO_Reg_CRTL   <= ( others => '0');
       -- RCV_LAN_Accept  <= '0';  --Timing-noch klären.
        RCV_Err_L       <= '0';
        CMD_RCV_L       <= '0';  --command or data type
        Rcvrdy1         <= '0';
        EPIOS_WR1       <= '0';
       -- start_Man_out1  <= '0';
      else
        --Start_Man_out1 <= Start_Man_out;
        Rcvrdy1 <=  '0';

        if EPIOS_WR1 = '0' and EPIOS_WR = '1' then
         -- ItsaDWORD <= '0';   --C und D sys_reset
          --ItsaCWORD <= '0';
          Start_Man_out <= '0';  --flag für reset vom Empfangsprozess

          case EPIOS_adr(ADDR_WIDTHS-1 downto 0) is

            when RCV_C_ADR =>       -- Command-Word wurde empfangen
                          Start_Man_out   <= '1';  --flag für Empfangsprozess setzen
                          --RCV_LAN_Accept  <= '1';  --Timing-noch klären.
                          EPIO_Reg_RCV_C  <= EPIOS_Datain;
                          MIL_RCV_L       <= EPIOS_Datain;
                       --   ItsaDWORD       <= '0';
                        --  ItsaCWORD       <= '1';
                          CMD_RCV_L         <= '1';
                          RCV_Err_L         <= '0';

            when RCV_D_ADR =>   -- Data-Word wurde empfangen
                          Start_Man_out   <= '1';  --flag für Empfangsprozess setzen
                          --RCV_LAN_Accept  <= '1';  --Timing-noch klären.
                          EPIO_Reg_RCV_D  <= EPIOS_Datain;
                          MIL_RCV_L       <= EPIOS_Datain;
                        --  ItsaDWORD       <= '1';   --C und D
                        --  ItsaCWORD       <= '0';
                          CMD_RCV_L       <= '0';
                          RCV_Err_L       <= '0';

            when CONTROL_ADR => -- Kontrollregister, u.a. um LAN zu MIL
                        --  ItsaDWORD <= '0';   --C und D sys_reset
                        --  ItsaCWORD <= '0';
                          EPIO_Reg_CRTL <= EPIOS_Datain;

            when others =>

          end case;

          if Start_Man_out = '1' then
            Rcvrdy1 <= '1';  -- im folgenden Takt 1x auf 1 setzen
          end if;

        end if; --if EPIOS_WR = '1' and EPIOS_WR1 = '0' then

        --RCV_LAN_Accept puls wieder abschalten
      --  if Rcvrdy2 = '0' and Rcvrdy3 = '1' then
      --    RCV_LAN_Accept <= '0';  --Timing-noch klären.
      --  end if; --not rst
      end if; --not rst
  end if; --rising_edge(sys_clk)

 end process;


RCV_SELLAN  <= sel_LANorMIL;

sel_LANorMIL <= EPIO_Reg_CRTL(0); -- 0 - select MIL-device, 1 - Eingangsdaten kommen von LAN
Mute_MILBus  <= EPIO_Reg_CRTL(1); -- 0- Encoderdaten werden in LAN und MIL-Encoder abgelegt,
                                 --  1 -Encoderdaten werden nur für LAN hinterlegt, es wird nichts über MIL-Bus gesendet


RDYStrobe: process(sys_clk,sys_reset,Rcvrdy1,Rcvrdy2)

 begin
    if rising_edge(sys_clk) then
      if sys_reset = '1' then
        RCV_RDY_L <=  '0';
        Rcvrdy2   <=  '0';
        --Rcvrdy3   <=  '0';
      else
        Rcvrdy2   <= Rcvrdy1;
        --Rcvrdy3   <= Rcvrdy2;
        RCV_RDY_L <= Rcvrdy2;  -- Strobe verlängern
      end if;
    end if; --if rising_edge(sys_clk) then

 end process;


 -- MILWort ans LAN senden aus IFK
 setupMILLAN: process(sys_clk,sys_reset,WR_MIL,SEND_DATA)

 begin

    if rising_edge(sys_clk) then

      if sys_reset = '1' then
        EPIO_Reg_TX_LAN <= ( others => '0');
      else
        if WR_MIL = '1' then
           EPIO_Reg_TX_LAN <= SEND_DATA;          -- vorerst nur Data-Typ
        end if;
      end if;
  end if;

 end process;


-- Statusregister Zustände definieren
 setupStatus: process(sys_clk,sys_reset,WR_MIL,nEmpf_en,EPIOS_adr,EPIOS_RD1,EPIOS_RD)

 begin

    if rising_edge(sys_clk) then

      if sys_reset = '1' then
        EPIO_Reg_STAT <= ( others => '0');

      else
        EPIO_Reg_STAT(3) <= MIL_BUSY_RX;  --MIL-BUS durch Empfang noch belegt
        EPIO_Reg_STAT(4) <= MIL_BUSY_TX;  --MIL-BUS durch Senden noch belegt
        EPIO_Reg_STAT(4) <= MIL_BUSY;     --MIL-Bus ist aktiv
        EPIO_Reg_STAT(1) <= nEmpf_en;     --bisher default 1 - Nicht verwendet
        if WR_MIL = '1' then              --wird
          EPIO_Reg_STAT(0) <= '1';        --Wort ist zum Abholen bereit
        else
          --wir das Register EPIO_Reg_TX_LAN gerade gelesen? dann reset des Statusbits
          if EPIOS_RD1 = '0' and EPIOS_RD = '1' then
            if EPIOS_adr(ADDR_WIDTHS-1 downto 0) = TX_LAN_ADR then
              EPIO_Reg_STAT(0) <= '0'; --reset Bit bei Zugriff
            end if;
          end if;
        end if;

      end if;
  end if;

 end process;

 --Später einbauen...
 ---- MILWort über LAN erhalten?
-- setupMILRCV: process(sys_clk,sys_reset,ItsaCWORD,ItsaDWORD)
--
-- begin
--
--    if rising_edge(sys_clk) then
--
--      if sys_reset = '1' then
--        --Rcvrdy1   <= '0';    -- puls für
--      else
--        --Rcvrdy1 <= '0';
--        if Start_Man_out1 = '0' and Start_Man_out = '1' then -- LH Flanke?
--          if ItsaCWORD = '1' or ItsaDWORD = '1' then -- MIL über LAN erhalten?
--            --Rcvrdy1 <= '1';
--          end if;
--        end if;
--      end if;
--  end if;
--
-- end process;
--

----Umschalten der MIL-Kommandos auf LAN
--LanselStrobe: process(sys_clk,sys_reset)
--
-- begin
--    if rising_edge(sys_clk) then
--      if sys_reset='1' then
--        LAN_instate <= LD_0;
--        --RCV_SELLAN <= '0';
--      else
--        case LAN_instate is
--           when LD_0 =>
--                        LAN_instate  <=  LD_1;
--           when LD_1 =>
--           when LD_2 =>
--
--         --  RCV_SELLAN<='1';
--
--           when others =>
--            LAN_instate  <=  LD_0;
--        end case;
--      end if;
--    end if; --if rising_edge(sys_clk) then
--
-- end process;


end Behavioral;

