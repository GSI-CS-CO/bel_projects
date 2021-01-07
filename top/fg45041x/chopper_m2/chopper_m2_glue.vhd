-- Zuweisen der Ausgaenge auf Chop_macro2_Bus und Chop_m2_LEDs

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity chopper_m2_glue is
	port (
			Off_UU_Out:				in	std_logic;
			Off_Anforderung_Out:	in	std_logic;
			
			Maske_Anf_SIS:			in 	std_logic;
			Maske_Anf_U:			in	std_logic;
			Maske_Anf_M:			in	std_logic;
			Maske_Anf_X:			in	std_logic;
			Maske_Anf_Y:			in	std_logic;
			Maske_Anf_Z:			in	std_logic;
			
			
			Chop_Macro2_Bus:		out std_logic_vector(1 downto 0);
			Chop_m2_LEDs:			out std_logic_vector(15 downto 0)
			
		);
end chopper_m2_glue;

architecture chopper_m2_glue_arch of chopper_m2_glue is	
begin
	Chop_Macro2_Bus <= Off_UU_Out & Off_Anforderung_Out;
	
	Chop_m2_LEDs <= Off_UU_Out &
					Off_Anforderung_Out &
					"00000000" &
					Maske_Anf_SIS &
					Maske_Anf_U &
					Maske_Anf_M &
					Maske_Anf_X &
					Maske_Anf_Y &
					Maske_Anf_Z;

end chopper_m2_glue_arch;
