library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;

library work;


package wb_mil_scu_pkg is

constant	c_mil_byte_addr_range:	integer := 16#1000# * 4;  						-- all resources (byte, word, double word) are alligned to modulo 4 addresses,
																																				-- so multiply the c_mil_byte_addr_range by 4.
constant	c_mil_addr_width:				integer	:= integer(ceil(log2(real(c_mil_byte_addr_range))));

constant c_xwb_gsi_mil_scu : t_sdb_device := (
	abi_class     => x"0000", -- undocumented device
	abi_ver_major => x"01",
	abi_ver_minor => x"00",
	wbd_endian    => c_sdb_endian_little,
	wbd_width     => x"7", -- 8/16/32-bit port granularity
	sdb_component => (
	addr_first    => x"0000000000000000",
	addr_last     => std_logic_vector(to_unsigned(c_mil_byte_addr_range-1, t_sdb_component.addr_last'length)),
	product => (
	vendor_id     => x"0000000000000651", -- GSI
	device_id     => x"35aa6b96",
	version       => x"00000001",
	date          => x"20130826",
	name          => "GSI_MIL_SCU        ")));	-- should be 19 Char

component wb_mil_scu IS 
generic (
		Clk_in_Hz:	INTEGER := 125_000_000		-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
																					-- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
		);
port	(
		clk_i:      		in		std_logic;
		nRst_i:     		in		std_logic;
		slave_i:    		in		t_wishbone_slave_in;
		slave_o:    		out		t_wishbone_slave_out;
		
		-- encoder (transmiter) signals of HD6408 --------------------------------------------------------------------------------
		nME_BOO:				in			std_logic;			-- HD6408-output:	transmit bipolar positive.
		nME_BZO:				in			std_logic;			-- HD6408-output:	transmit bipolar negative.
		
		ME_SD:					in			std_logic;			-- HD6408-output:	'1' => send data is active.
		ME_ESC:					in			std_logic;			-- HD6408-output:	encoder shift clock for shifting data into the encoder. The
																						--								encoder samples ME_SDI on low-to-high transition of ME_ESC.
		ME_SDI:					out			std_logic;			-- HD6408-input:	serial data in accepts a serial data stream at a data rate
																						--								equal to encoder shift clock.
		ME_EE:					out			std_logic;			-- HD6408-input:	a high on encoder enable initiates the encode cycle.
																						--								(Subject to the preceding cycle being completed).
		ME_SS:					out			std_logic;			-- HD6408-input:	sync select actuates a Command sync for an input high
																						--								and data sync for an input low.

		-- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
		ME_BOI:					out			std_logic;			-- HD6408-input:	A high input should be applied to bipolar one in when the bus is in its
																						--								positive state, this pin must be held low when the Unipolar input is used.
		ME_BZI:					out			std_logic;			-- HD6408-input:	A high input should be applied to bipolar zero in when the bus is in its
																						--								negative state. This pin must be held high when the Unipolar input is used.
		ME_UDI:					out			std_logic;			-- HD6408-input:	With ME_BZI high and ME_BOI low, this pin enters unipolar data in to the
																						--								transition finder circuit. If not used this input must be held low.
		ME_CDS:					in			std_logic;			-- HD6408-output:	high occurs during output of decoded data which was preced
																						--								by a command synchronizing character. Low indicares a data sync.
		ME_SDO:					in			std_logic;			-- HD6408-output:	serial data out delivers received data in correct NRZ format.
		ME_DSC:					in			std_logic;			-- HD6408-output:	decoder shift clock delivers a frequency (decoder clock : 12),
																						--								synchronized by the recovered serial data stream.
		ME_VW:					in			std_logic;			-- HD6408-output:	high indicates receipt of a VALID WORD.
		ME_TD:					in			std_logic;			-- HD6408-output:	take data is high during receipt of data after identification
																						--								of a sync pulse and two valid Manchester data bits

		-- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
--		ME_12MHz:				out			std_logic;			-- HD6408-input:	is connected on layout to ME_DC (decoder clock) and ME_EC (encoder clock)
		

		Mil_BOI:				out			std_logic;			-- HD6408-input:	connect positive bipolar receiver, in FPGA directed to the external
																						--								manchester en/decoder HD6408 via output ME_BOI or to the internal FPGA
																						--								vhdl manchester macro.
		Mil_BZI:				out			std_logic;			-- HD6408-input:	connect negative bipolar receiver, in FPGA directed to the external
																						--								manchester en/decoder HD6408 via output ME_BZI or to the internal FPGA
																						--								vhdl manchester macro.
		Sel_Mil_Drv:		out			std_logic;			-- HD6408-output:	active high, enable the external open collector driver to the transformer
		nSel_Mil_Rcv:		out			std_logic;			-- HD6408-output:	active low, enable the external differtial receive circuit.
		Mil_nBOO:				out			std_logic;			-- HD6408-output:	connect bipolar positive output to external open collector driver of
																						--								the transformer. Source is the external manchester en/decoder HD6408 via
																						--								nME_BOO or the internal FPGA vhdl manchester macro.
		Mil_nBZO:				out			std_logic;			-- HD6408-output:	connect bipolar negative output to external open collector driver of
																						--								the transformer. Source is the external manchester en/decoder HD6408 via
																						--								nME_BZO or the internal FPGA vhdl manchester macro.
		nLed_Mil_Rcv:		out			std_logic;
		nLed_Mil_Trm:		out			std_logic;
		nLed_Mil_Err:		out			std_logic;
		error_limit_reached:	out		std_logic;
		Mil_Decoder_Diag_p:	out		std_logic_vector(15 downto 0);
		Mil_Decoder_Diag_n:	out		std_logic_vector(15 downto 0);
		timing:					in			std_logic;
		nLed_Timing:		out			std_logic;
		Interlock_Intr:	in			std_logic;
		Data_Rdy_Intr:	in			std_logic;
		Data_Req_Intr:	in			std_logic;
		nLed_Interl:		out			std_logic;
		nLed_Dry:				out			std_logic;
		nLed_Drq:				out			std_logic
		);
end component wb_mil_scu;

end package wb_mil_scu_pkg;