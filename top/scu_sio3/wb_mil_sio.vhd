LIBRARY ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;
use work.aux_functions_pkg.all;
use work.mil_pkg.all;
use work.wb_mil_scu_pkg.all;
use work.genram_pkg.all;  



--+---------------------------------------------------------------------------------------------------------------------------------+
--| "wb_mil_scu" stellt in Verbindung mit der SCU-Aufsteck-Karte "FG900170_SCU_MIL1" alle Funktionen bereit, die benoetigt werden,  |
--| um SE-Funktionalitaet mit einer SCU realistieren zu koennen.                                                                    |
--|                                                                                                                                 |
--| Das Rechnerinterface zu den einzelnen SE-Funktionen ist mit dem wishbone bus realisiert.                                        |
--| Je nach angesprochener Funktion ist ein 32 Bit oder 16 Bit Zugriff vorgeschrieben.                                              |
--| Egal ob 32 Bit oder 16 Bit Resource, die Adressen muessen immer auf Modulo-4-Grenzen liegen.                                    |
--| Die Adressoffsets der einzelnen Funktionen sind in der Datei "wb_mil_scu_pkg" abgelegt. Ebenso ist dort die                     |
--| Component-Deklaration der Entity "wb_mil_scu" abgelegt. Dort ist auch der SDB-Record der wishbone-componente abgelegt.          |
--|                                                                                                                                 |
--| Folgende Funktionen sollen zum Abschluss des Projekts bereitstehen:                                                             |
--|   1) Mil-(Device)-Bus-Kommunikation. Wird als erstes realisiert.                                                                |
--|   2) Timing-Receiver (Manchester-Dekoder im FPGA)                                                                               |
--|   3) Event-Filter zur Steuerung:                                                                                                |
--|       a) welches Event in das Empfangs-Fifo geschreiben wird.                                                                   |
--|       b) welches Event den Event-Timer rueckstzen soll.                                                                         |
--|       c) ob ein Event einen Puls (oder zwei Events einen Rahmenpuls) an den beiden Lemoausgangsbuchsen bereitstellen soll.      |
--|   4) Delay-Timer, generiert einen Interrupt nach herunterzaehlen des geladenen Datums.                                          |
--|   5) Event-Timer, 32 Bit breit, kann per Event oder Software auf Null zurueckgestzt werden. Hat keinen Ueberlaufschutz.         |
--|   6) Wait-Timer, 24 Bit breit, kann per Software auf null gestzt werden. Hat keinen Ueberlaufschutz.                            |
--|   7) Interrupt-System.                                                                                                          |
--|   8) Test und Auslese ob die SCU-Aufsteck-Karte "FG900170_SCU_MIL1" bestueckt ist.                                              |
--|                                                                                                                                 |
--| Version | Autor       | Datum       | Grund                                                                                     |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   01    | W.Panschow  | 15.08.2013  | Bereitstellung der Mil-(Device)-Bus Funktion.                                             |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   02    | W.Panschow  | 14.11.2013  | Timing-Receiver, Event-Filter, Delay-Timer, Event-Timer und Wait-Timer implementiert.     |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   03    | W.Panschow  | 22.11.2013  | a) Es sind nur noch 32 Bit-Zugriffe auf alle Resourcen der wb_mil_scu erlaubt.            |
--|         |             |             | b) Die Led Trm-12-Spannung-okay ist angeschlossen. Die Led wird doppelt genutzt.          |
--|         |             |             |    Bei Fehlerhaften Zugriffen auf "wb_mil_scu"-Resourcen (z.B. kein 32Bit-Zugriff, oder   |
--|         |             |             |    Event-Fifo-auslesen, obwohl kein Event im Fifo steht) wird Trm-12 fuer kurze Zeit      |
--|         |             |             |    dunkel getastet.                                                                       |
--|         |             |             | c) Die User-1 Led signalisiert, dass das Event-Fifo nicht leer ist.                       |
--|         |             |             | d) Die Entprellung der Device-Bus-Interrupts ist implementiert. Diese Funktion ist immer  |
--|         |             |             |    einschaltet. Im Statusregister wird die Funktion immer als eingeschaltet signalisiert. |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   04    | W.Panschow  | 28.03.2014  | a) Interrupt-Ausgaenge zum Anschluss an die Mil-Interrupt-Instanz "mil_irq_inst"          |
--|         |             |             |    eingebaut (Interlock_Intr_o, Data_Rdy_Intr_o,  Data_Req_Intr_o, dly_intr_o,            |
--|         |             |             |    ev_fifo_ne_intr_o).                                                                    |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   05    | W.Panschow  | 22.04.2014  | a) evyer_10ms_intr_o changed to every_ms_intr_o.                                          |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   06    | K.Kaiser    | 25.02.2015  | a) generic map: Berechnung fuer every_ms, every_us  korrigiert                            |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   07    | K.Kaiser    | 05.05.2015  |    Bis zu 4 LEMO Buchsen (SIO) nun über Register auslesbar und kontrollierbar             |
--|         |             |             |    Die vorherige Funktion (Steuerung beliebiger LEMO Ausgänge per Event) bleibt erhalten  |
--|         |             |             |    wenn das entsprechende "event_cntrl" Statusbit gesetzt ist.                            |
--|         |             |             |    Ist das entsprechende "event_cntrl" Statusbit gleich '0'(=default), dann sind LEMOs    |
--|         |             |             |    a1) nach Reset als Inputs geschaltet.Dise sind (transparent)per Input Reg abfragbar    |
--|         |             |             |    a2) wird Lemo Out Enable Reg.Bit auf 1 gesetzt,dann werden das entsprechen Reg-Bit     |
--|         |             |             |        auf den entsprechenden LEMO Ausgang übertragen.                                    |
--|         |             |             |    Jeder Lemo Buchse sind eigene Bits zugeordnet, sie sind somit einzeln ansteuerbar.     |      
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   08    | K.Kaiser    | 10.07.2017  |    tx_fifo (1024x17) in Senderichtung puffert nun Sendedaten, damit nicht auf             |
--|         |             |             |    mil_trm_rdy gewartet werden muss. Wirkt performancesteigernd DevBus FG Param-transfers |
--|         |             |             |    p_regs_acc wird hierfür geändert.                                                      |
--|         |             |             |    Todo:Interrupts aus Standardisierungsgründen auf SIO3 Topebene umsortieren:            | 
--|         |             |             |    every_ms_intr_o Pos 1-->Pos7                                                           |
--|         |             |             |    clk_switch_intr Pos14-->Pos1                                                           |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   09    | K.Kaiser    | 25.7.2017   |    8 TX Register für Read Tasks und 8 korrespondierende RX Register hinzu                 |
--|         |             |             |    Diese werden von einem Scheduler in Round-Robin first-come-first-serve bedient         |
--|         |             |             |    Status avail Bits zeigen zunächst mit 0x00 "alle Taskregister leer" an.                 |
--|         |             |             |    Wird ein Task Register mit einem Funktionscode beschrieben, sendet es einen Request    |
--|         |             |             |    an den Scheduler. Dieser transferiert den Funktionscode, sobald der DevBus frei ist,   |
--|         |             |             |    an das TX Register und erwartet die Antwort des DevBus Slaves.                         |
--|         |             |             |    Wird das Datum richtig empfangen, wird kein Error Bit gesetzt.                         |
--|         |             |             |    Kommt es zum Parityfehler oder Timeout, wird ein Error Bit (Bit 17 Datenreg) gesetzt   |
--|         |             |             |    Aufgrund des Errorbits wird beim Auslesen des Datenregisters kein DTACK gegeben.       |
--|         |             |             |    Wurden Tasks durch Empfangen bzw Timeout beendet, wird das availbit zurückgenommen.     |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |


ENTITY wb_mil_sio IS

generic (
    Clk_in_Hz:  INTEGER := 125_000_000;   -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    sio_mil_first_reg_a:    unsigned(15 downto 0)  := x"0400";
    sio_mil_last_reg_a:     unsigned(15 downto 0)  := x"0440";
    evt_filt_first_a:       unsigned(15 downto 0)  := x"1000";
    evt_filt_last_a:        unsigned(15 downto 0)  := x"1FFF"
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
    dly_intr_o:     out     std_logic;
    nLed_Fifo_ne:   out     std_logic;
    ev_fifo_ne_intr_o:  out   std_logic;
    Interlock_Intr_i: in      std_logic;
    Data_Rdy_Intr_i:  in      std_logic;
    Data_Req_Intr_i:  in      std_logic;
    Interlock_Intr_o: out     std_logic;
    Data_Rdy_Intr_o:  out     std_logic;
    Data_Req_Intr_o:  out     std_logic;
    nLed_Interl:    out     std_logic;
    nLed_Dry:       out     std_logic;
    nLed_Drq:       out     std_logic;
    every_ms_intr_o:  out std_logic;
    lemo_data_o:    out     std_logic_vector(4 downto 1);
    lemo_nled_o:    out     std_logic_vector(4 downto 1);
    lemo_out_en_o:  out     std_logic_vector(4 downto 1);  
    lemo_data_i:    in      std_logic_vector(4 downto 1):= (others => '0');
    nsig_wb_err:    out     std_logic       -- '0' => gestretchte wishbone access Fehlermeldung
    );
end wb_mil_sio;


ARCHITECTURE arch_wb_mil_sio OF wb_mil_sio IS 

constant mil_rd_wr_data_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + mil_rd_wr_data_a;   --not used  for reads anymore, use task registers therefore.
                                                                                                        --this address writes data to tx_fifo for block transfers
constant mil_wr_cmd_a_map:          unsigned (15 downto 0) := sio_mil_first_reg_a + mil_wr_cmd_a;       --this address writes cmds to tx_fifo for block transfers
constant mil_wr_rd_status_a_map:    unsigned (15 downto 0) := sio_mil_first_reg_a + mil_wr_rd_status_a;
constant rd_clr_no_vw_cnt_a_map:    unsigned (15 downto 0) := sio_mil_first_reg_a + rd_clr_no_vw_cnt_a;
constant rd_wr_not_eq_cnt_a_map:    unsigned (15 downto 0) := sio_mil_first_reg_a + rd_wr_not_eq_cnt_a;
constant rd_clr_ev_fifo_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + rd_clr_ev_fifo_a;
constant rd_clr_ev_timer_a_map:     unsigned (15 downto 0) := sio_mil_first_reg_a + rd_clr_ev_timer_a ;
constant rd_wr_dly_timer_a_map:     unsigned (15 downto 0) := sio_mil_first_reg_a + rd_wr_dly_timer_a;
constant rd_clr_wait_timer_a_map:   unsigned (15 downto 0) := sio_mil_first_reg_a + rd_clr_wait_timer_a;
constant mil_wr_rd_lemo_conf_a_map: unsigned (15 downto 0) := sio_mil_first_reg_a + mil_wr_rd_lemo_conf_a;
constant mil_wr_rd_lemo_dat_a_map:  unsigned (15 downto 0) := sio_mil_first_reg_a + mil_wr_rd_lemo_dat_a;
constant mil_rd_lemo_inp_a_map:     unsigned (15 downto 0) := sio_mil_first_reg_a + mil_rd_lemo_inp_a;

-- kk start
constant wr_tx_taskreg0_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg0_a; --to write tx task function codes,   bit[31..16] don't care 
constant wr_tx_taskreg1_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg1_a;
constant wr_tx_taskreg2_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg2_a;
constant wr_tx_taskreg3_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg3_a;
constant wr_tx_taskreg4_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg4_a;
constant wr_tx_taskreg5_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg5_a;
constant wr_tx_taskreg6_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg6_a;
constant wr_tx_taskreg7_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + wr_tx_taskreg7_a;

constant rd_rx_taskreg0_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg0_a; --to read corresponding rx task word, bit[31..16] don't care
constant rd_rx_taskreg1_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg1_a;
constant rd_rx_taskreg2_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg2_a;
constant rd_rx_taskreg3_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg3_a;
constant rd_rx_taskreg4_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg4_a;
constant rd_rx_taskreg5_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg5_a;
constant rd_rx_taskreg6_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg6_a;
constant rd_rx_taskreg7_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + rd_rx_taskreg7_a;


constant rd_status_avail_a_map:      unsigned (15 downto 0) := sio_mil_first_reg_a + rd_status_avail_a; --to read corresponding avail bits, bit[31..8] don't care
-- kk end
signal    manchester_fpga:  std_logic;  -- '1' => fpga manchester endecoder selected, '0' => external hardware manchester endecoder 6408 selected.
signal    ev_filt_12_8b:    std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal    ev_filt_on:       std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal    debounce_on:      std_logic;  -- '1' => debounce of device bus interrupt input is on.
signal    puls2_frame:      std_logic;  -- '1' => aus zwei events wird der Rahmenpuls2 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal    puls1_frame:      std_logic;  -- '1' => aus zwei events wird der Rahmenpuls1 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal    ev_reset_on:      std_logic;  -- '1' => events koennen den event timer auf Null setzen, vorausgesetzt das Eventfilter ist richtig programmiert.
signal    clr_mil_rcv_err:  std_logic;

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

signal    mil_trm_start:    std_logic;
signal    mil_trm_cmd:      std_logic;
signal    mil_trm_data:     std_logic_vector(15 downto 0);
  
signal    mil_rd_start:     std_logic;
  
signal    sw_clr_ev_timer:  std_logic;
signal    ev_clr_ev_timer:  std_logic;
signal    ev_timer:         unsigned(31 downto 0);
  
signal    ena_led_count:    std_logic;
  
signal    nSel_Mil_Drv:     std_logic;
  
signal    wr_filt_ram:      std_logic;
signal    rd_filt_ram:      std_logic;
signal    stall_filter:     std_logic;
signal    filt_data_i:      std_logic_vector(5 downto 0);
signal    ev_fifo_ne:       std_logic;
signal    ev_fifo_full:     std_logic;
signal    rd_ev_fifo:       std_logic;
signal    clr_ev_fifo:      std_logic;

signal    dly_timer:        unsigned(24 downto 0);
signal    ld_dly_timer:     std_logic;
signal    stall_dly_timer:  std_logic;

signal    wait_timer:       unsigned(23 downto 0);
signal    clr_wait_timer:   std_logic;
  
signal    ep_read_port:     std_logic_vector(15 downto 0);    --event processing read port
  
signal    timing_received:  std_logic;

signal    ena_every_us:     std_logic;

signal    db_interlock_intr:  std_logic;
signal    db_data_rdy_intr: std_logic;
signal    db_data_req_intr: std_logic;

signal    dly_intr:         std_logic;
signal    every_ms:         std_logic;

signal    lemo_inp:         std_logic_vector (4 downto 1);
signal    lemo_i_reg:       std_logic_vector (4 downto 1);
signal    lemo_dat:         std_logic_vector (4 downto 1);
signal    lemo_out_en:      std_logic_vector (4 downto 1);
signal    lemo_event_en:    std_logic_vector (4 downto 1);

signal    io_1:             std_logic;
signal    io_2:             std_logic;

-----------------------------------------------------------
signal    tx_fifo_write_en: std_logic;
signal    tx_fifo_wr_pulse: std_logic;
signal    tx_fifo_data_in:  std_logic_vector (16 downto 0);

signal    tx_fifo_read_en:  std_logic;
signal    tx_fifo_data_out: std_logic_vector (16 downto 0);

signal    tx_fifo_empty:    std_logic;
signal    tx_fifo_full:     std_logic;
-----------------------------------------------------------
signal    tx_taskreg0:      std_logic_vector (16 downto 0);
signal    tx_taskreg1:      std_logic_vector (16 downto 0);   --bit 16 is the request bit, bit 15..0 are the bits for Function Code and DevBus Slave Address
signal    tx_taskreg2:      std_logic_vector (16 downto 0);
signal    tx_taskreg3:      std_logic_vector (16 downto 0);
signal    tx_taskreg4:      std_logic_vector (16 downto 0);
signal    tx_taskreg5:      std_logic_vector (16 downto 0);
signal    tx_taskreg6:      std_logic_vector (16 downto 0);
signal    tx_taskreg7:      std_logic_vector (16 downto 0);

signal    rx_taskreg0:      std_logic_vector (16 downto 0);
signal    rx_taskreg1:      std_logic_vector (16 downto 0);   --bit 16 is the error bit, bit 15..0 are the bits for the received DevBus data
signal    rx_taskreg2:      std_logic_vector (16 downto 0);
signal    rx_taskreg3:      std_logic_vector (16 downto 0);
signal    rx_taskreg4:      std_logic_vector (16 downto 0);
signal    rx_taskreg5:      std_logic_vector (16 downto 0);
signal    rx_taskreg6:      std_logic_vector (16 downto 0);
signal    rx_taskreg7:      std_logic_vector (16 downto 0);

signal    timeslot:         std_logic_vector (8 downto 0);
signal    set_avail_ps:     std_logic_vector (7 downto 0);
signal    clr_avail_ps:     std_logic_vector (7 downto 0);
signal    taskreg_ack:      std_logic_vector (7 downto 0);
signal    avail:            std_logic_vector (7 downto 0);

constant  timeout_cntr_max: integer := 70;  --max timeout 50 µs: TX Telegram + RX Telegram + 10µs Gap
signal    timeout_cntr:     integer :=  0;
signal    timeout_cntr_en:  std_logic;
signal    timeout_cntr_clr: std_logic;
signal    slave_i_stb_dly:  std_logic;
signal    mil_trm_start_dly:std_logic;
signal    task_runs:        std_logic;

signal    mil_rd_start_latched:        std_logic; 
signal    mil_rd_start_dly:            std_logic; 
signal    mil_rd_start_dly2:           std_logic; 
      

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
    res       => '0',
    clk       => clk_i,
    ena       => open,            -- das untersetzende enable muss in der gleichen Clockdomaene erzeugt werden.
                                  -- Das enable sollte nur ein Takt lang sein.
                                  -- Z.B. koennte eine weitere div_n-Instanz dieses Signal erzeugen.  
    div_o     => ena_led_count    -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );


every_1ms_inst: div_n
  generic map (
    n         => 1000, -- kk              -- Vorgabe der Taktuntersetzung. 1ms = 0.001 = 1/1000 * ena_every_us (1000)
    diag_on   => 0                        -- diag_on = 1 die Breite des Untersetzungzaehlers
                                          -- mit assert .. note ausgegeben.
    )

  port map (
    res       => '0',
    clk       => clk_i,
    ena       => ena_every_us,    -- das untersetzende enable muss in der gleichen Clockdomaene erzeugt werden.
                                  -- Das enable sollte nur ein Takt lang sein.
                                  -- Z.B. koennte eine weitere div_n-Instanz dieses Signal erzeugen.  
    div_o     => every_ms         -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );

every_ms_intr_o <= every_ms;




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
    nLED        => nLed_Mil_Rcv,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
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
    nLED        => nLed_Mil_Trm,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
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
    nLED        => nLed_Mil_Err,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
    );

led_interl: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,   -- if you use ena for a reduction, signal should be generated from the same 
                                     -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => db_interlock_intr,-- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                     -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => nLed_Interl,      -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open     
    );

led_dry: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => db_data_rdy_intr,-- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => nLed_dry,        -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
    );

led_drq: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => db_data_req_intr,-- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
    nLED        => nLed_drq,        -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open                                               
                                   
    );

led_timing: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => timing_received, -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => nLed_Timing,     --- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
    );

	 
lemo_nled_i: for i in 1 to 4 generate

lemo_nled_o_x: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => lemo_data_i(i),  -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => open,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => lemo_nled_o(i)   -- open drain output, active low, inactive tristate.
    );
end generate;



sig_wb_err: led_n
  generic map (
    stretch_cnt => 6
    )
  port map (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => ex_err,          -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => nsig_wb_err,     -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => open             -- open drain output, active low, inactive tristate.
    );

p_deb_intl: debounce
  generic map (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                            -- "DB_Out" diesem Pegel folgt.     
    )
  port map (
    DB_In   => Interlock_Intr_i,  -- Das zu entprellende Signal
    Reset   => not nRst_i,        -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_interlock_intr  -- Das entprellte Signal von "DB_In".
    );
    
Interlock_Intr_o <= db_interlock_intr;


p_deb_drdy: debounce
  generic map (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                            -- "DB_Out" diesem Pegel folgt.     
    )
  port map (
    DB_In   => Data_Rdy_Intr_i,   -- Das zu entprellende Signal
    Reset   => not nRst_i,        -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_data_rdy_intr   -- Das entprellte Signal von "DB_In"
    );

Data_Rdy_Intr_o <= db_data_rdy_intr;


p_deb_dreq: debounce
  generic map (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                            -- "DB_Out" diesem Pegel folgt.     
    )
  port map (
    DB_In   => Data_Req_Intr_i,   -- Das zu entprellende Signal
    Reset   => not nRst_i,        -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_data_req_intr   -- Das entprellte Signal von "DB_In".
    );
    
Data_Req_Intr_o <= db_data_req_intr;

p_deb_lemo_i: for i in 1 to 4 generate
p_deb_lemo_x: debounce
  generic map (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, DB_Cnt gibt die Zahl der Takte vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss,damit "DB_Out" diesem Pegel folgt.     
    )
  port map (
    DB_In   => lemo_data_i(i),              -- Das zu entprellende Signal
    Reset   => not nRst_i,                  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => lemo_inp(i)                  -- Das entprellte Signal von "DB_In".
    );
end generate;

	 



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
    Rd_Mil        =>  mil_rd_start_latched,
    Mil_RCV_D     =>  Mil_RCV_D,
    Mil_In_Pos    =>  Mil_BOI,
    Mil_In_Neg    =>  Mil_BZI,
    Mil_Cmd       =>  mil_trm_cmd,
    Wr_Mil        =>  mil_trm_start_dly,
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
    Mil_Rcv_Err   =>  Mil_Rcv_Error,
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
    Mil_Decoder_Diag_n  =>  Mil_Decoder_Diag_n,
    clr_mil_rcv_err     =>  clr_mil_rcv_err
    );
    
Sel_Mil_Drv <= not nSel_Mil_Drv;


event_processing_1: event_processing
  generic map (
    clk_in_hz         =>    Clk_in_Hz     -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    )
  port map (
    ev_filt_12_8b     => ev_filt_12_8b,
    ev_filt_on        => ev_filt_on,
    ev_reset_on       => ev_reset_on,
    puls1_frame       => puls1_frame,
    puls2_frame       => puls2_frame,
    timing_i          => timing,
    clk_i             => clk_i,
    nRst_i            => nRst_i,
    wr_filt_ram       => wr_filt_ram,
    rd_filt_ram       => rd_filt_ram,
    rd_ev_fifo        => rd_ev_fifo,
    clr_ev_fifo       => clr_ev_fifo,
    filt_addr         => slave_i.adr(11+2 downto 2),
    filt_data_i       => slave_i.dat(filter_data_width-1 downto 0),
    stall_o           => stall_filter,
    read_port_o       => ep_read_port,
    ev_fifo_ne        => ev_fifo_ne,
    ev_fifo_full      => ev_fifo_full,
    ev_timer_res      => ev_clr_ev_timer,
    ev_puls1          => io_1,
    ev_puls2          => io_2,
    timing_received   => timing_received
  );

ev_fifo_ne_intr_o <= ev_fifo_ne;


led_fifo_ne: led_n
  generic map (
    stretch_cnt => 4
    )
  port map (
    ena         => ena_led_count,   -- if you use ena for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => ev_fifo_ne,      -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => nLed_Fifo_ne,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => open
    );

------------------------------------------------------------------------------------------------------------------------------
tx_fifo : generic_sync_fifo        
  GENERIC MAP
  (
    g_data_width => 17, 
    g_size       => 1024
  )
    PORT MAP
    (
      clk_i   => clk_i, 
      rst_n_i => nrst_i, 
      we_i    => tx_fifo_write_en, 
      d_i     => tx_fifo_data_in, 
      rd_i    => tx_fifo_read_en, 
      q_o     => tx_fifo_data_out, 
      empty_o => tx_fifo_empty, 
      full_o  => tx_fifo_full
);

------------------------------------------------------------------------------------------------------------------------------  
-- common code section for tx and rx control   

-- mil_trm_data     <= tx_fifo_data_out(15 downto 0);  
-- mil_trm_cmd      <= tx_fifo_data_out(16);                    -- is only evaluated on mil_trm_start and must be stable with mil_trm_start_dly
-- tx_fifo_read_en  <= mil_trm_start and not mil_trm_start_dly; -- need only one pulse at each fifo pop  (wr_mil is triggered with mil_trm_start_dly)
-- 
-- 
-- tx_fifo_ctrl:process (clk_i, nrst_i)
-- begin
--   if nrst_i = '0' then
-- 
-- 
--     mil_trm_start     <= '0';
--     mil_trm_start_dly <= '0';
--   
--     
--   elsif rising_edge (clk_i) then 
--   
--    --from old code to be sure that harris trm_start is quiet on not ready condition
--     if mil_trm_rdy = '0' and manchester_fpga = '0' then mil_trm_start <= '0';          -- harris active,         give harris trm_start only when trm_rdy
--     elsif                    manchester_fpga = '1' then mil_trm_start <= '0'; end if;  -- internal coder active, give harris trm_start never
--   
-- 
--     if tx_fifo_empty = '0' and  mil_trm_rdy = '1' then                
--       mil_trm_start   <= '1'; --start tx as long as tx_fifo not empty and mil transmitter is ready
--     else 
--       mil_trm_start   <= '0';   
--     end if;
--     
--   end if;
--   
-- end process tx_fifo_ctrl;

--KK New scheduler added for TX_FIFO and Task Register control
---------------------------------------------------------------------------------------------------



tx_fifo_read_en  <= mil_trm_start and not mil_trm_start_dly; -- need only one pulse at each fifo pop  (wr_mil is triggered with mil_trm_start_dly

someclockedlogic_p : PROCESS (clk_i, nrst_i)
BEGIN
  IF nrst_i = '0' THEN
    timeout_cntr         <=  0 ;
    slave_i_stb_dly      <= '0';
    mil_trm_start_dly    <= '0';
    mil_rd_start_latched <= '0';
    mil_rd_start_dly     <= '0';
    mil_rd_start_dly2    <= '0';
    
  ELSIF rising_edge (clk_i) THEN
    IF timeout_cntr_clr= '1' then
      timeout_cntr    <= 0;                                              
    ELSIF timeout_cntr_en = '1' AND ena_every_us='1' THEN
      timeout_cntr    <= timeout_cntr + 1;
    ELSE
      NULL;
    END IF; 
    -- min  1 dsc pulse make hd6408 happy for reset of mil_rcv_rdy
    if mil_rd_start='1' then
       mil_rd_start_latched  <= mil_rd_start;
    elsif mil_rd_start_dly='1' and mil_rd_start_dly2='1' then
       mil_rd_start_latched <= '0';
    end if;
       
    if ena_every_us='1' then
       mil_rd_start_dly <= mil_rd_start_latched;
       mil_rd_start_dly2 <= mil_rd_start_dly;
    end if; 
    
    slave_i_stb_dly   <= slave_i.stb;
    mil_trm_start_dly <= mil_trm_start;
  END IF;
END PROCESS someclockedlogic_p;

schedule_mux : PROCESS (tx_taskreg0, tx_taskreg1, tx_taskreg2, tx_taskreg3, tx_taskreg4, tx_taskreg5, 
                        tx_taskreg6, tx_taskreg7, timeslot, tx_fifo_data_out)
BEGIN
  IF timeslot = "000000001" THEN
    mil_trm_data <= tx_taskreg0(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "000000010" THEN
    mil_trm_data <= tx_taskreg1(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "000000100" THEN
    mil_trm_data <= tx_taskreg2(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "000001000" THEN
    mil_trm_data <= tx_taskreg3(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "000010000" THEN
    mil_trm_data <= tx_taskreg4(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "000100000" THEN
    mil_trm_data <= tx_taskreg5(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "001000000" THEN
    mil_trm_data <= tx_taskreg6(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "010000000" THEN
    mil_trm_data <= tx_taskreg7(15 downto 0); 
    mil_trm_cmd  <= '1';
  ELSIF timeslot = "100000000" THEN
    mil_trm_data <= tx_fifo_data_out(15 DOWNTO 0); 
    mil_trm_cmd  <= tx_fifo_data_out(16);
  ELSE
    mil_trm_data <= x"baba"; 
    mil_trm_cmd  <= '0';
  END IF;
END PROCESS schedule_mux;


schedule_p : PROCESS (clk_i, nrst_i)
BEGIN
  IF nrst_i = '0' THEN
    timeslot      <= "100000000";                                  --1-Hot Queue, start with fifo due to timeout cntr set
    mil_trm_start <= '0';
    taskreg_ack   <= (OTHERS => '0');
    set_avail_ps  <= (OTHERS => '0');
    task_runs     <= '0';
    rx_taskreg0   <= (others => '0');
    rx_taskreg1   <= (others => '0');
    rx_taskreg2   <= (others => '0');
    rx_taskreg3   <= (others => '0');
    rx_taskreg4   <= (others => '0');
    rx_taskreg5   <= (others => '0');
    rx_taskreg6   <= (others => '0');
    rx_taskreg7   <= (others => '0');
    timeout_cntr_en <= '0'; 
    timeout_cntr_clr<= '0';
    mil_rd_start   <= '0';
  ELSIF rising_edge (clk_i) THEN
    mil_trm_start <= '0';
    mil_rd_start  <= '0';
    taskreg_ack   <= (OTHERS => '0');
    set_avail_ps  <= (OTHERS => '0');                            -- only for one pulse
    timeout_cntr_clr<= '0';
          
  IF timeslot(0) = '1' then
    IF tx_taskreg0(16) = '1'  or task_runs = '1'  THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(0)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task 
          task_runs       <= '0';
          set_avail_ps(0) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg0 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg0 <= '1' & x"beef";                         --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg0 <= '1' & x"babe";
          ELSE
            rx_taskreg0 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
 
 
   ELSIF timeslot(1) = '1' then
    IF tx_taskreg1(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(1)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50   or Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(1) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg1 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg1 <= '1' & x"beef";  				--rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg1 <= '1' & x"babe";
          ELSE
            rx_taskreg1 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(2) = '1' then
    IF tx_taskreg2(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(2)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(2) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg2 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg2 <= '1' & x"beef";                    --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg2 <= '1' & x"babe";
          ELSE
            rx_taskreg2 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL;                                                                                                                                                                          
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(3) = '1' then
    IF tx_taskreg3(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(3)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(3) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg3 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg3 <= '1' & x"beef";   --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg3 <= '1' & x"babe";
          ELSE
            rx_taskreg3 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(4) = '1' then
    IF tx_taskreg4(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(4)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(4) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg4 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg4 <= '1' & x"beef";                    --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg4 <= '1' & x"babe";
          ELSE
            rx_taskreg4 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(5) = '1' then
    IF tx_taskreg5(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(5)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(5) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg5 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg5 <= '1' & x"beef";                    --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg5 <= '1' & x"babe";
          ELSE
            rx_taskreg5 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(6) = '1' then
    IF tx_taskreg6(16) = '1'     or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(6)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(6) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg6 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg6 <= '1' & x"beef";                    --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg6 <= '1' & x"babe";
          ELSE
            rx_taskreg6 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 
   ELSIF timeslot(7) = '1' then
    IF tx_taskreg7(16) = '1'   or task_runs = '1' THEN                              --check for a task
      IF mil_trm_rdy = '1' AND task_runs = '0' THEN              --mil_trm should be ready when entering
        taskreg_ack(7)    <= '1';                                --pulse for acknowledge data
        mil_trm_start     <= '1';                                --pulse for tx start and timeout_cntr
        timeout_cntr_en   <= '1';
        task_runs         <= '1';
      ELSIF task_runs = '1' THEN
        IF timeout_cntr = 50 OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 10 for gap
          timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8); --rotate by 1 to jump to next task reg
          task_runs       <= '0';
          set_avail_ps(7) <= '1';                                --todo maybe wait until slave_i.stb=0
          timeout_cntr_en <= '0';                                --stop and reset timeout_cntr for next use
          timeout_cntr_clr<= '1';
          IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN        --prepare data output
            rx_taskreg7 <= '0' & MIL_RCV_D;
            mil_rd_start<= '1';
          ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN
            rx_taskreg7 <= '1' & x"beef";                    --rx_taskreg0(16)=1 causes intented DTACK Error
            mil_rd_start<= '1';
          ELSIF timeout_cntr = timeout_cntr_max THEN
            rx_taskreg7 <= '1' & x"babe";
          ELSE
            rx_taskreg7 <= '1' & x"dead";
          END IF;--MIL_Rcv_Rdy
        ELSE 
          NULL;                                                  -- wait for timeout or rx data
        END IF;--timeout_cntr
      ELSIF task_runs = '0' THEN                                 --Wait for MIL_RCV_RDY 
        NULL; 
      ELSE
        NULL;
      END IF; --mil_trm_rdy
    ELSE 
      timeslot        <= timeslot(7 DOWNTO 0) & timeslot(8);     --skip if there is nothing to do and rotate to next task  
    END IF;--tx_taskreg
    
 

  ELSIF timeslot(8) = '1' THEN     --tx fifo in timeslot 8
   
	
    IF  tx_fifo_empty='1'  and task_runs='0'  THEN
	 
        timeslot         <= timeslot(7 DOWNTO 0) & timeslot(8);    --skip if there is nothing to do and rotate to next task
        timeout_cntr_en  <= '0';
        timeout_cntr_clr <= '1';
        task_runs        <= '0';
    ELSIF tx_fifo_empty='0'  and  mil_trm_rdy = '1'  THEN                           -- pop fifo
        mil_trm_start    <= '1';   
        task_runs        <= '1';
        timeout_cntr_en  <= '1';                                    
        timeout_cntr_clr <= '1';                                              
	 ELSIF  tx_fifo_empty='1' and  mil_trm_rdy = '1' and timeout_cntr=22  THEN         -- wait for last telegram before exit
	     mil_trm_start    <= '0';   
        task_runs        <= '0';
        timeout_cntr_en  <= '0';                                    
        timeout_cntr_clr <= '1';

	 ELSE
	     NULL;
	 END IF;
 


  END IF; --timeslot
  
END IF;--clocked process

end process schedule_p;





--KK Added Regs tx_taskreg0..7, rx_taskreg0..7, status_avail reg performance improvement
-----------------------------------------------------------------------------------------------------
    
p_regs_acc: process (clk_i, nrst_i)
variable LA_a_var : unsigned (17 downto 2);          
  begin
    LA_a_var := unsigned(slave_i.adr(17 downto 2));  -- LA_a_var is evaluated on each clk edge 
                                                   
    if nrst_i = '0' then
      ex_stall        <= '1';
      ex_ack          <= '0';
      ex_err          <= '0';
      
      manchester_fpga <= '0';
      ev_filt_12_8b   <= '0';
      ev_filt_on      <= '0';
      debounce_on     <= '1';
      puls2_frame     <= '0';
      puls1_frame     <= '0';
      ev_reset_on     <= '0';
      clr_mil_rcv_err <= '1';
      
      --mil_trm_start   <= '0';
      --mil_rd_start    <= '0';
      --mil_trm_cmd     <= '0';
      --mil_trm_data <= (others => '0');

      rd_ev_fifo      <= '0';
      clr_ev_fifo     <= '1';
      wr_filt_ram     <= '0';
      rd_filt_ram     <= '0';
      sw_clr_ev_timer <= '1';
      ld_dly_timer    <= '0';
      clr_wait_timer  <= '1';
      lemo_out_en     <= (others => '0');
      lemo_dat        <= (others => '0');
      lemo_i_reg      <= (others => '0');

      tx_taskreg0     <= (others => '0');
      tx_taskreg1     <= (others => '0');
      tx_taskreg2     <= (others => '0');
      tx_taskreg3     <= (others => '0');
      tx_taskreg4     <= (others => '0');
      tx_taskreg5     <= (others => '0');
      tx_taskreg6     <= (others => '0');
      tx_taskreg7     <= (others => '0');


      clr_avail_ps    <= (others => '0'); 
      
    elsif rising_edge(clk_i) then
      lemo_i_reg        <= lemo_inp;
      ex_stall          <= '1';
      ex_ack            <= '0';
      ex_err            <= '0';

      rd_ev_fifo        <= '0';
      clr_ev_fifo       <= '0';
      wr_filt_ram       <= '0';
      rd_filt_ram       <= '0';

      clr_no_VW_cnt     <= '0';
      clr_not_equal_cnt <= '0';
      sw_clr_ev_timer   <= '0';
      ld_dly_timer      <= '0';
      clr_wait_timer    <= '0';
      tx_fifo_write_en  <= '0';
      slave_o.dat       <= (others => '0');

      if taskreg_ack(0)='1'  then tx_taskreg0(16) <= '0'; end if;
      if taskreg_ack(1)='1'  then tx_taskreg1(16) <= '0'; end if;
      if taskreg_ack(2)='1'  then tx_taskreg2(16) <= '0'; end if;
      if taskreg_ack(3)='1'  then tx_taskreg3(16) <= '0'; end if;
      if taskreg_ack(4)='1'  then tx_taskreg4(16) <= '0'; end if;
      if taskreg_ack(5)='1'  then tx_taskreg5(16) <= '0'; end if;
      if taskreg_ack(6)='1'  then tx_taskreg6(16) <= '0'; end if;
      if taskreg_ack(7)='1'  then tx_taskreg7(16) <= '0'; end if;
   
      -- avail pulse are set on received DevBus Data (faulty and o.k. ones) and cleared on RX Register access
      -- set_avail_ps must not change during register access
      if clr_avail_ps(0)='1'and set_avail_ps(0)='0' then avail(0)<='0'; elsif set_avail_ps(0)='1' then avail(0)<='1'; else null;end if;
      if clr_avail_ps(1)='1'and set_avail_ps(1)='0' then avail(1)<='0'; elsif set_avail_ps(1)='1' then avail(1)<='1'; else null;end if;  
      if clr_avail_ps(2)='1'and set_avail_ps(2)='0' then avail(2)<='0'; elsif set_avail_ps(2)='1' then avail(2)<='1'; else null;end if;     
      if clr_avail_ps(3)='1'and set_avail_ps(3)='0' then avail(3)<='0'; elsif set_avail_ps(3)='1' then avail(3)<='1'; else null;end if; 
      if clr_avail_ps(4)='1'and set_avail_ps(4)='0' then avail(4)<='0'; elsif set_avail_ps(4)='1' then avail(4)<='1'; else null;end if; 
      if clr_avail_ps(5)='1'and set_avail_ps(5)='0' then avail(5)<='0'; elsif set_avail_ps(5)='1' then avail(5)<='1'; else null;end if; 
      if clr_avail_ps(6)='1'and set_avail_ps(6)='0' then avail(6)<='0'; elsif set_avail_ps(6)='1' then avail(6)<='1'; else null;end if; 
      if clr_avail_ps(7)='1'and set_avail_ps(7)='0' then avail(7)<='0'; elsif set_avail_ps(7)='1' then avail(7)<='1'; else null;end if; 

      clr_avail_ps <= (others =>'0'); --pulses only set for 1 clk_i period



      --if mil_trm_rdy = '0' and manchester_fpga = '0' then mil_trm_start <= '0';  --now done from scheduler
      --elsif manchester_fpga = '1' then  mil_trm_start <= '0'; end if;
      
      --if mil_rcv_rdy = '0' and manchester_fpga = '0' then mil_rd_start <= '0';    --now done from scheduler
      --elsif manchester_fpga = '1' then  mil_rd_start <= '0'; end if;

      if slave_i.cyc = '1' and slave_i.stb = '1' and ex_stall = '1' then
      -- begin of wishbone cycle

      --##############################TX FIFO ACCESS#############################################
        if (LA_a_var = mil_wr_cmd_a_map) or  (LA_a_var = mil_rd_wr_data_a_map) then
        -- check existing word register
            if slave_i.sel = "1111" then
              if slave_i.we = '1'  then
                -- write low word
                  if tx_fifo_full ='0' then               
--                  if mil_trm_rdy = '1' and mil_trm_start = '0' then
--                    mil_trm_start <= '1';  --                   -- start on read mil transmitter
--                    mil_trm_cmd   <= slave_i.adr(2);            -- Address bit 2 results in CMD Telegram   
--                    mil_trm_data  <= slave_i.dat(15 downto 0);  -- update mil_trm_data

                    tx_fifo_data_in(16)          <= slave_i.adr(2);  --Address bit 2 results in CMD Telegram 
                    tx_fifo_data_in(15 downto 0) <= slave_i.dat(15 downto 0);
                    tx_fifo_write_en             <= slave_i.stb and not slave_i_stb_dly;
                  
                    ex_stall <= '0';
                    ex_ack   <= '1';
                  else
                  -- write to mil not allowed, because tx_fifo is full
                    ex_stall <= '0';
                    ex_err   <= '1';
                  end if;--tx_fifo_full
              else
                -- read low word not allowed on TX_FIFO --> Use TX_TASKREGs for READs
                  ex_stall <= '0';
                  ex_err <= '1';
              end if;--slave_i.we 
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;--slave_i.sel
       --##############################TX_TASKREG0############################################################
        --all taskreg accesses result in CMD Telegrams because they sent function codes
        ELSIF (LA_a_var = wr_tx_taskreg0_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg0(16) = '0' THEN --only successfull when taskreg is ready
              tx_taskreg0 <= '1' &  slave_i.dat(15 DOWNTO 0);  --set bit 16 means a new task request
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               --W/O register, read not allowed
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 --high/unaligned word access (others than sel=1111) not allowed
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;  
        --##############################TX_TASKREG1############################################################
        ELSIF (LA_a_var = wr_tx_taskreg1_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg1(16) = '0' THEN 
              tx_taskreg1 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;       
        --##############################TX_TASKREG2############################################################
        ELSIF (LA_a_var = wr_tx_taskreg2_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg2(16) = '0' THEN 
              tx_taskreg2 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;    
        --##############################TX_TASKREG3############################################################
        ELSIF (LA_a_var = wr_tx_taskreg3_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg3(16) = '0' THEN 
              tx_taskreg3 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;  
        --##############################TX_TASKREG4############################################################
        ELSIF (LA_a_var = wr_tx_taskreg4_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg4(16) = '0' THEN 
              tx_taskreg4 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;   
        --##############################TX_TASKREG5############################################################
        ELSIF (LA_a_var = wr_tx_taskreg5_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg5(16) = '0' THEN 
              tx_taskreg5 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;  
        --##############################TX_TASKREG6############################################################
        ELSIF (LA_a_var = wr_tx_taskreg6_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg6(16) = '0' THEN 
              tx_taskreg6 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;   
        --##############################TX_TASKREG7############################################################
        ELSIF (LA_a_var = wr_tx_taskreg7_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '1' AND tx_taskreg7(16) = '0' THEN 
              tx_taskreg7 <= '1' &  slave_i.dat(15 DOWNTO 0);  
              ex_stall    <= '0';
              ex_ack      <= '1';
            ELSE                                               
              ex_stall    <= '0';
              ex_err      <= '1';
            END IF;
          ELSE                                                 
            ex_stall      <= '0';
            ex_err        <= '1';
          END IF;   
       --##############################RX_TASKREG0############################################################
        ELSIF (LA_a_var = rd_rx_taskreg0_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg0(16) = '0' THEN --successful read only on rx_taskreg is not faulty
              slave_o.dat (15 downto 0)  <= rx_taskreg0(15 DOWNTO 0);
              clr_avail_ps(0)<= '1';                           --avail bit will be cleared by scu bus read access
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               --write attempts result in DTACK Error
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 --high/unaligned word access (others than sel=1111) not allowed
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG1############################################################
         ELSIF (LA_a_var = rd_rx_taskreg1_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg1(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg1(15 DOWNTO 0);
              clr_avail_ps(1)<= '1';                            
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG2############################################################
         ELSIF (LA_a_var = rd_rx_taskreg2_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg2(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg2(15 DOWNTO 0);
              clr_avail_ps(2)<= '1';                            
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG3############################################################
         ELSIF (LA_a_var = rd_rx_taskreg3_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg3(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg3(15 DOWNTO 0);
              clr_avail_ps(3)<= '1';                           
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG4############################################################
         ELSIF (LA_a_var = rd_rx_taskreg4_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg4(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg4(15 DOWNTO 0);
              clr_avail_ps(4)<= '1';                           
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG5############################################################
         ELSIF (LA_a_var = rd_rx_taskreg5_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg5(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg5(15 DOWNTO 0);
              clr_avail_ps(5)<= '1';                            
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG6############################################################
         ELSIF (LA_a_var = rd_rx_taskreg6_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg6(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg6(15 DOWNTO 0);
              clr_avail_ps(6)<= '1';                            
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################RX_TASKREG7############################################################
         ELSIF (LA_a_var = rd_rx_taskreg7_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' AND rx_taskreg7(16) = '0' THEN 
              slave_o.dat (15 downto 0)  <= rx_taskreg7(15 DOWNTO 0);
              clr_avail_ps(7)<= '1';                            
              ex_stall       <= '0';
              ex_ack         <= '1';
            ELSE                                               
              ex_stall       <= '0';
              ex_err         <= '1';
            END IF;
          ELSE                                                 
            ex_stall         <= '0';
            ex_err           <= '1';
          END IF;
       --##############################status_avail############################################################
        ELSIF (LA_a_var = rd_status_avail_a_map) THEN
          IF slave_i.sel = "1111" THEN
            IF slave_i.we = '0' THEN 
              slave_o.dat(15 DOWNTO 0) <= "00000000" & avail; -- available bits 7..0              
              ex_stall                 <= '0';
              ex_ack                   <= '1';
            ELSE
              ex_stall <= '0';
              ex_err   <= '1';
            END IF;
          ELSE
            ex_stall <= '0';
            ex_err   <= '1';
          END IF;
        --############################legacy registers###############################################
          elsif (LA_a_var = mil_wr_rd_status_a_map) then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write status register
                manchester_fpga <= slave_i.dat(b_sel_fpga_n6408);
                ev_filt_12_8b   <= slave_i.dat(b_ev_filt_12_8b);
                ev_filt_on      <= slave_i.dat(b_ev_filt_on);
                debounce_on     <= '1'; -- slave_i.dat(b_debounce_on);
                puls2_frame     <= slave_i.dat(b_puls2_frame);
                puls1_frame     <= slave_i.dat(b_puls1_frame);
                ev_reset_on     <= slave_i.dat(b_ev_reset_on);
                clr_mil_rcv_err <= slave_i.dat(b_mil_rcv_err);
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read status register
                slave_o.dat(15 downto 0) <= ( manchester_fpga & ev_filt_12_8b & ev_filt_on & debounce_on              -- mil-status[15..12]
                                            & puls2_frame & puls1_frame & ev_reset_on & mil_rcv_error                 -- mil-status[11..8]
                                            & tx_fifo_full & Mil_Cmd_Rcv & mil_rcv_rdy & ev_fifo_full                 -- mil-status[7..4]  tx_fifo_full instead of mil_trm_rdy                                          
                                            & ev_fifo_ne & db_data_req_intr & db_data_rdy_intr & db_interlock_intr ); -- mil-status[3..0]
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = mil_wr_rd_lemo_conf_a_map) then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write lemo config register
                lemo_out_en(1)       <= slave_i.dat(b_lemo1_out_en);
                lemo_out_en(2)       <= slave_i.dat(b_lemo2_out_en);
                lemo_out_en(3)       <= slave_i.dat(b_lemo3_out_en);
                lemo_out_en(4)       <= slave_i.dat(b_lemo4_out_en);
                lemo_event_en(1)     <= slave_i.dat(b_lemo1_event_en);                                        
                lemo_event_en(2)     <= slave_i.dat(b_lemo2_event_en);
                lemo_event_en(3)     <= slave_i.dat(b_lemo3_event_en);
                lemo_event_en(4)     <= slave_i.dat(b_lemo4_event_en);
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read lemo config register
                slave_o.dat(15 downto 0) <= ( "00000000" & lemo_event_en(4 downto 1) & lemo_out_en(4 downto 1) );-- mil-lemo config[15..0]
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = mil_wr_rd_lemo_dat_a_map) then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write lemo data register
                lemo_dat(1)        <= slave_i.dat(b_lemo1_dat);
                lemo_dat(2)        <= slave_i.dat(b_lemo2_dat);
                lemo_dat(3)        <= slave_i.dat(b_lemo3_dat);
                lemo_dat(4)        <= slave_i.dat(b_lemo4_dat);
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read lemo data register
                slave_o.dat(15 downto 0) <=  "000000000000" & lemo_dat(4 downto 1 );-- mil lemo data [15..0]
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = mil_rd_lemo_inp_a_map)then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write to lemo input register is without effect
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read lemo input register register
                slave_o.dat(15 downto 0) <= ( "000000000000" & lemo_i_reg (4 downto 1));--lemo input data [15..0]
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;
              
          elsif (LA_a_var = rd_clr_no_vw_cnt_a_map)then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write access clears no valid word counters
                clr_no_vw_cnt <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read no valid word counters
                slave_o.dat(15 downto 0) <= no_vw_cnt;
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- access to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = rd_wr_not_eq_cnt_a_map)then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write access clears not equal counters
                clr_not_equal_cnt <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read not equal counters
                slave_o.dat(15 downto 0) <= not_equal_cnt;
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- write to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = rd_clr_ev_fifo_a_map) then
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write access clears event fifo
                clr_ev_fifo <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read event fifo
                if ev_fifo_ne = '1' then
                  -- read is okay because fifo is not empty
                  rd_ev_fifo <= '1';
                  slave_o.dat(15 downto 0) <= ep_read_port;
                  ex_stall <= '0';
                  ex_ack <= '1';
                else
                  -- read is not okay because fifo is empty
                  ex_stall <= '0';
                  ex_err <= '1';
                end if;
              end if;
            else
              -- write to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = rd_clr_ev_timer_a_map) then
            if slave_i.sel = "1111" then -- only double word access allowed
              if slave_i.we = '1' then
                -- write access clears event timer
                sw_clr_ev_timer <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read complete double word
                slave_o.dat(31 downto 0) <= std_logic_vector(ev_timer);
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- no complete double word access
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = rd_wr_dly_timer_a_map) then
            if slave_i.sel = "1111" then -- only double word access allowed
              if slave_i.we = '1' then
                -- write access clears event timer
                ld_dly_timer <= '1';
                ex_stall <= stall_dly_timer;
                ex_ack <= not stall_dly_timer;
              else
                -- read complete double word
                slave_o.dat(31 downto 0) <= "0000000" & std_logic_vector(dly_timer);
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- no complete double word access
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          elsif (LA_a_var = rd_clr_wait_timer_a_map) then
            if slave_i.sel = "1111" then -- only double word access allowed
              if slave_i.we = '1' then
                -- write access clears wait timer
                clr_wait_timer <= '1';
                ex_stall <= '0';
                ex_ack <= '1';
              else
                -- read complete double word
                slave_o.dat(31 downto 0) <= x"00" & std_logic_vector(wait_timer);
                ex_stall <= '0';
                ex_ack <= '1';
              end if;
            else
              -- no complete double word access
              ex_stall <= '0';
              ex_err <= '1';
            end if;
            

          elsif (LA_a_var >= evt_filt_first_a)  and   (LA_a_var <= evt_filt_last_a)  then -- read or write event filter ram 
            if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
              if slave_i.we = '1' then
                -- write event filter ram
                wr_filt_ram <= '1';
                ex_stall <= stall_filter;
                ex_ack <= not stall_filter;
              else
                -- read event filter ram
                rd_filt_ram <= '1';
                slave_o.dat(15 downto 0) <= ep_read_port;
                ex_stall <= stall_filter;
                ex_ack <= not stall_filter;
              end if;
            else
              -- write to high word or unaligned word is not allowed
              ex_stall <= '0';
              ex_err <= '1';
            end if;

          else
            ex_stall <= '0';
            ex_err <= '1';
          end if; --LA_a_var
      end if;--slave_i.cyc
    end if;--clocked_process
  end process p_regs_acc;

  
 
lemo_data_o(1) <= io_1 when (lemo_event_en(1)='1') else lemo_dat(1);   -- To be compatible with former SCU solution
lemo_data_o(2) <= io_2 when (lemo_event_en(2)='1') else lemo_dat(2);   -- which allows 2 event-driven lemo outputs
lemo_data_o(3) <= lemo_dat(3);                                         -- This is used in SIO (not event drive-able)
lemo_data_o(4) <= lemo_dat(4);                                         -- This is used in SIO (not event drive-able)

lemo_out_en_o(1)<= '1' when puls1_frame='1' else lemo_out_en(1);   -- To be compatible with former SCU solution
lemo_out_en_o(2)<= '1' when puls2_frame='1' else lemo_out_en(2);   -- which allows 2 event-driven lemo outputs
lemo_out_en_o(3)<= lemo_out_en(3);                                 -- This is used in SIO
lemo_out_en_o(4)<= lemo_out_en(4);                                 -- This is used in SIO

  
  
p_every_us: div_n
  generic map (
    n         => integer(clk_in_hz/1_000_000), -- KK alle us einen Takt aktiv (ena_every_us * 1000 = 1ms)
    diag_on   => 0                -- diag_on = 1 die Breite des Untersetzungzaehlers
                                  -- mit assert .. note ausgegeben.
    )

  port map (
    res       => '0',
    clk       => clk_i,
    ena       => open,            -- das untersetzende enable muss in der gleichen Clockdomaene erzeugt werden.
                                  -- Das enable sollte nur ein Takt lang sein.
                                  -- Z.B. koennte eine weitere div_n-Instanz dieses Signal erzeugen.  
    div_o     => ena_every_us     -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );
    
-- Timer Section

p_ev_timer: process (clk_i, nRst_i)
  begin
    if nRst_i = '0' then
      ev_timer <= to_unsigned(0, ev_timer'length);
    elsif rising_edge(clk_i) then
      if sw_clr_ev_timer = '1' or ev_clr_ev_timer = '1' then
        ev_timer <=  to_unsigned(0, ev_timer'length);
      elsif ena_every_us = '1' then
        ev_timer <= ev_timer + 1;
      end if;
    end if;
  end process p_ev_timer;


p_delay_timer: process (clk_i, nRst_i)

  variable  dly_timer_start:  std_logic;

  begin
    if nRst_i = '0' then
      dly_timer       <= (others => '1');           --to_unsigned(-1, dly_timer'length);
      dly_timer_start := '0';
      dly_intr        <= '0';

    elsif rising_edge(clk_i) then
      
      stall_dly_timer <= '1';
      
      if ld_dly_timer = '1' then
        stall_dly_timer <= '0';
        dly_intr <= '0';                            -- laden des delay timers setzt delay interrupt zurueck
        dly_timer <= unsigned(slave_i.dat(dly_timer'range));
        if dly_timer(dly_timer'high) = '0' then     -- laden des delay timers bei dem das oberste bit = 0 ist
          dly_timer_start := '1';                   -- startet den delay timer.
        else
          dly_timer_start := '0';                   -- stoppt den delay timer.
        end if;
      end if;
        
      if dly_timer_start = '1' then
        if ena_every_us = '1' then
          if dly_timer(dly_timer'high) = '0' then
            dly_timer <= dly_timer - 1;
          else
            dly_intr <= '1';
          end if;
        end if;
      end if;
    end if;
  end process p_delay_timer;
  
dly_intr_o <= dly_intr;


p_wait_timer: process (clk_i, nRst_i)
  begin
    if nRst_i = '0' then
      wait_timer <= to_unsigned(0, wait_timer'length);
    elsif rising_edge(clk_i) then
      if clr_wait_timer = '1' then
        wait_timer <=  to_unsigned(0, wait_timer'length);
      elsif ena_every_us = '1' then
        wait_timer <= wait_timer + 1;
      end if;
    end if;
  end process p_wait_timer;

end arch_wb_mil_sio;
