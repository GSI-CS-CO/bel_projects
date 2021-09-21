-- 'chopper_m1_adr_deco' => Chopper-Macro1-Adress-Dekodierung,
-- Autor: W.Panschow, Stand: 29.04.03, Vers: V01 ";
-- Erweiterung 8.3.06, Delay Counter fuer Chopper Ausgaenge


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity chopper_m1_adr_deco is 
	port (
		Sub_Adr_Sync:			IN std_logic_vector(7 downto 1);		-- Vom Macro 'Modul_Bus' synchronisierte Modul-Bus-Subadressen[7..1]: Ausgang 'Sub_Adr_Sync[7..1]'.				--
		RD_Activ:				IN std_logic;
		WR_Activ:				IN std_logic;
		clk:					IN std_logic;							-- Sollte der Takt sein, der auch in den anderen Macros verwendet wird.											--
		Strahlweg_Reg_Rd:		OUT	std_logic;
		Strahlweg_Reg_Wr:		OUT	std_logic; 
		Strahlweg_Maske_Rd:		OUT	std_logic;	
		Strahlweg_Maske_Wr:		OUT	std_logic;
		Interlock_to_SE_RD:		OUT std_logic;
		Global_Status_RD:		OUT std_logic;
		
		HSI_act_pos_edge_RD:	OUT std_logic;
		HSI_neg_edge_RD:		OUT std_logic;
		HSI_act_neg_edge_RD:	OUT std_logic;
		
		HLI_act_pos_edge_RD:	OUT std_logic;
		HLI_neg_edge_RD:		OUT std_logic;
		HLI_act_neg_edge_RD:	OUT std_logic;
		
		Interlock_Reg_WR:		OUT std_logic;
		Interlock_Reg_RD:		OUT std_logic;
		
		TK8_Delay_WR:			OUT std_logic;
		TK8_Delay_RD:			OUT std_logic;
		
		
		Chop_Macro1_Activ:	OUT std_logic;
		Chop_RD_Activ:		OUT std_logic;
		Chop_DT_to_MB:		OUT	std_logic							-- Wird im Macro 'Modul_bus' mit 'Modul_bus'-internen DTACK-Gruenden Oder-Verknuepft. (Achtung dient direkt zur	--
																	-- Aktiverung des Modul-Bus-Datentreibers!)																		-- 
		);
		
CONSTANT C_Strahlweg_Reg_RW	: 	std_logic_vector(7 downto 0)	:= X"60";
CONSTANT C_Strahlweg_Maske_RW : std_logic_vector(7 downto 0)	:= X"62";
CONSTANT C_Interlock_to_SE_RD : std_logic_vector(7 downto 0)	:= X"64";
CONSTANT C_Global_Status_RD	: 	std_logic_vector(7 downto 0)	:= X"66";

CONSTANT C_HSI_act_pos_edge_RD: std_logic_vector(7 downto 0)    := X"6C"; 
CONSTANT C_HSI_neg_edge_RD: 	std_logic_vector(7 downto 0)    := X"6E"; 
CONSTANT C_HSI_act_neg_edge_RD: std_logic_vector(7 downto 0)    := X"70";

CONSTANT C_HLI_act_pos_edge_RD: std_logic_vector(7 downto 0)    := X"74";
CONSTANT C_HLI_neg_edge_RD: 	std_logic_vector(7 downto 0)    := X"76";
CONSTANT C_HLI_act_neg_edge_RD: std_logic_vector(7 downto 0)    := X"78";

CONSTANT C_Interlock_Reg_RW:	std_logic_vector(7 downto 0)	:= X"7A";
CONSTANT C_TK8_Delay_RW:		std_logic_vector(7 downto 0)	:= X"7C";

		
end chopper_m1_adr_deco;
		
architecture m1_adr_deco_arch of chopper_m1_adr_deco is



signal s_Strahlweg_Reg_Rd: 		std_logic;
signal s_Strahlweg_Reg_Wr: 		std_logic;
signal s_Strahlweg_Maske_Rd: 	std_logic;
signal s_Strahlweg_Maske_Wr: 	std_logic;
signal s_Interlock_to_SE_RD: 	std_logic;
signal s_Global_Status_RD: 		std_logic;

signal s_HSI_act_pos_edge_RD:	std_logic;
signal s_HSI_neg_edge_RD:		std_logic;
signal s_HSI_act_neg_edge_RD:	std_logic;
		
signal s_HLI_act_pos_edge_RD:	std_logic;
signal s_HLI_neg_edge_RD:		std_logic;
signal s_HLI_act_neg_edge_RD:	std_logic;

signal s_Interlock_Reg_RD:		std_logic;
signal s_Interlock_Reg_WR:		std_logic;

signal s_TK8_Delay_WR:			std_logic;
signal s_TK8_Delay_RD:			std_logic;

signal s_Chop_RD_Activ: 		std_logic;
signal s_WR_Dtack: 				std_logic;
signal s_RD_Dtack: 				std_logic;


begin

		address_decode: process (clk)
		begin	
				if (rising_edge(clk)) then
				
					s_Strahlweg_Reg_Rd <= '0';
					s_Strahlweg_Reg_Wr <= '0';
					s_Strahlweg_Maske_Rd <= '0';
					s_Strahlweg_Maske_Wr <= '0';
					s_Global_Status_RD <= '0';
					s_Interlock_to_SE_RD <= '0';
					
					s_HSI_act_pos_edge_RD <= '0';
					s_HSI_neg_edge_RD <= '0';
					s_HSI_act_neg_edge_RD <= '0';
					
					s_HLI_act_pos_edge_RD <= '0';
					s_HLI_neg_edge_RD <= '0';
					s_HLI_act_neg_edge_RD <= '0';
					
					s_Interlock_Reg_WR <= '0';
					s_Interlock_Reg_RD <= '0';
					
					s_TK8_Delay_WR <= '0';
					s_TK8_Delay_RD <= '0';
					
					s_Chop_RD_Activ <= '0';
					s_WR_Dtack <= '0';
					s_RD_Dtack <= '0';
					
						
					case Sub_Adr_Sync is
						when  C_Strahlweg_Reg_RW(7 downto 1) =>
							if RD_Activ = '1' then
								s_Strahlweg_Reg_Rd <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							elsif WR_Activ = '1' then
								s_Strahlweg_Reg_Wr <= '1';
								s_WR_Dtack <= '1';
							end if;
													
						when (C_Strahlweg_Maske_RW(7 downto 1)) =>
							if RD_Activ = '1' then
								s_Strahlweg_Maske_Rd <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							elsif WR_Activ = '1' then
								s_Strahlweg_Maske_Wr <= '1';
								s_WR_Dtack <= '1';
							end if;
							
						when (C_Interlock_to_SE_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_Interlock_to_SE_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
							
						when (C_Global_Status_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_Global_Status_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
							
						when (C_HSI_act_pos_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HSI_act_pos_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
						when (C_HSI_neg_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HSI_neg_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
							
						when (C_HSI_act_neg_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HSI_act_neg_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
						when (C_HLI_act_pos_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HLI_act_pos_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
						when (C_HLI_neg_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HLI_neg_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
						when (C_HLI_act_neg_edge_RD(7 downto 1)) =>
							if RD_Activ = '1' then
								s_HLI_act_neg_edge_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							end if;
						when (C_Interlock_Reg_RW(7 downto 1)) =>
							if RD_Activ = '1' then
								s_Interlock_Reg_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							elsif WR_Activ = '1' then
								s_Interlock_Reg_WR <= '1';
								s_WR_Dtack <= '1';
							end if;
						when (C_TK8_Delay_RW(7 downto 1)) =>
							if RD_Activ = '1' then
								s_TK8_Delay_RD <= '1';
								s_Chop_RD_Activ <= '1';
								s_RD_Dtack <= '1';
							elsif WR_Activ = '1' then
								s_TK8_Delay_WR <= '1';
								s_WR_Dtack <= '1';
							end if;
								
							
						when others =>	
							s_Strahlweg_Reg_Rd <= '0';
							s_Strahlweg_Reg_Wr <= '0';
							s_Strahlweg_Maske_Rd <= '0';
							s_Strahlweg_Maske_Wr <= '0';
							s_Global_Status_RD <= '0';
							s_Interlock_to_SE_RD <= '0';
							
							s_HSI_act_pos_edge_RD <= '0';
							s_HSI_neg_edge_RD <= '0';
							s_HSI_act_neg_edge_RD <= '0';
							
							s_HLI_act_pos_edge_RD <= '0';
							s_HLI_neg_edge_RD <= '0';
							s_HLI_act_neg_edge_RD <= '0';
							
							s_Interlock_Reg_WR <= '0';
							s_Interlock_Reg_RD <= '0';
							
							s_TK8_Delay_WR <= '0';
							s_TK8_Delay_RD <= '0';
							
							s_Chop_RD_Activ <= '0';
							s_WR_Dtack <= '0';
							s_RD_Dtack <= '0';
							
					end case;
				end if;
		end process;
			
		
	Chop_Macro1_Activ <= '1';
	
	Strahlweg_Reg_Rd	<= s_Strahlweg_Reg_Rd;
	Strahlweg_Reg_Wr	<= s_Strahlweg_Reg_Wr;
	Strahlweg_Maske_Rd	<= s_Strahlweg_Maske_Rd;
	Strahlweg_Maske_Wr	<= s_Strahlweg_Maske_Wr;
	Interlock_to_SE_RD	<= s_Interlock_to_SE_RD;
	Global_Status_RD	<= s_Global_Status_RD;
	
	HSI_act_pos_edge_RD <= s_HSI_act_pos_edge_RD;
	HSI_neg_edge_RD		<= s_HSI_neg_edge_RD;
	HSI_act_neg_edge_RD <= s_HSI_act_neg_edge_RD;
	
	HLI_act_pos_edge_RD <= s_HLI_act_pos_edge_RD;
	HLI_neg_edge_RD		<= s_HLI_neg_edge_RD;
	HLI_act_neg_edge_RD <= s_HLI_act_neg_edge_RD;
	
	Interlock_Reg_RD 	<= s_Interlock_Reg_RD;
	Interlock_Reg_WR 	<= s_Interlock_Reg_WR;
	
	TK8_Delay_RD		<= s_TK8_Delay_RD;
	TK8_Delay_WR		<= s_TK8_Delay_WR;
	
	Chop_RD_Activ 		<= s_Chop_RD_Activ;
	Chop_DT_to_MB		<= s_WR_Dtack or s_RD_Dtack;
	
end m1_adr_deco_arch;
										
											