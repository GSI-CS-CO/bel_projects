LIBRARY ieee;
USE ieee.std_logic_1164.all; 
USE IEEE.STD_LOGIC_unsigned.all;


ENTITY Kanal IS 
	port
	(
		nPort_Wr		: IN		STD_LOGIC;
		ID_OK 			: IN		STD_LOGIC;
		ID_Cntrl_Done	: IN		STD_LOGIC;
		TO_Port			: IN		STD_LOGIC_VECTOR(15 downto 0) := "0000000000000000";
		Wr_Strobe		: IN		STD_LOGIC;
		Port_In_La		: IN		STD_LOGIC;
		clk				: IN		STD_LOGIC;
		Rd_Strobe		: OUT		STD_LOGIC;
		Port_IO			: INOUT	STD_LOGIC_VECTOR(15 downto 0);
		From_Port		: OUT	STD_LOGIC_VECTOR(15 downto 0);
		nP_CTRL			: INOUT	STD_LOGIC_VECTOR(2 downto 1)
	);
END Kanal;



ARCHITECTURE Arch_Kanal OF Kanal IS

SIGNAL	S_Port_In_La	: STD_LOGIC_VECTOR(15 downto 0);

BEGIN 

P_CTRL: PROCESS (nPort_Wr, ID_OK, TO_Port, Wr_Strobe, nP_CTRL)
	BEGIN
		IF ID_OK = '0' THEN
			nP_CTRL	<= "ZZ";
		ELSE
			IF nPort_Wr = '0' THEN
				nP_CTRL(1) <= Wr_Strobe;
				nP_CTRL(2) <= 'Z';
				IF nP_CTRL(2) = '1' THEN
					Rd_Strobe <= '1';
				ELSE
					Rd_Strobe <= '0';
				END IF;
--				Rd_Strobe <= nP_CTRL(2);
			ELSE
				nP_CTRL(2) <= Wr_Strobe;
				nP_CTRL(1) <= 'Z';
				IF nP_CTRL(1) = '1' THEN
					Rd_Strobe <= '1';
				ELSE
					Rd_Strobe <= '0';
				END IF;
--				Rd_Strobe <= nP_CTRL(1);
			END IF;
		END IF;
	END PROCESS P_CTRL;

P_Port_Tri: PROCESS (nPort_Wr, ID_OK, ID_Cntrl_Done, To_Port)
	BEGIN
		IF ID_OK = '0' OR ID_Cntrl_Done = '0' THEN
			Port_IO <= (OTHERS => 'Z');
		ELSE
			IF nPort_Wr = '0' THEN
				Port_IO <= TO_Port;
			ELSE
				Port_IO <= (OTHERS => 'Z');
			END IF;
		END IF;
	END PROCESS P_Port_Tri;
	
P_Port_In_La: PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF Port_In_La = '0' THEN
				S_Port_In_La <= Port_IO;
			END IF;
		END IF;
	END PROCESS P_Port_In_La;
	

From_Port <= S_Port_In_La;

END; 