library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_arith.all;
use IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.all;

entity Mil_bipol_dec is
--+-------------------------------------------------------------------------------------------------------------------------------------+
--| Mil_bipol_dec:                                                                                                                      |
--|   Das positive und negative Manchester-Signal mit wird von zwei unhabhaengen Makros (Mil_dec_edge_timed) empfangen.                 |
--|   a)  Melden beide innerhalb der "max_jitter_ns" Zeit Valid Word, dann wird geprueft ob das empfangene Datum und die                |
--|       Kommando-Daten-Kennung Uebereinstimmt. Falls nicht wird "Data_not_equal" oder "CMD_not_Equal" generiert. Es wird kein         |
--|       "Rcv_Rdy" erzeugt.                                                                                                            |
--|       Stimmt alles ueberein wird "Rcv_Rdy" erzeugt.                                                                                 |
--|   b)  Hat einer der unabhaengigen Manchester-Dekoder ein gueltiges Telegramm empfangen, dann wird dies mit der entsprechenden       |
--|       Kommando-Daten-Kennung weitergegeben und "Rcv_Rdy" erzeugt. Der Manchester-Dekoder, der kein Telegramm empfangen hat,         |
--|       wird mit dem Fehlersignal "No_VW_p(n)" gekennzeichnet.                                                                        |
--|                                                                                                                                     |
--| Autor:    W.Panschow                                                                                                                |
--| Version:  1                                                                                                                         |
--| Datum:    29.02.12                                                                                                                  |
--|                                                                                                                                     |
--| Version: 2  Datum:  27.03.12  Autor:  W.Panschow                                                                                    |
--| Grund:    Wenn beide Decoder_n(p) ein Valid-Word geliefert haben, aber die Daten oder die Kommand-Daten-Kennung nicht uebereinstimmt,|
--|           wird kein "Rcv_Rdy" erzeugt. Dies hat zur Folge, dass die nachgeschaltete Datensenke auch kein "Rd_Mil" durchfuehrt.      |
--|           Dies hat wiederum zur Folge, dass die beiden Decoder nicht schnellst moeglich in den Idle-Zustand wechseln. In Version 2  |
--|           generieren die Fehler-Signale "Data_not_equal" oder "CMD_not_Equal" ein automatisches "RD_Mil"-Signal. D.H. beide Decoder |
--|           werden sofort zurueckgesetzt.                                                                                             |
--|                                                                                                                                     |
--| Version: 3  Datum:  28.03.12  Autor:  W.Panschow                                                                                    |
--| Grund:    Die Zeitdifferenz "max_jitter_ns" in der positive und negative Manchester-Encoder das "valid word" liefern muessen, war   |
--|           zu kurz definiert.  Dadurch wurde der Fehler "No_VW_p(n)" generiert, obwohl das Datum etwas spaeer empfangen wurde. Die   |
--|           erlaubte Differenz wurde von 100 ns auf 300 ns vergroessert.                                                              |
--|                                                                                                                                     |
--| Version: 4  Datum:  30.03.12  Autor:  W.Panschow                                                                                    |
--| Grund:    Zwei Fehlerzaehler die das no valid word des positiven und negativen Dekoders erfassen wurden eingebaut. Sie sind im      |
--|           Ausgang No_VW_Cnt zusammengefasst worden. Sie koennen mit dem Eingang Clr_No_VW_Cnt = '1' (muss mindesten eine 'clk'-     |
--|           Periode lang aktiv sein) zurueckgesetzt werden.                                                                           |
--|           Zwei weitere Fehlerzaehler erfassen, wenn beide Dekoder ein Telegram empfangen haben, ob ein Unterschied in den Daten oder|
--|           in der Komando-Daten-Kennung besteht. Beide Zaehler werden im Ausgang Not_Equal_Cnt zusammengefasst. Beide Zaehler werden |
--|           mit dem Eingang Clr_Not_Equal_Cnt = '1' (muss mindesten eine 'clk'-Periode lang aktiv sein) zurueckgesetzt.               |
--|                                                                                                                                     |
--| Version: 5  Datum:  02.04.12  Autor:  W.Panschow                                                                                    |
--| Grund:    Beim Einschalten der Interface-Karte koennte diese einen laufenden Transfer mit einer anderen Interface-Karte als Fehler  |
--|           auswerten. Dies wird vermieden, wenn die Fehlerzaehler der Interface-Karte erst aktivert werden, wenn das Power-up-Bit der|
--|           Interfacekarte quittiert wurde.                                                                                           |
--|                                                                                                                                     |
--| Version: 6  Datum:  23.04.12  Autor:  W.Panschow                                                                                    |
--| Grund:    Die Zeitdifferenz die zwischen Valid Word des positiven und des negativen Dekoders auftreten kann ist von der             |
--|           Devicebuslaenge abhaengig. Einer der beiden Dekoder muss immer darauf warten, dass das Paritiy-Bit die letzten Flanke mit |
--|           einem Spannungspegel beendet, der nicht dem Ruhepegel entspricht. Deshalb kommt am Ende der Parity-Bit-Zeit noch eine     |
--|           Flanke die durch das Ausschwingen in die Ruhespannung erzeugt wird. Dies wird aber nicht mehr aktiv betrieben, und dauert |
--|           bei einem laengeren Device-Bus durch die groessere kapatizive Last entsprechend läenger. Bis Version 5 war die            |
--|           Zeitdifferenz "max_jitter_ns" auf 300 ns festgelegt. In Version 6 ist "max_jitter_ns" auf 900 ns erhoeht worden.          |
--|           Das hat zur Folge, dass die Luecke zwischen den Telegrammen groesser als 900 ns sein muss.

--vk 20.082020
-- bipol select autogen hier hin kopiert
-- Anpassung der IOs
-- externe Reset fff nach hier verschoben
-- issue falsche clk domain "Manchester_clk"
-- vorsorglich schon mal sys_clk ergänzt
--+-------------------------------------------------------------------------------------------------------------------------------------+
  generic(
      CLK_in_Hz:        integer := 24_000_000;  -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                                -- in Hertz beschrieben werden.
      threshold_not_equal_err:  integer := 15;  -- Ueberschreiten die Fehlerzaehler "S_Data_not_equal_cnt" oder "S_CMD_not_equal_cnt"
                                                -- diesen Wert, wird der Ausgang "error_limit_reached" aktiv 'eins'.
      threshold_no_VW_err:      integer := 15   -- Ueberschreiten die Fehlerzaehler "S_No_VW_p_cnt" oder "S_No_VW_n_cnt" diesen Wert,
                                                -- wird der Ausgang "error_limit_reached" aktiv 'eins'.
      );
  port(
    sys_clk          : in  std_logic;
    sys_reset        : in  std_logic;        -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    M_in_p              : in  std_logic;        -- positiver Eingangsdatenstrom MIL-1553B
    M_in_n              : in  std_logic;        -- negativer Eingangsdatenstrom MIL-1553B
    RD_MIL              : in  std_logic;        -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
    Clr_No_VW_Cnt       : in  std_logic;        -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders. Muss synchron
                                                -- zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
    Clr_Not_Equal_Cnt:    in  std_logic;        -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                                -- Komando-Daten-Kennung (CMD_not_equal). Muss synchron zur Clock 'clk' und
                                                -- mindesten eine Periode lang aktiv sein!

    Power_up:             in  std_logic;        -- so lange Power_up = '1' ist, bleiben alle 4 Fehlerzaehler auf null.
    Manchester_clk:       in  std_logic;

    --vk ergänzt 20.08.2020
    Mil_Out_Pos      :    in  std_logic;
    Mil_Out_Neg      :    in  std_logic;
    Test             :    in  std_logic;

    Res_VW_Err       :    in  std_logic;

    rcv_error_ff     :    out std_logic;
    Rcv_Cmd:              out std_logic;        -- '1' es wurde ein Kommando empfangen.
    Rcv_Rdy:              out std_logic;        -- '1' es wurde ein Kommand oder Datum empfangen.
                                                -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data:         out std_logic_vector(15 downto 0);  -- Empfangenes Datum oder Komando
    Error_detect:         out std_logic;        -- Zusammengefassung aller moeglichen Fehlermeldungen (ein Takt aktiv '1').
    Rcv_Error_p:          out std_logic;        -- im positiven Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
    Rcv_Error_n:          out std_logic;        -- im negativen Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
    No_VW_p:              out std_logic;        -- im positiven Signalpfad kam kein Valid Word (ein Takt aktiv '1').
    No_VW_n:              out std_logic;        -- im negativen Signalpfad kam kein Valid Word (ein Takt aktiv '1').
    Data_not_equal:       out std_logic;        -- das Datum zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
    CMD_not_equal:        out std_logic;        -- das Komando zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
    No_VW_Cnt:            out std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                                              -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
    Not_Equal_Cnt:        out std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal,
                                                              -- Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal)
    error_limit_reached:  out std_logic;            -- wird aktiv 'eins' wenn die Fehlerzaehler die Generics "threshold_not_equal_err"
                                                    -- oder "threshold_no_VW_err" ueberschritten haben.
    Mil_Decoder_Diag_p:   out std_logic_vector(15 downto 0);  -- Diagnoseausgaenge des Manchester Decoders am positiven Signalpfad.
    Mil_Decoder_Diag_n:   out std_logic_vector(15 downto 0)   -- Diagnoseausgaenge des Manchester Decoders am negativen Signalpfad.


    );

end Mil_bipol_dec;

architecture Arch_Mil_bipol_dec of Mil_bipol_dec is


signal Manchester_In_p:     std_logic := '0';        -- positiver Eingangsdatenstrom MIL-1553B
signal Manchester_In_n:     std_logic := '0';        -- negativer Eingangsdatenstrom MIL-1553B


  type  T_Eval_RCV_SM is
                (
                RCV_Idle,
                RCV_p_RCV_n_together,
                RCV_p_first,
                RCV_n_first,
                RCV_fin,
                Wait_Rd_Mil
                );

  signal  Eval_RCV_SM : T_Eval_RCV_SM;

  constant  CLK_in_ps     : integer := (1000000000 / (CLK_in_Hz / 1000));


  constant  max_jitter_ns:    integer := 900;           -- maximale Zeitdiffenz zwischen dem valid word des positiven und negativen Dekoders
  constant  max_jitter_cnt:   integer := max_jitter_ns * 1000 / CLK_in_ps;
  constant  jitter_cnt_Width: integer := integer(ceil(log2(real(max_jitter_cnt))));
  signal    jitter_cnt:       std_logic_vector(jitter_cnt_Width downto 0);

  signal    S_Rcv_Rdy:        std_logic;
  signal    S_Rcv_Cmd:        std_logic := '0';
  signal    Res:              std_logic := '0';

component Mil_dec_edge_timed
  generic(
    CLK_in_Hz:        integer := 40_000_000;      -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                                  -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                                  -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                                  -- in Hertz beschrieben werden.
    Receive_pos_lane: integer range 0 TO 1 := 0   -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                                  -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    );
  port(
    Manchester_In:    in  std_logic;              -- Eingangsdatenstrom MIL-1553B
    RD_MIL:           in  std_logic;              -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine
                                                  -- Periode lang aktiv sein!
    Res:              in  std_logic;              -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    Clk:              in  std_logic;
    Rcv_Cmd:          out std_logic;              -- '1' es wurde ein Kommando empfangen.
    Rcv_Error:        out std_logic;              -- ist bei einem Fehler fuer einen Takt aktiv '1'.
    Rcv_Rdy:          out std_logic;              -- '1' es wurde ein Kommand oder Datum empfangen.
                                                  -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data:     out std_logic_vector(15 downto 0);  -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag: out std_logic_vector(15 downto 0)   -- Diagnoseausgaenge fuer Logikanalysator
    );
end component;


signal  RCV_Cmd_p:        std_logic;              -- von Mil_dec_p wurde ein Kommando empfangen.
signal  Rcv_p:            std_logic;              -- '1' es wurde von Mil_dec_p ein Kommando oder Datum empfangen.
                                                  -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
signal  Mil_Rcv_Data_p:   std_logic_vector(15 downto 0);  -- von Mil_dec_p empfangenes Datum oder Komando

signal  RCV_Cmd_n:        std_logic;              -- von Mil_dec_n wurde ein Kommando empfangen.
signal  Rcv_n:            std_logic;              -- '1' es wurde von Mil_dec_n ein Kommando oder Datum empfangen.
                                                  -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
signal  Mil_Rcv_Data_n:   std_logic_vector(15 downto 0);  -- von Mil_dec_n empfangenes Datum oder Komando

signal  S_Rcv_Error_p:    std_logic;              -- es ist ein Fehler auf der positiven Manchester-Spur aufgetreten
signal  S_Rcv_Error_n:    std_logic;              -- es ist ein Fehler auf der negativen Manchester-Spur aufgetreten
signal  S_No_VW_p:        std_logic;              -- es wurde kein Valid Word vom positiven Manchester-Dekoder erzeugt
signal  S_No_VW_n:        std_logic;              -- es wurde kein Valid Word vom negativen Manchester-Dekoder erzeugt
signal  S_Data_not_equal: std_logic;              -- Fehler: der positve und negative Manchester-Dekoder hat unterschiedliche Daten empfangen
signal  S_CMD_not_equal:  std_logic;              -- Fehler: der positve und negative Manchester-Dekoder hat
                                                  -- unterschiedliche Kommando/Daten-Kennung
signal  S_No_VW_p_cnt:    std_logic_vector(7 downto 0); -- Fehlerzaehler fuer fehlendes Valid Word vom positiven Manchester-Dekoder
signal  S_No_VW_n_cnt:    std_logic_vector(7 downto 0); -- Fehlerzaehler furr fehlendes Valid Word vom negativen Manchester-Dekoder
signal  S_Data_not_equal_cnt: std_logic_vector(7 downto 0); -- Fehlerzaehler wenn der positve und negative Manchester-Dekoder unterschiedliche
                                                            -- Daten empfangen hat
signal  S_CMD_not_equal_cnt:  std_logic_vector(7 downto 0); -- Fehlerzaehler wenn der positve und negative Manchester-Dekoder unterschiedliche
                                                            -- Kommando/Daten-Kennung erzeugt hat
signal  S_threshold_not_equal_err:  std_logic;        -- wird aktiv 'eins', wenn die Fehlerzaehler "S_Data_not_equal_cnt" oder "S_CMD_not_equal_cnt"
                                                      -- den Generic-Wert "threshold_not_equal_err" ueberschritten haben.
signal  S_threshold_no_VW_err:    std_logic;          -- wird aktiv 'eins', wenn die Fehlerzaehler "S_No_VW_p_cnt" oder "S_No_VW_n_cnt"
                                                      -- den Generic-Wert "threshold_no_VW_err" ueberschritten haben.
signal  S_Rd_Mil:       std_logic;

signal  S_CMD_not_eq_pulse:   std_logic;
signal  S_Data_not_eq_pulse:  std_logic;


begin

assert NOT (CLK_in_Hz < 20_000_000)
  report "Die Freq. fuer 'Mil_bipol_dec' ist " & integer'image(Clk_in_Hz) & " Hz. Sie sollte aber > 20 MHz sein!"
severity error;

assert NOT (CLK_in_Hz >= 20_000_000)
  report "Die Freq. fuer 'Mil_bipol_dec' ist " & integer'image(Clk_in_Hz) & " Hz."
severity note;

assert False
  report "Max_jitter_cnt ist " & integer'image(max_jitter_cnt) & ", und die Breite von jitter_cnt ist " & integer'image(jitter_cnt_Width + 1)
severity note;


Mil_dec_p:  Mil_dec_edge_timed
  generic map(
    CLK_in_Hz   => CLK_in_Hz,   -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                -- in Hertz beschrieben werden.
    Receive_pos_lane  => 1      -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    )
  port map (
    Manchester_In => Manchester_In_n,       -- Eingangsdatenstrom MIL-1553B. Achtung der negative Empfangsstrom
    RD_MIL        => S_RD_MIL,              -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine
                                            -- Periode lang aktiv sein!
    Res           => Res,                   -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    Clk           => Manchester_clk,
    Rcv_Cmd       => RCV_Cmd_p,             -- '1' es wurde ein Kommando empfangen.
    Rcv_Error     => S_Rcv_Error_p,         -- ist bei einem Fehler fuer einen Takt aktiv '1'.
    Rcv_Rdy       => Rcv_p,                 -- '1' es wurde ein Kommand oder Datum empfangen.
                                            -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data  => Mil_Rcv_Data_p,        -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag  => Mil_Decoder_Diag_p -- Diagnoseausgaenge fuer Logikanalysator
    );


Mil_dec_n:  Mil_dec_edge_timed
  generic map(
    CLK_in_Hz   => CLK_in_Hz,   -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                -- in Hertz beschrieben werden.
    Receive_pos_lane  => 0      -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    )
  port map (
    Manchester_In => Manchester_In_p,       -- Eingangsdatenstrom MIL-1553B. Achtung der positive Empfangsstrom
    RD_MIL        => S_RD_MIL,              -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine
                                            -- Periode lang aktiv sein!
    Res           => Res,                   -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    Clk           => Manchester_clk,
    Rcv_Cmd       => RCV_Cmd_n,             -- '1' es wurde ein Kommando empfangen.
    Rcv_Error     => S_Rcv_Error_n,         -- ist bei einem Fehler fuer einen Takt aktiv '1'.
    Rcv_Rdy       => Rcv_n,                 -- '1' es wurde ein Kommand oder Datum empfangen.
                                            -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data  => Mil_Rcv_Data_n,        -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag  => Mil_Decoder_Diag_n -- Diagnoseausgaenge fuer Logikanalysator
    );


P_Eval_RCV_SM:  process (Manchester_clk, Res)
  begin
    if Res = '1' then
      Eval_RCV_SM <= RCV_Idle;
      S_CMD_not_equal <= '0';
      S_Data_not_equal <= '0';
      S_No_VW_p <= '0';
      S_No_VW_n <= '0';
      S_Rcv_Rdy <= '0';

    elsif rising_edge(Manchester_clk) then

      S_No_VW_p <= '0';
      S_No_VW_n <= '0';
      S_CMD_not_eq_pulse <= '0';
      S_Data_not_eq_pulse <= '0';

      if RD_MIL = '1' then
        S_Rcv_Rdy <= '0';
      end if;


      case Eval_RCV_SM is

        when RCV_Idle =>

          S_CMD_not_equal <= '0';
          S_Data_not_equal <= '0';

          if S_Rcv_Rdy = '0' then                     -- nur wenn das vorherige Datum gelesen wurde, kann ein neues Datum empfangen werden!
            if Rcv_p = '1' and Rcv_n = '1' then
              Eval_RCV_SM <= RCV_p_RCV_n_together;    -- Beide Encoder haben gleichzeitig ein Telegram empfangen.
            elsif  Rcv_p = '1' and Rcv_n = '0' then
              Eval_RCV_SM <= RCV_p_first;             -- Der positive Encoder hat zuerst ein Telegram empfangen.
            elsif  Rcv_p = '0' and Rcv_n = '1' then
              Eval_RCV_SM <= RCV_n_first;             -- Der negative Encoder hat zuerst ein Telegram empfangen.
            end if;
          end if;

        when RCV_p_RCV_n_together =>                  -- Beide Encoder haben ein Telegram empfangen.

          if RCV_Cmd_p = RCV_Cmd_n then               -- teste ob beide Encoder die gleiche Kommando-Daten-Kennung haben
            S_RCV_Cmd <= RCV_Cmd_p;                   -- ja, uebernehme die Kommando-Daten-Kennung
          else
            S_CMD_not_equal <= '1';                   -- signalisiere ungleiche Kommand-Daten-Kennung
          end if;

          if Mil_Rcv_Data_p = Mil_Rcv_Data_n then     -- teste ob beide Encoder das gleiche Datum empfangen haben
            Mil_Rcv_Data <= Mil_Rcv_Data_p;           -- ja, uebernehme das Datum
          else
            S_Data_not_equal <= '1';                  -- signalisiere ungleiches Datum
          end if;

          Eval_RCV_SM <= RCV_fin;

        when RCV_p_first =>                           -- Der positive Encoder hat zuerst ein Telegram empfangen

          if jitter_cnt(jitter_cnt'high) = '0' then
            if Rcv_n = '1' then                       -- innerhalb der erlaubten Zeitspanne hat auch der negative Encoder ein Telegram empfangen
              Eval_RCV_SM <= RCV_p_RCV_n_together;    -- deshalb koennen beide Telegrame ausgewertet werden.
            end if;
          else
            S_RCV_Cmd <= RCV_Cmd_p;                   -- Kommando-Datenkennung
            Mil_Rcv_Data <= Mil_Rcv_Data_p;           -- und Datum vom positiven Encoder uebernehmen.
            S_No_VW_n <= '1';                         -- fehlender Empfang vom negativen Encoder signalisieren.
            Eval_RCV_SM <= RCV_fin;
          end if;

        when RCV_n_first =>                           -- Der negative Encoder hat zuerst ein Telegram empfangen

          if jitter_cnt(jitter_cnt'high) = '0' then
            if Rcv_p = '1' then                       -- innerhalb der erlaubten Zeitspanne hat auch der positive Encoder ein Telegram empfangen
              Eval_RCV_SM <= RCV_p_RCV_n_together;    -- deshalb koennen beide Telegrame ausgewertet werden.
            end if;
          else
            S_RCV_Cmd <= RCV_Cmd_n;                   -- Kommando-Datenkennung
            Mil_Rcv_Data <= Mil_Rcv_Data_n;           -- und Datum vom negativen Encoder uebernehmen.
            S_No_VW_p <= '1';                         -- fehlender Empfang vom positiven Encoder signalisieren.
            Eval_RCV_SM <= RCV_fin;
          end if;

        when RCV_fin =>
          S_CMD_not_eq_pulse <= S_CMD_not_equal;
          S_Data_not_eq_pulse <= S_Data_not_equal;
          if S_CMD_not_equal = '0' AND S_Data_not_equal = '0' then
            S_Rcv_Rdy <= '1';
            Eval_RCV_SM <= Wait_Rd_Mil;
          else
            Eval_RCV_SM <= RCV_Idle;                -- zwischen dem positiven und negativen Encoder ist eine Diffenz in der
                                                    -- Kommando-Daten-Kennung oder dem empfangenen Datum festgestellt worden.
          end if;

        when Wait_Rd_Mil =>
          if Rcv_p = '0' AND Rcv_n = '0'  then
           Eval_RCV_SM <= RCV_Idle;
          end if;

        when others =>

          Eval_RCV_SM <= RCV_Idle;

      end case;
    end if;

  end process P_Eval_RCV_SM;


P_jitter_cnt: process (Manchester_clk, Res)
  begin
    if Res = '1' then
      jitter_cnt <= conv_std_logic_vector(max_jitter_cnt-1, jitter_cnt'length);
    elsif rising_edge(Manchester_clk) then
      if Eval_RCV_SM = RCV_p_first or Eval_RCV_SM = RCV_n_first then
        if jitter_cnt(jitter_cnt'high) = '0' then
          jitter_cnt <= jitter_cnt - 1;
        end if;
      else
        jitter_cnt <= conv_std_logic_vector(max_jitter_cnt-1, jitter_cnt'length);
      end if;
    end if;
  end process P_jitter_cnt;


P_error_cnt:  process (Manchester_clk, Res, Power_up)
  begin
    if Res = '1' or Power_up = '1' then
      S_No_VW_p_cnt <= (others => '0');
      S_No_VW_n_cnt <= (others => '0');
      S_CMD_not_equal_cnt <= (others => '0');
      S_Data_not_equal_cnt <= (others => '0');
      S_threshold_no_VW_err <= '0';
      S_threshold_not_equal_err <= '0';

    elsif rising_edge(Manchester_clk) then

      if Clr_No_VW_Cnt = '1' then
        S_No_VW_p_cnt <= (others => '0');
      elsif S_No_VW_p_cnt < 255 AND S_No_VW_p = '1' then
        S_No_VW_p_cnt <= S_No_VW_p_cnt + 1;
      end if;

      if Clr_No_VW_Cnt = '1' then
        S_No_VW_n_cnt <= (others => '0');
      elsif S_No_VW_n_cnt < 255 AND S_No_VW_n = '1' then
        S_No_VW_n_cnt <= S_No_VW_n_cnt + 1;
      end if;

      if S_No_VW_p_cnt > threshold_no_VW_err or S_No_VW_n_cnt > threshold_no_VW_err then
        S_threshold_no_VW_err <= '1';
      else
        S_threshold_no_VW_err <= '0';
      end if;

      if Clr_Not_Equal_Cnt = '1' then
        S_CMD_not_equal_cnt <= (others => '0');
      elsif S_CMD_not_equal_cnt < 255 AND S_CMD_not_eq_pulse = '1' then
        S_CMD_not_equal_cnt <= S_CMD_not_equal_cnt + 1;
      end if;

      if Clr_Not_Equal_Cnt = '1' then
        S_Data_not_equal_cnt <= (others => '0');
      elsif S_Data_not_equal_cnt < 255 AND S_Data_not_eq_pulse = '1' then
        S_Data_not_equal_cnt <= S_Data_not_equal_cnt + 1;
      end if;

      if S_CMD_not_equal_cnt > threshold_not_equal_err or S_Data_not_equal_cnt > threshold_not_equal_err then
        S_threshold_not_equal_err <= '1';
      else
        S_threshold_not_equal_err <= '0';
      end if;

    end if;
  end process P_error_cnt;

No_VW_Cnt <= S_No_VW_p_cnt & S_No_VW_n_cnt;                   -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                                              -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"

Not_Equal_Cnt <= S_Data_not_equal_cnt & S_CMD_not_equal_cnt;  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal,
                                                              -- Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal).

error_limit_reached <= '1' when S_threshold_no_VW_err = '1' or S_threshold_not_equal_err = '1'
                           else '0';                          -- wird aktiv 'eins', wenn die Fehlerzaehler die Generics "threshold_not_equal_err"
                                                              -- oder "threshold_no_VW_err" ueberschritten haben.

S_RD_Mil <= Rd_Mil or S_Data_not_equal or S_CMD_not_equal;    -- S_Rd_Mil setzt 'rvc_rdy_p' und 'rcv_rdy_n' zurueck, und falls kein Fehler aufgetreten ist
                                                              -- setzt Rd_Mil (das empfangene Datum wird von extern gelesen) die Signale zurueck.

Rcv_Rdy <= S_Rcv_Rdy;

Error_Detect <= S_Rcv_Error_p or S_Rcv_Error_n or S_No_VW_p or S_No_VW_n or S_CMD_not_eq_pulse or S_Data_not_eq_pulse;

Rcv_Error_p <= S_Rcv_Error_p;
Rcv_Error_n <= S_Rcv_Error_n;
No_VW_p <= S_No_VW_p;
No_VW_n <= S_No_VW_n;
CMD_not_equal <= S_CMD_not_eq_pulse;
Data_not_equal <= S_Data_not_eq_pulse;
Rcv_Cmd <= S_Rcv_Cmd;


-------------------
--vk


Manchester_In_p <=   M_in_p when Test ='1' else NOT(Mil_Out_Pos);
Manchester_In_n <=   M_in_n when Test ='1' else NOT(Mil_Out_Neg);

--autogenerated vhdl
--vk issue falsche clk domain
PROCESS(Manchester_clk,sys_reset)
   BEGIN
      IF (RISING_EDGE(Manchester_clk)) THEN
         Res <= sys_reset;
      END IF;

END PROCESS;


--Reset VW-Antwort für VHDL-Decoder generieren
--->spaeter ins VHDl-Modul verlagern
  rcv_err_ff: process(sys_clk,Res_VW_Err)
  begin
    if Res_VW_Err = '1' then
      rcv_error_ff <= '0';
    elsif rising_edge(sys_clk) then
      rcv_error_ff <= '1';
    end if;
  end process;


end Arch_Mil_bipol_dec;
