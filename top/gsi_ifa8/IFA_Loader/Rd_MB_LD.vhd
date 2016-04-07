LIBRARY ieee;
USE ieee.std_logic_1164.all;


--  Entity Declaration

ENTITY Rd_MB_LD IS
	PORT
	(
		V_Data_Rd_Port		: IN STD_LOGIC_VECTOR(15 downto 0);
		Glob_Reg_Rd_Port	: IN STD_LOGIC_VECTOR(15 downto 0);
		Rd_Glob_Reg_Activ	: IN STD_LOGIC;
		Loader_Rd_Port		: IN STD_LOGIC_VECTOR(15 downto 0);
		Rd_Loader_Activ		: IN STD_LOGIC;
		I2C_Reg_Rd_Port		: IN STD_LOGIC_VECTOR(15 downto 0);
		Rd_I2C_Reg_Activ	: IN STD_LOGIC;		
		Data_Rd : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
	
END Rd_MB_LD;


ARCHITECTURE Arch_Rd_MB_LD OF Rd_MB_LD IS

SIGNAL	S_SEL : STD_LOGIC_VECTOR(3 DOWNTO 1);
	
BEGIN

S_SEL <= (Rd_I2C_Reg_Activ, Rd_Loader_Activ , Rd_Glob_Reg_Activ);

P_Rd_Mux:	PROCESS	(S_SEL, I2C_Reg_Rd_Port, Glob_Reg_Rd_Port, Loader_Rd_Port, V_Data_Rd_Port)
	BEGIN
		CASE S_SEL IS
			WHEN "000"	=> Data_Rd <= V_Data_Rd_Port;
			WHEN "001"	=> Data_Rd <= Glob_Reg_Rd_Port;
			WHEN "010"	=> Data_Rd <= Loader_Rd_Port;
			WHEN "100"	=> Data_Rd <= I2C_Reg_Rd_Port;
			WHEN OTHERS	=> Data_Rd <= V_Data_Rd_Port;
		END CASE;
	END PROCESS P_Rd_Mux;
	
END Arch_Rd_MB_LD;
