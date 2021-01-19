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
--|   08    | K.Kaiser    | 07.09.2017  |    Zur Durchsatzsteigerung des Devicebusses für die FG Parameterübertragung               |
--|         |             |             |    Ein One-Hot Timeslotscheduler steuert den DB-Zugriff (TS0=TX_FIFO, TX1..254=TaskRam)   |
--|         |             |             |    Scheduler in TS0: Alle TX_FIFO Aufträge werden abgewickelt bevor er in TS1 wechselt !  |
--|         |             |             |    tx_Fifo   (1024x17) Puffer für 1024  CMD oder Daten-Telegramme in Senderichtung        |
--|         |             |             |                        Bei Überfüllung des TX Fifo Puffers wird kein DTACK gegeben        |
--|         |             |             |    tx_TaskRam (1..254) Puffer für 254 CMD Telegramme (mit Rücklesen) vom DevBus Slave     |
--|         |             |             |                        Rücklesedaten werden in RX_Taskram gespeichert                     |
--|         |             |             |                        Avail und ggf Err Bit (Timeout oder Parity) wird gesetzt           |
--|         |             |             |                        Avail und ggf Err Bit wird bei Auslesen RX_Taskramzelle gelöscht   |
--|         |             |             |                        Solange eine Auftrag im  TX_TaskRam steht und nicht abgeschlossen  |
--|         |             |             |                        ist, wird ausserdem das tx_req Bit gesetzt                         |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
--|   08    | K.Kaiser    | 07.09.2017  |a)  hw6408_rdy Signal erkennt die Bestückung einer MilOption (bzw des Harris Bausteins)    |
--|         |             |             |    Wenn unbestückt: Registerzugriffe erzeugen eine etherbone cycle Error                  |
--|         |             |             |                                                                                           |
--|         |             |             |b)  Register reset_mil_macro für SW gesteuerten Reset des wb_mil_scu Macros hinzu          |
--|         |             |             |                                                                                           |
--|         |             | 12.01.2018  |    n_tx_req_led,n_rx_avail_led  hinzu                                                     |
--|         |             |             |    clr_rx_avail_ps, clr_rx_err_ps bei TX_Taskram Write Access hinzu                       |
--|         |             |             |                                                                                           |
--|         |             | 29.01.2018  |    clr_rx_err_ps ,clr_rx_avail_ps bei RX_Taskram Read Access wg Performance entfernt      |
--|         |             |             |    Grund:Die Etherbone Bridge kann (socket-orientiert) ihre Leseaufträge verwürfeln       |
--|         |             |             |                                                                                           |
--|         |             |             |                                                                                           |
--| --------+-------------+-------------+----------------------------------------------------------------------------------------   |
ENTITY wb_mil_scu IS

GENERIC (
      Clk_in_Hz:          INTEGER := 62_500_000;    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                                    -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden
                                                    -- SCU benutzt 62_500_000, SIO benutzt 125_000_000 Hz
      slave_i_adr_max:    INTEGER := 14             -- 14 für SCU, 17 für SIO
      );
PORT  (
    clk_i:                IN      STD_LOGIC;
    nRst_i:               IN      STD_LOGIC;
    slave_i:              IN      t_wishbone_slave_in;
    slave_o:              OUT     t_wishbone_slave_out;

    -- encoder (transmitter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:              IN      STD_LOGIC;-- HD6408-output: transmit bipolar positive.
    nME_BZO:              IN      STD_LOGIC;-- HD6408-output: transmit bipolar negative.

    ME_SD:                IN      STD_LOGIC;-- HD6408-output: '1' => send data is active.
    ME_ESC:               IN      STD_LOGIC;-- HD6408-output: encoder shift clock for shifting data into the encoder. The
                                            --                encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:               OUT     STD_LOGIC;-- HD6408-input:  serial data in accepts a serial data stream at a data rate
                                            --                equal to encoder shift clock.
    ME_EE:                OUT     STD_LOGIC;-- HD6408-input:  a high on encoder enable initiates the encode cycle.
                                            --                (Subject to the preceding cycle being completed).
    ME_SS:                OUT     STD_LOGIC;-- HD6408-input:  sync select actuates a Command sync for an input high
                                            --                and data sync for an input low.

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:               OUT     STD_LOGIC;-- HD6408-input:  A high input should be applied to bipolar one in when the bus is in its
                                            --                positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:               OUT     STD_LOGIC;-- HD6408-input:  A high input should be applied to bipolar zero in when the bus is in its
                                            --                negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:               OUT     STD_LOGIC;-- HD6408-input:  With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                            --                transition finder circuit. If not used this input must be held low.
    ME_CDS:               IN      STD_LOGIC;-- HD6408-output: high occurs during output of decoded data which was preced
                                            --                by a command synchronizing character. Low indicares a data sync.
    ME_SDO:               IN      STD_LOGIC;-- HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC:               IN      STD_LOGIC;-- HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                            --                synchronized by the recovered serial data stream.
    ME_VW:                IN      STD_LOGIC;-- HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD:                IN      STD_LOGIC;-- HD6408-output: take data is high during receipt of data after identification
                                            --                of a sync pulse and two valid Manchester data bits

    -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
    --    ME_12MHz:       out     std_logic;-- HD6408-input:    is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)


    Mil_BOI:              IN      STD_LOGIC;-- HD6408-input:  connect positive bipolar receiver, in FPGA directed to the external
                                            --                manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
                                            --                vhdl manchester macro.
    Mil_BZI:              IN      STD_LOGIC;-- HD6408-input:  connect negative bipolar receiver, in FPGA directed to the external
                                            --                manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
                                            --                vhdl manchester macro.
    Sel_Mil_Drv:          BUFFER  STD_LOGIC;-- HD6408-output: active high, enable the external open collector driver to the transformer
    nSel_Mil_Rcv:         OUT     STD_LOGIC;-- HD6408-output: active low, enable the external differtial receive circuit.
    Mil_nBOO:             OUT     STD_LOGIC;-- HD6408-output: connect bipolar positive output to external open collector driver of
                                            --                the transformer. Source is the external manchester en/decoder HD6408 via
                                            --                nME_BOO or the internal FPGA vhdl manchester macro.
    Mil_nBZO:             OUT     STD_LOGIC;-- HD6408-output: connect bipolar negative output to external open collector driver of
                                            --                the transformer. Source is the external manchester en/decoder HD6408 via
                                            --                nME_BZO or the internal FPGA vhdl manchester macro.
    nLed_Mil_Rcv:         OUT     STD_LOGIC;
    nLed_Mil_Trm:         OUT     STD_LOGIC;
    nLed_Mil_Err:         OUT     STD_LOGIC;
    error_limit_reached:  OUT     STD_LOGIC;
    Mil_Decoder_Diag_p:   OUT     STD_LOGIC_VECTOR(15 DOWNTO 0);
    Mil_Decoder_Diag_n:   OUT     STD_LOGIC_VECTOR(15 DOWNTO 0);
    timing:               IN      STD_LOGIC;
    nLed_Timing:          OUT     STD_LOGIC;
    dly_intr_o:           OUT     STD_LOGIC;
    nLed_Fifo_ne:         OUT     STD_LOGIC;
    ev_fifo_ne_intr_o:    OUT     STD_LOGIC;
    Interlock_Intr_i:     IN      STD_LOGIC;
    Data_Rdy_Intr_i:      IN      STD_LOGIC;
    Data_Req_Intr_i:      IN      STD_LOGIC;
    Interlock_Intr_o:     OUT     STD_LOGIC;
    Data_Rdy_Intr_o:      OUT     STD_LOGIC;
    Data_Req_Intr_o:      OUT     STD_LOGIC;
    nLed_Interl:          OUT     STD_LOGIC;
    nLed_Dry:             OUT     STD_LOGIC;
    nLed_Drq:             OUT     STD_LOGIC;
    every_ms_intr_o:      OUT     STD_LOGIC;
    lemo_data_o:          OUT     STD_LOGIC_VECTOR(4 DOWNTO 1);
    lemo_nled_o:          OUT     STD_LOGIC_VECTOR(4 DOWNTO 1);
    lemo_out_en_o:        OUT     STD_LOGIC_VECTOR(4 DOWNTO 1);
    lemo_data_i:          IN      STD_LOGIC_VECTOR(4 DOWNTO 1):= (OTHERS => '0');
    nsig_wb_err:          OUT     STD_LOGIC                   ;  -- '0' => gestretchte wishbone access Fehlermeldung
	  n_tx_req_led :        OUT     STD_LOGIC                   ;  -- low solange mindestens ein txreq ansteht
	  n_rx_avail_led:       OUT     STD_LOGIC                      -- low solange mindestens ein rxavail ansteht
    );
END wb_mil_scu;


ARCHITECTURE arch_wb_mil_scu OF wb_mil_scu IS

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
constant wr_soft_reset_a_map:       unsigned (15 downto 0) := sio_mil_first_reg_a + wr_soft_reset_a;


-- todo constant rd_status_avail_a_map:     unsigned (15 downto 0) := sio_mil_first_reg_a + rd_status_avail_a; --to read corresponding avail bits, bit[31..8] don't care

signal   manchester_fpga:           std_logic;  -- '1' => fpga manchester endecoder selected, '0' => external hardware manchester endecoder 6408 selected.
signal   ev_filt_12_8b:             std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal   ev_filt_on:                std_logic;  -- '1' => event filter is on, '0' => event filter is off.
signal   debounce_on:               std_logic;  -- '1' => debounce of device bus interrupt input is on.
signal   puls2_frame:               std_logic;  -- '1' => aus zwei events wird der Rahmenpuls2 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal   puls1_frame:               std_logic;  -- '1' => aus zwei events wird der Rahmenpuls1 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
signal   ev_reset_on:               std_logic;  -- '1' => events koennen den event timer auf Null setzen, vorausgesetzt das Eventfilter ist richtig programmiert.
signal   clr_mil_rcv_err:           std_logic;

signal   Mil_RCV_D:                 std_logic_vector(15 downto 0);
signal   Mil_Cmd_Rcv:               std_logic;
signal   mil_trm_rdy:               std_logic;
signal   mil_rcv_rdy:               std_logic;
signal   mil_rcv_error:             std_logic;

signal   clr_no_vw_cnt:             std_logic;
signal   no_vw_cnt:                 std_logic_vector(15 downto 0);

signal   clr_not_equal_cnt:         std_logic;
signal   not_equal_cnt:             std_logic_vector(15 downto 0);

signal   ex_stall, ex_ack, ex_err, intr: std_logic;  -- dummy


signal   mil_trm_cmd:               std_logic;
signal   mil_trm_data:              std_logic_vector(15 downto 0);

signal   mil_rd_start:              std_logic;

signal   sw_clr_ev_timer:           std_logic;
signal   ev_clr_ev_timer:           std_logic;
signal   ev_timer:                  unsigned(31 downto 0);

signal   ena_led_count:             std_logic;

signal   nSel_Mil_Drv:              std_logic;

signal   wr_filt_ram:               std_logic;
signal   rd_filt_ram:               std_logic;
signal   stall_filter:              std_logic;
signal   filt_data_i:               std_logic_vector(5 downto 0);
signal   ev_fifo_ne:                std_logic;
signal   ev_fifo_full:              std_logic;
signal   rd_ev_fifo:                std_logic;
signal   clr_ev_fifo:               std_logic;

signal   dly_timer:                 unsigned(24 downto 0);
signal   ld_dly_timer:              std_logic;
signal   stall_dly_timer:           std_logic;

signal   wait_timer:                unsigned(23 downto 0);
signal   clr_wait_timer:            std_logic;

signal   ep_read_port:              std_logic_vector(15 downto 0);    --event processing read port

signal   timing_received:           std_logic;

signal   ena_every_us:              std_logic;

signal   db_interlock_intr:         std_logic;
signal   db_data_rdy_intr:          std_logic;
signal   db_data_req_intr:          std_logic;

signal   dly_intr:                  std_logic;
signal   every_ms:                  std_logic;

signal   lemo_inp:                  std_logic_vector (4 downto 1);
signal   lemo_i_reg:                std_logic_vector (4 downto 1);
signal   lemo_dat:                  std_logic_vector (4 downto 1);
signal   lemo_out_en:               std_logic_vector (4 downto 1);
signal   lemo_event_en:             std_logic_vector (4 downto 1);

signal   io_1:                      std_logic;
signal   io_2:                      std_logic;

-------------------------------------------------------------------
signal   tx_fifo_write_en:          std_logic;
signal   tx_fifo_wr_pulse:          std_logic;
signal   tx_fifo_data_in:           std_logic_vector (16 downto 0);

signal   tx_fifo_read_en:           std_logic;
signal   tx_fifo_data_out:          std_logic_vector (16 downto 0);

signal   tx_fifo_empty:             std_logic;
signal   tx_fifo_full:              std_logic;

signal   tx_taskram_we:             std_logic;
signal   tx_taskram_re:             std_logic;
signal   tx_taskram_wr_a:           std_logic_vector (7 downto 0);
signal   tx_taskram_rd_a:           std_logic_vector (7 downto 0);
signal   tx_taskram_wr_d:           std_logic_vector (15 downto 0);
signal   tx_taskram_rd_d:           std_logic_vector (15 downto 0);

signal   tx_req:                    std_logic_vector (255 downto 1);
signal   tx_req_muxed:              std_logic_vector ( 15 downto 0);
signal   tx_task_ack:               std_logic_vector (255 downto 1);


signal   reset_6408:                std_logic;

signal   rx_taskram_we:             std_logic;
signal   rx_taskram_re:             std_logic;
signal   rx_taskram_wr_a:           std_logic_vector (7 downto 0);
signal   rx_taskram_rd_a:           std_logic_vector (7 downto 0);
signal   rx_taskram_wr_d:           std_logic_vector (15 downto 0);
signal   rx_taskram_rd_d:           std_logic_vector (15 downto 0);

signal   timeslot:                  integer RANGE 0 to ram_count;       --timeslot 0 is for tx_fifo, 1..255 is for tx_taskram
signal   set_rx_avail_ps:           std_logic_vector (255 downto 1);
signal   clr_rx_avail_ps:           std_logic_vector (255 downto 1);

signal   avail:                     std_logic_vector (255 downto 1);
signal   avail_muxed:               std_logic_vector (15 downto 0);

constant timeout_cntr_max:          integer := 70;  --max timeout 50 µs: TX Telegram + RX Telegram + 10µs Gap
signal   timeout_cntr:              integer :=  0;
signal   timeout_cntr_en:           std_logic;
signal   timeout_cntr_clr:          std_logic;
signal   slave_i_we_del:            std_logic;
signal   slave_i_stb_dly:           std_logic;
signal   slave_i_stb_dly2:           std_logic;

signal   task_runs:                 std_logic;

signal   mil_trm_start:             std_logic;
signal   mil_trm_start_dly:         std_logic;
signal   mil_trm_start_dly2:        std_logic;

signal   mil_rd_start_latched:      std_logic;
signal   mil_rd_start_dly:          std_logic;
signal   mil_rd_start_dly2:         std_logic;
signal   mil_rd_start_dly3:         std_logic;

signal   set_rx_err_ps:             std_logic_vector (255 downto 1);
signal   clr_rx_err_ps:             std_logic_vector (255 downto 1);
signal   rx_err:                    std_logic_vector (255 downto 0);
signal   rx_err_muxed:              std_logic_vector ( 15 downto 0);

signal   task_runs_del:             std_logic;

signal   hw6408_rdy:                std_logic;
signal   hw6408_rdy_sync:           std_logic;
-----------------------------------------------------------------------

signal   n_rst_mil_macro:           std_logic;
signal   ex_err_led:                std_logic;
signal   ex_stall_res:              std_logic;
signal   ex_ack_res:                std_logic;
signal   ex_err_res:                std_logic;
signal   n_modulreset:              std_logic;
signal   tx_req_led:                std_logic;
signal   rx_avail_led:              std_logic;

signal   Rst:                       std_logic;
signal   modulreset:                std_logic;
BEGIN


slave_o.rty               <= '0';
reset_6408                <= '0';

Rst <= NOT nRst_i;
modulreset <= NOT n_modulreset;

-- only for reset register access , the ext_xxx_res signals are valid, otherwise take regular ones

p_mux_slave_o_ctrl: PROCESS (slave_i,ex_stall_res,ex_ack_res,ex_err_res,ex_stall,ex_ack,ex_err)
VARIABLE LA_a_var          : UNSIGNED (slave_i_adr_max downto 2);
BEGIN
  LA_a_var                 := UNSIGNED(slave_i.adr(slave_i_adr_max downto 2));
  IF (LA_a_var = wr_soft_reset_a_map)    THEN
    slave_o.stall             <= ex_stall_res;
    slave_o.ack               <= ex_ack_res;
    slave_o.err               <= ex_err_res;
	 ex_err_led                <= ex_err_res;
  ELSE
    slave_o.stall             <= ex_stall;
    slave_o.ack               <= ex_ack;
    slave_o.err               <= ex_err;
	 ex_err_led                <= ex_err;
   END IF;
END PROCESS p_mux_slave_o_ctrl;



ena_led_cnt: div_n
  GENERIC MAP (
    n         => integer(real(clk_in_Hz) * 0.02),  -- Vorgabe der Taktuntersetzung: 20 ms
    diag_on   => 0                                 -- diag_on = 1 die Breite des Untersetzungzaehlers  mit assert .. note ausgegeben
   )
  PORT MAP (
    res       => '0',
    clk       => clk_i,
    ena       => open,                             -- das untersetzende enable muss in der gleichen Clockdomaene erzeugt werden.
                                                   -- ENA sollte nur ein Takt lang sein (z.B.durch eine weitere div_n-Instanz erzeugbar)
    div_o     => ena_led_count                     -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );


every_1ms_inst: div_n
  GENERIC MAP (
    n         => 1000, -- KK                       -- Vorgabe der Taktuntersetzung. 1ms = 0.001 = 1/1000 * ena_every_us (1000)
    diag_on   => 0                                 -- diag_on = 1 die Breite des Untersetzungzaehlers  mit assert .. note ausgegeben
    )
  PORT MAP (
    res       => '0',
    clk       => clk_i,
    ena       => ena_every_us,                     -- das untersetzende enable muss in der gleichen Clockdomaene erzeugt werden.
                                                   -- ENA sollte nur ein Takt lang sein (z.B.durch eine weitere div_n-Instanz erzeugbar)
    div_o     => every_ms                          -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );

every_ms_intr_o <= every_ms;

-- LED Signal Stretcher

led_rcv: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         => ena_led_count,    -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => Mil_Rcv_Rdy,      -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                     -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Mil_Rcv,     -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );


led_trm: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => Sel_Mil_Drv,     -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Mil_Trm,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );


led_err: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => Mil_Rcv_Error,   -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Mil_Err,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );

led_interl: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,   -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => db_interlock_intr,-- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                     -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Interl,      -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );

led_dry: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => db_data_rdy_intr,-- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_dry,        -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );

led_drq: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => db_data_req_intr,-- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_drq,        -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN

    );

led_timing: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => timing_received, -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Timing,     -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );


lemo_nled_i: FOR i IN 1 TO 4 GENERATE

lemo_nled_o_x: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- if you use ena for a reduction, signal should be generated from the same
                                    -- clock domain and should be only one clock period active.
    CLK         => clk_i,
    Sig_In      => lemo_data_i(i),  -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0'
                                    -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED        => OPEN,            -- Push-Pull output, active low, inactive high.
    nLed_opdrn  => lemo_nled_o(i)   -- open drain output, active low, inactive tristate.
    );
end generate;



sig_wb_err: led_n
  GENERIC MAP (
    stretch_cnt => 6
    )
  PORT MAP (
    ena         =>  ena_led_count,  -- ENA (generated by same clock domain) is for one clock period active.
    CLK         => clk_i,
    Sig_In      => ex_err_led, -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nsig_wb_err,     -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );



led_fifo_ne: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         => ena_led_count,   -- ENA (generated by same clock domain) is for one clock period active
    CLK         => clk_i,
    Sig_In      => ev_fifo_ne,      -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero.
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => nLed_Fifo_ne,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );


 -- Debouncer for Input Signals


p_deb_intl: debounce
  GENERIC MAP (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang DB_Out diesem Pegel folgt.
    )
  PORT MAP (
    DB_In   => Interlock_Intr_i,            -- Das zu entprellende Signal
    Reset   => Rst,                  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_interlock_intr            -- Das entprellte Signal von "DB_In".
    );

Interlock_Intr_o <= db_interlock_intr;


p_deb_drdy: debounce
  GENERIC MAP (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                            -- "DB_Out" diesem Pegel folgt.
    )
  PORT MAP (
    DB_In   => Data_Rdy_Intr_i,             -- Das zu entprellende Signal
    Reset   => Rst,                  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_data_rdy_intr             -- Das entprellte Signal von "DB_In"
    );

Data_Rdy_Intr_o <= db_data_rdy_intr;


p_deb_dreq: debounce
  GENERIC MAP (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, debounce count gibt die Anzahl von Taktperioden vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                            -- "DB_Out" diesem Pegel folgt.
    )
  PORT MAP (
    DB_In   => Data_Req_Intr_i,             -- Das zu entprellende Signal
    Reset   => Rst,                  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => db_data_req_intr             -- Das entprellte Signal von "DB_In".
    );

Data_Req_Intr_o <= db_data_req_intr;

p_deb_lemo_i: FOR I IN 1 TO 4 GENERATE
p_deb_lemo_x: debounce
  GENERIC MAP (
    DB_Cnt  => clk_in_hz / (1_000_000/ 2)   -- "DB_Cnt" = fuer 2 us, DB_Cnt gibt die Zahl der Takte vor, die das
                                            -- Signal "DB_In" mindestens '1' oder '0' sein muss,damit "DB_Out" diesem Pegel folgt.
    )
  PORT MAP (
    DB_In   => lemo_data_i(i),              -- Das zu entprellende Signal
    Reset   => Rst,                  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk     => clk_i,
    DB_Out  => lemo_inp(i)                  -- Das entprellte Signal von "DB_In".
    );
END GENERATE;





Mil_1:  mil_hw_or_soft_ip
  GENERIC MAP (
    Clk_in_Hz =>  Clk_in_Hz                  -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                             -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                             -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hz definiert sein.
    )
  PORT MAP  (
    -- encoder (transmitter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BZO           =>  nME_BZO,           -- in: HD6408-output: transmit bipolar positive.
    nME_BOO           =>  nME_BOO,           -- in: HD6408-output: transmit bipolar negative.

    ME_SD         =>  ME_SD,        -- in: HD6408-output: '1' => send data is active.
    ME_ESC        =>  ME_ESC,       -- in: HD6408-output: encoder shift clock for shifting data into the encoder. The,
                                    --                    encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI        =>  ME_SDI,       -- out: HD6408-input: serial data in accepts a serial data stream at a data rate
                                    --                    equal to encoder shift clock.
    ME_EE         =>  ME_EE,        -- out: HD6408-input: a high on encoder enable initiates the encode cycle.
                                    --                    (Subject to the preceding cycle being completed).
    ME_SS         =>  ME_SS,        -- out: HD6408-input: sync select actuates a Command sync for an input high
                                    --                    and data sync for an input low.
    Reset_Puls    =>  modulreset,

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
    Wr_Mil        =>  mil_trm_start_dly2,
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
    clr_mil_rcv_err     =>  clr_mil_rcv_err,
    hw6408_rdy          =>  hw6408_rdy
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
    nRst_i            => n_modulreset,
    wr_filt_ram       => wr_filt_ram,
    rd_filt_ram       => rd_filt_ram,
    rd_ev_fifo        => rd_ev_fifo,
    clr_ev_fifo       => clr_ev_fifo,
    filt_addr         => slave_i.adr(11+2 DOWNTO 2),
    filt_data_i       => slave_i.dat(filter_data_width-1 DOWNTO 0),
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

------------------------------------------------------------------------------------------------------------------------------
--  SW controlled reset mode (For in-depth reset of the wb_mil_scu macro and sub_blocks)
--
--  All LEDs, delay and event timer and the Dividers e.g. ena_every_us are not in the scope of the SW Modul Reset
--  All other Registers and processes can be influenced by the SW Modul Reset

--  As long as the n_rst_mil_macro is "0" only the mil macro reset register is accessible.
--  As long as the n_rst_mil_macro is "0" other registers responds with etherbone error.
--  n_rst_mil_macro bit is "1" after Power-on-Reset.
--  After set by SW, it needs to be cleared by SW too after a sufficient reset time depending on faulty devbus telegrams on the fly
--     Reset time should be more than 70µs for telegrams as invoked TX_Task Registers.
--     Reset time for Beam Diagnosis Mode depends on Burst length defined for the Beam Diagnosis job given before.
------------------------------------------------------------------------------------------------------------------------------

p_reset_reg: PROCESS (clk_i, nrst_i, slave_i)   -- reset register needs a own process, which not depends on n_modulreset
VARIABLE LA_a_var          : UNSIGNED (slave_i_adr_max downto 2);
BEGIN
  LA_a_var                 := UNSIGNED(slave_i.adr(slave_i_adr_max downto 2));           -- variables evaluated at each clk edge

  IF nrst_i = '0' THEN

    n_rst_mil_macro   <= '1';

    ex_stall_res      <= '1';
    ex_ack_res        <= '0';
    ex_err_res        <= '0';
	 n_modulreset      <= '0'; -- on powerup depending on nrst_i

  ELSIF RISING_EDGE(clk_i) THEN
    ex_stall_res      <= '1';
    ex_ack_res        <= '0';
    ex_err_res        <= '0';
 	 n_modulreset      <= n_rst_mil_macro; -- otherwise dependig on register bit

    IF slave_i.cyc = '1' AND slave_i.stb = '1' AND ex_stall_res = '1' THEN              -- begin of wishbone cycle
      IF (LA_a_var = wr_soft_reset_a_map)    THEN
        if slave_i.sel = "1111" then -- only word access to modulo-4 address allowed
          if slave_i.we = '1' then
             -- set_reset_flag
            n_rst_mil_macro          <= slave_i.dat(0);
            ex_stall_res             <= '0';
            ex_ack_res               <= '1';
          end if;
        else
          -- access to high word or unaligned word is not allowed
          ex_stall_res <= '0';
          ex_err_res   <= '1';
        end if; --slave_i.sel
      ELSE
        ex_stall_res <= '0';
        ex_err_res   <= '1';
      END IF; --LA_a_var
    END IF;--slave_i.cyc
  END IF;--clocked_process


END PROCESS p_reset_reg;




------------------------------------------------------------------------------------------------------------------------------
--  tx_taskreg ram must be a DP RAM (decoupling write and read)
--  If written with a task, a trm_req bit is set on trm_req vector. trm_req is cleared after completing a task.
--  trm_req may not be visible by SW, because DevBus may be faster than SW routines.
--  trm_req bits (0..255 max) are separate reg bits for controling them separately.

--  1-Hot Scheduler FSM scans all trm_req bits robin-round and provides related RAM content.
--  If trm_req bit is set, according tx_taskram content is sent as CMD Telegram  and DevBus answer is awaited (Timeout Cntr starting as well)

--  DevBus answer is stored in rx_taskreg RAM and rx_avail bit is set in rx_avail vector
--  If received telegram was faulty or a time-out, a related rx_err bit is set too
--  When rx_taskram data is read ,  related rx_avail and rx_err bits are cleared.
------------------------------------------------------------------------------------------------------------------------------

tx_taskram : generic_simple_dpram
  GENERIC MAP(
    g_data_width               => 16,
    g_size                     => 256,
    g_with_byte_enable         => false,
    g_addr_conflict_resolution => "write_first",
    g_dual_clock               => false
  )
  PORT MAP(
    rst_n_i                    => n_modulreset,
    clka_i                     => clk_i,
    wea_i                      => tx_taskram_we,
    aa_i                       => tx_taskram_wr_a,
    da_i                       => tx_taskram_wr_d,
    clkb_i                     => clk_i,
    ab_i                       => tx_taskram_rd_a,
    qb_o                       => tx_taskram_rd_d
  );

--Usage "simple dualport ram"  SCU Bus Port is r/w, DevBus Port is r/o
--     wr and rd on same address --> new wr data appears 1 clock later on qb_o
--Preference is resolved by SW :
--     SW writes only when tx_req bit not set (SW must not wr 2 times for same task)
--     Scheduler reads tx_ram cell only when tx_req bit is set
--     DPRAM write has preference -- no valid SCU Bus write will be lost



rx_taskram : generic_simple_dpram
  GENERIC MAP(
    g_data_width               => 16,
    g_size                     => 256,
    g_with_byte_enable         => false,
    g_addr_conflict_resolution => "write_first",
    g_dual_clock               => false)
  PORT MAP(
    rst_n_i                    => n_modulreset,
    clka_i                     => clk_i,
    wea_i                      => rx_taskram_we,
    aa_i                       => rx_taskram_wr_a,
    da_i                       => rx_taskram_wr_d,
    clkb_i                     => clk_i,
    ab_i                       => rx_taskram_rd_a,
    qb_o                       => rx_taskram_rd_d
  );


------------------------------------------------------------------------------------------------------------------------------
tx_fifo : generic_sync_fifo
  GENERIC MAP(
    g_data_width => 17,
    g_size       => 1024
  )
  PORT MAP(
    clk_i   => clk_i,
    rst_n_i => n_modulreset,
    we_i    => tx_fifo_write_en,
    d_i     => tx_fifo_data_in,
    rd_i    => tx_fifo_read_en,
    q_o     => tx_fifo_data_out,
    empty_o => tx_fifo_empty,
    full_o  => tx_fifo_full
  );


commonclockedlogic_p : PROCESS (clk_i, n_modulreset)
BEGIN
  IF n_modulreset = '0' THEN

    timeout_cntr            <=  0 ;
    slave_i_stb_dly         <= '0';
    slave_i_stb_dly2        <= '0';

    slave_i_we_del          <= '0';
    mil_trm_start_dly       <= '0';
    mil_trm_start_dly2      <= '0';
    mil_rd_start_latched    <= '0';
    mil_rd_start_dly        <= '0';
    mil_rd_start_dly2       <= '0';
    mil_rd_start_dly3       <= '0';

    task_runs_del           <= '0';

    hw6408_rdy_sync         <= '0';

  ELSIF rising_edge (clk_i) THEN

    IF timeout_cntr_clr= '1' then
      timeout_cntr          <= 0;
    ELSIF timeout_cntr_en = '1' AND ena_every_us='1' THEN
      timeout_cntr          <= timeout_cntr + 1;
    ELSE
      NULL;
    END IF;

    IF mil_rd_start='1' THEN                               -- min  1 dsc pulse make hd6408 happy for reset of mil_rcv_rdy
       mil_rd_start_latched  <= mil_rd_start;
    ELSIF mil_rd_start_dly2='1' AND mil_rd_start_dly3='1' THEN
       mil_rd_start_latched  <= '0';
    END IF;

    IF ena_every_us='1' THEN
       mil_rd_start_dly      <= mil_rd_start_latched;
       mil_rd_start_dly2     <= mil_rd_start_dly;
       mil_rd_start_dly3     <= mil_rd_start_dly2;
    END IF;

    hw6408_rdy_sync          <= hw6408_rdy;
    slave_i_stb_dly          <= slave_i.stb;
    slave_i_stb_dly2         <= slave_i_stb_dly;
    slave_i_we_del           <= slave_i.we;
    mil_trm_start_dly        <= mil_trm_start;
    mil_trm_start_dly2       <= mil_trm_start_dly;

    task_runs_del            <= task_runs;

  END IF;
END PROCESS commonclockedlogic_p;


-- This registered mux fetches data from TX_FIFO or TX_TASKRAM for DevBus Transmission
-- On Scheduler Timeslot 0 data is got from TX_FIFO
-- On Scheduler Timeslot 1....254 data is got from TX_TaskRam
-- Therefore TX_TaskRAM Address is calculated from Timeslot pointer
-- TX_TaskRAM Read Address must not be applied when Avail bit isn't set
-- This is done to keep  TX_TaskRAM ready for new writes from SCU Bus.


schedule_mux : PROCESS (clk_i, n_modulreset)
BEGIN
  IF n_modulreset = '0' THEN
    mil_trm_data     <= (others =>'0');
    mil_trm_cmd      <= '0';
    tx_taskram_rd_a  <= (others =>'0');
    tx_taskram_re    <= '0';
    tx_fifo_read_en  <= '0';
  ELSIF rising_edge (clk_i) THEN
    tx_taskram_re    <= '0';
    tx_taskram_rd_a  <= (others =>'0');
    tx_fifo_read_en  <= '0';

    IF timeslot = 0 THEN
      mil_trm_data       <= tx_fifo_data_out(15 DOWNTO 0);
      mil_trm_cmd        <= tx_fifo_data_out(16);                       -- tx_fifo cmd or data telegram depends on upper bit
      tx_fifo_read_en    <= mil_trm_start AND NOT mil_trm_start_dly;    -- only one pulse to pop fifo, mil_trm_start_dly used for start transmission

    ELSIF ((timeslot >= 1) AND (timeslot <= ram_count )) THEN
      mil_trm_cmd      <= '1';                                          -- tx_taskram sents only cmd telegrams
      mil_trm_data     <= tx_taskram_rd_d(15 DOWNTO 0);

      IF tx_req (timeslot) = '1' THEN
        tx_taskram_rd_a  <= std_logic_vector (to_unsigned (timeslot, 8)) ;
        tx_taskram_re    <= '1';
      end if;

    ELSE
        mil_trm_data     <= (others =>'0');
    END IF;
  END IF;--clk_i

END PROCESS schedule_mux;


schedule_p : PROCESS (clk_i, n_modulreset)
BEGIN
  IF n_modulreset = '0' THEN
    timeslot           <=  0 ;
    task_runs          <= '0';

    timeout_cntr_en    <= '0';
    timeout_cntr_clr   <= '0';

    mil_rd_start       <= '0';
    mil_trm_start      <= '0';

    rx_taskram_we      <= '0';
    rx_taskram_wr_d    <= (OTHERS => '0');
    rx_taskram_wr_a    <= (OTHERS => '0');

    set_rx_avail_ps    <= (OTHERS => '0');
    set_rx_err_ps      <= (OTHERS => '0');
  ELSIF rising_edge (clk_i) THEN
    -- these 6 register are only high for one pulse
    mil_trm_start      <= '0';
    mil_rd_start       <= '0';
    tx_task_ack        <= (OTHERS => '0');
    set_rx_avail_ps    <= (OTHERS => '0');
    set_rx_err_ps      <= (OTHERS => '0');
    timeout_cntr_clr   <= '0';
    rx_taskram_we      <= '0';
    -----------------------------------------Timeslot 0 TX_FIFO--------------------------------------------------------------------------------
    IF timeslot= 0 then                                                            --Empty whole TX_FIFO on timeslot 0

      IF  tx_fifo_empty='1'    AND task_runs='0'  THEN                             --skip if there is nothing to do or fifo task finished
        timeslot         <= timeslot + 1 ;
        timeout_cntr_en  <= '0';
        timeout_cntr_clr <= '1';
        task_runs        <= '0';
      ELSIF tx_fifo_empty='0'  AND  mil_trm_rdy = '1'  THEN                        --perform fifo task
        mil_trm_start    <= '1';
        task_runs        <= '1';
        timeout_cntr_en  <= '1';
        timeout_cntr_clr <= '1';
      ELSIF  tx_fifo_empty='1' AND  mil_trm_rdy = '1' AND timeout_cntr=22  THEN    --wait for last telegram before exit  todo: maybe 21 is ok too
        mil_trm_start    <= '0';
        task_runs        <= '0';
        timeout_cntr_en  <= '0';
        timeout_cntr_clr <= '1';
      ELSE
        NULL;
      END IF;
   --------------------------------------------Timeslot 1...254 TX_TaskRam----------------------------------------------------------------------
    ELSIF ((timeslot >= 1) AND (timeslot <= ram_count )) THEN      --If not Timeslot 0: Do all taskram slots one after another
                                                                   --Timeslot 255 reserved for "Beam Transmission Mode"
      IF    tx_req(timeslot)='0' and task_runs_del='0'  then           --proceed with scheduler on no task and no request

        IF timeslot < ram_count  THEN
          timeslot           <= timeslot +1;                       --jump to next timeslot(or to 0)
        ELSE
          timeslot           <= 0;
        END IF;

      ELSIF tx_req(timeslot) = '1'  or task_runs = '1'  THEN       --check for taskrequest or running task

        IF mil_trm_rdy = '1' AND task_runs_del = '0' and Mil_Rcv_Rdy = '0' THEN     --Case: No Task is running, but transmitter is ready
          mil_trm_start               <= '1';                      --      or pulse for tx start and timeout_cntr
          timeout_cntr_en             <= '1';
          task_runs                   <= '1';

        ELSIF task_runs = '1' THEN                                 --Case Transmitter is already running

          IF timeout_cntr = timeout_cntr_max  OR Mil_Rcv_Rdy = '1' THEN          --wait 20µs for tx, 20 for rx and 15 for gap
            --IF timeslot < ram_count THEN
            --  timeslot                <= timeslot +1;              --jump to next timeslot(or to 0)
            --ELSE
            --  timeslot                <= 0;
            --END IF;
            tx_task_ack(timeslot)     <= '1';                      --pulse to clear tx_req, needs 2 clocks to get effective
            task_runs                 <= '0';
            set_rx_avail_ps(timeslot) <= '1';                      --todo maybe wait until slave_i.stb=0
            timeout_cntr_en           <= '0';                      --stop and reset timeout_cntr for next use
            timeout_cntr_clr          <= '1';
            IF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '0' THEN      --prepare data output
              rx_taskram_wr_d         <= MIL_RCV_D;
              rx_taskram_wr_a         <= std_logic_vector (to_unsigned ((timeslot), 8)) ; --ram adr from 0..255 , timeslot from 0..255 (0=fifo ts)
              rx_taskram_we           <= '1';
              set_rx_err_ps(timeslot) <= '0';
              mil_rd_start            <= '1';                      --to bring hd6408 fsm back to idle
            ELSIF MIL_Rcv_Rdy = '1' AND Mil_Rcv_Error = '1' THEN   --this case handles hd6408 parity error
              rx_taskram_wr_d         <= x"beef";
              rx_taskram_wr_a         <= std_logic_vector (to_unsigned ((timeslot), 8)) ;
              rx_taskram_we           <= '1';
              set_rx_err_ps(timeslot) <= '1';                      --set rx_err bit
              mil_rd_start            <= '1';                      --to bring hd6408 fsm back to idle
            ELSIF timeout_cntr = timeout_cntr_max THEN             --this case handles telegram receive timeout
              rx_taskram_wr_d         <= x"babe";
              rx_taskram_wr_a         <= std_logic_vector (to_unsigned ((timeslot), 8)) ;
              rx_taskram_we           <= '1';
              set_rx_err_ps(timeslot) <= '1';
              mil_rd_start            <= '1';                       --if a telegram arrives long time later, there is no chance to bring hd6408 fsm back to
                                                                    --idle condition with behalf of mil_rd_start
            ELSE                                                    --this case should never be reached
              rx_taskram_wr_d         <= x"dead";
              rx_taskram_wr_a         <= std_logic_vector (to_unsigned ((timeslot), 8)) ;
              set_rx_err_ps(timeslot) <= '1';
              rx_taskram_we           <= '1';
            END IF;--MIL_Rcv_Rdy
          ELSE
            NULL;                                                  -- wait for timeout or rx data
          END IF;--End Case:timeout_cntr=55 or mil_rcv_rdy='1'

        END IF; -- End Case:mil_trm_rdy=1 or Task_runs=1


      ELSE
        NULL;
      END IF; -- tx_req and task_runs
  -----------------------------------------------------This ELSE should never be reached---------------------------------------------------------------------------
    ELSE
      null;
    END IF; -- all timeslots

  END IF;--clocked process
END PROCESS schedule_p;


-----------------------------------------------------------------------------------------------------

-- 254 tx_taskregs are implemented as sychronous ram
-- write is done by a single slave_i.wr pulse on access in ram address range
-- Avail/Err/Req Bits (0..254) are kept as registers due they need to be selective resetable (the sram isn't)

avail_muxer: PROCESS (avail,slave_i.adr)
VARIABLE LA_a_var    : UNSIGNED (slave_i_adr_max DOWNTO 2);
BEGIN
  LA_a_var             := UNSIGNED(slave_i.adr(slave_i_adr_max DOWNTO 2));

  IF     ( LA_a_var  >= rd_status_avail_first_adr) and (LA_a_var  <= rd_status_avail_last_adr ) THEN

    IF     LA_a_var = (rd_status_avail_first_adr      ) THEN avail_muxed <=      avail ( 15 DOWNTO   1) & '0';
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 1  ) THEN avail_muxed <=      avail ( 31 DOWNTO  16);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 2  ) THEN avail_muxed <=      avail ( 47 DOWNTO  32);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 3  ) THEN avail_muxed <=      avail ( 63 DOWNTO  48);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 4  ) THEN avail_muxed <=      avail ( 79 DOWNTO  64);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 5  ) THEN avail_muxed <=      avail ( 95 DOWNTO  80);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 6  ) THEN avail_muxed <=      avail (111 DOWNTO  96);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 7  ) THEN avail_muxed <=      avail (127 DOWNTO 112);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 8  ) THEN avail_muxed <=      avail (143 DOWNTO 128);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 9  ) THEN avail_muxed <=      avail (159 DOWNTO 144);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 10 ) THEN avail_muxed <=      avail (175 DOWNTO 160);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 11 ) THEN avail_muxed <=      avail (191 DOWNTO 176);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 12 ) THEN avail_muxed <=      avail (207 DOWNTO 192);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 13 ) THEN avail_muxed <=      avail (223 DOWNTO 208);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 14 ) THEN avail_muxed <=      avail (239 DOWNTO 224);
    ELSIF  LA_a_var = (rd_status_avail_first_adr + 15 ) THEN avail_muxed <= '0'& avail (254 DOWNTO 240);
    ELSE
      avail_muxed <= x"beef";	  -- other addresses out of range
    END IF;
  ELSE
    avail_muxed  <= x"dead";   -- other addresses out of range
  END IF;

END PROCESS avail_muxer;



rx_err_muxer: PROCESS (rx_err,slave_i.adr)
VARIABLE LA_a_var    : UNSIGNED (slave_i_adr_max DOWNTO 2);
BEGIN
  LA_a_var             := UNSIGNED(slave_i.adr(slave_i_adr_max DOWNTO 2));
  IF     ( LA_a_var  >= rd_rx_err_first_adr) and (LA_a_var  <= rd_rx_err_last_adr ) THEN

    IF     LA_a_var = (rd_rx_err_first_adr      ) THEN rx_err_muxed <=      rx_err( 15 DOWNTO   1) & '0';
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 1  ) THEN rx_err_muxed <=      rx_err( 31 DOWNTO  16);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 2  ) THEN rx_err_muxed <=      rx_err( 47 DOWNTO  32);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 3  ) THEN rx_err_muxed <=      rx_err( 63 DOWNTO  48);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 4  ) THEN rx_err_muxed <=      rx_err( 79 DOWNTO  64);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 5  ) THEN rx_err_muxed <=      rx_err( 95 DOWNTO  80);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 6  ) THEN rx_err_muxed <=      rx_err(111 DOWNTO  96);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 7  ) THEN rx_err_muxed <=      rx_err(127 DOWNTO 112);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 8  ) THEN rx_err_muxed <=      rx_err(143 DOWNTO 128);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 9  ) THEN rx_err_muxed <=      rx_err(159 DOWNTO 144);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 10 ) THEN rx_err_muxed <=      rx_err(175 DOWNTO 160);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 11 ) THEN rx_err_muxed <=      rx_err(191 DOWNTO 176);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 12 ) THEN rx_err_muxed <=      rx_err(207 DOWNTO 192);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 13 ) THEN rx_err_muxed <=      rx_err(223 DOWNTO 208);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 14 ) THEN rx_err_muxed <=      rx_err(239 DOWNTO 224);
    ELSIF  LA_a_var = (rd_rx_err_first_adr + 15 ) THEN rx_err_muxed <= '0'& rx_err(254 DOWNTO 240);
    ELSE
      rx_err_muxed <= x"beef";	  -- other addresses out of range
    END IF;
  ELSE
    rx_err_muxed  <= x"dead";     -- other addresses out of range
  END IF;

END PROCESS rx_err_muxer;



tx_req_muxer: PROCESS (tx_req,slave_i.adr)
VARIABLE LA_a_var    : UNSIGNED (slave_i_adr_max DOWNTO 2);
BEGIN
  LA_a_var             := UNSIGNED(slave_i.adr(slave_i_adr_max DOWNTO 2));
  IF     ( LA_a_var >= tx_ram_req_first_adr) and (LA_a_var  <= tx_ram_req_last_adr ) THEN

    IF     LA_a_var = (tx_ram_req_first_adr      ) THEN tx_req_muxed <=       tx_req( 15 DOWNTO   1) & '0';
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 1  ) THEN tx_req_muxed <=       tx_req( 31 DOWNTO  16);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 2  ) THEN tx_req_muxed <=       tx_req( 47 DOWNTO  32);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 3  ) THEN tx_req_muxed <=       tx_req( 63 DOWNTO  48);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 4  ) THEN tx_req_muxed <=       tx_req( 79 DOWNTO  64);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 5  ) THEN tx_req_muxed <=       tx_req( 95 DOWNTO  80);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 6  ) THEN tx_req_muxed <=       tx_req(111 DOWNTO  96);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 7  ) THEN tx_req_muxed <=       tx_req(127 DOWNTO 112);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 8  ) THEN tx_req_muxed <=       tx_req(143 DOWNTO 128);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 9  ) THEN tx_req_muxed <=       tx_req(159 DOWNTO 144);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 10 ) THEN tx_req_muxed <=       tx_req(175 DOWNTO 160);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 11 ) THEN tx_req_muxed <=       tx_req(191 DOWNTO 176);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 12 ) THEN tx_req_muxed <=       tx_req(207 DOWNTO 192);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 13 ) THEN tx_req_muxed <=       tx_req(223 DOWNTO 208);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 14 ) THEN tx_req_muxed <=       tx_req(239 DOWNTO 224);
    ELSIF  LA_a_var = (tx_ram_req_first_adr + 15 ) THEN tx_req_muxed <= '0' & tx_req(254 DOWNTO 240);
    ELSE
      tx_req_muxed <= x"beef";	  -- other addresses out of range
    END IF;
  ELSE
    tx_req_muxed  <= x"dead";     -- other addresses out of range
  END IF;

END PROCESS tx_req_muxer;
-----------------------------------------------------------------------------------------------------
large_or:PROCESS (tx_req,avail)
BEGIN

  IF tx_req = (tx_req'RANGE => '0') THEN
    tx_req_led <= '0';  --no req set, led keeps inactive
  ELSE
    tx_req_led <= '1';
  END IF;

  IF avail = (avail'RANGE => '0') THEN
    rx_avail_led <= '0'; --no avail set, led keeps inactive
  ELSE
    rx_avail_led <= '1';
  END IF;

END PROCESS large_or;


i_tx_req_led: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         => ena_led_count,   -- ENA (generated by same clock domain) is for one clock period active
    CLK         => clk_i,
    Sig_In      => tx_req_led,      -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero.
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => n_tx_req_led,    -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );

i_rx_avail_led: led_n
  GENERIC MAP (
    stretch_cnt => 4
    )
  PORT MAP (
    ena         => ena_led_count,   -- ENA (generated by same clock domain) is for one clock period active
    CLK         => clk_i,
    Sig_In      => rx_avail_led,    -- SigIn='1' holds "nLED" and "nLED_opdrn" on active zero.
                                    -- Outputs nLED and nLED_opdrn  are change to inactive state after "stretch_cnt" clock periodes.
    nLED        => n_rx_avail_led,  -- changed from opendrain to pushpull due to LED Selftest KK 20151015
    nLed_opdrn  => OPEN
    );


-----------------------------------------------------------------------------------------------------
-- Next Process controls WB Interface
-- Special Treatment for TX Fifo
--               SCU  write access is shortened to a single pulse action
--               Address bit 2 is signalling if Access is a Data or a CMD word
-- Special Treatment for TX_TaskRam
--               When written, the tx_req bit is set in the same moment
--               Write accesses to a cell, which has a tx_req bit already set, is not allowed
-- Special Treatment for RX_TaskRam
--               When read, the rx_err and rx_avail bits are cleared
-- Other Registers are implemented straight-forward
-- When Harris chip isn't operational, there maybe no mil piggy.
--      --> therefore no register accesses to MIL Option allowed due to hw6408_rdy_sync='0'
--      --> as a consequence, TX_FIFO will not get be filled, TX_TASKRAM wont be activated
--      --> when both are inactive, scheduler will run in idle mode.


p_regs_acc: PROCESS (clk_i, n_modulreset, slave_i)
VARIABLE LA_a_var          : UNSIGNED (slave_i_adr_max downto 2);
BEGIN
  LA_a_var                 := UNSIGNED(slave_i.adr(slave_i_adr_max downto 2));           -- variables evaluated at each clk edge

  IF n_modulreset = '0' THEN
    ex_stall          <= '1';
    ex_ack            <= '0';
    ex_err            <= '0';

    manchester_fpga   <= '0';
    ev_filt_12_8b     <= '0';
    ev_filt_on        <= '0';
    debounce_on       <= '1';
    puls2_frame       <= '0';
    puls1_frame       <= '0';
    ev_reset_on       <= '0';
    clr_mil_rcv_err   <= '1';


    rd_ev_fifo        <= '0';
    clr_ev_fifo       <= '1';
    wr_filt_ram       <= '0';
    rd_filt_ram       <= '0';
    sw_clr_ev_timer   <= '1';
    ld_dly_timer      <= '0';
    clr_wait_timer    <= '1';
    lemo_out_en       <= (others => '0');
    lemo_dat          <= (others => '0');
    lemo_i_reg        <= (others => '0');
    avail             <= (others => '0');
    rx_err            <= (others => '0');

    clr_rx_avail_ps   <= (others => '0');
    clr_rx_err_ps     <= (others => '0');
    tx_req            <= (others => '0');
    tx_taskram_we     <= '0';
    rx_taskram_re     <= '0';

  ELSIF RISING_EDGE(clk_i) THEN

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
    lemo_i_reg        <= lemo_inp;

    tx_fifo_write_en  <= '0';
    tx_taskram_we     <= '0';
    rx_taskram_re     <= '0';
    slave_o.dat       <= (others => '0');


    -- to clear selective request bits when tx_readout was done
    FOR I IN 1 TO 255 LOOP
      IF tx_task_ack(i)='1'  THEN tx_req(i) <= '0'; END IF;
    END LOOP;

    -- to clear selective data available bits when rx_readout was done
    FOR I IN 1 TO 255 LOOP
      IF clr_rx_avail_ps(i)='1'AND set_rx_avail_ps(i)='0' THEN avail(i)<='0';  ELSIF set_rx_avail_ps(i)='1' THEN avail(i)<='1';  ELSE NULL;END IF;
    END LOOP;

    FOR I IN 1 TO 255 LOOP
      IF clr_rx_err_ps(i)='1'  AND set_rx_err_ps(i)='0'   THEN rx_err(i)<='0'; ELSIF set_rx_err_ps(i)='1'   THEN rx_err(i)<='1'; ELSE NULL;END IF;
    END LOOP;


    clr_rx_avail_ps <= (OTHERS =>'0'); --pulses only set for 1 clk_i period
    clr_rx_err_ps   <= (others => '0');

    IF slave_i.cyc = '1' AND slave_i.stb = '1' AND ex_stall = '1'  THEN              -- begin of wishbone cycle

--##############################TX FIFO ACCESS#############################################
      IF (LA_a_var = mil_wr_cmd_a_map) OR  (LA_a_var = mil_rd_wr_data_a_map) THEN  -- fifo uses old cmd/data reg address
        IF slave_i.sel = "1111" THEN
          IF slave_i.we = '1' AND hw6408_rdy_sync = '1' THEN     -- only when hw6408 is operational, otherwise no  fifo write tasks allowed
              IF tx_fifo_full ='0' THEN
                tx_fifo_data_in(16)          <= slave_i.adr(2);  --Addressbit2='1' results in CMD Telegram due to x802 CMD Reg.Address
                tx_fifo_data_in(15 downto 0) <= slave_i.dat(15 downto 0);
                tx_fifo_write_en             <= slave_i.stb and not slave_i_stb_dly;
                ex_stall                     <= '0';
                ex_ack                       <= '1';
              ELSE
              -- write to mil not allowed, because tx_fifo is full
                ex_stall                     <= '0';
                ex_err                       <= '1';
              END IF;--tx_fifo_full
          ELSE
            -- read low word not allowed on TX_FIFO
              ex_stall                       <= '0';
              ex_err                         <= '1';
          END IF;--slave_i.we
        ELSE
          -- access to high word or unaligned word is not allowed
          ex_stall                           <= '0';
          ex_err                             <= '1';
        END IF;--slave_i.sel


--##############################TX_TASKRAM write access###################################################
      ELSIF (LA_a_var >= tx_taskram_first_adr and  LA_a_var <= tx_taskram_last_adr ) THEN
        IF slave_i.sel = "1111" THEN
          IF slave_i.we = '1' AND slave_i_we_del = '0'AND tx_req(to_integer(unsigned(slave_i.adr(9 downto 2))))='0' AND hw6408_rdy_sync = '1' THEN
            tx_taskram_wr_d                   <= slave_i.dat(15 DOWNTO 0);
            tx_taskram_wr_a                   <= slave_i.adr( 9 DOWNTO 2);                               --to address 1..xff in range tx_taskreg_first...last_a_var
            tx_taskram_we                     <= '1';
            tx_req(to_integer(unsigned(slave_i.adr(9 downto 2))))           <= '1';                      --Tx Requestbit 1..254 set here (cleared on readout)
            clr_rx_avail_ps(to_integer(unsigned(slave_i.adr(9 downto 2))))  <= '1';                      --kk 20180112 for PK to optimize etherbone accesses
            clr_rx_err_ps(to_integer(unsigned(slave_i.adr(9 downto 2))))    <= '1';                      --kk 20180112 for PK to optimize etherbone accesses
            ex_stall                          <= '0';
            ex_ack                            <= '1';
          ELSE
            ex_stall                          <= '0';                                                    --write on existing RAM Task isn't allowed
            ex_err                            <= '1';
          END IF;
        ELSE
          ex_stall                            <= '0';
          ex_err                              <= '1';
        END IF;


--##############################RX_TASKRAM############################################################
      ELSIF (LA_a_var >= rx_taskram_first_adr and  LA_a_var <= rx_taskram_last_adr ) THEN
        IF slave_i.sel = "1111" THEN

          IF slave_i.we = '0' THEN                                                                       --scu cycle needs 2 clocks for ram access
              IF slave_i_stb_dly2 ='0' THEN
                rx_taskram_re              <= '1';                                                       --first get data out of rx_taskram
                rx_taskram_rd_a            <= slave_i.adr( 9 DOWNTO 2);
              ELSE                                                                                       --second to present it to scu bus
                slave_o.dat (15 downto 0)  <= rx_taskram_rd_d;
                --clr_rx_avail_ps(to_integer(unsigned(slave_i.adr( 9 DOWNTO 2))))  <= '1';               --avail bit will be cleared by scu bus read access
                --clr_rx_err_ps  (to_integer(unsigned(slave_i.adr( 9 DOWNTO 2))))  <= '1';               --err bit will be cleared by scu bus read access
                ex_stall                   <= '0';                                                       --the 2 clrs were omitted due to performance optimisation
                ex_ack                     <= '1';                                                       --Etherbone may re-arange read tasks in same socket, may cause hazards here
              END IF;
          ELSE--write attempts result in DTACK Error
              ex_stall                   <= '0';
              ex_err                     <= '1';
          END IF;

        ELSE                                                                                            --highword/unaligned accesses (others than sel=1111) not allowed
           ex_stall                     <= '0';
           ex_err                       <= '1';
        END IF;--elsif


--##############################status_avail Regs######################################################
      ELSIF (LA_a_var >= rd_status_avail_first_adr AND LA_a_var <= rd_status_avail_last_adr) THEN
        IF slave_i.sel = "1111" THEN
          IF slave_i.we = '0' THEN
            slave_o.dat(15 DOWNTO 0) <= avail_muxed; -- Mux selects 16 bits out of avail Vector 255..1
            ex_stall                 <= '0';
            ex_ack                   <= '1';
          ELSE
            ex_stall                 <= '0';
            ex_err                   <= '1';
          END IF;
        ELSE
          ex_stall                   <= '0';
          ex_err                     <= '1';
        END IF;

--##############################rx_err Regs######################################################
      ELSIF (LA_a_var >= rd_rx_err_first_adr AND LA_a_var <= rd_rx_err_last_adr) THEN
        IF slave_i.sel = "1111" THEN
          IF slave_i.we = '0' THEN
            slave_o.dat(15 DOWNTO 0) <= rx_err_muxed; -- Mux selects 16 bits out of rx_err Vector 255..1
            ex_stall                 <= '0';
            ex_ack                   <= '1';
          ELSE
            ex_stall                 <= '0';
            ex_err                   <= '1';
          END IF;
        ELSE
          ex_stall                   <= '0';
          ex_err                     <= '1';
        END IF;

--##############################tx_req Regs######################################################
      ELSIF (LA_a_var >= tx_ram_req_first_adr AND LA_a_var <= tx_ram_req_last_adr) THEN
        IF slave_i.sel = "1111" THEN
          IF slave_i.we = '0' THEN
            slave_o.dat(15 DOWNTO 0) <= tx_req_muxed; -- Mux selects 16 bits out of tx_req Vector 255..1
            ex_stall                 <= '0';
            ex_ack                   <= '1';
          ELSE
            ex_stall                 <= '0';
            ex_err                   <= '1';
          END IF;
        ELSE
          ex_stall                   <= '0';
          ex_err                     <= '1';
        END IF;

--############################Regs from old MIL Macro)###############################################
       ELSIF (LA_a_var = mil_wr_rd_status_a_map)    THEN
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

      ELSIF (LA_a_var = mil_wr_rd_lemo_conf_a_map) THEN
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

       ELSIF (LA_a_var = mil_wr_rd_lemo_dat_a_map)  THEN
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



       ELSIF (LA_a_var = mil_rd_lemo_inp_a_map)     THEN
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

       ELSIF (LA_a_var = rd_clr_no_vw_cnt_a_map)    THEN
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

       ELSIF (LA_a_var = rd_wr_not_eq_cnt_a_map)    THEN
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

       ELSIF (LA_a_var = rd_clr_ev_fifo_a_map)      THEN
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

       ELSIF (LA_a_var = rd_clr_ev_timer_a_map)     THEN
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

       ELSIF (LA_a_var = rd_wr_dly_timer_a_map)     THEN
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

       ELSIF (LA_a_var = rd_clr_wait_timer_a_map)   THEN
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


       ELSIF (LA_a_var >= evt_filt_first_a)  AND   (LA_a_var <= evt_filt_last_a)  THEN -- read or write event filter ram
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


       ELSE
         ex_stall <= '0';
         ex_err <= '1';
       END IF; --LA_a_var

      end if;--slave_i.cyc
    end if;--clocked_process
END PROCESS p_regs_acc;



lemo_data_o(1)   <= io_1 when (lemo_event_en(1)='1') else lemo_dat(1);   -- To be compatible with former SCU solution
lemo_data_o(2)   <= io_2 when (lemo_event_en(2)='1') else lemo_dat(2);   -- which allows 2 event-driven lemo outputs
lemo_data_o(3)   <= lemo_dat(3);                                         -- This is used in SIO (not event drive-able)
lemo_data_o(4)   <= lemo_dat(4);                                         -- This is used in SIO (not event drive-able)

lemo_out_en_o(1) <= '1' when puls1_frame='1' else lemo_out_en(1);        -- To be compatible with former SCU solution
lemo_out_en_o(2) <= '1' when puls2_frame='1' else lemo_out_en(2);        -- which allows 2 event-driven lemo outputs
lemo_out_en_o(3) <= lemo_out_en(3);                                      -- This is used in SIO
lemo_out_en_o(4) <= lemo_out_en(4);                                      -- This is used in SIO



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

end arch_wb_mil_scu;
