----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    LED_out.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices: MAX10
-- Tool versions:
-- Description:
-- Erzeugt die LED Ausgabe
-- Darstellung der Versionsnummern
-- Blinken der Leds
-- Puffernd der LEDs

--
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
----------------------------------------------------------------------------------


LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

library work;


ENTITY LED_out is

  Port (
   sys_clk     : in  std_logic;     -- System-Clock
   sys_reset   : in std_logic;

   ENA_Every_20ms: IN STD_LOGIC;
   ENA_Every_1sec: IN STD_LOGIC;
   ENA_Every_2_5s: IN STD_LOGIC;

   MB_virt_RW  :   IN STD_LOGIC;
   IFA_Mode    :   IN STD_LOGIC;
   FG_Mode     :   IN STD_LOGIC;
   FG_DDS_Mode :   IN STD_LOGIC;

   MB_Mode     :   IN STD_LOGIC;
   Sweep_Mode  :   IN STD_LOGIC;

   IFA_VERS    :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);
   Sweep_VERS  :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);
   MB_VERS     :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);
   FG_VERS     :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);
   FB_VERS     :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);

   IFA_ADR     :   IN STD_LOGIC_VECTOR(7 DOWNTO 0);

   LED_SEL_IN  :   IN STD_LOGIC;
   nLED_TRM_IN :   IN STD_LOGIC;
   LED_RCV_IN  :   IN STD_LOGIC;
   nLED_DRQ_IN :   IN STD_LOGIC;
   nLED_DRDY_IN:   IN STD_LOGIC;
   nLED_INL_IN :   IN STD_LOGIC;
   UMIL15V     :   IN STD_LOGIC;
   UMIL5V      :   IN STD_LOGIC;

   FB_Mode     :   IN STD_LOGIC;

   PowerUP_Flag:   IN STD_LOGIC;
   ME_Err_Limit:   IN STD_LOGIC;
   JP_SEL_6408 :   IN STD_LOGIC;

   Sync_Event_Ser_Str: IN STD_LOGIC;
   A_EXT_CLK   :   IN STD_LOGIC;

   USER_LED_in :   IN STD_LOGIC;
   FAILSAVE_LED_in: IN STD_LOGIC;
   SHOW_CONFIG_in:  IN STD_LOGIC;

   nLED_out    :   OUT STD_LOGIC_VECTOR(27 DOWNTO 0)

);
end entity LED_out;



architecture Behavioral of LED_out is

--Der Strecher verlängert invertiert die LED-Signale
COMPONENT led_stretch
   PORT(ENA    : IN STD_LOGIC;
       CLK     : IN STD_LOGIC;
       Sig_In  : IN STD_LOGIC;
       nLED    : OUT STD_LOGIC
   );
END COMPONENT;


signal LED           : STD_LOGIC_VECTOR(27 downto 0)  := ( others =>'0');
signal nLED          : STD_LOGIC_VECTOR(27 downto 0)  := ( others =>'0');

signal blink         : std_logic:= '0'; -- steuert Blinklicht im sekundentakt

--Auswahlzähler
signal sel           : std_logic:= '0';
signal nextsel       : std_logic:= '0'; --puls für weiterzählen der modi select ausgabe
signal selcnt        : std_logic_vector(3 downto 0) := ( others =>'0');

signal Speicher1     : std_logic := '0';
signal Speicher2     : std_logic := '0';

--signal LEDdimmer     : std_logic := '0';
--signal nLED_swap           : STD_LOGIC_VECTOR(27 downto 0);

-----------------------------------------------------------
begin


--LEDs(27 ..25) ohne Stretcher
nLED(27)    <= SHOW_CONFIG_in;
nLED(26)    <= not FAILSAVE_LED_in;
nLED(25)    <= not USER_LED_in;

------------
--Stretcher
LED(24)     <= Sync_Event_Ser_Str;
LED(23)     <= A_EXT_CLK;
LED(22)     <= LED_SEL_IN;
LED(21)     <= LED_RCV_IN;
LED(20)     <= nLED_TRM_IN;

--LED(19 downto 12) s.u. sind sind  Ausgaben für Versionsausgaben oder IFK-Adresse

LED(11)     <= not nLED_INL_IN;
LED(10)     <= not nLED_DRQ_IN;
LED(9)      <= not nLED_DRDY_IN;
LED(8)      <= blink and PowerUP_Flag;
--/Stretcher
----------------
--LEDs für UMIL, ohne Stretcher
Speicher1 <= not (JP_SEL_6408 and ME_Err_Limit and blink);
Speicher2 <= (Speicher1 and UMIL15V);

nLED(7)     <= not ( (not Speicher2) and (Speicher1 and UMIL5V)) ;
nLED(6)     <= not Speicher2;


--Stretcher
--LED(5 downto 0) s.u. sind  Ausgaben für Versionsflag oder Betriebsmodus
--/Stretcher

---------------------------------------------------------------------------------------
----Blinker
--später zu Timer/Clock
Blinksig: process (sys_clk,sys_reset,blink,ENA_Every_1sec) -- shiftet
  begin

   if sys_reset='1' then
      blink <= '0';
   elsif rising_edge(sys_clk) then
     if ENA_Every_1sec = '1' then
         blink <= not blink;
     end if; -- 1sec puls
    end if; --elsif rising_edge(sys_clk)
end process;


--Später zu Timer/Clock
SelShift: process (sys_clk,sys_reset,ENA_Every_2_5s) -- shiftet
  begin

   if sys_reset='1' then
      nextsel  <= '0';
   elsif rising_edge(sys_clk) then
      nextsel <= '0';
      if ENA_Every_2_5s = '1' then
         nextsel <= '1'; -- kurzer enable Puls
      end if;  --counter ready?
    end if; --elsif rising_edge(sys_clk)
end process;


selMode: process (sys_clk,sys_reset,nextsel,selcnt) -- shiftet
  begin

   if sys_reset = '1' then
      selcnt   <= ( others =>'0');
      LED(5 downto 0)   <= (others =>'0');
      LED(19 downto 12) <= (others =>'0');

   elsif rising_edge(sys_clk) then
      if nextsel = '1' then
         if selcnt < x"7" then
            selcnt <= selcnt+1;
         end if;
      end if;

      LED(5 downto 0)  <= (others =>'0');
      LED(19 downto 12)<= (others =>'0');

      case selcnt  is

         when x"0" =>-- sel_VERS_IFA <='1';
               LED(19 downto 12) <= IFA_VERS;
               LED(5) <= '1';

         when x"1" =>--sel_VERS_FG  <='1';
               LED(19 downto 12) <= FG_VERS;
               LED(4) <= '1';

         when x"2" =>-- sel_VERS_MB  <='1';
               LED(19 downto 12) <= MB_VERS;
               LED(2) <= '1';

         when x"3" =>--sel_VERS_FB  <='1';
               LED(19 downto 12) <= FB_VERS;
               LED(1) <= '1';

         when x"4" =>-- sel_VERS_SWEEP <='1';
               LED(19 downto 12) <= Sweep_VERS;
               LED(0) <= '1';

         when others =>--  sel_VERS_ENABLE<='1';
               LED(5 downto 0)   <= IFA_Mode & FG_Mode & FG_DDS_Mode & MB_Mode & FB_Mode & (Sweep_MODE or MB_virt_RW);
               LED(19 downto 12) <= IFA_ADR;
      end case;

    end if; --elsif rising_edge(sys_clk)
end process;


--Strech LED outputs
S_1 : for I in 8 to 24 generate
         LED_Buff :
         led_stretch port map
              (ENA_Every_20ms, sys_clk, LED(I), nLED(I));
         end generate;

S_2 : for I in 0 to 5 generate
         LED_Buff :
         led_stretch port map
              (ENA_Every_20ms, sys_clk, LED(I), nLED(I));
         end generate;

--nLED_swap(27 downto 0) <=nLED(27 downto 0);

-----------------------------------------

nLED_out <= nLED; -- ev. when LEDdimmer='1' else (Others =>'1');

-----------------------------------------
--für später noch die Effekte
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
--end process

end Behavioral;



--tatsächliche Zuordnung zum Schaltplan und zur Layoutbezeichnung
--irgendwo auf PCB
--nLED_swap(27) <= nLED(27);   -- SHOW_CONFIG_in

--D84 PowerLed -- nicht dimmbar -> FET control?
--D82
--nLED_swap(24) <= nLED(24);--A_NLED_EVT_INRn
--D83
--nLED_swap(23) <= nLED(23);--A_nLED_EXTCLKn
--D85
--nLED_swap(25) <= nLED(25);   -- nUSER_LED_in
--D86
--nLED_swap(26) <= nLED(26);   -- nFAILSAVE_LD_LED

--D58
--nLED_swap(22) <=nLED(22); --SEL
----D59
--nLED_swap(20) <=nLED(21); --RCV
----D61
--nLED_swap(18) <=nLED(20); --TRM
----D62
--nLED_swap(17) <=nLED(19); --A7
----D63
--nLED_swap(16) <=nLED(18); --A6
----D64
--nLED_swap(15) <=nLED(17);   --A5
----D65
--nLED_swap(14) <=nLED(16);   --A4
----D67
--nLED_swap(12) <=nLED(15);   --A3
--
----D68
--nLED_swap(11) <=nLED(14);   --A2
----D66
--nLED_swap(13) <=nLED(13);   --A1
----D69
--nLED_swap(10) <=nLED(12);   --A0
--
----D70
--nLED_swap(9)  <=nLED(11);   --INL
----D71
--nLED_swap(8)  <=nLED(10);   --DRQ
--
----D72
--nLED_swap(7)  <=nLED(9);   -- DRY
----D73
--nLED_swap(6)  <=nLED(8);    --PUR
--
----D74
--nLED_swap(5)  <= nLED(7);    --M5
----D75
--nLED_swap(4)  <= nLED(6);    --M15
--
----D76
--nLED_swap(3)  <=nLED(5);    --IFA
----D77
--nLED_swap(2)  <=nLED(4);    -- FG
----D78
--nLED_swap(1)  <=nLED(3);    -- FGD
----D79
--nLED_swap(0)  <=nLED(2);     --MB
----D80
--nLED_swap(21) <=nLED(1);     --FB
----D81
--nLED_swap(19) <=nLED(0);     --RES
