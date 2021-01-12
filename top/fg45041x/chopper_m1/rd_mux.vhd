LIBRARY ieee;
USE ieee.std_logic_1164.all;


--  Entity Declaration

ENTITY Rd_Mux IS
	PORT
	(
		Rd_Chop_Out : IN STD_LOGIC_VECTOR(15 downto 0);
		Sel_Chop_Out : IN STD_LOGIC;

		Rd_Apk_ID_Port : IN STD_LOGIC_VECTOR(15 downto 0);
		Sel_Apk_ID : IN STD_LOGIC;

		Data_Rd : OUT STD_LOGIC_VECTOR(15 downto 0)
	);

END Rd_Mux;


ARCHITECTURE Rd_Mux_architecture OF Rd_Mux IS

SIGNAL	S_SEL : STD_LOGIC_VECTOR(2 DOWNTO 1);

BEGIN

S_SEL <= (Sel_Apk_ID, Sel_Chop_out);

P_Rd_Mux:	PROCESS	(S_SEL, Rd_Chop_Out, Rd_Apk_ID_Port)
	BEGIN
		CASE S_SEL IS
			WHEN "01" => Data_Rd <= Rd_Chop_Out;
			WHEN "10" => Data_Rd <= Rd_Apk_ID_Port;
			WHEN OTHERS => Data_Rd <= Rd_Chop_Out;
		END CASE;
	END PROCESS P_Rd_Mux;

END Rd_Mux_architecture;
