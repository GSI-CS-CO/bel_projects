LIBRARY ieee;
use ieee.std_logic_1164.all;
use work.mil_pkg.all;

entity mil_hw_or_soft_ip is
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

    Clk:        in    std_logic;
    Rd_Mil:       in    std_logic;
    Mil_RCV_D:      out   std_logic_vector(15 downto 0);
    Mil_In_Pos:     in    std_logic;
    Mil_In_Neg:     in    std_logic;
    Mil_Cmd:      in    std_logic;
    Wr_Mil:       in    std_logic;
    Mil_TRM_D:      in    std_logic_vector(15 downto 0);
    EPLD_Manchester_Enc:  in    std_logic := '0';
    Reset_6408:     out   std_logic;
    Mil_Trm_Rdy:    out   std_logic;
    nSel_Mil_Drv:   out   std_logic;
    nSel_Mil_Rcv:   out   std_logic;
    nMil_Out_Pos:   out   std_logic;
    nMil_Out_Neg:   out   std_logic;
    Mil_Cmd_Rcv:    out   std_logic;
    Mil_Rcv_Rdy:    out   std_logic;
    Mil_Rcv_Error:    out   std_logic;
    No_VW_Cnt:      out   std_logic_vector(15 downto 0);    -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                                            -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
    Clr_No_VW_Cnt:    in    std_logic;                      -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                                            -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    Not_Equal_Cnt:    out   std_logic_vector(15 downto 0);  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal, Bit[7..0] Fehlerzaehler fuer
                                                            -- unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
    Clr_Not_Equal_Cnt:  in    std_logic;                    -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer
                                                            -- unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
                                                            -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    error_limit_reached:  out   std_logic;
    Mil_Decoder_Diag_p: out   std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n: out   std_logic_vector(15 downto 0)
    );
end mil_hw_or_soft_ip;


ARCHITECTURE arch_mil_hw_or_soft_ip OF mil_hw_or_soft_ip IS 

component ser_par
generic (
    Clk_in_Hz:  integer
    );
port  (
    Mil_WR:       in    std_logic;
    Mil_Send_CMD:   in    std_logic;
    ME_SD:        in    std_logic;
    ME_ESC:       in    std_logic;
    SEL_6408:     in    std_logic;
    RD_MIL:       in    std_logic;
    ME_CDS:       in    std_logic;
    ME_VW:        in    std_logic;
    ME_TD:        in    std_logic;
    ME_DSC:       in    std_logic;
    ME_SDO:       in    std_logic;
    nME_BOO:      in    std_logic;
    nME_BZO:      in    std_logic;
    Reset:        in    std_logic;
    Clk:      in    std_logic;
    DI:         in    std_logic_vector(15 downto 0);
    ME_SS:        out   std_logic;
    ME_SDI:       out   std_logic;
    ME_EE:        out   std_logic;
    nRCV_Ena:     out   std_logic;
    nTRM_Ena:     out   std_logic;
    Trm_Rdy:      out   std_logic;
    CMD_RCV:      out   std_logic;
    Valid_W:      out   std_logic;
    RCV_Err:      out   std_logic;
    Reset_6408:     out   std_logic;
    D_out:        out   std_logic_vector(15 downto 0)
    );
end component;


signal    nMil_OUT_Neg_M:     std_logic;
signal    nMil_Out_Pos_M:     std_logic;
signal    nSel_Mil_Drv_M:     std_logic;
signal    nSel_Mil_RCV_M:     std_logic;
signal    Mil_Cmd_Rcv_M:      std_logic;
signal    Mil_Rcv_D_M:      std_logic_vector(15 downto 0);
signal    Mil_RCV_Error_M:    std_logic;
signal    Mil_Rcv_Rdy_M:      std_logic;
signal    Mil_Trm_Rdy_M:      std_logic;

signal    nTRM_ENA_6408:      std_logic;
signal    CMD_RCV_6408:     std_logic;
signal    nRCV_ENA_6408:      std_logic;
signal    RCV_Err_6408:     std_logic;
signal    TRM_RDY_6408:     std_logic;
signal    Valid_W_6408:     std_logic;
signal    D_OUT:          std_logic_vector(15 downto 0);

signal  SEL_6408:     std_logic;    -- used for modelsim

begin 

SEL_6408 <= not EPLD_Manchester_Enc;

HW6408: ser_par
generic map (
      Clk_in_Hz => Clk_in_Hz
      )
port map  (
      Mil_WR      => Wr_Mil,
      Mil_Send_CMD  => Mil_Cmd,
      ME_SD     => ME_SD,
      ME_ESC      => ME_ESC,
      SEL_6408    => SEL_6408,
      RD_MIL      => Rd_Mil,
      ME_CDS      => ME_CDS,
      ME_VW     => ME_VW,
      ME_TD     => ME_TD,
      ME_DSC      => ME_DSC,
      ME_SDO      => ME_SDO,
      nME_BOO     => nME_BOO,
      nME_BZO     => nME_BZO,
      Reset     => Reset_Puls,
      Clk       => Clk,
      DI        => MIL_TRM_D,
      ME_SS     => ME_SS,
      ME_SDI      => ME_SDI,
      ME_EE     => ME_EE,
      nRCV_Ena    => nRCV_ENA_6408,
      nTRM_Ena    => nTRM_ENA_6408,
      Trm_Rdy     => TRM_RDY_6408,
      CMD_RCV     => CMD_RCV_6408,
      Valid_W     => Valid_W_6408,
      RCV_Err     => RCV_Err_6408,
      Reset_6408    => Reset_6408,
      D_out     => D_OUT
      );

      
mil_en_dec: mil_en_decoder
generic map (
      Clk_in_Hz   => Clk_in_Hz  -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen (kuerzester
                                -- Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                -- Achtung, die Frequenz von "Clk" bzw. "Clk" muss im Generic "clk_in_Hz" richtig beschrieben sein,
                                -- sonst ist die "Baudrate" des Manchester-Ein-/Ausgangsdatenstroms verkehrt.
      )
port map  (
      RES         => Reset_Puls,
      Test        => '0',               -- Nur zur Simulation verwenden. Test-Multiplexer, wenn Test = 1 wird der
                                        -- Ausgang des Mil_Encoders direkt auf den Eingang des Mil_Decoders geschaltet.
      Clk         => Clk,
      CMD_TRM       => Mil_Cmd,
      Wr_Mil        => Wr_Mil,          -- Startet ein Mil-Send, muß mindestens 1 Takt aktiv sein.
      Mil_TRM_D     => Mil_TRM_D,       -- solange Mil_Rdy_4_WR = '0' ist, muß hier das zu sendende Datum anliegen. 
      Rd_Mil        => Rd_Mil,          -- setzt Rcv_Rdy zurueck. Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
      Clr_No_VW_Cnt   => Clr_No_VW_Cnt, -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                        -- Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
      Clr_Not_Equal_Cnt => Clr_Not_Equal_Cnt, -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                              -- Komando-Daten-Kennung (CMD_not_equal). Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
      Mil_in_Pos      => Mil_In_Pos,        -- positiver Eingangsdatenstrom MIL-1553B
      Mil_in_Neg      => Mil_In_Neg,        -- negativer Eingangsdatenstrom MIL-1553B
      Mil_Rdy_4_WR    => Mil_Trm_Rdy_M,     -- Das Sende-Register ist frei.
      nSel_Mil_Rcv    => nSel_Mil_RCV_M,    -- '0' selektiert den Empfangspfad.
      nSel_Mil_Drv    => nSel_Mil_Drv_M,    -- selektiert die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).
      nMil_Out_Pos    => nMil_Out_Pos_M,    -- Der positive Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
      nMil_Out_Neg    => nMil_OUT_Neg_M,    -- Der negative Bipolare Ausgang des Manchester-Sende-Stroms (null-aktiv).
      RCV_Rdy       => Mil_Rcv_Rdy_M,       -- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
      RCV_ERROR     => Mil_RCV_Error_M,     -- nur die Fehler, die nicht durch Redundanz koorigiert werden koennen, werden mit einen Takt langen Puls signalisiert.
      CMD_Rcv       => Mil_Cmd_Rcv_M,       -- '1' es wurde ein Kommando empfangen.
      Mil_RCV_D     => Mil_Rcv_D_M,         -- Empfangenes Datum oder Komando
      No_VW_Cnt     => No_VW_Cnt,           -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p", Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
      Not_Equal_Cnt   => Not_Equal_Cnt,     -- Bit[15..8] Fehlerzaehler fuer Data_not_equal, Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
      error_limit_reached => error_limit_reached, 
      Mil_Decoder_Diag_p  => Mil_Decoder_Diag_p,  -- nur auswerten wenn, der Softcore-Manchester-Decoder aktiviert ist. Diagnoseausgaenge des Positiven Signalpfades.
      Mil_Decoder_Diag_n  => Mil_Decoder_Diag_n   -- nur auswerten wenn, der Softcore-Manchester-Decoder aktiviert ist. Diagnoseausgaenge des negativen Signalpfades.
      );
  

Mil_RCV_D <= Mil_Rcv_D_M when EPLD_Manchester_Enc = '1' else D_OUT;

nSel_Mil_Drv <= nSel_Mil_Drv_M when EPLD_Manchester_Enc = '1' else nTRM_ENA_6408;

nSel_Mil_Rcv <= nSel_Mil_RCV_M when EPLD_Manchester_Enc = '1' else nRCV_ENA_6408;

Mil_Rcv_Rdy <= Mil_Rcv_Rdy_M when EPLD_Manchester_Enc = '1' else Valid_W_6408;

Mil_Cmd_Rcv <= Mil_Cmd_Rcv_M when EPLD_Manchester_Enc = '1' else CMD_RCV_6408;

Mil_Trm_Rdy <= Mil_Trm_Rdy_M when EPLD_Manchester_Enc = '1' else TRM_RDY_6408;

nMil_Out_Pos <= nMil_Out_Pos_M when EPLD_Manchester_Enc = '1' else nME_BOO;

nMil_Out_Neg <= nMil_OUT_Neg_M when EPLD_Manchester_Enc = '1' else nME_BZO;

Mil_Rcv_Error <= Mil_RCV_Error_M when EPLD_Manchester_Enc = '1' else RCV_Err_6408;

ME_BOI <= '0' when EPLD_Manchester_Enc = '1' else MIL_in_Pos;
ME_BZI <= '1' when EPLD_Manchester_Enc = '1' else MIL_in_Neg;


end arch_mil_hw_or_soft_ip;