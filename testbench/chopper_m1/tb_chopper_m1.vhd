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

-- ***************************************************************************
-- This file contains a Vhdl test bench template that is freely editable to   
-- suit user's needs .Comments are provided in each section to help the user  
-- fill out necessary details.                                                
-- ***************************************************************************
-- Generated on "08/26/2010 17:30:22"
                                                            
-- Vhdl Test Bench template for design  :  chopper_m1
-- 
-- Simulation tool : ModelSim-Altera (VHDL)
-- 

LIBRARY ieee;                                               
USE ieee.std_logic_1164.all;                                
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_signed.all;

LIBRARY STD;

ENTITY chopper_m1_vhd_tst IS
END chopper_m1_vhd_tst;
ARCHITECTURE chopper_m1_arch OF chopper_m1_vhd_tst IS
-- constants                                                 
-- signals                                                   
SIGNAL A_A : STD_LOGIC_VECTOR(4 DOWNTO 0);
SIGNAL A_AUX_A : STD_LOGIC_VECTOR(11 DOWNTO 0);
SIGNAL A_AUX_B : STD_LOGIC_VECTOR(11 DOWNTO 0);
SIGNAL A_AUX_C : STD_LOGIC_VECTOR(11 DOWNTO 0);
SIGNAL A_AUX_CA : STD_LOGIC;
SIGNAL A_AUX_CB : STD_LOGIC;
SIGNAL A_Bus_IO : STD_LOGIC_VECTOR(5 DOWNTO 1);
SIGNAL A_GR0_16BIT : STD_LOGIC;
SIGNAL A_GR0_APK_ID : STD_LOGIC;
SIGNAL A_GR1_16BIT : STD_LOGIC;
SIGNAL A_GR1_APK_ID : STD_LOGIC;
SIGNAL A_I2C_SCL : STD_LOGIC;
SIGNAL A_I2C_SDA : STD_LOGIC;
SIGNAL A_K0_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K0C_SPG : STD_LOGIC := '1';
SIGNAL A_K0D_SPG : STD_LOGIC := '0';
SIGNAL A_K1_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K1C_SPG : STD_LOGIC := '1';
SIGNAL A_K1D_SPG : STD_LOGIC := '0';
SIGNAL A_K2_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K2C_SPG : STD_LOGIC := '1';
SIGNAL A_K2D_SPG : STD_LOGIC := '0';
SIGNAL A_K3_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K3C_SPG : STD_LOGIC := '1';
SIGNAL A_K3D_SPG : STD_LOGIC := '0';
SIGNAL A_LA_CLK : STD_LOGIC;
SIGNAL A_Master_Clk_Out : STD_LOGIC;
SIGNAL A_Mod_Data : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL A_nDRQA : STD_LOGIC;
SIGNAL A_nDRQB : STD_LOGIC;
SIGNAL A_nDS : STD_LOGIC;
SIGNAL A_nDTACKA : STD_LOGIC;
SIGNAL A_nDTACKB : STD_LOGIC;
SIGNAL A_nGR0_ID_SEL : STD_LOGIC;
SIGNAL A_nGR1_ID_SEL : STD_LOGIC;
SIGNAL A_nINTERLOCKA : STD_LOGIC;
SIGNAL A_nINTERLOCKB : STD_LOGIC;
SIGNAL A_nK0_CTRL : STD_LOGIC_VECTOR(2 DOWNTO 1);
SIGNAL A_nK0_ID_EN : STD_LOGIC;
SIGNAL A_nK1_CTRL : STD_LOGIC_VECTOR(2 DOWNTO 1);
SIGNAL A_nK1_ID_EN : STD_LOGIC;
SIGNAL A_nK2_CTRL : STD_LOGIC_VECTOR(2 DOWNTO 1);
SIGNAL A_nK2_ID_EN : STD_LOGIC;
SIGNAL A_nK3_CTRL : STD_LOGIC_VECTOR(2 DOWNTO 1);
SIGNAL A_nK3_ID_EN : STD_LOGIC;
SIGNAL A_nMANUAL_RES : STD_LOGIC;
SIGNAL A_nMB_Reset : STD_LOGIC;
SIGNAL A_nSRQA : STD_LOGIC;
SIGNAL A_nSRQB : STD_LOGIC;
SIGNAL A_RDnWR : STD_LOGIC;
SIGNAL A_Sub_Adr : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL A_Test : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_VG_A : STD_LOGIC_VECTOR(4 DOWNTO 0);
SIGNAL A_VG_ID : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL A_VG_IO_Res : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL A_VG_K0_INP : STD_LOGIC;
SIGNAL A_VG_K0_MOD : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL A_VG_K1_INP : STD_LOGIC;
SIGNAL A_VG_K1_MOD : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL A_VG_K2_INP : STD_LOGIC;
SIGNAL A_VG_K2_MOD : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL A_VG_K3_INP : STD_LOGIC;
SIGNAL A_VG_K3_MOD : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL A_VG_Log_ID : STD_LOGIC_VECTOR(5 DOWNTO 0);
SIGNAL Chopper_Clk : STD_LOGIC := '0';
SIGNAL CTRL_LOAD : STD_LOGIC;
SIGNAL CTRL_RES : STD_LOGIC;
SIGNAL F_TCXO_In : STD_LOGIC := '0';
SIGNAL INIT_DONE : STD_LOGIC;
SIGNAL LOAD_ERROR : STD_LOGIC;
SIGNAL LOAD_OK : STD_LOGIC;
SIGNAL Loader_DB : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL Loader_Misc : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL Loader_WRnRD : STD_LOGIC;
SIGNAL nDT_Led : STD_LOGIC;
SIGNAL nExt_Data_En : STD_LOGIC;
SIGNAL nID_OK_Led : STD_LOGIC;
SIGNAL nIndepend_Clk_Ena : STD_LOGIC;
SIGNAL nK0_SWITCH_ENA : STD_LOGIC;
SIGNAL nK1_SWITCH_ENA : STD_LOGIC;
SIGNAL nK2_SWITCH_ENA : STD_LOGIC;
SIGNAL nK3_SWITCH_ENA : STD_LOGIC;
SIGNAL nLED : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL nLED_Skal : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL nMaster_Clk_ENA : STD_LOGIC;
SIGNAL nPowerup_Led : STD_LOGIC;
SIGNAL nSEL_LED_GRP : STD_LOGIC_VECTOR(1 DOWNTO 0);
SIGNAL nSkal_OK_Led : STD_LOGIC;
SIGNAL nSlave_Clk_ENA : STD_LOGIC;
SIGNAL RDnWR_Transceiver : STD_LOGIC;
SIGNAL RELOAD : STD_LOGIC;
SIGNAL SEL_B : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL TP : STD_LOGIC_VECTOR(12 DOWNTO 1);

SIGNAL	In_K1_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	In_K0_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen

SIGNAL	Mux_In_K0_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K1_D : STD_LOGIC_VECTOR(15 downto 0) := x"FFFF";	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K2_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K3_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen

SIGNAL	S_Last_Rd_Memory	: STD_LOGIC_VECTOR(15 DOWNTO 0);

SIGNAL	Async_On	: INTEGER := 0;


COMPONENT chopper_m1
	PORT (
	A_A : IN STD_LOGIC_VECTOR(4 DOWNTO 0);
	A_AUX_A : INOUT STD_LOGIC_VECTOR(11 DOWNTO 0);
	A_AUX_B : INOUT STD_LOGIC_VECTOR(11 DOWNTO 0);
	A_AUX_C : INOUT STD_LOGIC_VECTOR(11 DOWNTO 0);
	A_AUX_CA : INOUT STD_LOGIC;
	A_AUX_CB : INOUT STD_LOGIC;
	A_Bus_IO : INOUT STD_LOGIC_VECTOR(5 DOWNTO 1);
	A_GR0_16BIT : IN STD_LOGIC;
	A_GR0_APK_ID : IN STD_LOGIC;
	A_GR1_16BIT : IN STD_LOGIC;
	A_GR1_APK_ID : IN STD_LOGIC;
	A_I2C_SCL : INOUT STD_LOGIC;
	A_I2C_SDA : INOUT STD_LOGIC;
	A_K0_D : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	A_K0C_SPG : IN STD_LOGIC;
	A_K0D_SPG : IN STD_LOGIC;
	A_K1_D : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	A_K1C_SPG : IN STD_LOGIC;
	A_K1D_SPG : IN STD_LOGIC;
	A_K2_D : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	A_K2C_SPG : IN STD_LOGIC;
	A_K2D_SPG : IN STD_LOGIC;
	A_K3_D : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	A_K3C_SPG : IN STD_LOGIC;
	A_K3D_SPG : IN STD_LOGIC;
	A_LA_CLK : OUT STD_LOGIC;
	A_Master_Clk_Out : OUT STD_LOGIC;
	A_Mod_Data : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	A_nDRQA : OUT STD_LOGIC;
	A_nDRQB : OUT STD_LOGIC;
	A_nDS : IN STD_LOGIC;
	A_nDTACKA : OUT STD_LOGIC;
	A_nDTACKB : OUT STD_LOGIC;
	A_nGR0_ID_SEL : OUT STD_LOGIC;
	A_nGR1_ID_SEL : OUT STD_LOGIC;
	A_nINTERLOCKA : OUT STD_LOGIC;
	A_nINTERLOCKB : OUT STD_LOGIC;
	A_nK0_CTRL : INOUT STD_LOGIC_VECTOR(2 DOWNTO 1);
	A_nK0_ID_EN : OUT STD_LOGIC;
	A_nK1_CTRL : INOUT STD_LOGIC_VECTOR(2 DOWNTO 1);
	A_nK1_ID_EN : OUT STD_LOGIC;
	A_nK2_CTRL : INOUT STD_LOGIC_VECTOR(2 DOWNTO 1);
	A_nK2_ID_EN : OUT STD_LOGIC;
	A_nK3_CTRL : INOUT STD_LOGIC_VECTOR(2 DOWNTO 1);
	A_nK3_ID_EN : OUT STD_LOGIC;
	A_nMANUAL_RES : OUT STD_LOGIC;
	A_nMB_Reset : IN STD_LOGIC;
	A_nSRQA : OUT STD_LOGIC;
	A_nSRQB : OUT STD_LOGIC;
	A_RDnWR : IN STD_LOGIC;
	A_Sub_Adr : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	A_Test : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	A_VG_A : IN STD_LOGIC_VECTOR(4 DOWNTO 0);
	A_VG_ID : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	A_VG_IO_Res : INOUT STD_LOGIC_VECTOR(1 DOWNTO 0);
	A_VG_K0_INP : IN STD_LOGIC;
	A_VG_K0_MOD : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
	A_VG_K1_INP : IN STD_LOGIC;
	A_VG_K1_MOD : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
	A_VG_K2_INP : IN STD_LOGIC;
	A_VG_K2_MOD : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
	A_VG_K3_INP : IN STD_LOGIC;
	A_VG_K3_MOD : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
	A_VG_Log_ID : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
	Chopper_Clk : IN STD_LOGIC;
	CTRL_LOAD : IN STD_LOGIC;
	CTRL_RES : IN STD_LOGIC;
	F_TCXO_In : IN STD_LOGIC;
	INIT_DONE : OUT STD_LOGIC;
	LOAD_ERROR : OUT STD_LOGIC;
	LOAD_OK : OUT STD_LOGIC;
	Loader_DB : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
	Loader_Misc : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
	Loader_WRnRD : OUT STD_LOGIC;
	nDT_Led : OUT STD_LOGIC;
	nExt_Data_En : OUT STD_LOGIC;
	nID_OK_Led : OUT STD_LOGIC;
	nIndepend_Clk_Ena : OUT STD_LOGIC;
	nK0_SWITCH_ENA : OUT STD_LOGIC;
	nK1_SWITCH_ENA : OUT STD_LOGIC;
	nK2_SWITCH_ENA : OUT STD_LOGIC;
	nK3_SWITCH_ENA : OUT STD_LOGIC;
	nLED : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	nLED_Skal : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	nMaster_Clk_ENA : OUT STD_LOGIC;
	nPowerup_Led : OUT STD_LOGIC;
	nSEL_LED_GRP : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
	nSkal_OK_Led : OUT STD_LOGIC;
	nSlave_Clk_ENA : OUT STD_LOGIC;
	RDnWR_Transceiver : OUT STD_LOGIC;
	RELOAD : OUT STD_LOGIC;
	SEL_B : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
	TP : INOUT STD_LOGIC_VECTOR(12 DOWNTO 1)
	);
END COMPONENT;

CONSTANT	C_Std_DS			: INTEGER := 700;
CONSTANT	C_Std_Gap			: INTEGER := 200;

CONSTANT 	C_Rd_ID 					: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FE";
CONSTANT 	C_Rd_Skal_Adr				: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FC";
CONSTANT 	C_Rd_EPLD_Vers_Rd_Wr_Stat	: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FA";
CONSTANT 	C_Rd_Skal2_Adr				: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F8";

CONSTANT	C_RdWr_Echo_Reg				: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F6";
CONSTANT	C_RdWr_I2C					: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F4";
CONSTANT	C_RdWr_Loader_Reg2			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F2";
CONSTANT	C_RdWr_Loader_Reg1			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F0";

CONSTANT 	C_Rd_Freq_1_in_10kHz	 	: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"EE";
CONSTANT 	C_Rd_Freq_2_in_10kHz	 	: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"EC";

CONSTANT 	C_Rd_Status_Cnt 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"00";
CONSTANT 	C_Wr_Control_Cnt 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := C_Rd_Status_Cnt;

CONSTANT	C_Strahlweg_Reg_RW			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"60";
CONSTANT	C_Strahlweg_Maske_RW		: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"62";
CONSTANT	C_Interlock_to_SE_RD		: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"64";
CONSTANT	C_Global_Status_RD			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"66";

CONSTANT 	C_HSI_act_pos_edge_RD		: std_logic_vector(7 downto 0) := X"6C"; 
CONSTANT 	C_HSI_neg_edge_RD			: std_logic_vector(7 downto 0) := X"6E"; 
CONSTANT 	C_HSI_act_neg_edge_RD		: std_logic_vector(7 downto 0) := X"70";

CONSTANT 	C_HLI_act_pos_edge_RD		: std_logic_vector(7 downto 0) := X"74";
CONSTANT 	C_HLI_neg_edge_RD			: std_logic_vector(7 downto 0) := X"76";
CONSTANT 	C_HLI_act_neg_edge_RD		: std_logic_vector(7 downto 0) := X"78";

CONSTANT 	C_Interlock_Reg_RW			: std_logic_vector(7 downto 0)	:= X"7A";

CONSTANT 	C_Rd_K0_APK_ID	 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"16";
CONSTANT 	C_Rd_K1_APK_ID	 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"18";
CONSTANT 	C_Rd_K2_APK_ID	 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"46";
CONSTANT 	C_Rd_K3_APK_ID	 			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"48";
CONSTANT 	C_TK8_Delay_RW:				std_logic_vector(7 downto 0)	:= X"7C";

CONSTANT	WR	: STD_LOGIC := '0';
CONSTANT	RD	: STD_LOGIC := '1';


SIGNAL zeit_10ns          : TIME := 10 ns;

SIGNAL	i	: INTEGER;


PROCEDURE MB_Cycle			(
			Mod_Adr_In					: IN STD_LOGIC_VECTOR(4 DOWNTO 0);
			Sub_Adr_In					: IN STD_LOGIC_VECTOR(7 DOWNTO 0);
			RDnWR_In					: IN STD_LOGIC;
			Wr_Data						: IN STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Ist RDnWR_IN = '0' dann wird mit diesem Wort ein Modulbus-Write-Zyklus durchgefuehrt.
			DS_Time_in_ns				: IN INTEGER;
			DS_Gap_in_ns				: IN INTEGER;
			SIGNAL Mod_Adr_Out			: OUT STD_LOGIC_VECTOR(4 DOWNTO 0);
			SIGNAL Sub_Adr_Out			: OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			SIGNAL S_MOD_DATA			: INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			SIGNAL RDnWr_Out			: OUT STD_LOGIC;
			SIGNAL nDS					: OUT STD_LOGIC			
			) IS

		
	BEGIN
	RDnWR_Out <= RDnWR_In;
	Mod_Adr_Out <= (OTHERS => 'H');
	Sub_Adr_Out <= (OTHERS => 'H');
	nDS <= '1';
	wait for 100 ns;
	IF  RDnWR_In = '0' THEN
	 	S_Mod_Data <= WR_Data(15 DOWNTO 8);
	 ELSE 
	 	S_MOD_Data <= (OTHERS => 'Z');
	END IF;
	Mod_Adr_Out <= Mod_Adr_In;
	Sub_Adr_Out <= Sub_Adr_In;
	wait for 100 ns;
	nDS <= '0';
w_dtack1:
	FOR n in 1 to DS_Time_in_ns LOOP
		wait for 1 ns;
		EXIT w_dtack1 when (A_nDTACKA = '0') AND (Async_On = 1);
	END LOOP w_dtack1;
	nDS <= '1';
	wait for 10 ns;	
	Mod_Adr_Out <= (OTHERS => 'H');
	Sub_Adr_Out <= (OTHERS => 'H');	
	wait for DS_Gap_in_ns / 2 * ns;
	IF  RDnWR_In = '0' THEN
	 	S_Mod_Data <= WR_Data(7 DOWNTO 0);
	 ELSE
	 	S_MOD_Data <= (OTHERS => 'Z');
	END IF;
	Mod_Adr_Out <= Mod_Adr_In;
	Sub_Adr_Out(7 DOWNTO 1) <= Sub_Adr_In(7 DOWNTO 1);
	Sub_Adr_Out(0) <= '1';
	wait for DS_Gap_in_ns / 2 * ns;
	nDS <= '0';
w_dtack2:
	FOR n in 1 to DS_Time_in_ns LOOP
		wait for 1 ns;
		EXIT w_dtack2 when (A_nDTACKA = '0') AND (Async_On = 1);
	END LOOP w_dtack2;
--	wait for 10 ns;
	nDS <= '1';
	wait for 100 ns;	
	Mod_Adr_Out <= (OTHERS => 'H');
	Sub_Adr_Out <= (OTHERS => 'H');
	wait for 100 ns;
	END MB_Cycle;
	


BEGIN
	i1 : chopper_m1
	PORT MAP (
-- list connections between master ports and signals
	A_A => A_A,
	A_AUX_A => A_AUX_A,
	A_AUX_B => A_AUX_B,
	A_AUX_C => A_AUX_C,
	A_AUX_CA => A_AUX_CA,
	A_AUX_CB => A_AUX_CB,
	A_Bus_IO => A_Bus_IO,
	A_GR0_16BIT => A_GR0_16BIT,
	A_GR0_APK_ID => A_GR0_APK_ID,
	A_GR1_16BIT => A_GR1_16BIT,
	A_GR1_APK_ID => A_GR1_APK_ID,
	A_I2C_SCL => A_I2C_SCL,
	A_I2C_SDA => A_I2C_SDA,
	A_K0_D => A_K0_D,
	A_K0C_SPG => A_K0C_SPG,
	A_K0D_SPG => A_K0D_SPG,
	A_K1_D => A_K1_D,
	A_K1C_SPG => A_K1C_SPG,
	A_K1D_SPG => A_K1D_SPG,
	A_K2_D => A_K2_D,
	A_K2C_SPG => A_K2C_SPG,
	A_K2D_SPG => A_K2D_SPG,
	A_K3_D => A_K3_D,
	A_K3C_SPG => A_K3C_SPG,
	A_K3D_SPG => A_K3D_SPG,
	A_LA_CLK => A_LA_CLK,
	A_Master_Clk_Out => A_Master_Clk_Out,
	A_Mod_Data => A_Mod_Data,
	A_nDRQA => A_nDRQA,
	A_nDRQB => A_nDRQB,
	A_nDS => A_nDS,
	A_nDTACKA => A_nDTACKA,
	A_nDTACKB => A_nDTACKB,
	A_nGR0_ID_SEL => A_nGR0_ID_SEL,
	A_nGR1_ID_SEL => A_nGR1_ID_SEL,
	A_nINTERLOCKA => A_nINTERLOCKA,
	A_nINTERLOCKB => A_nINTERLOCKB,
	A_nK0_CTRL => A_nK0_CTRL,
	A_nK0_ID_EN => A_nK0_ID_EN,
	A_nK1_CTRL => A_nK1_CTRL,
	A_nK1_ID_EN => A_nK1_ID_EN,
	A_nK2_CTRL => A_nK2_CTRL,
	A_nK2_ID_EN => A_nK2_ID_EN,
	A_nK3_CTRL => A_nK3_CTRL,
	A_nK3_ID_EN => A_nK3_ID_EN,
	A_nMANUAL_RES => A_nMANUAL_RES,
	A_nMB_Reset => A_nMB_Reset,
	A_nSRQA => A_nSRQA,
	A_nSRQB => A_nSRQB,
	A_RDnWR => A_RDnWR,
	A_Sub_Adr => A_Sub_Adr,
	A_Test => A_Test,
	A_VG_A => A_VG_A,
	A_VG_ID => A_VG_ID,
	A_VG_IO_Res => A_VG_IO_Res,
	A_VG_K0_INP => A_VG_K0_INP,
	A_VG_K0_MOD => A_VG_K0_MOD,
	A_VG_K1_INP => A_VG_K1_INP,
	A_VG_K1_MOD => A_VG_K1_MOD,
	A_VG_K2_INP => A_VG_K2_INP,
	A_VG_K2_MOD => A_VG_K2_MOD,
	A_VG_K3_INP => A_VG_K3_INP,
	A_VG_K3_MOD => A_VG_K3_MOD,
	A_VG_Log_ID => A_VG_Log_ID,
	Chopper_Clk => Chopper_Clk,
	CTRL_LOAD => CTRL_LOAD,
	CTRL_RES => CTRL_RES,
	F_TCXO_In => F_TCXO_In,
	INIT_DONE => INIT_DONE,
	LOAD_ERROR => LOAD_ERROR,
	LOAD_OK => LOAD_OK,
	Loader_DB => Loader_DB,
	Loader_Misc => Loader_Misc,
	Loader_WRnRD => Loader_WRnRD,
	nDT_Led => nDT_Led,
	nExt_Data_En => nExt_Data_En,
	nID_OK_Led => nID_OK_Led,
	nIndepend_Clk_Ena => nIndepend_Clk_Ena,
	nK0_SWITCH_ENA => nK0_SWITCH_ENA,
	nK1_SWITCH_ENA => nK1_SWITCH_ENA,
	nK2_SWITCH_ENA => nK2_SWITCH_ENA,
	nK3_SWITCH_ENA => nK3_SWITCH_ENA,
	nLED => nLED,
	nLED_Skal => nLED_Skal,
	nMaster_Clk_ENA => nMaster_Clk_ENA,
	nPowerup_Led => nPowerup_Led,
	nSEL_LED_GRP => nSEL_LED_GRP,
	nSkal_OK_Led => nSkal_OK_Led,
	nSlave_Clk_ENA => nSlave_Clk_ENA,
	RDnWR_Transceiver => RDnWR_Transceiver,
	RELOAD => RELOAD,
	SEL_B => SEL_B,
	TP => TP
	);
	

Mux_In_K0_D(0) <= A_K3_D(0) after 5 us;  	--HSI_ACT <= Chop_HSI_On
Mux_In_K0_D(1) <= A_K3_D(6) after 5 us;	--HLI-ACT <= Chop_HLI_On

P_TCXO: Process
	begin
		loop
	       F_TCXO_In <= NOT F_TCXO_In;
	       wait for 25 ns;
		end loop;
	end process P_TCXO;

P_chopper_clk: Process
	begin
		loop
	       chopper_clk <= NOT chopper_clk;
	       wait for 25 ns;
		end loop;
	end process P_chopper_clk;

P_K3_ID: PROCESS (A_nK3_ID_EN, A_nGR1_ID_SEL, A_VG_K3_INP, Mux_In_K3_D)
	BEGIN
	IF ((A_VG_K3_INP = '1' AND A_nK3_ID_EN = '1') OR (A_VG_K3_INP = '0' AND A_nK3_ID_EN = '0')) AND (A_nGR1_ID_SEL /= '0') THEN
		A_K3_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((A_VG_K3_INP = '1' AND A_nK3_ID_EN = '1') OR (A_VG_K3_INP = '0' AND A_nK3_ID_EN = '0')) AND (A_nGR1_ID_SEL = '0') THEN
		A_K3_D <= X"ED30";
	ELSIF A_VG_K3_INP = '1' THEN
		A_K3_D <=  Mux_In_K3_D;
	ELSE
		A_K3_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;

P_K2_ID: PROCESS (A_nK2_ID_EN, A_nGR1_ID_SEL, A_VG_K2_INP, Mux_In_K2_D)
	BEGIN
	IF ((A_VG_K2_INP = '1' AND A_nK2_ID_EN = '1') OR (A_VG_K2_INP = '0' AND A_nK2_ID_EN = '0')) AND (A_nGR1_ID_SEL /= '0') THEN
		A_K2_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((A_VG_K2_INP = '1' AND A_nK2_ID_EN = '1') OR (A_VG_K2_INP = '0' AND A_nK2_ID_EN = '0')) AND (A_nGR1_ID_SEL = '0') THEN
		A_K2_D <= X"ED30";
	ELSIF A_VG_K2_INP = '1' THEN
		A_K2_D <=  Mux_In_K2_D;
	ELSE
		A_K2_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;
	
P_K1_ID: PROCESS (A_nK1_ID_EN, A_nGR0_ID_SEL, A_VG_K1_INP, Mux_In_K1_D)
	BEGIN
	IF ((A_VG_K1_INP = '1' AND A_nK1_ID_EN = '1') OR (A_VG_K1_INP = '0' AND A_nK1_ID_EN = '0')) AND (A_nGR0_ID_SEL /= '0') THEN
		A_K1_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((A_VG_K1_INP = '1' AND A_nK1_ID_EN = '1') OR (A_VG_K1_INP = '0' AND A_nK1_ID_EN = '0')) AND (A_nGR0_ID_SEL = '0') THEN
		A_K1_D <= X"D047";
	ELSIF A_VG_K1_INP = '1' THEN
		A_K1_D <=  Mux_In_K1_D;
	ELSE
		A_K1_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;

	
P_K0_ID: PROCESS (A_nK0_ID_EN, A_nGR0_ID_SEL, A_VG_K0_INP, Mux_In_K0_D)
	BEGIN
	IF ((A_VG_K0_INP = '1' AND A_nK0_ID_EN = '1') OR (A_VG_K0_INP = '0' AND A_nK0_ID_EN = '0')) AND (A_nGR0_ID_SEL /= '0') THEN
		A_K0_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((A_VG_K0_INP = '1' AND A_nK0_ID_EN = '1') OR (A_VG_K0_INP = '0' AND A_nK0_ID_EN = '0')) AND (A_nGR0_ID_SEL = '0') THEN
		A_K0_D <= X"D040";
	ELSIF A_VG_K0_INP = '1' THEN
		A_K0_D <=  Mux_In_K0_D;
	ELSE
		A_K0_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;


P_Dtack_Test: PROCESS (A_nDS)
	BEGIN
	IF A_nDS'EVENT AND A_nDS = '1' THEN
		IF A_nDTACKA = '1' THEN
			ASSERT FALSE REPORT "Kein Dtack " SEVERITY Failure;
		END IF;
	END IF; 
	END PROCESS;
	
P_Data_Stable_Test: PROCESS (A_Mod_Data)
	BEGIN
		IF ((A_nDTACKA = '0') OR (A_nDTACKB = '0')) AND A_RDnWR = '1' THEN
			ASSERT FALSE REPORT "Datenbus nicht stabil" SEVERITY Failure;
		END IF;
	END PROCESS;
	
P_MB_RD_Memory: PROCESS (A_RDnWR, A_nDTACKA, A_nDTACKB, A_Sub_Adr(0))
	BEGIN
	
		IF ((A_nDTACKA = '0') OR (A_nDTACKB = '0')) AND (A_RDnWR = '1') THEN
			IF A_Sub_Adr(0) = '0' THEN
				S_Last_Rd_Memory(15 DOWNTO 8) <= A_Mod_Data;
			ELSIF A_Sub_Adr(0) = '1' THEN
				S_Last_Rd_Memory(7 DOWNTO 0) <= A_Mod_Data;
			END IF;
		END IF;
	END PROCESS;
	

strahlalarm_test: process
begin
	--Mux_In_K1_D <= not Mux_In_K1_D;
	wait for 5 us;
end process;

check_interlock: process(A_K3_D(5), A_K3_D(10))
begin
	if A_K3_D(5) = '0' or A_K3_D(10) = '0' then
		ASSERT FALSE REPORT "Interlock HSI or HLI occured" SEVERITY Failure;
	end if;
end process;

check_chopper_version: process(A_Sub_Adr, A_Mod_Data, A_nDTACKA)
begin
	if A_Sub_Adr = C_Global_Status_RD + 1 and A_nDTACKA = '0' then
		if A_Mod_Data < x"11" then
			ASSERT FALSE REPORT "Chopper Version failure" SEVERITY Failure;
		end if;
	end if;
end process;

check_timestamps: process(A_Sub_Adr, A_Mod_Data, A_nDTACKA)
begin
	if A_Sub_Adr = C_HSI_act_pos_edge_RD and A_nDTACKA = '0' then
		if A_Mod_Data(A_Mod_Data'high) /= '1' then
			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
			REPORT "Timestamp ungueltig!";
		end if;
		if A_Mod_Data(0) = '1' then
			ASSERT FALSE REPORT "Soll-Signal zu frueh" SEVERITY Failure;
		end if;
		if A_Mod_Data(1) = '1' then
			ASSERT FALSE REPORT "Signal unterbrochen" SEVERITY Failure;
		end if;
	end if;
	if A_Sub_Adr = C_HSI_neg_edge_RD and A_nDTACKA = '0' then
		if A_Mod_Data(A_Mod_Data'high) /= '1' then
			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
			REPORT "Timestamp ungueltig!";
		end if;
	end if;
	if A_Sub_Adr = C_HSI_act_neg_edge_RD and A_nDTACKA = '0' then
		if A_Mod_Data(A_Mod_Data'high) /= '1' then
			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
			REPORT "Timestamp ungueltig!";
		end if;
	end if;
end process;




always : PROCESS                                              
-- optional sensitivity list                                  
-- (        )                                                 
-- variable declarations                                      
BEGIN                                                         

	A_VG_A <= "11000";
	A_VG_ID <= conv_std_logic_vector(39,A_VG_ID'length);
	A_VG_Log_ID <= "000001";
	A_GR0_APK_ID <= '1';
	A_GR1_APK_ID <= '1';
	A_GR0_16BIT	<= '1';
	A_GR1_16BIT <= '1';
	A_VG_K0_INP <= '1';
	A_VG_K1_INP <= '1';
	A_VG_K0_MOD <= "11";
	A_VG_K1_MOD <= "11";
	
	nSEL_LED_GRP <= "10";
	
	Loader_DB <= "HLHL";
	Loader_Misc <= "LLLL";
	
	A_VG_K2_INP <= '0';
	A_VG_K3_INP <= '0';	
	A_VG_K2_MOD <= "11";
	A_VG_K3_MOD <= "11";
	
	
	--Mux_In_K0_D <= x"0000";
	Mux_In_K1_D <= x"FFFF";	-- Strahlalarm Input (Activ Low)
	--Mux_In_K2_D <= X"0000";
	--Mux_In_K3_D <= X"0000";
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00001";
	
	
	
	A_nMB_Reset <= '1';
	wait for 100 ns;
	A_nMB_Reset <= '0';
	wait for 100 ns;
	A_nMB_Reset <= '1';
			
  wait for 2 us;
    
	-- Chopper_HLI (active high)
	Mux_In_K0_D(15 downto 2) <= x"000" & "00";  

	
	REPORT "Test 1: Anforderung vor Strahlpuls, laenger als 10us";
	
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00011"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 40 us;
	
	REPORT "Test2: Anforderung vor Strahlpuls, kuerzer als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	

	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 40 us;
	
	REPORT "Test3: Anforderung nach Strahlpuls, laenger als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00001"; -- kein Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; -- Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 40 us;
	
	REPORT "Test4: Anforderung vor Strahlpuls beendet";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 40 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 30 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 40 us;
	
	REPORT "Test5: Anforderung vor Strahlpuls, waehrend Strahlpus unterbrochen";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	A_Bus_IO <= "00000"; --  Strahl anfordern
	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;
	

	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, A_A, A_Sub_Adr, A_MOD_DATA, A_RDnWR, A_nDS);
	wait for 8*zeit_10ns;

	--wait for 40 us;

	ASSERT FALSE REPORT "Testbench beendet" SEVERITY Failure;
        -- code executes for every event on sensitivity list  
WAIT;                                                        
END PROCESS always;                                          
END chopper_m1_arch;
