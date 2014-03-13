--TITLE "'Test_User_Reg' Autor: W.Panschow, Stand: 25.09.08, Vers: V01 ";

-- Version 2, W.Panschow, d. 23.11.2012
--	Ausgang 'User_Reg_Rd_active' hinzugefügt. Kennzeichnet, dass das Macro Daten zum Lesen aum Ausgang 'Data_to_SCUB' bereithält. 'User_Reg_Rd_active' kann übergeordnet zur Steuerung des
--	am 'SCU_Bus_Slave' vorgeschalteten Multiplexers verendet werden. Dieser ist nötig, wenn verschiedene Makros Leseregister zum 'SCU_Bus_Slave'-Eingang 'Data_to_SCUB' anlegen müssen.
--
library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY Test_User_Reg IS
	generic
		(
		Base_addr:	INTEGER := 16#0200#
		);
		
	port(
		Adr_from_SCUB_LA:	in		std_logic_vector(15 downto 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val:		in		std_logic;							-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:		in		std_logic;							-- '1' => Rd-Cycle is active
		Ext_Rd_fin:			in		std_logic;							-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:		in		std_logic;							-- '1' => Wr-Cycle is active
		Ext_Wr_fin:			in		std_logic;							-- marks end of write cycle, active one for one clock period of sys_clk
		clk:				in		std_logic;							-- should be the same clk, used by SCU_Bus_Slave
		nReset:				in		std_logic;
		User1_Reg:			out		std_logic_vector(15 downto 0);		-- Daten-Reg. User1
		User2_Reg:			out		std_logic_vector(15 downto 0);		-- Daten-Reg. User2
		User_Reg_rd_active:	out		std_logic;							-- read data available at 'Data_to_SCUB'-output
		Data_to_SCUB:		out		std_logic_vector(15 downto 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		out		std_logic							-- connect Dtack to SCUB-Macro
		);	
	end Test_User_Reg;


ARCHITECTURE Arch_Test_User_Reg OF Test_User_Reg IS

constant	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
constant	User_Reg_1_addr_offset:		INTEGER := 0;						-- Offset zur Base_addr zum Setzen oder Rücklesen des User_Reg_1 Registers
constant	User_Reg_2_addr_offset:		INTEGER := 1;						-- Offset zur Base_addr zum Setzen oder Rücklesen des User_Reg_2 Registers

constant	C_User_Reg_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + User_Reg_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des User_Reg_1 Registers
constant	C_User_Reg_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + User_Reg_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des User_Reg_2 Registers

signal		S_User_Reg_1:		std_logic_vector(15 downto 0);
signal		S_User_Reg_1_Rd:	std_logic;
signal		S_User_Reg_1_Wr:	std_logic;
signal		S_User_Reg_2:		std_logic_vector(15 downto 0);
signal		S_User_Reg_2_Rd:	std_logic;
signal		S_User_Reg_2_Wr:	std_logic;

signal		S_Dtack:			std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

begin


P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_User_Reg_1_Rd <= '0';
			S_User_Reg_1_Wr <= '0';
			S_User_Reg_2_Rd <= '0';
			S_User_Reg_2_Wr <= '0';
			S_Dtack <= '0';
			User_Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_User_Reg_1_Rd <= '0';
			S_User_Reg_1_Wr <= '0';
			S_User_Reg_2_Rd <= '0';
			S_User_Reg_2_Wr <= '0';
			S_Dtack <= '0';
			User_Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS
				
					when C_User_Reg_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_User_Reg_1_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_User_Reg_1_Rd <= '1';
							User_Reg_rd_active <= '1';
						end if;

					when C_User_Reg_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_User_Reg_2_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_User_Reg_2_Rd <= '1';
							User_Reg_rd_active <= '1';
						end if;
						
					when others => 
						S_User_Reg_1_Rd <= '0';
						S_User_Reg_1_Wr <= '0';
						S_User_Reg_2_Rd <= '0';
						S_User_Reg_2_Wr <= '0';
						S_Dtack <= '0';
						User_Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;

	
P_User_Reg:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_User_Reg_1 <= (others => '0');
			S_User_Reg_2 <= (others => '0');
		
		elsif rising_edge(clk) then
			if S_User_Reg_1_Wr = '1' then
				S_User_Reg_1 <= Data_from_SCUB_LA;
			end if;
			if S_User_Reg_2_Wr = '1' then
				S_User_Reg_2 <= Data_from_SCUB_LA;
			end if;
		end if;
	end process P_User_Reg;
	

P_read_mux:	process (S_User_Reg_1_Rd, S_User_Reg_2_Rd, S_User_Reg_1, S_User_Reg_2)
	begin
		if S_User_Reg_1_Rd = '1' then
			S_Read_port <= S_User_Reg_1;
		elsif S_User_Reg_2_Rd = '1' then
			S_Read_port <= S_User_Reg_2;
		else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


User1_Reg <= S_User_Reg_1;		-- Daten-Reg. User1
User2_Reg <= S_User_Reg_2;		-- Daten-Reg. User2


end Arch_Test_User_Reg;