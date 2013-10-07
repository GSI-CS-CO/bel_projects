LIBRARY ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;
use work.aux_functions_pkg.all;
use work.mil_pkg.all;
use work.wb_mil_scu_pkg.all;

ENTITY wb_mil_scu IS 
generic (
    Clk_in_Hz:  INTEGER := 125_000_000    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    );
port  (
    clk_i:          in    std_logic;
    nRst_i:         in    std_logic;
    slave_i:        in    t_wishbone_slave_in;
    slave_o:        out   t_wishbone_slave_out;
    
    -- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:        in      std_logic;      -- HD6408-output: transmit bipolar positive.
    nME_BZO:        in      std_logic;      -- HD6408-output: transmit bipolar negative.
    
    ME_SD:          in      std_logic;      -- HD6408-output: '1' => send data is active.
    ME_ESC:         in      std_logic;      -- HD6408-output: encoder shift clock for shifting data into the encoder. The
                                            --                encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:         out     std_logic;      -- HD6408-input:  serial data in accepts a serial data stream at a data rate
                                            --                equal to encoder shift clock.
    ME_EE:          out     std_logic;      -- HD6408-input:  a high on encoder enable initiates the encode cycle.
                                            --                (Subject to the preceding cycle being completed).
    ME_SS:          out     std_logic;      -- HD6408-input:  sync select actuates a Command sync for an input high
                                            --                and data sync for an input low.

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:         out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar one in when the bus is in its
                                            --                positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:         out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar zero in when the bus is in its
                                            --                negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:         out     std_logic;      -- HD6408-input:  With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                            --                transition finder circuit. If not used this input must be held low.
    ME_CDS:         in      std_logic;      -- HD6408-output: high occurs during output of decoded data which was preced
                                            --                by a command synchronizing character. Low indicares a data sync.
    ME_SDO:         in      std_logic;      -- HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC:         in      std_logic;      -- HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                            --                synchronized by the recovered serial data stream.
    ME_VW:          in      std_logic;      -- HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD:          in      std_logic;      -- HD6408-output: take data is high during receipt of data after identification
                                            --                of a sync pulse and two valid Manchester data bits

    -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
--    ME_12MHz:       out     std_logic;      -- HD6408-input:    is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
    

    Mil_BOI:        in  std_logic;          -- HD6408-input:  connect positive bipolar receiver, in FPGA directed to the external
                                            --                manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
                                            --                vhdl manchester macro.
    Mil_BZI:        in  std_logic;          -- HD6408-input:  connect negative bipolar receiver, in FPGA directed to the external
                                            --                manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
                                            --                vhdl manchester macro.
    Sel_Mil_Drv:    buffer  std_logic;      -- HD6408-output: active high, enable the external open collector driver to the transformer
    nSel_Mil_Rcv:   out     std_logic;      -- HD6408-output: active low, enable the external differtial receive circuit.
    Mil_nBOO:       out     std_logic;      -- HD6408-output: connect bipolar positive output to external open collector driver of
                                            --                the transformer. Source is the external manchester en/decoder HD6408 via
                                            --                nME_BOO or the internal FPGA vhdl manchester macro.
    Mil_nBZO:       out     std_logic;      -- HD6408-output: connect bipolar negative output to external open collector driver of
                                            --                the transformer. Source is the external manchester en/decoder HD6408 via
                                            --                nME_BZO or the internal FPGA vhdl manchester macro.
    nLed_Mil_Rcv:   out     std_logic;
    nLed_Mil_Trm:   out     std_logic;
    nLed_Mil_Err:   out     std_logic;
    error_limit_reached:  out   std_logic;
    Mil_Decoder_Diag_p: out   std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n: out   std_logic_vector(15 downto 0);
    timing:         in      std_logic;
    nLed_Timing:    out     std_logic;
    Interlock_Intr: in      std_logic;
    Data_Rdy_Intr:  in      std_logic;
    Data_Req_Intr:  in      std_logic;
    nLed_Interl:    out     std_logic;
    nLed_Dry:       out     std_logic;
    nLed_Drq:       out     std_logic
    );
end wb_mil_scu;


ARCHITECTURE arch_wb_mil_scu OF wb_mil_scu IS 

component Mil_12Mhz
  PORT (
    inclk0    : IN STD_LOGIC  := '0';
    c0    : OUT STD_LOGIC ;
    locked    : OUT STD_LOGIC 
  );
end component;


-- allowed wishbone addresses
constant  mil_wr_data_a:      unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#00#, c_mil_addr_width-2);
constant  mil_wr_cmd_a:       unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#01#, c_mil_addr_width-2);
constant  mil_wr_rd_status_a: unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#02#, c_mil_addr_width-2);
constant  rd_wr_no_vw_cnt_a:  unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#03#, c_mil_addr_width-2);
constant  rd_wr_not_eq_cnt_a: unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#04#, c_mil_addr_width-2);

constant  test_echo_1_a:      unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#10#, c_mil_addr_width-2);
constant  test_echo_2_a:      unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#fff#, c_mil_addr_width-2);
constant  test_event_a:       unsigned(c_mil_addr_width-1 downto 2) := to_unsigned(16#ffe#, c_mil_addr_width-2);

-- bit positions of mil control/status register
constant  b_sel_fpga_n6408: integer := 15;  -- '1' => fpga manchester endecoder selected, '0' => external hardware manchester endecoder 6408 selected.
constant  b_ev_filt_12_n8:  integer := 14;  -- '1' => event filter decode 12 bit of the event, '0' => event filter decode 8 bit of the event.
constant  b_ev_filter_on:   integer := 13;  -- '1' => event filter is on, '0' => event filter is off.
constant  b_debounce_on:    integer := 12;  -- '1' => debounce of device bus interrupt input is on.
constant  b_puls2_rahmen:   integer := 11;  -- '1' => aus zwei events wird der Rahmenpuls2 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_puls1_rahmen:   integer := 10;  -- '1' => aus zwei events wird der Rahmenpuls1 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_ev_reset_on:    integer := 09;  -- '1' => events koennen den event timer auf Null setzen, vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_mil_rcv_err:    integer := 08;

constant  b_mil_rcv_rdy:    integer := 4;   -- '1' => command or data received from mil bus.
constant  b_ev_fifo_full:   integer := 3;   -- '1' => event fifo is full.
constant  b_data_req:       integer := 2;   -- '1' => data request interrupt of device bus is active.
constant  b_data_rdy:       integer := 1;   -- '1' => data ready interrupt of device bus is active.
constant  b_interlock:      integer := 0;   -- '1' => Interlock of device bus is active.

signal    manchester_fpga:  std_logic;  -- '1' => fpga manchester endecoder selected, '0' => external hardware manchester endecoder 6408 selected.
signal    ev_filt_12_n8:    std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal    ev_filter_on:     std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal    debounce_on:      std_logic;  -- '1' => debounce of device bus interrupt input is on.
signal    puls2_rahmen:     std_logic;  -- '1' => aus zwei events wird der Rahmenpuls2 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal    puls1_rahmen:     std_logic;  -- '1' => aus zwei events wird der Rahmenpuls1 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal    ev_reset_on:      std_logic;  -- '1' => events koennen den event timer auf Null setzen, vorausgesetzt das Eventfilter ist richtig programmiert.
signal    clr_mil_rcv_err:  std_logic;


signal    Sel_EPLD_n6408:   std_logic;  --!!!!
signal    Mil_Rcv_Err_ff:   std_logic;  --!!!!

signal    Mil_RCV_D:      std_logic_vector(15 downto 0);
signal    Mil_Cmd_Rcv:    std_logic;
signal    mil_trm_rdy:    std_logic;
signal    mil_rcv_rdy:    std_logic;
signal    mil_rcv_error:  std_logic;

signal    clr_no_vw_cnt:    std_logic;
signal    no_vw_cnt:        std_logic_vector(15 downto 0);

signal    clr_not_equal_cnt:  std_logic;
signal    not_equal_cnt:    std_logic_vector(15 downto 0);


signal    Reset_6408:     std_logic;

signal  ex_stall, ex_ack, ex_err, intr: std_logic;  -- dummy

signal    mil_trm_start:  std_logic;
signal    mil_trm_cmd:    std_logic;
signal    mil_trm_data:   std_logic_vector(15 downto 0);

signal    mil_rd_start:   std_logic;

signal    test_echo_1:    std_logic_vector(31 downto 0);
signal    test_echo_2:    std_logic_vector(31 downto 0);

signal    timing_cmd, timing_rcv, event_fin:  std_logic;
signal    event_d:        std_logic_vector(15 downto 0);

signal    ena_led_count:  std_logic;

signal    nSel_Mil_Drv:   std_logic;


begin


slave_o.stall             <= ex_stall;
slave_o.ack               <= ex_ack;
slave_o.int               <= Intr;
slave_o.err               <= ex_err;
slave_o.rty               <= '0';


ena_led_cnt: div_n
  generic map (
    n         => integer(real(clk_in_Hz) * 0.02),   -- Vorgabe der Taktuntersetzung. 20 ms
    diag_on   => 0                        -- diag_on = 1 die Breite des Untersetzungzaehlers
                                          -- mit assert .. note ausgegeben.
    )

  port map (
    res       => open,
    clk       => clk_i,
    ena       => open,            -- das untersetzende enable muss in der gleichen Clockdomäne erzeugt werden.
                                  -- Das enable sollte nur ein Takt lang sein.
                                  -- Z.B. könnte eine weitere div_n-Instanz dieses Signal erzeugen.  
    div_o     => ena_led_count    -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );


led_rcv: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Mil_Rcv_Rdy,     -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_Mil_Rcv     -- open drain output, active low, inactive tristate.
    );


led_trm: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Sel_Mil_Drv,     -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_Mil_Trm     -- open drain output, active low, inactive tristate.
    );


led_err: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Mil_Rcv_Error,   -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_Mil_Err     -- open drain output, active low, inactive tristate.
    );

led_interl: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Interlock_Intr,  -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_Interl      -- open drain output, active low, inactive tristate.
    );

led_dry: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Data_Rdy_Intr,   -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_dry     -- open drain output, active low, inactive tristate.
    );

led_drq: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => Data_Req_Intr,   -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_drq         -- open drain output, active low, inactive tristate.
    );

led_timing: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => timing_cmd and timing_rcv,   -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => nLed_Timing      -- open drain output, active low, inactive tristate.
    );

Serial_Timing:  mil_dec_edge_timed
  generic map (
    CLK_in_Hz         => clk_in_Hz,     -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                        -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                        -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                        -- in Hertz beschrieben werden.
    Receive_pos_lane  => 0              -- '0' => der Manchester-Datenstrom wird bipolar über Übertrager empfangen.
                                        -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                        -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    )
  port map (
    Manchester_In     => Timing,        -- Eingangsdatenstrom MIL-1553B
    RD_MIL            => event_fin,     -- setzt Rvc_Cmd, Rcv_Rdy und Rcv_Error zurück. Muss synchron zur Clock 'clk' und 
                                        -- mindesten eine Periode lang aktiv sein!
    Res               => not nRst_i,    -- Muss mindestens einmal für eine Periode von 'clk' aktiv ('1') gewesen sein.
    clk               => clk_i,
    Rcv_Cmd           => timing_cmd,    -- '1' es wurde ein Kommando empfangen.
    Rcv_Error         => open,          -- ist bei einem Fehler für einen Takt aktiv '1'.
    Rcv_Rdy           => timing_rcv,    -- '1' es wurde ein Kommand oder Datum empfangen.
                                        -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data      => event_d,       -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag  => open           -- Diagnoseausgänge für Logikanalysator
    );

event_fin <= timing_rcv and timing_cmd;

Mil_1:  mil_hw_or_soft_ip
  generic map (
    Clk_in_Hz =>  Clk_in_Hz      -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                 -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                 -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                 -- in Hertz beschrieben werden.
    )
  port map  (
    -- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BZO       =>  nME_BZO,      -- in: HD6408-output: transmit bipolar positive.
    nME_BOO       =>  nME_BOO,      -- in: HD6408-output: transmit bipolar negative.
    
    ME_SD         =>  ME_SD,        -- in: HD6408-output: '1' => send data is active.
    ME_ESC        =>  ME_ESC,       -- in: HD6408-output: encoder shift clock for shifting data into the encoder. The,
                                    --                    encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI        =>  ME_SDI,       -- out: HD6408-input: serial data in accepts a serial data stream at a data rate
                                    --                    equal to encoder shift clock.
    ME_EE         =>  ME_EE,        -- out: HD6408-input: a high on encoder enable initiates the encode cycle.
                                    --                    (Subject to the preceding cycle being completed).
    ME_SS         =>  ME_SS,        -- out: HD6408-input: sync select actuates a Command sync for an input high
                                    --                    and data sync for an input low.
    Reset_Puls    =>  not nRst_i,

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI        =>  ME_BOI,       -- out: HD6408-input: A high input should be applied to bipolar one in when the bus is in its
                                    --                    positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI        =>  ME_BZI,       -- out: HD6408-input: A high input should be applied to bipolar zero in when the bus is in its
                                    --                    negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI        =>  ME_UDI,       -- out: HD6408-input: With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                    --                    transition finder circuit. If not used this input must be held low.
    ME_CDS        =>  ME_CDS,       -- in: HD6408-output: high occurs during output of decoded data which was preced
                                    --                    by a command synchronizing character. Low indicares a data sync.
    ME_SDO        =>  ME_SDO,       -- in: HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC        =>  ME_DSC,       -- in: HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                    --                    synchronized by the recovered serial data stream.
    ME_VW         =>  ME_VW,        -- in: HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD         =>  ME_TD,        -- in: HD6408-output: take data is high during receipt of data after identification
                                    --                    of a sync pulse and two valid Manchester data bits
    Clk           =>  clk_i,
    Rd_Mil        =>  mil_rd_start,
    Mil_RCV_D     =>  Mil_RCV_D,
    Mil_In_Pos    =>  Mil_BOI,
    Mil_In_Neg    =>  Mil_BZI,
    Mil_Cmd       =>  mil_trm_cmd,
    Wr_Mil        =>  mil_trm_start,
    Mil_TRM_D     =>  mil_trm_data,
    EPLD_Manchester_Enc => manchester_fpga,
    Reset_6408    =>  Reset_6408,
    Mil_Trm_Rdy   =>  mil_trm_rdy,
    nSel_Mil_Drv  =>  nSel_Mil_Drv,
    nSel_Mil_Rcv  =>  nSel_Mil_Rcv,
    nMil_Out_Pos  =>  Mil_nBOO,
    nMil_Out_Neg  =>  Mil_nBZO,
    Mil_Cmd_Rcv   =>  Mil_Cmd_Rcv,
    Mil_Rcv_Rdy   =>  Mil_Rcv_Rdy,
    Mil_Rcv_Error =>  Mil_Rcv_Error,
    No_VW_Cnt     =>  no_vw_cnt,      -- Bit[15..8] Fehlerzaehler fuer No Valid Word des positiven Decoders "No_VW_p",
                                      -- Bit[7..0] Fehlerzaehler fuer No Valid Word des negativen Decoders "No_VM_n"
    Clr_No_VW_Cnt =>  clr_no_vw_cnt,  -- Loescht die no valid word Fehler-Zaehler des positiven und negativen Dekoders.
                                      -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    Not_Equal_Cnt =>  not_equal_cnt,  -- Bit[15..8] Fehlerzaehler fuer Data_not_equal,
                                      -- Bit[7..0] Fehlerzaehler fuer unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
    Clr_Not_Equal_Cnt =>  clr_not_equal_cnt,  -- Loescht die Fehlerzaehler fuer Data_not_equal und den Fehlerzaehler fuer unterschiedliche
                                              -- Komando-Daten-Kennung (CMD_not_equal).
                                              -- Muss synchron zur Clock 'Clk' und mindesten eine Periode lang aktiv sein!
    error_limit_reached =>  error_limit_reached,
    Mil_Decoder_Diag_p  =>  Mil_Decoder_Diag_p,
    Mil_Decoder_Diag_n  =>  Mil_Decoder_Diag_n
    );
    
Sel_Mil_Drv <= not nSel_Mil_Drv;
    

p_regs_acc: process (clk_i, nrst_i)
  begin
    if nrst_i = '0' then
      ex_stall        <= '1';
      ex_ack          <= '0';
      ex_err          <= '0';
      
      manchester_fpga <= '0';
      ev_filt_12_n8   <= '0';
      ev_filter_on    <= '0';
      debounce_on     <= '0';
      puls2_rahmen    <= '0';
      puls1_rahmen    <= '0';
      ev_reset_on     <= '0';
      clr_mil_rcv_err <= '0';
      
      mil_trm_start   <= '0';
      mil_rd_start    <= '0';
      mil_trm_cmd     <= '0';
      mil_trm_data <= (others => '0');
      
    elsif rising_edge(clk_i) then
      ex_stall        <= '1';
      ex_ack          <= '0';
      ex_err          <= '0';
      
      clr_no_VW_cnt   <= '0';
      clr_not_equal_cnt <= '0';

      if mil_trm_rdy = '0' and manchester_fpga = '0' then mil_trm_start <= '0';
      elsif manchester_fpga = '1' then  mil_trm_start <= '0'; end if;
      
      if mil_rcv_rdy = '0' and manchester_fpga = '0' then mil_rd_start <= '0';
      elsif manchester_fpga = '1' then  mil_rd_start <= '0'; end if;

      if slave_i.cyc = '1' and slave_i.stb = '1' and ex_stall = '1' then
      -- begin of wishbone cycle
        case unsigned(slave_i.adr(c_mil_addr_width-1 downto 2)) is
        -- check existing word register
          when mil_wr_cmd_a | mil_wr_data_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "0011" then
              -- write to low word is allowed
                if mil_trm_rdy = '1' and mil_trm_start = '0' then
                  mil_trm_start <= '1';
                  mil_trm_cmd   <= slave_i.adr(2);
                  mil_trm_data  <= slave_i.dat(15 downto 0);
                    ex_stall <= '0';
                    ex_ack <= '1';
                else
                -- write to mil not allowed, because mil transmit is active
                  ex_stall <= '0';
                  ex_err <= '1';
                end if;
              else
                -- write to high word or unaligned word is not allowed
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
              if Mil_Rcv_Rdy = '1' and mil_rd_start = '0' then
                mil_rd_start <= '1';
                slave_o.dat(15 downto 0) <= Mil_RCV_D;
                  ex_stall <= '0';
                  ex_ack <= '1';
              else
              -- read mil not allowed, because no mil_rcv_rdy
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            end if;

          when mil_wr_rd_status_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "0011" then
                manchester_fpga <= slave_i.dat(b_sel_fpga_n6408);
                ev_filt_12_n8   <= slave_i.dat(b_ev_filt_12_n8);
                ev_filter_on    <= slave_i.dat(b_ev_filter_on);
                debounce_on     <= slave_i.dat(b_debounce_on);
                puls2_rahmen    <= slave_i.dat(b_puls2_rahmen);
                puls1_rahmen    <= slave_i.dat(b_puls1_rahmen);
                ev_reset_on     <= slave_i.dat(b_ev_reset_on);
                clr_mil_rcv_err <= slave_i.dat(b_mil_rcv_err);
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- write to high word or unaligned word is not allowed
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
              slave_o.dat(15 downto 0) <= ( manchester_fpga & ev_filt_12_n8 & ev_filter_on & debounce_on  -- mil-status[15..12]
                                          & puls2_rahmen & puls1_rahmen & ev_reset_on & mil_rcv_err_ff    -- mil-status[11..8]
                                          & mil_trm_rdy & Mil_Cmd_Rcv & mil_rcv_rdy & '0'                 -- mil-status[7..4]
                                          & '0' & Data_Req_Intr & Data_Rdy_Intr & Interlock_Intr );       -- mil-status[3..0]
              ex_stall <= '0';
              ex_ack <= '1';
            end if;
              
          when rd_wr_no_vw_cnt_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "0011" then
                clr_no_vw_cnt <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- write to high word or unaligned word is not allowed
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
              slave_o.dat(15 downto 0) <= no_vw_cnt;
              ex_stall <= '0';
              ex_ack <= '1';
            end if;

          when rd_wr_not_eq_cnt_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "0011" then
                clr_not_equal_cnt <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- write to high word or unaligned word is not allowed
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
              slave_o.dat(15 downto 0) <= not_equal_cnt;
              ex_stall <= '0';
              ex_ack <= '1';
            end if;

          when test_echo_1_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "1111" then
                test_echo_1 <= slave_i.dat(31 downto 0);
                ex_stall <= '0';
                ex_ack <= '1';
              else
              -- double word write is not complete written
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
            -- read complete double word
              slave_o.dat(31 downto 0) <= test_echo_1;
              ex_stall <= '0';
              ex_ack <= '1';
            end if;

          when test_echo_2_a =>
            if slave_i.we = '1' then
              if slave_i.sel = "1111" then
                test_echo_2 <= slave_i.dat(31 downto 0);
                ex_stall <= '0';
                ex_ack <= '1';
              else
              -- double word write is not complete written
                ex_stall <= '0';
                ex_err <= '1';
              end if;
            else
            -- read complete double word
              slave_o.dat(31 downto 0) <= test_echo_2;
              ex_stall <= '0';
              ex_ack <= '1';
            end if;

          when test_event_a =>
            if slave_i.we = '1' then
              ex_stall <= '0';
              ex_err <= '1';
            else
            -- read complete double word
              slave_o.dat(15 downto 0) <= event_d;
              ex_stall <= '0';
              ex_ack <= '1';
            end if;

          when others =>
            ex_stall <= '0';
            ex_err <= '1';
        end case;
      end if;
    end if;
  end process p_regs_acc;
  

P_Mil_Rcv_Err:  process (clk_i, nRst_i)
  begin
    if nRst_i = '0' then
      Mil_Rcv_Err_ff <= '0';
    elsif rising_edge(clk_i) then
      if Mil_Rcv_Error = '1' then
        Mil_Rcv_Err_ff <= '1';
      elsif clr_mil_rcv_err = '1' then
        Mil_Rcv_Err_ff <= '0';
      end if;
    end if;
  end process P_Mil_Rcv_Err;
  

end arch_wb_mil_scu;