library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;


entity Mil_dec_edge_timed is
--+-------------------------------------------------------------------------------------------------------------------------------------+
--| "Mil_dec_edge_timed" empfaengt einen manchester kodierten Datenstrom nach MIL-STD-1553B Protokoll.                                  |
--|                                                                                                                                     |
--| Version 1;  Autor: W.Panschow; Datum: 16.02.2010                                                                                    |
--|                                                                                                                                     |
--| Version 2;  Autor: W.Panschow; Datum: 22.11.2012                                                                                    |
--| Änderungen:                                                                                                                         |
--|   1)  Das Enable das alle 500ns fuer einen Takt aktiv sein soll, wird jetzt aus der Mil_Clk erzeugt. Damit wird verhindert, dass    |
--|       das Enable welches vormals außerhalb dieses Makros erzeugt wiurde eventuell von einer anderen Clock-Domaene abgeleitet wurde. |
--|   2)  Die Umschaltung zwischen High- und Standard-Speed wurde entfernt. Es gibt nur noch Standard-Speed (Bitrate 1 us)              |
--|                                                                                                                                     |
--| Version 3;  Autor: W.Panschow; Datum: 13.09.2013                                                                                    |
--| Änderungen:                                                                                                                         |
--|   1)  Mit den Umzug des Makros ins GIT-Repository ist die Versionskennung aus dem Makro-Namen entfernt worden.                      |
--|   2)  Generic "Mil_CLK_in_Hz" in "CLK_in_Hz" umbenannt. Signal "Mil_Clk" in "clk" umbenannt.                                        |
--|                                                                                                                                     |
--| Version 4;  Autor: W.Panschow; Datum: 23.09.2013                                                                                    |
--|   1)  Die Breite von "S_Time_between_2_Edges_cnt" wird jetzt aus der "Timeout-Zeit" berechnet.                                      |
--|   2)  CLK_in_ps wird jetzt richtig berechnet.                                                                                       |
--|   3)  Ein paar Umlaute entfernt.                                                                                                    |
--+-------------------------------------------------------------------------------------------------------------------------------------+
  generic(
    CLK_in_Hz:        integer := 24_000_000;    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                                -- in Hertz beschrieben werden.
    Receive_pos_lane: integer range 0 TO 1 := 0 -- '0' => der Manchester-Datenstrom wird bipolar ueber Übertrager empfangen.
                                                -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                                -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    );
  port(
    Manchester_In:      in  std_logic;    -- Eingangsdatenstrom MIL-1553B
    RD_MIL:             in  std_logic;    -- setzt Rvc_Cmd, Rcv_Rdy und Rcv_Error zurueck. Muss synchron zur Clock 'clk' und 
                                          -- mindesten eine Periode lang aktiv sein!
    Res:                in  std_logic;    -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    clk:                in  std_logic;
    Rcv_Cmd:            out std_logic;    -- '1' es wurde ein Kommando empfangen.
    Rcv_Error:          out std_logic;    -- ist bei einem Fehler fuer einen Takt aktiv '1'.
    Rcv_Rdy:            out std_logic;    -- '1' es wurde ein Kommand oder Datum empfangen.
                                          -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data:       out std_logic_vector(15 downto 0);  -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag:   out std_logic_vector(15 downto 0)   -- Diagnoseausgaenge fuer Logikanalysator
    );

end Mil_dec_edge_timed;

architecture Arch_Mil_dec_edge_timed of Mil_dec_edge_timed is

  type  T_RCV_SM  is
                (
                RCV_Idle,
                Sync1or2,
                Sync2orData,    
                Data,
                Parity,
                Err
                );

  signal  RCV_SM :  T_RCV_SM;

  function  How_many_Bits  (int: integer) return integer is

    variable i, tmp : integer;
  
    begin
      tmp   := int;
      i   := 0;
      WHILE tmp > 0 LOOP
        tmp := tmp / 2;
        i := i + 1;
      end LOOP;
      return i;
    end How_many_bits;

  constant  Data_Size:              integer := 16;
        
  constant  Delta_in_ns:            integer := 200;
                                    
  constant  CLK_in_ps:              integer := (1000000000 / (CLK_in_Hz / 1000));
          
  constant  Sync_max_ns:            integer := 1500 + Delta_in_ns;
  constant  Sync_max_cnt:           integer := Sync_max_ns * 1000 / CLK_in_ps;
  constant  Sync_min_ns:            integer := 1500 - Delta_in_ns;
  constant  Sync_min_cnt:           integer := Sync_min_ns * 1000 / CLK_in_ps;
  
  constant  Sync_with_bit_max_ns:   integer := 2000 + Delta_in_ns;
  constant  Sync_with_bit_max_cnt:  integer := Sync_with_bit_max_ns * 1000 / CLK_in_ps;
  constant  Sync_with_bit_min_ns:   integer := 2000 - Delta_in_ns;
  constant  Sync_with_bit_min_cnt:  integer := Sync_with_bit_min_ns * 1000 / CLK_in_ps;
  
  constant  Bit_short_time_max_ns:  integer := 500 + Delta_in_ns;
  constant  Bit_short_time_max_cnt: integer := Bit_short_time_max_ns * 1000 / CLK_in_ps;
  constant  Bit_short_time_min_ns:  integer := 500 - Delta_in_ns;
  constant  Bit_short_time_min_cnt: integer := Bit_short_time_min_ns * 1000 / CLK_in_ps;
  
  constant  Bit_short_standard_ns:  integer := 500;
  constant  Bit_short_standard_cnt: integer := Bit_short_standard_ns * 1000 / CLK_in_ps;
  
  constant  Bit_long_time_max_ns:   integer := 1000 + Delta_in_ns;
  constant  Bit_long_time_max_cnt:  integer := Bit_long_time_max_ns * 1000 / CLK_in_ps;
  constant  Bit_long_time_min_ns:   integer := 1000 - Delta_in_ns;
  constant  Bit_long_time_min_cnt:  integer := Bit_long_time_min_ns * 1000 / CLK_in_ps;

  constant  Timeout_Time_ns:        integer := Sync_with_bit_max_ns + Delta_in_ns;
  constant  Timeout_Time_cnt:       integer := Timeout_Time_ns * 1000 / CLK_in_ps;
  
  constant  C_Time_between_2_Edges_cnt_width  : integer := How_many_bits(Timeout_Time_cnt)+1; -- um ein Bit groesser


  --------------------------------------------------
  -- Zaehler misst die Zeit zwischen zwei Flanken --
  --------------------------------------------------
  signal  S_Time_between_2_Edges_cnt  : unsigned(C_Time_between_2_Edges_cnt_width-1 downto 0);

  signal  S_Is_Sync:              std_logic;
  signal  S_Clr_Is_Sync:          std_logic;
  signal  S_Is_Sync_with_bit:     std_logic;
  signal  S_Clr_Is_Sync_with_bit: std_logic;
  signal  S_Is_Bit_short:         std_logic;
  signal  S_Clr_Is_Bit_Short:     std_logic;
  signal  S_Is_Bit_long:          std_logic;
  signal  S_Clr_Is_Bit_Long:      std_logic;
  signal  S_Is_Timeout:           std_logic;
  signal  S_Leave_Parity_Tst:     std_logic;
  signal  S_wait_short:           std_logic;

  signal  S_Next_Short:           std_logic;

  signal  S_Mil_Rcv_Shift_Reg:    std_logic_vector(Data_Size+2-1 downto 0);
                                    -- + 2, weil in Bit [0] das Parity-Bit gespeichert wird,    --
                                    --  und weil eine vorgeladene '1' als implizite Endekennung --
                                    --  verwendet wird. So lange Bit[17] = '0' ist laeuft die   --
                                    --  Datenaufnahme inklusive des Paritybits.                 --

  signal  S_Mil_Rcv_Data:         std_logic_vector(Data_Size-1 downto 0);

  signal  S_Manchester_Sync:      std_logic_vector(2 downto 0);
  signal  S_Edge_Detect:          std_logic;

  signal  S_Is_Cmd:               std_logic;
  signal  S_Is_Cmd_Memory:        std_logic;

  signal  S_MiL_Parity_Tst:       std_logic;

  signal  S_Rcv_Rdy:              std_logic;
  signal  S_Rcv_Rdy_Delayed:      std_logic;

  signal  S_Shift_Ena:            std_logic;

  signal  S_Parity_Ok:            std_logic;

  signal  S_Rcv_Error:            std_logic;
  

--+-----------------------------------------------------+
--|  CMD 8080hex Unipolar (negiert ueber Optokoppler)   |
--|       3   2             2 2             2           |
--|  _____   _  _ _ _ _ _ _ __  _ _ _ _ _ _ __ _____    |
--|       ___ __ _ _ _ _ _ _  __ _ _ _ _ _ _  _         |
--|       <------------ 18,5 us -------------->         |
--|                                                     |
--|  CMD 8000hex Unipolar (negiert ueber Optokoppler)   |
--|       3   2                                         | 
--|  _____   _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ ______    |
--|       ___ __ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _          |
--|       <------------ 18,0 us ------------->          |
--|                                                     |
--|  CMD 8080hex Bipolar                                |
--|       3  3   2             2 2             2        |
--|       ___   _  _ _ _ _ _ _ __  _ _ _ _ _ _ __       |
--|  _____   ___ __ _ _ _ _ _ _  __ _ _ _ _ _ _  _____  |
--|       <------------  19,5 us --------------->       |
--|                                                     |
--|                                                     |
--|  CMD 8000hex Bipolar                                |
--|       3  3   2                                      | 
--|       ___   _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _      |
--|  _____   ___ __ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _____ |
--|       <-------------  20,0 us --------------->      |
--|                                                     |
--+-----------------------------------------------------+
  
begin

assert not (Clk_in_Hz < 20_000_000)
  report "Die Freq. fuer 'Mil_dec_edge_timed_vhd' ist " & integer'image(Clk_in_Hz) & " Hz. Sie sollte aber > 20 MHz sein!"
severity error;

assert not (Clk_in_Hz >= 20_000_000)
  report "Die Freq. fuer 'Mil_dec_edge_timed_vhd' ist " & integer'image(Clk_in_Hz) & " Hz."
severity note;

assert false
  report "Clockperiode in ist " & integer'image(CLK_in_ps) & " ps."
severity note;

assert false
  report "Die Standard-bit-short-time betraegt " & integer'image(Bit_short_standard_ns) & " ns, dies entspricht einem Count von " &  integer'image(Bit_short_standard_cnt)
severity note;

assert false
  report "Timeoutzeit zwischen zwei Flanken betraegt " & integer'image(Timeout_Time_ns) & " ns, dies entspricht einem Count von " &  integer'image(Timeout_Time_cnt)
severity note;

P_Edge_Detect:  process (clk, S_Manchester_Sync)
  begin
    if rising_edge(clk) then
      S_Manchester_Sync(2 downto 0) <= (S_Manchester_Sync(1 downto 0) & not Manchester_In);
    end if;
    S_Edge_Detect   <= (not S_Manchester_Sync(2) and     S_Manchester_Sync(1))
                    or (    S_Manchester_Sync(2) and not S_Manchester_Sync(1));
  end process P_Edge_Detect;

  
P_Time_between_2_Edges_cnt: process (clk)
  begin
    if rising_edge(clk) then
      if S_Edge_Detect = '1' or RCV_SM = RCV_Idle then
        S_Time_between_2_Edges_cnt <= (others => '0');
      elsif S_Time_between_2_Edges_cnt(S_Time_between_2_Edges_cnt'high) = '0' then
        S_Time_between_2_Edges_cnt <= S_Time_between_2_Edges_cnt + 1;
      else
        S_Time_between_2_Edges_cnt <= S_Time_between_2_Edges_cnt;
      end if;
    end if;
  end process P_Time_between_2_Edges_cnt;


P_Is_Sync:  process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Sync_min_cnt and (RCV_SM = Sync1or2 or RCV_SM = Sync2orData) then
        S_Is_Sync <= '1';
      elsif S_Time_between_2_Edges_cnt = Sync_max_cnt or S_Clr_Is_Sync = '1' then
        S_Is_Sync <= '0';
      end if;
    end if;
  end process P_Is_Sync;
  

P_Is_Sync_with_bit: process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Sync_with_bit_min_cnt and (RCV_SM = Sync1or2 or RCV_SM = Sync2orData) then
        S_Is_Sync_with_bit <= '1';
      elsif S_Time_between_2_Edges_cnt = Sync_with_bit_max_cnt or S_Clr_Is_Sync_with_Bit = '1' then
        S_Is_Sync_with_bit <= '0';
      end if;
    end if;
  end process P_Is_Sync_with_bit;
  

P_Is_Bit_short: process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Bit_short_time_min_cnt and (RCV_SM = Data or RCV_SM = Sync2orData) then
        S_Is_Bit_short <= '1';
      elsif S_Time_between_2_Edges_cnt = Bit_short_time_max_cnt or S_Clr_Is_Bit_Short = '1' then
        S_Is_Bit_short <= '0';
      end if;
    end if;
  end process P_Is_Bit_short;
  
P_wait_short: process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Bit_short_standard_cnt-1 and RCV_SM = Parity then
        S_wait_short <= '1';
      else
        S_wait_short <= '0';
      end if;
    end if;
  end process P_wait_short;

P_Is_Bit_long:  process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Bit_long_time_min_cnt and RCV_SM = Data then
        S_Is_Bit_Long <= '1';
      elsif S_Time_between_2_Edges_cnt = Bit_long_time_max_cnt or S_Clr_Is_Bit_Long = '1' then
        S_Is_Bit_Long <= '0';
      end if;
    end if;
  end process P_Is_Bit_long;
  

P_Is_Timeout: process (clk)
  begin
    if rising_edge(clk) then
      if S_Time_between_2_Edges_cnt = Timeout_Time_cnt - 3 and RCV_SM = Parity then
        S_Leave_Parity_Tst <= '1';
      elsif S_Time_between_2_Edges_cnt = Timeout_Time_cnt then
        S_Is_Timeout <= '1';
      elsif RCV_SM = RCV_Idle then
        S_Leave_Parity_Tst <= '0';
        S_Is_Timeout <= '0';
      end if;
    end if;
  end process P_Is_Timeout;


P_Mil_Rcv_Data: process (clk, Res)
  begin
    if Res = '1' then
      S_Mil_Rcv_Data <= (others => '0');
    elsif rising_edge(clk) then
      if S_Parity_OK = '1' then
        S_Mil_Rcv_Data <= S_Mil_Rcv_Shift_Reg(16 downto 1);
      end if;
    end if;
  end process P_Mil_Rcv_Data;
  

P_Mil_Rcv_Shift_Reg:  process (clk)
  begin
    if rising_edge(clk) then
      if RCV_SM = RCV_Idle then
        S_Mil_Parity_Tst <= '0';
        S_Mil_Rcv_Shift_Reg <= std_logic_vector(to_unsigned(1, S_Mil_Rcv_Shift_Reg'length)); 
      elsif S_Shift_Ena = '1' then
        if Receive_pos_lane = 1 then
          S_Mil_Rcv_Shift_Reg <= (S_Mil_Rcv_Shift_Reg(S_Mil_Rcv_Shift_Reg'high-1 downto 0) & S_Manchester_Sync(2));
          if S_Manchester_Sync(2) = '1' then
           S_Mil_Parity_Tst <= not S_Mil_Parity_Tst;
          end if;
        else
          S_Mil_Rcv_Shift_Reg <= (S_Mil_Rcv_Shift_Reg(S_Mil_Rcv_Shift_Reg'high-1 downto 0) & not S_Manchester_Sync(2));
          if S_Manchester_Sync(2) = '0' then
            S_Mil_Parity_Tst <= not S_Mil_Parity_Tst;
          end if;
        end if;
      else
        S_Mil_Rcv_Shift_Reg <= S_Mil_Rcv_Shift_Reg;
      end if;
    end if;
  end process P_Mil_Rcv_Shift_Reg;


P_Rcv_Error:  process (Res, clk)
  begin
    if Res = '1' then
      S_Rcv_Error <= '0';
    elsif rising_edge(clk) then
      if RCV_SM = Err or  S_Is_Timeout = '1' then
        S_Rcv_Error <= '1';
      else
        S_Rcv_Error <= '0';
      end if;
    end if;
  end process P_Rcv_Error;


P_RCV_SM: process (clk, Res, S_Is_Timeout)
  begin
    if Res = '1' or S_Is_Timeout = '1' then
      RCV_SM <= RCV_Idle;
      S_Shift_Ena <= '0';
      S_Parity_OK <= '0';
      S_Clr_Is_Sync_with_bit <= '1';
      S_Clr_Is_Sync <= '1';
      S_Clr_Is_Bit_Long <= '1';
      S_Clr_Is_Bit_Short <= '1';
      S_Is_Cmd <= '0';

    elsif rising_edge(clk) then
      S_Shift_Ena <= '0';
      S_Parity_OK <= '0';     
      S_Clr_Is_Sync_with_bit <= '0';
      S_Clr_Is_Sync <= '0';
      S_Clr_Is_Bit_Long <= '0';
      S_Clr_Is_Bit_Short <= '0';
      
      case RCV_SM is
      
        when RCV_Idle =>
          S_Next_Short <= '0';
          if S_Edge_Detect = '1' then
            RCV_SM <= Sync1or2;
          end if;
          
        when Sync1or2 =>
          if S_Edge_Detect = '1' then
            if S_Is_Sync_with_bit = '1' then
              S_Clr_Is_Sync_with_bit <= '1';
              S_Shift_Ena <= '1';
              if Receive_pos_lane = 1 then S_Is_Cmd <= '0'; else S_Is_Cmd <= '1'; end if;
              RCV_SM <= Data;
            elsif S_Is_Sync = '1' then
              S_Clr_Is_Sync <= '1';
              RCV_SM <= Sync2orData;
            else
              RCV_SM <= RCV_Idle;
            end if;
          end if;
        
        when Sync2orData =>
          if S_Edge_Detect = '1' then
            if S_Is_Sync_with_bit = '1' then
              S_Clr_Is_Sync_with_bit <= '1';
              S_Shift_Ena <= '1';
              if Receive_pos_lane = 1 then S_Is_Cmd <= '1'; else S_Is_Cmd <= '0'; end if;
              RCV_SM <= Data;
            elsif S_Is_Sync = '1' then
              S_Clr_Is_Sync <= '1';
              if Receive_pos_lane = 1 then S_Is_Cmd <= '1'; else S_Is_Cmd <= '0'; end if;
              S_Next_Short <= '1';
              RCV_SM <= Data;
            elsif S_Is_Bit_short = '1' then
              S_Clr_Is_Bit_Short <= '1';
              if Receive_pos_lane = 1 then S_Is_Cmd <= '0'; else S_Is_Cmd <= '1'; end if;
              S_Shift_Ena <= '1';
              RCV_SM <= Data;
            else
              RCV_SM <= Err;
            end if;
          end if;

        when Data =>
          if S_Mil_Rcv_Shift_Reg(S_Mil_Rcv_Shift_Reg'high) = '0' then     
            if S_Edge_Detect = '1' then
              if (S_Is_Bit_short = '1' and S_Next_Short = '1') or (S_Is_Bit_long = '1' and S_Next_Short = '0') then
                S_Clr_Is_Bit_long <= '1';
                S_Clr_Is_Bit_short <= '1';
                S_Shift_Ena <= '1';
                S_Next_Short <= '0';
                RCV_SM <= Data;
              elsif S_Is_Bit_short = '1' and S_Next_Short = '0' then
                S_Clr_Is_Bit_short <= '1';
                S_Next_Short <= '1';
                RCV_SM <= Data;
              elsif S_Is_Bit_long = '1' and S_Next_Short = '1' then
                S_Clr_Is_Bit_Long <= '1';
                RCV_SM <= Err;
              else
                S_Clr_Is_Bit_Long <= '1';
                RCV_SM <= Err;
              end if;
            else
              RCV_SM <= Data;
            end if;
          else
            RCV_SM <= Parity;
          end if;

        when Parity =>
          if  (S_Mil_Rcv_Shift_Reg(0) = '1' and Receive_pos_lane = 0) or 
              (S_Mil_Rcv_Shift_Reg(0) = '0' and Receive_pos_lane = 1) then
          --+-------------------------------------------------------------------------------------------------------------------------+
          --| Das Empfangene letzte Bit war eine Null, d.h. es kommt noch eine Flanke! Diese muss noch abgewartet werden, sonst ist   |
          --| die Ablaufsteuerung zu frueh in RCV_Idle und wertet die letzte Flanke als Sync-Flanke, was zu einem Fehler fuehrt.      |
          --+-------------------------------------------------------------------------------------------------------------------------+
            if S_Edge_Detect = '1' or S_Leave_Parity_Tst = '1' then -- Falls die Flanke zu spaet kommt, soll kein Timeout auftreten.
                                                                    -- Es kann trotzdem zu einem Fehler kommen, wenn der 'Rcv_Idle' durch
                                                                    -- die 'spaete' Flanke verlassen wird und ein gueltiges Telegramm in einem
                                                                    -- 'passenden' Abstand beginnt und damit die 'spaete' Flanke nicht verworfen
                                                                    -- werden kann. Der Fehler ist nur im Blockmode moeglich.
              if Receive_pos_lane = 0 then
                if S_Mil_Parity_Tst = S_Mil_Rcv_Shift_Reg(0) then
                  S_Parity_OK     <= '1';
                  RCV_SM <= RCV_Idle;
                else
                  RCV_SM <= Err;
                end if;
              else
                if S_Mil_Parity_Tst = not S_Mil_Rcv_Shift_Reg(0) then
                  S_Parity_OK     <= '1';
                  RCV_SM <= RCV_Idle;
                else
                  RCV_SM <= Err;
                end if;
              end if;
            end if;
          --+-------------------------------------------------------------------------------------------------------------------------+
          --| Das Empfangene letzte Bit war eine Eins, d.h. es kommt keine Flanke mehr! Es kann direkt nach RCV_Idle verzweigt        |
          --| werden, wenn der Parity-Test keinen Fehler entdeckt.                                                                    |
          --+-------------------------------------------------------------------------------------------------------------------------+
          elsif S_wait_short = '1' then
            if Receive_pos_lane = 0 then
              if S_Mil_Parity_Tst = not S_Mil_Rcv_Shift_Reg(0) then
                S_Parity_OK       <= '1';
                RCV_SM <= RCV_Idle;
              else
                RCV_SM <= Err;
              end if;
            else
              if S_Mil_Parity_Tst = S_Mil_Rcv_Shift_Reg(0) then
                S_Parity_OK     <= '1';
                RCV_SM <= RCV_Idle;
              else
                RCV_SM <= Err;
              end if;
            end if;
          end if;

        when Err =>
          RCV_SM <= RCV_Idle;

        when others =>
          RCV_SM <= RCV_Idle;
          
        end case;
    end if;

  end process P_RCV_SM; 


P_Read: process (clk, Res)
  begin
    if Res = '1' then
      S_Rcv_Rdy <= '0';
      S_Is_Cmd_Memory <= '0';
    elsif rising_edge(clk) then
      S_Rcv_Rdy_Delayed <= S_Rcv_Rdy;
      if Rd_Mil = '1' or RCV_SM = Err then
        S_Rcv_Rdy <= '0';
        S_Is_Cmd_Memory <= '0';
      elsif S_Parity_Ok = '1' then
        if S_Is_CMD = '1' then
          S_Is_Cmd_Memory <= '1';
        else
          S_Is_Cmd_Memory <= '0';
        end if;
        S_Rcv_Rdy <= '1';
      end if;
    end if;
  end process P_Read;


Mil_Rcv_Data <= S_Mil_Rcv_Data;

Rcv_Rdy   <= S_Rcv_Rdy_Delayed;
Rcv_Cmd   <= S_Is_Cmd_Memory;
Rcv_Error <= S_Rcv_Error;

P_Diag: process (
        S_Is_Cmd_Memory, RCV_SM, S_Rcv_Rdy_Delayed, S_Edge_Detect, S_Is_Timeout,
        S_Is_Sync_with_bit, S_Is_Sync, S_Is_Bit_long, S_Is_Bit_short, S_Rcv_Error,
        S_Leave_Parity_Tst
        )
  begin
    if Receive_pos_lane = 1 then
      Mil_Decoder_Diag(15)  <=  '1';
    else
      Mil_Decoder_Diag(15)  <=  '0';
    end if;
    Mil_Decoder_Diag(14)  <=  S_Leave_Parity_Tst;
    Mil_Decoder_Diag(13)  <=  S_Is_Cmd_Memory;
    if RCV_SM = RCV_Idle then
      Mil_Decoder_Diag(12)  <=  '1';
    else
      Mil_Decoder_Diag(12)  <=  '0';
    end if;
    if RCV_SM = Sync1or2 then
      Mil_Decoder_Diag(11)  <=  '1';
    else
      Mil_Decoder_Diag(11)  <=  '0';
    end if;
    if RCV_SM = Parity then
      Mil_Decoder_Diag(10)  <=  '1';
    else
      Mil_Decoder_Diag(10)  <=  '0';
    end if;
    if RCV_SM = Sync2orData then
      Mil_Decoder_Diag(9) <=  '1';
    else
      Mil_Decoder_Diag(9) <=  '0';
    end if;
    if RCV_SM = Data then
      Mil_Decoder_Diag(8) <=  '1';
    else
      Mil_Decoder_Diag(8) <=  '0';
    end if;
    Mil_Decoder_Diag(7)   <=  S_Rcv_Error;
    Mil_Decoder_Diag(6)   <=  S_Rcv_Rdy_Delayed;  
    Mil_Decoder_Diag(5)   <=  S_Edge_Detect;  
    Mil_Decoder_Diag(4)   <=  S_Is_Timeout; 
    Mil_Decoder_Diag(3)   <=  S_Is_Sync_with_bit; 
    Mil_Decoder_Diag(2)   <=  S_Is_Sync;
    Mil_Decoder_Diag(1)   <=  S_Is_Bit_long;
    Mil_Decoder_Diag(0)   <=  S_Is_Bit_short;
  end process P_Diag;
  
end Arch_Mil_dec_edge_timed;
