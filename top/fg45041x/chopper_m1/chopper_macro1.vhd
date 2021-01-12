-- Copyright (C) 1991-2010 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions
-- and other software and tools, and its AMPP partner logic
-- functions, and any output files from any of the foregoing
-- (including device programming or simulation files), and any
-- associated documentation or information are expressly subject
-- to the terms and conditions of the Altera Program License
-- Subscription Agreement, Altera MegaCore Function License
-- Agreement, or other applicable license agreement, including,
-- without limitation, that your use is for the sole purpose of
-- programming logic devices manufactured by Altera and sold by
-- Altera or its authorized distributors.  Please refer to the
-- applicable agreement for further details.

-- PROGRAM		"Quartus II"
-- VERSION		"Version 9.1 Build 350 03/24/2010 Service Pack 2 SJ Full Version"
-- CREATED		"Fri Aug 27 12:28:56 2010"

LIBRARY ieee;
USE ieee.std_logic_1164.all;

LIBRARY work;

ENTITY chopper_macro1 IS
GENERIC (Clk_in_HZ : INTEGER := 200000000;
		Test : INTEGER := 0
		);
	PORT
	(
		RD_Activ :  IN  STD_LOGIC;
		WR_Activ :  IN  STD_LOGIC;
		Skal_OK :  IN  STD_LOGIC;
		Reset :  IN  STD_LOGIC;
		Clk :  IN  STD_LOGIC;
		Off_UU_In :  IN  STD_LOGIC;
		Off_Anforderung_In :  IN  STD_LOGIC;
		Beam_Control_In :  IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Data_WR :  IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Strahlalarm_In :  IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Sub_Adr_Sync :  IN  STD_LOGIC_VECTOR(7 DOWNTO 1);
		Chop_m1_Rd_Activ :  OUT  STD_LOGIC;
		Chop1_Macro1_Activ :  OUT  STD_LOGIC;
		Chop_m1_Test_Vers_Aktiv :  OUT  STD_LOGIC;
		Strahlweg_Reg_WR :  OUT  STD_LOGIC;
		Strahlweg_Reg_RD :  OUT  STD_LOGIC;
		Strahlweg_Maske_RD :  OUT  STD_LOGIC;
		Global_Status_RD :  OUT  STD_LOGIC;
		Chop_DT_to_MB :  OUT  STD_LOGIC;
		Interlock_to_SE_RD :  OUT  STD_LOGIC;
		Beam_Control_Out :  OUT  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Chop_m1_LEDs :  OUT  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Chop_m1_RD_data :  OUT  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Trafo_Timing_Out :  OUT  STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END chopper_macro1;

ARCHITECTURE bdf_type OF chopper_macro1 IS

COMPONENT chopper_m1_adr_deco
	PORT(RD_Activ : IN STD_LOGIC;
		 WR_Activ : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 Sub_Adr_Sync : IN STD_LOGIC_VECTOR(7 DOWNTO 1);
		 Strahlweg_Reg_Rd : OUT STD_LOGIC;
		 Strahlweg_Reg_Wr : OUT STD_LOGIC;
		 Strahlweg_Maske_Rd : OUT STD_LOGIC;
		 Strahlweg_Maske_Wr : OUT STD_LOGIC;
		 Interlock_to_SE_RD : OUT STD_LOGIC;
		 Global_Status_RD : OUT STD_LOGIC;
		 HSI_act_pos_edge_RD : OUT STD_LOGIC;
		 HSI_neg_edge_RD : OUT STD_LOGIC;
		 HSI_act_neg_edge_RD : OUT STD_LOGIC;
		 HLI_act_pos_edge_RD : OUT STD_LOGIC;
		 HLI_neg_edge_RD : OUT STD_LOGIC;
		 HLI_act_neg_edge_RD : OUT STD_LOGIC;
		 Interlock_Reg_WR : OUT STD_LOGIC;
		 Interlock_Reg_RD : OUT STD_LOGIC;
		 TK8_Delay_WR : OUT STD_LOGIC;
		 TK8_Delay_RD : OUT STD_LOGIC;
		 Chop_Macro1_Activ : OUT STD_LOGIC;
		 Chop_RD_Activ : OUT STD_LOGIC;
		 Chop_DT_to_MB : OUT STD_LOGIC
	);
END COMPONENT;

COMPONENT chopper_m1_logic
GENERIC (Clk_in_HZ : INTEGER;
			Test : INTEGER
			);
	PORT(Skal_OK : IN STD_LOGIC;
		 Strahlweg_Reg_WR : IN STD_LOGIC;
		 Strahlweg_Reg_RD : IN STD_LOGIC;
		 Strahlweg_Maske_WR : IN STD_LOGIC;
		 Interlock_Reg_WR : IN STD_LOGIC;
		 TK8_Delay_WR : IN STD_LOGIC;
		 CLK : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 Off_Anforderung_In : IN STD_LOGIC;
		 Off_UU_In : IN STD_LOGIC;
		 Beam_Control_In : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Data_WR : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Strahlalarm_In : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Beam_Control_Out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Chop_m1_LEDs : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_act_neg_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_act_pos_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_neg_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_act_neg_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_act_pos_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_neg_latch_out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Interlock_Reg : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Interlock_to_SE : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Strahlweg_Maske : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Strahlweg_Reg : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 TK8_Delay : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Trafo_Timing_Out : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

COMPONENT chopper_m1_rd_reg
	PORT(Strahlweg_Reg_RD : IN STD_LOGIC;
		 Strahlweg_Maske_RD : IN STD_LOGIC;
		 Interlock_to_SE_RD : IN STD_LOGIC;
		 Global_Status_RD : IN STD_LOGIC;
		 HSI_act_pos_edge_RD : IN STD_LOGIC;
		 HSI_neg_edge_RD : IN STD_LOGIC;
		 HSI_act_neg_edge_RD : IN STD_LOGIC;
		 HLI_act_pos_edge_RD : IN STD_LOGIC;
		 HLI_neg_edge_RD : IN STD_LOGIC;
		 HLI_act_neg_edge_RD : IN STD_LOGIC;
		 Interlock_Reg_RD : IN STD_LOGIC;
		 TK8_Delay_RD : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 Global_Status : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_act_neg_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_act_pos_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HLI_neg_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_act_neg_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_act_pos_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 HSI_neg_edge : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Interlock_Reg : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Interlock_to_SE : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Strahlweg_Maske : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Strahlweg_Reg : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 TK8_Delay : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Chop_RD_Data : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

COMPONENT global_stat
	PORT(Chopper_Vers : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 Frei : IN STD_LOGIC_VECTOR(15 DOWNTO 8);
		 Global_Status : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

COMPONENT chop_m1_vers
GENERIC (Test : INTEGER
			);
	PORT(		 Test_Vers_Aktiv : OUT STD_LOGIC;
		 chop_m1_vers : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	);
END COMPONENT;

SIGNAL	Chop_Macro1_Activ :  STD_LOGIC;
SIGNAL	Global_Status :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Global_Status_RD_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	HLI_act_neg_edge_RD :  STD_LOGIC;
SIGNAL	HLI_act_neg_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	HLI_act_pos_edge_RD :  STD_LOGIC;
SIGNAL	HLI_act_pos_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	HLI_neg_edge_RD :  STD_LOGIC;
SIGNAL	HLI_neg_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	HSI_act_neg_edge_RD :  STD_LOGIC;
SIGNAL	HSI_act_neg_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	HSI_act_pos_edge_RD :  STD_LOGIC;
SIGNAL	HSI_act_pos_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	HSI_neg_edge_RD :  STD_LOGIC;
SIGNAL	HSI_neg_latch :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Interlock_Reg :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Interlock_Reg_RD :  STD_LOGIC;
SIGNAL	Interlock_Reg_WR :  STD_LOGIC;
SIGNAL	Interlock_to_SE :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Interlock_to_SE_RD_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Strahlweg_Maske :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Strahlweg_Maske_RD_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Strahlweg_Maske_WR :  STD_LOGIC;
SIGNAL	Strahlweg_Reg :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Strahlweg_Reg_RD_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Strahlweg_Reg_WR_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	TK8_Delay :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	TK8_Delay_RD :  STD_LOGIC;
SIGNAL	TK8_Delay_WR :  STD_LOGIC;
SIGNAL	SYNTHESIZED_WIRE_0 :  STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL	SYNTHESIZED_WIRE_1 :  STD_LOGIC_VECTOR(0 TO 7);


BEGIN
SYNTHESIZED_WIRE_1 <= "00000000";



b2v_chop_m1_adr_deco : chopper_m1_adr_deco
PORT MAP(RD_Activ => RD_Activ,
		 WR_Activ => WR_Activ,
		 clk => Clk,
		 Sub_Adr_Sync => Sub_Adr_Sync,
		 Strahlweg_Reg_Rd => Strahlweg_Reg_RD_ALTERA_SYNTHESIZED,
		 Strahlweg_Reg_Wr => Strahlweg_Reg_WR_ALTERA_SYNTHESIZED,
		 Strahlweg_Maske_Rd => Strahlweg_Maske_RD_ALTERA_SYNTHESIZED,
		 Strahlweg_Maske_Wr => Strahlweg_Maske_WR,
		 Interlock_to_SE_RD => Interlock_to_SE_RD_ALTERA_SYNTHESIZED,
		 Global_Status_RD => Global_Status_RD_ALTERA_SYNTHESIZED,
		 HSI_act_pos_edge_RD => HSI_act_pos_edge_RD,
		 HSI_neg_edge_RD => HSI_neg_edge_RD,
		 HSI_act_neg_edge_RD => HSI_act_neg_edge_RD,
		 HLI_act_pos_edge_RD => HLI_act_pos_edge_RD,
		 HLI_neg_edge_RD => HLI_neg_edge_RD,
		 HLI_act_neg_edge_RD => HLI_act_neg_edge_RD,
		 Interlock_Reg_WR => Interlock_Reg_WR,
		 Interlock_Reg_RD => Interlock_Reg_RD,
		 TK8_Delay_WR => TK8_Delay_WR,
		 TK8_Delay_RD => TK8_Delay_RD,
		 Chop_RD_Activ => Chop_m1_Rd_Activ,
		 Chop_DT_to_MB => Chop_DT_to_MB);


b2v_chop_m1_logic : chopper_m1_logic
GENERIC MAP(Clk_in_HZ => 200000000,
			Test => 0
			)
PORT MAP(Skal_OK => Skal_OK,
		 Strahlweg_Reg_WR => Strahlweg_Reg_WR_ALTERA_SYNTHESIZED,
		 Strahlweg_Reg_RD => Strahlweg_Reg_RD_ALTERA_SYNTHESIZED,
		 Strahlweg_Maske_WR => Strahlweg_Maske_WR,
		 Interlock_Reg_WR => Interlock_Reg_WR,
		 TK8_Delay_WR => TK8_Delay_WR,
		 CLK => Clk,
		 Reset => Reset,
		 Off_Anforderung_In => Off_Anforderung_In,
		 Off_UU_In => Off_UU_In,
		 Beam_Control_In => Beam_Control_In,
		 Data_WR => Data_WR,
		 Strahlalarm_In => Strahlalarm_In,
		 Beam_Control_Out => Beam_Control_Out,
		 Chop_m1_LEDs => Chop_m1_LEDs,
		 HLI_act_neg_latch_out => HLI_act_neg_latch,
		 HLI_act_pos_latch_out => HLI_act_pos_latch,
		 HLI_neg_latch_out => HLI_neg_latch,
		 HSI_act_neg_latch_out => HSI_act_neg_latch,
		 HSI_act_pos_latch_out => HSI_act_pos_latch,
		 HSI_neg_latch_out => HSI_neg_latch,
		 Interlock_Reg => Interlock_Reg,
		 Interlock_to_SE => Interlock_to_SE,
		 Strahlweg_Maske => Strahlweg_Maske,
		 Strahlweg_Reg => Strahlweg_Reg,
		 TK8_Delay => TK8_Delay,
		 Trafo_Timing_Out => Trafo_Timing_Out);


b2v_Chopper_m1_rd_reg : chopper_m1_rd_reg
PORT MAP(Strahlweg_Reg_RD => Strahlweg_Reg_RD_ALTERA_SYNTHESIZED,
		 Strahlweg_Maske_RD => Strahlweg_Maske_RD_ALTERA_SYNTHESIZED,
		 Interlock_to_SE_RD => Interlock_to_SE_RD_ALTERA_SYNTHESIZED,
		 Global_Status_RD => Global_Status_RD_ALTERA_SYNTHESIZED,
		 HSI_act_pos_edge_RD => HSI_act_pos_edge_RD,
		 HSI_neg_edge_RD => HSI_neg_edge_RD,
		 HSI_act_neg_edge_RD => HSI_act_neg_edge_RD,
		 HLI_act_pos_edge_RD => HLI_act_pos_edge_RD,
		 HLI_neg_edge_RD => HLI_neg_edge_RD,
		 HLI_act_neg_edge_RD => HLI_act_neg_edge_RD,
		 Interlock_Reg_RD => Interlock_Reg_RD,
		 TK8_Delay_RD => TK8_Delay_RD,
		 clk => Clk,
		 Global_Status => Global_Status,
		 HLI_act_neg_edge => HLI_act_neg_latch,
		 HLI_act_pos_edge => HLI_act_pos_latch,
		 HLI_neg_edge => HLI_neg_latch,
		 HSI_act_neg_edge => HSI_act_neg_latch,
		 HSI_act_pos_edge => HSI_act_pos_latch,
		 HSI_neg_edge => HSI_neg_latch,
		 Interlock_Reg => Interlock_Reg,
		 Interlock_to_SE => Interlock_to_SE,
		 Strahlweg_Maske => Strahlweg_Maske,
		 Strahlweg_Reg => Strahlweg_Reg,
		 TK8_Delay => TK8_Delay,
		 Chop_RD_Data => Chop_m1_RD_data);


b2v_inst3 : global_stat
PORT MAP(Chopper_Vers => SYNTHESIZED_WIRE_0,
		 Frei => SYNTHESIZED_WIRE_1,
		 Global_Status => Global_Status);


b2v_inst4 : chop_m1_vers
GENERIC MAP(Test => 0
			)
PORT MAP(		 Test_Vers_Aktiv => Chop_m1_Test_Vers_Aktiv,
		 chop_m1_vers => SYNTHESIZED_WIRE_0);


Strahlweg_Reg_WR <= Strahlweg_Reg_WR_ALTERA_SYNTHESIZED;
Strahlweg_Reg_RD <= Strahlweg_Reg_RD_ALTERA_SYNTHESIZED;
Strahlweg_Maske_RD <= Strahlweg_Maske_RD_ALTERA_SYNTHESIZED;
Global_Status_RD <= Global_Status_RD_ALTERA_SYNTHESIZED;
Interlock_to_SE_RD <= Interlock_to_SE_RD_ALTERA_SYNTHESIZED;

END bdf_type;
