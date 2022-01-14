----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IFK122.vhd - Behavioral
-- Project Name:  IFA10
-- Target Devices:
-- Tool versions:
-- Description:
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


ENTITY IFK122 IS

  PORT
  (
   sys_clk        : in std_logic;      -- System-Clock
   sys_reset      : in std_logic;
   Ena_Every_100ns: IN STD_LOGIC;

   FG122_Mode     : in std_logic;      -- card 122 selected

   FC_Str         : in std_logic;      -- Funktionscode-Strobe  ______+-+_______
   FC             : in STD_LOGIC_VECTOR(7 downto 0);           -- Funktionscode
   Data_in        : in std_logic_vector(15 downto 0);  -- latched data from MIL interface

   FG122_Data_in  : in STD_LOGIC_VECTOR(15 DOWNTO 0);  -- DB-Data from VG port
   A_ME_SD        : in STD_LOGIC;

   FG122_Res      : OUT STD_LOGIC;
   FG122_Funct    : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
   FG122_addr     : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);  -- adr-Data from VG port
   FG122_Data_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
   FG122_DBRD     : OUT STD_LOGIC;
   FG122_DBWR     : OUT STD_LOGIC;           -- muss Pos-Puls bleiben, weil damit auch die DIRS der Ios gesteuert werden
                                             -- Wird nur bei B15 invertiert, um neg nWR Puls zu erzeugen
   FG122_send_en  : OUT STD_LOGIC;           -- send enable für Daten lesen
   FG122_rd_str   : OUT STD_LOGIC;           -- ende read-strobe
   FG122_RD_Aktiv : OUT STD_LOGIC;

   SendData       : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)  --data to send

);

END IFK122;


architecture Behavioral of IFK122 is

-----------------------
--Funktionsdecodierung

constant C_set_INR_M:  unsigned(7 downto 0)     := x"12";  --SET_INR_MASKE --keine Auswertung hier erforderlich -> durch IFA-Register bereits abgedeckt
constant C_INT_STS:    unsigned(7 downto 0)     := x"C9";  --INT. IFK-STATUS LESEN  --keine Auswertung hier erforderlich -> durch IFA-Register bereits abgedeckt
constant C_rst_Rst:    unsigned(7 downto 0)     := x"00"; -- Reset rücksetzen

--122  only

constant C_set_Rst:    unsigned(7 downto 0)     := x"01"; -- Reset setzen
constant C_set_data:   unsigned(7 downto 0)     := x"10"; -- Datenbus setzen
constant C_set_addr:   unsigned(7 downto 0)     := x"11"; -- Adresse setzen
constant C_get_data:   unsigned(7 downto 0)     := x"90"; -- Datenbus lesen


-------------------------------------

--FC-Strobe detection
signal   fc_edge1       : std_logic :='0' ;

-- Statemachine für Functionscode-Abarbeitung
type     blk_type is (idle,get_122DB,FCready,wait_1us,wait_SD_1,get_122DB_1us,set_122DB_1us);

signal   blk_sm         : blk_type := idle;
signal   wait_next      : blk_type := idle;  --next state after nx100ns Pulse

constant dly1us         : integer := 9;      --9x 100ns warten
constant delay_cnt_max  : integer := 10;    -- max. counter value
signal   delay_cnt      : integer range 0 to delay_cnt_max := 0; --Delay-Zähler
signal   A_ME_SD_1      : std_logic :='0' ;  -- für Flankendetektion von A_ME_SD


-------------------------------------------------------------------
begin

--Decode function strobe
SetFunct: process(sys_clk, sys_reset,FC_Str,FC,ena_every_100ns,FG122_Mode,fc_edge1,A_ME_SD,FG122_Data_in)

begin

    if sys_reset = '1' or FG122_Mode= '0' then
      FG122_Funct    <= (others =>'1');
      FG122_DBRD     <='0';
      FG122_DBWR     <='0';
      blk_sm         <= idle;
      wait_next      <= idle;
      delay_cnt      <= 0;
      FG122_send_en  <= '0';
      FG122_rd_str   <= '0';
      FG122_Res      <= '0';
      fc_edge1       <= '0';
      SendData       <= x"7788";-- <= (others =>'0');      -- keine Ausgabe
      FG122_RD_Aktiv <= '0';


    elsif rising_edge(sys_clk) then

      --Flankendetektoren
      fc_edge1       <= FC_Str;     -- detect FC-Strobe, LH-transition
      A_ME_SD_1      <= A_ME_SD;    -- detect end of SD, HL-transition
      FG122_rd_str   <= '0';        -- Strobe für Daten nun MIL-Tx


     --State-Machine für die Kommandos
      case blk_sm is
         when idle =>
                              delay_cnt      <= 0;       -- Delay-Zähler auf 0
                              wait_next      <= idle;    -- Nach 1us Delay wieder zurück
                              FG122_DBRD     <= '0';
                              FG122_DBWR     <= '0';
                              FG122_send_en  <= '0';
                              FG122_RD_Aktiv <= '0';
                              FG122_rd_str   <= '0';
                              FG122_Res      <= '0';

         when  wait_SD_1  =>     --Aufs Ende von SendData (SD) des MIL-Encoders warten
                              if A_ME_SD_1 = '1' and A_ME_SD = '0' then-- HL-Flanke an A_ME_SD -> Wort gesendet
                                 blk_sm   <= idle;    -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK
                              end if;

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


        when get_122DB_1us => FG122_DBRD     <= '1';           --Lesepuls nach extern für 1 us
                              blk_sm         <= wait_1us;
                              delay_cnt      <= 0;
                              wait_next      <= get_122DB;     --Daten nach 1us abholen

        when set_122DB_1us => FG122_DBWR     <= '1';           --Schreibpuls nach extern für 1 us
                              blk_sm         <= wait_1us;
                              delay_cnt      <= 0;
                              wait_next      <= idle;

        when get_122DB =>
                              --FG122_DBRD     <= '0';            --Lesepuls nach extern abschalten
                              SendData       <= FG122_Data_in;    --Daten immer einlesen -> gültig erst bei FG122_rd_str = H
                              FG122_rd_str   <= '1';              --Daten nun MIL senden Strobe
                              blk_sm         <= FCready;

         when FCready => -- auf Ende von FC-Strobe warten
                              FG122_DBRD     <= '0';     --Lesepuls nach extern wieder zu 0
                              blk_sm         <= wait_SD_1;

         when others =>
                              blk_sm  <= idle;
      end case;   --case blk_sm is

      --Nur Befehlsdecodierung bei FC-Strobe LH
      if FG122_Mode = '1' then
         if fc_edge1 = '0' and FC_Str ='1' then    -- Detect FC-Strobe LH transition

            FG122_Funct <= (others =>'1');      -- Funktionsausgabe default auf L, wenn neues Kommando?
            case unsigned(FC) is
               when C_set_addr => --Adressbus setzen
                                 FG122_Funct(0) <= '0';
                                 FG122_addr     <= Data_in(7 downto 0);
                                 blk_sm         <= wait_1us;      -- Bit0 in FC für 1us an
                                 wait_next      <= idle;          -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK

               when C_set_data =>
                                 FG122_Funct(1) <= '0';           -- FC anzeigen
                                 --FG122_DBWR     <= '1';         -- seFG122_rd_str <= '0';   setzen und Ausgabe auf Databus
                                 FG122_Data_out <= Data_in;
                                 blk_sm         <= set_122DB_1us; -- Bit1 in Funktion für 1us an, dito FG122_DBWR
                                 wait_next      <= idle;          -- nach 1 us weiter auf Datenwort neu anfordern mit 1us nACK

               when C_get_data =>
                                 FG122_Funct(2) <= '0';           --Lesepuls auf Funktionsausgabe
                                 FG122_DBRD     <= '0';           --Lesepuls nach extern
                                 FG122_RD_Aktiv <= '1';           --MIL-Auswahl TX select
                                 FG122_send_en  <= '1';
                                 blk_sm         <= wait_1us;
                                 wait_next      <= get_122DB_1us;     --Daten nach 1us abholen

               when C_set_Rst =>
                                 FG122_Funct(7) <= '0';
                                 FG122_Res      <= '1';
                                 blk_sm         <= idle;          --Bit7 in Function für 1us an (externes reset)

               when C_rst_Rst =>
                                 -- in idle bleiben
               when others =>
            end case;   -- case unsigned(FC)
         end if; --if fc_edge1='0' and FC_Str='1' then  --Functionstrobe
      else
         FG122_Funct    <= (others =>'1');      -- Funktionsausgabe default auf L, wenn neues Kommando?
         SendData       <= x"7788";-- <= (others =>'0');      -- keine Ausgabe
         FG122_send_en  <= '0';
         FG122_RD_Aktiv <= '0';
         FG122_rd_str   <= '0';
      end if; --if FG122_Mode = '1' then

    end if; --rising_edge
end process;


end Behavioral;

--        /RD_DB    =90H ==> DATENBUS LESEN
--------------------------------------------------------------------
-- 122 PAL
--*IDENTIFICATION
--FILE_NAME:   US10$ROOT:[DBAPL.FG380.LOGIC]120_FKT.DCB
--ENTW./VERS.: H.GROSS/APL 18.07.90
--KOMMENTAR: GAL FUER SE-INTERFACEKARTE I/O BUS-VERSION  FG 380.120
--
-->CODE NR.
--
--*X-NAMES
--FC[7..0],FCVALID ; 9 Eingaenge
--
--*Y-NAMES
--INT_STS,INR_MASKE,SET_AB,SET_DB,RD_DB, RES_DB   ; 6 AUSGAENGE
--
--*PAL
--TYPE=PEEL173   ;(ERSETZT ALTEN TYP PAL 12L10  Platz:IC21)
--
--*PINS
--FC[7..0]=[10..3],FCVALID=11,
--INT_STS=14,INR_MASKE=15,SET_AB=16,SET_DB=17,RD_DB=18,RES_DB=23 ;
--; PIN>>>1,2,13,19,20,21,22=NC,12=GND,24=VCC
  --end Behavioral;
--INT_STS=14,INR_MASKE=15,
--SET_AB=F0,
--SET_DB=F1
--RD_DB =F2,
--RES_DB=F7
--; PIN>>>1,2,13,19,20,21,22=NC,12=GND,24=VCC
--
--*BOOLEAN_EQUATION
--;================= zur internen Kartenverwaltung ===========================
--;
--;
--;  -C9-      |  1    1    0    0  |   1    0    0    1  |
--/INT_STS     = FC7& FC6&/FC5&/FC4 &  FC3&/FC2&/FC1& FC0 ;
--;
--;  -12-      |  0    0    0    1  |   0    0    1    0  |
--/INR_MASKE   =/FC7&/FC6&/FC5& FC4 & /FC3&/FC2& FC1&/FC0 & /FCVALID ;
--;===========================================================================
--;
--;################ ZUR I/O BUS FG_385 STEUERUNG #############################
--;
--;  -01-      |  0    0    0    0  |   0    0    0    1  |
--/RES_DB      =/FC7&/FC6&/FC5&/FC4 & /FC3&/FC2&/FC1& FC0 & /FCVALID ;
--;
--;  -11-      |  0    0    0    1  |   0    0    0    1  |
--/SET_AB      =/FC7&/FC6&/FC5& FC4 & /FC3&/FC2&/FC1& FC0 & /FCVALID ;
--;
--;  -10-      |  0    0    0    1  |   0    0    0    0  |
--/SET_DB      =/FC7&/FC6&/FC5& FC4 & /FC3&/FC2&/FC1&/FC0 & /FCVALID ;
--;
--;  -90-      |  1    0    0    1  |   0    0    0    0  |
--/RD_DB       = FC7&/FC6&/FC5& FC4 & /FC3&/FC2&/FC1&/FC0 & /FCVALID ;
--;
--;#########################################################################
--
--*FUNKTION-TABLE
--
--
--*RUN-CONTROLL
--LISTING = PLOT,FUSE-PLOT,PINOUT,EQUATINS;
--PROGFORMAT =JEDEC;
--;SWITCH(1)=1;
--*END
--
--
--*COMMENT
--DAS PEEL173 (LOESCHBAR)  ERSETZT DEN ALTEN PAL12L10
--PAL STECKPLATZ: IC21
--PLATZ:
--ANWENDER: EICKHOFF
--;
--DESCRIPTION
--ES GILT:
--        /RES_DB   =01H ==> RESET DATEN-BUS
--                  =00H ==> RESET FREIGABE
--        /INT_STS  =C9H ==>INT. IFK-STATUS LESEN
--        /INR_MASKE=12H ==> SET_INR_MASKE
--        /SET_AB   =11H ==> ADRESSBUS SETZEN
--        /SET_DB   =10H ==> DATENBUS SETZEN
--        /RD_DB    =90H ==> DATENBUS LESEN

