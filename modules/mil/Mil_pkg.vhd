library IEEE;
use IEEE.STD_LOGIC_1164.all;
use work.aux_functions_pkg.all;

library work;

package mil_pkg is


component Mil_dec_edge_timed is
--+-------------------------------------------------------------------------------------------------------------------------------------+
--| "Mil_dec_edge_timed" empfaengt einen manchester kodierten Datenstrom nach MIL-STD-1553B Protokoll.                                  |
--|                                                                                                                                     |
--| Version 1;  Autor: W.Panschow; Datum: 16.02.2010                                                                                    |
--|                                                                                                                                     |
--| Version 2;  Autor: W.Panschow; Datum: 22.11.2012                                                                                    |
--| Änderungen:                                                                                                                         |
--|   1)  Das Enable das alle 500ns fuer einen Takt aktiv sein soll, wird jetzt aus der Mil_Clk erzeugt. Damit wird verhindert, dass    |
--|       das Enablewelches vormals außerhalb dieses Makros erzeugt wiurde eventuell von einer anderen Clock-Domaene abgeleitet wurde.  |
--|   2)  Die Umschaltung zwischen High- und Standard-Speed wurde entfernt. Es gibt nur noch Standard-Speed (Bitrate 1 us)              |
--|                                                                                                                                     |
--| Version 3;  Autor: W.Panschow; Datum: 13.09.2013                                                                                    |
--| Änderungen:                                                                                                                         |
--|   1)  Mit den Umzug des Makros ins GIT-Repository ist die Versionskennung aus dem Makro-Namen entfernt worden.                      |
--|   2)  Generic "Mil_CLK_in_Hz" in "CLK_in_Hz" umbenannt. Signal "Mil_Clk" in "clk" umbenannt.                                        |
--+-------------------------------------------------------------------------------------------------------------------------------------+
  generic(
    CLK_in_Hz:        integer := 24_000_000;    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                                -- in Hertz beschrieben werden.
    Receive_pos_lane: integer range 0 TO 1 := 0 -- 0 => der Manchester-Datenstrom wird bipolar ueber Übertrager empfangen.
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
    Mil_Decoder_Diag:   out std_logic_vector(15 downto 0);   -- Diagnoseausgaenge fuer Logikanalysator
    mil_err_cnt:        out std_logic_vector(31 downto 0);
    clr_mil_err_cnt:    in std_logic
    );
end component;


component mil_enc_vhdl is
--+---------------------------------------------------------------------------------------------------------------------------------+
--| "mil_enc_vhdl" sendet einen manchester kodierten Datenstrom nach MIL-STD-1553B Protokoll.                                       |
--|                                                                                                                                 |
--| Version 4;  Autor: W.Panschow; Datum: 13.11.2012                                                                                |
--| Änderungen:                                                                                                                     |
--|   1)  Das Enable das alle 500ns fuer einen Takt aktiv sein soll, wird jetzt aus der Clk erzeugt. Damit wird verhindert, dass das|
--|       Enable welches vormals außerhalb dieses Makros erzeugt wiurde eventuell von einer anderen Clock-Domaene abgeleitet wurde. |
--|   2)  Die Umschaltung zwischen High- und Standard-Speed wurde entfernt. Es gibt nur noch Standard-Speed (Bitrate 1 us)          |
--|                                                                                                                                 |
--| Version 5;  Autor: W.Panschow; Datum: 13.08.2013                                                                                |
--| Änderungen:                                                                                                                     |
--|   1)  Mit den Umzug des Makros ins GIT-Repository ist die Versionskennung aus dem Makro-Namen entfernt worden.                  |
--|   2)  Die Definition der Komponente "div_n" wird direkt aus dem "aux_functions_pkg" entnommen.                                  |
--+---------------------------------------------------------------------------------------------------------------------------------+
generic (Clk_in_Hz : INTEGER);
  port(
      Mil_TRM_D:    in    std_logic_vector(15 DOWNTO 0);  -- solange 'Mil_TRM' aktiv ist muß hier das zu sendende Datum anliegen.     -- 
      Cmd_Trm:      in    std_logic;          -- Cmd_Trm = Eins waehrend 'Wr_Mil' aktiv => ein Command-Sync. wird erzeugt, sonst       --
                                              -- wird ein Data-Sync. generiert.                                                       --
      Wr_Mil:       in    std_logic;          -- Startet ein Mil-Send, muß mindestens 1 Takt aktiv sein.                              --
      Clk:          in    std_logic;          -- Die Frequenz muß mindestens 4 MHz betragen.                                          --
      Reset:        in    std_logic;          -- Die Ablaufsteuerung 'Mil_TRM_SM' zurueckgesetzt, unterbricht ein laufendes Mil-Send. --
      nMil_Out_Pos: out   std_logic;          -- Der positive Bipolare Ausgang ist null-aktiv.                                        --
      nMil_Out_Neg: out   std_logic;          -- Der negative Bipolare Ausgang ist null-aktiv.                                        --
      nSel_Mil_Drv: out   std_logic;          -- Soll die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).   --
      nSel_Mil_Rcv: out   std_logic;          -- '0' selektiert den Empfangspfad.                                                     --
      Mil_Rdy_4_Wr: out   std_logic;          -- Das Sende-Register ist frei.                                                         --
      SD:           out   std_logic           -- V02: Bildet das Signal "SD" des 6408-ICs nach, wird fuer den Blockmode der           --
                                              -- Interfacekarte benoetigt.                                                             --
      );
end component;


component Mil_bipol_dec is
--+-------------------------------------------------------------------------------------------------------------------------------------+
--| Mil_bipol_dec:                                                                                                                      |
--|   Das positive und negative Manchester-Signal mit wird von zwei unhabhaengen Makros (Mil_dec_edge_timed) empfangen.                 |
--|   a)  Melden beide innerhalb der "max_jitter_ns" Zeit Valid Word, dann wird geprueft ob das empfangene Datum und die                |
--|       Kommando-Daten-Kennung uebereinstimmt. Falls nicht wird "Data_not_equal" oder "CMD_not_Equal" generiert. Es wird kein         |
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
--| Grund:  Wenn beide Decoder_n(p) ein Valid-Word geliefert haben, aber die Daten oder die Kommand-Daten-Kennung nicht ueberein-stimmt,|
--|         wird kein "Rcv_Rdy" erzeugt. Dies hat zur Folge, dass die nachgeschaltete Datensenke auch kein "Rd_Mil" durchfuehrt.        |
--|         Dies hat wiederum zur Folge, dass die beiden Decoder nicht schnellst moeglich in den Idle-Zustand wechseln. In Version 2    |
--|         generieren die Fehler-Signale "Data_not_equal" oder "CMD_not_Equal" ein automatisches "RD_Mil"-Signal. D.H. beide Decoder   |
--|         werden sofort zurueckgesetzt.                                                                                               |
--|                                                                                                                                     |
--| Version: 3  Datum:  28.03.12  Autor:  W.Panschow                                                                                    |
--| Grund:  Die Zeitdifferenz "max_jitter_ns" in der positive und negative Manchester-Encoder das "valid word" liefern muessen, war zu  |
--|         kurz definiert. Dadurch wurde der Fehler "No_VW_p(n)" generiert, obwohl das Datum etwas spaeter empfangen wurde. Die        |
--|         erlaubte Differenz wurde von 100 ns auf 300 ns vergroessert.                                                                |
--|                                                                                                                                     |
--| Version: 4  Datum:  30.03.12  Autor:  W.Panschow                                                                                    |
--| Grund:  Zwei Fehlerzaehler die das no valid word des positiven und negativen Dekoders erfassen wurden eingebaut. Sie sind im        |
--|         Ausgang No_VW_Cnt zusammengefasst worden. Sie koennen mit dem Eingang Clr_No_VW_Cnt = '1' (muss mindesten eine 'clk'-       |
--|         Periode lang aktiv sein) zurueckgesetzt werden.                                                                             |
--|         Zwei weitere Fehlerzaehler erfassen, wenn beide Dekoder ein Telegram empfangen haben, ob ein Unterschied in den Daten oder  |
--|         in der Komando-Daten-Kennung besteht. Beide Zaehler werden im Ausgang Not_Equal_Cnt zusammengefasst. Beide Zaehler werden   |
--|         mit dem Eingang Clr_Not_Equal_Cnt = '1' (muss mindesten eine 'clk'-Periode lang aktiv sein) zurueckgesetzt.                 |
--|                                                                                                                                     |
--| Version: 5  Datum:  02.04.12  Autor:  W.Panschow                                                                                    |
--| Grund:  Beim Einschalten der Interface-Karte koennte diese einen laufenden Transfer mit einer anderen Interface-Karte als Fehler    |
--|         auswerten. Dies wird vermieden, wenn die Fehlerzaehler der Interface-Karte erst aktivert werden, wenn das Power-up-Bit der  |
--|         Interfacekarte quittiert wurde.                                                                                             |
--|                                                                                                                                     |
--| Version: 6  Datum:  23.04.12  Autor:  W.Panschow                                                                                    |
--| Grund:  Die Zeitdifferenz die zwischen Valid Word des positiven und des negativen Dekoders auftreten kann ist von der               |
--|         Devicebuslaenge abhaengig. Einer der beiden Dekoder muss immer darauf warten, dass das Paritiy-Bit die letzten Flanke mit   |
--|         einem Spannungspegel beendet, der nicht dem Ruhepegel entspricht. Deshalb kommt am Ende der Parity-Bit-Zeit noch eine       |
--|         Flanke die durch das Ausschwingen in die Ruhespannung erzeugt wird. Dies wird aber nicht mehr aktiv betrieben, und dauert   |
--|         bei einem laengeren Device-Bus durch die groessere kapatizive Last entsprechend laeenger. Bis Version 5 war die             |
--|         Zeitdifferenz "max_jitter_ns" auf 300 ns festgelegt. In Version 6 ist "max_jitter_ns" auf 900 ns erhoeht worden.            |
--|         Das hat zur Folge, dass die Luecke zwischen den Telegrammen groesser als 900 ns sein muss.                                  |
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
    Manchester_In_p:      in  std_logic;        -- positiver Eingangsdatenstrom MIL-1553B
    Manchester_In_n:      in  std_logic;        -- negativer Eingangsdatenstrom MIL-1553B
    RD_MIL:               in  std_logic;        -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
    Clr_No_VW_Cnt:        in  std_logic;        -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders. Muss synchron 
                                                -- zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
    Clr_Not_Equal_Cnt:    in  std_logic;        -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                                -- Komando-Daten-Kennung (CMD_not_equal). Muss synchron zur Clock 'clk' und
                                                -- mindesten eine Periode lang aktiv sein!
    Res:                  in  std_logic;        -- Muss mindestens einmal fuer eine Periode von 'clk' aktiv ('1') gewesen sein.
    Power_up:             in  std_logic;        -- so lange Power_up = '1' ist, bleiben alle 4 Fehlerzaehler auf null.
    Clk:                  in  std_logic;
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
end component;


component mil_en_decoder is 
--+---------------------------------------------------------------------------------------------------------------------------------+
--| "mil_enc_decoder"  realisiert Sende- und Empfangsfunktion zu/von einem manchester kodierten Feldbus.                            |
--| Die Kodierung entspricht dem MIL-STD-1553B Protokoll.                                                                           |
--|                                                                                                                                 |
--| Version 1;  Autor: W.Panschow; Datum: 15.08.2013                                                                                |
--+---------------------------------------------------------------------------------------------------------------------------------+
generic (
    CLK_in_Hz:        integer := 24_000_000     -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                                -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                                -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                                -- in Hertz beschrieben werden.
    );
port  (
    RES:            in      std_logic;              -- Muss mindestens einmal fuer eine Periode von 'Clk' aktiv ('1') gewesen sein.
    Test:           in      std_logic := '0';       -- Nur zur Simulation verwenden. Test-Multiplexer, wenn Test = 1 wird der
                                                    -- Ausgang des Mil_Encoders direkt auf den Eingang des Mil_Decoders geschaltet.
    Clk:            in      std_logic;
    CMD_TRM:        in      std_logic;              -- Cmd_Trm = Eins waehrend 'Wr_Mil' aktiv => ein Command-Sync. wird erzeugt,
                                                    -- sonst wird ein Data-Sync. generiert.
    Wr_Mil:         in      std_logic;              -- Startet ein Mil-Send, muß mindestens 1 Takt aktiv sein.
    Mil_TRM_D:      in      std_logic_vector(15 downto 0);  -- solange Mil_Rdy_4_WR = '0' ist, muß hier das zu sendende Datum anliegen. 
    Rd_Mil:         in      std_logic;              -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'Clk' und mindesten eine Periode
                                                    -- lang aktiv sein!
    Clr_No_VW_Cnt:  in      std_logic;              -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                                    -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    Clr_Not_Equal_Cnt:  in    std_logic;            -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                                    -- Komando-Daten-Kennung (CMD_not_equal). Muss synchron zur Clock 'Clk' und mindestens
                                                    -- eine Periode lang aktiv sein!
    Mil_in_Neg:     in      std_logic;              -- negativer Eingangsdatenstrom MIL-1553B
    Mil_in_Pos:     in      std_logic;              -- positiver Eingangsdatenstrom MIL-1553B
    Mil_Rdy_4_WR:   out     std_logic;              -- Das Sende-Register ist frei.
    nSel_Mil_RCV:   out     std_logic;              -- '0' selektiert den Empfangspfad.
    nSel_Mil_DRV:   out     std_logic;              -- selektiert die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).
    nMil_Out_Pos:   buffer  std_logic;              -- Der positive Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
    nMil_Out_Neg:   buffer  std_logic;              -- Der negative Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
    RCV_Rdy:        out     std_logic;              -- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    RCV_ERROR:      out     std_logic;
    CMD_Rcv:        out     std_logic;              -- '1' es wurde ein Kommando empfangen.
    Mil_RCV_D:      out     std_logic_vector(15 downto 0);  -- Empfangenes Datum oder Komando
    No_VW_Cnt:      out     std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                                            -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n".
    Not_Equal_Cnt:  out     std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal,
                                                            -- Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
    error_limit_reached:  out   std_logic;                  -- wird aktiv 'eins' wenn die Fehlerzaehler die Generics "threshold_not_equal_err"
                                                            -- oder "threshold_no_VW_err" ueberschritten haben.
    Mil_Decoder_Diag_p: out   std_logic_vector(15 downto 0);-- Diagnoseausgaenge des Positiven Signalpfades, 
                                                            -- nur auswerten wenn, der Softcore-Manchester-Decoder aktiviert ist. 
    Mil_Decoder_Diag_n: out   std_logic_vector(15 downto 0) -- Diagnoseausgaenge des negativen Signalpfades,
                                                            -- nur auswerten wenn, der Softcore-Manchester-Decoder aktiviert ist. 
    );
end component;


component mil_hw_or_soft_ip is
--+---------------------------------------------------------------------------------------------------------------------------------+
--| "mil_hw_or_soft_ip"  stellt zwei unterschiedliche Interfaces zum Betreiben eines manchester encodierten Feldbusses bereit.      |
--| Die Encodierung entpricht dem MIL-STD-1553B Protokoll.                                                                          |
--|                                                                                                                                 |
--| Interface 1: kommuniziert mit externer Hardware (IC = HD6408). Hierfuer steht das _hw_ im Namen dieses Makros. Es stellt den    |
--| Manchester-Encoder und -Decoder bereit. Auf FPGA-Seite sind hauptsaechlich die Schieberegister fuer die Seriell-Parallel-       |
--| Wandlung (Empfangsweg) und die Parallel-Seriell-Wandlung (Sendeweg) realisiert.                                                 |
--|                                                                                                                                 |
--| Interface 2: braucht keine externe Hardware zur Manchester-Encodierung. Die Encodierung wird im FPGA durchgefuehrt. Dafuer      |
--| _soft_ip_ im Namen dieses Makros.                                                                                               |
--|                                                                                                                                 |
--| Die Auswahl mit welchem Interace gearbeitet werden soll, wird durch den Eingang EPLD_Manchester_Enc gesteuert. Eine '1'         |
--| selektiert den im EPLD realisierten Manchester-Encoder.                                                                         |
--| Die Kodierung entspricht dem MIL-STD-1553B Protokoll.                                                                           |
--|                                                                                                                                 |
--| Version 1;  Autor: W.Panschow; Datum: 15.08.2013                                                                                |
--+---------------------------------------------------------------------------------------------------------------------------------+
generic (
    Clk_in_Hz:    INTEGER := 125_000_000      -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                              -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                              -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                              -- in Hertz beschrieben werden.
    );
port  (
    -- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:        in      std_logic;      -- output:  transmit bipolar positive.
    nME_BZO:        in      std_logic;      -- output:  transmit bipolar negative.
    
    ME_SD:          in      std_logic;      -- output:  '1' => send data is active.
    ME_ESC:         in      std_logic;      -- output:  encoder shift clock for shifting data into the encoder. The
                                            --          encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:         out     std_logic;      -- input:   serial data in accepts a serial data stream at a data rate
                                            --          equal to encoder shift clock.
    ME_EE:          out     std_logic;      -- input:   a high on encoder enable initiates the encode cycle.
                                            --          (Subject to the preceding cycle being completed).
    ME_SS:          out     std_logic;      -- input:   sync select actuates a Command sync for an input high
                                            --          and data sync for an input low.
    Reset_Puls:     in      std_logic;

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:         out     std_logic;      -- input:   A high input should be applied to bipolar one in when the bus is in its
                                            --          positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:         out     std_logic;      -- input:   A high input should be applied to bipolar zero in when the bus is in its
                                            --          negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:         out     std_logic;      -- input:   With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                            --          transition finder circuit. If not used this input must be held low.
    ME_CDS:         in      std_logic;      -- output:  high occurs during output of decoded data which was preced
                                            --          by a command synchronizing character. Low indicares a data sync.
    ME_SDO:         in      std_logic;      -- output:  serial data out delivers received data in correct NRZ format.
    ME_DSC:         in      std_logic;      -- output:  decoder shift clock delivers a frequency (decoder clock : 12),
                                            --          synchronized by the recovered serial data stream.
    ME_VW:          in      std_logic;      -- output:  high indicates receipt of a VALID WORD.
    ME_TD:          in      std_logic;      -- output:  take data is high during receipt of data after identification
                                            --          of a sync pulse and two valid Manchester data bits

    Clk:            in      std_logic;
    Rd_Mil:         in      std_logic;
    Mil_RCV_D:      out     std_logic_vector(15 downto 0);
    Mil_In_Pos:     in      std_logic;
    Mil_In_Neg:     in      std_logic;
    Mil_Cmd:        in      std_logic;
    Wr_Mil:         in      std_logic;
    Mil_TRM_D:      in      std_logic_vector(15 downto 0);
    EPLD_Manchester_Enc:  in    std_logic := '0';
    Reset_6408:     out   std_logic;
    Mil_Trm_Rdy:    out   std_logic;
    nSel_Mil_Drv:   out   std_logic;
    nSel_Mil_Rcv:   out   std_logic;
    nMil_Out_Pos:   out   std_logic;
    nMil_Out_Neg:   out   std_logic;
    Mil_Cmd_Rcv:    out   std_logic;
    Mil_Rcv_Rdy:    out   std_logic;
    Mil_Rcv_Err:    out   std_logic;
    No_VW_Cnt:      out   std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                                          -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
    Clr_No_VW_Cnt:  in    std_logic;                      -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                                          -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    Not_Equal_Cnt:  out   std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal,
                                                          -- Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
    Clr_Not_Equal_Cnt:  in    std_logic;                  -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                                          -- Komando-Daten-Kennung (CMD_not_equal).
                                                          -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    error_limit_reached:  out   std_logic;
    Mil_Decoder_Diag_p: out   std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n: out   std_logic_vector(15 downto 0);
    clr_mil_rcv_err:    in    std_logic;
    hw6408_rdy:         out   std_logic
    );
end component;

end package mil_pkg;
