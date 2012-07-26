LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Rd_Apk_ID IS
	GENERIC (
			ST_160_Pol		: INTEGER := 0
			);
	PORT
	(
		Sub_Adr_La			: IN STD_LOGIC_VECTOR(7 downto 1);
		Extern_Rd_Activ		: IN STD_LOGIC;
		Powerup_Res			: IN STD_LOGIC;
		Clk					: IN STD_LOGIC;
		K3_ID				: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		K2_ID				: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		K1_ID				: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		K0_ID				: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		Rd_Apk_ID_Port		: OUT STD_LOGIC_VECTOR(15 downto 0);
		Rd_Apk_ID_Activ		: OUT STD_LOGIC;
		Dtack_Apk_ID		: OUT STD_LOGIC
	);
	
END Rd_Apk_ID;



ARCHITECTURE arch_Rd_Apk_ID OF Rd_Apk_ID IS

	
	CONSTANT 	C_Rd_K0_ID 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"16";
	CONSTANT 	C_Rd_K1_ID 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"18";
	CONSTANT 	C_Rd_K2_ID 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"46";
	CONSTANT 	C_Rd_K3_ID 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"48";
	SIGNAL		S_Rd_K0_ID			: STD_LOGIC;
	SIGNAL		S_Rd_K1_ID			: STD_LOGIC;
	SIGNAL		S_Rd_K2_ID			: STD_LOGIC;
	SIGNAL		S_Rd_K3_ID			: STD_LOGIC;
	
	SIGNAL		S_Rd_Dtack			: STD_LOGIC;
	SIGNAL		S_Dtack_Apk_ID		: STD_LOGIC;

	SIGNAL		Sel					: std_logic_vector(3 DOWNTO 0);
	
	
BEGIN


P_Rd_Apk_ID_Adr_Deco:	PROCESS (clk, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Rd_K0_ID			<= '0';
			S_Rd_K1_ID			<= '0';
			S_Rd_K2_ID			<= '0';
			S_Rd_K3_ID			<= '0';
			S_Rd_Dtack			<= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_Rd_K0_ID			<= '0';
			S_Rd_K1_ID			<= '0';
			S_Rd_K2_ID			<= '0';
			S_Rd_K3_ID			<= '0';
			S_Rd_Dtack			<= '0';
			CASE Sub_Adr_La IS
				WHEN C_Rd_K0_ID(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' THEN
						S_Rd_K0_ID	<= '1';
						S_Rd_Dtack	<= '1';
					END IF;
				WHEN C_Rd_K1_ID(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' THEN
						S_Rd_K1_ID	<= '1';
						S_Rd_Dtack	<= '1';
					END IF;
				WHEN C_Rd_K2_ID(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' AND ST_160_Pol = 1 THEN
						S_Rd_K2_ID	<= '1';
						S_Rd_Dtack	<= '1';
					END IF;
				WHEN C_Rd_K3_ID(7 DOWNTO 1) =>
					IF Extern_Rd_Activ = '1' AND ST_160_Pol = 1  THEN
						S_Rd_K3_ID	<= '1';
						S_Rd_Dtack	<= '1';
					END IF;
				WHEN OTHERS =>
					S_Rd_K0_ID			<= '0';
					S_Rd_K1_ID			<= '0';
					S_Rd_K2_ID			<= '0';
					S_Rd_K3_ID			<= '0';
					S_Rd_Dtack			<= '0';
			END CASE;
			IF S_Rd_Dtack = '1' THEN
				S_Dtack_Apk_ID <= '1';
			ELSE
				S_Dtack_Apk_ID <= '0';
			END IF;
		END IF;
	END PROCESS P_Rd_Apk_ID_Adr_Deco;


--P_Apk_ID_Mux:	PROCESS (
--						S_Rd_K0_ID, K0_ID,
--						S_Rd_K1_ID, K1_ID,
--						S_Rd_K2_ID, K2_ID,
--						S_Rd_K3_ID, K3_ID
--						)
	
--	BEGIN
		Sel <= (S_Rd_K3_ID, S_Rd_K2_ID, S_Rd_K1_ID, S_Rd_K0_ID);
		
		WITH Sel SELECT
			Rd_Apk_ID_Port <=	K0_ID WHEN "0001",
								K1_ID WHEN "0010",
								K2_ID WHEN "0100",
								K3_ID WHEN "1000",
								K0_ID WHEN OTHERS;
--	END PROCESS P_Apk_ID_Mux;


Dtack_Apk_ID <= S_Dtack_Apk_ID;

Rd_Apk_ID_Activ <= S_Rd_Dtack OR S_Dtack_Apk_ID;


END arch_Rd_Apk_ID;
