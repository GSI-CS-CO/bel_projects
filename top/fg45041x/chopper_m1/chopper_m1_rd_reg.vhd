--TITLE "'chopper_m1_rd_reg' => Chopper-Read-Register-File, Autor: W.Panschow, Stand: 13.02.02, Vers: V02 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity chopper_m1_rd_reg is
	port (
		Strahlweg_Reg       : IN std_logic_vector(15 downto 0);
		Strahlweg_Reg_RD    : IN std_logic;
		Strahlweg_Maske     : IN std_logic_vector(15 downto 0);
		Strahlweg_Maske_RD  : IN std_logic;
		Interlock_to_SE     : IN std_logic_vector(15 downto 0);
		Interlock_to_SE_RD  : IN std_logic;
		Global_Status       : IN std_logic_vector(15 downto 0)  := X"0000";
		Global_Status_RD    : IN std_logic;

		HSI_act_pos_edge    : IN std_logic_vector(15 downto 0);
		HSI_act_pos_edge_RD : IN std_logic;

		HSI_neg_edge        : IN std_logic_vector(15 downto 0);
		HSI_neg_edge_RD     : IN std_logic;

		HSI_act_neg_edge    : IN std_logic_vector(15 downto 0);
		HSI_act_neg_edge_RD : IN std_logic;

		HLI_act_pos_edge    : IN std_logic_vector(15 downto 0);
		HLI_act_pos_edge_RD : IN std_logic;

		HLI_neg_edge        : IN std_logic_vector(15 downto 0);
		HLI_neg_edge_RD     : IN std_logic;

		HLI_act_neg_edge    : IN std_logic_vector(15 downto 0);
		HLI_act_neg_edge_RD : IN std_logic;

		Interlock_Reg       : IN std_logic_vector(15 downto 0);
		Interlock_Reg_RD    : IN std_logic;

		TK8_Delay           : IN std_logic_vector(15 downto 0);
		TK8_Delay_RD        : IN std_logic;

		clk                 : IN std_logic;

		Chop_RD_Data        : OUT std_logic_vector(15 downto 0)
		);


end chopper_m1_rd_reg;

architecture chopper_m1_rd_reg_arch of chopper_m1_rd_reg is
begin

		Chop_RD_Data <= Strahlweg_Reg when Strahlweg_Reg_RD = '1' else
			Strahlweg_Maske  when Strahlweg_Maske_RD  = '1' else
			Interlock_to_SE  when Interlock_to_SE_RD  = '1' else
			Global_Status    when Global_Status_RD    = '1' else
			HSI_act_pos_edge when HSI_act_pos_edge_RD = '1' else
			HSI_neg_edge     when HSI_neg_edge_RD     = '1' else
			HSI_act_neg_edge when HSI_act_neg_edge_RD = '1' else
			HLI_act_pos_edge when HLI_act_pos_edge_RD = '1' else
			HLI_neg_edge     when HLI_neg_edge_RD     = '1' else
			HLI_act_neg_edge when HLI_act_neg_edge_RD = '1' else
			Interlock_Reg    when Interlock_Reg_RD    = '1' else
			TK8_Delay        when TK8_Delay_RD        = '1' else
			"XXXXXXXXXXXXXXXX";


end chopper_m1_rd_reg_arch;
