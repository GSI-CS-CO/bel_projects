-- Copyright (C) 1991-2005 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic       
-- functions, and any output files any of the foregoing           
-- (including device programming or simulation files), and any    
-- associated documentation or information are expressly subject  
-- to the terms and conditions of the Altera Program License      
-- Subscription Agreement, Altera MegaCore Function License       
-- Agreement, or other applicable license agreement, including,   
-- without limitation, that your use is for the sole purpose of   
-- programming logic devices manufactured by Altera and sold by   
-- Altera or its authorized distributors.  Please refer to the    
-- applicable agreement for further details.

-- PROGRAM "Quartus II"
-- VERSION "Version 5.0 Build 168 06/22/2005 Service Pack 1 SJ Full Version"

LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY chopper_macro2_vhdl IS 
	port
	(
		RD_Activ :  IN  STD_LOGIC;
		WR_Activ :  IN  STD_LOGIC;
		Skal_OK :  IN  STD_LOGIC;
		Clk :  IN  STD_LOGIC;
		Reset :  IN  STD_LOGIC;
		Anforder_In :  IN  STD_LOGIC_VECTOR(15 downto 0);
		Data_WR :  IN  STD_LOGIC_VECTOR(15 downto 0);
		Sub_Adr_Sync :  IN  STD_LOGIC_VECTOR(7 downto 1);
		Chop_m2_Rd_Activ :  OUT  STD_LOGIC;
		Chop_Macro2_Activ :  OUT  STD_LOGIC;
		Chop_m2_DT_to_MB :  OUT  STD_LOGIC;
		Chop_m2_Test_Vers_Aktiv :  OUT  STD_LOGIC;
		Anforder_Inputs :  OUT  STD_LOGIC_VECTOR(15 downto 0);
		Chop_m2_LEDs :  OUT  STD_LOGIC_VECTOR(15 downto 0);
		Chop_m2_RD_Data :  OUT  STD_LOGIC_VECTOR(15 downto 0);
		Chop_Macro2_Bus_IO :  OUT  STD_LOGIC_VECTOR(1 downto 0);
		EMI_Schwelle :  OUT  STD_LOGIC_VECTOR(15 downto 0)
	);
END chopper_macro2_vhdl;

ARCHITECTURE bdf_type OF chopper_macro2_vhdl IS 

component chop_m2_vers
GENERIC (Test:INTEGER);
	PORT(		 Test_Vers_Aktiv : OUT STD_LOGIC;
		 chop_m2_vers : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

component chopper_m2_adr_deco
	PORT(RD_Activ : IN STD_LOGIC;
		 WR_Activ : IN STD_LOGIC;
		 Sel_Chop_Macro2 : IN STD_LOGIC;
		 Macro_Global_Off : IN STD_LOGIC;
		 CLK : IN STD_LOGIC;
		 Sub_Adr_Sync : IN STD_LOGIC_VECTOR(7 downto 1);
		 Anforder_Maske_WR : OUT STD_LOGIC;
		 Anforder_Inputs_RD : OUT STD_LOGIC;
		 Global_Status_RD : OUT STD_LOGIC;
		 EMI_Schwelle_WR : OUT STD_LOGIC;
		 EMI_Schwelle_RD : OUT STD_LOGIC;
		 Chop_m2_Rd_Activ : OUT STD_LOGIC;
		 Chop_Macro2_Activ : OUT STD_LOGIC;
		 Chop_m2_DT_to_MB : OUT STD_LOGIC
	);
end component;

component chopper_m2_rd_reg
	PORT(Global_Status_RD : IN STD_LOGIC;
		 Anforder_Inputs_RD : IN STD_LOGIC;
		 EMI_Schwelle_RD : IN STD_LOGIC;
		 Clk : IN STD_LOGIC;
		 Anforder_Inputs : IN STD_LOGIC_VECTOR(15 downto 0);
		 EMI_Schwelle : IN STD_LOGIC_VECTOR(15 downto 0);
		 Global_Status : IN STD_LOGIC_VECTOR(15 downto 0);
		 Chop_RD_Data : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component chopper_m2_logic
	PORT(Skal_OK : IN STD_LOGIC;
		 Anforder_Maske_WR : IN STD_LOGIC;
		 EMI_Schwelle_WR : IN STD_LOGIC;
		 CLK : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 Anforder_In : IN STD_LOGIC_VECTOR(15 downto 0);
		 Data_WR : IN STD_LOGIC_VECTOR(15 downto 0);
		 Off_Anforderung_Out : OUT STD_LOGIC;
		 Off_UU_Out : OUT STD_LOGIC;
		 nMask_Anf_U : OUT STD_LOGIC;
		 nMask_Anf_X : OUT STD_LOGIC;
		 nMask_Anf_Y : OUT STD_LOGIC;
		 nMask_Anf_Z : OUT STD_LOGIC;
		 Anforder_Inputs : OUT STD_LOGIC_VECTOR(15 downto 0);
		 EMI_Schwelle : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component global_stat
	PORT(Chopper_Vers : IN STD_LOGIC_VECTOR(7 downto 0);
		 Frei : IN STD_LOGIC_VECTOR(15 downto 8);
		 Global_Status : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component chopper_m2_glue
	PORT(Off_UU_Out : IN STD_LOGIC;
		 Off_Anforderung_Out : IN STD_LOGIC;
		 Maske_Anf_U : IN STD_LOGIC;
		 Maske_Anf_X : IN STD_LOGIC;
		 Maske_Anf_Y : IN STD_LOGIC;
		 Maske_Anf_Z : IN STD_LOGIC;
		 Chop_m2_LEDs : OUT STD_LOGIC_VECTOR(15 downto 0);
		 Chop_Macro2_Bus : OUT STD_LOGIC_VECTOR(1 downto 0)
	);
end component;

signal	Anforder_Inputs_ALTERA_SYNTHESIZED :  STD_LOGIC_VECTOR(15 downto 0);
signal	Anforder_Inputs_RD :  STD_LOGIC;
signal	Anforder_Maske_WR :  STD_LOGIC;
signal	EMI_Schwelle_ALTERA_SYNTHESIZED :  STD_LOGIC_VECTOR(15 downto 0);
signal	EMI_Schwelle_RD :  STD_LOGIC;
signal	EMI_Schwelle_WR :  STD_LOGIC;
signal	Global_Status :  STD_LOGIC_VECTOR(15 downto 0);
signal	Global_Status_RD :  STD_LOGIC;
signal	nMask_Anf_U :  STD_LOGIC;
signal	nMask_Anf_X :  STD_LOGIC;
signal	nMask_Anf_Y :  STD_LOGIC;
signal	nMask_Anf_Z :  STD_LOGIC;
signal	Off_Anforderung_Out :  STD_LOGIC;
signal	Off_UU_Out :  STD_LOGIC;
signal	Sel_Chop_Macro2 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_0 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_1 :  STD_LOGIC_VECTOR(7 downto 0);
signal	SYNTHESIZED_WIRE_2 :  STD_LOGIC_VECTOR(0 to 7);


BEGIN 
SYNTHESIZED_WIRE_0 <= '0';
SYNTHESIZED_WIRE_2 <= "00000000";



b2v_inst : chop_m2_vers
GENERIC MAP(Test => 0)
PORT MAP(		 Test_Vers_Aktiv => Chop_m2_Test_Vers_Aktiv,
		 chop_m2_vers => SYNTHESIZED_WIRE_1);

b2v_inst1 : chopper_m2_adr_deco
PORT MAP(RD_Activ => RD_Activ,
		 WR_Activ => WR_Activ,
		 Sel_Chop_Macro2 => Sel_Chop_Macro2,
		 Macro_Global_Off => SYNTHESIZED_WIRE_0,
		 CLK => Clk,
		 Sub_Adr_Sync => Sub_Adr_Sync,
		 Anforder_Maske_WR => Anforder_Maske_WR,
		 Anforder_Inputs_RD => Anforder_Inputs_RD,
		 Global_Status_RD => Global_Status_RD,
		 EMI_Schwelle_WR => EMI_Schwelle_WR,
		 EMI_Schwelle_RD => EMI_Schwelle_RD,
		 Chop_m2_Rd_Activ => Chop_m2_Rd_Activ,
		 Chop_Macro2_Activ => Chop_Macro2_Activ,
		 Chop_m2_DT_to_MB => Chop_m2_DT_to_MB);

b2v_inst2 : chopper_m2_rd_reg
PORT MAP(Global_Status_RD => Global_Status_RD,
		 Anforder_Inputs_RD => Anforder_Inputs_RD,
		 EMI_Schwelle_RD => EMI_Schwelle_RD,
		 Clk => Clk,
		 Anforder_Inputs => Anforder_Inputs_ALTERA_SYNTHESIZED,
		 EMI_Schwelle => EMI_Schwelle_ALTERA_SYNTHESIZED,
		 Global_Status => Global_Status,
		 Chop_RD_Data => Chop_m2_RD_Data);

b2v_inst3 : chopper_m2_logic
PORT MAP(Skal_OK => Skal_OK,
		 Anforder_Maske_WR => Anforder_Maske_WR,
		 EMI_Schwelle_WR => EMI_Schwelle_WR,
		 CLK => Clk,
		 Reset => Reset,
		 Anforder_In => Anforder_In,
		 Data_WR => Data_WR,
		 Off_Anforderung_Out => Off_Anforderung_Out,
		 Off_UU_Out => Off_UU_Out,
		 nMask_Anf_U => nMask_Anf_U,
		 nMask_Anf_X => nMask_Anf_X,
		 nMask_Anf_Y => nMask_Anf_Y,
		 nMask_Anf_Z => nMask_Anf_Z,
		 Anforder_Inputs => Anforder_Inputs_ALTERA_SYNTHESIZED,
		 EMI_Schwelle => EMI_Schwelle_ALTERA_SYNTHESIZED);

b2v_inst4 : global_stat
PORT MAP(Chopper_Vers => SYNTHESIZED_WIRE_1,
		 Frei => SYNTHESIZED_WIRE_2,
		 Global_Status => Global_Status);

b2v_inst5 : chopper_m2_glue
PORT MAP(Off_UU_Out => Off_UU_Out,
		 Off_Anforderung_Out => Off_Anforderung_Out,
		 Maske_Anf_U => nMask_Anf_U,
		 Maske_Anf_X => nMask_Anf_X,
		 Maske_Anf_Y => nMask_Anf_Y,
		 Maske_Anf_Z => nMask_Anf_Z,
		 Chop_m2_LEDs => Chop_m2_LEDs,
		 Chop_Macro2_Bus => Chop_Macro2_Bus_IO);
Anforder_Inputs <= Anforder_Inputs_ALTERA_SYNTHESIZED;
EMI_Schwelle <= EMI_Schwelle_ALTERA_SYNTHESIZED;

END; 