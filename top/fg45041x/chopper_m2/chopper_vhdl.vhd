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

ENTITY chopper_vhdl IS 
	port
	(
		A_RDnWR :  IN  STD_LOGIC;
		A_nDS :  IN  STD_LOGIC;
		A_VG_K1_INP :  IN  STD_LOGIC;
		A_VG_K0_INP :  IN  STD_LOGIC;
		A_GR0_APK_ID :  IN  STD_LOGIC;
		A_GR0_16BIT :  IN  STD_LOGIC;
		A_VG_K3_INP :  IN  STD_LOGIC;
		A_VG_K2_INP :  IN  STD_LOGIC;
		A_GR1_APK_ID :  IN  STD_LOGIC;
		A_GR1_16BIT :  IN  STD_LOGIC;
		A_nMB_Reset :  IN  STD_LOGIC;
		Kicker_Clk :  IN  STD_LOGIC;
		A_K0D_SPG :  IN  STD_LOGIC;
		A_K0C_SPG :  IN  STD_LOGIC;
		A_K1D_SPG :  IN  STD_LOGIC;
		A_K1C_SPG :  IN  STD_LOGIC;
		A_K2D_SPG :  IN  STD_LOGIC;
		A_K2C_SPG :  IN  STD_LOGIC;
		A_K3D_SPG :  IN  STD_LOGIC;
		A_K3C_SPG :  IN  STD_LOGIC;
		CTRL_LOAD :  IN  STD_LOGIC;
		CTRL_RES :  IN  STD_LOGIC;
		F_TCXO_In :  IN  STD_LOGIC;
		Loader_CLK :  IN  STD_LOGIC;
		A_I2C_SDA :  INOUT  STD_LOGIC;
		A_I2C_SCL :  INOUT  STD_LOGIC;
		A_A :  IN  STD_LOGIC_VECTOR(4 downto 0);
		A_AUX_A :  INOUT  STD_LOGIC_VECTOR(11 downto 0);
		A_AUX_B :  INOUT  STD_LOGIC_VECTOR(11 downto 0);
		A_AUX_C :  INOUT  STD_LOGIC_VECTOR(11 downto 0);
		A_Bus_IO :  INOUT  STD_LOGIC_VECTOR(5 downto 1);
		A_K0_D :  INOUT  STD_LOGIC_VECTOR(15 downto 0);
		A_K1_D :  INOUT  STD_LOGIC_VECTOR(15 downto 0);
		A_K2_D :  INOUT  STD_LOGIC_VECTOR(15 downto 0);
		A_K3_D :  INOUT  STD_LOGIC_VECTOR(15 downto 0);
		A_Mod_Data :  INOUT  STD_LOGIC_VECTOR(7 downto 0);
		A_nK0_CTRL :  INOUT  STD_LOGIC_VECTOR(2 downto 1);
		A_nK1_CTRL :  INOUT  STD_LOGIC_VECTOR(2 downto 1);
		A_nK2_CTRL :  INOUT  STD_LOGIC_VECTOR(2 downto 1);
		A_nK3_CTRL :  INOUT  STD_LOGIC_VECTOR(2 downto 1);
		A_Sub_Adr :  IN  STD_LOGIC_VECTOR(7 downto 0);
		A_Test :  INOUT  STD_LOGIC_VECTOR(15 downto 0);
		A_VG_A :  IN  STD_LOGIC_VECTOR(4 downto 0);
		A_VG_ID :  INOUT  STD_LOGIC_VECTOR(7 downto 0);
		A_VG_IO_Res :  INOUT  STD_LOGIC_VECTOR(1 downto 0);
		A_VG_K0_MOD :  IN  STD_LOGIC_VECTOR(1 downto 0);
		A_VG_K1_MOD :  IN  STD_LOGIC_VECTOR(1 downto 0);
		A_VG_K2_MOD :  IN  STD_LOGIC_VECTOR(1 downto 0);
		A_VG_K3_MOD :  IN  STD_LOGIC_VECTOR(1 downto 0);
		A_VG_Log_ID :  IN  STD_LOGIC_VECTOR(5 downto 0);
		Loader_DB :  INOUT  STD_LOGIC_VECTOR(3 downto 0);
		Loader_Misc :  INOUT  STD_LOGIC_VECTOR(3 downto 0);
		nSEL_LED_GRP :  IN  STD_LOGIC_VECTOR(1 downto 0);
		SEL_B :  IN  STD_LOGIC_VECTOR(3 downto 0);
		TP :  INOUT  STD_LOGIC_VECTOR(12 downto 1);
		nPowerup_Led :  OUT  STD_LOGIC;
		nExt_Data_En :  OUT  STD_LOGIC;
		nDT_Led :  OUT  STD_LOGIC;
		nSkal_OK_Led :  OUT  STD_LOGIC;
		nID_OK_Led :  OUT  STD_LOGIC;
		A_nK0_ID_EN :  OUT  STD_LOGIC;
		A_nK1_ID_EN :  OUT  STD_LOGIC;
		A_nK2_ID_EN :  OUT  STD_LOGIC;
		A_nK3_ID_EN :  OUT  STD_LOGIC;
		A_nINTERLOCKA :  OUT  STD_LOGIC;
		A_nINTERLOCKB :  OUT  STD_LOGIC;
		A_nSRQA :  OUT  STD_LOGIC;
		A_nSRQB :  OUT  STD_LOGIC;
		A_nDRQB :  OUT  STD_LOGIC;
		A_nDRQA :  OUT  STD_LOGIC;
		A_nDTACKA :  OUT  STD_LOGIC;
		A_nDTACKB :  OUT  STD_LOGIC;
		nMaster_Clk_ENA :  OUT  STD_LOGIC;
		nSlave_Clk_ENA :  OUT  STD_LOGIC;
		nIndepend_Clk_Ena :  OUT  STD_LOGIC;
		nK0_SWITCH_ENA :  OUT  STD_LOGIC;
		nK1_SWITCH_ENA :  OUT  STD_LOGIC;
		nK2_SWITCH_ENA :  OUT  STD_LOGIC;
		nK3_SWITCH_ENA :  OUT  STD_LOGIC;
		Loader_WRnRD :  OUT  STD_LOGIC;
		INIT_DONE :  OUT  STD_LOGIC;
		LOAD_OK :  OUT  STD_LOGIC;
		LOAD_ERROR :  OUT  STD_LOGIC;
		RELOAD :  OUT  STD_LOGIC;
		A_nGR0_ID_SEL :  OUT  STD_LOGIC;
		A_nGR1_ID_SEL :  OUT  STD_LOGIC;
		A_LA_CLK :  OUT  STD_LOGIC;
		A_nMANUAL_RES :  OUT  STD_LOGIC;
		A_Master_Clk_Out :  OUT  STD_LOGIC;
		nLED :  OUT  STD_LOGIC_VECTOR(15 downto 0);
		nLED_Skal :  OUT  STD_LOGIC_VECTOR(7 downto 0)
	);
END chopper_vhdl;

ARCHITECTURE bdf_type OF chopper_vhdl IS 

component bus_io
GENERIC (I0_1_is_Input:INTEGER;
			I0_2_is_Input:INTEGER;
			I0_3_is_Input:INTEGER;
			I0_4_is_Input:INTEGER;
			I0_5_is_Input:INTEGER;
			ST_160_Pol:INTEGER);
	PORT(To_IO_5 : IN STD_LOGIC;
		 To_IO_4 : IN STD_LOGIC;
		 To_IO_3 : IN STD_LOGIC;
		 To_IO_2 : IN STD_LOGIC;
		 To_IO_1 : IN STD_LOGIC;
		 A_Bus_IO : INOUT STD_LOGIC_VECTOR(5 downto 1)
	);
end component;

component debounce_skal
GENERIC (Clk_in_Hz:INTEGER;
			DB_in_ns:INTEGER;
			Test:INTEGER);
	PORT(A_VG_K1_INP : IN STD_LOGIC;
		 A_VG_K0_INP : IN STD_LOGIC;
		 A_GR0_APK_ID : IN STD_LOGIC;
		 A_GR0_16BIT : IN STD_LOGIC;
		 A_VG_K3_INP : IN STD_LOGIC;
		 A_VG_K2_INP : IN STD_LOGIC;
		 A_GR1_APK_ID : IN STD_LOGIC;
		 A_GR1_16BIT : IN STD_LOGIC;
		 Latch_Inputs : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 A_VG_K0_MOD : IN STD_LOGIC_VECTOR(1 downto 0);
		 A_VG_K1_MOD : IN STD_LOGIC_VECTOR(1 downto 0);
		 A_VG_K2_MOD : IN STD_LOGIC_VECTOR(1 downto 0);
		 A_VG_K3_MOD : IN STD_LOGIC_VECTOR(1 downto 0);
		 Logic : IN STD_LOGIC_VECTOR(5 downto 0);
		 DB_K1_INP : OUT STD_LOGIC;
		 DB_K0_INP : OUT STD_LOGIC;
		 DB_GR0_APK_ID : OUT STD_LOGIC;
		 DB_K3_INP : OUT STD_LOGIC;
		 DB_K2_INP : OUT STD_LOGIC;
		 DB_GR1_APK_ID : OUT STD_LOGIC;
		 DB_Valid : OUT STD_LOGIC;
		 DB_160_Skal : OUT STD_LOGIC_VECTOR(7 downto 0);
		 DB_Logic : OUT STD_LOGIC_VECTOR(5 downto 0);
		 DB_Mod_Skal : OUT STD_LOGIC_VECTOR(7 downto 0);
		 K1_K0_Skal : OUT STD_LOGIC_VECTOR(7 downto 0);
		 K3_K2_Skal : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

component apk_stecker_id_cntrl
GENERIC (Clk_in_Hz:INTEGER;
			K0_APK_ST_ID:INTEGER;
			K1_APK_ST_ID:INTEGER;
			K2_APK_ST_ID:INTEGER;
			K3_APK_ST_ID:INTEGER;
			ST_160_Pol:INTEGER;
			Use_LPM:INTEGER;
			Wait_in_ns:INTEGER);
	PORT(Start_ID_Cntrl : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 DB_GR1_APK_ID : IN STD_LOGIC;
		 DB_GR0_APK_ID : IN STD_LOGIC;
		 A_K0_D : IN STD_LOGIC_VECTOR(15 downto 0);
		 A_K1_D : IN STD_LOGIC_VECTOR(15 downto 0);
		 A_K2_D : IN STD_LOGIC_VECTOR(15 downto 0);
		 A_K3_D : IN STD_LOGIC_VECTOR(15 downto 0);
		 K1_K0_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 K3_K2_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 ID_Cntrl_Done : OUT STD_LOGIC;
		 APK_ST_ID_OK : OUT STD_LOGIC;
		 La_Ena_Skal_In : OUT STD_LOGIC;
		 La_Ena_Port_In : OUT STD_LOGIC;
		 A_nK3_ID_En : OUT STD_LOGIC;
		 A_nK2_ID_En : OUT STD_LOGIC;
		 A_nGR1_ID_Sel : OUT STD_LOGIC;
		 A_nK1_ID_En : OUT STD_LOGIC;
		 A_nK0_ID_En : OUT STD_LOGIC;
		 A_nGR0_ID_Sel : OUT STD_LOGIC;
		 K0_ID : OUT STD_LOGIC_VECTOR(15 downto 0);
		 K1_ID : OUT STD_LOGIC_VECTOR(15 downto 0);
		 K2_ID : OUT STD_LOGIC_VECTOR(15 downto 0);
		 K3_ID : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component chopper_pll
	PORT(inclk0 : IN STD_LOGIC;
		 c0 : OUT STD_LOGIC;
		 c1 : OUT STD_LOGIC;
		 locked : OUT STD_LOGIC
	);
end component;

component skal_test
GENERIC (Gr0_16Bit:INTEGER;
			Gr0_APK_ID:INTEGER;
			Gr1_16Bit:INTEGER;
			Gr1_APK_ID:INTEGER;
			K0_Input:INTEGER;
			K0C_Def_Level:INTEGER;
			K0D_Def_Level:INTEGER;
			K1_Input:INTEGER;
			K1C_Def_Level:INTEGER;
			K1D_Def_Level:INTEGER;
			K2_Input:INTEGER;
			K2C_Def_Level:INTEGER;
			K2D_Def_Level:INTEGER;
			K3_Input:INTEGER;
			K3C_Def_Level:INTEGER;
			K3D_Def_Level:INTEGER;
			Logic_Nr_End:INTEGER;
			Logic_Nr_Start:INTEGER;
			No_Level_Test:INTEGER;
			No_Logic_Test:INTEGER;
			No_Port_Dir_Test:INTEGER;
			ST_160_Pol:INTEGER);
	PORT(A_K3D_SPG : IN STD_LOGIC;
		 A_K3C_SPG : IN STD_LOGIC;
		 A_K2D_SPG : IN STD_LOGIC;
		 A_K2C_SPG : IN STD_LOGIC;
		 A_K1D_SPG : IN STD_LOGIC;
		 A_K1C_SPG : IN STD_LOGIC;
		 A_K0D_SPG : IN STD_LOGIC;
		 A_K0C_SPG : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 Logik : IN STD_LOGIC_VECTOR(5 downto 0);
		 St_160_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 VG_Mod_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 All_Okay : OUT STD_LOGIC;
		 Mod_Skal_Ok : OUT STD_LOGIC;
		 Level_Ok : OUT STD_LOGIC;
		 Logic_Nr_Ok : OUT STD_LOGIC;
		 nK0_Switch_Ena : OUT STD_LOGIC;
		 nK1_Switch_Ena : OUT STD_LOGIC;
		 nK2_Switch_Ena : OUT STD_LOGIC;
		 nK3_Switch_Ena : OUT STD_LOGIC;
		 nSkal_Okay_Led : OUT STD_LOGIC
	);
end component;

component k12_k23_logik_leds
GENERIC (Test:INTEGER);
	PORT(Logik_Aktiv : IN STD_LOGIC;
		 Test_Vers_Aktiv : IN STD_LOGIC;
		 Ena : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 Live_LED : IN STD_LOGIC_VECTOR(7 downto 0);
		 Logik : IN STD_LOGIC_VECTOR(5 downto 0);
		 Sel : IN STD_LOGIC_VECTOR(1 downto 0);
		 St_160_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 VG_Mod_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 nLED_Skal : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

component kicker_leds
GENERIC (Use_LPM:INTEGER);
	PORT(clk : IN STD_LOGIC;
		 Led_Ena : IN STD_LOGIC;
		 Led_Sig : IN STD_LOGIC_VECTOR(15 downto 0);
		 nLed_Out : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component chopper_macro1_vhdl
GENERIC (Clk_in_HZ:INTEGER;
			Test:INTEGER);
	PORT(RD_Activ : IN STD_LOGIC;
		 WR_Activ : IN STD_LOGIC;
		 Skal_OK : IN STD_LOGIC;
		 Clk : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 Off_Anforderung_In : IN STD_LOGIC;
		 Off_UU_In : IN STD_LOGIC;
		 Beam_Control_In : IN STD_LOGIC_VECTOR(15 downto 0);
		 Data_WR : IN STD_LOGIC_VECTOR(15 downto 0);
		 Strahlalarm_In : IN STD_LOGIC_VECTOR(15 downto 0);
		 Sub_Adr_Sync : IN STD_LOGIC_VECTOR(7 downto 1);
		 Chop_m1_Rd_Activ : OUT STD_LOGIC;
		 Chop1_Macro1_Activ : OUT STD_LOGIC;
		 Chop_DT_to_MB : OUT STD_LOGIC;
		 Chop_m1_Test_Vers_Aktiv : OUT STD_LOGIC;
		 Strahlweg_Reg_WR : OUT STD_LOGIC;
		 Strahlweg_Reg_RD : OUT STD_LOGIC;
		 Strahlweg_Maske_RD : OUT STD_LOGIC;
		 Interlock_to_SE_RD : OUT STD_LOGIC;
		 Global_Status_RD : OUT STD_LOGIC;
		 Beam_Control_Out : OUT STD_LOGIC_VECTOR(15 downto 0);
		 Chop_m1_LEDs : OUT STD_LOGIC_VECTOR(15 downto 0);
		 Chop_m1_RD_data : OUT STD_LOGIC_VECTOR(15 downto 0);
		 Trafo_Timing_Out : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component clk_cntrl
GENERIC (Test:INTEGER);
	PORT(Skal_Okay : IN STD_LOGIC;
		 Master_Clk : IN STD_LOGIC;
		 Extern_Clk : IN STD_LOGIC;
		 Reset : IN STD_LOGIC;
		 Logic : IN STD_LOGIC_VECTOR(5 downto 0);
		 nMaster_Clk_Ena : OUT STD_LOGIC;
		 nSlave_Clk_Ena : OUT STD_LOGIC;
		 nIndenpend_Clk_Ena : OUT STD_LOGIC;
		 Extern_Clk_OK : OUT STD_LOGIC
	);
end component;

component epld_vers
GENERIC (Test:INTEGER);
	PORT(		 Test_Activ : OUT STD_LOGIC;
		 Vers_Rev : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

component kanal
	PORT(nPort_Wr : IN STD_LOGIC;
		 ID_OK : IN STD_LOGIC;
		 ID_Cntrl_Done : IN STD_LOGIC;
		 Wr_Strobe : IN STD_LOGIC;
		 Port_In_La : IN STD_LOGIC;
		 clk : IN STD_LOGIC;
		 nP_CTRL : INOUT STD_LOGIC_VECTOR(2 downto 1);
		 Port_IO : INOUT STD_LOGIC_VECTOR(15 downto 0);
		 TO_Port : IN STD_LOGIC_VECTOR(15 downto 0);
		 Rd_Strobe : OUT STD_LOGIC;
		 From_Port : OUT STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

component master
	PORT(inclk0 : IN STD_LOGIC;
		 pfdena : IN STD_LOGIC;
		 c0 : OUT STD_LOGIC;
		 c1 : OUT STD_LOGIC;
		 locked : OUT STD_LOGIC
	);
end component;

component modulbus_loader
GENERIC (CLK_in_Hz:INTEGER;
			I2C_Freq_in_Hz:INTEGER;
			Loader_Clk_in_Hz:INTEGER;
			LPM_Ein_ist_1:INTEGER;
			Mod_Id:INTEGER;
			nDS_Deb_in_ns:INTEGER;
			Res_Deb_in_ns:INTEGER;
			St_160_pol:INTEGER;
			Test_Ein_ist_1:INTEGER);
	PORT(Stat_IN7 : IN STD_LOGIC;
		 Stat_IN6 : IN STD_LOGIC;
		 Stat_IN5 : IN STD_LOGIC;
		 Stat_IN4 : IN STD_LOGIC;
		 Stat_IN3 : IN STD_LOGIC;
		 Stat_IN2 : IN STD_LOGIC;
		 Macro_Activ : IN STD_LOGIC;
		 Macro_Skal_Ok : IN STD_LOGIC;
		 RDnWR : IN STD_LOGIC;
		 nDS : IN STD_LOGIC;
		 CLK : IN STD_LOGIC;
		 nMB_Reset : IN STD_LOGIC;
		 Extern_Dtack : IN STD_LOGIC;
		 CTRL_Load : IN STD_LOGIC;
		 CTRL_Res : IN STD_LOGIC;
		 Loader_Clk : IN STD_LOGIC;
		 Loader_DB : INOUT STD_LOGIC_VECTOR(3 downto 0);
		 Mod_Adr : IN STD_LOGIC_VECTOR(4 downto 0);
		 Mod_Data : INOUT STD_LOGIC_VECTOR(7 downto 0);
		 St_160_Auxi : IN STD_LOGIC_VECTOR(5 downto 0);
		 St_160_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 Sub_Adr : IN STD_LOGIC_VECTOR(7 downto 0);
		 V_Data_Rd : IN STD_LOGIC_VECTOR(15 downto 0);
		 Vers_Rev : IN STD_LOGIC_VECTOR(7 downto 0);
		 VG_Mod_Adr : IN STD_LOGIC_VECTOR(4 downto 0);
		 VG_Mod_Id : INOUT STD_LOGIC_VECTOR(7 downto 0);
		 VG_Mod_Skal : IN STD_LOGIC_VECTOR(7 downto 0);
		 nExt_Data_En : OUT STD_LOGIC;
		 nDt_Mod_Bus : OUT STD_LOGIC;
		 Extern_Wr_Activ : OUT STD_LOGIC;
		 Extern_Wr_Fin : OUT STD_LOGIC;
		 Extern_Rd_Activ : OUT STD_LOGIC;
		 Extern_Rd_Fin : OUT STD_LOGIC;
		 Powerup_Res : OUT STD_LOGIC;
		 nInterlock : OUT STD_LOGIC;
		 ID_OK : OUT STD_LOGIC;
		 nID_OK_Led : OUT STD_LOGIC;
		 Led_Ena : OUT STD_LOGIC;
		 nPowerup_Led : OUT STD_LOGIC;
		 nDt_Led : OUT STD_LOGIC;
		 nSEL_I2C : OUT STD_LOGIC;
		 Loader_WRnRD : OUT STD_LOGIC;
		 RELOAD : OUT STD_LOGIC;
		 LOAD_OK : OUT STD_LOGIC;
		 LOAD_ERROR : OUT STD_LOGIC;
		 INIT_DONE : OUT STD_LOGIC;
		 Data_Wr_La : OUT STD_LOGIC_VECTOR(15 downto 0);
		 Sub_Adr_La : OUT STD_LOGIC_VECTOR(7 downto 1)
	);
end component;

signal	A_Master_Clk_Out_ALTERA_SYNTHESIZED :  STD_LOGIC;
signal	APK_Skal_Ok :  STD_LOGIC;
signal	APK_ST_ID_OK :  STD_LOGIC;
signal	Beam_Control_In :  STD_LOGIC_VECTOR(15 downto 0);
signal	Beam_Control_Out :  STD_LOGIC_VECTOR(15 downto 0);
signal	Chop_m1_LEDs :  STD_LOGIC_VECTOR(15 downto 0);
signal	CLK :  STD_LOGIC;
signal	Data_Wr_La :  STD_LOGIC_VECTOR(15 downto 0);
signal	DB_160_Skal :  STD_LOGIC_VECTOR(7 downto 0);
signal	DB_GR0_APK_ID :  STD_LOGIC;
signal	DB_GR1_APK_ID :  STD_LOGIC;
signal	DB_K0_INP :  STD_LOGIC;
signal	DB_K1_INP :  STD_LOGIC;
signal	DB_K2_INP :  STD_LOGIC;
signal	DB_K3_INP :  STD_LOGIC;
signal	DB_Mod_Skal :  STD_LOGIC_VECTOR(7 downto 0);
signal	Extern_Clk :  STD_LOGIC;
signal	Extern_Dtack :  STD_LOGIC;
signal	Extern_Rd_Activ :  STD_LOGIC;
signal	Extern_Rd_Fin :  STD_LOGIC;
signal	Extern_Wr_Activ :  STD_LOGIC;
signal	Extern_Wr_Fin :  STD_LOGIC;
signal	F_150MHz :  STD_LOGIC;
signal	ID_Cntrl_Done :  STD_LOGIC;
signal	ID_OK :  STD_LOGIC;
signal	K1_K0_Skal :  STD_LOGIC_VECTOR(7 downto 0);
signal	K3_K2_Skal :  STD_LOGIC_VECTOR(7 downto 0);
signal	La_Ena_Port_In :  STD_LOGIC;
signal	La_Ena_Skal_In :  STD_LOGIC;
signal	Led_Ena :  STD_LOGIC;
signal	Level_Ok :  STD_LOGIC;
signal	Live_LED :  STD_LOGIC_VECTOR(7 downto 0);
signal	Logic :  STD_LOGIC_VECTOR(5 downto 0);
signal	Logic_Nr_Ok :  STD_LOGIC;
signal	Mod_Skal_Ok :  STD_LOGIC;
signal	nDT_MOD_BUS :  STD_LOGIC;
signal	nInterlock :  STD_LOGIC;
signal	nSkal_Okay_Led :  STD_LOGIC;
signal	Powerup_Res :  STD_LOGIC;
signal	Skal_Okay :  STD_LOGIC;
signal	Start_ID_Cntrl :  STD_LOGIC;
signal	Strahlalarm_In :  STD_LOGIC_VECTOR(15 downto 0);
signal	Sub_Adr_La :  STD_LOGIC_VECTOR(7 downto 1);
signal	T :  STD_LOGIC_VECTOR(15 downto 0);
signal	Test_Activ :  STD_LOGIC;
signal	Trafo_Timing_Out :  STD_LOGIC_VECTOR(15 downto 0);
signal	V_Data_Rd :  STD_LOGIC_VECTOR(15 downto 0);
signal	Vers_Rev :  STD_LOGIC_VECTOR(7 downto 0);
signal	SYNTHESIZED_WIRE_36 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_37 :  STD_LOGIC_VECTOR(0 to 11);
signal	SYNTHESIZED_WIRE_38 :  STD_LOGIC_VECTOR(0 to 1);
signal	SYNTHESIZED_WIRE_8 :  STD_LOGIC_VECTOR(0 to 14);
signal	SYNTHESIZED_WIRE_9 :  STD_LOGIC_VECTOR(1 downto 0);
signal	SYNTHESIZED_WIRE_39 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_40 :  STD_LOGIC_VECTOR(0 to 11);
signal	SYNTHESIZED_WIRE_21 :  STD_LOGIC_VECTOR(0 to 15);
signal	SYNTHESIZED_WIRE_22 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_23 :  STD_LOGIC_VECTOR(0 to 15);
signal	SYNTHESIZED_WIRE_24 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_25 :  STD_LOGIC_VECTOR(0 to 15);
signal	SYNTHESIZED_WIRE_26 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_27 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_28 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_41 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_32 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_33 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_34 :  STD_LOGIC;
signal	SYNTHESIZED_WIRE_35 :  STD_LOGIC;


BEGIN 
A_nMANUAL_RES <= '1';
A_nINTERLOCKA <= SYNTHESIZED_WIRE_32;
A_nINTERLOCKB <= SYNTHESIZED_WIRE_32;
A_nSRQA <= SYNTHESIZED_WIRE_33;
A_nSRQB <= SYNTHESIZED_WIRE_33;
A_nDRQB <= SYNTHESIZED_WIRE_34;
A_nDRQA <= SYNTHESIZED_WIRE_34;
A_nDTACKA <= SYNTHESIZED_WIRE_35;
A_nDTACKB <= SYNTHESIZED_WIRE_35;
SYNTHESIZED_WIRE_36 <= '0';
SYNTHESIZED_WIRE_37 <= "000000000000";
SYNTHESIZED_WIRE_38 <= "00";
SYNTHESIZED_WIRE_8 <= "000000000000000";
SYNTHESIZED_WIRE_39 <= '0';
SYNTHESIZED_WIRE_40 <= "000000000000";
SYNTHESIZED_WIRE_21 <= "1111111111111111";
SYNTHESIZED_WIRE_22 <= '1';
SYNTHESIZED_WIRE_23 <= "0000000000000000";
SYNTHESIZED_WIRE_24 <= '1';
SYNTHESIZED_WIRE_25 <= "0000000000000000";
SYNTHESIZED_WIRE_26 <= '1';
SYNTHESIZED_WIRE_27 <= '1';
SYNTHESIZED_WIRE_28 <= '1';
SYNTHESIZED_WIRE_41 <= '0';



b2v_Bus_IO : bus_io
GENERIC MAP(I0_1_is_Input => 1,I0_2_is_Input => 1,I0_3_is_Input => 1,I0_4_is_Input => 1,I0_5_is_Input => 1,ST_160_Pol => 1)
PORT MAP(To_IO_5 => SYNTHESIZED_WIRE_36,
		 To_IO_4 => SYNTHESIZED_WIRE_36,
		 To_IO_3 => SYNTHESIZED_WIRE_36,
		 To_IO_2 => SYNTHESIZED_WIRE_36,
		 To_IO_1 => SYNTHESIZED_WIRE_36,
		 A_Bus_IO => A_Bus_IO);

b2v_DB_Skal : debounce_skal
GENERIC MAP(Clk_in_Hz => 20000000,DB_in_ns => 200,Test => 0)
PORT MAP(A_VG_K1_INP => A_VG_K1_INP,
		 A_VG_K0_INP => A_VG_K0_INP,
		 A_GR0_APK_ID => A_GR0_APK_ID,
		 A_GR0_16BIT => A_GR0_16BIT,
		 A_VG_K3_INP => A_VG_K3_INP,
		 A_VG_K2_INP => A_VG_K2_INP,
		 A_GR1_APK_ID => A_GR1_APK_ID,
		 A_GR1_16BIT => A_GR1_16BIT,
		 Latch_Inputs => La_Ena_Skal_In,
		 Reset => Powerup_Res,
		 clk => A_Master_Clk_Out_ALTERA_SYNTHESIZED,
		 A_VG_K0_MOD => A_VG_K0_MOD,
		 A_VG_K1_MOD => A_VG_K1_MOD,
		 A_VG_K2_MOD => A_VG_K2_MOD,
		 A_VG_K3_MOD => A_VG_K3_MOD,
		 Logic => A_VG_Log_ID,
		 DB_K1_INP => DB_K1_INP,
		 DB_K0_INP => DB_K0_INP,
		 DB_GR0_APK_ID => DB_GR0_APK_ID,
		 DB_K3_INP => DB_K3_INP,
		 DB_K2_INP => DB_K2_INP,
		 DB_GR1_APK_ID => DB_GR1_APK_ID,
		 DB_Valid => Start_ID_Cntrl,
		 DB_160_Skal => DB_160_Skal,
		 DB_Logic => Logic,
		 DB_Mod_Skal => DB_Mod_Skal,
		 K1_K0_Skal => K1_K0_Skal,
		 K3_K2_Skal => K3_K2_Skal);

b2v_ID_Cntrl : apk_stecker_id_cntrl
GENERIC MAP(Clk_in_Hz => 150000000,K0_APK_ST_ID => 53312,K1_APK_ST_ID => 53319,K2_APK_ST_ID => 60720,K3_APK_ST_ID => 60720,ST_160_Pol => 1,Use_LPM => 1,Wait_in_ns => 200)
PORT MAP(Start_ID_Cntrl => Start_ID_Cntrl,
		 Reset => Powerup_Res,
		 clk => F_150MHz,
		 DB_GR1_APK_ID => DB_GR1_APK_ID,
		 DB_GR0_APK_ID => DB_GR0_APK_ID,
		 A_K0_D => A_K0_D,
		 A_K1_D => A_K1_D,
		 A_K2_D => A_K2_D,
		 A_K3_D => A_K3_D,
		 K1_K0_Skal => K1_K0_Skal,
		 K3_K2_Skal => K3_K2_Skal,
		 ID_Cntrl_Done => ID_Cntrl_Done,
		 APK_ST_ID_OK => APK_ST_ID_OK,
		 La_Ena_Skal_In => La_Ena_Skal_In,
		 La_Ena_Port_In => La_Ena_Port_In,
		 A_nK3_ID_En => A_nK3_ID_EN,
		 A_nK2_ID_En => A_nK2_ID_EN,
		 A_nGR1_ID_Sel => A_nGR1_ID_SEL,
		 A_nK1_ID_En => A_nK1_ID_EN,
		 A_nK0_ID_En => A_nK0_ID_EN,
		 A_nGR0_ID_Sel => A_nGR0_ID_SEL);

APK_Skal_Ok <= Skal_Okay AND APK_ST_ID_OK;

b2v_inst1 : chopper_pll
PORT MAP(inclk0 => Kicker_Clk,
		 c0 => CLK,
		 c1 => Extern_Clk);

process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(0) = '1') then
	TP(12) <= SYNTHESIZED_WIRE_37(0);
else
	TP(12) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(1) = '1') then
	TP(11) <= SYNTHESIZED_WIRE_37(1);
else
	TP(11) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(2) = '1') then
	TP(10) <= SYNTHESIZED_WIRE_37(2);
else
	TP(10) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(3) = '1') then
	TP(9) <= SYNTHESIZED_WIRE_37(3);
else
	TP(9) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(4) = '1') then
	TP(8) <= SYNTHESIZED_WIRE_37(4);
else
	TP(8) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(5) = '1') then
	TP(7) <= SYNTHESIZED_WIRE_37(5);
else
	TP(7) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(6) = '1') then
	TP(6) <= SYNTHESIZED_WIRE_37(6);
else
	TP(6) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(7) = '1') then
	TP(5) <= SYNTHESIZED_WIRE_37(7);
else
	TP(5) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(8) = '1') then
	TP(4) <= SYNTHESIZED_WIRE_37(8);
else
	TP(4) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(9) = '1') then
	TP(3) <= SYNTHESIZED_WIRE_37(9);
else
	TP(3) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(10) = '1') then
	TP(2) <= SYNTHESIZED_WIRE_37(10);
else
	TP(2) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_37,SYNTHESIZED_WIRE_37)
begin
if (SYNTHESIZED_WIRE_37(11) = '1') then
	TP(1) <= SYNTHESIZED_WIRE_37(11);
else
	TP(1) <= 'Z';
end if;
end process;


b2v_inst11 : skal_test
GENERIC MAP(Gr0_16Bit => 1,Gr0_APK_ID => 1,Gr1_16Bit => 1,Gr1_APK_ID => 1,K0_Input => 1,K0C_Def_Level => 1,K0D_Def_Level => 0,K1_Input => 1,K1C_Def_Level => 1,K1D_Def_Level => 0,K2_Input => 0,K2C_Def_Level => 1,K2D_Def_Level => 0,K3_Input => 0,K3C_Def_Level => 1,K3D_Def_Level => 0,Logic_Nr_End => 1,Logic_Nr_Start => 1,No_Level_Test => 0,No_Logic_Test => 0,No_Port_Dir_Test => 0,ST_160_Pol => 1)
PORT MAP(A_K3D_SPG => A_K3D_SPG,
		 A_K3C_SPG => A_K3C_SPG,
		 A_K2D_SPG => A_K2D_SPG,
		 A_K2C_SPG => A_K2C_SPG,
		 A_K1D_SPG => A_K1D_SPG,
		 A_K1C_SPG => A_K1C_SPG,
		 A_K0D_SPG => A_K0D_SPG,
		 A_K0C_SPG => A_K0C_SPG,
		 clk => CLK,
		 Logik => Logic,
		 St_160_Skal => DB_160_Skal,
		 VG_Mod_Skal => DB_Mod_Skal,
		 All_Okay => Skal_Okay,
		 Mod_Skal_Ok => Mod_Skal_Ok,
		 Level_Ok => Level_Ok,
		 Logic_Nr_Ok => Logic_Nr_Ok,
		 nK0_Switch_Ena => nK0_SWITCH_ENA,
		 nK1_Switch_Ena => nK1_SWITCH_ENA,
		 nK2_Switch_Ena => nK2_SWITCH_ENA,
		 nK3_Switch_Ena => nK3_SWITCH_ENA);

b2v_inst15 : k12_k23_logik_leds
GENERIC MAP(Test => 0)
PORT MAP(Logik_Aktiv => Skal_Okay,
		 Test_Vers_Aktiv => Test_Activ,
		 Ena => Led_Ena,
		 clk => CLK,
		 Live_LED => Live_LED,
		 Logik => Logic,
		 Sel => nSEL_LED_GRP,
		 St_160_Skal => DB_160_Skal,
		 VG_Mod_Skal => DB_Mod_Skal,
		 nLED_Skal => nLED_Skal);

Live_LED(0) <= NOT(T(0));


b2v_inst17 : kicker_leds
GENERIC MAP(Use_LPM => 1)
PORT MAP(clk => CLK,
		 Led_Ena => Led_Ena,
		 Led_Sig => Chop_m1_LEDs,
		 nLed_Out => nLED);
SYNTHESIZED_WIRE_35 <= nDT_MOD_BUS;

Live_LED(2) <= APK_ST_ID_OK;


b2v_inst21 : chopper_macro1_vhdl
GENERIC MAP(Clk_in_HZ => 200000000,Test => 0)
PORT MAP(RD_Activ => Extern_Rd_Activ,
		 WR_Activ => Extern_Wr_Activ,
		 Skal_OK => APK_Skal_Ok,
		 Clk => CLK,
		 Reset => Powerup_Res,
		 Off_Anforderung_In => A_Bus_IO(1),
		 Off_UU_In => A_Bus_IO(2),
		 Beam_Control_In => Beam_Control_In,
		 Data_WR => Data_Wr_La,
		 Strahlalarm_In => Strahlalarm_In,
		 Sub_Adr_Sync => Sub_Adr_La,
		 Chop_DT_to_MB => Extern_Dtack,
		 Beam_Control_Out => Beam_Control_Out,
		 Chop_m1_LEDs => Chop_m1_LEDs,
		 Chop_m1_RD_data => V_Data_Rd,
		 Trafo_Timing_Out => Trafo_Timing_Out);

b2v_inst33 : clk_cntrl
GENERIC MAP(Test => 0)
PORT MAP(Skal_Okay => Skal_Okay,
		 Master_Clk => A_Master_Clk_Out_ALTERA_SYNTHESIZED,
		 Extern_Clk => Extern_Clk,
		 Reset => Powerup_Res,
		 Logic => Logic,
		 nMaster_Clk_Ena => nMaster_Clk_ENA,
		 nSlave_Clk_Ena => nSlave_Clk_ENA,
		 nIndenpend_Clk_Ena => nIndepend_Clk_Ena,
		 Extern_Clk_OK => T(0));

SYNTHESIZED_WIRE_9 <= NOT(SYNTHESIZED_WIRE_38);

T(15 downto 1) <= SYNTHESIZED_WIRE_8;


process(SYNTHESIZED_WIRE_9,SYNTHESIZED_WIRE_38)
begin
if (SYNTHESIZED_WIRE_38(0) = '1') then
	A_VG_IO_Res(1) <= SYNTHESIZED_WIRE_9(1);
else
	A_VG_IO_Res(1) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_9,SYNTHESIZED_WIRE_38)
begin
if (SYNTHESIZED_WIRE_38(1) = '1') then
	A_VG_IO_Res(0) <= SYNTHESIZED_WIRE_9(0);
else
	A_VG_IO_Res(0) <= 'Z';
end if;
end process;


b2v_inst7 : epld_vers
GENERIC MAP(Test => 0)
PORT MAP(		 Test_Activ => Test_Activ,
		 Vers_Rev => Vers_Rev);

process(SYNTHESIZED_WIRE_39,SYNTHESIZED_WIRE_39)
begin
if (SYNTHESIZED_WIRE_39 = '1') then
	SYNTHESIZED_WIRE_33 <= SYNTHESIZED_WIRE_39;
else
	SYNTHESIZED_WIRE_33 <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_39,SYNTHESIZED_WIRE_39)
begin
if (SYNTHESIZED_WIRE_39 = '1') then
	SYNTHESIZED_WIRE_34 <= SYNTHESIZED_WIRE_39;
else
	SYNTHESIZED_WIRE_34 <= 'Z';
end if;
end process;

SYNTHESIZED_WIRE_32 <= nInterlock;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(0) = '1') then
	A_AUX_A(11) <= SYNTHESIZED_WIRE_40(0);
else
	A_AUX_A(11) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(1) = '1') then
	A_AUX_A(10) <= SYNTHESIZED_WIRE_40(1);
else
	A_AUX_A(10) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(2) = '1') then
	A_AUX_A(9) <= SYNTHESIZED_WIRE_40(2);
else
	A_AUX_A(9) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(3) = '1') then
	A_AUX_A(8) <= SYNTHESIZED_WIRE_40(3);
else
	A_AUX_A(8) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(4) = '1') then
	A_AUX_A(7) <= SYNTHESIZED_WIRE_40(4);
else
	A_AUX_A(7) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(5) = '1') then
	A_AUX_A(6) <= SYNTHESIZED_WIRE_40(5);
else
	A_AUX_A(6) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(6) = '1') then
	A_AUX_A(5) <= SYNTHESIZED_WIRE_40(6);
else
	A_AUX_A(5) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(7) = '1') then
	A_AUX_A(4) <= SYNTHESIZED_WIRE_40(7);
else
	A_AUX_A(4) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(8) = '1') then
	A_AUX_A(3) <= SYNTHESIZED_WIRE_40(8);
else
	A_AUX_A(3) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(9) = '1') then
	A_AUX_A(2) <= SYNTHESIZED_WIRE_40(9);
else
	A_AUX_A(2) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(10) = '1') then
	A_AUX_A(1) <= SYNTHESIZED_WIRE_40(10);
else
	A_AUX_A(1) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(11) = '1') then
	A_AUX_A(0) <= SYNTHESIZED_WIRE_40(11);
else
	A_AUX_A(0) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(0) = '1') then
	A_AUX_B(11) <= SYNTHESIZED_WIRE_40(0);
else
	A_AUX_B(11) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(1) = '1') then
	A_AUX_B(10) <= SYNTHESIZED_WIRE_40(1);
else
	A_AUX_B(10) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(2) = '1') then
	A_AUX_B(9) <= SYNTHESIZED_WIRE_40(2);
else
	A_AUX_B(9) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(3) = '1') then
	A_AUX_B(8) <= SYNTHESIZED_WIRE_40(3);
else
	A_AUX_B(8) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(4) = '1') then
	A_AUX_B(7) <= SYNTHESIZED_WIRE_40(4);
else
	A_AUX_B(7) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(5) = '1') then
	A_AUX_B(6) <= SYNTHESIZED_WIRE_40(5);
else
	A_AUX_B(6) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(6) = '1') then
	A_AUX_B(5) <= SYNTHESIZED_WIRE_40(6);
else
	A_AUX_B(5) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(7) = '1') then
	A_AUX_B(4) <= SYNTHESIZED_WIRE_40(7);
else
	A_AUX_B(4) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(8) = '1') then
	A_AUX_B(3) <= SYNTHESIZED_WIRE_40(8);
else
	A_AUX_B(3) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(9) = '1') then
	A_AUX_B(2) <= SYNTHESIZED_WIRE_40(9);
else
	A_AUX_B(2) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(10) = '1') then
	A_AUX_B(1) <= SYNTHESIZED_WIRE_40(10);
else
	A_AUX_B(1) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(11) = '1') then
	A_AUX_B(0) <= SYNTHESIZED_WIRE_40(11);
else
	A_AUX_B(0) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(0) = '1') then
	A_AUX_C(11) <= SYNTHESIZED_WIRE_40(0);
else
	A_AUX_C(11) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(1) = '1') then
	A_AUX_C(10) <= SYNTHESIZED_WIRE_40(1);
else
	A_AUX_C(10) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(2) = '1') then
	A_AUX_C(9) <= SYNTHESIZED_WIRE_40(2);
else
	A_AUX_C(9) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(3) = '1') then
	A_AUX_C(8) <= SYNTHESIZED_WIRE_40(3);
else
	A_AUX_C(8) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(4) = '1') then
	A_AUX_C(7) <= SYNTHESIZED_WIRE_40(4);
else
	A_AUX_C(7) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(5) = '1') then
	A_AUX_C(6) <= SYNTHESIZED_WIRE_40(5);
else
	A_AUX_C(6) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(6) = '1') then
	A_AUX_C(5) <= SYNTHESIZED_WIRE_40(6);
else
	A_AUX_C(5) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(7) = '1') then
	A_AUX_C(4) <= SYNTHESIZED_WIRE_40(7);
else
	A_AUX_C(4) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(8) = '1') then
	A_AUX_C(3) <= SYNTHESIZED_WIRE_40(8);
else
	A_AUX_C(3) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(9) = '1') then
	A_AUX_C(2) <= SYNTHESIZED_WIRE_40(9);
else
	A_AUX_C(2) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(10) = '1') then
	A_AUX_C(1) <= SYNTHESIZED_WIRE_40(10);
else
	A_AUX_C(1) <= 'Z';
end if;
end process;


process(SYNTHESIZED_WIRE_40,SYNTHESIZED_WIRE_40)
begin
if (SYNTHESIZED_WIRE_40(11) = '1') then
	A_AUX_C(0) <= SYNTHESIZED_WIRE_40(11);
else
	A_AUX_C(0) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(0) = '1') then
	A_Test(15) <= T(15);
else
	A_Test(15) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(1) = '1') then
	A_Test(14) <= T(14);
else
	A_Test(14) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(2) = '1') then
	A_Test(13) <= T(13);
else
	A_Test(13) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(3) = '1') then
	A_Test(12) <= T(12);
else
	A_Test(12) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(4) = '1') then
	A_Test(11) <= T(11);
else
	A_Test(11) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(5) = '1') then
	A_Test(10) <= T(10);
else
	A_Test(10) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(6) = '1') then
	A_Test(9) <= T(9);
else
	A_Test(9) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(7) = '1') then
	A_Test(8) <= T(8);
else
	A_Test(8) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(8) = '1') then
	A_Test(7) <= T(7);
else
	A_Test(7) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(9) = '1') then
	A_Test(6) <= T(6);
else
	A_Test(6) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(10) = '1') then
	A_Test(5) <= T(5);
else
	A_Test(5) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(11) = '1') then
	A_Test(4) <= T(4);
else
	A_Test(4) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(12) = '1') then
	A_Test(3) <= T(3);
else
	A_Test(3) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(13) = '1') then
	A_Test(2) <= T(2);
else
	A_Test(2) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(14) = '1') then
	A_Test(1) <= T(1);
else
	A_Test(1) <= 'Z';
end if;
end process;


process(T,SYNTHESIZED_WIRE_21)
begin
if (SYNTHESIZED_WIRE_21(15) = '1') then
	A_Test(0) <= T(0);
else
	A_Test(0) <= 'Z';
end if;
end process;


b2v_K0 : kanal
PORT MAP(nPort_Wr => DB_K0_INP,
		 ID_OK => ID_OK,
		 ID_Cntrl_Done => ID_Cntrl_Done,
		 Wr_Strobe => SYNTHESIZED_WIRE_22,
		 Port_In_La => La_Ena_Port_In,
		 clk => CLK,
		 nP_CTRL => A_nK0_CTRL,
		 Port_IO => A_K0_D,
		 TO_Port => SYNTHESIZED_WIRE_23,
		 From_Port => Beam_Control_In);

b2v_K1 : kanal
PORT MAP(nPort_Wr => DB_K1_INP,
		 ID_OK => ID_OK,
		 ID_Cntrl_Done => ID_Cntrl_Done,
		 Wr_Strobe => SYNTHESIZED_WIRE_24,
		 Port_In_La => La_Ena_Port_In,
		 clk => CLK,
		 nP_CTRL => A_nK1_CTRL,
		 Port_IO => A_K1_D,
		 TO_Port => SYNTHESIZED_WIRE_25,
		 From_Port => Strahlalarm_In);

b2v_K2 : kanal
PORT MAP(nPort_Wr => DB_K2_INP,
		 ID_OK => ID_OK,
		 ID_Cntrl_Done => ID_Cntrl_Done,
		 Wr_Strobe => SYNTHESIZED_WIRE_26,
		 Port_In_La => La_Ena_Port_In,
		 clk => CLK,
		 nP_CTRL => A_nK2_CTRL,
		 Port_IO => A_K2_D,
		 TO_Port => Trafo_Timing_Out);

b2v_K3 : kanal
PORT MAP(nPort_Wr => DB_K3_INP,
		 ID_OK => ID_OK,
		 ID_Cntrl_Done => ID_Cntrl_Done,
		 Wr_Strobe => SYNTHESIZED_WIRE_27,
		 Port_In_La => La_Ena_Port_In,
		 clk => CLK,
		 nP_CTRL => A_nK3_CTRL,
		 Port_IO => A_K3_D,
		 TO_Port => Beam_Control_Out);

b2v_master_clk1 : master
PORT MAP(inclk0 => F_TCXO_In,
		 pfdena => SYNTHESIZED_WIRE_28,
		 c0 => A_Master_Clk_Out_ALTERA_SYNTHESIZED,
		 c1 => F_150MHz);

b2v_MB_Ld : modulbus_loader
GENERIC MAP(CLK_in_Hz => 200000000,I2C_Freq_in_Hz => 400000,Loader_Clk_in_Hz => 150000000,LPM_Ein_ist_1 => 1,Mod_Id => 39,nDS_Deb_in_ns => 20,Res_Deb_in_ns => 100,St_160_pol => 1,Test_Ein_ist_1 => 0)
PORT MAP(Stat_IN7 => Skal_Okay,
		 Stat_IN6 => Mod_Skal_Ok,
		 Stat_IN5 => Level_Ok,
		 Stat_IN4 => Logic_Nr_Ok,
		 Stat_IN3 => APK_ST_ID_OK,
		 Stat_IN2 => SYNTHESIZED_WIRE_41,
		 Macro_Activ => SYNTHESIZED_WIRE_41,
		 Macro_Skal_Ok => SYNTHESIZED_WIRE_41,
		 RDnWR => A_RDnWR,
		 nDS => A_nDS,
		 CLK => CLK,
		 nMB_Reset => A_nMB_Reset,
		 Extern_Dtack => Extern_Dtack,
		 CTRL_Load => CTRL_LOAD,
		 CTRL_Res => CTRL_RES,
		 Loader_Clk => F_150MHz,
		 Loader_DB => Loader_DB,
		 Mod_Adr => A_A,
		 Mod_Data => A_Mod_Data,
		 St_160_Auxi => Logic,
		 St_160_Skal => DB_160_Skal,
		 Sub_Adr => A_Sub_Adr,
		 V_Data_Rd => V_Data_Rd,
		 Vers_Rev => Vers_Rev,
		 VG_Mod_Adr => A_VG_A,
		 VG_Mod_Id => A_VG_ID,
		 VG_Mod_Skal => DB_Mod_Skal,
		 nExt_Data_En => nExt_Data_En,
		 nDt_Mod_Bus => nDT_MOD_BUS,
		 Extern_Wr_Activ => Extern_Wr_Activ,
		 Extern_Rd_Activ => Extern_Rd_Activ,
		 Powerup_Res => Powerup_Res,
		 nInterlock => nInterlock,
		 ID_OK => ID_OK,
		 nID_OK_Led => nID_OK_Led,
		 Led_Ena => Led_Ena,
		 nPowerup_Led => nPowerup_Led,
		 nDt_Led => nDT_Led,
		 nSEL_I2C => A_I2C_SCL,
		 Loader_WRnRD => Loader_WRnRD,
		 RELOAD => RELOAD,
		 LOAD_OK => LOAD_OK,
		 LOAD_ERROR => LOAD_ERROR,
		 INIT_DONE => INIT_DONE,
		 Data_Wr_La => Data_Wr_La,
		 Sub_Adr_La => Sub_Adr_La);
A_Master_Clk_Out <= A_Master_Clk_Out_ALTERA_SYNTHESIZED;

Live_LED(7 downto 2) <= "000000";
END; 