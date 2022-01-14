----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IF_Mode_a.vhd - Behavioral
-- Project Name:  6408_encode
-- Target Devices:
-- Tool versions:
-- Description:
-- Steuert den 6408 Encoder
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- Umwandlung des alten TDFs nach VHDL und
-- zusätzlich ist ESC nicht mehr als Taktquelle für die Register ---> keine Timing-Probleme mehr
-- Glitches in sendenable der Treiberausgangssteuerung beseitigt
-- BZO und BZI vom 6408 werden/müssen mit Zeitverzögerung durchgeschleift , um Sychron mit dem Treiber enable-Signal zu sein



LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;


entity A6408_encoder is

  Port
  (
  --system controls
    sys_clk     : in  STD_LOGIC;  -- systemclock at least 2x of bin Data clk -=> (approx. >2MHz)
    sys_reset   : in  STD_LOGIC;
    enable      : in  STD_LOGIC;  -- '1' - enable this modul
    ESC         : in  STD_LOGIC;  -- Encoder-Shift-Clock   |

  -- encode data controls
    WR_MIL      : in  STD_LOGIC;  -- start encoding with a LH change here -- Startet ein Mil-Send, muß mindestens 1 Takt (SYS_Clock) aktiv sein.                --
    Sts_Da      : in  std_logic_vector(15 downto 0); --data word to be transmitted
    SS_sel      : in  STD_LOGIC;  -- sync type select

    BZO         : in  STD_LOGIC;  -- differentielle Signale vom '6408'
    BOO         : in  STD_LOGIC;

    -- encoded signals
    SD          : in  STD_LOGIC;  -- force send data to 6408
    SS          : out STD_LOGIC;  -- sync select vorerst fixed 0 -> send Data Words
    SDI         : OUT STD_LOGIC;  -- Serielle Daten zum '6408' für zu sendende Daten)

    EE          : out STD_LOGIC;  -- Encoder Enable (Startet Sendesequenz beim '6408')
    Send_En     : out STD_LOGIC;  -- Enable Signal zur Sender-Freigabe
    BZO_out     : out STD_LOGIC;  -- Differentielle zeitverzögerte Signale zum Treiber
    BOO_out     : out STD_LOGIC;

    enc_busy    : out STD_LOGIC   -- ongoing encoding, active from WR_MIL signal strobe to Send_En

  );

end A6408_encoder;


architecture Behavioral of A6408_encoder is

--+----------------------------------------------------------------------------------------------------------------
--| Shift-Reg. für zu sendende Daten. Das Schieberegister wird so lange mit 'SD[15..0]' geladen SD nicht Aktiv ist.
--+----------------------------------------------------------------------------------------------------------------
--v_Data_out.  (data[], clock, load, sclr)   = (Sts_Da[], NOT ESC, NOT SD, SCLR);

signal ESC1       : STD_LOGIC := '0';  -- ESC clk Speicher für HL/LH detection
signal v_Data_out : std_logic_vector(15 downto 0) := ( others =>'0'); --Ausgangsschieberegister
signal v_EE       : STD_LOGIC := '0';  -- ENable ausgabe
signal start_sd   : STD_LOGIC := '0';
signal SD1        : STD_LOGIC := '0';  -- Send data Speicher für HL/LH detection

signal BO1        : STD_LOGIC := '0';  -- MIL-Bus Signalspeicher für Delay
signal BZ1        : STD_LOGIC := '0';  -- MIL-Bus Signalspeicher für Delay

Begin

SS <= SS_sel; -- vorerst Sync nur Datenwörter


--generiert ESC HL detection
ESCClk:process (sys_reset,sys_clk,ESC)
begin
   if (sys_reset = '1') then
      ESC1 <= '0';
   elsif rising_edge(sys_clk) then
      ESC1 <= ESC;
   end if;
end process;


-- Daten zum MIL-Device senden
SerinSd:process (sys_reset,sys_clk,ESC,ESC1,v_Data_out,SD,WR_MIL)

  begin

   if (sys_reset = '1') then
      v_Data_out <= (others =>'0');
   elsif rising_edge(sys_clk) then
      if SD = '0' then           --Shifter nur laden, wenn SD=0  vom 6408  und WR puls
        -- if WR_MIL = '1' then
            v_Data_out <= Sts_Da;
        -- end if;
      else
         if ESC = '0' and ESC1 ='1' then --ESC HL detected ?
            v_Data_out <= v_Data_out(14 downto 0) & '0';
         end if;
      end if;
    end if;
end process;

SDI <= v_Data_out(15);

--Encoder_Enable  Puls
--dtdf alt -> v_EE.(S, R, CLK, CLRN) = ( WR_MIL, SD, CLK, NOT SCLR);
RRgen:process (sys_reset,sys_clk,WR_MIL)

 begin

   if (sys_reset = '1') then
      v_EE <= '0';
   elsif rising_edge(sys_clk) then
      if WR_MIL = '1' then
         v_EE <= '1';
      elsif SD = '1'  then
         v_EE <= '0';
      end if;
   end if;
 end process;

EE <= v_EE;

--nSend_En  =  NOT (BZO != BOO); -- Ausgang für Sender_Enable = (BZO xor BOO)
--im tdf übel -> gibt Glitches auf Send_En
--ebenso (aus mil_serpar.vhdl)
--send_ena: process(esc,nman_send_en)
--  begin
--    if nman_send_en = '0' then
--      send_en <= '0';
--    elsif rising_edge(esc) then
--      send_en <= '1';
--    end if;

--  end process;
-- sehr laufzeitkritisch bei BOO BZO
-- Glitches werden dann immer noch bei ESC zur HL-Flanke generiert .im LogicAnalyser gesehen
--> daher besser die ESC zählen oder send enable anders/konstanter generieren
senenn:process (sys_reset,sys_clk,v_EE,ESC,ESC1,start_sd,SD,SD1)
 begin

    if (sys_reset = '1') then
        Send_en  <= '1';
        SD1 <= '0';
    elsif rising_edge(sys_clk) then
        if ESC = '1' and ESC1 ='0' then  --test immer bei bei LH  von ESC
            SD1 <= SD;  --delaykette
            Send_en <='0'; --Rücksetzen --default value
            if start_sd = '1' or SD='1' or SD1='1' then --Signale sind genau ab hier gespiegelt
               Send_en <= '1';
            end if;
        end if;
   end if; --if risig_edge

 end process;

--generate Startpuls
startp:process (sys_reset,sys_clk,v_EE,ESC,ESC1)
 begin

  if (sys_reset = '1') then
      start_sd <= '0';
  elsif rising_edge(sys_clk) then
      if v_EE='0' then
        start_sd <= '0';   --default value
      elsif v_EE = '1' and ESC = '0' and ESC1 ='1' then --ESC HL detected ?
        start_sd <= '1';
      end if;
  end if; --if risig_edge

 end process;

 --BUS-Verzögerung in Relation zur BOO/BZO-Ausgabe zu Send_en
-- das Treiber-An-Signal ist nun synchron zu der differentiellen Ausgangsleitungen
 BOOdel:process (sys_reset,sys_clk,BOO,BZO,BZ1,BO1)
 begin

   if (sys_reset = '1') then
      BZO_out  <= '0';
      BOO_out  <= '0';
      BO1      <= '0';
      BZ1      <= '0';
   elsif rising_edge(sys_clk) then
      BZ1      <= BZO;                       --2x syclk delay
      BZO_out  <= BZ1;
      BO1      <= BOO;
      BOO_out  <= BO1;
   end if; --if risig_edge

 end process;

 enc_busy <= start_sd or SD or SD1;






end Behavioral;  --MIL Device encoder

