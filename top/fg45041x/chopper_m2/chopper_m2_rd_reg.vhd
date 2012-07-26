--"'chopper_m2_rd_reg' => Chopper-Macro2-Read-Register-File, Autor: W.Panschow, Stand: 29.04.03, Vers: V01 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity chopper_m2_rd_reg is
	port (
			Global_Status:				in std_logic_vector(15 downto 0) := x"0000";
			Global_Status_RD:			in std_logic;
			Anforder_Inputs:			in std_logic_vector(15 downto 0) := x"0000";
			Anforder_Inputs_RD:			in std_logic;
			EMI_Schwelle:				in std_logic_vector(15 downto 0);
			EMI_Schwelle_RD:			in std_logic;
			Clk:						in std_logic;

			Chop_RD_Data:				out std_logic_vector(15 downto 0)
		);
end chopper_m2_rd_reg;

architecture chopper_m2_rd_reg_arch of chopper_m2_rd_reg is

begin

	
	Chop_RD_Data <= Global_Status when Global_Status_RD = '1' else
					Anforder_Inputs when Anforder_Inputs_RD = '1' else
					EMI_Schwelle when EMI_Schwelle_RD = '1' else
					"XXXXXXXXXXXXXXXX";


end chopper_m2_rd_reg_arch;