-- Copyright (C) 1991-2009 Altera Corporation
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

-- VENDOR "Altera"
-- PROGRAM "Quartus II"
-- VERSION "Version 9.0 Build 235 06/17/2009 Service Pack 2 SJ Full Version"

-- DATE "08/27/2009 13:19:57"

-- 
-- Device: Altera EP2C5F256C6 Package FBGA256
-- 

-- 
-- This VHDL file should be used for ModelSim-Altera (VHDL) only
-- 

LIBRARY IEEE, cycloneii;
USE IEEE.std_logic_1164.all;
USE cycloneii.cycloneii_components.all;

ENTITY 	sweep_04 IS
    PORT (
	Stat_Sel : OUT std_logic;
	SWEEP_SEL : IN std_logic;
	TAKE_DA : IN std_logic;
	PU_RESET : IN std_logic;
	CLK : IN std_logic;
	FKT : IN std_logic_vector(7 DOWNTO 0);
	\Sweep-Out\ : OUT std_logic_vector(15 DOWNTO 0);
	\/HW_Trig\ : IN std_logic;
	D : IN std_logic_vector(15 DOWNTO 0);
	\Sweep-Stat\ : OUT std_logic_vector(15 DOWNTO 0);
	\SWEEP-VERS\ : OUT std_logic_vector(3 DOWNTO 0)
	);
END sweep_04;

ARCHITECTURE structure OF sweep_04 IS
SIGNAL gnd : std_logic := '0';
SIGNAL vcc : std_logic := '1';
SIGNAL devoe : std_logic := '1';
SIGNAL devclrn : std_logic := '1';
SIGNAL devpor : std_logic := '1';
SIGNAL ww_devoe : std_logic;
SIGNAL ww_devclrn : std_logic;
SIGNAL ww_devpor : std_logic;
SIGNAL ww_Stat_Sel : std_logic;
SIGNAL ww_SWEEP_SEL : std_logic;
SIGNAL ww_TAKE_DA : std_logic;
SIGNAL ww_PU_RESET : std_logic;
SIGNAL ww_CLK : std_logic;
SIGNAL ww_FKT : std_logic_vector(7 DOWNTO 0);
SIGNAL \ww_Sweep-Out\ : std_logic_vector(15 DOWNTO 0);
SIGNAL \ww_/HW_Trig\ : std_logic;
SIGNAL ww_D : std_logic_vector(15 DOWNTO 0);
SIGNAL \ww_Sweep-Stat\ : std_logic_vector(15 DOWNTO 0);
SIGNAL \ww_SWEEP-VERS\ : std_logic_vector(3 DOWNTO 0);
SIGNAL \Deco|RES_DFF~clkctrl_INCLK_bus\ : std_logic_vector(3 DOWNTO 0);
SIGNAL \120~clkctrl_INCLK_bus\ : std_logic_vector(3 DOWNTO 0);
SIGNAL \St1|Init_HW_DFF~clkctrl_INCLK_bus\ : std_logic_vector(3 DOWNTO 0);
SIGNAL \CLK~clkctrl_INCLK_bus\ : std_logic_vector(3 DOWNTO 0);
SIGNAL \SW_SUB|auto_generated|result_int[0]~0_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[2]~4_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[3]~6_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[0]~19_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[1]~21_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[3]~25_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[5]~29_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[11]~41_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[12]~43_combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita0~combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita2~combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita4~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita0~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita2~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita3~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita5~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita7~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita9~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita0~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita2~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita4~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita11~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita13~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita14~combout\ : std_logic;
SIGNAL \St1|SM_Idle~1_combout\ : std_logic;
SIGNAL \rtl~1_combout\ : std_logic;
SIGNAL \rtl~2_combout\ : std_logic;
SIGNAL \rtl~4_combout\ : std_logic;
SIGNAL \Deco|Soft_Trig_ff~4_combout\ : std_logic;
SIGNAL \St1|SM_Wr_Delta~1_combout\ : std_logic;
SIGNAL \St1|Delta_Not_Zero~3_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[10]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[9]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[7]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[6]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[5]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[3]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[2]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[0]~feeder_combout\ : std_logic;
SIGNAL \CLK~combout\ : std_logic;
SIGNAL \CLK~clkctrl_outclk\ : std_logic;
SIGNAL \TAKE_DA~combout\ : std_logic;
SIGNAL \Deco|STAT1_DFF~2_combout\ : std_logic;
SIGNAL \Deco|_~83_combout\ : std_logic;
SIGNAL \Deco|STAT1_DFF~3_combout\ : std_logic;
SIGNAL \Deco|_~84_combout\ : std_logic;
SIGNAL \PU_RESET~combout\ : std_logic;
SIGNAL \SWEEP_SEL~combout\ : std_logic;
SIGNAL \Deco|RES_DFF~1_combout\ : std_logic;
SIGNAL \Deco|RES_DFF~0_combout\ : std_logic;
SIGNAL \Deco|RES_DFF~regout\ : std_logic;
SIGNAL \Deco|RES_DFF~clkctrl_outclk\ : std_logic;
SIGNAL \Deco|STAT1_DFF~regout\ : std_logic;
SIGNAL \Flat_Reg|dffs[15]~feeder_combout\ : std_logic;
SIGNAL \Deco|Ld_Delay_DFF~2_combout\ : std_logic;
SIGNAL \Deco|Ld_Delay_DFF~1_combout\ : std_logic;
SIGNAL \Deco|Ld_Delay_DFF~regout\ : std_logic;
SIGNAL \Deco|Ld_Delta_DFF~1_combout\ : std_logic;
SIGNAL \Deco|Ld_Delta_DFF~regout\ : std_logic;
SIGNAL \Deco|Ld_Flattop_Int_DFF~2_combout\ : std_logic;
SIGNAL \Deco|Ld_Flattop_Int_DFF~1_combout\ : std_logic;
SIGNAL \Deco|Ld_Flattop_Int_DFF~regout\ : std_logic;
SIGNAL \Deco|Set_Flattop_DFF~1_combout\ : std_logic;
SIGNAL \Deco|Set_Flattop_DFF~regout\ : std_logic;
SIGNAL \St1|SM_Seq_Err~2_combout\ : std_logic;
SIGNAL \St1|SM_Seq_Err~3_combout\ : std_logic;
SIGNAL \St1|_~90_combout\ : std_logic;
SIGNAL \St1|_~96_combout\ : std_logic;
SIGNAL \St1|SM_Wr_Delta~2_combout\ : std_logic;
SIGNAL \St1|SM_Wr_Delta~regout\ : std_logic;
SIGNAL \St1|SM_Wr_Delay~1_combout\ : std_logic;
SIGNAL \St1|SM_Wr_Delay~0_combout\ : std_logic;
SIGNAL \St1|SM_Wr_Delay~regout\ : std_logic;
SIGNAL \St1|_~91_combout\ : std_logic;
SIGNAL \St1|_~95_combout\ : std_logic;
SIGNAL \St1|SM_Wr_FT_Int~1_combout\ : std_logic;
SIGNAL \St1|SM_Wr_FT_Int~0_combout\ : std_logic;
SIGNAL \St1|SM_Wr_FT_Int~regout\ : std_logic;
SIGNAL \St1|_~88_combout\ : std_logic;
SIGNAL \St1|_~87_combout\ : std_logic;
SIGNAL \St1|SM_Set_Flattop~0_combout\ : std_logic;
SIGNAL \St1|SM_Set_Flattop~regout\ : std_logic;
SIGNAL \St1|_~89_combout\ : std_logic;
SIGNAL \St1|SM_Seq_Err~1_combout\ : std_logic;
SIGNAL \St1|SM_Seq_Err~4_combout\ : std_logic;
SIGNAL \St1|SM_Seq_Err~regout\ : std_logic;
SIGNAL \St1|Seq_Err_FF~3_combout\ : std_logic;
SIGNAL \St1|Seq_Err_FF~regout\ : std_logic;
SIGNAL \St1|_~92_combout\ : std_logic;
SIGNAL \St1|SM_Idle~2_combout\ : std_logic;
SIGNAL \St1|SM_Idle~regout\ : std_logic;
SIGNAL \Flat_Reg|dffs[12]~feeder_combout\ : std_logic;
SIGNAL \St1|Wr_FT_Int~combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[11]~feeder_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[8]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[11]~feeder_combout\ : std_logic;
SIGNAL \~GND~combout\ : std_logic;
SIGNAL \Deco|Soft_Trig_ff~2_combout\ : std_logic;
SIGNAL \Deco|Soft_Trig_ff~3_combout\ : std_logic;
SIGNAL \Deco|Stop_DFF~1_combout\ : std_logic;
SIGNAL \Deco|Stop_DFF~regout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita1~combout\ : std_logic;
SIGNAL \St1|_~86_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[9]~feeder_combout\ : std_logic;
SIGNAL \/HW_Trig~combout\ : std_logic;
SIGNAL \Deco|Soft_Trig_ff~0_combout\ : std_logic;
SIGNAL \Deco|Soft_Trig_ff~regout\ : std_logic;
SIGNAL \Deco|Ena_Soft_Trig_Dff~8_combout\ : std_logic;
SIGNAL \Deco|Ena_Soft_Trig_Dff~regout\ : std_logic;
SIGNAL \St1|Trigger_Sync~1_combout\ : std_logic;
SIGNAL \St1|Trigger_Sync~regout\ : std_logic;
SIGNAL \St1|Trigger_Sync_1~regout\ : std_logic;
SIGNAL \St1|Trigger_FF~5_combout\ : std_logic;
SIGNAL \St1|Trigger_FF~6_combout\ : std_logic;
SIGNAL \St1|Trigger_FF~regout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita1~combout\ : std_logic;
SIGNAL \St1|Wr_Delay~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|_~2_combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita4~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita6~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita8~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita10~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita11~combout\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita11~COUT\ : std_logic;
SIGNAL \St1|Delay_Timer|auto_generated|counter_comb_bita12~combout\ : std_logic;
SIGNAL \St1|Delay_Fin~0_combout\ : std_logic;
SIGNAL \St1|Delay_Fin~regout\ : std_logic;
SIGNAL \St1|S_Stop_Delta~5_combout\ : std_logic;
SIGNAL \114~combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[8]~feeder_combout\ : std_logic;
SIGNAL \St1|Delta_Not_Zero~1_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[5]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[4]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[7]~feeder_combout\ : std_logic;
SIGNAL \St1|Delta_Not_Zero~2_combout\ : std_logic;
SIGNAL \St1|Delta_Not_Zero~4_combout\ : std_logic;
SIGNAL \St1|Delta_Not_Zero~regout\ : std_logic;
SIGNAL \St1|_~93_combout\ : std_logic;
SIGNAL \St1|SM_W_Start~0_combout\ : std_logic;
SIGNAL \St1|SM_W_Start~regout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|_~57_combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita3~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita5~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita6~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita7~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita8~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita9~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita10~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita12~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita15~combout\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita15~COUT\ : std_logic;
SIGNAL \St1|TO_Timer|auto_generated|counter_comb_bita16~combout\ : std_logic;
SIGNAL \St1|Timeout~regout\ : std_logic;
SIGNAL \St1|S_Stop_Delta~6_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[3]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[1]~feeder_combout\ : std_logic;
SIGNAL \Delta_Reg|dffs[0]~feeder_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[0]~20\ : std_logic;
SIGNAL \SUB_REG|dffs[1]~22\ : std_logic;
SIGNAL \SUB_REG|dffs[2]~23_combout\ : std_logic;
SIGNAL \ADD_C~0_combout\ : std_logic;
SIGNAL \St1|_~85_combout\ : std_logic;
SIGNAL \St1|SM_Stop~0_combout\ : std_logic;
SIGNAL \St1|SM_Stop~regout\ : std_logic;
SIGNAL \St1|Stop_DFF~regout\ : std_logic;
SIGNAL \St1|_~94_combout\ : std_logic;
SIGNAL \St1|SM_Work~0_combout\ : std_logic;
SIGNAL \St1|SM_Work~regout\ : std_logic;
SIGNAL \St1|Work_DFF~feeder_combout\ : std_logic;
SIGNAL \St1|Work_DFF~regout\ : std_logic;
SIGNAL \120~combout\ : std_logic;
SIGNAL \120~clkctrl_outclk\ : std_logic;
SIGNAL \ADD_C~regout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita0~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita1~combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita1~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita2~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita3~combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita3~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita4~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita5~combout\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita5~COUT\ : std_logic;
SIGNAL \RND_CNT|auto_generated|counter_comb_bita6~combout\ : std_logic;
SIGNAL \136~combout\ : std_logic;
SIGNAL \SUB_REG|dffs[2]~24\ : std_logic;
SIGNAL \SUB_REG|dffs[3]~26\ : std_logic;
SIGNAL \SUB_REG|dffs[4]~27_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[4]~28\ : std_logic;
SIGNAL \SUB_REG|dffs[5]~30\ : std_logic;
SIGNAL \SUB_REG|dffs[6]~31_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[6]~32\ : std_logic;
SIGNAL \SUB_REG|dffs[7]~33_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[7]~34\ : std_logic;
SIGNAL \SUB_REG|dffs[8]~35_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[8]~36\ : std_logic;
SIGNAL \SUB_REG|dffs[9]~38\ : std_logic;
SIGNAL \SUB_REG|dffs[10]~39_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[10]~40\ : std_logic;
SIGNAL \SUB_REG|dffs[11]~42\ : std_logic;
SIGNAL \SUB_REG|dffs[12]~44\ : std_logic;
SIGNAL \SUB_REG|dffs[13]~45_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[13]~46\ : std_logic;
SIGNAL \SUB_REG|dffs[14]~48\ : std_logic;
SIGNAL \SUB_REG|dffs[15]~50\ : std_logic;
SIGNAL \SUB_REG|dffs[16]~51_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[16]~52\ : std_logic;
SIGNAL \SUB_REG|dffs[17]~53_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[15]~49_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[14]~47_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[1]~feeder_combout\ : std_logic;
SIGNAL \SUB_REG|dffs[9]~37_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[0]~1\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[1]~2_combout\ : std_logic;
SIGNAL \rtl~3_combout\ : std_logic;
SIGNAL \SUB_C~regout\ : std_logic;
SIGNAL \11~combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[1]~3\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[2]~5\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[3]~7\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[4]~8_combout\ : std_logic;
SIGNAL \rtl~0_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[4]~9\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[5]~10_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[5]~57_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[5]~11\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[6]~12_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[6]~56_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[6]~13\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[7]~15\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[8]~17\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[9]~19\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[10]~21\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[11]~23\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[12]~24_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[12]~50_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[12]~25\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[13]~26_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[13]~49_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[13]~27\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[14]~29\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[15]~31\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[16]~32_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[16]~46_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[16]~33\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[17]~34_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[17]~45_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[17]~35\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[18]~36_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[13]~feeder_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[18]~44_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[18]~37\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[19]~39\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[20]~41\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[21]~42_combout\ : std_logic;
SIGNAL \R_Fin~regout\ : std_logic;
SIGNAL \St1|Init_HW_DFF~0_combout\ : std_logic;
SIGNAL \St1|Init_HW_DFF~regout\ : std_logic;
SIGNAL \St1|Init_HW_DFF~clkctrl_outclk\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[20]~40_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[20]~42_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[19]~38_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[14]~feeder_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[19]~43_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[15]~30_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[10]~feeder_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[15]~47_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[14]~28_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[14]~48_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[11]~22_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[6]~feeder_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[11]~51_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[10]~20_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[10]~52_combout\ : std_logic;
SIGNAL \Flat_Reg|dffs[4]~feeder_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[9]~18_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[9]~53_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[8]~16_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[8]~54_combout\ : std_logic;
SIGNAL \SW_SUB|auto_generated|result_int[7]~14_combout\ : std_logic;
SIGNAL \3|$00000|auto_generated|result_node[7]~55_combout\ : std_logic;
SIGNAL \St1|W_Start_DFF~feeder_combout\ : std_logic;
SIGNAL \St1|W_Start_DFF~regout\ : std_logic;
SIGNAL \St1|Idle~combout\ : std_logic;
SIGNAL \St1|Stop_Exec_SRFF~3_combout\ : std_logic;
SIGNAL \St1|Stop_Exec_SRFF~regout\ : std_logic;
SIGNAL \St1|Wr_Delta~combout\ : std_logic;
SIGNAL \St1|TO_Err_FF~3_combout\ : std_logic;
SIGNAL \St1|TO_Err_FF~regout\ : std_logic;
SIGNAL \Flat_Reg|dffs\ : std_logic_vector(15 DOWNTO 0);
SIGNAL \Delta_Reg|dffs\ : std_logic_vector(11 DOWNTO 0);
SIGNAL \RND_CNT|auto_generated|pre_hazard\ : std_logic_vector(6 DOWNTO 0);
SIGNAL \SUB_REG|dffs\ : std_logic_vector(17 DOWNTO 0);
SIGNAL \St1|Delay_Timer|auto_generated|safe_q\ : std_logic_vector(12 DOWNTO 0);
SIGNAL \St1|TO_Timer|auto_generated|safe_q\ : std_logic_vector(16 DOWNTO 0);
SIGNAL \SW_Reg|dffs\ : std_logic_vector(20 DOWNTO 0);
SIGNAL \D~combout\ : std_logic_vector(15 DOWNTO 0);
SIGNAL \FKT~combout\ : std_logic_vector(7 DOWNTO 0);

BEGIN

Stat_Sel <= ww_Stat_Sel;
ww_SWEEP_SEL <= SWEEP_SEL;
ww_TAKE_DA <= TAKE_DA;
ww_PU_RESET <= PU_RESET;
ww_CLK <= CLK;
ww_FKT <= FKT;
\Sweep-Out\ <= \ww_Sweep-Out\;
\ww_/HW_Trig\ <= \/HW_Trig\;
ww_D <= D;
\Sweep-Stat\ <= \ww_Sweep-Stat\;
\SWEEP-VERS\ <= \ww_SWEEP-VERS\;
ww_devoe <= devoe;
ww_devclrn <= devclrn;
ww_devpor <= devpor;

\Deco|RES_DFF~clkctrl_INCLK_bus\ <= (gnd & gnd & gnd & \Deco|RES_DFF~regout\);

\120~clkctrl_INCLK_bus\ <= (gnd & gnd & gnd & \120~combout\);

\St1|Init_HW_DFF~clkctrl_INCLK_bus\ <= (gnd & gnd & gnd & \St1|Init_HW_DFF~regout\);

\CLK~clkctrl_INCLK_bus\ <= (gnd & gnd & gnd & \CLK~combout\);

\SUB_REG|dffs[12]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[12]~43_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(12));

\SUB_REG|dffs[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[11]~41_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(11));

\SW_SUB|auto_generated|result_int[0]~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[0]~0_combout\ = \SW_Reg|dffs\(0) & (GND # !\SUB_REG|dffs\(6)) # !\SW_Reg|dffs\(0) & (\SUB_REG|dffs\(6) $ GND)
-- \SW_SUB|auto_generated|result_int[0]~1\ = CARRY(\SW_Reg|dffs\(0) # !\SUB_REG|dffs\(6))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110011010111011",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(0),
	datab => \SUB_REG|dffs\(6),
	datad => VCC,
	combout => \SW_SUB|auto_generated|result_int[0]~0_combout\,
	cout => \SW_SUB|auto_generated|result_int[0]~1\);

\SW_SUB|auto_generated|result_int[2]~4\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[2]~4_combout\ = (\SW_Reg|dffs\(2) $ \SUB_REG|dffs\(8) $ \SW_SUB|auto_generated|result_int[1]~3\) # GND
-- \SW_SUB|auto_generated|result_int[2]~5\ = CARRY(\SW_Reg|dffs\(2) & (!\SW_SUB|auto_generated|result_int[1]~3\ # !\SUB_REG|dffs\(8)) # !\SW_Reg|dffs\(2) & !\SUB_REG|dffs\(8) & !\SW_SUB|auto_generated|result_int[1]~3\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000101011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(2),
	datab => \SUB_REG|dffs\(8),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[1]~3\,
	combout => \SW_SUB|auto_generated|result_int[2]~4_combout\,
	cout => \SW_SUB|auto_generated|result_int[2]~5\);

\SW_SUB|auto_generated|result_int[3]~6\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[3]~6_combout\ = \SW_Reg|dffs\(3) & (\SUB_REG|dffs\(9) & !\SW_SUB|auto_generated|result_int[2]~5\ # !\SUB_REG|dffs\(9) & \SW_SUB|auto_generated|result_int[2]~5\ & VCC) # !\SW_Reg|dffs\(3) & (\SUB_REG|dffs\(9) & 
-- (\SW_SUB|auto_generated|result_int[2]~5\ # GND) # !\SUB_REG|dffs\(9) & !\SW_SUB|auto_generated|result_int[2]~5\)
-- \SW_SUB|auto_generated|result_int[3]~7\ = CARRY(\SW_Reg|dffs\(3) & \SUB_REG|dffs\(9) & !\SW_SUB|auto_generated|result_int[2]~5\ # !\SW_Reg|dffs\(3) & (\SUB_REG|dffs\(9) # !\SW_SUB|auto_generated|result_int[2]~5\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100101001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(3),
	datab => \SUB_REG|dffs\(9),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[2]~5\,
	combout => \SW_SUB|auto_generated|result_int[3]~6_combout\,
	cout => \SW_SUB|auto_generated|result_int[3]~7\);

\Delta_Reg|dffs[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[10]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(10));

\Delta_Reg|dffs[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[6]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(6));

\SUB_REG|dffs[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[5]~29_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(5));

\SUB_REG|dffs[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[3]~25_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(3));

\Delta_Reg|dffs[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[2]~feeder_combout\,
	sdata => \~GND~combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(2));

\SUB_REG|dffs[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[1]~21_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(1));

\SUB_REG|dffs[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[0]~19_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(0));

\SUB_REG|dffs[0]~19\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[0]~19_combout\ = \SUB_REG|dffs\(0) & (\Delta_Reg|dffs\(0) $ VCC) # !\SUB_REG|dffs\(0) & \Delta_Reg|dffs\(0) & VCC
-- \SUB_REG|dffs[0]~20\ = CARRY(\SUB_REG|dffs\(0) & \Delta_Reg|dffs\(0))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110011010001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(0),
	datab => \Delta_Reg|dffs\(0),
	datad => VCC,
	combout => \SUB_REG|dffs[0]~19_combout\,
	cout => \SUB_REG|dffs[0]~20\);

\SUB_REG|dffs[1]~21\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[1]~21_combout\ = \SUB_REG|dffs\(1) & (\Delta_Reg|dffs\(1) & \SUB_REG|dffs[0]~20\ & VCC # !\Delta_Reg|dffs\(1) & !\SUB_REG|dffs[0]~20\) # !\SUB_REG|dffs\(1) & (\Delta_Reg|dffs\(1) & !\SUB_REG|dffs[0]~20\ # !\Delta_Reg|dffs\(1) & 
-- (\SUB_REG|dffs[0]~20\ # GND))
-- \SUB_REG|dffs[1]~22\ = CARRY(\SUB_REG|dffs\(1) & !\Delta_Reg|dffs\(1) & !\SUB_REG|dffs[0]~20\ # !\SUB_REG|dffs\(1) & (!\SUB_REG|dffs[0]~20\ # !\Delta_Reg|dffs\(1)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(1),
	datab => \Delta_Reg|dffs\(1),
	datad => VCC,
	cin => \SUB_REG|dffs[0]~20\,
	combout => \SUB_REG|dffs[1]~21_combout\,
	cout => \SUB_REG|dffs[1]~22\);

\SUB_REG|dffs[3]~25\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[3]~25_combout\ = \SUB_REG|dffs\(3) & (\Delta_Reg|dffs\(3) & \SUB_REG|dffs[2]~24\ & VCC # !\Delta_Reg|dffs\(3) & !\SUB_REG|dffs[2]~24\) # !\SUB_REG|dffs\(3) & (\Delta_Reg|dffs\(3) & !\SUB_REG|dffs[2]~24\ # !\Delta_Reg|dffs\(3) & 
-- (\SUB_REG|dffs[2]~24\ # GND))
-- \SUB_REG|dffs[3]~26\ = CARRY(\SUB_REG|dffs\(3) & !\Delta_Reg|dffs\(3) & !\SUB_REG|dffs[2]~24\ # !\SUB_REG|dffs\(3) & (!\SUB_REG|dffs[2]~24\ # !\Delta_Reg|dffs\(3)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(3),
	datab => \Delta_Reg|dffs\(3),
	datad => VCC,
	cin => \SUB_REG|dffs[2]~24\,
	combout => \SUB_REG|dffs[3]~25_combout\,
	cout => \SUB_REG|dffs[3]~26\);

\SUB_REG|dffs[5]~29\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[5]~29_combout\ = \SUB_REG|dffs\(5) & (\Delta_Reg|dffs\(5) & \SUB_REG|dffs[4]~28\ & VCC # !\Delta_Reg|dffs\(5) & !\SUB_REG|dffs[4]~28\) # !\SUB_REG|dffs\(5) & (\Delta_Reg|dffs\(5) & !\SUB_REG|dffs[4]~28\ # !\Delta_Reg|dffs\(5) & 
-- (\SUB_REG|dffs[4]~28\ # GND))
-- \SUB_REG|dffs[5]~30\ = CARRY(\SUB_REG|dffs\(5) & !\Delta_Reg|dffs\(5) & !\SUB_REG|dffs[4]~28\ # !\SUB_REG|dffs\(5) & (!\SUB_REG|dffs[4]~28\ # !\Delta_Reg|dffs\(5)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(5),
	datab => \Delta_Reg|dffs\(5),
	datad => VCC,
	cin => \SUB_REG|dffs[4]~28\,
	combout => \SUB_REG|dffs[5]~29_combout\,
	cout => \SUB_REG|dffs[5]~30\);

\SUB_REG|dffs[11]~41\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[11]~41_combout\ = \SUB_REG|dffs\(11) & (\Delta_Reg|dffs\(11) & \SUB_REG|dffs[10]~40\ & VCC # !\Delta_Reg|dffs\(11) & !\SUB_REG|dffs[10]~40\) # !\SUB_REG|dffs\(11) & (\Delta_Reg|dffs\(11) & !\SUB_REG|dffs[10]~40\ # !\Delta_Reg|dffs\(11) & 
-- (\SUB_REG|dffs[10]~40\ # GND))
-- \SUB_REG|dffs[11]~42\ = CARRY(\SUB_REG|dffs\(11) & !\Delta_Reg|dffs\(11) & !\SUB_REG|dffs[10]~40\ # !\SUB_REG|dffs\(11) & (!\SUB_REG|dffs[10]~40\ # !\Delta_Reg|dffs\(11)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(11),
	datab => \Delta_Reg|dffs\(11),
	datad => VCC,
	cin => \SUB_REG|dffs[10]~40\,
	combout => \SUB_REG|dffs[11]~41_combout\,
	cout => \SUB_REG|dffs[11]~42\);

\SUB_REG|dffs[12]~43\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[12]~43_combout\ = \SUB_REG|dffs\(12) & (\SUB_REG|dffs[11]~42\ $ GND) # !\SUB_REG|dffs\(12) & !\SUB_REG|dffs[11]~42\ & VCC
-- \SUB_REG|dffs[12]~44\ = CARRY(\SUB_REG|dffs\(12) & !\SUB_REG|dffs[11]~42\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100001010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(12),
	datad => VCC,
	cin => \SUB_REG|dffs[11]~42\,
	combout => \SUB_REG|dffs[12]~43_combout\,
	cout => \SUB_REG|dffs[12]~44\);

\RND_CNT|auto_generated|counter_reg_bit1a[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita4~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(4));

\RND_CNT|auto_generated|counter_reg_bit1a[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita2~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(2));

\RND_CNT|auto_generated|counter_reg_bit1a[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita0~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(0));

\RND_CNT|auto_generated|counter_comb_bita0\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita0~combout\ = \RND_CNT|auto_generated|pre_hazard\(0) $ VCC
-- \RND_CNT|auto_generated|counter_comb_bita0~COUT\ = CARRY(\RND_CNT|auto_generated|pre_hazard\(0))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101010110101010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \RND_CNT|auto_generated|pre_hazard\(0),
	datad => VCC,
	combout => \RND_CNT|auto_generated|counter_comb_bita0~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita0~COUT\);

\RND_CNT|auto_generated|counter_comb_bita2\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita2~combout\ = \RND_CNT|auto_generated|pre_hazard\(2) & (\RND_CNT|auto_generated|counter_comb_bita1~COUT\ $ GND) # !\RND_CNT|auto_generated|pre_hazard\(2) & !\RND_CNT|auto_generated|counter_comb_bita1~COUT\ & VCC
-- \RND_CNT|auto_generated|counter_comb_bita2~COUT\ = CARRY(\RND_CNT|auto_generated|pre_hazard\(2) & !\RND_CNT|auto_generated|counter_comb_bita1~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100001010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \RND_CNT|auto_generated|pre_hazard\(2),
	datad => VCC,
	cin => \RND_CNT|auto_generated|counter_comb_bita1~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita2~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita2~COUT\);

\RND_CNT|auto_generated|counter_comb_bita4\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita4~combout\ = \RND_CNT|auto_generated|pre_hazard\(4) & (\RND_CNT|auto_generated|counter_comb_bita3~COUT\ $ GND) # !\RND_CNT|auto_generated|pre_hazard\(4) & !\RND_CNT|auto_generated|counter_comb_bita3~COUT\ & VCC
-- \RND_CNT|auto_generated|counter_comb_bita4~COUT\ = CARRY(\RND_CNT|auto_generated|pre_hazard\(4) & !\RND_CNT|auto_generated|counter_comb_bita3~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100001010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \RND_CNT|auto_generated|pre_hazard\(4),
	datad => VCC,
	cin => \RND_CNT|auto_generated|counter_comb_bita3~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita4~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita4~COUT\);

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita9~combout\,
	sdata => \D~combout\(9),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(9));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita7~combout\,
	sdata => \D~combout\(7),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(7));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita5~combout\,
	sdata => \D~combout\(5),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(5));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita3~combout\,
	sdata => \D~combout\(3),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(3));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita2~combout\,
	sdata => \D~combout\(2),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(2));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita0~combout\,
	sdata => \D~combout\(0),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(0));

\St1|Delay_Timer|auto_generated|counter_comb_bita0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita0~combout\ = !\St1|Delay_Timer|auto_generated|safe_q\(0)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(0))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101010101010101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(0),
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita0~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\);

\St1|Delay_Timer|auto_generated|counter_comb_bita2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita2~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(2) & \St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\ & VCC # !\St1|Delay_Timer|auto_generated|safe_q\(2) & 
-- !\St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\
-- \St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(2) & !\St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(2),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita2~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\);

\St1|Delay_Timer|auto_generated|counter_comb_bita3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita3~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(3) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(3) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(3) # !\St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(3),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita2~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita3~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\);

\St1|Delay_Timer|auto_generated|counter_comb_bita5\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita5~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(5) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(5) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(5) # !\St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(5),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita5~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\);

\St1|Delay_Timer|auto_generated|counter_comb_bita7\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita7~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(7) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(7) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(7) # !\St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(7),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita7~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\);

\St1|Delay_Timer|auto_generated|counter_comb_bita9\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita9~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(9) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(9) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(9) # !\St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delay_Timer|auto_generated|safe_q\(9),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita9~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[14]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita14~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(14));

\St1|TO_Timer|auto_generated|counter_reg_bit1a[13]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita13~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(13));

\St1|TO_Timer|auto_generated|counter_reg_bit1a[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita11~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(11));

\St1|TO_Timer|auto_generated|counter_reg_bit1a[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita4~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(4));

\St1|TO_Timer|auto_generated|counter_reg_bit1a[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita2~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(2));

\St1|TO_Timer|auto_generated|counter_reg_bit1a[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita0~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(0));

\St1|TO_Timer|auto_generated|counter_comb_bita0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita0~combout\ = !\St1|TO_Timer|auto_generated|safe_q\(0)
-- \St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(0))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101010101010101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(0),
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita0~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\);

\St1|TO_Timer|auto_generated|counter_comb_bita2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita2~combout\ = \St1|TO_Timer|auto_generated|safe_q\(2) & \St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(2) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(2) & !\St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(2),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita2~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\);

\St1|TO_Timer|auto_generated|counter_comb_bita4\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita4~combout\ = \St1|TO_Timer|auto_generated|safe_q\(4) & \St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(4) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(4) & !\St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(4),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita4~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\);

\St1|TO_Timer|auto_generated|counter_comb_bita11\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita11~combout\ = \St1|TO_Timer|auto_generated|safe_q\(11) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(11) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(11) # !\St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(11),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita11~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\);

\St1|TO_Timer|auto_generated|counter_comb_bita13\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita13~combout\ = \St1|TO_Timer|auto_generated|safe_q\(13) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(13) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(13) # !\St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(13),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita13~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\);

\St1|TO_Timer|auto_generated|counter_comb_bita14\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita14~combout\ = \St1|TO_Timer|auto_generated|safe_q\(14) & \St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(14) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(14) & !\St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(14),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita13~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita14~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\);

\SW_Reg|dffs[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \rtl~1_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(3));

\SW_Reg|dffs[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \rtl~2_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(2));

\SW_Reg|dffs[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \rtl~4_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(0));

\Flat_Reg|dffs[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[9]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(9));

\Flat_Reg|dffs[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[7]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(7));

\Flat_Reg|dffs[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[5]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(5));

\Flat_Reg|dffs[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[3]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(3));

\Flat_Reg|dffs[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	sdata => \D~combout\(2),
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	sload => VCC,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(2));

\Flat_Reg|dffs[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[0]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(0));

\St1|SM_Idle~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Idle~1_combout\ = \St1|SM_Seq_Err~regout\ # \R_Fin~regout\ & (\St1|SM_Work~regout\ # \St1|SM_Stop~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111010101010",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Seq_Err~regout\,
	datab => \St1|SM_Work~regout\,
	datac => \St1|SM_Stop~regout\,
	datad => \R_Fin~regout\,
	combout => \St1|SM_Idle~1_combout\);

\rtl~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \rtl~1_combout\ = \SW_SUB|auto_generated|result_int[3]~6_combout\ & (!\Deco|Set_Flattop_DFF~regout\ # !\St1|SM_Set_Flattop~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Set_Flattop~regout\,
	datac => \SW_SUB|auto_generated|result_int[3]~6_combout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \rtl~1_combout\);

\rtl~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \rtl~2_combout\ = \SW_SUB|auto_generated|result_int[2]~4_combout\ & (!\Deco|Set_Flattop_DFF~regout\ # !\St1|SM_Set_Flattop~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Set_Flattop~regout\,
	datac => \SW_SUB|auto_generated|result_int[2]~4_combout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \rtl~2_combout\);

\rtl~4\ : cycloneii_lcell_comb
-- Equation(s):
-- \rtl~4_combout\ = \SW_SUB|auto_generated|result_int[0]~0_combout\ & (!\St1|SM_Set_Flattop~regout\ # !\Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0111011100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[0]~0_combout\,
	combout => \rtl~4_combout\);

\Deco|Soft_Trig_ff~4\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Soft_Trig_ff~4_combout\ = \Deco|_~84_combout\ & \SWEEP_SEL~combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100000011000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|_~84_combout\,
	datac => \SWEEP_SEL~combout\,
	combout => \Deco|Soft_Trig_ff~4_combout\);

\St1|SM_Wr_Delta~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_Delta~1_combout\ = \Deco|Ld_Delta_DFF~regout\ & \St1|SM_Idle~regout\ # !\Deco|Ld_Delta_DFF~regout\ & (\Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Idle~regout\,
	datac => \Deco|Set_Flattop_DFF~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|SM_Wr_Delta~1_combout\);

\St1|Delta_Not_Zero~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delta_Not_Zero~3_combout\ = !\Delta_Reg|dffs\(1) & !\Delta_Reg|dffs\(0) & !\Delta_Reg|dffs\(2) & !\Delta_Reg|dffs\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000001",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(1),
	datab => \Delta_Reg|dffs\(0),
	datac => \Delta_Reg|dffs\(2),
	datad => \Delta_Reg|dffs\(3),
	combout => \St1|Delta_Not_Zero~3_combout\);

\FKT[7]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(7),
	combout => \FKT~combout\(7));

\D[2]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(2),
	combout => \D~combout\(2));

\Delta_Reg|dffs[10]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[10]~feeder_combout\ = \D~combout\(10)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010101010101010",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \D~combout\(10),
	combout => \Delta_Reg|dffs[10]~feeder_combout\);

\Flat_Reg|dffs[9]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[9]~feeder_combout\ = \D~combout\(9)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(9),
	combout => \Flat_Reg|dffs[9]~feeder_combout\);

\Flat_Reg|dffs[7]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[7]~feeder_combout\ = \D~combout\(7)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(7),
	combout => \Flat_Reg|dffs[7]~feeder_combout\);

\Delta_Reg|dffs[6]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[6]~feeder_combout\ = \D~combout\(6)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(6),
	combout => \Delta_Reg|dffs[6]~feeder_combout\);

\Flat_Reg|dffs[5]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[5]~feeder_combout\ = \D~combout\(5)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(5),
	combout => \Flat_Reg|dffs[5]~feeder_combout\);

\Flat_Reg|dffs[3]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[3]~feeder_combout\ = \D~combout\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(3),
	combout => \Flat_Reg|dffs[3]~feeder_combout\);

\Delta_Reg|dffs[2]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[2]~feeder_combout\ = \D~combout\(2)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(2),
	combout => \Delta_Reg|dffs[2]~feeder_combout\);

\Flat_Reg|dffs[0]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[0]~feeder_combout\ = \D~combout\(0)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(0),
	combout => \Flat_Reg|dffs[0]~feeder_combout\);

\CLK~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_CLK,
	combout => \CLK~combout\);

\CLK~clkctrl\ : cycloneii_clkctrl
-- pragma translate_off
GENERIC MAP (
	clock_type => "global clock",
	ena_register_mode => "falling edge")
-- pragma translate_on
PORT MAP (
	inclk => \CLK~clkctrl_INCLK_bus\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	outclk => \CLK~clkctrl_outclk\);

\FKT[6]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(6),
	combout => \FKT~combout\(6));

\TAKE_DA~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_TAKE_DA,
	combout => \TAKE_DA~combout\);

\FKT[4]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(4),
	combout => \FKT~combout\(4));

\Deco|STAT1_DFF~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|STAT1_DFF~2_combout\ = \FKT~combout\(7) & !\FKT~combout\(6) & \TAKE_DA~combout\ & \FKT~combout\(4)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0010000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(7),
	datab => \FKT~combout\(6),
	datac => \TAKE_DA~combout\,
	datad => \FKT~combout\(4),
	combout => \Deco|STAT1_DFF~2_combout\);

\FKT[3]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(3),
	combout => \FKT~combout\(3));

\FKT[0]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(0),
	combout => \FKT~combout\(0));

\FKT[2]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(2),
	combout => \FKT~combout\(2));

\FKT[1]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(1),
	combout => \FKT~combout\(1));

\Deco|_~83\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|_~83_combout\ = !\FKT~combout\(5) & \FKT~combout\(0) & !\FKT~combout\(2) & !\FKT~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(5),
	datab => \FKT~combout\(0),
	datac => \FKT~combout\(2),
	datad => \FKT~combout\(1),
	combout => \Deco|_~83_combout\);

\Deco|STAT1_DFF~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|STAT1_DFF~3_combout\ = \SWEEP_SEL~combout\ & \Deco|STAT1_DFF~2_combout\ & !\FKT~combout\(3) & \Deco|_~83_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000100000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SWEEP_SEL~combout\,
	datab => \Deco|STAT1_DFF~2_combout\,
	datac => \FKT~combout\(3),
	datad => \Deco|_~83_combout\,
	combout => \Deco|STAT1_DFF~3_combout\);

\Deco|_~84\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|_~84_combout\ = !\FKT~combout\(7) & !\FKT~combout\(6) & \TAKE_DA~combout\ & !\FKT~combout\(4)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000010000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(7),
	datab => \FKT~combout\(6),
	datac => \TAKE_DA~combout\,
	datad => \FKT~combout\(4),
	combout => \Deco|_~84_combout\);

\PU_RESET~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_PU_RESET,
	combout => \PU_RESET~combout\);

\SWEEP_SEL~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_SWEEP_SEL,
	combout => \SWEEP_SEL~combout\);

\Deco|RES_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|RES_DFF~1_combout\ = \PU_RESET~combout\ # !\SWEEP_SEL~combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100111111001111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \PU_RESET~combout\,
	datac => \SWEEP_SEL~combout\,
	combout => \Deco|RES_DFF~1_combout\);

\Deco|RES_DFF~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|RES_DFF~0_combout\ = \Deco|RES_DFF~1_combout\ # \Deco|_~83_combout\ & \Deco|_~84_combout\ & !\FKT~combout\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|_~83_combout\,
	datab => \Deco|_~84_combout\,
	datac => \FKT~combout\(3),
	datad => \Deco|RES_DFF~1_combout\,
	combout => \Deco|RES_DFF~0_combout\);

\Deco|RES_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|RES_DFF~0_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|RES_DFF~regout\);

\Deco|RES_DFF~clkctrl\ : cycloneii_clkctrl
-- pragma translate_off
GENERIC MAP (
	clock_type => "global clock",
	ena_register_mode => "falling edge")
-- pragma translate_on
PORT MAP (
	inclk => \Deco|RES_DFF~clkctrl_INCLK_bus\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	outclk => \Deco|RES_DFF~clkctrl_outclk\);

\Deco|STAT1_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|STAT1_DFF~3_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|STAT1_DFF~regout\);

\D[15]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(15),
	combout => \D~combout\(15));

\Flat_Reg|dffs[15]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[15]~feeder_combout\ = \D~combout\(15)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(15),
	combout => \Flat_Reg|dffs[15]~feeder_combout\);

\FKT[5]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_FKT(5),
	combout => \FKT~combout\(5));

\Deco|Ld_Delay_DFF~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ld_Delay_DFF~2_combout\ = \FKT~combout\(1) & \FKT~combout\(2) & !\FKT~combout\(5)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000010100000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(1),
	datac => \FKT~combout\(2),
	datad => \FKT~combout\(5),
	combout => \Deco|Ld_Delay_DFF~2_combout\);

\Deco|Ld_Delay_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ld_Delay_DFF~1_combout\ = \Deco|Soft_Trig_ff~4_combout\ & \Deco|Ld_Delay_DFF~2_combout\ & \FKT~combout\(0) & !\FKT~combout\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000010000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Soft_Trig_ff~4_combout\,
	datab => \Deco|Ld_Delay_DFF~2_combout\,
	datac => \FKT~combout\(0),
	datad => \FKT~combout\(3),
	combout => \Deco|Ld_Delay_DFF~1_combout\);

\Deco|Ld_Delay_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Ld_Delay_DFF~1_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Ld_Delay_DFF~regout\);

\Deco|Ld_Delta_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ld_Delta_DFF~1_combout\ = \Deco|Soft_Trig_ff~4_combout\ & \Deco|Ld_Delay_DFF~2_combout\ & !\FKT~combout\(0) & !\FKT~combout\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Soft_Trig_ff~4_combout\,
	datab => \Deco|Ld_Delay_DFF~2_combout\,
	datac => \FKT~combout\(0),
	datad => \FKT~combout\(3),
	combout => \Deco|Ld_Delta_DFF~1_combout\);

\Deco|Ld_Delta_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Ld_Delta_DFF~1_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Ld_Delta_DFF~regout\);

\Deco|Ld_Flattop_Int_DFF~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ld_Flattop_Int_DFF~2_combout\ = !\FKT~combout\(5) & !\FKT~combout\(0) & !\FKT~combout\(2) & !\FKT~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000001",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(5),
	datab => \FKT~combout\(0),
	datac => \FKT~combout\(2),
	datad => \FKT~combout\(1),
	combout => \Deco|Ld_Flattop_Int_DFF~2_combout\);

\Deco|Ld_Flattop_Int_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ld_Flattop_Int_DFF~1_combout\ = \SWEEP_SEL~combout\ & \Deco|_~84_combout\ & \FKT~combout\(3) & \Deco|Ld_Flattop_Int_DFF~2_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1000000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SWEEP_SEL~combout\,
	datab => \Deco|_~84_combout\,
	datac => \FKT~combout\(3),
	datad => \Deco|Ld_Flattop_Int_DFF~2_combout\,
	combout => \Deco|Ld_Flattop_Int_DFF~1_combout\);

\Deco|Ld_Flattop_Int_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Ld_Flattop_Int_DFF~1_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Ld_Flattop_Int_DFF~regout\);

\Deco|Set_Flattop_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Set_Flattop_DFF~1_combout\ = \SWEEP_SEL~combout\ & \Deco|_~84_combout\ & \FKT~combout\(3) & \Deco|_~83_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1000000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SWEEP_SEL~combout\,
	datab => \Deco|_~84_combout\,
	datac => \FKT~combout\(3),
	datad => \Deco|_~83_combout\,
	combout => \Deco|Set_Flattop_DFF~1_combout\);

\Deco|Set_Flattop_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Set_Flattop_DFF~1_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Set_Flattop_DFF~regout\);

\St1|SM_Seq_Err~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Seq_Err~2_combout\ = \St1|SM_Wr_Delta~regout\ & !\Deco|Ld_Delay_DFF~regout\ & (\Deco|Ld_Flattop_Int_DFF~regout\ # \Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000101000001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Wr_Delta~regout\,
	datab => \Deco|Ld_Flattop_Int_DFF~regout\,
	datac => \Deco|Ld_Delay_DFF~regout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \St1|SM_Seq_Err~2_combout\);

\St1|SM_Seq_Err~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Seq_Err~3_combout\ = \St1|SM_Seq_Err~2_combout\ # \St1|_~92_combout\ & !\St1|SM_Idle~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011111100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|_~92_combout\,
	datac => \St1|SM_Seq_Err~2_combout\,
	datad => \St1|SM_Idle~regout\,
	combout => \St1|SM_Seq_Err~3_combout\);

\St1|_~90\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~90_combout\ = !\St1|Seq_Err_FF~regout\ & (\Deco|Ld_Delta_DFF~regout\ # \Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Delta_DFF~regout\,
	datac => \St1|Seq_Err_FF~regout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \St1|_~90_combout\);

\St1|_~96\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~96_combout\ = !\Deco|Ld_Flattop_Int_DFF~regout\ & !\Deco|Ld_Delay_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000001100000011",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Flattop_Int_DFF~regout\,
	datac => \Deco|Ld_Delay_DFF~regout\,
	combout => \St1|_~96_combout\);

\St1|SM_Wr_Delta~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_Delta~2_combout\ = \St1|SM_Wr_Delta~1_combout\ & (\St1|SM_Wr_Delta~regout\ & \Deco|Ld_Delta_DFF~regout\) # !\St1|SM_Wr_Delta~1_combout\ & (\Deco|Ld_Delta_DFF~regout\ # \St1|_~96_combout\ & \St1|SM_Wr_Delta~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111010101000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Wr_Delta~1_combout\,
	datab => \St1|_~96_combout\,
	datac => \St1|SM_Wr_Delta~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|SM_Wr_Delta~2_combout\);

\St1|SM_Wr_Delta\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Wr_Delta~2_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Wr_Delta~regout\);

\St1|SM_Wr_Delay~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_Delay~1_combout\ = \Deco|Ld_Delay_DFF~regout\ & (\St1|SM_Wr_Delay~regout\ # \St1|SM_Wr_Delta~regout\ & !\Deco|Ld_Delta_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1000100011001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Wr_Delay~regout\,
	datab => \Deco|Ld_Delay_DFF~regout\,
	datac => \St1|SM_Wr_Delta~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|SM_Wr_Delay~1_combout\);

\St1|SM_Wr_Delay~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_Delay~0_combout\ = \St1|SM_Wr_Delay~1_combout\ # !\St1|_~90_combout\ & \St1|SM_Wr_Delay~regout\ & \St1|_~96_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101110011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|_~90_combout\,
	datab => \St1|SM_Wr_Delay~1_combout\,
	datac => \St1|SM_Wr_Delay~regout\,
	datad => \St1|_~96_combout\,
	combout => \St1|SM_Wr_Delay~0_combout\);

\St1|SM_Wr_Delay\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Wr_Delay~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Wr_Delay~regout\);

\St1|_~91\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~91_combout\ = !\Deco|Ld_Delay_DFF~regout\ & \St1|_~90_combout\ & \St1|SM_Wr_Delay~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Delay_DFF~regout\,
	datac => \St1|_~90_combout\,
	datad => \St1|SM_Wr_Delay~regout\,
	combout => \St1|_~91_combout\);

\St1|_~95\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~95_combout\ = \Deco|Ld_Flattop_Int_DFF~regout\ & !\Deco|Ld_Delay_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000110000001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Flattop_Int_DFF~regout\,
	datac => \Deco|Ld_Delay_DFF~regout\,
	combout => \St1|_~95_combout\);

\St1|SM_Wr_FT_Int~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_FT_Int~1_combout\ = \St1|SM_Wr_FT_Int~regout\ & (\Deco|Ld_Flattop_Int_DFF~regout\ # !\St1|_~87_combout\ & !\Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100000011010000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|_~87_combout\,
	datab => \Deco|Ld_Flattop_Int_DFF~regout\,
	datac => \St1|SM_Wr_FT_Int~regout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \St1|SM_Wr_FT_Int~1_combout\);

\St1|SM_Wr_FT_Int~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Wr_FT_Int~0_combout\ = \St1|SM_Wr_FT_Int~1_combout\ # !\St1|_~90_combout\ & \St1|_~95_combout\ & \St1|SM_Wr_Delay~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111010011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|_~90_combout\,
	datab => \St1|_~95_combout\,
	datac => \St1|SM_Wr_FT_Int~1_combout\,
	datad => \St1|SM_Wr_Delay~regout\,
	combout => \St1|SM_Wr_FT_Int~0_combout\);

\St1|SM_Wr_FT_Int\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Wr_FT_Int~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Wr_FT_Int~regout\);

\St1|_~88\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~88_combout\ = \St1|SM_Wr_FT_Int~regout\ & !\Deco|Ld_Flattop_Int_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000110000001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Wr_FT_Int~regout\,
	datac => \Deco|Ld_Flattop_Int_DFF~regout\,
	combout => \St1|_~88_combout\);

\St1|_~87\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~87_combout\ = !\St1|Seq_Err_FF~regout\ & (\Deco|Ld_Delay_DFF~regout\ # \Deco|Ld_Delta_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011001100110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|Seq_Err_FF~regout\,
	datac => \Deco|Ld_Delay_DFF~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|_~87_combout\);

\St1|SM_Set_Flattop~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Set_Flattop~0_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ # \St1|_~88_combout\ & !\St1|_~87_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010000010101000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \St1|_~88_combout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \St1|_~87_combout\,
	combout => \St1|SM_Set_Flattop~0_combout\);

\St1|SM_Set_Flattop\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Set_Flattop~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Set_Flattop~regout\);

\St1|_~89\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~89_combout\ = \St1|SM_Set_Flattop~regout\ & !\Deco|Set_Flattop_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \St1|_~89_combout\);

\St1|SM_Seq_Err~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Seq_Err~1_combout\ = \St1|_~86_combout\ & (\St1|_~89_combout\ # \St1|_~87_combout\ & \St1|_~88_combout\) # !\St1|_~86_combout\ & (\St1|_~87_combout\ & \St1|_~88_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111100010001000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|_~86_combout\,
	datab => \St1|_~89_combout\,
	datac => \St1|_~87_combout\,
	datad => \St1|_~88_combout\,
	combout => \St1|SM_Seq_Err~1_combout\);

\St1|SM_Seq_Err~4\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Seq_Err~4_combout\ = \St1|_~91_combout\ # \St1|SM_Seq_Err~1_combout\ # !\Deco|Ld_Delta_DFF~regout\ & \St1|SM_Seq_Err~3_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111111110100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Ld_Delta_DFF~regout\,
	datab => \St1|SM_Seq_Err~3_combout\,
	datac => \St1|_~91_combout\,
	datad => \St1|SM_Seq_Err~1_combout\,
	combout => \St1|SM_Seq_Err~4_combout\);

\St1|SM_Seq_Err\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Seq_Err~4_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Seq_Err~regout\);

\St1|Seq_Err_FF~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Seq_Err_FF~3_combout\ = \St1|Seq_Err_FF~regout\ & (!\Deco|Ld_Delta_DFF~regout\ # !\St1|SM_Wr_Delta~regout\) # !\St1|Seq_Err_FF~regout\ & (\St1|SM_Seq_Err~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0111111101110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Wr_Delta~regout\,
	datab => \Deco|Ld_Delta_DFF~regout\,
	datac => \St1|Seq_Err_FF~regout\,
	datad => \St1|SM_Seq_Err~regout\,
	combout => \St1|Seq_Err_FF~3_combout\);

\St1|Seq_Err_FF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Seq_Err_FF~3_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Seq_Err_FF~regout\);

\St1|_~92\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~92_combout\ = !\St1|Seq_Err_FF~regout\ & (\Deco|Ld_Delay_DFF~regout\ # \Deco|Ld_Flattop_Int_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Delay_DFF~regout\,
	datac => \St1|Seq_Err_FF~regout\,
	datad => \Deco|Ld_Flattop_Int_DFF~regout\,
	combout => \St1|_~92_combout\);

\St1|SM_Idle~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Idle~2_combout\ = !\St1|SM_Idle~1_combout\ & (\St1|_~92_combout\ # \St1|SM_Idle~regout\ # \Deco|Ld_Delta_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101010101010100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Idle~1_combout\,
	datab => \St1|_~92_combout\,
	datac => \St1|SM_Idle~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|SM_Idle~2_combout\);

\St1|SM_Idle\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Idle~2_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Idle~regout\);

\D[12]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(12),
	combout => \D~combout\(12));

\Flat_Reg|dffs[12]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[12]~feeder_combout\ = \D~combout\(12)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(12),
	combout => \Flat_Reg|dffs[12]~feeder_combout\);

\St1|Wr_FT_Int\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Wr_FT_Int~combout\ = \St1|SM_Wr_FT_Int~regout\ & \Deco|Ld_Flattop_Int_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100000011000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Wr_FT_Int~regout\,
	datac => \Deco|Ld_Flattop_Int_DFF~regout\,
	combout => \St1|Wr_FT_Int~combout\);

\Flat_Reg|dffs[12]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[12]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(12));

\D[11]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(11),
	combout => \D~combout\(11));

\Flat_Reg|dffs[11]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[11]~feeder_combout\ = \D~combout\(11)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(11),
	combout => \Flat_Reg|dffs[11]~feeder_combout\);

\Flat_Reg|dffs[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[11]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(11));

\D[8]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(8),
	combout => \D~combout\(8));

\Flat_Reg|dffs[8]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[8]~feeder_combout\ = \D~combout\(8)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(8),
	combout => \Flat_Reg|dffs[8]~feeder_combout\);

\Flat_Reg|dffs[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[8]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(8));

\Delta_Reg|dffs[11]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[11]~feeder_combout\ = \D~combout\(11)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(11),
	combout => \Delta_Reg|dffs[11]~feeder_combout\);

\~GND\ : cycloneii_lcell_comb
-- Equation(s):
-- \~GND~combout\ = GND

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	combout => \~GND~combout\);

\Deco|Soft_Trig_ff~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Soft_Trig_ff~2_combout\ = !\FKT~combout\(2) & \FKT~combout\(5)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \FKT~combout\(2),
	datad => \FKT~combout\(5),
	combout => \Deco|Soft_Trig_ff~2_combout\);

\Deco|Soft_Trig_ff~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Soft_Trig_ff~3_combout\ = \SWEEP_SEL~combout\ & !\FKT~combout\(3) & \Deco|_~84_combout\ & \Deco|Soft_Trig_ff~2_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0010000000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \SWEEP_SEL~combout\,
	datab => \FKT~combout\(3),
	datac => \Deco|_~84_combout\,
	datad => \Deco|Soft_Trig_ff~2_combout\,
	combout => \Deco|Soft_Trig_ff~3_combout\);

\Deco|Stop_DFF~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Stop_DFF~1_combout\ = \FKT~combout\(0) & \Deco|Soft_Trig_ff~3_combout\ & \FKT~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1000100000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(0),
	datab => \Deco|Soft_Trig_ff~3_combout\,
	datad => \FKT~combout\(1),
	combout => \Deco|Stop_DFF~1_combout\);

\Deco|Stop_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Stop_DFF~1_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Stop_DFF~regout\);

\St1|TO_Timer|auto_generated|counter_comb_bita1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita1~combout\ = \St1|TO_Timer|auto_generated|safe_q\(1) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(1) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(1) # !\St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(1),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita0~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita1~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita1~COUT\);

\St1|_~86\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~86_combout\ = !\St1|Seq_Err_FF~regout\ & (\Deco|Ld_Flattop_Int_DFF~regout\ # \Deco|Ld_Delay_DFF~regout\ # \Deco|Ld_Delta_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100001110",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Ld_Flattop_Int_DFF~regout\,
	datab => \Deco|Ld_Delay_DFF~regout\,
	datac => \St1|Seq_Err_FF~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|_~86_combout\);

\D[9]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(9),
	combout => \D~combout\(9));

\Delta_Reg|dffs[9]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[9]~feeder_combout\ = \D~combout\(9)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \D~combout\(9),
	combout => \Delta_Reg|dffs[9]~feeder_combout\);

\/HW_Trig~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => \ww_/HW_Trig\,
	combout => \/HW_Trig~combout\);

\Deco|Soft_Trig_ff~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Soft_Trig_ff~0_combout\ = !\FKT~combout\(0) & \Deco|Soft_Trig_ff~3_combout\ & !\FKT~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000001000100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(0),
	datab => \Deco|Soft_Trig_ff~3_combout\,
	datad => \FKT~combout\(1),
	combout => \Deco|Soft_Trig_ff~0_combout\);

\Deco|Soft_Trig_ff\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Soft_Trig_ff~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Soft_Trig_ff~regout\);

\Deco|Ena_Soft_Trig_Dff~8\ : cycloneii_lcell_comb
-- Equation(s):
-- \Deco|Ena_Soft_Trig_Dff~8_combout\ = \FKT~combout\(0) & (\Deco|Ena_Soft_Trig_Dff~regout\ # \Deco|Soft_Trig_ff~3_combout\ & !\FKT~combout\(1)) # !\FKT~combout\(0) & \Deco|Ena_Soft_Trig_Dff~regout\ & (!\FKT~combout\(1) # !\Deco|Soft_Trig_ff~3_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011000011111000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \FKT~combout\(0),
	datab => \Deco|Soft_Trig_ff~3_combout\,
	datac => \Deco|Ena_Soft_Trig_Dff~regout\,
	datad => \FKT~combout\(1),
	combout => \Deco|Ena_Soft_Trig_Dff~8_combout\);

\Deco|Ena_Soft_Trig_Dff\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Deco|Ena_Soft_Trig_Dff~8_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Deco|Ena_Soft_Trig_Dff~regout\);

\St1|Trigger_Sync~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Trigger_Sync~1_combout\ = \Deco|Ena_Soft_Trig_Dff~regout\ & (\Deco|Soft_Trig_ff~regout\) # !\Deco|Ena_Soft_Trig_Dff~regout\ & !\/HW_Trig~combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000000110011",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \/HW_Trig~combout\,
	datac => \Deco|Soft_Trig_ff~regout\,
	datad => \Deco|Ena_Soft_Trig_Dff~regout\,
	combout => \St1|Trigger_Sync~1_combout\);

\St1|Trigger_Sync\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Trigger_Sync~1_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Trigger_Sync~regout\);

\St1|Trigger_Sync_1\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	sdata => \St1|Trigger_Sync~regout\,
	sload => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Trigger_Sync_1~regout\);

\St1|Trigger_FF~5\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Trigger_FF~5_combout\ = \St1|Trigger_Sync~regout\ & !\St1|Trigger_Sync_1~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000101000001010",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Trigger_Sync~regout\,
	datac => \St1|Trigger_Sync_1~regout\,
	combout => \St1|Trigger_FF~5_combout\);

\St1|Trigger_FF~6\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Trigger_FF~6_combout\ = \St1|Trigger_FF~regout\ & \St1|Wr_Delta~combout\ # !\St1|Trigger_FF~regout\ & (\St1|Trigger_FF~5_combout\ & \St1|SM_W_Start~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010110010100000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Wr_Delta~combout\,
	datab => \St1|Trigger_FF~5_combout\,
	datac => \St1|Trigger_FF~regout\,
	datad => \St1|SM_W_Start~regout\,
	combout => \St1|Trigger_FF~6_combout\);

\St1|Trigger_FF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Trigger_FF~6_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Trigger_FF~regout\);

\St1|Delay_Timer|auto_generated|counter_comb_bita1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita1~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(1) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(1) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(1) # !\St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(1),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita0~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita1~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita1~COUT\);

\D[1]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(1),
	combout => \D~combout\(1));

\St1|Wr_Delay\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Wr_Delay~combout\ = \Deco|Ld_Delay_DFF~regout\ & \St1|SM_Wr_Delay~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100000011000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Ld_Delay_DFF~regout\,
	datac => \St1|SM_Wr_Delay~regout\,
	combout => \St1|Wr_Delay~combout\);

\St1|Delay_Timer|auto_generated|_~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|_~2_combout\ = \Deco|Ld_Delay_DFF~regout\ & (\St1|SM_Wr_Delay~regout\ # \St1|SM_W_Start~regout\ & \St1|Trigger_FF~regout\) # !\Deco|Ld_Delay_DFF~regout\ & \St1|SM_W_Start~regout\ & \St1|Trigger_FF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1110101011000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Ld_Delay_DFF~regout\,
	datab => \St1|SM_W_Start~regout\,
	datac => \St1|Trigger_FF~regout\,
	datad => \St1|SM_Wr_Delay~regout\,
	combout => \St1|Delay_Timer|auto_generated|_~2_combout\);

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita1~combout\,
	sdata => \D~combout\(1),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(1));

\St1|Delay_Timer|auto_generated|counter_comb_bita4\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita4~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(4) & \St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\ & VCC # !\St1|Delay_Timer|auto_generated|safe_q\(4) & 
-- !\St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\
-- \St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(4) & !\St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(4),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita3~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita4~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita4~COUT\);

\D[4]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(4),
	combout => \D~combout\(4));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita4~combout\,
	sdata => \D~combout\(4),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(4));

\St1|Delay_Timer|auto_generated|counter_comb_bita6\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita6~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(6) & \St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\ & VCC # !\St1|Delay_Timer|auto_generated|safe_q\(6) & 
-- !\St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\
-- \St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(6) & !\St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(6),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita5~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita6~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita6~COUT\);

\D[6]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(6),
	combout => \D~combout\(6));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita6~combout\,
	sdata => \D~combout\(6),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(6));

\St1|Delay_Timer|auto_generated|counter_comb_bita8\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita8~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(8) & \St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\ & VCC # !\St1|Delay_Timer|auto_generated|safe_q\(8) & 
-- !\St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\
-- \St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(8) & !\St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(8),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita7~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita8~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita8~COUT\);

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita8~combout\,
	sdata => \D~combout\(8),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(8));

\St1|Delay_Timer|auto_generated|counter_comb_bita10\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita10~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(10) & \St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\ & VCC # !\St1|Delay_Timer|auto_generated|safe_q\(10) & 
-- !\St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\
-- \St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\ = CARRY(!\St1|Delay_Timer|auto_generated|safe_q\(10) & !\St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(10),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita9~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita10~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\);

\D[10]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(10),
	combout => \D~combout\(10));

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita10~combout\,
	sdata => \D~combout\(10),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(10));

\St1|Delay_Timer|auto_generated|counter_comb_bita11\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita11~combout\ = \St1|Delay_Timer|auto_generated|safe_q\(11) & (GND # !\St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\) # !\St1|Delay_Timer|auto_generated|safe_q\(11) & 
-- (\St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\ $ GND)
-- \St1|Delay_Timer|auto_generated|counter_comb_bita11~COUT\ = CARRY(\St1|Delay_Timer|auto_generated|safe_q\(11) # !\St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delay_Timer|auto_generated|safe_q\(11),
	datad => VCC,
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita10~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita11~combout\,
	cout => \St1|Delay_Timer|auto_generated|counter_comb_bita11~COUT\);

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita11~combout\,
	sdata => \D~combout\(11),
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(11));

\St1|Delay_Timer|auto_generated|counter_comb_bita12\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Timer|auto_generated|counter_comb_bita12~combout\ = \St1|Delay_Timer|auto_generated|counter_comb_bita11~COUT\ $ !\St1|Delay_Timer|auto_generated|safe_q\(12)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000000001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datad => \St1|Delay_Timer|auto_generated|safe_q\(12),
	cin => \St1|Delay_Timer|auto_generated|counter_comb_bita11~COUT\,
	combout => \St1|Delay_Timer|auto_generated|counter_comb_bita12~combout\);

\St1|Delay_Timer|auto_generated|counter_reg_bit1a[12]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Timer|auto_generated|counter_comb_bita12~combout\,
	sdata => \~GND~combout\,
	sload => \St1|Wr_Delay~combout\,
	ena => \St1|Delay_Timer|auto_generated|_~2_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Timer|auto_generated|safe_q\(12));

\St1|Delay_Fin~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delay_Fin~0_combout\ = \St1|Trigger_FF~regout\ & \St1|Delay_Timer|auto_generated|safe_q\(12)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|Trigger_FF~regout\,
	datad => \St1|Delay_Timer|auto_generated|safe_q\(12),
	combout => \St1|Delay_Fin~0_combout\);

\St1|Delay_Fin\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delay_Fin~0_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delay_Fin~regout\);

\St1|S_Stop_Delta~5\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|S_Stop_Delta~5_combout\ = \St1|SM_W_Start~regout\ & (!\St1|Delay_Fin~regout\ # !\St1|Delta_Not_Zero~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delta_Not_Zero~regout\,
	datac => \St1|SM_W_Start~regout\,
	datad => \St1|Delay_Fin~regout\,
	combout => \St1|S_Stop_Delta~5_combout\);

\114\ : cycloneii_lcell_comb
-- Equation(s):
-- \114~combout\ = \St1|S_Stop_Delta~5_combout\ & (\Deco|Stop_DFF~regout\ # \St1|Timeout~regout\) # !\St1|Wr_Delta~combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111110101010101",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Wr_Delta~combout\,
	datab => \Deco|Stop_DFF~regout\,
	datac => \St1|Timeout~regout\,
	datad => \St1|S_Stop_Delta~5_combout\,
	combout => \114~combout\);

\Delta_Reg|dffs[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[9]~feeder_combout\,
	sdata => \~GND~combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(9));

\Delta_Reg|dffs[8]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[8]~feeder_combout\ = \D~combout\(8)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010101010101010",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \D~combout\(8),
	combout => \Delta_Reg|dffs[8]~feeder_combout\);

\Delta_Reg|dffs[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[8]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(8));

\St1|Delta_Not_Zero~1\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delta_Not_Zero~1_combout\ = !\Delta_Reg|dffs\(10) & !\Delta_Reg|dffs\(9) & !\Delta_Reg|dffs\(8) & !\Delta_Reg|dffs\(11)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000001",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(10),
	datab => \Delta_Reg|dffs\(9),
	datac => \Delta_Reg|dffs\(8),
	datad => \Delta_Reg|dffs\(11),
	combout => \St1|Delta_Not_Zero~1_combout\);

\D[5]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(5),
	combout => \D~combout\(5));

\Delta_Reg|dffs[5]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[5]~feeder_combout\ = \D~combout\(5)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(5),
	combout => \Delta_Reg|dffs[5]~feeder_combout\);

\Delta_Reg|dffs[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[5]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(5));

\Delta_Reg|dffs[4]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[4]~feeder_combout\ = \D~combout\(4)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \D~combout\(4),
	combout => \Delta_Reg|dffs[4]~feeder_combout\);

\Delta_Reg|dffs[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[4]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(4));

\D[7]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(7),
	combout => \D~combout\(7));

\Delta_Reg|dffs[7]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[7]~feeder_combout\ = \D~combout\(7)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(7),
	combout => \Delta_Reg|dffs[7]~feeder_combout\);

\Delta_Reg|dffs[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[7]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(7));

\St1|Delta_Not_Zero~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delta_Not_Zero~2_combout\ = !\Delta_Reg|dffs\(6) & !\Delta_Reg|dffs\(5) & !\Delta_Reg|dffs\(4) & !\Delta_Reg|dffs\(7)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000000001",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(6),
	datab => \Delta_Reg|dffs\(5),
	datac => \Delta_Reg|dffs\(4),
	datad => \Delta_Reg|dffs\(7),
	combout => \St1|Delta_Not_Zero~2_combout\);

\St1|Delta_Not_Zero~4\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Delta_Not_Zero~4_combout\ = !\St1|Delta_Not_Zero~2_combout\ # !\St1|Delta_Not_Zero~1_combout\ # !\St1|Delta_Not_Zero~3_combout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101111111111111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Delta_Not_Zero~3_combout\,
	datac => \St1|Delta_Not_Zero~1_combout\,
	datad => \St1|Delta_Not_Zero~2_combout\,
	combout => \St1|Delta_Not_Zero~4_combout\);

\St1|Delta_Not_Zero\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Delta_Not_Zero~4_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Delta_Not_Zero~regout\);

\St1|_~93\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~93_combout\ = !\Deco|Stop_DFF~regout\ & \St1|SM_W_Start~regout\ & (!\St1|Delay_Fin~regout\ # !\St1|Delta_Not_Zero~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0001000001010000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Stop_DFF~regout\,
	datab => \St1|Delta_Not_Zero~regout\,
	datac => \St1|SM_W_Start~regout\,
	datad => \St1|Delay_Fin~regout\,
	combout => \St1|_~93_combout\);

\St1|SM_W_Start~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_W_Start~0_combout\ = \St1|Timeout~regout\ & \St1|_~89_combout\ & !\St1|_~86_combout\ # !\St1|Timeout~regout\ & (\St1|_~93_combout\ # \St1|_~89_combout\ & !\St1|_~86_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101110100001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Timeout~regout\,
	datab => \St1|_~89_combout\,
	datac => \St1|_~86_combout\,
	datad => \St1|_~93_combout\,
	combout => \St1|SM_W_Start~0_combout\);

\St1|SM_W_Start\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_W_Start~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_W_Start~regout\);

\St1|TO_Timer|auto_generated|_~57\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|_~57_combout\ = \St1|SM_Set_Flattop~regout\ # \St1|SM_Wr_Delta~regout\ # \St1|SM_W_Start~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111111111100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Set_Flattop~regout\,
	datac => \St1|SM_Wr_Delta~regout\,
	datad => \St1|SM_W_Start~regout\,
	combout => \St1|TO_Timer|auto_generated|_~57_combout\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita1~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(1));

\St1|TO_Timer|auto_generated|counter_comb_bita3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita3~combout\ = \St1|TO_Timer|auto_generated|safe_q\(3) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(3) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(3) # !\St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(3),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita2~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita3~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita3~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita3~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(3));

\St1|TO_Timer|auto_generated|counter_comb_bita5\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita5~combout\ = \St1|TO_Timer|auto_generated|safe_q\(5) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(5) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(5) # !\St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(5),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita4~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita5~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita5~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(5));

\St1|TO_Timer|auto_generated|counter_comb_bita6\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita6~combout\ = \St1|TO_Timer|auto_generated|safe_q\(6) & \St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(6) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(6) & !\St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(6),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita5~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita6~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita6~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(6));

\St1|TO_Timer|auto_generated|counter_comb_bita7\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita7~combout\ = \St1|TO_Timer|auto_generated|safe_q\(7) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(7) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(7) # !\St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(7),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita6~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita7~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita7~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(7));

\St1|TO_Timer|auto_generated|counter_comb_bita8\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita8~combout\ = \St1|TO_Timer|auto_generated|safe_q\(8) & \St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(8) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(8) & !\St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(8),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita7~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita8~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita8~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(8));

\St1|TO_Timer|auto_generated|counter_comb_bita9\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita9~combout\ = \St1|TO_Timer|auto_generated|safe_q\(9) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(9) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(9) # !\St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(9),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita8~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita9~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita9~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(9));

\St1|TO_Timer|auto_generated|counter_comb_bita10\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita10~combout\ = \St1|TO_Timer|auto_generated|safe_q\(10) & \St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(10) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(10) & !\St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(10),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita9~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita10~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita10~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita10~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(10));

\St1|TO_Timer|auto_generated|counter_comb_bita12\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita12~combout\ = \St1|TO_Timer|auto_generated|safe_q\(12) & \St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\ & VCC # !\St1|TO_Timer|auto_generated|safe_q\(12) & 
-- !\St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\
-- \St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\ = CARRY(!\St1|TO_Timer|auto_generated|safe_q\(12) & !\St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(12),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita11~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita12~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita12~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[12]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita12~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(12));

\St1|TO_Timer|auto_generated|counter_comb_bita15\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita15~combout\ = \St1|TO_Timer|auto_generated|safe_q\(15) & (GND # !\St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\) # !\St1|TO_Timer|auto_generated|safe_q\(15) & 
-- (\St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\ $ GND)
-- \St1|TO_Timer|auto_generated|counter_comb_bita15~COUT\ = CARRY(\St1|TO_Timer|auto_generated|safe_q\(15) # !\St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \St1|TO_Timer|auto_generated|safe_q\(15),
	datad => VCC,
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita14~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita15~combout\,
	cout => \St1|TO_Timer|auto_generated|counter_comb_bita15~COUT\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[15]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita15~combout\,
	sdata => \St1|SM_Wr_Delta~regout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(15));

\St1|TO_Timer|auto_generated|counter_comb_bita16\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Timer|auto_generated|counter_comb_bita16~combout\ = \St1|TO_Timer|auto_generated|safe_q\(16) $ !\St1|TO_Timer|auto_generated|counter_comb_bita15~COUT\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010110100101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \St1|TO_Timer|auto_generated|safe_q\(16),
	cin => \St1|TO_Timer|auto_generated|counter_comb_bita15~COUT\,
	combout => \St1|TO_Timer|auto_generated|counter_comb_bita16~combout\);

\St1|TO_Timer|auto_generated|counter_reg_bit1a[16]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Timer|auto_generated|counter_comb_bita16~combout\,
	sdata => \~GND~combout\,
	sload => \St1|SM_Wr_Delta~regout\,
	ena => \St1|TO_Timer|auto_generated|_~57_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Timer|auto_generated|safe_q\(16));

\St1|Timeout\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	sdata => \St1|TO_Timer|auto_generated|safe_q\(16),
	sload => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Timeout~regout\);

\St1|S_Stop_Delta~6\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|S_Stop_Delta~6_combout\ = \St1|S_Stop_Delta~5_combout\ & (\Deco|Stop_DFF~regout\ # \St1|Timeout~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111110000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \Deco|Stop_DFF~regout\,
	datac => \St1|Timeout~regout\,
	datad => \St1|S_Stop_Delta~5_combout\,
	combout => \St1|S_Stop_Delta~6_combout\);

\Delta_Reg|dffs[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[11]~feeder_combout\,
	sdata => \~GND~combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(11));

\D[3]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(3),
	combout => \D~combout\(3));

\Delta_Reg|dffs[3]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[3]~feeder_combout\ = \D~combout\(3)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(3),
	combout => \Delta_Reg|dffs[3]~feeder_combout\);

\Delta_Reg|dffs[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[3]~feeder_combout\,
	sdata => \~GND~combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(3));

\Delta_Reg|dffs[1]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[1]~feeder_combout\ = \D~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \D~combout\(1),
	combout => \Delta_Reg|dffs[1]~feeder_combout\);

\Delta_Reg|dffs[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[1]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(1));

\D[0]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(0),
	combout => \D~combout\(0));

\Delta_Reg|dffs[0]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Delta_Reg|dffs[0]~feeder_combout\ = \D~combout\(0)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \D~combout\(0),
	combout => \Delta_Reg|dffs[0]~feeder_combout\);

\Delta_Reg|dffs[0]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Delta_Reg|dffs[0]~feeder_combout\,
	sdata => \St1|S_Stop_Delta~6_combout\,
	sload => \St1|S_Stop_Delta~6_combout\,
	ena => \114~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Delta_Reg|dffs\(0));

\SUB_REG|dffs[2]~23\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[2]~23_combout\ = (\Delta_Reg|dffs\(2) $ \SUB_REG|dffs\(2) $ !\SUB_REG|dffs[1]~22\) # GND
-- \SUB_REG|dffs[2]~24\ = CARRY(\Delta_Reg|dffs\(2) & (\SUB_REG|dffs\(2) # !\SUB_REG|dffs[1]~22\) # !\Delta_Reg|dffs\(2) & \SUB_REG|dffs\(2) & !\SUB_REG|dffs[1]~22\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100110001110",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(2),
	datab => \SUB_REG|dffs\(2),
	datad => VCC,
	cin => \SUB_REG|dffs[1]~22\,
	combout => \SUB_REG|dffs[2]~23_combout\,
	cout => \SUB_REG|dffs[2]~24\);

\ADD_C~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \ADD_C~0_combout\ = !\ADD_C~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100001111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \ADD_C~regout\,
	combout => \ADD_C~0_combout\);

\St1|_~85\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~85_combout\ = \St1|SM_Stop~regout\ & !\R_Fin~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000011001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Stop~regout\,
	datad => \R_Fin~regout\,
	combout => \St1|_~85_combout\);

\St1|SM_Stop~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Stop~0_combout\ = \St1|_~85_combout\ # \St1|S_Stop_Delta~5_combout\ & (\St1|Timeout~regout\ # \Deco|Stop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111110101000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|S_Stop_Delta~5_combout\,
	datab => \St1|Timeout~regout\,
	datac => \Deco|Stop_DFF~regout\,
	datad => \St1|_~85_combout\,
	combout => \St1|SM_Stop~0_combout\);

\St1|SM_Stop\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Stop~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Stop~regout\);

\St1|Stop_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	sdata => \St1|SM_Stop~regout\,
	sload => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Stop_DFF~regout\);

\St1|_~94\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|_~94_combout\ = \St1|Delta_Not_Zero~regout\ & \St1|Delay_Fin~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100110000000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|Delta_Not_Zero~regout\,
	datad => \St1|Delay_Fin~regout\,
	combout => \St1|_~94_combout\);

\St1|SM_Work~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|SM_Work~0_combout\ = \R_Fin~regout\ & \St1|SM_W_Start~regout\ & (\St1|_~94_combout\) # !\R_Fin~regout\ & (\St1|SM_Work~regout\ # \St1|SM_W_Start~regout\ & \St1|_~94_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101110001010000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \R_Fin~regout\,
	datab => \St1|SM_W_Start~regout\,
	datac => \St1|SM_Work~regout\,
	datad => \St1|_~94_combout\,
	combout => \St1|SM_Work~0_combout\);

\St1|SM_Work\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|SM_Work~0_combout\,
	aclr => \Deco|RES_DFF~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|SM_Work~regout\);

\St1|Work_DFF~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Work_DFF~feeder_combout\ = \St1|SM_Work~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \St1|SM_Work~regout\,
	combout => \St1|Work_DFF~feeder_combout\);

\St1|Work_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Work_DFF~feeder_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Work_DFF~regout\);

\120\ : cycloneii_lcell_comb
-- Equation(s):
-- \120~combout\ = !\St1|Stop_DFF~regout\ & !\St1|Work_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000000000001111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \St1|Stop_DFF~regout\,
	datad => \St1|Work_DFF~regout\,
	combout => \120~combout\);

\120~clkctrl\ : cycloneii_clkctrl
-- pragma translate_off
GENERIC MAP (
	clock_type => "global clock",
	ena_register_mode => "falling edge")
-- pragma translate_on
PORT MAP (
	inclk => \120~clkctrl_INCLK_bus\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	outclk => \120~clkctrl_outclk\);

ADD_C : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \ADD_C~0_combout\,
	aclr => \120~clkctrl_outclk\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \ADD_C~regout\);

\RND_CNT|auto_generated|counter_comb_bita1\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita1~combout\ = \RND_CNT|auto_generated|pre_hazard\(1) & !\RND_CNT|auto_generated|counter_comb_bita0~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(1) & (\RND_CNT|auto_generated|counter_comb_bita0~COUT\ # GND)
-- \RND_CNT|auto_generated|counter_comb_bita1~COUT\ = CARRY(!\RND_CNT|auto_generated|counter_comb_bita0~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(1))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110000111111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \RND_CNT|auto_generated|pre_hazard\(1),
	datad => VCC,
	cin => \RND_CNT|auto_generated|counter_comb_bita0~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita1~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita1~COUT\);

\RND_CNT|auto_generated|counter_reg_bit1a[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita1~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(1));

\RND_CNT|auto_generated|counter_comb_bita3\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita3~combout\ = \RND_CNT|auto_generated|pre_hazard\(3) & !\RND_CNT|auto_generated|counter_comb_bita2~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(3) & (\RND_CNT|auto_generated|counter_comb_bita2~COUT\ # GND)
-- \RND_CNT|auto_generated|counter_comb_bita3~COUT\ = CARRY(!\RND_CNT|auto_generated|counter_comb_bita2~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(3))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110000111111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \RND_CNT|auto_generated|pre_hazard\(3),
	datad => VCC,
	cin => \RND_CNT|auto_generated|counter_comb_bita2~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita3~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita3~COUT\);

\RND_CNT|auto_generated|counter_reg_bit1a[3]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita3~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(3));

\RND_CNT|auto_generated|counter_comb_bita5\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita5~combout\ = \RND_CNT|auto_generated|pre_hazard\(5) & !\RND_CNT|auto_generated|counter_comb_bita4~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(5) & (\RND_CNT|auto_generated|counter_comb_bita4~COUT\ # GND)
-- \RND_CNT|auto_generated|counter_comb_bita5~COUT\ = CARRY(!\RND_CNT|auto_generated|counter_comb_bita4~COUT\ # !\RND_CNT|auto_generated|pre_hazard\(5))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110000111111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \RND_CNT|auto_generated|pre_hazard\(5),
	datad => VCC,
	cin => \RND_CNT|auto_generated|counter_comb_bita4~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita5~combout\,
	cout => \RND_CNT|auto_generated|counter_comb_bita5~COUT\);

\RND_CNT|auto_generated|counter_reg_bit1a[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita5~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(5));

\RND_CNT|auto_generated|counter_comb_bita6\ : cycloneii_lcell_comb
-- Equation(s):
-- \RND_CNT|auto_generated|counter_comb_bita6~combout\ = \RND_CNT|auto_generated|counter_comb_bita5~COUT\ $ !\RND_CNT|auto_generated|pre_hazard\(6)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000000001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datad => \RND_CNT|auto_generated|pre_hazard\(6),
	cin => \RND_CNT|auto_generated|counter_comb_bita5~COUT\,
	combout => \RND_CNT|auto_generated|counter_comb_bita6~combout\);

\RND_CNT|auto_generated|counter_reg_bit1a[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \RND_CNT|auto_generated|counter_comb_bita6~combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \RND_CNT|auto_generated|pre_hazard\(6));

\136\ : cycloneii_lcell_comb
-- Equation(s):
-- \136~combout\ = \ADD_C~regout\ & !\RND_CNT|auto_generated|pre_hazard\(6)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000110000001100",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \ADD_C~regout\,
	datac => \RND_CNT|auto_generated|pre_hazard\(6),
	combout => \136~combout\);

\SUB_REG|dffs[2]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[2]~23_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(2));

\SUB_REG|dffs[4]~27\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[4]~27_combout\ = (\Delta_Reg|dffs\(4) $ \SUB_REG|dffs\(4) $ !\SUB_REG|dffs[3]~26\) # GND
-- \SUB_REG|dffs[4]~28\ = CARRY(\Delta_Reg|dffs\(4) & (\SUB_REG|dffs\(4) # !\SUB_REG|dffs[3]~26\) # !\Delta_Reg|dffs\(4) & \SUB_REG|dffs\(4) & !\SUB_REG|dffs[3]~26\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100110001110",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(4),
	datab => \SUB_REG|dffs\(4),
	datad => VCC,
	cin => \SUB_REG|dffs[3]~26\,
	combout => \SUB_REG|dffs[4]~27_combout\,
	cout => \SUB_REG|dffs[4]~28\);

\SUB_REG|dffs[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[4]~27_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(4));

\SUB_REG|dffs[6]~31\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[6]~31_combout\ = (\Delta_Reg|dffs\(6) $ \SUB_REG|dffs\(6) $ !\SUB_REG|dffs[5]~30\) # GND
-- \SUB_REG|dffs[6]~32\ = CARRY(\Delta_Reg|dffs\(6) & (\SUB_REG|dffs\(6) # !\SUB_REG|dffs[5]~30\) # !\Delta_Reg|dffs\(6) & \SUB_REG|dffs\(6) & !\SUB_REG|dffs[5]~30\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100110001110",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(6),
	datab => \SUB_REG|dffs\(6),
	datad => VCC,
	cin => \SUB_REG|dffs[5]~30\,
	combout => \SUB_REG|dffs[6]~31_combout\,
	cout => \SUB_REG|dffs[6]~32\);

\SUB_REG|dffs[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[6]~31_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(6));

\SUB_REG|dffs[7]~33\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[7]~33_combout\ = \Delta_Reg|dffs\(7) & (\SUB_REG|dffs\(7) & \SUB_REG|dffs[6]~32\ & VCC # !\SUB_REG|dffs\(7) & !\SUB_REG|dffs[6]~32\) # !\Delta_Reg|dffs\(7) & (\SUB_REG|dffs\(7) & !\SUB_REG|dffs[6]~32\ # !\SUB_REG|dffs\(7) & 
-- (\SUB_REG|dffs[6]~32\ # GND))
-- \SUB_REG|dffs[7]~34\ = CARRY(\Delta_Reg|dffs\(7) & !\SUB_REG|dffs\(7) & !\SUB_REG|dffs[6]~32\ # !\Delta_Reg|dffs\(7) & (!\SUB_REG|dffs[6]~32\ # !\SUB_REG|dffs\(7)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(7),
	datab => \SUB_REG|dffs\(7),
	datad => VCC,
	cin => \SUB_REG|dffs[6]~32\,
	combout => \SUB_REG|dffs[7]~33_combout\,
	cout => \SUB_REG|dffs[7]~34\);

\SUB_REG|dffs[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[7]~33_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(7));

\SUB_REG|dffs[8]~35\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[8]~35_combout\ = (\Delta_Reg|dffs\(8) $ \SUB_REG|dffs\(8) $ !\SUB_REG|dffs[7]~34\) # GND
-- \SUB_REG|dffs[8]~36\ = CARRY(\Delta_Reg|dffs\(8) & (\SUB_REG|dffs\(8) # !\SUB_REG|dffs[7]~34\) # !\Delta_Reg|dffs\(8) & \SUB_REG|dffs\(8) & !\SUB_REG|dffs[7]~34\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100110001110",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(8),
	datab => \SUB_REG|dffs\(8),
	datad => VCC,
	cin => \SUB_REG|dffs[7]~34\,
	combout => \SUB_REG|dffs[8]~35_combout\,
	cout => \SUB_REG|dffs[8]~36\);

\SUB_REG|dffs[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[8]~35_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(8));

\SUB_REG|dffs[9]~37\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[9]~37_combout\ = \SUB_REG|dffs\(9) & (\Delta_Reg|dffs\(9) & \SUB_REG|dffs[8]~36\ & VCC # !\Delta_Reg|dffs\(9) & !\SUB_REG|dffs[8]~36\) # !\SUB_REG|dffs\(9) & (\Delta_Reg|dffs\(9) & !\SUB_REG|dffs[8]~36\ # !\Delta_Reg|dffs\(9) & 
-- (\SUB_REG|dffs[8]~36\ # GND))
-- \SUB_REG|dffs[9]~38\ = CARRY(\SUB_REG|dffs\(9) & !\Delta_Reg|dffs\(9) & !\SUB_REG|dffs[8]~36\ # !\SUB_REG|dffs\(9) & (!\SUB_REG|dffs[8]~36\ # !\Delta_Reg|dffs\(9)))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000010111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(9),
	datab => \Delta_Reg|dffs\(9),
	datad => VCC,
	cin => \SUB_REG|dffs[8]~36\,
	combout => \SUB_REG|dffs[9]~37_combout\,
	cout => \SUB_REG|dffs[9]~38\);

\SUB_REG|dffs[10]~39\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[10]~39_combout\ = (\Delta_Reg|dffs\(10) $ \SUB_REG|dffs\(10) $ !\SUB_REG|dffs[9]~38\) # GND
-- \SUB_REG|dffs[10]~40\ = CARRY(\Delta_Reg|dffs\(10) & (\SUB_REG|dffs\(10) # !\SUB_REG|dffs[9]~38\) # !\Delta_Reg|dffs\(10) & \SUB_REG|dffs\(10) & !\SUB_REG|dffs[9]~38\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100110001110",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \Delta_Reg|dffs\(10),
	datab => \SUB_REG|dffs\(10),
	datad => VCC,
	cin => \SUB_REG|dffs[9]~38\,
	combout => \SUB_REG|dffs[10]~39_combout\,
	cout => \SUB_REG|dffs[10]~40\);

\SUB_REG|dffs[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[10]~39_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(10));

\SUB_REG|dffs[13]~45\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[13]~45_combout\ = \SUB_REG|dffs\(13) & !\SUB_REG|dffs[12]~44\ # !\SUB_REG|dffs\(13) & (\SUB_REG|dffs[12]~44\ # GND)
-- \SUB_REG|dffs[13]~46\ = CARRY(!\SUB_REG|dffs[12]~44\ # !\SUB_REG|dffs\(13))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110000111111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SUB_REG|dffs\(13),
	datad => VCC,
	cin => \SUB_REG|dffs[12]~44\,
	combout => \SUB_REG|dffs[13]~45_combout\,
	cout => \SUB_REG|dffs[13]~46\);

\SUB_REG|dffs[13]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[13]~45_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(13));

\SUB_REG|dffs[14]~47\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[14]~47_combout\ = \SUB_REG|dffs\(14) & (\SUB_REG|dffs[13]~46\ $ GND) # !\SUB_REG|dffs\(14) & !\SUB_REG|dffs[13]~46\ & VCC
-- \SUB_REG|dffs[14]~48\ = CARRY(\SUB_REG|dffs\(14) & !\SUB_REG|dffs[13]~46\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100001010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(14),
	datad => VCC,
	cin => \SUB_REG|dffs[13]~46\,
	combout => \SUB_REG|dffs[14]~47_combout\,
	cout => \SUB_REG|dffs[14]~48\);

\SUB_REG|dffs[15]~49\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[15]~49_combout\ = \SUB_REG|dffs\(15) & !\SUB_REG|dffs[14]~48\ # !\SUB_REG|dffs\(15) & (\SUB_REG|dffs[14]~48\ # GND)
-- \SUB_REG|dffs[15]~50\ = CARRY(!\SUB_REG|dffs[14]~48\ # !\SUB_REG|dffs\(15))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101001011111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(15),
	datad => VCC,
	cin => \SUB_REG|dffs[14]~48\,
	combout => \SUB_REG|dffs[15]~49_combout\,
	cout => \SUB_REG|dffs[15]~50\);

\SUB_REG|dffs[16]~51\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[16]~51_combout\ = \SUB_REG|dffs\(16) & (\SUB_REG|dffs[15]~50\ $ GND) # !\SUB_REG|dffs\(16) & !\SUB_REG|dffs[15]~50\ & VCC
-- \SUB_REG|dffs[16]~52\ = CARRY(\SUB_REG|dffs\(16) & !\SUB_REG|dffs[15]~50\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100001100",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SUB_REG|dffs\(16),
	datad => VCC,
	cin => \SUB_REG|dffs[15]~50\,
	combout => \SUB_REG|dffs[16]~51_combout\,
	cout => \SUB_REG|dffs[16]~52\);

\SUB_REG|dffs[16]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[16]~51_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(16));

\SUB_REG|dffs[17]~53\ : cycloneii_lcell_comb
-- Equation(s):
-- \SUB_REG|dffs[17]~53_combout\ = \SUB_REG|dffs\(17) $ \SUB_REG|dffs[16]~52\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101001011010",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(17),
	cin => \SUB_REG|dffs[16]~52\,
	combout => \SUB_REG|dffs[17]~53_combout\);

\SUB_REG|dffs[17]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[17]~53_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(17));

\SUB_REG|dffs[15]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[15]~49_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(15));

\SUB_REG|dffs[14]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[14]~47_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(14));

\Flat_Reg|dffs[1]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[1]~feeder_combout\ = \D~combout\(1)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(1),
	combout => \Flat_Reg|dffs[1]~feeder_combout\);

\Flat_Reg|dffs[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[1]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(1));

\SUB_REG|dffs[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SUB_REG|dffs[9]~37_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \136~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_REG|dffs\(9));

\SW_SUB|auto_generated|result_int[1]~2\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[1]~2_combout\ = \SUB_REG|dffs\(7) & (\SW_Reg|dffs\(1) & !\SW_SUB|auto_generated|result_int[0]~1\ # !\SW_Reg|dffs\(1) & (\SW_SUB|auto_generated|result_int[0]~1\ # GND)) # !\SUB_REG|dffs\(7) & (\SW_Reg|dffs\(1) & 
-- \SW_SUB|auto_generated|result_int[0]~1\ & VCC # !\SW_Reg|dffs\(1) & !\SW_SUB|auto_generated|result_int[0]~1\)
-- \SW_SUB|auto_generated|result_int[1]~3\ = CARRY(\SUB_REG|dffs\(7) & (!\SW_SUB|auto_generated|result_int[0]~1\ # !\SW_Reg|dffs\(1)) # !\SUB_REG|dffs\(7) & !\SW_Reg|dffs\(1) & !\SW_SUB|auto_generated|result_int[0]~1\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100100101011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(7),
	datab => \SW_Reg|dffs\(1),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[0]~1\,
	combout => \SW_SUB|auto_generated|result_int[1]~2_combout\,
	cout => \SW_SUB|auto_generated|result_int[1]~3\);

\rtl~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \rtl~3_combout\ = \SW_SUB|auto_generated|result_int[1]~2_combout\ & (!\St1|SM_Set_Flattop~regout\ # !\Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0111011100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[1]~2_combout\,
	combout => \rtl~3_combout\);

SUB_C : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	sdata => \ADD_C~regout\,
	aclr => \120~clkctrl_outclk\,
	sload => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SUB_C~regout\);

\11\ : cycloneii_lcell_comb
-- Equation(s):
-- \11~combout\ = \SUB_C~regout\ # \St1|SM_Set_Flattop~regout\ & \Deco|Set_Flattop_DFF~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111110011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Set_Flattop~regout\,
	datac => \SUB_C~regout\,
	datad => \Deco|Set_Flattop_DFF~regout\,
	combout => \11~combout\);

\SW_Reg|dffs[1]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \rtl~3_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(1));

\SW_SUB|auto_generated|result_int[4]~8\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[4]~8_combout\ = (\SUB_REG|dffs\(10) $ \SW_Reg|dffs\(4) $ \SW_SUB|auto_generated|result_int[3]~7\) # GND
-- \SW_SUB|auto_generated|result_int[4]~9\ = CARRY(\SUB_REG|dffs\(10) & \SW_Reg|dffs\(4) & !\SW_SUB|auto_generated|result_int[3]~7\ # !\SUB_REG|dffs\(10) & (\SW_Reg|dffs\(4) # !\SW_SUB|auto_generated|result_int[3]~7\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011001001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(10),
	datab => \SW_Reg|dffs\(4),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[3]~7\,
	combout => \SW_SUB|auto_generated|result_int[4]~8_combout\,
	cout => \SW_SUB|auto_generated|result_int[4]~9\);

\rtl~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \rtl~0_combout\ = \SW_SUB|auto_generated|result_int[4]~8_combout\ & (!\St1|SM_Set_Flattop~regout\ # !\Deco|Set_Flattop_DFF~regout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0111011100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[4]~8_combout\,
	combout => \rtl~0_combout\);

\SW_Reg|dffs[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \rtl~0_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(4));

\SW_SUB|auto_generated|result_int[5]~10\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[5]~10_combout\ = \SUB_REG|dffs\(11) & (\SW_Reg|dffs\(5) & !\SW_SUB|auto_generated|result_int[4]~9\ # !\SW_Reg|dffs\(5) & (\SW_SUB|auto_generated|result_int[4]~9\ # GND)) # !\SUB_REG|dffs\(11) & (\SW_Reg|dffs\(5) & 
-- \SW_SUB|auto_generated|result_int[4]~9\ & VCC # !\SW_Reg|dffs\(5) & !\SW_SUB|auto_generated|result_int[4]~9\)
-- \SW_SUB|auto_generated|result_int[5]~11\ = CARRY(\SUB_REG|dffs\(11) & (!\SW_SUB|auto_generated|result_int[4]~9\ # !\SW_Reg|dffs\(5)) # !\SUB_REG|dffs\(11) & !\SW_Reg|dffs\(5) & !\SW_SUB|auto_generated|result_int[4]~9\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100100101011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(11),
	datab => \SW_Reg|dffs\(5),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[4]~9\,
	combout => \SW_SUB|auto_generated|result_int[5]~10_combout\,
	cout => \SW_SUB|auto_generated|result_int[5]~11\);

\3|$00000|auto_generated|result_node[5]~57\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[5]~57_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(0) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[5]~10_combout\)) # !\Deco|Set_Flattop_DFF~regout\ & 
-- (\SW_SUB|auto_generated|result_int[5]~10_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(0),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[5]~10_combout\,
	combout => \3|$00000|auto_generated|result_node[5]~57_combout\);

\SW_Reg|dffs[5]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[5]~57_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(5));

\SW_SUB|auto_generated|result_int[6]~12\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[6]~12_combout\ = (\SUB_REG|dffs\(12) $ \SW_Reg|dffs\(6) $ \SW_SUB|auto_generated|result_int[5]~11\) # GND
-- \SW_SUB|auto_generated|result_int[6]~13\ = CARRY(\SUB_REG|dffs\(12) & \SW_Reg|dffs\(6) & !\SW_SUB|auto_generated|result_int[5]~11\ # !\SUB_REG|dffs\(12) & (\SW_Reg|dffs\(6) # !\SW_SUB|auto_generated|result_int[5]~11\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011001001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SUB_REG|dffs\(12),
	datab => \SW_Reg|dffs\(6),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[5]~11\,
	combout => \SW_SUB|auto_generated|result_int[6]~12_combout\,
	cout => \SW_SUB|auto_generated|result_int[6]~13\);

\3|$00000|auto_generated|result_node[6]~56\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[6]~56_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(1) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[6]~12_combout\)) # !\Deco|Set_Flattop_DFF~regout\ & 
-- (\SW_SUB|auto_generated|result_int[6]~12_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \Flat_Reg|dffs\(1),
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[6]~12_combout\,
	combout => \3|$00000|auto_generated|result_node[6]~56_combout\);

\SW_Reg|dffs[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[6]~56_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(6));

\SW_SUB|auto_generated|result_int[7]~14\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[7]~14_combout\ = \SW_Reg|dffs\(7) & (\SUB_REG|dffs\(13) & !\SW_SUB|auto_generated|result_int[6]~13\ # !\SUB_REG|dffs\(13) & \SW_SUB|auto_generated|result_int[6]~13\ & VCC) # !\SW_Reg|dffs\(7) & (\SUB_REG|dffs\(13) & 
-- (\SW_SUB|auto_generated|result_int[6]~13\ # GND) # !\SUB_REG|dffs\(13) & !\SW_SUB|auto_generated|result_int[6]~13\)
-- \SW_SUB|auto_generated|result_int[7]~15\ = CARRY(\SW_Reg|dffs\(7) & \SUB_REG|dffs\(13) & !\SW_SUB|auto_generated|result_int[6]~13\ # !\SW_Reg|dffs\(7) & (\SUB_REG|dffs\(13) # !\SW_SUB|auto_generated|result_int[6]~13\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100101001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(7),
	datab => \SUB_REG|dffs\(13),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[6]~13\,
	combout => \SW_SUB|auto_generated|result_int[7]~14_combout\,
	cout => \SW_SUB|auto_generated|result_int[7]~15\);

\SW_SUB|auto_generated|result_int[8]~16\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[8]~16_combout\ = (\SW_Reg|dffs\(8) $ \SUB_REG|dffs\(14) $ \SW_SUB|auto_generated|result_int[7]~15\) # GND
-- \SW_SUB|auto_generated|result_int[8]~17\ = CARRY(\SW_Reg|dffs\(8) & (!\SW_SUB|auto_generated|result_int[7]~15\ # !\SUB_REG|dffs\(14)) # !\SW_Reg|dffs\(8) & !\SUB_REG|dffs\(14) & !\SW_SUB|auto_generated|result_int[7]~15\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000101011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(8),
	datab => \SUB_REG|dffs\(14),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[7]~15\,
	combout => \SW_SUB|auto_generated|result_int[8]~16_combout\,
	cout => \SW_SUB|auto_generated|result_int[8]~17\);

\SW_SUB|auto_generated|result_int[9]~18\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[9]~18_combout\ = \SW_Reg|dffs\(9) & (\SUB_REG|dffs\(15) & !\SW_SUB|auto_generated|result_int[8]~17\ # !\SUB_REG|dffs\(15) & \SW_SUB|auto_generated|result_int[8]~17\ & VCC) # !\SW_Reg|dffs\(9) & (\SUB_REG|dffs\(15) & 
-- (\SW_SUB|auto_generated|result_int[8]~17\ # GND) # !\SUB_REG|dffs\(15) & !\SW_SUB|auto_generated|result_int[8]~17\)
-- \SW_SUB|auto_generated|result_int[9]~19\ = CARRY(\SW_Reg|dffs\(9) & \SUB_REG|dffs\(15) & !\SW_SUB|auto_generated|result_int[8]~17\ # !\SW_Reg|dffs\(9) & (\SUB_REG|dffs\(15) # !\SW_SUB|auto_generated|result_int[8]~17\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100101001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(9),
	datab => \SUB_REG|dffs\(15),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[8]~17\,
	combout => \SW_SUB|auto_generated|result_int[9]~18_combout\,
	cout => \SW_SUB|auto_generated|result_int[9]~19\);

\SW_SUB|auto_generated|result_int[10]~20\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[10]~20_combout\ = (\SW_Reg|dffs\(10) $ \SUB_REG|dffs\(16) $ \SW_SUB|auto_generated|result_int[9]~19\) # GND
-- \SW_SUB|auto_generated|result_int[10]~21\ = CARRY(\SW_Reg|dffs\(10) & (!\SW_SUB|auto_generated|result_int[9]~19\ # !\SUB_REG|dffs\(16)) # !\SW_Reg|dffs\(10) & !\SUB_REG|dffs\(16) & !\SW_SUB|auto_generated|result_int[9]~19\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1001011000101011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(10),
	datab => \SUB_REG|dffs\(16),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[9]~19\,
	combout => \SW_SUB|auto_generated|result_int[10]~20_combout\,
	cout => \SW_SUB|auto_generated|result_int[10]~21\);

\SW_SUB|auto_generated|result_int[11]~22\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[11]~22_combout\ = \SW_Reg|dffs\(11) & (\SUB_REG|dffs\(17) & !\SW_SUB|auto_generated|result_int[10]~21\ # !\SUB_REG|dffs\(17) & \SW_SUB|auto_generated|result_int[10]~21\ & VCC) # !\SW_Reg|dffs\(11) & (\SUB_REG|dffs\(17) & 
-- (\SW_SUB|auto_generated|result_int[10]~21\ # GND) # !\SUB_REG|dffs\(17) & !\SW_SUB|auto_generated|result_int[10]~21\)
-- \SW_SUB|auto_generated|result_int[11]~23\ = CARRY(\SW_Reg|dffs\(11) & \SUB_REG|dffs\(17) & !\SW_SUB|auto_generated|result_int[10]~21\ # !\SW_Reg|dffs\(11) & (\SUB_REG|dffs\(17) # !\SW_SUB|auto_generated|result_int[10]~21\))

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0110100101001101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(11),
	datab => \SUB_REG|dffs\(17),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[10]~21\,
	combout => \SW_SUB|auto_generated|result_int[11]~22_combout\,
	cout => \SW_SUB|auto_generated|result_int[11]~23\);

\SW_SUB|auto_generated|result_int[12]~24\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[12]~24_combout\ = \SW_Reg|dffs\(12) & (GND # !\SW_SUB|auto_generated|result_int[11]~23\) # !\SW_Reg|dffs\(12) & (\SW_SUB|auto_generated|result_int[11]~23\ $ GND)
-- \SW_SUB|auto_generated|result_int[12]~25\ = CARRY(\SW_Reg|dffs\(12) # !\SW_SUB|auto_generated|result_int[11]~23\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(12),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[11]~23\,
	combout => \SW_SUB|auto_generated|result_int[12]~24_combout\,
	cout => \SW_SUB|auto_generated|result_int[12]~25\);

\3|$00000|auto_generated|result_node[12]~50\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[12]~50_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(7) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[12]~24_combout\)) # !\Deco|Set_Flattop_DFF~regout\ 
-- & (\SW_SUB|auto_generated|result_int[12]~24_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(7),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[12]~24_combout\,
	combout => \3|$00000|auto_generated|result_node[12]~50_combout\);

\SW_Reg|dffs[12]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[12]~50_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(12));

\SW_SUB|auto_generated|result_int[13]~26\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[13]~26_combout\ = \SW_Reg|dffs\(13) & \SW_SUB|auto_generated|result_int[12]~25\ & VCC # !\SW_Reg|dffs\(13) & !\SW_SUB|auto_generated|result_int[12]~25\
-- \SW_SUB|auto_generated|result_int[13]~27\ = CARRY(!\SW_Reg|dffs\(13) & !\SW_SUB|auto_generated|result_int[12]~25\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(13),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[12]~25\,
	combout => \SW_SUB|auto_generated|result_int[13]~26_combout\,
	cout => \SW_SUB|auto_generated|result_int[13]~27\);

\3|$00000|auto_generated|result_node[13]~49\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[13]~49_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & \Flat_Reg|dffs\(8) # !\Deco|Set_Flattop_DFF~regout\ & (\SW_SUB|auto_generated|result_int[13]~26_combout\)) # !\St1|SM_Set_Flattop~regout\ 
-- & (\SW_SUB|auto_generated|result_int[13]~26_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Flat_Reg|dffs\(8),
	datac => \Deco|Set_Flattop_DFF~regout\,
	datad => \SW_SUB|auto_generated|result_int[13]~26_combout\,
	combout => \3|$00000|auto_generated|result_node[13]~49_combout\);

\SW_Reg|dffs[13]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[13]~49_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(13));

\SW_SUB|auto_generated|result_int[14]~28\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[14]~28_combout\ = \SW_Reg|dffs\(14) & (GND # !\SW_SUB|auto_generated|result_int[13]~27\) # !\SW_Reg|dffs\(14) & (\SW_SUB|auto_generated|result_int[13]~27\ $ GND)
-- \SW_SUB|auto_generated|result_int[14]~29\ = CARRY(\SW_Reg|dffs\(14) # !\SW_SUB|auto_generated|result_int[13]~27\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0101101010101111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(14),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[13]~27\,
	combout => \SW_SUB|auto_generated|result_int[14]~28_combout\,
	cout => \SW_SUB|auto_generated|result_int[14]~29\);

\SW_SUB|auto_generated|result_int[15]~30\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[15]~30_combout\ = \SW_Reg|dffs\(15) & \SW_SUB|auto_generated|result_int[14]~29\ & VCC # !\SW_Reg|dffs\(15) & !\SW_SUB|auto_generated|result_int[14]~29\
-- \SW_SUB|auto_generated|result_int[15]~31\ = CARRY(!\SW_Reg|dffs\(15) & !\SW_SUB|auto_generated|result_int[14]~29\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(15),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[14]~29\,
	combout => \SW_SUB|auto_generated|result_int[15]~30_combout\,
	cout => \SW_SUB|auto_generated|result_int[15]~31\);

\SW_SUB|auto_generated|result_int[16]~32\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[16]~32_combout\ = \SW_Reg|dffs\(16) & (GND # !\SW_SUB|auto_generated|result_int[15]~31\) # !\SW_Reg|dffs\(16) & (\SW_SUB|auto_generated|result_int[15]~31\ $ GND)
-- \SW_SUB|auto_generated|result_int[16]~33\ = CARRY(\SW_Reg|dffs\(16) # !\SW_SUB|auto_generated|result_int[15]~31\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(16),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[15]~31\,
	combout => \SW_SUB|auto_generated|result_int[16]~32_combout\,
	cout => \SW_SUB|auto_generated|result_int[16]~33\);

\3|$00000|auto_generated|result_node[16]~46\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[16]~46_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & \Flat_Reg|dffs\(11) # !\Deco|Set_Flattop_DFF~regout\ & (\SW_SUB|auto_generated|result_int[16]~32_combout\)) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[16]~32_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111011110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \Flat_Reg|dffs\(11),
	datad => \SW_SUB|auto_generated|result_int[16]~32_combout\,
	combout => \3|$00000|auto_generated|result_node[16]~46_combout\);

\SW_Reg|dffs[16]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[16]~46_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(16));

\SW_SUB|auto_generated|result_int[17]~34\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[17]~34_combout\ = \SW_Reg|dffs\(17) & \SW_SUB|auto_generated|result_int[16]~33\ & VCC # !\SW_Reg|dffs\(17) & !\SW_SUB|auto_generated|result_int[16]~33\
-- \SW_SUB|auto_generated|result_int[17]~35\ = CARRY(!\SW_Reg|dffs\(17) & !\SW_SUB|auto_generated|result_int[16]~33\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100001100000011",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(17),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[16]~33\,
	combout => \SW_SUB|auto_generated|result_int[17]~34_combout\,
	cout => \SW_SUB|auto_generated|result_int[17]~35\);

\3|$00000|auto_generated|result_node[17]~45\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[17]~45_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & \Flat_Reg|dffs\(12) # !\Deco|Set_Flattop_DFF~regout\ & (\SW_SUB|auto_generated|result_int[17]~34_combout\)) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[17]~34_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Flat_Reg|dffs\(12),
	datac => \Deco|Set_Flattop_DFF~regout\,
	datad => \SW_SUB|auto_generated|result_int[17]~34_combout\,
	combout => \3|$00000|auto_generated|result_node[17]~45_combout\);

\SW_Reg|dffs[17]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[17]~45_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(17));

\SW_SUB|auto_generated|result_int[18]~36\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[18]~36_combout\ = \SW_Reg|dffs\(18) & (GND # !\SW_SUB|auto_generated|result_int[17]~35\) # !\SW_Reg|dffs\(18) & (\SW_SUB|auto_generated|result_int[17]~35\ $ GND)
-- \SW_SUB|auto_generated|result_int[18]~37\ = CARRY(\SW_Reg|dffs\(18) # !\SW_SUB|auto_generated|result_int[17]~35\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(18),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[17]~35\,
	combout => \SW_SUB|auto_generated|result_int[18]~36_combout\,
	cout => \SW_SUB|auto_generated|result_int[18]~37\);

\D[13]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(13),
	combout => \D~combout\(13));

\Flat_Reg|dffs[13]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[13]~feeder_combout\ = \D~combout\(13)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(13),
	combout => \Flat_Reg|dffs[13]~feeder_combout\);

\Flat_Reg|dffs[13]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[13]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(13));

\3|$00000|auto_generated|result_node[18]~44\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[18]~44_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & (\Flat_Reg|dffs\(13)) # !\Deco|Set_Flattop_DFF~regout\ & \SW_SUB|auto_generated|result_int[18]~36_combout\) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[18]~36_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111100001110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \SW_SUB|auto_generated|result_int[18]~36_combout\,
	datad => \Flat_Reg|dffs\(13),
	combout => \3|$00000|auto_generated|result_node[18]~44_combout\);

\SW_Reg|dffs[18]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[18]~44_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(18));

\SW_SUB|auto_generated|result_int[19]~38\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[19]~38_combout\ = \SW_Reg|dffs\(19) & \SW_SUB|auto_generated|result_int[18]~37\ & VCC # !\SW_Reg|dffs\(19) & !\SW_SUB|auto_generated|result_int[18]~37\
-- \SW_SUB|auto_generated|result_int[19]~39\ = CARRY(!\SW_Reg|dffs\(19) & !\SW_SUB|auto_generated|result_int[18]~37\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010010100000101",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	dataa => \SW_Reg|dffs\(19),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[18]~37\,
	combout => \SW_SUB|auto_generated|result_int[19]~38_combout\,
	cout => \SW_SUB|auto_generated|result_int[19]~39\);

\SW_SUB|auto_generated|result_int[20]~40\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[20]~40_combout\ = \SW_Reg|dffs\(20) & (GND # !\SW_SUB|auto_generated|result_int[19]~39\) # !\SW_Reg|dffs\(20) & (\SW_SUB|auto_generated|result_int[19]~39\ $ GND)
-- \SW_SUB|auto_generated|result_int[20]~41\ = CARRY(\SW_Reg|dffs\(20) # !\SW_SUB|auto_generated|result_int[19]~39\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0011110011001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	datab => \SW_Reg|dffs\(20),
	datad => VCC,
	cin => \SW_SUB|auto_generated|result_int[19]~39\,
	combout => \SW_SUB|auto_generated|result_int[20]~40_combout\,
	cout => \SW_SUB|auto_generated|result_int[20]~41\);

\SW_SUB|auto_generated|result_int[21]~42\ : cycloneii_lcell_comb
-- Equation(s):
-- \SW_SUB|auto_generated|result_int[21]~42_combout\ = !\SW_SUB|auto_generated|result_int[20]~41\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111100001111",
	sum_lutc_input => "cin")
-- pragma translate_on
PORT MAP (
	cin => \SW_SUB|auto_generated|result_int[20]~41\,
	combout => \SW_SUB|auto_generated|result_int[21]~42_combout\);

R_Fin : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \SW_SUB|auto_generated|result_int[21]~42_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \R_Fin~regout\);

\St1|Init_HW_DFF~0\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Init_HW_DFF~0_combout\ = \R_Fin~regout\ # !\St1|SM_Idle~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100110011",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datab => \St1|SM_Idle~regout\,
	datad => \R_Fin~regout\,
	combout => \St1|Init_HW_DFF~0_combout\);

\St1|Init_HW_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Init_HW_DFF~0_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Init_HW_DFF~regout\);

\St1|Init_HW_DFF~clkctrl\ : cycloneii_clkctrl
-- pragma translate_off
GENERIC MAP (
	clock_type => "global clock",
	ena_register_mode => "falling edge")
-- pragma translate_on
PORT MAP (
	inclk => \St1|Init_HW_DFF~clkctrl_INCLK_bus\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	outclk => \St1|Init_HW_DFF~clkctrl_outclk\);

\Flat_Reg|dffs[15]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[15]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(15));

\3|$00000|auto_generated|result_node[20]~42\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[20]~42_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & \Flat_Reg|dffs\(15) # !\Deco|Set_Flattop_DFF~regout\ & (\SW_SUB|auto_generated|result_int[20]~40_combout\)) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[20]~40_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Flat_Reg|dffs\(15),
	datac => \Deco|Set_Flattop_DFF~regout\,
	datad => \SW_SUB|auto_generated|result_int[20]~40_combout\,
	combout => \3|$00000|auto_generated|result_node[20]~42_combout\);

\SW_Reg|dffs[20]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[20]~42_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(20));

\D[14]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "input",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => GND,
	padio => ww_D(14),
	combout => \D~combout\(14));

\Flat_Reg|dffs[14]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[14]~feeder_combout\ = \D~combout\(14)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(14),
	combout => \Flat_Reg|dffs[14]~feeder_combout\);

\Flat_Reg|dffs[14]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[14]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(14));

\3|$00000|auto_generated|result_node[19]~43\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[19]~43_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & (\Flat_Reg|dffs\(14)) # !\Deco|Set_Flattop_DFF~regout\ & \SW_SUB|auto_generated|result_int[19]~38_combout\) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[19]~38_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111100001110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \SW_SUB|auto_generated|result_int[19]~38_combout\,
	datad => \Flat_Reg|dffs\(14),
	combout => \3|$00000|auto_generated|result_node[19]~43_combout\);

\SW_Reg|dffs[19]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[19]~43_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(19));

\Flat_Reg|dffs[10]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[10]~feeder_combout\ = \D~combout\(10)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(10),
	combout => \Flat_Reg|dffs[10]~feeder_combout\);

\Flat_Reg|dffs[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[10]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(10));

\3|$00000|auto_generated|result_node[15]~47\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[15]~47_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & (\Flat_Reg|dffs\(10)) # !\Deco|Set_Flattop_DFF~regout\ & \SW_SUB|auto_generated|result_int[15]~30_combout\) # 
-- !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[15]~30_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111100001110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \SW_SUB|auto_generated|result_int[15]~30_combout\,
	datad => \Flat_Reg|dffs\(10),
	combout => \3|$00000|auto_generated|result_node[15]~47_combout\);

\SW_Reg|dffs[15]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[15]~47_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(15));

\3|$00000|auto_generated|result_node[14]~48\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[14]~48_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(9) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[14]~28_combout\)) # !\Deco|Set_Flattop_DFF~regout\ 
-- & (\SW_SUB|auto_generated|result_int[14]~28_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(9),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[14]~28_combout\,
	combout => \3|$00000|auto_generated|result_node[14]~48_combout\);

\SW_Reg|dffs[14]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[14]~48_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(14));

\Flat_Reg|dffs[6]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[6]~feeder_combout\ = \D~combout\(6)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \D~combout\(6),
	combout => \Flat_Reg|dffs[6]~feeder_combout\);

\Flat_Reg|dffs[6]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[6]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(6));

\3|$00000|auto_generated|result_node[11]~51\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[11]~51_combout\ = \St1|SM_Set_Flattop~regout\ & (\Deco|Set_Flattop_DFF~regout\ & (\Flat_Reg|dffs\(6)) # !\Deco|Set_Flattop_DFF~regout\ & \SW_SUB|auto_generated|result_int[11]~22_combout\) # !\St1|SM_Set_Flattop~regout\ 
-- & (\SW_SUB|auto_generated|result_int[11]~22_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111100001110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Set_Flattop~regout\,
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \SW_SUB|auto_generated|result_int[11]~22_combout\,
	datad => \Flat_Reg|dffs\(6),
	combout => \3|$00000|auto_generated|result_node[11]~51_combout\);

\SW_Reg|dffs[11]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[11]~51_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(11));

\3|$00000|auto_generated|result_node[10]~52\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[10]~52_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(5) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[10]~20_combout\)) # !\Deco|Set_Flattop_DFF~regout\ 
-- & (\SW_SUB|auto_generated|result_int[10]~20_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(5),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[10]~20_combout\,
	combout => \3|$00000|auto_generated|result_node[10]~52_combout\);

\SW_Reg|dffs[10]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[10]~52_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(10));

\Flat_Reg|dffs[4]~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \Flat_Reg|dffs[4]~feeder_combout\ = \D~combout\(4)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111000011110000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \D~combout\(4),
	combout => \Flat_Reg|dffs[4]~feeder_combout\);

\Flat_Reg|dffs[4]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \Flat_Reg|dffs[4]~feeder_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \St1|Wr_FT_Int~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \Flat_Reg|dffs\(4));

\3|$00000|auto_generated|result_node[9]~53\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[9]~53_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(4) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[9]~18_combout\)) # !\Deco|Set_Flattop_DFF~regout\ & 
-- (\SW_SUB|auto_generated|result_int[9]~18_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1101111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Deco|Set_Flattop_DFF~regout\,
	datab => \Flat_Reg|dffs\(4),
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[9]~18_combout\,
	combout => \3|$00000|auto_generated|result_node[9]~53_combout\);

\SW_Reg|dffs[9]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[9]~53_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(9));

\3|$00000|auto_generated|result_node[8]~54\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[8]~54_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(3) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[8]~16_combout\)) # !\Deco|Set_Flattop_DFF~regout\ & 
-- (\SW_SUB|auto_generated|result_int[8]~16_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(3),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[8]~16_combout\,
	combout => \3|$00000|auto_generated|result_node[8]~54_combout\);

\SW_Reg|dffs[8]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[8]~54_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(8));

\3|$00000|auto_generated|result_node[7]~55\ : cycloneii_lcell_comb
-- Equation(s):
-- \3|$00000|auto_generated|result_node[7]~55_combout\ = \Deco|Set_Flattop_DFF~regout\ & (\St1|SM_Set_Flattop~regout\ & \Flat_Reg|dffs\(2) # !\St1|SM_Set_Flattop~regout\ & (\SW_SUB|auto_generated|result_int[7]~14_combout\)) # !\Deco|Set_Flattop_DFF~regout\ & 
-- (\SW_SUB|auto_generated|result_int[7]~14_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1011111110000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \Flat_Reg|dffs\(2),
	datab => \Deco|Set_Flattop_DFF~regout\,
	datac => \St1|SM_Set_Flattop~regout\,
	datad => \SW_SUB|auto_generated|result_int[7]~14_combout\,
	combout => \3|$00000|auto_generated|result_node[7]~55_combout\);

\SW_Reg|dffs[7]\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \3|$00000|auto_generated|result_node[7]~55_combout\,
	aclr => \St1|Init_HW_DFF~clkctrl_outclk\,
	ena => \11~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \SW_Reg|dffs\(7));

\St1|W_Start_DFF~feeder\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|W_Start_DFF~feeder_combout\ = \St1|SM_W_Start~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1111111100000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datad => \St1|SM_W_Start~regout\,
	combout => \St1|W_Start_DFF~feeder_combout\);

\St1|W_Start_DFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|W_Start_DFF~feeder_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|W_Start_DFF~regout\);

\St1|Idle\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Idle~combout\ = \St1|SM_Seq_Err~regout\ # !\St1|SM_Idle~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010101011111111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|SM_Seq_Err~regout\,
	datad => \St1|SM_Idle~regout\,
	combout => \St1|Idle~combout\);

\St1|Stop_Exec_SRFF~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Stop_Exec_SRFF~3_combout\ = \St1|Stop_Exec_SRFF~regout\ & \St1|Wr_Delta~combout\ # !\St1|Stop_Exec_SRFF~regout\ & (\Deco|Stop_DFF~regout\ & \St1|S_Stop_Delta~5_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1010110010100000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Wr_Delta~combout\,
	datab => \Deco|Stop_DFF~regout\,
	datac => \St1|Stop_Exec_SRFF~regout\,
	datad => \St1|S_Stop_Delta~5_combout\,
	combout => \St1|Stop_Exec_SRFF~3_combout\);

\St1|Stop_Exec_SRFF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|Stop_Exec_SRFF~3_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|Stop_Exec_SRFF~regout\);

\St1|Wr_Delta\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|Wr_Delta~combout\ = !\Deco|Ld_Delta_DFF~regout\ # !\St1|SM_Wr_Delta~regout\

-- pragma translate_off
GENERIC MAP (
	lut_mask => "0000111111111111",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	datac => \St1|SM_Wr_Delta~regout\,
	datad => \Deco|Ld_Delta_DFF~regout\,
	combout => \St1|Wr_Delta~combout\);

\St1|TO_Err_FF~3\ : cycloneii_lcell_comb
-- Equation(s):
-- \St1|TO_Err_FF~3_combout\ = \St1|TO_Err_FF~regout\ & (\St1|Wr_Delta~combout\) # !\St1|TO_Err_FF~regout\ & \St1|Timeout~regout\ & (\St1|_~93_combout\)

-- pragma translate_off
GENERIC MAP (
	lut_mask => "1100101011000000",
	sum_lutc_input => "datac")
-- pragma translate_on
PORT MAP (
	dataa => \St1|Timeout~regout\,
	datab => \St1|Wr_Delta~combout\,
	datac => \St1|TO_Err_FF~regout\,
	datad => \St1|_~93_combout\,
	combout => \St1|TO_Err_FF~3_combout\);

\St1|TO_Err_FF\ : cycloneii_lcell_ff
PORT MAP (
	clk => \CLK~clkctrl_outclk\,
	datain => \St1|TO_Err_FF~3_combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	regout => \St1|TO_Err_FF~regout\);

\Stat_Sel~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \Deco|STAT1_DFF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => ww_Stat_Sel);

\Sweep-Out[15]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(20),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(15));

\Sweep-Out[14]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(19),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(14));

\Sweep-Out[13]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(18),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(13));

\Sweep-Out[12]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(17),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(12));

\Sweep-Out[11]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(16),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(11));

\Sweep-Out[10]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(15),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(10));

\Sweep-Out[9]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(14),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(9));

\Sweep-Out[8]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(13),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(8));

\Sweep-Out[7]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(12),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(7));

\Sweep-Out[6]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(11),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(6));

\Sweep-Out[5]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(10),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(5));

\Sweep-Out[4]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(9),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(4));

\Sweep-Out[3]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(8),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(3));

\Sweep-Out[2]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(7),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(2));

\Sweep-Out[1]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(6),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(1));

\Sweep-Out[0]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \SW_Reg|dffs\(5),
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Out\(0));

\Sweep-Stat[15]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(15));

\Sweep-Stat[14]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|W_Start_DFF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(14));

\Sweep-Stat[13]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|Work_DFF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(13));

\Sweep-Stat[12]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|Idle~combout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(12));

\Sweep-Stat[11]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|Stop_Exec_SRFF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(11));

\Sweep-Stat[10]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|TO_Err_FF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(10));

\Sweep-Stat[9]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|Seq_Err_FF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(9));

\Sweep-Stat[8]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \St1|Trigger_FF~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(8));

\Sweep-Stat[7]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(7));

\Sweep-Stat[6]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(6));

\Sweep-Stat[5]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(5));

\Sweep-Stat[4]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => \Deco|Ena_Soft_Trig_Dff~regout\,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(4));

\Sweep-Stat[3]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(3));

\Sweep-Stat[2]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(2));

\Sweep-Stat[1]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(1));

\Sweep-Stat[0]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_Sweep-Stat\(0));

\SWEEP-VERS[3]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_SWEEP-VERS\(3));

\SWEEP-VERS[2]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => VCC,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_SWEEP-VERS\(2));

\SWEEP-VERS[1]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_SWEEP-VERS\(1));

\SWEEP-VERS[0]~I\ : cycloneii_io
-- pragma translate_off
GENERIC MAP (
	input_async_reset => "none",
	input_power_up => "low",
	input_register_mode => "none",
	input_sync_reset => "none",
	oe_async_reset => "none",
	oe_power_up => "low",
	oe_register_mode => "none",
	oe_sync_reset => "none",
	operation_mode => "output",
	output_async_reset => "none",
	output_power_up => "low",
	output_register_mode => "none",
	output_sync_reset => "none")
-- pragma translate_on
PORT MAP (
	datain => GND,
	devclrn => ww_devclrn,
	devpor => ww_devpor,
	devoe => ww_devoe,
	oe => VCC,
	padio => \ww_SWEEP-VERS\(0));
END structure;


