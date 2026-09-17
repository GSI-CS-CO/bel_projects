--TITLE "'io_reg' Autor: R.Hartmann, Stand: 22.07.2014, Vers: V01 ";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY io_reg IS
	generic
		(
		Base_addr:	INTEGER := 16#0230#
		);
		
	port(
		Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);	-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);	-- latched data from SCU_Bus 
		Ext_Adr_Val:			in		std_logic;								-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in		std_logic;								-- '1' => Rd-Cycle is active
		Ext_Rd_fin:				in		std_logic;								-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in		std_logic;								-- '1' => Wr-Cycle is active
		Ext_Wr_fin:				in		std_logic;								-- marks end of write cycle, active one for one clock period of sys_clk
		clk:						in		std_logic;								-- should be the same clk, used by SCU_Bus_Slave
		nReset:					in		std_logic;
--
		Reg_IO1:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out1
		Reg_IO2:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out2
		Reg_IO3:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out3
		Reg_IO4:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out4
		Reg_IO5:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out5
		Reg_IO6:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out6
		Reg_IO7:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out7
		Reg_IO8:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. Out7
--
		Reg_rd_active:		out	std_logic;								-- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		out	std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		out	std_logic								-- connect Dtack to SCUB-Macro
		);	
	end io_reg;

	

ARCHITECTURE Arch_io_reg OF io_reg IS

constant	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
constant	Reg_IO_1_addr_offset:	INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_1 Registers
constant	Reg_IO_2_addr_offset:	INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_2 Registers
constant	Reg_IO_3_addr_offset:	INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_3 Registers
constant	Reg_IO_4_addr_offset:	INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_4 Registers
constant	Reg_IO_5_addr_offset:	INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_5 Registers
constant	Reg_IO_6_addr_offset:	INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_6 Registers
constant	Reg_IO_7_addr_offset:	INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_7 Registers
constant	Reg_IO_8_addr_offset:	INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_IO_8 Registers
--
constant	C_Reg_IO_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + Reg_IO_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_1 Registers
constant	C_Reg_IO_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_2 Registers
constant	C_Reg_IO_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + Reg_IO_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_3 Registers
constant	C_Reg_IO_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_4 Registers
constant	C_Reg_IO_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_5 Registers
constant	C_Reg_IO_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_6 Registers
constant	C_Reg_IO_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_7 Registers
constant	C_Reg_IO_8_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_IO_8_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_IO_8 Registers


signal		S_Reg_IO_1:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_1_Rd:	std_logic;
signal		S_Reg_IO_1_Wr:	std_logic;

signal		S_Reg_IO_2:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_2_Rd:	std_logic;
signal		S_Reg_IO_2_Wr:	std_logic;

signal		S_Reg_IO_3:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_3_Rd:	std_logic;
signal		S_Reg_IO_3_Wr:	std_logic;

signal		S_Reg_IO_4:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_4_Rd:	std_logic;
signal		S_Reg_IO_4_Wr:	std_logic;

signal		S_Reg_IO_5:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_5_Rd:	std_logic;
signal		S_Reg_IO_5_Wr:	std_logic;

signal		S_Reg_IO_6:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_6_Rd:	std_logic;
signal		S_Reg_IO_6_Wr:	std_logic;

signal		S_Reg_IO_7:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_7_Rd:	std_logic;
signal		S_Reg_IO_7_Wr:	std_logic;

signal		S_Reg_IO_8:		std_logic_vector(15 downto 0);
signal		S_Reg_IO_8_Rd:	std_logic;
signal		S_Reg_IO_8_Wr:	std_logic;

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

begin


P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_Reg_IO_1_Rd <= '0';
			S_Reg_IO_1_Wr <= '0';
			S_Reg_IO_2_Rd <= '0';
			S_Reg_IO_2_Wr <= '0';
			S_Reg_IO_3_Rd <= '0';
			S_Reg_IO_3_Wr <= '0';
			S_Reg_IO_4_Rd <= '0';
			S_Reg_IO_4_Wr <= '0';
			S_Reg_IO_5_Rd <= '0';
			S_Reg_IO_5_Wr <= '0';
			S_Reg_IO_6_Rd <= '0';
			S_Reg_IO_6_Wr <= '0';
			S_Reg_IO_7_Rd <= '0';
			S_Reg_IO_7_Wr <= '0';
			S_Reg_IO_8_Rd <= '0';
			S_Reg_IO_8_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_Reg_IO_1_Rd <= '0';
			S_Reg_IO_1_Wr <= '0';
			S_Reg_IO_2_Rd <= '0';
			S_Reg_IO_2_Wr <= '0';
			S_Reg_IO_3_Rd <= '0';
			S_Reg_IO_3_Wr <= '0';
			S_Reg_IO_4_Rd <= '0';
			S_Reg_IO_4_Wr <= '0';
			S_Reg_IO_5_Rd <= '0';
			S_Reg_IO_5_Wr <= '0';
			S_Reg_IO_6_Rd <= '0';
			S_Reg_IO_6_Wr <= '0';
			S_Reg_IO_7_Rd <= '0';
			S_Reg_IO_7_Wr <= '0';
			S_Reg_IO_8_Rd <= '0';
			S_Reg_IO_8_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_Reg_IO_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_1_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_1_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_IO_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_2_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_2_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_IO_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_3_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_3_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_IO_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_4_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_4_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_IO_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_5_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_5_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_IO_6_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_6_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_6_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_IO_7_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_7_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_7_Rd <= '1';
							Reg_rd_active <= '1';
						end if;
						
						when C_Reg_IO_8_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_8_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_IO_8_Rd <= '1';
							Reg_rd_active <= '1';
						end if;
						
					when others => 

						S_Reg_IO_1_Rd <= '0';
						S_Reg_IO_1_Wr <= '0';
						S_Reg_IO_2_Rd <= '0';
						S_Reg_IO_2_Wr <= '0';
						S_Reg_IO_3_Rd <= '0';
						S_Reg_IO_3_Wr <= '0';
						S_Reg_IO_4_Rd <= '0';
						S_Reg_IO_4_Wr <= '0';
						S_Reg_IO_5_Rd <= '0';
						S_Reg_IO_5_Wr <= '0';
						S_Reg_IO_6_Rd <= '0';
						S_Reg_IO_6_Wr <= '0';
						S_Reg_IO_7_Rd <= '0';
						S_Reg_IO_7_Wr <= '0';
						S_Reg_IO_8_Rd <= '0';
						S_Reg_IO_8_Wr <= '0';

						S_Dtack <= '0';
						Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;

	
P_Reg_IO:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_Reg_IO_1 <= (others => '0');
			S_Reg_IO_2 <= (others => '0');
			S_Reg_IO_3 <= (others => '0');
			S_Reg_IO_4 <= (others => '0');
			S_Reg_IO_5 <= (others => '0');
			S_Reg_IO_6 <= (others => '0');
			S_Reg_IO_7 <= (others => '0');
			S_Reg_IO_8 <= (others => '0');
		
		elsif rising_edge(clk) then
			if S_Reg_IO_1_Wr = '1' then	S_Reg_IO_1 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_2_Wr = '1' then	S_Reg_IO_2 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_3_Wr = '1' then	S_Reg_IO_3 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_4_Wr = '1' then	S_Reg_IO_4 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_5_Wr = '1' then	S_Reg_IO_5 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_6_Wr = '1' then	S_Reg_IO_6 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_7_Wr = '1' then	S_Reg_IO_7 <= Data_from_SCUB_LA;
			end if;
			if S_Reg_IO_8_Wr = '1' then	S_Reg_IO_8 <= Data_from_SCUB_LA;
			end if;
	end if;
	end process P_Reg_IO;
	

	P_read_mux:	process (	S_Reg_IO_1_Rd,  S_Reg_IO_1, S_Reg_IO_2_Rd,  S_Reg_IO_2,
									S_Reg_IO_3_Rd,  S_Reg_IO_3, S_Reg_IO_4_Rd,  S_Reg_IO_4,
									S_Reg_IO_5_Rd,  S_Reg_IO_5, S_Reg_IO_6_Rd,  S_Reg_IO_6,
									S_Reg_IO_7_Rd,  S_Reg_IO_7, S_Reg_IO_8_Rd,  S_Reg_IO_8)

	begin
		if 	S_Reg_IO_1_Rd = '1' then	S_Read_port <= S_Reg_IO_1;
		elsif S_Reg_IO_2_Rd = '1' then	S_Read_port <= S_Reg_IO_2;
		elsif S_Reg_IO_3_Rd = '1' then	S_Read_port <= S_Reg_IO_3;
		elsif S_Reg_IO_4_Rd = '1' then	S_Read_port <= S_Reg_IO_4;
		elsif S_Reg_IO_5_Rd = '1' then	S_Read_port <= S_Reg_IO_5;
		elsif S_Reg_IO_6_Rd = '1' then	S_Read_port <= S_Reg_IO_6;
		elsif S_Reg_IO_7_Rd = '1' then	S_Read_port <= S_Reg_IO_7;
		elsif S_Reg_IO_8_Rd = '1' then	S_Read_port <= S_Reg_IO_8;
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


Reg_IO1 <= S_Reg_IO_1;		-- Daten-Reg. Out1
Reg_IO2 <= S_Reg_IO_2;		-- Daten-Reg. Out2
Reg_IO3 <= S_Reg_IO_3;		-- Daten-Reg. Out3
Reg_IO4 <= S_Reg_IO_4;		-- Daten-Reg. Out4
Reg_IO5 <= S_Reg_IO_5;		-- Daten-Reg. Out5
Reg_IO6 <= S_Reg_IO_6;		-- Daten-Reg. Out6
Reg_IO7 <= S_Reg_IO_7;		-- Daten-Reg. Out7
Reg_IO8 <= S_Reg_IO_8;		-- Daten-Reg. Out8
--

end Arch_io_reg;