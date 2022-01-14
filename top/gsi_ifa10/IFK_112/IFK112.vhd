----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IFK112.vhd - Behavioral
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
----------------------------------------------------------------------------------
----ifk ident-code anpassen füe alle versionen

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;


ENTITY IFK112 IS

  PORT
  (
   sys_clk        : in std_logic;      -- System-Clock
   sys_reset      : in std_logic;      --
   Ena_Every_100ns: IN STD_LOGIC;
   Ena_Every_1us  : IN STD_LOGIC;

   Ifk_sel        : in std_logic;      -- Card selected
   FG112_Mode     : in std_logic;      -- card 112 selected

   FC_Str         : in std_logic;      -- Funktionscode-Strobe  ______+-+_______
   FC             : in STD_LOGIC_VECTOR(7 downto 0);           -- Funktionscode
   Data_in        : in std_logic_vector(15 downto 0);  -- latched data from MIL interface

   FG112_ADC      : in STD_LOGIC_VECTOR(7 DOWNTO 0);
   FG112_nLH      : in STD_LOGIC;  --16A
   FG112_nOBF     : in STD_LOGIC;
   FG112_LnH      : in STD_LOGIC;   --17B
   FG112_IBF      : in STD_LOGIC;
   FG112_nALARM   : in STD_LOGIC;
   A_ME_SD        : in STD_LOGIC;

   FG112_Funct    : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
   FG112_DAC      : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
   FG112_nACK     : OUT STD_LOGIC;
   FG112_nSTB     : OUT STD_LOGIC;
   FG112_nDRDY    : OUT STD_LOGIC;
   FG112_Res      : OUT STD_LOGIC;
   FG112_BLKErr   : OUT std_logic;     -- card 112 Block mode error

   FG112_send_en  : OUT STD_LOGIC;  --send enable für Daten lesen
   FG112_rd_str   : OUT STD_LOGIC;  --ende read-strobe

   FG112_RD_Aktiv : OUT STD_LOGIC;

   SendData       : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)  --data to send
);

END IFK112;


architecture Behavioral of IFK112 is

-----------------------
--Funktionsdecodierung

constant C_set_INR_M:  unsigned(7 downto 0)     := x"12";  -- SET_INR_MASKE --keine Auswertung hier erforderlich -> durch IFA-Register bereits abgedeckt
constant C_INT_STS:    unsigned(7 downto 0)     := x"C9";  -- INT. IFK-STATUS LESEN  --keine Auswertung hier erforderlich -> durch IFA-Register bereits abgedeckt

constant C_rst_Rst:    unsigned(7 downto 0)     := x"00";  -- Reset rücksetzen
constant C_set_Rst:    unsigned(7 downto 0)     := x"01";  -- Reset setzen

--112  only
constant C_set_DAC:   unsigned(7 downto 0)      := x"06";  -- Sollwert setzen
constant C_get_ADC:   unsigned(7 downto 0)      := x"81";  -- Nicht im Schaltplan vorhanden / liest einen einzelnen DAC-Wert

constant C_set_BLKmode: unsigned(7 downto 0)    := x"8F";  -- Blockmode aktivieren
constant C_set_BLKTestmode: unsigned(7 downto 0):= x"8E";  -- Blockmode aktivieren mit auto Loop nHBT

constant C_get_Status:  unsigned(7 downto 0)    := x"C0";  -- Status lesen, um nACK fürs auslesen zu aktivieren

-------------------------------------

--FC-Strobe detection
signal fc_edge1      : std_logic :='0' ;

-- Statemachine für Functionscode-abarbeitung
type   blk_type is (idle,get_122DB,FCready,set_112DAC_1,set_112DAC_2,wait_nOBH,send_BLK,wait_SD,wait_SD_1,force_nACK,wait_1us,wait_100ns,wait_gADC);

signal blk_sm        : blk_type := idle;
signal wait_next     : blk_type := idle;  --next state after nx100ns Pulse

constant dly1us       : integer := 9;     -- 10x 100ns warten
constant delay_cnt_max: integer := 15;    -- max. counter value
signal delay_cnt      : integer range 0 to delay_cnt_max := 0; --Delay-Zähler

constant timersec1_cnt_max: integer := 1000000;   -- max. counter value  für 1 sec in 1us  Schritten
signal timer_sec1_cnt   : integer range 0 to timersec1_cnt_max := 0;

constant timer100us_max : integer := 100;   -- max. counter value  für 100us in 1us  Schritten
signal timer100us_cnt   : integer range 0 to timer100us_max := 0;
signal timer100us_aktiv : std_logic :='0' ;

signal DAC112_out    : STD_LOGIC_VECTOR(15 DOWNTO 0);       -- speichert den DAC-Ausgabewert
signal ADC112_in     : STD_LOGIC_VECTOR(7 DOWNTO 0);        -- speichert den ADC-Eingangswert

signal FG112_nLH_1   : std_logic :='0' ;  -- für Flankendetektion
signal FG112_nOBF_1  : std_logic :='0' ;  -- für Flankendetektion

--signal BLKM_Trigger  : std_logic :='0' ;  -- Aktiviere Blockmode
signal BLKM_aktiv    : std_logic :='0' ;  -- Blockmode ist aktiv

signal A_ME_SD_1     : std_logic :='0' ;  -- für Flankendetektion von A_ME_SD


------------------
--test für Blockmode Autotrigger-Ausgabe

signal nACK_cnt      : STD_LOGIC_VECTOR(15 DOWNTO 0) := (others =>'0');  -- Zähler-> liefert Datenwort für Testblockmode
signal TestBlkOn     : std_logic :='0';   -- signalisiert aktiven Blockmode


begin

--Decode function strobe
SetFunct: process(sys_clk, sys_reset,FC_Str,FC,ena_every_100ns,FG112_Mode,Ena_Every_1us,A_ME_SD,A_ME_SD_1,fc_edge1)

begin

    if sys_reset = '1' or FG112_Mode='0' then
      FG112_Funct    <= (others =>'1');
      blk_sm         <= idle;
      wait_next      <= idle;

      delay_cnt      <= 0;
      timer_sec1_cnt <= 0;

      FG112_nSTB     <= '1';
      DAC112_out     <= (others =>'0');
      FG112_send_en  <= '0';
      FG112_rd_str   <= '0';
      FG112_Res      <= '0';
      FG112_nACK     <= '1';
      BLKM_aktiv     <= '0';
--      BLKM_Trigger   <= '0';
      A_ME_SD_1      <= '0';
      TestBlkOn      <= '0';
      fc_edge1       <= '0';
      FG112_nOBF_1   <= '0';
      SendData       <= x"0000";
      FG112_RD_Aktiv <= '0';

    elsif rising_edge(sys_clk) then

      --Flankendetektoren
      fc_edge1       <= FC_Str;     -- detect FC-Strobe, LH-transition
      A_ME_SD_1      <= A_ME_SD;    -- detect end of SD, HL-transition
      FG112_nOBF_1   <= FG112_nOBF; -- detect write HIGH byte, HL-transition

      --1sec timer
      --Auto-Reset Blockmode
      if Ena_Every_1us = '1' then
         if (timer_sec1_cnt > 0) then
            timer_sec1_cnt <= timer_sec1_cnt-1;
         else
--            BLKM_Trigger <= '0';    --Blockmode nach 1 sec zurücksetzen
            BLKM_aktiv   <= '0';
         end if;
      end if;-- Ena_Every_1us ='1' then

     --State-Machine für die Kommandos
      case blk_sm is
         when idle =>

                              delay_cnt      <= 0;       -- Delay-Zähler auf 0
                              wait_next      <= idle;    -- Nach 1us Delay wieder zurück
                              FG112_nSTB     <= '1';     -- DAC-Strobe für 112 default 'H'
                              FG112_send_en  <= '0';
                              FG112_rd_str   <= '0';
                              FG112_Res      <= '0';
                              FG112_RD_Aktiv <= '0';
                              FG112_nACK     <= '1';
--                              BLKM_Trigger   <= '0';

-------------
--112 set DAC, get ADC, BLK-Mode

      --timing 1sec und abschalten bis andere adresse / F6 1= gesetzt
        when wait_nOBH =>     FG112_nACK  <= '1';                 --default kein nACK setzen
                              if TestBlkOn = '0' then             -- auf Schreibpuls von HIGH-Byte warten /Testmodus ist aus(default)
                                 if FG112_nOBF_1 = '0' and FG112_nOBF='1' then-- HL-Flanke an FG112_nOBF
                                    FG112_RD_Aktiv <= '1';
                                    FG112_send_en  <= '1';
                                    SendData       <= FG112_ADC & ADC112_in(7 downto 0);-- FG122_Data_in(15 downto 0); --store
                                    FG112_rd_str   <= '1';        --Daten nun senden Strobe
                                    blk_sm         <= wait_SD;    --Daten auf MIL senden und danach mit nACK nach extern quittieren
                                 end if;
                              else --Testmodus eingeschaltet (FC=8E)(neu)
                                 FG112_RD_Aktiv <= '1';
                                 FG112_send_en  <= '1';
                                 nACK_cnt       <= nACK_cnt+1;
                                 SendData       <= nACK_cnt;      --FG122_Data_in(15 downto 0); --store
                                 FG112_rd_str   <= '1';           --Daten nun senden Strobe
                                 blk_sm         <= wait_SD;       --Daten auf MIL senden und danach mit nACK nach extern quittieren
                              end if;

         when  wait_SD  =>    --Aufs Ende von SendData (SD) des MIL-Encoders warten
                              FG112_rd_str      <= '0';
                              if BLKM_aktiv = '1' then            -- permanent senden oder nur 1x ist nicht spezifiziert)
                                 if A_ME_SD_1 = '1' and A_ME_SD = '0' then-- HL-Flanke an A_ME_SD -> Wort gesendet
                                    blk_sm      <= force_nACK;    -- 1us nACK erzeugen
                                    wait_next   <= send_BLK;      -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK
                                 end if;
                              else
                                 blk_sm   <= idle;                -- ohne Blockmode nur 1x lesen
                              end if;

         when  wait_gADC  =>
                              FG112_nACK  <= '1';
                              FG112_RD_Aktiv <= '1';
                              FG112_send_en  <= '1';
                              SendData       <= FG112_ADC & ADC112_in(7 downto 0); --store
                              FG112_rd_str   <= '1';           --Daten nun senden Strobe
                              blk_sm         <= wait_SD_1;     --Daten auf MIL senden und danach mit nACK nach extern quittieren

         when  wait_SD_1  =>     --Aufs Ende von SendData (SD) des MIL-Encoders warten
                              FG112_nACK  <= '1';
                              FG112_rd_str      <= '0';
                           -- if BLKM_aktiv = '1' then            -- permanent senden oder nur 1x ist nicht spezifiziert)
                              if A_ME_SD_1 = '1' and A_ME_SD = '0' then-- HL-Flanke an A_ME_SD -> Wort gesendet
                                 blk_sm      <= force_nACK;    -- 1us nACK erzeugen
                                 wait_next   <= idle;    -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK
                              end if;
                           -- else
                              -- blk_sm   <= idle;                -- ohne Blockmode nur 1x lesen
                           -- end if;

         when send_BLK  =>    FG112_rd_str      <= '0';
                              FG112_nACK        <= '1';
                              blk_sm            <= wait_nOBH;     -- wieder zurück

         when force_nACK =>   -- nur nACK-Puls
                              FG112_nACK        <= '0';           -- 1us nACK erzeugen
                              delay_cnt         <= 0;
                              blk_sm            <= wait_1us;
   ------------------------
         --Daten auf 112 ausgeben
         when set_112DAC_1 => FG112_nSTB        <= '0';           -- DAC-Strobe HL setzen
                              FG112_Funct(5)    <= '0';           -- FC F5 Anzeige auf VG-Funktionsausgabe
                              blk_sm            <= wait_1us;      -- 1 us warten
                              wait_next         <= set_112DAC_2;

         when set_112DAC_2 => FG112_nSTB        <= '1';           -- DAC-Strobe LH setzen
                              FG112_Funct(5)    <= '1';           -- FC F5 Anzeige auf VG-Funktionsausgabe
                              blk_sm            <= idle;          -- Das wars, den Rest macht das Zielsystem

   --------------------------------
        --Wartezyklus
         when wait_1us  =>    -- Wartezeit ca. dly1us (oder was als dly1us definiert wurde), danach geht's mit dem in "wait_next" weiter
                              if ena_every_100ns = '1' then --Jitter mit 100ns
                                 if delay_cnt >= dly1us then
                                    delay_cnt <= 0;
                                    blk_sm    <= wait_next; --hier dann weiter
                                 else
                                    delay_cnt <= delay_cnt + 1;
                                 end if;
                              end if;

         when wait_100ns  =>   -- Wartezeit ca. 100ns, danach geht's mit dem in wait_next weiter
                              if ena_every_100ns = '1' then
                                 if delay_cnt >= 2 then
                                    delay_cnt <= 0;
                                    blk_sm    <= wait_next; --hier dann weiter
                                 else
                                    delay_cnt <= delay_cnt + 1;
                                 end if;
                              end if;

         when others =>
                              blk_sm  <= idle;
      end case;   --case blk_sm is

      --Nun Befehlsdecodierung bei FC-Strobe
      if fc_edge1='0' and FC_Str='1' then    -- Detect FC-Strobe LH transition

         FG112_Funct <= (others =>'1');      -- Funktionsausgabe default auf H, wenn neues Kommando?

         if FG112_Mode='1' then

            timer_sec1_cnt <= 0;          -- Bei FC-Strobe 1sec-Timer auch nicht weiter laufen lassen
            case unsigned(FC) is
               when C_set_BLKmode => -- Blockmode aktivieren
                                 -- VG-FC F6 permanent so lange BLKMODE
                                    FG112_Funct(6) <= '0';        -- Anzeige in VG-FC
                                    BLKM_aktiv     <= '1';
--                                    BLKM_Trigger   <= '1';
                                    TestBlkOn      <= '0';
                                    timer_sec1_cnt <= timersec1_cnt_max;
                                    blk_sm         <= force_nACK; -- 1us nACK erzeugen
                                    wait_next      <= wait_nOBH;  -- nach 1 us weiter auf Datenwort neu anfordern von extern

               when C_set_BLKTestmode => --Testmodus / falls gesetzt, wird permanent alle 20us ein Datenwort über den MIL-BUS gesendet
                                 -- VG-FC F6 permanent so lange BLKMODE
                                 --if Ifk_sel = '1' then
                                    FG112_Funct(6) <= '0';        -- Anzeige in VG-FC
                                    BLKM_aktiv     <= '1';
--                                    BLKM_Trigger   <= '1';
                                    TestBlkOn      <= '1';        -- permanentes Triggern einschalten
                                    timer_sec1_cnt <= timersec1_cnt_max;
                                    blk_sm         <= force_nACK; -- 1us nACK erzeugen
                                    wait_next      <= send_BLK;   -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK
                                 --end if;

               when C_set_DAC =>    --DAC setzen
                                    -- VG-FC F5 1us PULS
                                    --/STR      1us PULS
                                    BLKM_aktiv     <= '0';
--                                    BLKM_Trigger   <= '0';
                                    DAC112_out     <= Data_in;       -- Zustand ausgeben
                                    blk_sm         <= set_112DAC_1;

               when C_get_ADC =>    --1x lesen
                                    -- erzeugt eine nACK nach SD und wartet dann auf nHBT Flanke von extern
                                    FG112_Funct(2) <= '0';        -- Anzeige in VG-FC
                                    SendData       <= FG112_ADC & ADC112_in(7 downto 0);--store
                                    blk_sm         <= wait_gADC; -- 1us nACK --Daten nach nHBT Flanke senden
                                 -- wait_next      <= wait_nOBH;  -- Daten auf MIL senden und danach, wenn Blockmode nACK nach extern quittieren

               when C_set_Rst =>
                                    BLKM_aktiv     <= '0';
--                                    BLKM_Trigger   <= '0';
                                    FG112_Funct(7) <= '0';        -- Bit7 in VG-FC für 1us an (externes Reset)
                                    blk_sm         <= wait_1us;
                                    wait_next      <= idle;

               when C_rst_Rst =>
                                    BLKM_aktiv     <= '0';
--                                    BLKM_Trigger   <= '0';
                                    blk_sm         <= idle;

               when C_get_Status =>
                                    BLKM_aktiv     <= '0';
--                                    BLKM_Trigger   <= '0';
                                    blk_sm         <= force_nACK;
                                    wait_next      <= idle;       -- nach 1us weiter auf idle

               when others =>
                                 -- z.B. DRQ -abfragen

            end case;   --unsigned(FC)
         else
            FG112_rd_str   <= '0';
            FG112_Res      <= '0';
            FG112_RD_Aktiv <= '0';
            SendData       <= x"0000";
         end if; --if FG112_Mode='1' then
      end if; --if fc_edge1='0' and FC_Str='1' then  --Functionstrobe
    end if; --rising_edge
end process;

FG112_BLKErr <= not BLKM_aktiv;

--100us Timer Retrigger
Timer100us:process(sys_clk,sys_reset,FG112_Mode,FG112_nOBF_1,FG112_nOBF,Ena_Every_1us,BLKM_aktiv,timer100us_cnt)
begin
    if sys_reset = '1' then
      timer100us_cnt    <= 0;
      timer100us_aktiv  <= '0';
      FG112_nDRDY       <= '1';
    elsif rising_edge(sys_clk) then
      if FG112_Mode = '1' then
         -- 100us timer start
         if FG112_nOBF_1 = '0' and FG112_nOBF = '1' then    -- LH an FG112_nOBF
            timer100us_cnt    <= timer100us_max;
            timer100us_aktiv <='1';
         else
           if Ena_Every_1us ='1' then
               if (timer100us_cnt > 0) then
                  timer100us_cnt    <= timer100us_cnt-1;
                  timer100us_aktiv  <= '1';
               else
                  timer100us_aktiv  <= '0';
               end if;
            end if;
         end if;
         if timer100us_aktiv='1' then -- and  (BLKM_Trigger='1'  or BLKM_aktiv='1') then
            FG112_nDRDY <= timer100us_aktiv;
         else
            FG112_nDRDY <= not FG112_nOBF;
         end if;
      else
         FG112_nDRDY <= '1';
      end if;  --if else FG112_Mode='1'
    end if;    --rising_edge
end process;

--select DAC output
--wird vom Zielsystem umgeschaltet FG112_LnH
SetDAC112C: process(sys_clk, sys_reset,FG112_LnH,DAC112_out,FG112_Mode)

begin

    if sys_reset = '1' then
      FG112_DAC <= (others =>'0');
    elsif rising_edge(sys_clk) then
      if FG112_Mode='1' then
         if FG112_LnH ='1' then
            FG112_DAC <= DAC112_out(7 downto 0);
         else
            FG112_DAC <= DAC112_out(15 downto 8);
         end if;  --FG112_LnH ='0'
      else
         FG112_DAC <= (others =>'0');
      end if; --ifelse FG112_Mode='1'
    end if;    --rising_edge
end process;


--Mux ADC input for 112
GetADC112C: process(sys_clk, sys_reset,FG112_ADC,FG112_nLH,FG112_nLH_1,FG112_Mode)

begin

    if sys_reset = '1' then
      ADC112_in   <= (others =>'0');
      FG112_nLH_1 <= '0';--16A
    elsif rising_edge(sys_clk) then
      FG112_nLH_1 <= FG112_nLH;     --Zustände für Flankendetektor speichern
   if FG112_Mode = '1' then
         if FG112_nLH_1 = '0' and FG112_nLH = '1' then   -- LH an LBT
            ADC112_in(7 downto 0) <= FG112_ADC;
         end if;
      else
         ADC112_in <= (others =>'0');
      end if; --ifelse FG112_Mode='1'
    end if;    --rising_edge
end process;


end Behavioral;

---------------------------------------------------------------------------
-- 112 PAL
--*IDENTIFICATION
--File-Name:      US10$ROOT:[HARTMANN.LOGIC.F380]112_FKT.DCB
--Entw./Vers.:    RHart/APL, V1.0 / 14-DEC-1990
--Kommentar:      fuer SD-yP Interface-Karte  (FG380.112)
--;
--;
--*X-NAMES
--FC[7..0], FC_OK, ADR_OK;
--
--*Y-NAMES
--IN_STS, INR_M, SOLL1, BLOCK, RESET;
--
--*PAL
--TYPE = PEEL173;
--
--*PINS
--FC[7..0]=[10..3], FC_OK=1, ADR_OK=13,
--IN_STS=14, INR_M=15, SOLL1=21, BLOCK=22, RESET=23;
--;
--;           Pin-Nr. am PAL  |  Pin-Nr. an der VG_Leiste
--;          -----------------+---------------------------
--;                  16       |         12a    (-)
--;                  17       |         12c    (-)
--;                  18       |         13a    (-)
--;                  19       |         13c    (-)
--;                  20       |         14a    (-)
--;                  21       |         14c    (Sollwert 1)
--;                  22       |         15a    (Block-Mode Ein)
--;                  23       |         15c    (Reset)
--*FUNCTION-TABLE
-- $ (FC[7..0]), FC_OK, ADR_OK :((IN_STS, INR_M, SOLL1, BLOCK, RESET));
--;
--;==============================================================================
--    01H    ,    0   ,    -   :     1      1      1      1       0 ; Reset
--    8FH    ,    -   ,    1   :     1      1      1      0       1 ; Block-Mode
--;=================== zur internen Kartenverwaltung ============================
--;
--    06H    ,    0   ,    -   :     1      1      0      1       1 ;Sollwert 1
--    12H    ,    0   ,    -   :     1      0      1      1       1 ;INR-Maske
--    C9H    ,    -   ,    -   :     0      1      1      1       1 ;Int_STS
--;
--;==============================================================================
--;
--         REST    :     1      1      1      1       1;
--;
--*RUN-CONTROL
-- PROG= JEDEC;
-- LIST= EQUATIONS, PINOUT, PLOT;
-- TEST-VECTORS=GENERATE;
--;OPTIMIZATION=P-TERMS;
--;SWITCH(1)=1;
--*END
