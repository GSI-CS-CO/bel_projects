--TITLE "'Test_User_Reg' Autor: W.Panschow, Stand: 25.09.08, Vers: V01 ";
--
library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.numeric_std.all;

ENTITY Test_User_Reg IS
	GENERIC
		(
		Base_addr:	INTEGER := 16#0200#
		);
		
	PORT(
		Adr_from_SCUB_LA	: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA	: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val			: IN	STD_LOGIC;							-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active		: in	std_logic;							-- '1' => Rd-Cycle is active
		Ext_Rd_fin			: in	std_logic;							-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active		: in	std_logic;							-- '1' => Wr-Cycle is active
		Ext_Wr_fin			: in	std_logic;							-- marks end of write cycle, active one for one clock period of sys_clk
		clk					: IN	STD_LOGIC;							-- should be the same clk, used by SCU_Bus_Slave
		nReset				: IN	STD_LOGIC;
		User1_Reg	  		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- Daten-Reg. User1
		User2_Reg	  		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- Daten-Reg. User2
		Data_to_SCUB		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB		: OUT	STD_LOGIC									-- connect Dtack to SCUB-Macro
		);	
	END Test_User_Reg;


ARCHITECTURE Arch_Test_User_Reg OF Test_User_Reg IS

CONSTANT	addr_width:						INTEGER := Adr_from_SCUB_LA'length;
CONSTANT	User_Reg_1_addr_offset:		INTEGER := 0;						-- Offset zur Base_addr zum Setzen oder Rücklesen des User_Reg_1 Registers
CONSTANT	User_Reg_2_addr_offset:		INTEGER := 1;						-- Offset zur Base_addr zum Setzen oder Rücklesen des User_Reg_2 Registers

CONSTANT	C_User_Reg_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + User_Reg_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des User_Reg_1 Registers
CONSTANT	C_User_Reg_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + User_Reg_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des User_Reg_2 Registers

SIGNAL	S_User_Reg_1:		STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	S_User_Reg_1_Rd:	STD_LOGIC;
SIGNAL	S_User_Reg_1_Wr:	STD_LOGIC;
SIGNAL	S_User_Reg_2:		STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	S_User_Reg_2_Rd:	STD_LOGIC;
SIGNAL	S_User_Reg_2_Wr:	STD_LOGIC;

SIGNAL	S_Dtack:			STD_LOGIC;
SIGNAL	S_Read_Port:	STD_LOGIC_VECTOR(Data_to_SCUB'range);

BEGIN


P_Adr_Deco:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			S_User_Reg_1_Rd <= '0';
			S_User_Reg_1_Wr <= '0';
			S_User_Reg_2_Rd <= '0';
			S_User_Reg_2_Wr <= '0';
			S_Dtack <= '0';
		
		ELSIF rising_edge(clk) THEN
			S_User_Reg_1_Rd <= '0';
			S_User_Reg_1_Wr <= '0';
			S_User_Reg_2_Rd <= '0';
			S_User_Reg_2_Wr <= '0';
			S_Dtack <= '0';
			
			IF Ext_Adr_Val = '1' THEN

				CASE unsigned(ADR_from_SCUB_LA) IS
				
					WHEN C_User_Reg_1_Addr =>
						IF Ext_Wr_active = '1' THEN
							S_Dtack <= '1';
							S_User_Reg_1_Wr <= '1';
						END IF;
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_User_Reg_1_Rd <= '1';
						END IF;

					WHEN C_User_Reg_2_Addr =>
						IF Ext_Wr_active = '1' THEN
							S_Dtack <= '1';
							S_User_Reg_2_Wr <= '1';
						END IF;
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_User_Reg_2_Rd <= '1';
						END IF;
						
					WHEN OTHERS => null;

				END CASE;
			END IF;
		END IF;
	
	END PROCESS P_Adr_Deco;

	
P_User_Reg:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			S_User_Reg_1 <= (OTHERS => '0');
			S_User_Reg_2 <= (OTHERS => '0');
		
		ELSIF rising_edge(clk) THEN
			IF S_User_Reg_1_Wr = '1' THEN
				S_User_Reg_1 <= Data_from_SCUB_LA;
			END IF;
			IF S_User_Reg_2_Wr = '1' THEN
				S_User_Reg_2 <= Data_from_SCUB_LA;
			END IF;
		END IF;
	END PROCESS P_User_Reg;
	

P_read_mux:	PROCESS (S_User_Reg_1_Rd, S_User_Reg_2_Rd, S_User_Reg_1, S_User_Reg_2)
	BEGIN
		IF S_User_Reg_1_Rd = '1' THEN
			S_Read_port <= S_User_Reg_1;
		ELSIF S_User_Reg_2_Rd = '1' THEN
			S_Read_port <= S_User_Reg_2;
		ELSE
			S_Read_Port <= (OTHERS => '-');
		END IF;
	END PROCESS P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


User1_Reg <= S_User_Reg_1;		-- Daten-Reg. User1
User2_Reg <= S_User_Reg_2;		-- Daten-Reg. User2


END Arch_Test_User_Reg;