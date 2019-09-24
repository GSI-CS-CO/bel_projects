--TITLE "HW- Interlock : Kai Lüghausen";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY hw_interlock IS
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
		HW_ILock_in:				in	std_logic_vector(15 downto 0);	-- Hardware Interlock out
		HW_ILock_out:				out	std_logic_vector(15 downto 0);	-- Hardware Interlock out
--
		Reg_rd_active:		out	std_logic;								-- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		out	std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		out	std_logic								-- connect Dtack to SCUB-Macro
		);	
	end hw_interlock;

	

ARCHITECTURE Arch_hw_interlock OF hw_interlock IS

constant	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
constant	Reg_HW_IL_addr_offset:	INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Reg_HW_IL Registers
--
constant	C_Reg_HW_IL_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + Reg_HW_IL_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des Reg_HW_IL Registers


signal		S_Reg_HW_IL:		std_logic_vector(15 downto 0);
signal		S_Reg_HW_IL_Rd:	std_logic;
signal		S_Reg_HW_IL_Wr:	std_logic;


signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

begin


P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_Reg_HW_IL_Rd <= '0';
			S_Reg_HW_IL_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_Reg_HW_IL_Rd <= '0';
			S_Reg_HW_IL_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_Reg_HW_IL_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Reg_HW_IL_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Reg_HW_IL_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						
					when others => 

						S_Reg_HW_IL_Rd <= '0';
						
						S_Dtack <= '0';
						Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;

	
P_Reg_IO:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_Reg_HW_IL <= (others => '0');
		
		elsif rising_edge(clk) then
			if S_Reg_HW_IL_Wr = '1' then	S_Reg_HW_IL <= Data_from_SCUB_LA or HW_ILock_in;
			else
			S_Reg_HW_IL <= S_Reg_HW_IL or HW_ILock_in;
			end if;
	end if;
	end process P_Reg_IO;
	

	P_read_mux:	process (	S_Reg_HW_IL_Rd,  S_Reg_HW_IL)

	begin
		if 	S_Reg_HW_IL_Rd = '1' then	S_Read_port <= S_Reg_HW_IL;
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


HW_ILock_out <= S_Reg_HW_IL;		-- Daten-Reg. Out1
--

end Arch_hw_interlock;
