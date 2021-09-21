-- "'chopper_m2_adr_deco' => Chopper-Macro2-Adress-Dekodierung, Autor: W.Panschow, Stand: 29.04.03, Vers: V01 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;



entity chopper_m2_adr_deco is
	port (
				Sub_Adr_Sync:			in std_logic_vector(7 downto 1);  	-- Vom Macro 'Modul_Bus' synchronisierte Modul-Bus-Subadressen[7..1]: Ausgang 'Sub_Adr_Sync[7..1]'.				--
				RD_Activ:				in std_logic;
				WR_Activ:				in std_logic;
				Sel_Chop_Macro2:		in std_logic;
				Macro_Global_Off:		in std_logic;
				CLK:					in std_logic;						-- Sollte der Takt sein, der auch in den anderen Macros verwendet wird.											--
				Anforder_Maske_WR:		out std_logic;
				Anforder_Inputs_RD:		out std_logic;
				Global_Status_RD:		out std_logic;
				EMI_Schwelle_WR:		out std_logic;
				EMI_Schwelle_RD:		out std_logic;
				Chop_m2_Rd_Activ:		out std_logic;
				Chop_Macro2_Activ:		out std_logic;
				Chop_m2_DT_to_MB:		out std_logic						-- Wird im Macro 'Modul_bus' mit 'Modul_bus'-internen DTACK-Gruenden Oder-Verknuepft. (Achtung dient direkt zur	--
																			-- Aktiverung des Modul-Bus-Datentreibers!)																		-- 
		);
		
	CONSTANT C_Global_Status_RD:		std_logic_vector(7 downto 0) := x"66";
	CONSTANT C_Anforder_Maske_WR:		std_logic_vector(7 downto 0) := x"68";
	CONSTANT C_Anforder_Inputs_RD:		std_logic_vector(7 downto 0) := x"6A";
	CONSTANT C_EMI_Schwelle_WR:			std_logic_vector(7 downto 0) := x"32";	-- Achtung die ist eigentlich die Adresse fuer K3-Schreiben!
	CONSTANT C_EMI_Schwelle_RD:			std_logic_vector(7 downto 0) := x"36";	-- Achtung die ist eigentlich die Adresse fuer K3-Lesen!
	
end chopper_m2_adr_deco;

architecture chopper_m2_adr_deco_arch of chopper_m2_adr_deco is

signal		s_Anforder_Maske_WR:	std_logic;
signal		s_Anforder_Inputs_RD:	std_logic;
signal		s_Global_Status_RD:		std_logic;
signal		s_EMI_Schwelle_WR:		std_logic;
signal		s_EMI_Schwelle_RD:		std_logic;

signal 		s_Chop_RD_Activ: 		std_logic;
signal 		s_WR_Dtack: 			std_logic;
signal 		s_RD_Dtack: 			std_logic;

begin

Strahlweg_Reg_Rd_ff: process (clk)
	begin	
		if rising_edge(clk) then
		
			s_Anforder_Maske_WR <= '0';
			s_Anforder_Inputs_RD <= '0';
			s_Global_Status_RD <= '0';
			s_EMI_Schwelle_WR <= '0';
			s_EMI_Schwelle_RD <= '0';
			s_Chop_RD_Activ <= '0';
			s_WR_Dtack <= '0';
			s_RD_Dtack <= '0';
			
			
			case Sub_Adr_Sync is
				when  (C_Anforder_Maske_WR(7 downto 1)) =>
					if WR_Activ = '1' then
						s_WR_Dtack <= '1';
						s_Anforder_Maske_WR <= '1';
					end if;
													
				when (C_Anforder_Inputs_RD(7 downto 1)) =>
					if RD_Activ = '1' then
						s_RD_Dtack <= '1';
						s_Chop_RD_Activ <= '1';
						s_Anforder_Inputs_RD <= '1';
					end if;
				
				
				when (C_EMI_Schwelle_WR(7 downto 1)) =>
					if WR_Activ = '1' then
						s_WR_Dtack  <= '1';
						s_EMI_Schwelle_WR <= '1';
					end if;
				
				when (C_EMI_Schwelle_RD(7 downto 1)) =>
					if RD_Activ = '1' then
						s_RD_Dtack <= '1';
						s_Chop_RD_Activ <= '1';
						s_EMI_Schwelle_RD <= '1';
					end if;
			
				when (C_Global_Status_RD(7 downto 1)) =>
					if RD_Activ = '1' then
						s_RD_Dtack <= '1';
						s_Chop_RD_Activ <= '1';
						s_Global_Status_RD <= '1';
					end if;
				
				when others =>	
					s_Anforder_Maske_WR <= '0';
					s_Anforder_Inputs_RD <= '0';
					s_Global_Status_RD <= '0';
					s_EMI_Schwelle_WR <= '0';
					s_EMI_Schwelle_RD <= '0';
					s_Chop_RD_Activ <= '0';
					s_WR_Dtack <= '0';
					s_RD_Dtack <= '0';
							
				end case;
			end if;
	end process;
	
	Chop_Macro2_Activ <= '1';
	
	Anforder_Maske_WR <=  s_Anforder_Maske_WR;		
	Anforder_Inputs_RD <= s_Anforder_Inputs_RD;
	EMI_Schwelle_WR	<= s_EMI_Schwelle_WR;
	EMI_Schwelle_RD	<= s_EMI_Schwelle_RD;
	Global_Status_RD <= s_Global_Status_RD;	
	Chop_m2_Rd_Activ <= s_Chop_RD_Activ;
	
	Chop_m2_DT_to_MB <= s_WR_Dtack or s_RD_Dtack;



end chopper_m2_adr_deco_arch;	
	