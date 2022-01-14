LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY global_reg IS
	GENERIC(
			Freq_1_in_Hz		: INTEGER := 250000000;
			Freq_2_in_Hz		: INTEGER := 50000000;
			RdWr_Echo_Reg_Adr	: INTEGER := 246;
			Rd_Freq_1_Adr		: INTEGER := 238;
			Rd_Freq_2_Adr		: INTEGER := 236;
			Test				: INTEGER := 0
			);
			
	PORT
	(
		Sub_Adr_La			: IN STD_LOGIC_VECTOR(7 downto 1);
		Data_In				: IN STD_LOGIC_VECTOR(15 downto 0);
		Extern_Wr_Activ		: IN STD_LOGIC;
		Extern_Wr_Fin		: IN STD_LOGIC;
		Extern_Rd_Activ		: IN STD_LOGIC;
		Extern_Rd_Fin		: IN STD_LOGIC;
		Powerup_Res			: IN STD_LOGIC;
		Clk					: IN STD_LOGIC;

		Glob_Reg_Rd_Port	: OUT STD_LOGIC_VECTOR(15 downto 0);
		Rd_Glob_Reg_Activ	: OUT STD_LOGIC;
		Dtack_Glob_Reg		: OUT STD_LOGIC
	);
	
END global_reg;



ARCHITECTURE arch_global_reg OF global_reg IS

	
	FUNCTION	prod_or_test  (production, test_data, test : INTEGER) RETURN INTEGER IS

		VARIABLE data : INTEGER;
			
		BEGIN
			IF Test = 1 THEN
				data := test_data;
			ELSE
				data := production;
			END IF;

			RETURN data;

		END prod_or_test;


	CONSTANT 	C_Freq_1_in_10kHz			: STD_LOGIC_VECTOR(15 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Freq_1_in_Hz / 10000, Glob_Reg_Rd_Port'LENGTH);
	CONSTANT 	C_Rd_Freq_1_in_10kHz		: STD_LOGIC_VECTOR(7 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Rd_Freq_1_Adr, 8);
	SIGNAL		S_Rd_Freq_1_in_10kHz		: STD_LOGIC;
	
	CONSTANT 	C_Freq_2_in_10kHz			: STD_LOGIC_VECTOR(15 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Freq_2_in_Hz / 10000, Glob_Reg_Rd_Port'LENGTH);
	CONSTANT 	C_Rd_Freq_2_in_10kHz		: STD_LOGIC_VECTOR(7 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Rd_Freq_2_Adr, 8);
	SIGNAL		S_Rd_Freq_2_in_10kHz		: STD_LOGIC;

	CONSTANT 	C_RdWr_Echo_Register 		: STD_LOGIC_VECTOR(7 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(RdWr_Echo_Reg_Adr, 8);
	SIGNAL		S_Rd_Echo					: STD_LOGIC;
	SIGNAL		S_Wr_Echo					: STD_LOGIC;	
	SIGNAL		S_Echo_Register				: STD_LOGIC_VECTOR(15 DOWNTO 0);
	
	SIGNAL		S_Rd_Dtack					: STD_LOGIC;
	SIGNAL		S_Wr_Dtack					: STD_LOGIC;
	SIGNAL		S_Dtack_Glob_Reg			: STD_LOGIC;


	SIGNAL		S_nRESET		: STD_LOGIC;
	
BEGIN

S_nRESET <= not Powerup_Res;

P_Global_Reg_Adr_Deco:	PROCESS (clk, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Rd_Freq_1_in_10kHz		<= '0';
			S_Rd_Freq_2_in_10kHz		<= '0';
			S_Rd_Echo					<= '0';
			S_Wr_Echo					<= '0';
			S_Rd_Dtack					<= '0';
			S_Wr_Dtack					<= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_Rd_Freq_1_in_10kHz 		<= '0';
			S_Rd_Freq_2_in_10kHz		<= '0';
			S_Rd_Echo					<= '0';
			S_Wr_Echo					<= '0';	
			S_Rd_Dtack					<= '0';
			S_Wr_Dtack					<= '0';
			CASE Sub_Adr_La IS
				WHEN C_Rd_Freq_1_in_10kHz(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' THEN
						S_Rd_Freq_1_in_10kHz	<= '1';
						S_Rd_Dtack				<= '1';
					END IF;
				WHEN C_Rd_Freq_2_in_10kHz(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' THEN
						S_Rd_Freq_2_in_10kHz	<= '1';
						S_Rd_Dtack				<= '1';
					END IF;
				WHEN C_RdWr_Echo_Register(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' THEN
						S_Rd_Echo				<= '1';
						S_Rd_Dtack				<= '1';
					END IF;
					IF Extern_Wr_Activ = '1' THEN
						S_Wr_Echo				<= '1';
						S_Wr_Dtack				<= '1';
					END IF;
				WHEN OTHERS =>
					S_Rd_Freq_1_in_10kHz 		<= '0';
					S_Rd_Freq_2_in_10kHz		<= '0';
					S_Rd_Echo					<= '0';
					S_Wr_Echo					<= '0';
					S_Rd_Dtack					<= '0';
					S_Wr_Dtack					<= '0';
			END CASE;
			IF S_Rd_Dtack = '1' or S_Wr_Dtack = '1' THEN
				S_Dtack_Glob_Reg <= '1';
			ELSE
				S_Dtack_Glob_Reg <= '0';
			END IF;
		END IF;
	END PROCESS P_Global_Reg_Adr_Deco;


P_Echo_Register:	PROCESS (CLK, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Echo_Register <= (OTHERS => '0');
		ELSIF clk'EVENT AND clk = '1' THEN
			IF S_Wr_Echo = '1' THEN
				S_Echo_Register <= Data_In;
			END IF;
		END IF;
	END PROCESS P_Echo_Register;
	

P_Glob_Reg_Rd_Mux:	PROCESS (S_Rd_Echo, S_Echo_Register, S_Rd_Freq_1_in_10kHz, S_Rd_Freq_2_in_10kHz)
	BEGIN
		IF S_Rd_Echo = '1' THEN
			Glob_Reg_Rd_Port <= S_Echo_Register;
		ELSIF S_Rd_Freq_1_in_10kHz = '1' THEN
			Glob_Reg_Rd_Port <= C_Freq_1_in_10kHz;
		ELSE
			Glob_Reg_Rd_Port <= C_Freq_2_in_10kHz;
		END IF;
	END PROCESS P_Glob_Reg_Rd_Mux;


Rd_Glob_Reg_Activ <= S_Rd_Dtack;

Dtack_Glob_Reg <= S_Dtack_Glob_Reg;

END arch_global_reg;
