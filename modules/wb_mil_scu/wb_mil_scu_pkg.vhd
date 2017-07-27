library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;

library work;


package wb_mil_scu_pkg is

constant  c_mil_byte_addr_range:  integer := 16#2000# * 4;              -- all resources (byte, word, double word) are alligned to modulo 4 addresses,
                                                                        -- so multiply the c_mil_byte_addr_range by 4.
constant  c_mil_addr_width:       integer := integer(ceil(log2(real(c_mil_byte_addr_range))));

constant c_xwb_gsi_mil_scu : t_sdb_device := (
  abi_class     => x"0000",             -- undocumented device
  abi_ver_major => x"01",
  abi_ver_minor => x"00",
  wbd_endian    => c_sdb_endian_big,    -- '1' = little, '0' = big
  wbd_width     => x"4",                -- only 32-bit port granularity allowed
  sdb_component => (
  addr_first    => x"0000000000000000",
  addr_last     => std_logic_vector(to_unsigned(c_mil_byte_addr_range-1, t_sdb_component.addr_last'length)),
  --addr_last     => std_logic_vector(to_unsigned(16#1FFF#, 64)),
  product => (
  vendor_id     => x"0000000000000651", -- GSI
  device_id     => x"35aa6b96",
  version       => x"00000001",
  date          => x"20130826",
  name          => "GSI_MIL_SCU        ")));  -- should be 19 Char


constant  filter_data_width:  integer := 6;
constant  filter_ram_size:    integer := 4096;
constant  filter_addr_width:  integer := integer(ceil(log2(real(filter_ram_size))));

----------------------------------------------------------------------------------------------------------------------------------------
-- In Mil-Option  WB address offsets are multiplied by 4, eg mil_wr_cmd_a =1 gives real eb-tools address :wb_base plus 4              --
-- In DevBus Slave SIO3 offsets are multiplied by 2, eg mil_wr_cmd=1 gives real eb-tools address: sio_mil_first_reg_adr + 2 e.g. x802 --
----------------------------------------------------------------------------------------------------------------------------------------
constant  sio_mil_first_reg_a:      integer := 16#400#;  -- first register in SIO Addressing scheme, used for reg address remapping
constant  sio_mil_last_reg_a:       integer := 16#440#;  -- last register  in SIO Addressing scheme, used for reg address remapping

constant  mil_rd_wr_data_a:         integer := 16#00#;   -- read mil bus                 wb_mil_scu_offset + 16#00#, not used anymore, use task registers therefore.
                                                         -- write data to mil bus:       wb_mil_scu_offset + 16#00#, write for tx block mode fifo, bit[31..16] don't care.
constant  mil_wr_cmd_a:             integer := 16#01#;   -- write command to mil bus:    wb_mil_scu_offset + 16#04#, write for tx block mode fifo, bit[31..16] don't care.


constant  mil_wr_rd_status_a:       integer := 16#02#;   -- kk read mil status:          wb_mil_scu_offset + 16#08#, data[31..16] always zero.
                                                         -- write mil control reg:       wb_mil_scu_offset + 16#08#, bits 15..0 can be changed. Data[31..16] don't care.
constant  rd_clr_no_vw_cnt_a:       integer := 16#03#;   -- read no valid counters:      wb_mil_scu_offset + 16#0C#. Data[31..16] always zero.
                                                         -- write(clears)novalid counter wb_mil_scu_offset + 16#0C#. Data[31..0] don't care
constant  rd_wr_not_eq_cnt_a:       integer := 16#04#;   -- read not equal counters:     wb_mil_scu_offset + 16#10#. Data[31..16] always zero.
                                                         -- write (clears) not equal counters: wb_mil_scu_offset + 16#10#. Data[31..0] don't care.
constant  rd_clr_ev_fifo_a:         integer := 16#05#;   -- read event fifo:             wb_mil_scu_offset + 16#14#, only allowed when event fifo is not empty. Data[31..16] always zero.
                                                         -- write (clears) event fifo:   wb_mil_scu_offset + 16#14#. Data[31..0] don't care. 
constant  rd_clr_ev_timer_a:        integer := 16#06#;   -- read event timer:            wb_mil_scu_offset + 16#18#.
                                                         -- write (sw-clear) event timer wb_mil_scu_offset + 16#18#.
constant  rd_wr_dly_timer_a:        integer := 16#07#;   -- read delay timer:            wb_mil_scu_offset + 16#1C#.
                                                         -- write delay timer:           wb_mil_scu_offset + 16#1C#.
constant  rd_clr_wait_timer_a:      integer := 16#08#;   -- read wait timer:             wb_mil_scu_offset + 16#20#.
                                                         -- write (clear) wait timer:    wb_mil_scu_offset + 16#20#.
constant  mil_wr_rd_lemo_conf_a:    integer := 16#09#;   -- read mil lemo config:        wb_mil_scu_offset + 16#24#, data[31..4] always zero.
                                                         -- write mil lemo config:       wb_mil_scu_offset + 16#24#, bits 3..0 can be changed.Data[31..4] don't care
constant  mil_wr_rd_lemo_dat_a:     integer := 16#0A#;   -- read mil lemo dat:           wb_mil_scu_offset + 16#28#, data[31..4] always zero.
                                                         -- write mil lemo dat:          wb_mil_scu_offset + 16#28#, bits 3..0 can be changed.Data[31..4] don't car
constant  mil_rd_lemo_inp_a:        integer := 16#0B#;   -- read mil lemo inp:           wb_mil_scu_offset + 16#2C#, data[31..4] always zero.

constant  rd_ev_timer_LW_a:         integer := 16#0C#;   -- read event timer lower Word  wb_mil_scu_offset + 16#30#.
                                                         -- reserved                     wb_mil_scu_offset + 16#34#.
constant  rd_wait_timer_LW_a:       integer := 16#0e#;   -- read wait timer lower Word   wb_mil_scu_offset + 16#38#.
                                                         -- reserved                     wb_mil_scu_offset + 16#3c#..   
constant  rd_wr_dly_timer_LW_a:     integer := 16#10#;   -- read event timer latch LW    wb_mil_scu_offset + 16#40#.
                                                         -- write event timer latch LW   wb_mil_scu_offset + 16#40#.
constant  rd_wr_dly_timer_HW_a:     integer := 16#11#;   -- read event timer latch HW    wb_mil_scu_offset + 16#44#.
                                                         -- write event timer latch HW   wb_mil_scu_offset + 16#44#. 
                                                         
constant  wr_tx_taskreg0_a :        integer := 16#20#;   -- write tx_taskreg0            wb_mil_scu_offset + 16#80#  to write tx task function codes,   bit[31..16] don't care                                                     
constant  wr_tx_taskreg1_a :        integer := 16#21#;   -- write tx_taskreg1            wb_mil_scu_offset + 16#84#  
constant  wr_tx_taskreg2_a :        integer := 16#22#;   -- write tx_taskreg2            wb_mil_scu_offset + 16#88#  
constant  wr_tx_taskreg3_a :        integer := 16#23#;   -- write tx_taskreg3            wb_mil_scu_offset + 16#8C#  
constant  wr_tx_taskreg4_a :        integer := 16#24#;   -- write tx_taskreg4            wb_mil_scu_offset + 16#90#  
constant  wr_tx_taskreg5_a :        integer := 16#25#;   -- write tx_taskreg5            wb_mil_scu_offset + 16#94#  
constant  wr_tx_taskreg6_a :        integer := 16#26#;   -- write tx_taskreg6            wb_mil_scu_offset + 16#98#  
constant  wr_tx_taskreg7_a :        integer := 16#27#;   -- write tx_taskreg7            wb_mil_scu_offset + 16#9C# 

constant  rd_rx_taskreg0_a :        integer := 16#30#;   -- read rx_taskreg0             wb_mil_scu_offset + 16#80#  to read corresponding rx task word, bit[31..16] don't care                                                       
constant  rd_rx_taskreg1_a :        integer := 16#31#;   -- read rx_taskreg1             wb_mil_scu_offset + 16#84#  
constant  rd_rx_taskreg2_a :        integer := 16#32#;   -- read rx_taskreg2             wb_mil_scu_offset + 16#88#  
constant  rd_rx_taskreg3_a :        integer := 16#33#;   -- read rx_taskreg3             wb_mil_scu_offset + 16#8C#  
constant  rd_rx_taskreg4_a :        integer := 16#34#;   -- read rx_taskreg4             wb_mil_scu_offset + 16#90#  
constant  rd_rx_taskreg5_a :        integer := 16#35#;   -- read rx_taskreg5             wb_mil_scu_offset + 16#94#  
constant  rd_rx_taskreg6_a :        integer := 16#36#;   -- read rx_taskreg6             wb_mil_scu_offset + 16#98#  
constant  rd_rx_taskreg7_a :        integer := 16#37#;   -- read rx_taskreg7             wb_mil_scu_offset + 16#9C#  

constant  rd_status_avail_a :       integer := 16#40#;   -- read status_busy             wb_mil_scu_offset + 16#100# 
  

                                                         
                                                         
                                                         
                                                         

constant  ev_filt_first_a:          integer := 16#1000#;  -- first event filter ram address: wb_mil_scu_offset + 16#4000. 
constant  ev_filt_last_a:           integer := 16#1FFF#;  -- last event filter  ram address: wb_mil_scu_offset + 16#7FFC.


-- bit positions of mil control/status register
constant  b_sel_fpga_n6408: integer := 15;  -- '1' => fpga manchester endecoder selected, '0' => external hardware manchester endecoder 6408 selected.
constant  b_ev_filt_12_8b:  integer := 14;  -- '1' => event filter decode 12 bit of the event, '0' => event filter decode 8 bit of the event.
constant  b_ev_filt_on:     integer := 13;  -- '1' => event filter is on, '0' => event filter is off.
constant  b_debounce_on:    integer := 12;  -- '1' => debounce of device bus interrupt input is on.
constant  b_puls2_frame:    integer := 11;  -- '1' => aus zwei events wird der Rahmenpuls2 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_puls1_frame:    integer := 10;  -- '1' => aus zwei events wird der Rahmenpuls1 gebildet. Vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_ev_reset_on:    integer := 9;   -- '1' => events koennen den event timer auf Null setzen, vorausgesetzt das Eventfilter ist richtig programmiert.
constant  b_mil_rcv_err:    integer := 8;   -- '1' => an receive error okkurs. If this bit is '1', then it holds information
                                            --        until it's cleared by writing a one to this position of thencontrol register.
constant  b_mil_trm_rdy:    integer := 7;   -- '1' => ready to tranmit data or commands.
constant  b_mil_cmd_rcv:    integer := 6;   -- '1' => command received.
constant  b_mil_rcv_rdy:    integer := 5;   -- '1' => command or data received from mil bus.
constant  b_ev_fifo_full:   integer := 4;   -- '1' => event fifo is full.
constant  b_ev_fifo_ne:     integer := 3;   -- '1' => event fifo is not empty.
constant  b_data_req:       integer := 2;   -- '1' => data request interrupt of device bus is active.
constant  b_data_rdy:       integer := 1;   -- '1' => data ready interrupt of device bus is active.
constant  b_interlock:      integer := 0;   -- '1' => Interlock of device bus is active.


constant  b_lemo1_out_en:   integer := 0;   -- '1' => MIL Lemo 1 is enabled as output
constant  b_lemo2_out_en:   integer := 1;   -- '1' => MIL Lemo 2 is enabled as output
constant  b_lemo3_out_en:   integer := 2;   -- '1' => MIL Lemo 3 is enabled as output
constant  b_lemo4_out_en:   integer := 3;   -- '1' => MIL Lemo 4 is enabled as output

constant  b_lemo1_event_en: integer := 4;   -- '1' => MIL Lemo 1 is in Event driven mode
constant  b_lemo2_event_en: integer := 5;   -- '1' => MIL Lemo 2 is in Event driven mode
constant  b_lemo3_event_en: integer := 6;   -- '1' => MIL Lemo 3 is in Event driven mode
constant  b_lemo4_event_en: integer := 7;   -- '1' => MIL Lemo 4 is in Event driven mode

constant  b_lemo1_dat:      integer := 0;   -- '1' => MIL Lemo 1 output data (default='0')
constant  b_lemo2_dat:      integer := 1;   -- '1' => MIL Lemo 2 output data (default='0')
constant  b_lemo3_dat:      integer := 2;   -- '1' => MIL Lemo 3 output data (default='0')
constant  b_lemo4_dat:      integer := 3;   -- '1' => MIL Lemo 4 output data (default='0')

constant  b_lemo1_inp:      integer := 0;   -- '1' => MIL Lemo 1 (debounced) pin input status
constant  b_lemo2_inp:      integer := 1;   -- '1' => MIL Lemo 2 (debounced) pin input status
constant  b_lemo3_inp:      integer := 2;   -- '1' => MIL Lemo 3 (debounced) pin input status
constant  b_lemo4_inp:      integer := 3;   -- '1' => MIL Lemo 4 (debounced) pin input status


component wb_mil_scu IS

generic (
    Clk_in_Hz:  INTEGER := 125_000_000    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    );
port  (
    clk_i:              in      std_logic;
    nRst_i:             in      std_logic;
    slave_i:            in      t_wishbone_slave_in;
    slave_o:            out     t_wishbone_slave_out;
    
    -- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:            in      std_logic;      -- HD6408-output: transmit bipolar positive.
    nME_BZO:            in      std_logic;      -- HD6408-output: transmit bipolar negative.
    
    ME_SD:              in      std_logic;      -- HD6408-output: '1' => send data is active.
    ME_ESC:             in      std_logic;      -- HD6408-output: encoder shift clock for shifting data into the encoder. The
                                                --                encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:             out     std_logic;      -- HD6408-input:  serial data in accepts a serial data stream at a data rate
                                                --                equal to encoder shift clock.
    ME_EE:              out     std_logic;      -- HD6408-input:  a high on encoder enable initiates the encode cycle.
                                                --                (Subject to the preceding cycle being completed).
    ME_SS:              out     std_logic;      -- HD6408-input:  sync select actuates a Command sync for an input high
                                                --                and data sync for an input low.

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:             out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar one in when the bus is in its
                                                --                positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:             out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar zero in when the bus is in its
                                                --                negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:             out     std_logic;      -- HD6408-input:  With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                                --                transition finder circuit. If not used this input must be held low.
    ME_CDS:             in      std_logic;      -- HD6408-output: high occurs during output of decoded data which was preced
                                                --                by a command synchronizing character. Low indicares a data sync.
    ME_SDO:             in      std_logic;      -- HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC:             in      std_logic;      -- HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                                --                synchronized by the recovered serial data stream.
    ME_VW:              in      std_logic;      -- HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD:              in      std_logic;      -- HD6408-output: take data is high during receipt of data after identification
                                                --                of a sync pulse and two valid Manchester data bits

    -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
--    ME_12MHz:       out     std_logic;        -- HD6408-input:  is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
    

    Mil_BOI:            in      std_logic;      -- HD6408-input:  connect positive bipolar receiver, in FPGA directed to the external
                                                --                manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
                                                --                vhdl manchester macro.
    Mil_BZI:            in      std_logic;      -- HD6408-input:  connect negative bipolar receiver, in FPGA directed to the external
                                                --                manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
                                                --                vhdl manchester macro.
    Sel_Mil_Drv:        buffer  std_logic;      -- HD6408-output: active high, enable the external open collector driver to the transformer
    nSel_Mil_Rcv:       out     std_logic;      -- HD6408-output: active low, enable the external differtial receive circuit.
    Mil_nBOO:           out     std_logic;      -- HD6408-output: connect bipolar positive output to external open collector driver of
                                                --                the transformer. Source is the external manchester en/decoder HD6408 via
                                                --                nME_BOO or the internal FPGA vhdl manchester macro.
    Mil_nBZO:           out     std_logic;      -- HD6408-output: connect bipolar negative output to external open collector driver of
                                                --                the transformer. Source is the external manchester en/decoder HD6408 via
                                                --                nME_BZO or the internal FPGA vhdl manchester macro.
    nLed_Mil_Rcv:       out     std_logic;
    nLed_Mil_Trm:       out     std_logic;
    nLed_Mil_Err:       out     std_logic;
    error_limit_reached:out     std_logic;
    Mil_Decoder_Diag_p: out     std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n: out     std_logic_vector(15 downto 0);
    timing:             in      std_logic;
    nLed_Timing:        out     std_logic;
    dly_intr_o:         out     std_logic;
    nLed_Fifo_ne:       out     std_logic;
    ev_fifo_ne_intr_o:  out     std_logic;
    Interlock_Intr_i:   in      std_logic;
    Data_Rdy_Intr_i:    in      std_logic;
    Data_Req_Intr_i:    in      std_logic;
    Interlock_Intr_o:   out     std_logic;
    Data_Rdy_Intr_o:    out     std_logic;
    Data_Req_Intr_o:    out     std_logic;
    nLed_Interl:        out     std_logic;
    nLed_Dry:           out     std_logic;
    nLed_Drq:           out     std_logic;
    every_ms_intr_o:    out     std_logic;
    lemo_data_o:        out     std_logic_vector(4 downto 1);
    lemo_nled_o:        out     std_logic_vector(4 downto 1);
    lemo_out_en_o:      out     std_logic_vector(4 downto 1);  
    lemo_data_i:        in      std_logic_vector(4 downto 1):= (others => '0');
    nsig_wb_err:        out     std_logic                           -- '0' => gestretchte wishbone access Fehlermeldung
    );
end component wb_mil_scu;


component wb_mil_scu_v2 IS 
generic (
    Clk_in_Hz:  INTEGER := 125_000_000    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    );
port  (
    clk_i:              in      std_logic;
    nRst_i:             in      std_logic;
    slave_i:            in      t_wishbone_slave_in;
    slave_o:            out     t_wishbone_slave_out;
    
    -- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:            in      std_logic;      -- HD6408-output: transmit bipolar positive.
    nME_BZO:            in      std_logic;      -- HD6408-output: transmit bipolar negative.
    
    ME_SD:              in      std_logic;      -- HD6408-output: '1' => send data is active.
    ME_ESC:             in      std_logic;      -- HD6408-output: encoder shift clock for shifting data into the encoder. The
                                                --                encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:             out     std_logic;      -- HD6408-input:  serial data in accepts a serial data stream at a data rate
                                                --                equal to encoder shift clock.
    ME_EE:              out     std_logic;      -- HD6408-input:  a high on encoder enable initiates the encode cycle.
                                                --                (Subject to the preceding cycle being completed).
    ME_SS:              out     std_logic;      -- HD6408-input:  sync select actuates a Command sync for an input high
                                                --                and data sync for an input low.

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:             out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar one in when the bus is in its
                                                --                positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:             out     std_logic;      -- HD6408-input:  A high input should be applied to bipolar zero in when the bus is in its
                                                --                negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:             out     std_logic;      -- HD6408-input:  With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                                --                transition finder circuit. If not used this input must be held low.
    ME_CDS:             in      std_logic;      -- HD6408-output: high occurs during output of decoded data which was preced
                                                --                by a command synchronizing character. Low indicares a data sync.
    ME_SDO:             in      std_logic;      -- HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC:             in      std_logic;      -- HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                                --                synchronized by the recovered serial data stream.
    ME_VW:              in      std_logic;      -- HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD:              in      std_logic;      -- HD6408-output: take data is high during receipt of data after identification
                                                --                of a sync pulse and two valid Manchester data bits

    -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
--    ME_12MHz:       out     std_logic;        -- HD6408-input:  is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
    

    Mil_BOI:            in      std_logic;      -- connect positive bipolar receiver, in FPGA directed to the external
                                                -- manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
                                                -- vhdl manchester macro.
    Mil_BZI:            in      std_logic;      -- connect negative bipolar receiver, in FPGA directed to the external
                                                -- manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
                                                -- vhdl manchester macro.
    Sel_Mil_Drv:        out     std_logic;      -- HD6408-output: active high, enable the external open collector driver to the transformer
    nSel_Mil_Rcv:       out     std_logic;      -- HD6408-output: active low, enable the external differtial receive circuit.
    Mil_nBOO:           out     std_logic;      -- connect bipolar positive output to external open collector driver of
                                                -- the transformer. Source is the external manchester en/decoder HD6408 via
                                                -- nME_BOO or the internal FPGA vhdl manchester macro.
    Mil_nBZO:           out     std_logic;      -- connect bipolar negative output to external open collector driver of
                                                -- the transformer. Source is the external manchester en/decoder HD6408 via
                                                -- nME_BZO or the internal FPGA vhdl manchester macro.
    nLed_Mil_Rcv:       out     std_logic;
    nLed_Mil_Trm:       out     std_logic;
    nLed_Mil_Err:       out     std_logic;
    error_limit_reached:out     std_logic;
    Mil_Decoder_Diag_p: out     std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n: out     std_logic_vector(15 downto 0);
    timing:             in      std_logic;
    nLed_Timing:        out     std_logic;
    dly_intr_o:         out     std_logic;
    nLed_Fifo_ne:       out     std_logic;
    ev_fifo_ne_intr_o:  out     std_logic;
    Interlock_Intr_i:   in      std_logic;
    Data_Rdy_Intr_i:    in      std_logic;
    Data_Req_Intr_i:    in      std_logic;
    Interlock_Intr_o:   out     std_logic;
    Data_Rdy_Intr_o:    out     std_logic;
    Data_Req_Intr_o:    out     std_logic;
    nLed_Interl:        out     std_logic;
    nLed_Dry:           out     std_logic;
    nLed_Drq:           out     std_logic;
    every_ms_intr_o:    out     std_logic;
	 lemo_data_o:         out     std_logic_vector(4 downto 1);
    lemo_nled_o:        out     std_logic_vector(4 downto 1);
	 lemo_out_en_o:       out     std_logic_vector(4 downto 1);  
    lemo_data_i:        in      std_logic_vector(4 downto 1):= (others => '0');
    nsig_wb_err:        out     std_logic       -- '0' => gestretchte wishbone access Fehlermeldung

    );
end component wb_mil_scu_v2;

component event_processing is 
  generic (
    clk_in_hz:  INTEGER := 125_000_000    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    );
  port (
    ev_filt_12_8b:    in    std_logic;
    ev_filt_on:       in    std_logic;
    ev_reset_on:      in    std_logic;
    puls1_frame:      in    std_logic;
    puls2_frame:      in    std_logic;
    timing_i:         in    std_logic;
    clk_i:            in    std_logic;
    nRst_i:           in    std_logic;
    wr_filt_ram:      in    std_logic;
    rd_filt_ram:      in    std_logic;
    rd_ev_fifo:       in    std_logic;
    clr_ev_fifo:      in    std_logic;
    filt_addr:        in    std_logic_vector(filter_addr_width-1 downto 0);
    filt_data_i:      in    std_logic_vector(filter_data_width-1 downto 0);
    stall_o:          out   std_logic;
    read_port_o:      out   std_logic_vector(15 downto 0);
    ev_fifo_ne:       out   std_logic;
    ev_fifo_full:     out   std_logic;
    ev_timer_res:     out   std_logic;
    ev_puls1:         out   std_logic;
    ev_puls2:         out   std_logic;
    timing_received:  out   std_logic
    );
end component event_processing;

component mil_pll is
  port(
    inclk0:           in    std_logic;
    c0:               out   std_logic;
    locked:           out   std_logic);
end component mil_pll;

constant c_mil_irq_ctrl_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"9602eb72",
    version       => x"00000001",
    date          => x"20170131",
    name          => "IRQ_MASTER_CTRL    ")));

end package wb_mil_scu_pkg;
