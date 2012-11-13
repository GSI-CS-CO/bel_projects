--TITLE "'MIL_Reg' Autor: R.Hartmann, Stand: 06.07.2012, Vers: V01 ";
--
library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.numeric_std.all;

ENTITY MIL_Reg IS
	GENERIC
		(
		Base_addr:	INTEGER := 16#0200#
		);
		
	PORT(
		Adr_from_SCUB_LA	: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA	: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val			: IN	STD_LOGIC;									-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active		: in	std_logic;									-- '1' => Rd-Cycle is active
		Ext_Rd_fin			: in	std_logic;									-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active		: in	std_logic;									-- '1' => Wr-Cycle is active
		Ext_Wr_fin			: in	std_logic;									-- marks end of write cycle, active one for one clock period of sys_clk
		clk					: IN	STD_LOGIC;									-- should be the same clk, used by SCU_Bus_Slave
		nReset				: IN	STD_LOGIC;
		Data_from_MIL		: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Status_from_MIL	: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		SS_to_MIL			: OUT	STD_LOGIC;									-- Synchron-Selekt CMD/Data
		WR_to_MIL			: OUT	STD_LOGIC;									-- Write CMD/Data
		CMD_Data_to_MIL	: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		CMD_to_MIL	 		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Data_to_MIL			: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Data_to_SCUB		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB		: OUT	STD_LOGIC									-- connect Dtack to SCUB-Macro
		);	
	END MIL_Reg;


ARCHITECTURE Arch_MIL_Reg OF MIL_Reg IS

CONSTANT	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
CONSTANT	MIL_Data_addr_offset:	INTEGER := 0;						-- Offset zur Base_addr zum Setzen oder Rücklesen des MIL_Data Registers
CONSTANT	MIL_CMD_addr_offset:		INTEGER := 1;						-- Offset zur Base_addr zum Setzen oder Rücklesen des MIL_CMD Registers
CONSTANT	MIL_Read_addr_offset:	INTEGER := 2;						-- Offset zur Base_addr zum Setzen oder Rücklesen des MIL_CMD Registers
CONSTANT	MIL_Sts_addr_offset:		INTEGER := 3;						-- Offset zur Base_addr zum Setzen oder Rücklesen des MIL_Data Registers

CONSTANT	C_MIL_CMD_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + MIL_CMD_addr_offset),  addr_width);	-- Adresse zum Setzen oder Rücklesen des MIL_CMD Registers
CONSTANT	C_MIL_Data_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + MIL_Data_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des MIL_Data Registers
CONSTANT	C_MIL_Read_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + MIL_Read_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des MIL_CMD Registers
CONSTANT	C_MIL_Sts_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + MIL_Sts_addr_offset),  addr_width);	-- Adresse zum Setzen oder Rücklesen des MIL_Data Registers

SIGNAL	S_MIL_CMD:		STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	S_MIL_CMD_Rd:	STD_LOGIC;
SIGNAL	S_MIL_CMD_Wr:	STD_LOGIC;
SIGNAL	S_MIL_Data:		STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	S_MIL_Data_Rd:	STD_LOGIC;
SIGNAL	S_MIL_Data_Wr:	STD_LOGIC;

SIGNAL	S_MIL_CMD_Data: STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	S_SS_to_MIL: 	 STD_LOGIC;
SIGNAL	S_WR_to_MIL_Wr: STD_LOGIC;
SIGNAL	S_MIL_Read_Rd:	 STD_LOGIC;
SIGNAL	S_MIL_Sts_Rd:	 STD_LOGIC;

SIGNAL	S_Dtack:			 STD_LOGIC;
SIGNAL	S_Read_Port:	 STD_LOGIC_VECTOR(Data_to_SCUB'range);

BEGIN


P_Adr_Deco:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			S_MIL_CMD_Rd   <= '0';
			S_MIL_CMD_Wr   <= '0';
			S_MIL_Data_Rd  <= '0';
			S_MIL_Data_Wr  <= '0';

			S_MIL_Read_Rd  <= '0';
			S_MIL_Sts_Rd   <= '0';

			S_Dtack <= '0';
		
		ELSIF rising_edge(clk) THEN
			S_MIL_CMD_Rd  <= '0';
			S_MIL_CMD_Wr  <= '0';
			S_MIL_Data_Rd <= '0';
			S_MIL_Data_Wr <= '0';

			S_MIL_Read_Rd  <= '0';
			S_MIL_Sts_Rd   <= '0';

			S_Dtack <= '0';
			
			IF Ext_Adr_Val = '1' THEN

				CASE unsigned(ADR_from_SCUB_LA) IS
				
					WHEN C_MIL_CMD_Addr =>
						IF Ext_Wr_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_CMD_Wr <= '1';
						END IF;
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_CMD_Rd <= '1';
						END IF;

					WHEN C_MIL_Data_Addr =>
						IF Ext_Wr_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_Data_Wr <= '1';
						END IF;
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_Data_Rd <= '1';
						END IF;
						
					WHEN C_MIL_Read_Addr =>
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_Read_Rd <= '1';
						END IF;

					WHEN C_MIL_Sts_Addr =>
						IF Ext_Rd_active = '1' THEN
							S_Dtack <= '1';
							S_MIL_Sts_Rd <= '1';
						END IF;
						
					WHEN OTHERS => null;

				END CASE;
			END IF;
		END IF;
	
	END PROCESS P_Adr_Deco;

	
P_User_Reg:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			S_MIL_CMD  		<= (OTHERS => '0');
			S_MIL_Data 		<= (OTHERS => '0');
	
		ELSIF rising_edge(clk) THEN
			IF S_MIL_CMD_Wr 	 = '1' THEN
				S_MIL_CMD   	<= Data_from_SCUB_LA;
			END IF;
			IF S_MIL_Data_Wr   = '1' THEN
				S_MIL_Data   	<= Data_from_SCUB_LA;
			END IF;
		END IF;
	END PROCESS P_User_Reg;
	

P_WR_MIL:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			S_MIL_CMD_Data	<= (OTHERS => '0');
			S_SS_to_MIL 	<= '0';
			S_WR_to_MIL_Wr	<= '0';
	
		ELSIF rising_edge(clk) THEN
			IF S_MIL_Data_Wr   = '1' THEN
				S_SS_to_MIL 	<= '0';					--- Data-Synch.			
				S_MIL_CMD_Data	<= Data_from_SCUB_LA;
			END IF;
			IF S_MIL_CMD_Wr 	 = '1' THEN
				S_SS_to_MIL 	<= '1';					--- CMD-Synch.			
				S_MIL_CMD_Data	<= Data_from_SCUB_LA;
			END IF;
			IF		 (S_MIL_CMD_Wr OR S_MIL_Data_Wr)	= '1' THEN
						S_WR_to_MIL_Wr <= '1'; 			-- set '0'
			ELSIF	 (S_MIL_CMD_Wr OR S_MIL_Data_Wr)	= '0' THEN
						S_WR_to_MIL_Wr <= '0'; 			-- set '1'
			END IF;
	END IF;
	END PROCESS P_WR_MIL;	

		
P_read_mux:	PROCESS (S_MIL_CMD_Rd, S_MIL_Data_Rd, S_MIL_Read_Rd, S_MIL_Sts_Rd, S_MIL_CMD, S_MIL_Data)
	BEGIN
		IF S_MIL_CMD_Rd = '1' THEN
			S_Read_port <= S_MIL_CMD;
		END IF;
		IF S_MIL_Data_Rd = '1' THEN
			S_Read_port <= S_MIL_Data;
		END IF;

		IF S_MIL_Read_Rd = '1' THEN
			S_Read_port <= Data_from_MIL;
		END IF;
		IF S_MIL_Sts_Rd = '1' THEN
			S_Read_port <= Status_from_MIL;
		END IF;

	END PROCESS P_Read_mux;


Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;

SS_to_MIL 	 		<= S_SS_to_MIL;
WR_to_MIL 	 		<= S_WR_to_MIL_Wr;
CMD_Data_to_MIL	<=	S_MIL_CMD_Data;	
CMD_to_MIL	 		<=	S_MIL_CMD;	
Data_to_MIL	 		<= S_MIL_Data;

END Arch_MIL_Reg;