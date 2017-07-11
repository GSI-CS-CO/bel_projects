library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

library work;

package scu_sio3_pkg is

component wb_mil_wrapper_sio is 
generic (
		Clk_in_Hz:		INTEGER := 125_000_000;		-- Manchester IP needs 20 Mhz clock for proper detection of short 500ns data pulses
    sio_mil_first_reg_a:    unsigned(15 downto 0)  := x"0400";
    sio_mil_last_reg_a:     unsigned(15 downto 0)  := x"0411";
    evt_filt_first_a:       unsigned(15 downto 0)  := x"1000";
    evt_filt_last_a:        unsigned(15 downto 0)  := x"1FFF"
		);
port	(
		Adr_from_SCUB_LA: 		in			std_logic_vector(15 downto 0);
		Data_from_SCUB_LA:		in			std_logic_vector(15 downto 0);
		Ext_Adr_Val:					in			std_logic;									
		Ext_Rd_active:				in			std_logic;									
		Ext_Rd_fin:						in			std_logic;											-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:				in			std_logic;										
		Ext_Wr_fin:						in			std_logic;											-- marks end of write cycle, active one for one clock period of sys_clk
		clk:									in			std_logic;											-- should be the same clk, used by SCU_Bus_Slave
		Data_to_SCUB:					out			std_logic_vector(15 downto 0);
		Data_for_SCUB:				out			std_logic;
		Dtack_to_SCUB:				out			std_logic;

		nME_BZO:							in			std_logic;
		nME_BOO:							in			std_logic;
		Reset_Puls:						in			std_logic;
		ME_SD:								in			std_logic;
		ME_ESC:								in			std_logic;
		ME_CDS:								in			std_logic;
		ME_SDO:								in			std_logic;
		ME_DSC:								in			std_logic;
		ME_VW:								in			std_logic;
		ME_TD:								in			std_logic;
		ME_SDI:								out			std_logic;
		ME_SS:								out			std_logic;
		ME_EE:								out			std_logic;
		Mil_In_Pos:			 			in			std_logic;											--A_Mil1_BOI
		Mil_In_Neg:			 			in			std_logic;											--A_Mil1_BZI
		ME_BOI:								out			std_logic;	
		ME_BZI:								out			std_logic;	
		nSel_Mil_Drv:				 	out			std_logic;											--A_MIL1_OUT_En = not nSEl_Mil_Drv
		nSel_Mil_Rcv:		 			out			std_logic;											--A_Mil1_nIN_En
		nMil_Out_Pos:		 			out			std_logic;											--A_Mil1_nBZO
		nMil_Out_Neg:		 			out			std_logic;											--A_Mil1_nBOO
		nLed_Mil_Rcv:		 			out   	std_logic;											--For Led and TestPort
		
		nLed_Mil_Trm:					out			std_logic;
		nLED_Mil_Rcv_Error:		out	    std_logic;											--For Led and TestPort
		error_limit_reached:	out			std_logic;											--not used
		Mil_Decoder_Diag_p:	 	out			std_logic_vector(15 downto 0);	--EPLD-Manchester-Decoders Diagnose: des Positiven Signalpfades
		Mil_Decoder_Diag_n:	 	out			std_logic_vector(15 downto 0);	--EPLD-Manchester-Decoders Diagnose: des Negativen Signalpfades

		timing:								in			std_logic;
		nLed_Timing:					out			std_logic;
		dly_intr_o:						out			std_logic;
		nLed_Fifo_ne:					out			std_logic;
		ev_fifo_ne_intr_o:		out			std_logic;
		Interlock_Intr_i:			in			std_logic;
		Data_Rdy_Intr_i:			in			std_logic;
		Data_Req_Intr_i:			in			std_logic;
		Interlock_Intr_o:			out			std_logic;
		Data_Rdy_Intr_o:			out			std_logic;
		Data_Req_Intr_o:			out			std_logic;
		nLed_Interl:					out			std_logic;
		nLed_Dry:							out			std_logic;
		nLed_Drq:							out			std_logic;
		every_ms_intr_o:			out			std_logic;
					-- lemo I/F
		lemo_data_o:					out			std_logic_vector(4 downto 1);
		lemo_nled_o:					out			std_logic_vector(4 downto 1);
		lemo_out_en_o:				out			std_logic_vector(4 downto 1);
		lemo_data_i:				  in			std_logic_vector(4 downto 1)
		);
end component;

component wb_mil_sio IS 
generic (
    Clk_in_Hz:               INTEGER := 125_000_000; -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                                     -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    sio_mil_first_reg_a:    unsigned(15 downto 0)  := x"0400";
    sio_mil_last_reg_a:     unsigned(15 downto 0)  := x"0411";
    evt_filt_first_a:       unsigned(15 downto 0)  := x"1000";
    evt_filt_last_a:        unsigned(15 downto 0)  := x"1FFF"
    );
port    (
    clk_i:                in  std_logic;
    nRst_i:               in  std_logic;
    slave_i:              in  t_wishbone_slave_in;
    slave_o:              out t_wishbone_slave_out;
    
    -- encoder (transmitter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO:              in  std_logic;-- HD6408-output: transmit bipolar positive.
    nME_BZO:              in  std_logic;-- HD6408-output: transmit bipolar negative.
    ME_SD:                in  std_logic;-- HD6408-output: '1' => send data is active.
    ME_ESC:               in  std_logic;-- HD6408-output: encoder shift clock for shifting data into the encoder. The
                                        --                encoder samples ME_SDI on low-to-high transition of ME_ESC.
    ME_SDI:               out std_logic;-- HD6408-input:  serial data in accepts a serial data stream at a data rate
                                        --                equal to encoder shift clock.
    ME_EE:                out std_logic;-- HD6408-input:  a high on encoder enable initiates the encode cycle.
                                        --                (Subject to the preceding cycle being completed).
    ME_SS:                out std_logic;-- HD6408-input:  sync select actuates a Command sync for an input high
                                        --                and data sync for an input low.

    -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI:               out std_logic;-- HD6408-input:  A high input should be applied to bipolar one in when the bus is in its
                                        --                positive state, this pin must be held low when the Unipolar input is used.
    ME_BZI:               out std_logic;-- HD6408-input:  A high input should be applied to bipolar zero in when the bus is in its
                                        --                negative state. This pin must be held high when the Unipolar input is used.
    ME_UDI:               out std_logic;-- HD6408-input:  With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
                                        --                transition finder circuit. If not used this input must be held low.
    ME_CDS:               in  std_logic;-- HD6408-output: high occurs during output of decoded data which was preced
                                        --                by a command synchronizing character. Low indicares a data sync.
    ME_SDO:               in  std_logic;-- HD6408-output: serial data out delivers received data in correct NRZ format.
    ME_DSC:               in  std_logic;-- HD6408-output: decoder shift clock delivers a frequency (decoder clock : 12),
                                        --                synchronized by the recovered serial data stream.
    ME_VW:                in  std_logic;-- HD6408-output: high indicates receipt of a VALID WORD.
    ME_TD:                in  std_logic;-- HD6408-output: take data is high during receipt of data after identification
                                        --                of a sync pulse and two valid Manchester data bits

    -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
--  ME_12MHz:             out std_logic;-- HD6408-input:  is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
    

    Mil_BOI:              in  std_logic;-- connect positive bipolar receiver, in FPGA directed to the external
                                        -- manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
                                        -- vhdl manchester macro.
    Mil_BZI:              in  std_logic;-- connect negative bipolar receiver, in FPGA directed to the external
                                        -- manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA manchester coder
    Sel_Mil_Drv:          out std_logic; --HD6408-output: active high, enable the external open collector driver to the transformer
    nSel_Mil_Rcv:         out std_logic;-- HD6408-output: active low, enable the external differtial receive circuit.
    Mil_nBOO:             out std_logic;-- connect bipolar positive output to external open collector driver of
                                        -- the transformer. Source is the external manchester en/decoder HD6408 via
                                        -- nME_BOO or the internal FPGA vhdl manchester macro.
    Mil_nBZO:             out std_logic;-- connect bipolar negative output to external open collector driver of
                                        -- the transformer. Source is the external manchester en/decoder HD6408 via
                                        -- nME_BZO or the internal FPGA vhdl manchester macro.
    nLed_Mil_Rcv:         out std_logic;
    nLed_Mil_Trm:         out std_logic;
    nLed_Mil_Err:         out std_logic;
    error_limit_reached:  out std_logic;
    Mil_Decoder_Diag_p:   out std_logic_vector(15 downto 0);
    Mil_Decoder_Diag_n:   out std_logic_vector(15 downto 0);
    timing:               in  std_logic;
    nLed_Timing:          out std_logic;
    dly_intr_o:           out std_logic;
    nLed_Fifo_ne:         out std_logic;
    ev_fifo_ne_intr_o:    out std_logic;
    Interlock_Intr_i:     in  std_logic;
    Data_Rdy_Intr_i:      in  std_logic;
    Data_Req_Intr_i:      in  std_logic;
    Interlock_Intr_o:     out std_logic;
    Data_Rdy_Intr_o:      out std_logic;
    Data_Req_Intr_o:      out std_logic;
    nLed_Interl:          out std_logic;
    nLed_Dry:             out std_logic;
    nLed_Drq:             out std_logic;
    every_ms_intr_o:      out std_logic;
    lemo_data_o:          out std_logic_vector(4 downto 1);
    lemo_nled_o:          out std_logic_vector(4 downto 1); 
    lemo_out_en_o:        out std_logic_vector(4 downto 1);  
    lemo_data_i:          in  std_logic_vector(4 downto 1):= (others => '0');
    nsig_wb_err:          out std_logic --'0' => gestretchte wishbone access Fehlermeldung

    );
end component wb_mil_sio;


component flash_loader_v01
	PORT
	(
		noe_in		: IN STD_LOGIC 
	);
END component flash_loader_v01;


component pll_sio
	port(
		inclk0:		in			std_logic;
		c0:				out			std_logic;
		c1:				out			std_logic;
		locked:		out			std_logic
	);
end component;

component mil_pll
	PORT(
		inclk0:		IN			std_logic	:= '0';
		c0:				OUT			std_logic ;
		locked:		OUT			std_logic 
	);
end component;


component SysClock
	port(
		inclk0:		in			std_logic := '0';
		c0:				out			std_logic;
		c1:				out			std_logic;
		locked:		out			std_logic 
	);
end component;


component pu_reset
	generic(
		PU_Reset_in_clks : INTEGER
	);
	port	(
		Clk:			in			std_logic;
		PU_Res:		out			std_logic
	);
end component;



component generic_sync_fifo
    generic (
      g_data_width             : natural;
      g_size                   : natural;
      g_show_ahead             : boolean := false;
      g_with_empty             : boolean := true;
      g_with_full              : boolean := true;
      g_with_almost_empty      : boolean := false;
      g_with_almost_full       : boolean := false;
      g_with_count             : boolean := false;
      g_almost_empty_threshold : integer := 0;
      g_almost_full_threshold  : integer := 0);
    port (
      rst_n_i        : in  std_logic := '1';
      clk_i          : in  std_logic;
      d_i            : in  std_logic_vector(g_data_width-1 downto 0);
      we_i           : in  std_logic;
      q_o            : out std_logic_vector(g_data_width-1 downto 0);
      rd_i           : in  std_logic;
      empty_o        : out std_logic;
      full_o         : out std_logic;
      almost_empty_o : out std_logic;
      almost_full_o  : out std_logic;
      count_o        : out std_logic_vector(f_log2_size(g_size)-1 downto 0));
  end component;



end package scu_sio3_pkg;
