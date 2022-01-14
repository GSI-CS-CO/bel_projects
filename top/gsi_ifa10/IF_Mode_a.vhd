----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IF_Mode_a.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices:
-- Tool versions:
-- Description:
-- Generiert den Auswahlvektor für die IO-Signale enable/output/input
-- SCU_Mode,MB_Mode,IFA_Mode und TEST_Mode
-- mit dem Wahlschalter kann zwischen TEST_MODE und den Betriebsmodi umgeschaltet werden
-- SELECT =0xF -> Testmode
-- SELECT =0-0xE -> SCU_Mode oder B_Mode oder IFA_Mode abhängig von den eingangssignalen
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- VK added 122 and 112 Mode
-- Für den Betrieb als 221 muss SWITCH auf 7 stehen

----------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;


library work;
use work.ifk10_pkg.all;

ENTITY IF_Mode_a IS

  PORT
  (
   sys_clk     : in  std_logic;     -- System-Clock
   sys_reset   : in  std_logic;     -- syn. Clear
   A_SEL_B     : in  STD_LOGIC_VECTOR(3 DOWNTO 0); -- Signale vom Select-Schalter
   HW_SEL_MB   : in  std_logic;     -- Hardware Select: Interface-Karte fest auf Modul-Bus-Mode eingestellt
   HW_SEL_SCUB : in  std_logic;     -- Hardware Select: Interface-Karte fest auf SCU-Bus-Mode eingestellt
   IF_Mode_Reg : in  STD_LOGIC_VECTOR(15 downto 0);         -- mode register für die Funktionsumschaltung
  
   HW_SEL_MB_out  : out  std_logic; -- Hardware Select: Interface-Karte fest auf Modul-Bus-Mode eingestellt
   HW_SEL_SCUB_out: out  std_logic; -- Hardware Select: Interface-Karte fest auf SCU-Bus-Mode eingestellt
   IF_Mode     : out STD_LOGIC_VECTOR(15 downto 0);   -- Interface-Karten-Mode
   IFA_Mode    : out std_logic;     -- Interface-Karten-SEL
   FG_Mode     : out std_logic;     -- Funktionsgen.-SEL
   FG_DDS_Mode : out std_logic;     -- Funktionsgen.-SEL
   MB_Mode     : out std_logic;     -- Modulbus-SEL
   SCU_Mode    : out std_logic;     -- SCU-BUS-Mode
   SCU_ACU_Mode: out std_logic;     -- SCU_ACU_Mode
   Sweep_Mode  : out std_logic;     -- Sweeper-SEL
   FG112_Mode  : out std_logic;     -- card 112 selected
   FG122_Mode  : out std_logic;     -- card 122 selected
   FG203_Mode  : out std_logic;     -- card 203 selected
   FG203B_Mode : out std_logic;     -- card 203 selected
   FG211_Mode  : out std_logic;     -- card 211 selected

   HW_SEL_Mode : out STD_LOGIC_VECTOR(3 DOWNTO 0); -- Welche Harware wird ausgewählt
   IFA_id      : out STD_LOGIC_VECTOR(7 DOWNTO 0)  -- Welche Harware wird ausgewählt

);

END IF_Mode_a;


architecture Behavioral of IF_Mode_a is


signal v_IFA_Mode    : STD_LOGIC :='0' ;
signal v_FG_Mode     : STD_LOGIC :='0' ;
signal v_FG_DDS_Mode : STD_LOGIC :='0' ;
signal v_MB_Mode     : STD_LOGIC :='0' ;
signal v_SCU_Mode    : STD_LOGIC :='0' ;
signal v_SCU_ACU_Mode: STD_LOGIC :='0' ;
signal v_Sweep_Mode  : STD_LOGIC :='0' ;

signal v_FG112_Mode  : STD_LOGIC :='0' ;
signal v_FG122_Mode  : STD_LOGIC :='0' ;

signal v_FG203_Mode  : STD_LOGIC :='0' ;
signal v_FG203B_Mode : STD_LOGIC :='0' ;
signal v_FG211_Mode  : STD_LOGIC :='0' ;


 --signal Sweep_En    :  std_logic:='1';     -- Sweeper-Modul Enable
  -- signal FG_En       : std_logic:='1';     -- Funktionsg. Enable
   -- signal FG_DDS_En   : std_logic:='1';     -- Funktionsg. mit DDS-Interface Enable


begin


--Hardware IOBUF auswählen
--Sub-Modi setzen
--
setvMode: process(sys_clk,sys_reset,HW_SEL_MB,HW_SEL_SCUB,IF_Mode_Reg,A_SEL_B)

begin
 if sys_reset='1'then

      IF_Mode        <= ( others =>'0');
      v_IFA_Mode     <='0'; -- clear IFA_Mode
      v_MB_Mode      <='0'; -- clear MB_Mode
      --sub modes
      v_FG_Mode      <='0'; -- clear FG_Mode
      v_FG_DDS_Mode  <='0'; -- clear FG-DDS_Mode
      v_SCU_Mode     <='0'; -- clear SCU_Mode
      v_SCU_ACU_Mode <='0'; -- clear SCU_ACU_Mode
      v_Sweep_Mode   <='0'; -- clear Sweep_Mode

      v_FG112_Mode   <= '0';
      v_FG122_Mode   <= '0';
      v_FG203_Mode   <= '0';
      v_FG203B_Mode  <= '0';
      v_FG211_Mode   <= '0';

      HW_SEL_MB_out  <= '0';
      HW_SEL_SCUB_out<= '0';

      HW_SEL_Mode    <= C_HW_NO_MODE;
      IFA_id         <= IFA_ID_NO;

    elsif rising_edge(sys_clk) then

         IF_Mode        <= C_IFA_MODE; 
         v_IFA_Mode     <= '0'; -- clear IFA_Mode
         v_MB_Mode      <= '0'; -- clear MB_Mode
         v_SCU_Mode     <= '0'; -- clear SCU_Mode
         v_FG112_Mode   <= '0';
         v_FG122_Mode   <= '0';
         v_FG203_Mode   <= '0';
         v_FG203B_Mode  <= '0';
         v_FG211_Mode   <= '0';

         --sub Modes
         v_FG_Mode      <= '0'; -- clear FG_Mode
         v_FG_DDS_Mode  <= '0'; -- clear FG-DDS_Mode
         v_SCU_ACU_Mode <= '0'; -- clear SCU+SCU_ACU_Mode
         v_Sweep_Mode   <= '0'; -- clear Sweep_Mode

         HW_SEL_MB_out  <= '0'; 
         HW_SEL_SCUB_out<= '0';

         HW_SEL_Mode    <= C_HW_NO_MODE;
         IFA_id         <= IFA_ID_NO;


      case A_SEL_B is
      
         when C_HW_SW_IFA230 =>  --0x00  -- select 230 Mode

            IFA_id            <= IFA_ID_230;
            --Die Hardware-Select-Leitungen in den anderen Modi weitergeben
            HW_SEL_MB_out     <= HW_SEL_MB;
            HW_SEL_SCUB_out   <= HW_SEL_SCUB;
            --Es darf nur eines von beiden Signalen aktiv sein
            --sind SCUB und MB beide gesetzt, dann ist das im ifa_crtl Register zu erkennen

            IF HW_SEL_MB='1' AND ( HW_SEL_SCUB='0') THEN    -- nur mit dem MB-Sel-Signal von VG-Leiste ==> Modul-Bus-Mode
               v_MB_Mode   <= '1';                          -- set   MB_Mode
               IF_Mode     <= C_MB_MODE;
               HW_SEL_Mode <= C_HW_MB_MODE;
               
            ELSIF  HW_SEL_SCUB='1' AND ( HW_SEL_MB='0') THEN   -- nur mit dem SCU-Select-Signal von VG-Leiste ==> FAIR-Bus-Mode 
            
               v_SCU_Mode  <='1'; -- set   SCU_Mode
               HW_SEL_Mode <= C_HW_SCU_MODE;
               
               IF (IF_Mode_Reg= C_SCU_ACU_Mode)   THEN     -- und Software-Mode SCU_ACU ==> IFA im SCU_ACU-Mode
                  v_SCU_ACU_Mode<='1'; -- set SCU_ACU_Mode        -- Check
                  IF_Mode       <= C_SCU_ACU_MODE;    
               ELSE              
                  v_SCU_ACU_Mode<='0'; -- no SCU_ACU_Mode
                  IF_Mode       <= C_SCU_MODE;     
               END IF;
               
            ELSE
               HW_SEL_Mode       <= C_HW_IFA_MODE; --default ist ersteinmal IFA-Mode, falls nicht von SCU oder MB überschrieben

                IF (IF_Mode_Reg = C_FG_Mode) THEN -- AND FG_EN='1')
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     IF_Mode       <= C_FG_MODE;                                    
               ELSIF (IF_Mode_Reg = C_FG_DDS_Mode)  THEN --AND FG_DDS_EN='1')
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     v_FG_DDS_Mode <='1'; -- set   FG-DDS_Mode
                     IF_Mode       <= C_FG_DDS_Mode;                       
               ELSIF (IF_Mode_Reg = C_Sweep_Mode)  THEN  --AND Sweep_EN='1'
                     v_Sweep_Mode  <='1'; -- set   Sweep_Mode
                     IF_Mode       <= C_SWEEP_MODE;                        
               ELSE
                     v_IFA_Mode    <='1'; -- set   IFA_Mode
                     IF_Mode       <= C_IFA_MODE;                 
               END IF;
               
            END IF;

         when C_HW_SW_IFAIOMODE =>  --0x01  -- select Mode IO-Bus

            IFA_id            <= IFA_ID_230;
            HW_SEL_Mode       <= C_HW_IFA_MODE; --default ist ersteinmal IFA-Mode, falls nicht von SCU oder MB überschrieben
            
            v_IFA_Mode    <='1'; -- bei  IFA_Mode bleiben
            IF_Mode       <= C_IO_BUS_MODE;-- Check

         when C_HW_SW_IFA112 => --Modus für 112 card, sw =0x02
            --bei 112 sind MB und FB select NC
            IFA_id      <= IFA_ID_112;
            v_IFA_Mode  <= '1'; -- set IFA_Mode  -- IFA Mode bleibt wegen Register MILIO erhalten
            v_FG112_Mode<= '1';
            IF_Mode     <= C_112_MODE;
            HW_SEL_Mode <= C_HW_112_MODE;

         when C_HW_SW_IFA122 => --Modus für 122 card , sw =0x03
         --bei 122 sind MB und FB select NC
            IFA_id      <= IFA_ID_122;
            v_IFA_Mode  <= '1'; -- set   IFA_Mode  -- IFA Mode bleibt wegen Register MILIO erhalten
            v_FG122_Mode<= '1';
            IF_Mode     <= C_122_MODE;
            HW_SEL_Mode <= C_HW_122_MODE;

         when C_HW_SW_IFA203 => --Modus für 203 card   , sw =0x04
            --bei 203 sind MB und FB select NC
            IFA_id      <= IFA_ID_203;
            v_IFA_Mode  <= '1'; -- set   IFA_Mode  -- IFA Mode bleibt wegen Register MILIO erhalten
            v_FG203_Mode<= '1'; -- für die Detailauswahl
            IF_Mode     <= C_203_MODE;             -- 203 läuft nur im IFA-Mode
            HW_SEL_Mode <= C_HW_IFA_MODE;


         when C_HW_SW_IFA203B => --Modus für 203B card    , sw =0x05
            --bei 203 sind MB und FB select NC
            IFA_id      <= IFA_ID_203;
            v_IFA_Mode  <= '1'; -- set   IFA_Mode  -- IFA Mode bleibt wegen Register MILIO erhalten
            v_FG203B_Mode<= '1'; -- für die Detailauswahl
            IF_Mode     <= C_203B_MODE;  --203B läuft nur im IFA-Mode
            HW_SEL_Mode <= C_HW_IFA_MODE;

         when C_HW_SW_IFA211 => --Modus für 211 card, sw =0x06
            --bei 211 ist nur MB select aktiv
            IFA_id         <= IFA_ID_211;
            v_FG211_Mode   <= '1';

            IF HW_SEL_MB = '1' THEN    -- nur mit dem MB-Sel-Signal von VG-Leiste ==> Modul-Bus-Mode
               v_MB_Mode   <= '1';     -- set MB_Mode
               IF_Mode     <= C_MB_MODE;
               HW_SEL_Mode <= C_HW_MB_MODE;
               HW_SEL_MB_out  <= HW_SEL_MB; --nur MB Leitung weitergeben ,211 kennt nur MB-Mode
            else
               HW_SEL_Mode       <= C_HW_IFA_MODE; --default ist erstmal IFA-Mode, falls nicht von MB überschrieben

               IF (IF_Mode_Reg = C_FG_Mode) THEN --AND FG_EN='1') 
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     IF_Mode       <= C_FG_MODE;
               ELSIF (IF_Mode_Reg = C_FG_DDS_Mode) THEN --AND FG_DDS_EN='1')
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     v_FG_DDS_Mode <='1'; -- set   FG-DDS_Mode
                     IF_Mode       <= C_FG_DDS_Mode;
               ELSIF (IF_Mode_Reg = C_Sweep_Mode) THEN --AND Sweep_EN='1'
                     v_Sweep_Mode  <='1'; -- set   Sweep_Mode
                     IF_Mode       <= C_SWEEP_MODE;
               ELSE
                     v_IFA_Mode    <='1'; -- set   IFA_Mode
                     IF_Mode       <= C_IFA_MODE;
               END IF;
            END IF;


         when C_HW_SW_IFA221 =>  --0x07  -- select 221 Mode
            IFA_id            <= IFA_ID_221;
            --Die Hardware-Select-Leitungen in den anderen Modi weitergeben
           -- HW_SEL_SCUB_out   <= '0';
            --HW_SEL_MB_out     <= '0';
            --Es darf nur eines von beiden aktiv sein
            --sind SCUB und MB beide gesetzt, dann ist das in ifa_crtl zu lesen
            HW_SEL_MB_out     <= HW_SEL_MB;
            HW_SEL_SCUB_out   <= HW_SEL_SCUB;

            IF HW_SEL_MB='1' AND ( HW_SEL_SCUB='0') THEN   -- nur mit dem MB-Sel-Signal von VG-Leiste ==> Modul-Bus-Mode
               v_MB_Mode   <= '1'; -- set   MB_Mode
               IF_Mode     <= C_MB_MODE;
               HW_SEL_Mode <= C_HW_MB_MODE;

            ELSIF  HW_SEL_SCUB='1' AND ( HW_SEL_MB='0') THEN   -- nur mit dem SCU-Select-Signal von VG-Leiste ==> FAIR-Bus-Mode

               v_SCU_Mode  <='1'; -- set   SCU_Mode
               HW_SEL_Mode <= C_HW_SCU_MODE;

               IF (IF_Mode_Reg= C_SCU_ACU_Mode)   THEN     -- und Software-Mode SCU_ACU ==> IFA im SCU_ACU-Mode
                  v_SCU_ACU_Mode<='1'; -- set SCU_ACU_Mode        -- Check
                  IF_Mode       <= C_SCU_ACU_MODE;
               ELSE
                  v_SCU_ACU_Mode<='0'; -- no SCU_ACU_Mode
                  IF_Mode       <= C_SCU_MODE;
               END IF;

            ELSE
               HW_SEL_Mode       <= C_HW_IFA_MODE; --default ist erst mal IFA-Mode, falls nicht von SCU oder MB vorher überschrieben

               IF (IF_Mode_Reg = C_FG_Mode) THEN --AND FG_EN='1') 
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     IF_Mode       <= C_FG_MODE;
               ELSIF (IF_Mode_Reg = C_FG_DDS_Mode)  THEN --AND FG_DDS_EN='1')
                     v_FG_Mode     <='1'; -- set   FG_Mode
                     v_FG_DDS_Mode <='1'; -- set   FG-DDS_Mode
                     IF_Mode       <= C_FG_DDS_Mode;
               ELSIF (IF_Mode_Reg = C_Sweep_Mode) THEN --AND Sweep_EN='1'
                     v_Sweep_Mode  <='1'; -- set   Sweep_Mode
                     IF_Mode       <= C_SWEEP_MODE;
               ELSE
                     v_IFA_Mode    <='1'; -- set   IFA_Mode
                     IF_Mode       <= C_IFA_MODE;
               END IF;
            END IF;

            
         when C_HW_SW_TSTZ =>  -- sw =0xE
            --kein MB und FB select im Testmode
            v_IFA_Mode   <='1'; -- set   IF   A_Mode
            IF_Mode      <= C_IFA_MODE;
            HW_SEL_Mode  <= C_HW_TEST_MODEZ;
            IFA_id       <= IFA_ID_TEST2;

         when C_HW_SW_TST =>  --sw =0xF
            --kein MB und FB select im TestmodeFG_DDS_EN
            v_IFA_Mode  <='1'; -- set   IFA_Mode
            IF_Mode     <= C_IFA_MODE;
            HW_SEL_Mode <= C_HW_TEST_MODE;
            IFA_id      <= IFA_ID_TEST;


          when others  =>

     end case;
   end if; --rising_edge(sys_clk) then
end process;


IFA_Mode    <= v_IFA_Mode;
MB_Mode     <= v_MB_Mode;
SCU_Mode    <= v_SCU_Mode;
FG112_Mode  <= v_FG112_Mode;
FG122_Mode  <= v_FG122_Mode;
FG203_Mode  <= v_FG203_Mode;
FG203B_Mode <= v_FG203B_Mode;
FG211_Mode  <= v_FG211_Mode;


FG_Mode     <= v_FG_Mode;
FG_DDS_Mode <= v_FG_DDS_Mode;

SCU_ACU_Mode<= v_SCU_ACU_Mode;
Sweep_Mode  <= v_Sweep_Mode;


end Behavioral;


--         if FG203_Mode='1' or FG203B_Mode='1' then -- bei 203 gibt es noch kein MB und FB /SCU select
--         Sel_MB_in      <= '0';
--      else
--         Sel_MB_in      <= not V_Bin(7);  --SWT4  -- Achtung: Kollision mit .203 B7 als NINL
--      end if;


--      if FG203_Mode='1' or FG203B_Mode='1'  then
--         Sel_FB_in      <= '0';
--    else
--         Sel_FB_in      <= not V_Bin(8);
--      end if;


         -- v_IFA_Mode_s  <= v_IFA_Mode OR v_FG_Mode OR v_Sweep_Mode OR v_FG_DDS_Mode;
         -- v_SCU_Mode_s  <= v_SCU_Mode OR v_SCU_ACU_Mode; --check v_SCU_Mode immer 1 im SCU mode?
         -- HW_SEL_Mode   <= '0' & v_IFA_Mode_s  & v_SCU_Mode_s & v_MB_Mode;
