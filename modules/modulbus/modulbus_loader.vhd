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
-- CREATED		"Fri Aug 27 13:25:02 2010"

LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY modulbus_loader IS 
GENERIC (CLK_in_Hz : INTEGER := 200000000;
		I2C_Freq_in_Hz : INTEGER := 400000;
		Loader_Base_Adr : INTEGER := 240;
		Loader_Clk_in_Hz : INTEGER := 150000000;
		LPM_Ein_ist_1 : INTEGER := 1;
		Mod_Id : INTEGER := 1;
		nDS_Deb_in_ns : INTEGER := 20;
		Res_Deb_in_ns : INTEGER := 100;
		St_160_pol : INTEGER := 0;
		Test_Ein_ist_1 : INTEGER := 0
		);
	PORT
	(
		RDnWR :  IN  STD_LOGIC;
		nDS :  IN  STD_LOGIC;
		CLK :  IN  STD_LOGIC;
		nMB_Reset :  IN  STD_LOGIC;
		Extern_Dtack :  IN  STD_LOGIC;
		CTRL_Load :  IN  STD_LOGIC;
		CTRL_Res :  IN  STD_LOGIC;
		Loader_Clk :  IN  STD_LOGIC;
		Stat_IN2 :  IN  STD_LOGIC;
		Stat_IN3 :  IN  STD_LOGIC;
		Stat_IN4 :  IN  STD_LOGIC;
		Stat_IN5 :  IN  STD_LOGIC;
		Stat_IN6 :  IN  STD_LOGIC;
		Stat_IN7 :  IN  STD_LOGIC;
		Macro_Activ :  IN  STD_LOGIC;
		Macro_Skal_Ok :  IN  STD_LOGIC;
		Loader_DB :  INOUT  STD_LOGIC_VECTOR(3 DOWNTO 0);
		Mod_Adr :  IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		Mod_Data :  INOUT  STD_LOGIC_VECTOR(7 DOWNTO 0);
		St_160_Auxi :  IN  STD_LOGIC_VECTOR(5 DOWNTO 0);
		St_160_Skal :  IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
		Sub_Adr :  IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
		V_Data_Rd :  IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Vers_Rev :  IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
		VG_Mod_Adr :  IN  STD_LOGIC_VECTOR(4 DOWNTO 0);
		VG_Mod_Id :  INOUT  STD_LOGIC_VECTOR(7 DOWNTO 0);
		VG_Mod_Skal :  IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
		nExt_Data_En :  OUT  STD_LOGIC;
		nDt_Mod_Bus :  OUT  STD_LOGIC;
		Extern_Wr_Activ :  OUT  STD_LOGIC;
		Extern_Wr_Fin :  OUT  STD_LOGIC;
		Extern_Rd_Activ :  OUT  STD_LOGIC;
		Extern_Rd_Fin :  OUT  STD_LOGIC;
		Powerup_Res :  OUT  STD_LOGIC;
		ID_OK :  OUT  STD_LOGIC;
		nID_OK_Led :  OUT  STD_LOGIC;
		Led_Ena :  OUT  STD_LOGIC;
		nPowerup_Led :  OUT  STD_LOGIC;
		nDt_Led :  OUT  STD_LOGIC;
		Loader_WRnRD :  OUT  STD_LOGIC;
		INIT_DONE :  OUT  STD_LOGIC;
		LOAD_OK :  OUT  STD_LOGIC;
		LOAD_ERROR :  OUT  STD_LOGIC;
		RELOAD :  OUT  STD_LOGIC;
		nInterlock :  OUT  STD_LOGIC;
		nSEL_I2C :  OUT  STD_LOGIC;
		Data_Wr_La :  OUT  STD_LOGIC_VECTOR(15 DOWNTO 0);
		Sub_Adr_La :  OUT  STD_LOGIC_VECTOR(7 DOWNTO 1)
	);
END modulbus_loader;

ARCHITECTURE bdf_type OF modulbus_loader IS 

COMPONENT global_reg
GENERIC (Freq_1_in_Hz : INTEGER;
			Freq_2_in_Hz : INTEGER;
			Rd_Freq_1_Adr : INTEGER;
			Rd_Freq_2_Adr : INTEGER;
			RdWr_Echo_Reg_Adr : INTEGER;
			Test : INTEGER
			);
	PORT(Extern_Wr_Activ : IN STD_LOGIC;
		 Extern_Wr_Fin : IN STD_LOGIC;
		 Extern_Rd_Activ : IN STD_LOGIC;
		 Extern_Rd_Fin : IN STD_LOGIC;
		 Powerup_Res : IN STD_LOGIC;
		 Clk : IN STD_LOGIC;
		 Data_In : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Sub_Adr_La : IN STD_LOGIC_VECTOR(7 DOWNTO 1);
		 Rd_Glob_Reg_Activ : OUT STD_LOGIC;
		 Dtack_Glob_Reg : OUT STD_LOGIC;
		 Glob_Reg_Rd_Port : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

COMPONENT i2c_cntrl
GENERIC (Clk_in_Hz : INTEGER;
			I2C_Freq_in_Hz : INTEGER;
			RdWr_I2C_Adr : INTEGER;
			Test : INTEGER;
			Use_LPM : INTEGER
			);
	PORT(Extern_Wr_Activ : IN STD_LOGIC;
		 Extern_Wr_Fin : IN STD_LOGIC;
		 Extern_Rd_Activ : IN STD_LOGIC;
		 Extern_Rd_Fin : IN STD_LOGIC;
		 Powerup_Res : IN STD_LOGIC;
		 Clk : IN STD_LOGIC;
		 Data_In : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Sub_Adr_La : IN STD_LOGIC_VECTOR(7 DOWNTO 1);
		 VG_Mod_ID : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		 nSEL_I2C : OUT STD_LOGIC;
		 Rd_I2C_Reg_Activ : OUT STD_LOGIC;
		 Dtack_I2C_Reg : OUT STD_LOGIC;
		 I2C_Reg_Rd_Port : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 VG_ID_Out : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	);
END COMPONENT;

COMPONENT loader_mb
GENERIC (CLK_in_Hz : INTEGER;
			Loader_Base_Adr : INTEGER
			);
	PORT(Extern_Wr_Activ : IN STD_LOGIC;
		 Extern_Wr_Fin : IN STD_LOGIC;
		 Extern_Rd_Activ : IN STD_LOGIC;
		 Extern_Rd_Fin : IN STD_LOGIC;
		 Powerup_Res : IN STD_LOGIC;
		 Clk : IN STD_LOGIC;
		 CTRL_LOAD : IN STD_LOGIC;
		 CTRL_RES : IN STD_LOGIC;
		 Data_In : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 LOADER_DB : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
		 Sub_Adr_La : IN STD_LOGIC_VECTOR(7 DOWNTO 1);
		 LOADER_WRnRD : OUT STD_LOGIC;
		 RELOAD : OUT STD_LOGIC;
		 LOAD_OK : OUT STD_LOGIC;
		 LOAD_ERROR : OUT STD_LOGIC;
		 INIT_DONE : OUT STD_LOGIC;
		 Rd_Loader_Activ : OUT STD_LOGIC;
		 Dtack_Loader_Reg : OUT STD_LOGIC;
		 Loader_Rd_Port : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

COMPONENT modulbus_v5
GENERIC (CLK_in_Hz : INTEGER;
			Loader_Base_Adr : INTEGER;
			Mod_Id : INTEGER;
			nDS_Deb_in_ns : INTEGER;
			Res_Deb_in_ns : INTEGER;
			St_160_pol : INTEGER;
			Test : INTEGER;
			Use_LPM : INTEGER
			);
	PORT(Macro_Activ : IN STD_LOGIC;
		 Macro_Skal_OK : IN STD_LOGIC;
		 RDnWR : IN STD_LOGIC;
		 nDS : IN STD_LOGIC;
		 CLK : IN STD_LOGIC;
		 nMB_Reset : IN STD_LOGIC;
		 Extern_Dtack : IN STD_LOGIC;
		 Epld_Vers : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 Mod_Adr : IN STD_LOGIC_VECTOR(4 DOWNTO 0);
		 Mod_Data : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		 St_160_Auxi : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
		 St_160_Skal : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 Stat_IN : IN STD_LOGIC_VECTOR(7 DOWNTO 2);
		 Sub_Adr : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 V_Data_Rd : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 VG_Mod_Adr : IN STD_LOGIC_VECTOR(4 DOWNTO 0);
		 VG_Mod_Id : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 VG_Mod_Skal : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 nExt_Data_En : OUT STD_LOGIC;
		 nDt_Mod_Bus : OUT STD_LOGIC;
		 Extern_Wr_Activ : OUT STD_LOGIC;
		 Extern_Wr_Fin : OUT STD_LOGIC;
		 Extern_Rd_Activ : OUT STD_LOGIC;
		 Extern_Rd_Fin : OUT STD_LOGIC;
		 Powerup_Res : OUT STD_LOGIC;
		 nInterlock : OUT STD_LOGIC;
		 Timeout : OUT STD_LOGIC;
		 Id_OK : OUT STD_LOGIC;
		 nID_OK_Led : OUT STD_LOGIC;
		 Led_Ena : OUT STD_LOGIC;
		 nPower_Up_Led : OUT STD_LOGIC;
		 nSel_Led : OUT STD_LOGIC;
		 nDt_Led : OUT STD_LOGIC;
		 Data_Wr_La : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Sub_Adr_La : OUT STD_LOGIC_VECTOR(7 DOWNTO 1)
	);
END COMPONENT;

COMPONENT rd_mb_ld
	PORT(Rd_Glob_Reg_Activ : IN STD_LOGIC;
		 Rd_Loader_Activ : IN STD_LOGIC;
		 Rd_I2C_Reg_Activ : IN STD_LOGIC;
		 Glob_Reg_Rd_Port : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 I2C_Reg_Rd_Port : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Loader_Rd_Port : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 V_Data_Rd_Port : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 Data_Rd : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
END COMPONENT;

SIGNAL	Data_Wr_La_ALTERA_SYNTHESIZED :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Dtack_Glob_Reg :  STD_LOGIC;
SIGNAL	Dtack_I2C_Reg :  STD_LOGIC;
SIGNAL	Dtack_Loader_Reg :  STD_LOGIC;
SIGNAL	Extern_Dtack_S :  STD_LOGIC;
SIGNAL	Extern_Rd_Activ_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Extern_Rd_Fin_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Extern_Wr_Activ_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Extern_Wr_Fin_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Glob_Reg_Rd_Port :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	I2C_Reg_Rd_Port :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Loader_Rd_Port :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	Powerup_Res_ALTERA_SYNTHESIZED :  STD_LOGIC;
SIGNAL	Rd_Glob_Reg_Activ :  STD_LOGIC;
SIGNAL	Rd_I2C_Reg_Activ :  STD_LOGIC;
SIGNAL	Rd_Loader_Activ :  STD_LOGIC;
SIGNAL	Sub_Adr_La_ALTERA_SYNTHESIZED :  STD_LOGIC_VECTOR(7 DOWNTO 1);
SIGNAL	V_Data_Rd_Intern :  STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL	VG_Id :  STD_LOGIC_VECTOR(7 DOWNTO 0);

SIGNAL	GDFX_TEMP_SIGNAL_0 :  STD_LOGIC_VECTOR(7 DOWNTO 2);

BEGIN 

GDFX_TEMP_SIGNAL_0 <= (Stat_IN7 & Stat_IN6 & Stat_IN5 & Stat_IN4 & Stat_IN3 & Stat_IN2);


b2v_glob_reg : global_reg
GENERIC MAP(Freq_1_in_Hz => 200000000,
			Freq_2_in_Hz => 150000000,
			Rd_Freq_1_Adr => 238,
			Rd_Freq_2_Adr => 236,
			RdWr_Echo_Reg_Adr => 246,
			Test => 0
			)
PORT MAP(Extern_Wr_Activ => Extern_Wr_Activ_ALTERA_SYNTHESIZED,
		 Extern_Wr_Fin => Extern_Wr_Fin_ALTERA_SYNTHESIZED,
		 Extern_Rd_Activ => Extern_Rd_Activ_ALTERA_SYNTHESIZED,
		 Extern_Rd_Fin => Extern_Rd_Fin_ALTERA_SYNTHESIZED,
		 Powerup_Res => Powerup_Res_ALTERA_SYNTHESIZED,
		 Clk => CLK,
		 Data_In => Data_Wr_La_ALTERA_SYNTHESIZED,
		 Sub_Adr_La => Sub_Adr_La_ALTERA_SYNTHESIZED,
		 Rd_Glob_Reg_Activ => Rd_Glob_Reg_Activ,
		 Dtack_Glob_Reg => Dtack_Glob_Reg,
		 Glob_Reg_Rd_Port => Glob_Reg_Rd_Port);


b2v_I2C : i2c_cntrl
GENERIC MAP(Clk_in_Hz => 200000000,
			I2C_Freq_in_Hz => 400000,
			RdWr_I2C_Adr => 244,
			Test => 0,
			Use_LPM => 1
			)
PORT MAP(Extern_Wr_Activ => Extern_Wr_Activ_ALTERA_SYNTHESIZED,
		 Extern_Wr_Fin => Extern_Wr_Fin_ALTERA_SYNTHESIZED,
		 Extern_Rd_Activ => Extern_Rd_Activ_ALTERA_SYNTHESIZED,
		 Extern_Rd_Fin => Extern_Rd_Fin_ALTERA_SYNTHESIZED,
		 Powerup_Res => Powerup_Res_ALTERA_SYNTHESIZED,
		 Clk => CLK,
		 Data_In => Data_Wr_La_ALTERA_SYNTHESIZED,
		 Sub_Adr_La => Sub_Adr_La_ALTERA_SYNTHESIZED,
		 VG_Mod_ID => VG_Mod_Id,
		 nSEL_I2C => nSEL_I2C,
		 Rd_I2C_Reg_Activ => Rd_I2C_Reg_Activ,
		 Dtack_I2C_Reg => Dtack_I2C_Reg,
		 I2C_Reg_Rd_Port => I2C_Reg_Rd_Port,
		 VG_ID_Out => VG_Id);


Extern_Dtack_S <= Extern_Dtack OR Dtack_Loader_Reg OR Dtack_I2C_Reg OR Dtack_Glob_Reg;


b2v_loader : loader_mb
GENERIC MAP(CLK_in_Hz => 200000000,
			Loader_Base_Adr => 240
			)
PORT MAP(Extern_Wr_Activ => Extern_Wr_Activ_ALTERA_SYNTHESIZED,
		 Extern_Wr_Fin => Extern_Wr_Fin_ALTERA_SYNTHESIZED,
		 Extern_Rd_Activ => Extern_Rd_Activ_ALTERA_SYNTHESIZED,
		 Extern_Rd_Fin => Extern_Rd_Fin_ALTERA_SYNTHESIZED,
		 Powerup_Res => Powerup_Res_ALTERA_SYNTHESIZED,
		 Clk => Loader_Clk,
		 CTRL_LOAD => CTRL_Load,
		 CTRL_RES => CTRL_Res,
		 Data_In => Data_Wr_La_ALTERA_SYNTHESIZED,
		 LOADER_DB => Loader_DB,
		 Sub_Adr_La => Sub_Adr_La_ALTERA_SYNTHESIZED,
		 LOADER_WRnRD => Loader_WRnRD,
		 RELOAD => RELOAD,
		 LOAD_OK => LOAD_OK,
		 LOAD_ERROR => LOAD_ERROR,
		 INIT_DONE => INIT_DONE,
		 Rd_Loader_Activ => Rd_Loader_Activ,
		 Dtack_Loader_Reg => Dtack_Loader_Reg,
		 Loader_Rd_Port => Loader_Rd_Port);


b2v_mb_macro : modulbus_v5
GENERIC MAP(CLK_in_Hz => 200000000,
			Loader_Base_Adr => 240,
			Mod_Id => 1,
			nDS_Deb_in_ns => 20,
			Res_Deb_in_ns => 100,
			St_160_pol => 0,
			Test => 0,
			Use_LPM => 1
			)
PORT MAP(Macro_Activ => Macro_Activ,
		 Macro_Skal_OK => Macro_Skal_Ok,
		 RDnWR => RDnWR,
		 nDS => nDS,
		 CLK => CLK,
		 nMB_Reset => nMB_Reset,
		 Extern_Dtack => Extern_Dtack_S,
		 Epld_Vers => Vers_Rev,
		 Mod_Adr => Mod_Adr,
		 Mod_Data => Mod_Data,
		 St_160_Auxi => St_160_Auxi,
		 St_160_Skal => St_160_Skal,
		 Stat_IN => GDFX_TEMP_SIGNAL_0,
		 Sub_Adr => Sub_Adr,
		 V_Data_Rd => V_Data_Rd_Intern,
		 VG_Mod_Adr => VG_Mod_Adr,
		 VG_Mod_Id => VG_Id,
		 VG_Mod_Skal => VG_Mod_Skal,
		 nExt_Data_En => nExt_Data_En,
		 nDt_Mod_Bus => nDt_Mod_Bus,
		 Extern_Wr_Activ => Extern_Wr_Activ_ALTERA_SYNTHESIZED,
		 Extern_Wr_Fin => Extern_Wr_Fin_ALTERA_SYNTHESIZED,
		 Extern_Rd_Activ => Extern_Rd_Activ_ALTERA_SYNTHESIZED,
		 Extern_Rd_Fin => Extern_Rd_Fin_ALTERA_SYNTHESIZED,
		 Powerup_Res => Powerup_Res_ALTERA_SYNTHESIZED,
		 nInterlock => nInterlock,
		 Id_OK => ID_OK,
		 nID_OK_Led => nID_OK_Led,
		 Led_Ena => Led_Ena,
		 nPower_Up_Led => nPowerup_Led,
		 nDt_Led => nDt_Led,
		 Data_Wr_La => Data_Wr_La_ALTERA_SYNTHESIZED,
		 Sub_Adr_La => Sub_Adr_La_ALTERA_SYNTHESIZED);


b2v_mux : rd_mb_ld
PORT MAP(Rd_Glob_Reg_Activ => Rd_Glob_Reg_Activ,
		 Rd_Loader_Activ => Rd_Loader_Activ,
		 Rd_I2C_Reg_Activ => Rd_I2C_Reg_Activ,
		 Glob_Reg_Rd_Port => Glob_Reg_Rd_Port,
		 I2C_Reg_Rd_Port => I2C_Reg_Rd_Port,
		 Loader_Rd_Port => Loader_Rd_Port,
		 V_Data_Rd_Port => V_Data_Rd,
		 Data_Rd => V_Data_Rd_Intern);

Extern_Wr_Activ <= Extern_Wr_Activ_ALTERA_SYNTHESIZED;
Extern_Wr_Fin <= Extern_Wr_Fin_ALTERA_SYNTHESIZED;
Extern_Rd_Activ <= Extern_Rd_Activ_ALTERA_SYNTHESIZED;
Extern_Rd_Fin <= Extern_Rd_Fin_ALTERA_SYNTHESIZED;
Powerup_Res <= Powerup_Res_ALTERA_SYNTHESIZED;
Data_Wr_La <= Data_Wr_La_ALTERA_SYNTHESIZED;
Sub_Adr_La <= Sub_Adr_La_ALTERA_SYNTHESIZED;

END bdf_type;