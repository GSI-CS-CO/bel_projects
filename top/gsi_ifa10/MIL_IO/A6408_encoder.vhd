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
-- vk 13.092021 -- encodinn zu State machine geändert - fehler bei ausgabe beseitigt


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
    ESC         : in  STD_LOGIC;  -- Encoder-Shift-Clock

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


signal ESC1       : STD_LOGIC := '0';  -- ESC clk Speicher für HL/LH detection
signal v_Data_out : std_logic_vector(15 downto 0) := ( others =>'0'); --Ausgangsschieberegister
signal v_EE       : STD_LOGIC := '0';  -- 6408 für Encoding antriggern

signal BO1        : STD_LOGIC := '0';  -- MIL-Bus Signalspeicher für Delay
signal BZ1        : STD_LOGIC := '0';  -- MIL-Bus Signalspeicher für Delay


type enc_states is (
   ENC_0,
   ENC_1,
   ENC_2,
   ENC_3,
   ENC_4
);

signal enc_state : enc_states :=  ENC_0;


Begin

encode:Process(sys_clk,sys_reset,enc_state,ESC,ESC1,Sts_Da,SD,WR_MIL,v_Data_out) --set und reset der valid Anzeige bits für empfangene Daten

  begin
    if (sys_reset = '1') then

      enc_state   <= ENC_0; --Warten auf WR_MIL
      ESC1        <= '0';
      Send_en     <= '0';
      enc_busy    <= '0';
      v_EE        <= '0';

    elsif rising_edge(sys_clk) then

         ESC1 <= ESC;         --Zustand von Flankendetektor MIL-CLK aktualisieren

         case enc_state is

            when    ENC_0 =>
                                 Send_en     <= '0';  -- Treiber sperren
                                 enc_busy    <= '0';  -- Anzeige, dass Encoding nicjt aktiv
                                 v_EE        <= '0';  -- keine Anforderung an 6408
                                 if WR_MIL = '1' then -- Schreibpuls?
                                    enc_state  <= ENC_1;
                                    v_Data_out <= Sts_Da; --Daten übernehmen
                                    enc_busy    <= '1';
                                    v_EE <= '1';
                                 end if;

            when    ENC_1 =>     if ESC = '0' and ESC1 ='1' then --erstes ESC HL detected
                                    enc_state <= ENC_2;  -- 6408 startet immer mit nächsten LH den SYNC
                                 end if;

            when    ENC_2 =>     -- Warten auf SD='1';
                                 if ESC = '1' and ESC1 ='0' then --erstes ESC LH detected ?
                                    Send_en <= '1';      -- Treiber freigeben, SYNC beginnt
                                 end if;
                                 if SD = '1' then
                                    v_EE <= '0';         -- Anforderung zurücksetzen
                                    enc_state <= ENC_3;
                                 end if;

             when    ENC_3 =>    --Solange SD='1' ist, Databits raustakten
                                 if ESC = '0' and ESC1 ='1' then --ESC HL detected -> neues Bit
                                    v_Data_out <= v_Data_out(14 downto 0) & '0';
                                 end if;

                                 --Ende der Sequenz
                                 --SD nur Testen bei LH von ESC -> Störunempfindlicher
                                 if ESC = '1' and ESC1 ='0' then
                                    if SD = '0' then
                                       enc_state <= ENC_4;
                                    end if;
                                 end if;

             when    ENC_4 =>    if ESC = '1' and ESC1 ='0' then --ESC HL detected ?
                                    -- Parity ging auch raus -> alles beendet
                                    Send_en     <= '0';  --Treiber sperren
                                    enc_busy    <= '0';  --Anzeige, dass Encoding beendet ist
                                    enc_state <= ENC_0;
                                 end if;

            when others =>
                                 enc_state <= ENC_0;

          end case;     -- case enc_state

      end if;           -- rising_edge(sys_clk)
end process;


SS <= SS_sel; -- vorerst Sync nur Datenwörter

-- Daten zum MIL-Device senden
SDI <= v_Data_out(15);

EE <= v_EE;


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


end Behavioral;  --MIL Device encoder

