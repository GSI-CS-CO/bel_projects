

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_arith.all;
use IEEE.STD_LOGIC_unsigned.all;
use work.aux_functions_pkg.all; 

entity mil_enc_vhdl is
--+---------------------------------------------------------------------------------------------------------------------------------+
--| "mil_enc_vhdl" sendet einen manchester kodierten Datenstrom nach MIL-STD-1553B Protokoll.                                       |
--|                                                                                                                                 |
--| Version 4;  Autor: W.Panschow; Datum: 13.11.2012                                                                                |
--| Aenderungen:                                                                                                                    |
--|   1)  Das Enable das alle 500ns fuer einen Takt aktiv sein soll, wird jetzt aus der Clk erzeugt. Damit wird verhindert, dass das|
--|       Enable welches vormals außerhalb dieses Makros erzeugt wiurde eventuell von einer anderen Clock-Domaene abgeleitet wurde. |
--|   2)  Die Umschaltung zwischen High- und Standard-Speed wurde entfernt. Es gibt nur noch Standard-Speed (Bitrate 1 us)          |
--|                                                                                                                                 |
--| Version 5;  Autor: W.Panschow; Datum: 13.08.2013                                                                                |
--| Aenderungen:                                                                                                                    |
--|   1)  Mit den Umzug des Makros ins GIT-Repository ist die Versionskennung aus dem Makro-Namen entfernt worden.                  |
--|   2)  Die Definition der Komponente "div_n" wird direkt aus dem "aux_functions_pkg" entnommen.                                  |
--+---------------------------------------------------------------------------------------------------------------------------------+
generic (Clk_in_Hz : INTEGER);
  port(
      Mil_TRM_D:    in    std_logic_vector(15 DOWNTO 0);  -- solange 'Mil_TRM' aktiv ist muß hier das zu sendende Datum anliegen.    --
      Cmd_Trm:      in    std_logic;          -- Cmd_Trm = Eins waehrend 'Wr_Mil' aktiv => ein Command-Sync. wird erzeugt, sonst     --
                                              -- wird ein Data-Sync. generiert.                                                      --
      Wr_Mil:       in    std_logic;          -- Startet ein Mil-Send, muß mindestens 1 Takt aktiv sein.                             --
      Clk:          in    std_logic;          -- Die Frequenz muß mindestens 4 MHz betragen.                                         --
      Reset:        in    std_logic;          -- Die Ablaufsteuerung 'Mil_TRM_SM' zurueckgesetzt, unterbricht ein laufendes Mil-Send.--
      nMil_Out_Pos: out   std_logic;          -- Der positive Bipolare Ausgang ist null-aktiv.                                       --
      nMil_Out_Neg: out   std_logic;          -- Der negative Bipolare Ausgang ist null-aktiv.                                       --
      nSel_Mil_Drv: out   std_logic;          -- Soll die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).  --
      nSel_Mil_Rcv: out   std_logic;          -- '0' selektiert den Empfangspfad.                                                    --
      Mil_Rdy_4_Wr: out   std_logic;          -- Das Sende-Register ist frei.                                                        --
      SD:           out   std_logic           -- V02: Bildet das Signal "SD" des 6408-ICs nach, wird fuer den Blockmode der          --
                                              -- Interfacekarte benoetigt.                                                           --
      );
    
end mil_enc_vhdl;

architecture Arch_mil_enc_vhdl of mil_enc_vhdl is

type  T_Encode_SM is
          (
          TRM_Idle,
          Sync1,
          Sync2,
          Sync3,
          Sync4,
          Sync5,
          Sync6,
          IDLE_LO,
          IDLE_HI,
          TRM_LO,
          TRM_HI
          );

signal  S_Encode_SM : T_Encode_SM;


constant  Clk_in_ns:        integer := 1_000_000_000 / Clk_in_Hz;
constant  c_cnt_for_ever_500ns: integer := 500 / Clk_in_ns;

  
signal  s_every_500ns:    std_logic;

signal  S_Shift_Reg:      std_logic_vector(15 DOWNTO 0);  -- In 'shift' wird das Sendedatum[15..0] in   --
                                                          -- einen seriellen Bitstrom umgewandelt.      --

signal  S_Bit_cnt:        std_logic_vector(4 DOWNTO 0);   -- Der Zaehler bestimmt das Ende des seriellen --
                                                          -- Datenstromes (inklusive Parity-Bit).        --
signal  S_Odd_Parity:     std_logic;

signal  S_Wr_Mil_Merker:  std_logic;

signal  S_Mil_Rdy_4_Wr_Dly: std_logic_vector(3 DOWNTO 0);   -- V03: Damit zwischen zwei TRMs mindestens 1 us Pause gemacht wird. 

signal  S_Cmd_Trm:        std_logic;

signal  S_Out_pos:        std_logic;
signal  S_Out_neg:        std_logic;

signal  S_Bipol_Out_Pos:  std_logic;    -- Der bipolare positive Ausgang ist ein getaktetes Register, damit werden etwaige  --
                                        -- Uebergangsfunktionen von 'Out_Pos' unterdrueckt.                                 --
signal  S_Bipol_Out_Neg:  std_logic;    -- Der bipolare negative Ausgang ist ein getaktetes Register, damit werden etwaige  --
                                        -- Uebergangsfunktionen von 'Out_Neg' unterdrueckt.                                 --

signal  S_Sel_Mil_Drv:    std_logic;    -- Sind 'Out_Pos' und 'Out_Neg' ungleich in ihrem Logikpegel werden die Mil-Treiber aktiviert.--
                                        -- Damit etwaige Uebergangsfunktionen weggefiltert werden ist das Register getaktet.          --

signal  S_SD:             std_logic;    -- Bildet das Signal "SD" des 6408-ICs nach, wird fuer den Blockmode der Interfacekarte benoetigt.--

signal  delay_n_rcv_en:   std_logic_vector(1 DOWNTO 0); -- soll nach dem Senden eines Telegramms das Selektieren des Empfangspfades verzoegern  --

begin

ASSERT not (500 rem Clk_in_ns /= 0)
  REPORT "Die Berechnung 'c_cnt_for_every_500ns' soltte ganzzahlig sein. Sie hat aber den Wert von => " & real'image(real(500) / real(Clk_in_ns))
SEVERITY note;

sel_every_500ns: div_n
  generic map (
    n       => c_cnt_for_ever_500ns,
    diag_on => 1
    )
  port map  (
    res   => Reset,
    clk   => Clk,
    ena   => '1',
    div_o => s_every_500ns
    );

P_delay_n_rcv_en: process (Clk)
  begin
    if rising_edge(Clk) then
      if s_every_500ns = '1' then
        delay_n_rcv_en <= delay_n_rcv_en(0) & S_Sel_Mil_Drv;
      end if;
    end if;
  end process P_delay_n_rcv_en;


P_Cmd_Trm:  process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Cmd_Trm <= '0';
    elsif rising_edge(Clk) then
      if Wr_Mil = '1' then
        S_Cmd_Trm <= Cmd_Trm;
      end if;
    end if;
  end process P_Cmd_Trm;


P_Odd_Parity: process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Odd_Parity <= '0';
    elsif rising_edge(Clk) then
      if (S_Encode_SM = Idle_HI AND s_every_500ns = '1') OR (S_Encode_SM = Trm_Idle AND S_Odd_Parity = '1') then
        S_Odd_Parity <= not S_Odd_Parity;
      end if;
    end if;
  end process P_Odd_Parity;


P_Wr_Mil_Merker:  process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Wr_Mil_Merker <= '0';
    elsif rising_edge(Clk) then
      if S_Encode_SM = Sync1 then
        S_Wr_Mil_Merker <= '0';
      elsif Wr_Mil = '1' then
        S_Wr_Mil_Merker <= '1';
      end if;
    end if;
  end process P_Wr_Mil_Merker;


P_Shift_Reg:  process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Shift_Reg <= (OTHERS => '0');
    elsif rising_edge(Clk) then
      if Wr_Mil = '1' then
        S_Shift_Reg <= Mil_TRM_D;
      elsif (S_Encode_SM = Idle_LO OR S_Encode_SM = Idle_HI) AND s_every_500ns = '1' then
        S_Shift_Reg <= (S_Shift_Reg(S_Shift_Reg'high-1 DOWNTO 0) & '0');
      end if;
    end if;
  end process P_Shift_Reg;


P_Bit_cnt:  process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Bit_cnt <= (OTHERS => '0');
    elsif rising_edge(Clk) then
      if S_Encode_SM = TRM_Idle then
        S_Bit_cnt <= (OTHERS => '0');
      elsif (S_Encode_SM = Idle_LO OR S_Encode_SM = Idle_HI) AND s_every_500ns = '1' then
        S_Bit_cnt <= S_Bit_cnt + 1;
      end if;
    end if;
  end process P_Bit_cnt;


P_Encode_SM:  process (Clk, Reset)    
  begin
    if Reset = '1' then
      S_Encode_SM <= TRM_Idle;
      S_Out_pos <= '0';
      S_Out_neg <= '0';
      S_SD <= '0';                  -- V02

    elsif rising_edge(Clk) then
      if s_every_500ns = '1' then
      
        S_Out_pos <= '0';
        S_Out_neg <= '0';
      
        case S_Encode_SM IS
          when TRM_Idle =>
            S_SD <= '0';                  -- V02
            if S_Wr_Mil_Merker = '1' AND Wr_Mil = '0' then  -- Wr_Mil muss abgeschlossen sein sonst shifted das Shift-Register waerend  --
              S_Encode_SM <= Sync1;                         -- des Ladens, weil Trm_Idle schon waehrend Wr_Mil verlassen wuerde.        --
            end if;

          when Sync1 =>
            S_Out_pos <= S_Cmd_Trm;
            S_Out_neg <= not S_Cmd_Trm;
            S_Encode_SM <= Sync2;
          when Sync2 =>
            S_Out_pos <= S_Cmd_Trm;
            S_Out_neg <= not S_Cmd_Trm;
            S_Encode_SM <= Sync3;
          when Sync3 =>
            S_Out_pos <= S_Cmd_Trm;
            S_Out_neg <= not S_Cmd_Trm;
            S_Encode_SM <= Sync4;

          when Sync4 =>
            S_Out_pos <= not S_Cmd_Trm;
            S_Out_neg <= S_Cmd_Trm;
            S_Encode_SM <= Sync5;
          when Sync5 =>
            S_Out_pos <= not S_Cmd_Trm;
            S_Out_neg <= S_Cmd_Trm;
            S_Encode_SM <= Sync6;
          when Sync6 =>
            S_Out_pos <= not S_Cmd_Trm;
            S_Out_neg <= S_Cmd_Trm;
            S_SD <= '1';                  -- V02
            if S_Shift_Reg(S_Shift_Reg'high) = '1' then
              S_Encode_SM <= Idle_HI;
            else
              S_Encode_SM <= Idle_LO;
            end if;

          when IDLE_LO =>
            S_Out_pos <= '0';
            S_Out_neg <= '1';
            S_Encode_SM <= TRM_LO;

          when TRM_LO =>
            S_Out_pos <= '1';
            S_Out_neg <= '0';
            if S_Bit_cnt < 16 then
              if S_Shift_Reg(S_Shift_Reg'high) = '1' then
                S_Encode_SM <= Idle_HI;
              else
                S_Encode_SM <= Idle_LO;
              end if;
            elsif S_Bit_cnt = 16 then
              S_SD <= '0';                -- V02
              if S_Odd_Parity = '0' then          -- Das Odd-Parity-Bit zaehlt selbst mit bei der Parity-Bildung, --
                S_Encode_SM <= Idle_HI;           -- deshalb wird bei Parity == GND nach Idle_HI verzweigt.       --
              else
                S_Encode_SM <= Idle_LO;
              end if;
            else
              S_Encode_SM <= TRM_Idle;
            end if;

          when IDLE_HI =>
            S_Out_pos <= '1';
            S_Out_neg <= '0';
            S_Encode_SM <= TRM_HI;

          when TRM_HI =>
            S_Out_pos <= '0';
            S_Out_neg <= '1';
            if S_Bit_cnt < 16 then
              if S_Shift_Reg(S_Shift_Reg'high) = '1' then
                S_Encode_SM <= Idle_HI;
              else
                S_Encode_SM <= Idle_LO;
              end if;
            elsif S_Bit_cnt = 16 then
              S_SD <= '0';                -- V02
              if S_Odd_Parity = '0' then          -- Das Odd-Parity-Bit zaehlt selbst mit bei der Parity-Bildung, --
                S_Encode_SM <= Idle_HI;           -- deshalb wird bei Parity == GND nach Idle_HI verzweigt.       --
              else
                S_Encode_SM <= Idle_LO;
              end if;
            else
              S_Encode_SM <= TRM_Idle;
            end if;
        
        end case;
        
      end if;
    end if;
  end process P_Encode_SM;


P_No_Glitch:  process (Clk)   
  begin
    if rising_edge(Clk) then
      S_Bipol_Out_Pos <= S_Out_pos;       -- Um Uebergangsfunktionen von 'Out_Pos/Neg' zu vermeiden wird    --
      S_Bipol_Out_Neg <= S_Out_neg;       -- die Logik ueber Systemtakt getaktete Register gefuehrt.        --
      S_Sel_Mil_Drv <= S_Out_pos XOR S_Out_neg; -- Nur bei ungleichen Logikpegel werden die externen Treiber der    --
                                                -- bipol. Ausgaenge aktiviert. Um Uebergangsfuktionen zu vermeiden, --
                                                -- und um das gleiche Timing zu haben wie 'Bipol_Out_Pos/Neg', wird --
                                                -- die Logik ueber Systemtakt getaktete Register gefuehrt.          --
    end if;
  end process P_No_Glitch;


P_Rdy_4_Wr: process (Clk)   
  begin
    if rising_edge(Clk) then
      if Wr_Mil = '1' then                                                      -- V03
        S_Mil_Rdy_4_Wr_Dly(3 downto 0) <= X"0";                                 -- V03
      elsif S_Encode_SM = TRM_Idle AND s_every_500ns = '1' then                 -- V03
        S_Mil_Rdy_4_Wr_Dly(3 downto 0) <= S_Mil_Rdy_4_Wr_Dly(2 downto 0) & '1'; -- V03
      end if;
    end if;
  end process P_Rdy_4_Wr;
  


nMil_Out_Pos <= not S_Bipol_Out_Pos;      -- Die Bipolaren Ausgaenge muessen an den externen Treibern --
nMil_Out_Neg <= not S_Bipol_Out_Neg;      -- 'aktiv null' sein.                                       --
nSel_Mil_Drv <= not S_Sel_Mil_Drv;        -- Die externen Treiber werden mit Null-Pegel aktiviert.    --
nSel_Mil_Rcv <= S_Sel_Mil_Drv or delay_n_rcv_en(1);

Mil_Rdy_4_Wr <= S_Mil_Rdy_4_Wr_Dly(1);        -- V03

SD <= S_SD;       -- Bildet das Signal "SD" des 6408-ICs nach, wird fuer den Blockmode der Interfacekarte benoetigt.  --

end;

