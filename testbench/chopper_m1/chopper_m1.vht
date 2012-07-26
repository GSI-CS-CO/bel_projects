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
SIGNAL A_K0C_SPG : STD_LOGIC;
SIGNAL A_K0D_SPG : STD_LOGIC;
SIGNAL A_K1_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K1C_SPG : STD_LOGIC;
SIGNAL A_K1D_SPG : STD_LOGIC;
SIGNAL A_K2_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K2C_SPG : STD_LOGIC;
SIGNAL A_K2D_SPG : STD_LOGIC;
SIGNAL A_K3_D : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_K3C_SPG : STD_LOGIC;
SIGNAL A_K3D_SPG : STD_LOGIC;
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
SIGNAL Chopper_Clk : STD_LOGIC;
SIGNAL CTRL_LOAD : STD_LOGIC;
SIGNAL CTRL_RES : STD_LOGIC;
SIGNAL F_TCXO_In : STD_LOGIC;
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
init : PROCESS                                               
-- variable declarations                                     
BEGIN                                                        
        -- code that executes only once                      
WAIT;                                                       
END PROCESS init;                                           
always : PROCESS                                              
-- optional sensitivity list                                  
-- (        )                                                 
-- variable declarations                                      
BEGIN                                                         
        -- code executes for every event on sensitivity list  
WAIT;                                                        
END PROCESS always;                                          
END chopper_m1_arch;
