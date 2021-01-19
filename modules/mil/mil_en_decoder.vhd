LIBRARY ieee;
use ieee.std_logic_1164.all; 
use work.mil_pkg.all;

ENTITY mil_en_decoder IS 
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
    Wr_Mil:         in      std_logic;              -- Startet ein Mil-Send, muÃŸ mindestens 1 Takt aktiv sein.
    Mil_TRM_D:      in      std_logic_vector(15 downto 0);  -- solange Mil_Rdy_4_WR = '0' ist, muÃŸ hier das zu sendende Datum anliegen. 
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
end mil_en_decoder;


ARCHITECTURE arch_mil_en_decoder OF mil_en_decoder IS 


SIGNAL  Neg_In :  std_logic;
SIGNAL  Pos_In :  std_logic;
SIGNAL  nMil_Out_Pos_Buf : std_logic;
SIGNAL  nMil_Out_Neg_Buf : std_logic;

BEGIN 

-- Test-Multiplexer, wenn Test = 1 wird der Ausgang des Mil_Encoders direkt auf den Eingang des Mil_Decoders geschaltet.
Pos_in <= not nMil_Out_Pos_Buf when (Test = '1') else Mil_in_Pos;
Neg_in <= not nMil_Out_Neg_Buf when (Test = '1') else Mil_in_Neg;

nMil_Out_Pos <= nMil_Out_Pos_Buf;
nMil_Out_Neg <= nMil_Out_Neg_Buf;


mil_dec:  Mil_bipol_dec
generic map (
      Clk_in_Hz       => Clk_in_Hz,   -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen (kuerzester
                                      -- Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                      -- Achtung, die Frequenz von "Mil_Clk" muss im Generic "Clk_in_Hz" richtig beschrieben sein,
                                      -- sonst ist die "Baudrate" des Manchester-Ein-/Ausgangsdatenstroms verkehrt.
      threshold_not_equal_err => 15,  -- ueberschreiten die Fehlerzaehler "S_Data_not_equal_cnt" oder "S_CMD_not_equal_cnt" diesen Wert,
                                      -- dann wird der Ausgang "error_limit_reached" aktiv 'eins'. 
      threshold_no_VW_err   => 15     -- ueberschreiten die Fehlerzaehler "S_No_VW_p_cnt" oder "S_No_VW_n_cnt" diesen Wert,
                                      -- dann wird der Ausgang "error_limit_reached" aktiv 'eins'. 
      )
port map  (
      Manchester_In_p   => Neg_In,        -- positiver Eingangsdatenstrom MIL-1553B
      Manchester_In_n   => Pos_In,        -- negativer Eingangsdatenstrom MIL-1553B
      RD_MIL            => Rd_Mil,        -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'Mil_Clk' und mindesten eine Periode lang aktiv sein!
      Clr_No_VW_Cnt     => Clr_No_VW_Cnt,   -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                            -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
      Clr_Not_Equal_Cnt => Clr_Not_Equal_Cnt, -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                              -- Komando-Daten-Kennung (CMD_not_equal).
                                              -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
      Res               => RES,           -- Muss mindestens einmal fuer eine Periode von 'Clk' aktiv ('1') gewesen sein.
      Power_up          => '0',           -- so lange Power_up = '1' ist, bleiben alle 4 Fehlerzaehler auf null.
      Clk           => Clk,
      Rcv_Cmd         => CMD_Rcv,       -- '1' es wurde ein Kommando empfangen.
      Rcv_Rdy         => RCV_Rdy,       -- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
      Mil_Rcv_Data      => Mil_RCV_D,     -- Empfangenes Datum oder Komando
      Error_detect      => RCV_ERROR,     -- Zusammengefassung der nicht korrigierbarenn Fehlermeldungen (ein Takt aktiv '1').
      No_VW_Cnt       => No_VW_Cnt,       -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                          -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
      Not_Equal_Cnt     => Not_Equal_Cnt, -- Bit[15..8] Fehlerzaehler fuer Data_not_equal, Bit[7..0] Fehlerzaehler
                                          -- fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal)
      error_limit_reached   => error_limit_reached, -- wird aktiv 'eins', wenn die Fehlerzaehler die Generics "threshold_not_equal_err" oder 
                                                    -- "threshold_no_VW_err" ueberschritten haben.
      Mil_Decoder_Diag_p    => Mil_Decoder_Diag_p,  -- Diagnoseausgaenge des Manchester Decoders am positiven Signalpfad.
      Mil_Decoder_Diag_n    => Mil_Decoder_Diag_n,  -- Diagnoseausgaenge des Manchester Decoders am negativen Signalpfad.
      Rcv_Error_p       => open,          -- im positiven Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
      Rcv_Error_n       => open,          -- im negativen Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
      No_VW_p         => open,            -- im positiven Signalpfad kam kein Valid Word (ein Takt aktiv '1').
      No_VW_n         => open,            -- im negativen Signalpfad kam kein Valid Word (ein Takt aktiv '1').
      Data_not_equal      => open,        -- das Datum zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
      CMD_not_equal     => open           -- das Komando zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
      );


mil_enc : mil_enc_vhdl
generic map (
      Clk_in_Hz => Clk_in_Hz            -- Achtung, die Frequenz von "Clk" muss im Generic "Clk_in_Hz" richtig beschrieben sein,
                                        -- sonst ist die "Baudrate" des Manchester-Ausgangsdatenstroms verkehrt.
      )
port map  (
      Cmd_Trm       => CMD_TRM,       -- Cmd_Trm = Eins waehrend 'Wr_Mil' aktiv => ein Command-Sync. wird erzeugt, sonst wird ein Data-Sync. generiert.
      Wr_Mil        => Wr_Mil,        -- Startet ein Mil-Send, muÃŸ mindestens 1 Takt aktiv sein.
      Clk           => Clk,           -- Achtung, die Frequenz von "Clk" muss im Generic "Clk_in_Hz" richtig beschrieben sein,
                                      -- sonst ist die "Baudrate" des Manchester-Ausgangsdatenstroms verkehrt.
      Reset         => RES,           -- Die Ablaufsteuerung 'Mil_TRM_SM' wird zurueckgesetzt, unterbricht ein laufendes Mil-Send.
      Mil_TRM_D     => Mil_TRM_D,     -- solange Mil_Rdy_4_Wr gleich '0' ist, muss hier das zu sendende Datum anliegen.
      nMil_Out_Pos  => nMil_Out_Pos_Buf,-- Der positive Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
      nMil_Out_Neg  => nMil_Out_Neg_Buf,-- Der negative Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
      nSel_Mil_Drv  => nSel_Mil_Drv,  -- selektiert die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).
      nSel_Mil_Rcv  => nSel_Mil_Rcv,  -- '0' selektiert den Empfangspfad.
      Mil_Rdy_4_Wr  => Mil_Rdy_4_WR,  -- Das Sende-Register ist frei.
      SD        => open               -- V02: Bildet das Signal "SD" des 6408-ICs nach, wird fuer den Blockmode der Interfacekarte benoetigt. --
      );

end arch_mil_en_decoder;