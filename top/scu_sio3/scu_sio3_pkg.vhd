library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;

package scu_sio3_pkg is

constant	SCU_SIO2_ID:		INTEGER range 16#0200# to 16#020F# := 16#0200#;


component sio3_Test_User_Reg
	generic
		(
		Base_addr:	INTEGER
		);
	port(
		Adr_from_SCUB_LA: 	in		std_logic_vector(15 downto 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val:				in		std_logic;												-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in		std_logic;												-- '1' => Rd-Cycle is active
		Ext_Rd_fin:					in		std_logic;												-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in		std_logic;												-- '1' => Wr-Cycle is active
		Ext_Wr_fin:					in		std_logic;												-- marks end of write cycle, active one for one clock period of sys_clk
		clk:								in		std_logic;												-- should be the same clk, used by SCU_Bus_Slave
		nReset:							in		std_logic;
		User1_Reg:					out		std_logic_vector(15 downto 0);		-- Daten-Reg. User1
		User2_Reg:					out		std_logic_vector(15 downto 0);		-- Daten-Reg. User2
		User_Reg_rd_active:	out		std_logic;												-- read data available at 'Data_to_SCUB'-output
		Data_to_SCUB:				out		std_logic_vector(15 downto 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:			out		std_logic													-- connect Dtack to SCUB-Macro
		);	
end component sio3_Test_User_Reg;



component wb_mil_wrapper is 
generic (
		Clk_in_Hz:		INTEGER := 125_000_000;		-- Manchester IP needs 20 Mhz clock for proper detection of short 500ns data pulses
																						-- Generic "Mil_clk_in_Hz"	"Baudrate" des Manchester-Ein-/Ausgangsdatenstroms umgepolt.
		Base_Addr:		INTEGER := 16#400#
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
		Mil_Trm_Rdy:					buffer	std_logic;											--For Led and TestPort
		nSel_Mil_Drv:				 	out			std_logic;											--A_MIL1_OUT_En = not nSEl_Mil_Drv
		nSel_Mil_Rcv:		 			out			std_logic;											--A_Mil1_nIN_En
		nMil_Out_Pos:		 			out			std_logic;											--A_Mil1_nBZO
		nMil_Out_Neg:		 			out			std_logic;											--A_Mil1_nBOO
		Mil_Rcv_Rdy:		 			buffer	std_logic;											--For Led and TestPort
		
		nLed_Mil_Trm:					out			std_logic;
		nLED_Mil_Rcv_Error:		buffer	std_logic;											--For Led and TestPort
		error_limit_reached:	out			std_logic;											--not used
		No_VW_Cnt:						buffer	std_logic_vector(15 downto 0);	--EPLD-Manchester-Decoders Diagnose: Bit[15..8] Fehlerzähler No Valid Word des positiven Decoders "No_VW_p", 
																																	--																	 Bit[7..0]	Fehlerzähler No Valid Word des negativen Decoders "No_VM_n"
		Not_Equal_Cnt:		 		buffer	std_logic_vector(15 downto 0);	--EPLD-Manchester-Decoders Diagnose: Bit[15..8] Fehlerzähler Data_not_equal, 
																																	--																	 Bit[7..0]	Fehlerzähler unterschiedliche CMD-Data-Kennung (CMD_not_equal).
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

component flash_loader_v01
	port(
		noe_in:		in		std_logic
	);
end component;


component led_n
	generic(
		stretch_cnt:			INTEGER := 3
	); 
	port(
			ena:		in			std_logic := '1';
			CLK:		in			std_logic;
			Sig_In:	in			std_logic;
			nLED:		out			std_logic
	);
END component led_n;


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

component zeitbasis
	generic	(
		CLK_in_Hz:				INTEGER;
		diag_on:					INTEGER
		);
	port(
		Res:							in		std_logic;
		Clk:							in		std_logic;
		Ena_every_100ns:	out		std_logic;
		Ena_every_166ns:	out		std_logic;
		Ena_every_250ns:	out		std_logic;
		Ena_every_500ns:	out		std_logic;
		Ena_every_1us:		out		std_logic;
		Ena_Every_20ms:		out		std_logic
	);
end component;

end package scu_sio3_pkg;
