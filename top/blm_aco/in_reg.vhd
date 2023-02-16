--TITLE "'in_reg' Autor: R.Hartmann, Stand: 23.03.2016 ";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY in_reg IS
	generic
		(
		Base_addr:	INTEGER := 16#0230#
		);
		
	port(
		Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);	-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);	-- latched data from SCU_Bus 
		Ext_Adr_Val:			  in		std_logic;								-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in		std_logic;								-- '1' => Rd-Cycle is active
		Ext_Rd_fin:				  in		std_logic;								-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in		std_logic;								-- '1' => Wr-Cycle is active
		Ext_Wr_fin:				  in		std_logic;								-- marks end of write cycle, active one for one clock period of sys_clk
		clk:						    in		std_logic;								-- should be the same clk, used by SCU_Bus_Slave
		nReset:					    in		std_logic;
--
		Reg_In1:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out1
		Reg_In2:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out2
		Reg_In3:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out3
		Reg_In4:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out4
		Reg_In5:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out5
		Reg_In6:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out6
		Reg_In7:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out7
		Reg_In8:				    in  	std_logic_vector(15 downto 0);	-- Daten-Reg. Out7
--
		Reg_rd_active:		out	std_logic;								-- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		out	std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		out	std_logic								-- connect Dtack to SCUB-Macro
		);	
	end in_reg;

	

ARCHITECTURE Arch_in_reg OF in_reg IS

constant	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
constant	Reg_In_1_addr_offset:	INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_1 Registers
constant	Reg_In_2_addr_offset:	INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_2 Registers
constant	Reg_In_3_addr_offset:	INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_3 Registers
constant	Reg_In_4_addr_offset:	INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_4 Registers
constant	Reg_In_5_addr_offset:	INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_5 Registers
constant	Reg_In_6_addr_offset:	INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_6 Registers
constant	Reg_In_7_addr_offset:	INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_7 Registers
constant	Reg_In_8_addr_offset:	INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_In_8 Registers
--
constant	C_Reg_In_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + Reg_In_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_1 Registers
constant	C_Reg_In_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_2 Registers
constant	C_Reg_In_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + Reg_In_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_3 Registers
constant	C_Reg_In_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_4 Registers
constant	C_Reg_In_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_5 Registers
constant	C_Reg_In_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_6 Registers
constant	C_Reg_In_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_7 Registers
constant	C_Reg_In_8_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + Reg_In_8_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_In_8 Registers


signal		S_Reg_In_1:		std_logic_vector(15 downto 0);
signal		S_Reg_In_1_Rd:	std_logic;

signal		S_Reg_In_2:		std_logic_vector(15 downto 0);
signal		S_Reg_In_2_Rd:	std_logic;

signal		S_Reg_In_3:		std_logic_vector(15 downto 0);
signal		S_Reg_In_3_Rd:	std_logic;

signal		S_Reg_In_4:		std_logic_vector(15 downto 0);
signal		S_Reg_In_4_Rd:	std_logic;

signal		S_Reg_In_5:		std_logic_vector(15 downto 0);
signal		S_Reg_In_5_Rd:	std_logic;

signal		S_Reg_In_6:		std_logic_vector(15 downto 0);
signal		S_Reg_In_6_Rd:	std_logic;

signal		S_Reg_In_7:		std_logic_vector(15 downto 0);
signal		S_Reg_In_7_Rd:	std_logic;

signal		S_Reg_In_8:		std_logic_vector(15 downto 0);
signal		S_Reg_In_8_Rd:	std_logic;

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

begin


P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_Reg_In_1_Rd <= '0';
			S_Reg_In_2_Rd <= '0';
			S_Reg_In_3_Rd <= '0';
			S_Reg_In_4_Rd <= '0';
			S_Reg_In_5_Rd <= '0';
			S_Reg_In_6_Rd <= '0';
			S_Reg_In_7_Rd <= '0';
			S_Reg_In_8_Rd <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_Reg_In_1_Rd <= '0';
			S_Reg_In_2_Rd <= '0';
			S_Reg_In_3_Rd <= '0';
			S_Reg_In_4_Rd <= '0';
			S_Reg_In_5_Rd <= '0';
			S_Reg_In_6_Rd <= '0';
			S_Reg_In_7_Rd <= '0';
			S_Reg_In_8_Rd <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_Reg_In_1_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_1_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_In_2_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_2_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_In_3_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_3_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_Reg_In_4_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_4_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_In_5_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_5_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_In_6_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_6_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_Reg_In_7_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_7_Rd <= '1';
							Reg_rd_active <= '1';
						end if;
						
						when C_Reg_In_8_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_In_8_Rd <= '1';
							Reg_rd_active <= '1';
						end if;
						
					when others => 

            S_Reg_In_1_Rd <= '0';
            S_Reg_In_2_Rd <= '0';
            S_Reg_In_3_Rd <= '0';
            S_Reg_In_4_Rd <= '0';
            S_Reg_In_5_Rd <= '0';
            S_Reg_In_6_Rd <= '0';
            S_Reg_In_7_Rd <= '0';
            S_Reg_In_8_Rd <= '0';

						S_Dtack <= '0';
						Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;

	

	P_read_mux:	process (	S_Reg_In_1_Rd,  Reg_In1, S_Reg_In_2_Rd,  Reg_In2,
                        S_Reg_In_3_Rd,  Reg_In3, S_Reg_In_4_Rd,  Reg_In4,
                        S_Reg_In_5_Rd,  Reg_In5, S_Reg_In_6_Rd,  Reg_In6,
                        S_Reg_In_7_Rd,  Reg_In7, S_Reg_In_8_Rd,  Reg_In8)

	begin
		if 	  S_Reg_In_1_Rd = '1' then	S_Read_port <= Reg_In1;
		elsif S_Reg_In_2_Rd = '1' then	S_Read_port <= Reg_In2;
		elsif S_Reg_In_3_Rd = '1' then	S_Read_port <= Reg_In3;
		elsif S_Reg_In_4_Rd = '1' then	S_Read_port <= Reg_In4;
		elsif S_Reg_In_5_Rd = '1' then	S_Read_port <= Reg_In5;
		elsif S_Reg_In_6_Rd = '1' then	S_Read_port <= Reg_In6;
		elsif S_Reg_In_7_Rd = '1' then	S_Read_port <= Reg_In7;
		elsif S_Reg_In_8_Rd = '1' then	S_Read_port <= Reg_In8;
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


end Arch_in_reg;