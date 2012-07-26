-- "'chopper_m2_logic' => Chopper-Macro2-Logic, Autor: W.Panschow, S.Rauch, Stand: 27.11.2008, Vers: V03 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;

entity chopper_m2_logic is
	port (
			Skal_OK:					in std_logic;
			Data_WR:					in std_logic_vector(15 downto 0);
			Anforder_In:				in std_logic_vector(15 downto 0);
			Anforder_Maske_WR:			in std_logic;
			EMI_Schwelle_WR:			in std_logic;
			CLK:						in std_logic;		-- Sollte der Takt sein, der auch in den anderen Macros verwendet wird.
			Reset:						in std_logic;

			Off_Anforderung_Out:		out std_logic;
			Off_UU_Out:					out std_logic;

			nMask_Anf_SIS:				out std_logic;
			nMask_Anf_U:				out std_logic;
			nMask_Anf_X:				out std_logic;
			nMask_Anf_Y:				out std_logic;	
			nMask_Anf_Z:				out std_logic;
			nMask_Anf_M:				out std_logic;
			

			Anforder_Inputs:			out std_logic_vector(15 downto 0);
			EMI_Schwelle:				out std_logic_vector(15 downto 0)
		);
end chopper_m2_logic;

architecture chopper_m2_logic_arch of chopper_m2_logic is

-- Register

signal s_Anforder_Maske:				std_logic_vector(15 downto 0);
signal s_EMI_Schwelle:					std_logic_vector(15 downto 0);
signal s_Logik_not_Sel_or_Reset:		std_logic;


--+-------------------------------------+
--|	Die Namen der Anforder-Eingänge 	|
--+-------------------------------------+
signal	Exp_Anf_X0:				std_logic;
signal	Exp_Anf_X1:				std_logic;
signal	Exp_Anf_X2:				std_logic;
signal	Exp_Anf_X3:				std_logic;
signal	Exp_Anf_X4_5:			std_logic;
signal	Exp_Anf_X6:				std_logic;
signal	Exp_Anf_X7:				std_logic;
signal	Exp_Anf_X8:				std_logic;
signal	Exp_Anf_Y7:				std_logic;
signal	Exp_Anf_Z6:				std_logic;
signal	Exp_Anf_UU:				std_logic;
signal	Exp_Anf_M1:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Exp_Anf_M2:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Exp_Anf_M3:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Exp_Anf_SIS:			std_logic;


--+-----------------------------------------------------------------------------+
--|		Die Maskierungs-Bits der Anforder-Eingänge (von Interlock-SE)		 	|
--+-----------------------------------------------------------------------------+
signal	Mask_Anf_X0:				std_logic;
signal	Mask_Anf_X1:				std_logic;
signal	Mask_Anf_X2:				std_logic;
signal	Mask_Anf_X3:				std_logic;
signal	Mask_Anf_X4_5:				std_logic;
signal	Mask_Anf_X6:				std_logic;
signal	Mask_Anf_X7:				std_logic;
signal	Mask_Anf_X8:				std_logic;
signal	Mask_Anf_Y7:				std_logic;

signal	Mask_Anf_Z6:				std_logic;
signal	Mask_Anf_UU:				std_logic;
signal	Mask_Anf_M1:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Mask_Anf_M2:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Mask_Anf_M3:				std_logic;		-- hinzugefügt am 5.12.2007
signal	Mask_Anf_SIS:				std_logic;


begin
	Logik_not_Sel_or_Reset: process (Clk)
	begin
		if rising_edge(Clk) then
			if Reset = '1' or Skal_OK = '0' then
				s_Logik_not_Sel_or_Reset <= '1';
			else
				s_Logik_not_Sel_or_Reset <= '0';
			end if;
		end if;
	end process;
	
	EMI_Schwelle_ff: process (Clk, Reset)
	begin
		if Reset = '1' then
			s_EMI_Schwelle <= x"0000";
		elsif rising_edge(Clk) then
			if EMI_Schwelle_WR = '1' then
				s_EMI_Schwelle <= Data_WR;
			end if;
		end if;
	end process;
	
	EMI_Schwelle <= s_EMI_Schwelle;
	
--+-----------------------------------------------------------------------------+
--|		Maskierung der Anforderungs_Eingänge (von Interlock-SE)					|
--+-----------------------------------------------------------------------------+
	
	Anforder_Maske_ff: process (Clk, Reset)
	begin
		if Reset = '1'then
			s_Anforder_Maske <= x"0000";
		elsif rising_edge(Clk) then
			if Anforder_Maske_WR = '1' then
				s_Anforder_Maske <= Data_WR;
			end if;
		end if;
	end process;
	
	-- Belegung vom 27.11.2008 mit neuem Signal Mask_Anf_SIS			
	Mask_Anf_X0	 					<= s_Anforder_Maske(0);
	Mask_Anf_X1	 					<= s_Anforder_Maske(1);
	Mask_Anf_X2	 					<= s_Anforder_Maske(2);
	Mask_Anf_X3	 					<= s_Anforder_Maske(3);
	Mask_Anf_X4_5 					<= s_Anforder_Maske(4);
	Mask_Anf_X6	 					<= s_Anforder_Maske(5);
	Mask_Anf_X7	 					<= s_Anforder_Maske(6);
	Mask_Anf_X8	 					<= s_Anforder_Maske(7);
	Mask_Anf_Y7	 					<= s_Anforder_Maske(8);
	-- frei	 						<= s_Anforder_Maske(9);
	Mask_Anf_Z6	 					<= s_Anforder_Maske(10);
	Mask_Anf_UU	 					<= s_Anforder_Maske(11);
	Mask_Anf_M1	 					<= s_Anforder_Maske(12);
	Mask_Anf_M2	 					<= s_Anforder_Maske(13);
	Mask_Anf_M3	 					<= s_Anforder_Maske(14);
	Mask_Anf_SIS	 				<= s_Anforder_Maske(15);
			
	--+-------------------------------------+
	--|  Die Anforderungs-Eingänge			|
	--+-------------------------------------+
	-- Belegung vom 27.11.2008 mit neuem Signal Exp_Anf_SIS			
	Exp_Anf_X0					<= not Anforder_In(0);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X1					<= not Anforder_In(1);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X2					<= not Anforder_In(2);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X3					<= not Anforder_In(3);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X4_5				<= not Anforder_In(4);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X6					<= not Anforder_In(5);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X7					<= not Anforder_In(6);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_X8					<= not Anforder_In(7);		-- kein Strom == 0 Volt, soll der Aktive Pegel sein	-- V06: not..
	Exp_Anf_Y7					<= not Anforder_In(8);		--	V06: not..									--
	-- frei						<= not Anforder_In(9);		--	V06: not..									--
	Exp_Anf_Z6					<= not Anforder_In(10);		--	V06: not..									--
	Exp_Anf_UU					<= not Anforder_In(11);		--	V06: not..									--
	Exp_Anf_M1					<= not Anforder_In(12);		--	V06: not..									--
	Exp_Anf_M2					<= not Anforder_In(13);		--	
	Exp_Anf_M3					<= not Anforder_In(14);		--	
	Exp_Anf_SIS					<= not Anforder_In(15);		--	neu am 27.11.2008
--+-------------------------------------------------------------------------------------------------+
--|	Die Anforder-Eingänge werden hier zusammengefasst, damit sie von der SE gelesen werden können	|
--+-------------------------------------------------------------------------------------------------+
	Anforder_Inputs		<= (
							Exp_Anf_SIS	&	Exp_Anf_M3	&	Exp_Anf_M2	&	Exp_Anf_M1	&	-- Bit 15..12
							Exp_Anf_UU 	&	Exp_Anf_Z6	& 	'0'			&	Exp_Anf_Y7	&	-- Bit 11..8
							Exp_Anf_X8	&	Exp_Anf_X7	&	Exp_Anf_X6	&	Exp_Anf_X4_5 &	-- Bit 7..4
							Exp_Anf_X3	&	Exp_Anf_X2	&	Exp_Anf_X1	&	Exp_Anf_X0		-- Bit 3..0
							);
	
	Off_UU_Out			<=	(Exp_Anf_UU and not Mask_Anf_UU);
	

	Off_Anforderung_Out	<=  (Exp_Anf_X0 and not Mask_Anf_X0)
						or (Exp_Anf_X1 and not Mask_Anf_X1)
						or (Exp_Anf_X2 and not Mask_Anf_X2)
						or (Exp_Anf_X3 and not Mask_Anf_X3)
						or (Exp_Anf_X4_5 and not Mask_Anf_X4_5)
						or (Exp_Anf_X6 and not Mask_Anf_X6)
						or (Exp_Anf_X7 and not Mask_Anf_X7)
						or (Exp_Anf_X8 and not Mask_Anf_X8)
						or (Exp_Anf_Y7 and not Mask_Anf_Y7)
						or (Exp_Anf_Z6 and not Mask_Anf_Z6)
						or (Exp_Anf_M1 and not Mask_Anf_M1)
						or (Exp_Anf_M2 and not Mask_Anf_M2)
						or (Exp_Anf_M3 and not Mask_Anf_M3)
						or (Exp_Anf_SIS and not Mask_Anf_SIS);
						
	nMask_Anf_SIS 	<= not Mask_Anf_SIS;
	
	nMask_Anf_U		<=	not Mask_Anf_UU;

	nMask_Anf_X		<=	not 
						(	Mask_Anf_X0
							or	Mask_Anf_X1
							or	Mask_Anf_X2
							or	Mask_Anf_X3
							or	Mask_Anf_X4_5
							or	Mask_Anf_X6
							or	Mask_Anf_X7
							or	Mask_Anf_X8
						);

	nMask_Anf_Y		<=	not (Mask_Anf_Y7);

	nMask_Anf_Z		<=	not (Mask_Anf_Z6);
	
	nMask_Anf_M		<=	not	(Mask_Anf_M1 or Mask_Anf_M2 or Mask_Anf_M3);
	
		

end chopper_m2_logic_arch;