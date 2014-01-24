--TITLE "'AW_IO_Reg' Autor: W.Panschow/R.Hartmann, Stand: 04.11.2013, Vers: V01 ";

-- Version 2, W.Panschow, d. 23.11.2012
--	Ausgang 'AWOut_Reg_Rd_active' hinzugefügt. Kennzeichnet, dass das Macro Daten zum Lesen aum Ausgang 'Data_to_SCUB' bereithält. 'AWOut_Reg_Rd_active' kann übergeordnet zur Steuerung des
--	am 'SCU_Bus_Slave' vorgeschalteten Multiplexers verendet werden. Dieser ist nötig, wenn verschiedene Makros Leseregister zum 'SCU_Bus_Slave'-Eingang 'Data_to_SCUB' anlegen müssen.
--
library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY AW_IO_Reg IS
	generic
		(
		Base_addr:	INTEGER := 16#0400#
		);
		
	port(
		Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val:			in		std_logic;							-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in		std_logic;							-- '1' => Rd-Cycle is active
		Ext_Rd_fin:				in		std_logic;							-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in		std_logic;							-- '1' => Wr-Cycle is active
		Ext_Wr_fin:				in		std_logic;							-- marks end of write cycle, active one for one clock period of sys_clk
		clk:						in		std_logic;							-- should be the same clk, used by SCU_Bus_Slave
		nReset:					in		std_logic;
		AWIn1:					in		std_logic_vector(15 downto 0);	-- Input-Port 1
		AWIn2:					in		std_logic_vector(15 downto 0);	-- Input-Port 2
		AWIn3:					in		std_logic_vector(15 downto 0);	-- Input-Port 3
		AWIn4:					in		std_logic_vector(15 downto 0);	-- Input-Port 4
		AWIn5:					in		std_logic_vector(15 downto 0);	-- Input-Port 5
		AWOut_Reg1:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut1
		AWOut_Reg2:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut2
		AWOut_Reg3:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut3
		AWOut_Reg4:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut4
		AWOut_Reg5:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut5
		AWOut_Reg_rd_active:	out	std_logic;							-- read data available at 'Data_to_SCUB'-AWOut
		Data_to_SCUB:			out	std_logic_vector(15 downto 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:			out	std_logic							-- connect Dtack to SCUB-Macro
		);	
	end AW_IO_Reg;


ARCHITECTURE Arch_AW_IO_Reg OF AW_IO_Reg IS

constant	addr_width:							INTEGER := Adr_from_SCUB_LA'length;
constant	AWOut_Reg_1_addr_offset:		INTEGER := 0;						-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant	AWOut_Reg_2_addr_offset:		INTEGER := 1;						-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	AWOut_Reg_3_addr_offset:		INTEGER := 2;						-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant	AWOut_Reg_4_addr_offset:		INTEGER := 3;						-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	AWOut_Reg_5_addr_offset:		INTEGER := 4;						-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	AWIn_1_addr_offset:				INTEGER := 10;						-- Offset zur Base_addr zum Rücklesen des AWIN_Port1
constant	AWIn_2_addr_offset:				INTEGER := 11;						-- Offset zur Base_addr zum Rücklesen des AWIN_Port2
constant	AWIn_3_addr_offset:				INTEGER := 12;						-- Offset zur Base_addr zum Rücklesen des AWIN_Port3
constant	AWIn_4_addr_offset:				INTEGER := 13;						-- Offset zur Base_addr zum Rücklesen des AWIN_Port4
constant	AWIn_5_addr_offset:				INTEGER := 14;						-- Offset zur Base_addr zum Rücklesen des AWIN_Port5

constant	C_AWOut_Reg_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + AWOut_Reg_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant	C_AWOut_Reg_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWOut_Reg_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	C_AWOut_Reg_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + AWOut_Reg_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant	C_AWOut_Reg_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWOut_Reg_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant	C_AWOut_Reg_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWOut_Reg_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_5 Registers

constant	C_AWIN_1_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWIn_1_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn1
constant	C_AWIN_2_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWIn_2_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn2
constant	C_AWIN_3_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWIn_3_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn3
constant	C_AWIN_4_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWIn_4_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn4
constant	C_AWIN_5_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + AWIn_5_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn5


signal		S_AWOut_Reg_1:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_1_Rd:	std_logic;
signal		S_AWOut_Reg_1_Wr:	std_logic;
signal		S_AWOut_Reg_2:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_2_Rd:	std_logic;
signal		S_AWOut_Reg_2_Wr:	std_logic;
signal		S_AWOut_Reg_3:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_3_Rd:	std_logic;
signal		S_AWOut_Reg_3_Wr:	std_logic;
signal		S_AWOut_Reg_4:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_4_Rd:	std_logic;
signal		S_AWOut_Reg_4_Wr:	std_logic;
signal		S_AWOut_Reg_5:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_5_Rd:	std_logic;
signal		S_AWOut_Reg_5_Wr:	std_logic;

signal		S_AWIn1_Rd:			std_logic;
signal		S_AWIn2_Rd:			std_logic;
signal		S_AWIn3_Rd:			std_logic;
signal		S_AWIn4_Rd:			std_logic;
signal		S_AWIn5_Rd:			std_logic;

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

begin


P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_AWOut_Reg_1_Rd <= '0';
			S_AWOut_Reg_1_Wr <= '0';
			S_AWOut_Reg_2_Rd <= '0';
			S_AWOut_Reg_2_Wr <= '0';
			S_AWOut_Reg_3_Rd <= '0';
			S_AWOut_Reg_3_Wr <= '0';
			S_AWOut_Reg_4_Rd <= '0';
			S_AWOut_Reg_4_Wr <= '0';
			S_AWOut_Reg_5_Rd <= '0';
			S_AWOut_Reg_5_Wr <= '0';

			S_AWIn1_Rd <= '0';
			S_AWIn2_Rd <= '0';
			S_AWIn3_Rd <= '0';
			S_AWIn4_Rd <= '0';
			S_AWIn5_Rd <= '0';

			S_Dtack <= '0';
			AWOut_Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_AWOut_Reg_1_Rd <= '0';
			S_AWOut_Reg_1_Wr <= '0';
			S_AWOut_Reg_2_Rd <= '0';
			S_AWOut_Reg_2_Wr <= '0';
			S_AWOut_Reg_3_Rd <= '0';
			S_AWOut_Reg_3_Wr <= '0';
			S_AWOut_Reg_4_Rd <= '0';
			S_AWOut_Reg_4_Wr <= '0';
			S_AWOut_Reg_5_Rd <= '0';
			S_AWOut_Reg_5_Wr <= '0';

			S_AWIn1_Rd <= '0';
			S_AWIn2_Rd <= '0';
			S_AWIn3_Rd <= '0';
			S_AWIn4_Rd <= '0';
			S_AWIn5_Rd <= '0';

			S_Dtack <= '0';
			AWOut_Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS
				
					when C_AWOut_Reg_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_1_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_1_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_2_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_2_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_3_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_3_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_4_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_4_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWOut_Reg_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_5_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_5_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn1_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						when C_AWIN_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn2_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						when C_AWIN_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn3_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						when C_AWIN_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn4_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						when C_AWIN_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn5_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						
					when others => 
						S_AWOut_Reg_1_Rd <= '0';
						S_AWOut_Reg_1_Wr <= '0';
						S_AWOut_Reg_2_Rd <= '0';
						S_AWOut_Reg_2_Wr <= '0';
						S_AWOut_Reg_3_Rd <= '0';
						S_AWOut_Reg_3_Wr <= '0';
						S_AWOut_Reg_4_Rd <= '0';
						S_AWOut_Reg_4_Wr <= '0';
						S_AWOut_Reg_5_Rd <= '0';
						S_AWOut_Reg_5_Wr <= '0';

						S_AWIn1_Rd <= '0';
						S_AWIn2_Rd <= '0';
						S_AWIn3_Rd <= '0';
						S_AWIn4_Rd <= '0';
						S_AWIn5_Rd <= '0';

						S_Dtack <= '0';
						AWOut_Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;

	
P_AWOut_Reg:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_AWOut_Reg_1 <= (others => '0');
			S_AWOut_Reg_2 <= (others => '0');
			S_AWOut_Reg_3 <= (others => '0');
			S_AWOut_Reg_4 <= (others => '0');
			S_AWOut_Reg_5 <= (others => '0');
		
		elsif rising_edge(clk) then
			if S_AWOut_Reg_1_Wr = '1' then
				S_AWOut_Reg_1 <= Data_from_SCUB_LA;
			end if;
			if S_AWOut_Reg_2_Wr = '1' then
				S_AWOut_Reg_2 <= Data_from_SCUB_LA;
			end if;
			if S_AWOut_Reg_3_Wr = '1' then
				S_AWOut_Reg_3 <= Data_from_SCUB_LA;
			end if;
			if S_AWOut_Reg_4_Wr = '1' then
				S_AWOut_Reg_4 <= Data_from_SCUB_LA;
			end if;
			if S_AWOut_Reg_5_Wr = '1' then
				S_AWOut_Reg_5 <= Data_from_SCUB_LA;
			end if;
		end if;
	end process P_AWOut_Reg;
	

P_read_mux:	process (S_AWOut_Reg_1_Rd, S_AWOut_Reg_2_Rd, S_AWOut_Reg_3_Rd, S_AWOut_Reg_4_Rd, S_AWOut_Reg_5_Rd,
							S_AWOut_Reg_1,    S_AWOut_Reg_2,    S_AWOut_Reg_3,    S_AWOut_Reg_4,    S_AWOut_Reg_5,
							S_AWIn1_Rd, 		S_AWIn2_Rd,			S_AWIn3_Rd,			S_AWIn4_Rd,			S_AWIn5_Rd,
							AWIn1,				AWIn2,				AWIn3,				AWIn4,				AWIn5) 
	begin
		if S_AWOut_Reg_1_Rd = '1' then
			S_Read_port <= S_AWOut_Reg_1;
		elsif S_AWOut_Reg_2_Rd = '1' then
			S_Read_port <= S_AWOut_Reg_2;
		elsif S_AWOut_Reg_3_Rd = '1' then
			S_Read_port <= S_AWOut_Reg_3;
		elsif S_AWOut_Reg_4_Rd = '1' then
			S_Read_port <= S_AWOut_Reg_4;
		elsif S_AWOut_Reg_5_Rd = '1' then
			S_Read_port <= S_AWOut_Reg_5;

		elsif S_AWIn1_Rd = '1' then
			S_Read_port <= AWIn1;		-- read Input-Port1
		elsif S_AWIn2_Rd = '1' then
			S_Read_port <= AWIn2;
		elsif S_AWIn3_Rd = '1' then
			S_Read_port <= AWIn3;
		elsif S_AWIn4_Rd = '1' then
			S_Read_port <= AWIn4;
		elsif S_AWIn5_Rd = '1' then
			S_Read_port <= AWIn5;
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


AWOut_Reg1 <= S_AWOut_Reg_1;		-- Daten-Reg. AWOut1
AWOut_Reg2 <= S_AWOut_Reg_2;		-- Daten-Reg. AWOut2
AWOut_Reg3 <= S_AWOut_Reg_3;		-- Daten-Reg. AWOut3
AWOut_Reg4 <= S_AWOut_Reg_4;		-- Daten-Reg. AWOut4
AWOut_Reg5 <= S_AWOut_Reg_5;		-- Daten-Reg. AWOut5


end Arch_AW_IO_Reg;