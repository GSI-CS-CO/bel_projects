-- testbench for chopper m1 
-- 13.03.2008
-- testing min puls length of Off_Alv and Off_HSI


LIBRARY ieee;                                               
USE ieee.std_logic_1164.all;                                
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_signed.all;

LIBRARY STD;


ENTITY chopper_tb IS
END chopper_tb;



ARCHITECTURE chopper_arch OF chopper_tb IS
-- constants                                                 
-- signals
--signal	chopper_clk : std_logic := '0';                                                   
SIGNAL t_sig_A_K0D_SPG : STD_LOGIC := '0';
SIGNAL t_sig_A_K0C_SPG : STD_LOGIC := '1';
SIGNAL t_sig_A_K1D_SPG : STD_LOGIC := '0';
SIGNAL t_sig_A_K1C_SPG : STD_LOGIC := '1';
SIGNAL t_sig_A_K2D_SPG : STD_LOGIC := '0';
SIGNAL t_sig_A_K2C_SPG : STD_LOGIC := '1';
SIGNAL t_sig_A_K3D_SPG : STD_LOGIC := '0';
SIGNAL t_sig_A_K3C_SPG : STD_LOGIC := '1';
SIGNAL t_sig_CTRL_LOAD : STD_LOGIC := 'L';
SIGNAL t_sig_CTRL_RES : STD_LOGIC := 'L';
SIGNAL t_sig_Loader_CLK : STD_LOGIC;
SIGNAL t_sig_SEL_B : STD_LOGIC_VECTOR(3 downto 0);
SIGNAL t_sig_A_nDS : STD_LOGIC := '1';
SIGNAL t_sig_A_VG_A : STD_LOGIC_VECTOR(4 downto 0);
SIGNAL t_sig_A_A : STD_LOGIC_VECTOR(4 downto 0);
SIGNAL t_sig_chopper_clk : STD_LOGIC := '0';
SIGNAL t_sig_A_nMB_Reset : STD_LOGIC;
SIGNAL t_sig_A_VG_ID : STD_LOGIC_VECTOR(7 downto 0);
SIGNAL t_sig_A_RDnWR : STD_LOGIC;
SIGNAL t_sig_A_Sub_Adr : STD_LOGIC_VECTOR(7 downto 0) := "HHHHHHHH";
SIGNAL t_sig_F_TCXO_In : STD_LOGIC := '0';
SIGNAL t_sig_A_VG_K3_INP : STD_LOGIC;
SIGNAL t_sig_nSEL_LED_GRP : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_A_VG_K1_INP : STD_LOGIC;
SIGNAL t_sig_A_VG_K0_INP : STD_LOGIC;
SIGNAL t_sig_A_VG_K2_INP : STD_LOGIC;
SIGNAL t_sig_A_GR1_APK_ID : STD_LOGIC;
SIGNAL t_sig_A_VG_Log_ID : STD_LOGIC_VECTOR(5 downto 0);
SIGNAL t_sig_A_GR0_APK_ID : STD_LOGIC;
SIGNAL t_sig_A_GR1_16BIT : STD_LOGIC;
SIGNAL t_sig_A_GR0_16BIT : STD_LOGIC;
SIGNAL t_sig_A_VG_K3_MOD : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_A_VG_K1_MOD : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_A_VG_K2_MOD : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_A_VG_K0_MOD : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_nPowerup_Led : STD_LOGIC;
SIGNAL t_sig_nExt_Data_En : STD_LOGIC;
SIGNAL t_sig_nDT_Led : STD_LOGIC;
SIGNAL t_sig_nSkal_OK_Led : STD_LOGIC;
SIGNAL t_sig_nID_OK_Led : STD_LOGIC;
SIGNAL t_sig_A_nK0_ID_EN : STD_LOGIC;
SIGNAL t_sig_A_nK1_ID_EN : STD_LOGIC;
SIGNAL t_sig_A_nK2_ID_EN : STD_LOGIC;
SIGNAL t_sig_A_nK3_ID_EN : STD_LOGIC := 'H';
SIGNAL t_sig_A_nINTERLOCKA : STD_LOGIC;
SIGNAL t_sig_A_nINTERLOCKB : STD_LOGIC;
SIGNAL t_sig_A_nSRQA : STD_LOGIC;
SIGNAL t_sig_A_nSRQB : STD_LOGIC;
SIGNAL t_sig_A_nDRQB : STD_LOGIC;
SIGNAL t_sig_A_nDRQA : STD_LOGIC;
SIGNAL t_sig_A_nDTACKA : STD_LOGIC;
SIGNAL t_sig_A_nDTACKB : STD_LOGIC;
SIGNAL t_sig_nMaster_Clk_ENA : STD_LOGIC;
SIGNAL t_sig_nSlave_Clk_ENA : STD_LOGIC;
SIGNAL t_sig_nIndepend_Clk_Ena : STD_LOGIC;
SIGNAL t_sig_nK0_SWITCH_ENA : STD_LOGIC;
SIGNAL t_sig_nK1_SWITCH_ENA : STD_LOGIC;
SIGNAL t_sig_nK2_SWITCH_ENA : STD_LOGIC;
SIGNAL t_sig_nK3_SWITCH_ENA : STD_LOGIC;
SIGNAL t_sig_Loader_WRnRD : STD_LOGIC;
SIGNAL t_sig_INIT_DONE : STD_LOGIC;
SIGNAL t_sig_LOAD_OK : STD_LOGIC;
SIGNAL t_sig_LOAD_ERROR : STD_LOGIC;
SIGNAL t_sig_RELOAD : STD_LOGIC;
SIGNAL t_sig_A_Master_Clk_Out : STD_LOGIC;
SIGNAL t_sig_A_nGR0_ID_SEL : STD_LOGIC := 'H';
SIGNAL t_sig_A_nGR1_ID_SEL : STD_LOGIC := 'H';
SIGNAL t_sig_A_LA_CLK : STD_LOGIC;
SIGNAL t_sig_A_nMANUAL_RES : STD_LOGIC;
SIGNAL t_sig_nLED : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_nLED_Skal : STD_LOGIC_VECTOR(7 downto 0);
SIGNAL t_sig_A_I2C_SDA : STD_LOGIC;
SIGNAL t_sig_A_I2C_SCL : STD_LOGIC;
SIGNAL t_sig_A_K3_D : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_A_K2_D : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_A_Mod_Data : STD_LOGIC_VECTOR(7 downto 0);
SIGNAL t_sig_A_AUX_A : STD_LOGIC_VECTOR(11 downto 0);
SIGNAL t_sig_A_AUX_B : STD_LOGIC_VECTOR(11 downto 0);
SIGNAL t_sig_A_AUX_C : STD_LOGIC_VECTOR(11 downto 0);
SIGNAL t_sig_A_Bus_IO : STD_LOGIC_VECTOR(5 downto 1);
SIGNAL t_sig_A_K0_D : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_A_nK0_CTRL : STD_LOGIC_VECTOR(2 downto 1);
SIGNAL t_sig_A_K1_D : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_A_nK1_CTRL : STD_LOGIC_VECTOR(2 downto 1);
SIGNAL t_sig_A_nK2_CTRL : STD_LOGIC_VECTOR(2 downto 1);
SIGNAL t_sig_A_nK3_CTRL : STD_LOGIC_VECTOR(2 downto 1);
SIGNAL t_sig_A_Test : STD_LOGIC_VECTOR(15 downto 0);
SIGNAL t_sig_A_VG_IO_Res : STD_LOGIC_VECTOR(1 downto 0);
SIGNAL t_sig_Loader_DB : STD_LOGIC_VECTOR(3 downto 0);
SIGNAL t_sig_Loader_Misc : STD_LOGIC_VECTOR(3 downto 0);
SIGNAL t_sig_TP : STD_LOGIC_VECTOR(12 downto 1);

SIGNAL	Async_On	: INTEGER := 0;

SIGNAL	In_K1_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	In_K0_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen

SIGNAL	Mux_In_K0_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K1_D : STD_LOGIC_VECTOR(15 downto 0) := x"FFFF";	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K2_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen
SIGNAL	Mux_In_K3_D : STD_LOGIC_VECTOR(15 downto 0);	-- Eingangssignale muessen ueber getrennte Signale beschrieben werden, sonst Konflikt mit Apk_ID-Lesen



--COMPONENT Kicker_Vers_1_Rev_0
COMPONENT chopper_m1
	PORT (
	A_K0D_SPG : in STD_LOGIC;
	A_K0C_SPG : in STD_LOGIC;
	A_K1D_SPG : in STD_LOGIC;
	A_K1C_SPG : in STD_LOGIC;
	A_K2D_SPG : in STD_LOGIC;
	A_K2C_SPG : in STD_LOGIC;
	A_K3D_SPG : in STD_LOGIC;
	A_K3C_SPG : in STD_LOGIC;
	CTRL_LOAD : in STD_LOGIC;
	CTRL_RES : in STD_LOGIC;
	--Loader_CLK : in STD_LOGIC;
	SEL_B : in STD_LOGIC_VECTOR(3 downto 0);
	A_nDS : in STD_LOGIC;
	A_VG_A : in STD_LOGIC_VECTOR(4 downto 0);
	A_A : in STD_LOGIC_VECTOR(4 downto 0);
	chopper_clk : in STD_LOGIC;
	A_nMB_Reset : in STD_LOGIC;
	A_VG_ID : inout STD_LOGIC_VECTOR(7 downto 0);
	A_RDnWR : in STD_LOGIC;
	A_Sub_Adr : in STD_LOGIC_VECTOR(7 downto 0);
	F_TCXO_In : in STD_LOGIC;
	A_VG_K3_INP : in STD_LOGIC;
	nSEL_LED_GRP : in STD_LOGIC_VECTOR(1 downto 0);
	A_VG_K1_INP : in STD_LOGIC;
	A_VG_K0_INP : in STD_LOGIC;
	A_VG_K2_INP : in STD_LOGIC;
	A_GR1_APK_ID : in STD_LOGIC;
	A_VG_Log_ID : in STD_LOGIC_VECTOR(5 downto 0);
	A_GR0_APK_ID : in STD_LOGIC;
	A_GR1_16BIT : in STD_LOGIC;
	A_GR0_16BIT : in STD_LOGIC;
	A_VG_K3_MOD : in STD_LOGIC_VECTOR(1 downto 0);
	A_VG_K1_MOD : in STD_LOGIC_VECTOR(1 downto 0);
	A_VG_K2_MOD : in STD_LOGIC_VECTOR(1 downto 0);
	A_VG_K0_MOD : in STD_LOGIC_VECTOR(1 downto 0);
	nPowerup_Led : out STD_LOGIC;
	nExt_Data_En : out STD_LOGIC;
	nDT_Led : out STD_LOGIC;
	nSkal_OK_Led : out STD_LOGIC;
	nID_OK_Led : out STD_LOGIC;
	A_nK0_ID_EN : out STD_LOGIC;
	A_nK1_ID_EN : out STD_LOGIC;
	A_nK2_ID_EN : out STD_LOGIC;
	A_nK3_ID_EN : out STD_LOGIC;
	A_nINTERLOCKA : out STD_LOGIC;
	A_nINTERLOCKB : out STD_LOGIC;
	A_nSRQA : out STD_LOGIC;
	A_nSRQB : out STD_LOGIC;
	A_nDRQB : out STD_LOGIC;
	A_nDRQA : out STD_LOGIC;
	A_nDTACKA : out STD_LOGIC;
	A_nDTACKB : out STD_LOGIC;
	nMaster_Clk_ENA : out STD_LOGIC;
	nSlave_Clk_ENA : out STD_LOGIC;
	nIndepend_Clk_Ena : out STD_LOGIC;
	nK0_SWITCH_ENA : out STD_LOGIC;
	nK1_SWITCH_ENA : out STD_LOGIC;
	nK2_SWITCH_ENA : out STD_LOGIC;
	nK3_SWITCH_ENA : out STD_LOGIC;
	Loader_WRnRD : out STD_LOGIC;
	INIT_DONE : out STD_LOGIC;
	LOAD_OK : out STD_LOGIC;
	LOAD_ERROR : out STD_LOGIC;
	RELOAD : out STD_LOGIC;
--	chopper_clk : in std_logic;
	A_Master_Clk_Out : out STD_LOGIC;
	A_nGR0_ID_SEL : out STD_LOGIC;
	A_nGR1_ID_SEL : out STD_LOGIC;
	A_LA_CLK : out STD_LOGIC;
	A_nMANUAL_RES : out STD_LOGIC;
	nLED : out STD_LOGIC_VECTOR(15 downto 0);
	nLED_Skal : out STD_LOGIC_VECTOR(7 downto 0);
	A_I2C_SDA : inout STD_LOGIC;
	A_I2C_SCL : inout STD_LOGIC;
	A_K3_D : inout STD_LOGIC_VECTOR(15 downto 0);
	A_K2_D : inout STD_LOGIC_VECTOR(15 downto 0);
	A_Mod_Data : inout STD_LOGIC_VECTOR(7 downto 0);
	A_AUX_A : inout STD_LOGIC_VECTOR(11 downto 0);
	A_AUX_B : inout STD_LOGIC_VECTOR(11 downto 0);
	A_AUX_C : inout STD_LOGIC_VECTOR(11 downto 0);
	A_Bus_IO : inout STD_LOGIC_VECTOR(5 downto 1);
	A_K0_D : inout STD_LOGIC_VECTOR(15 downto 0);
	A_nK0_CTRL : inout STD_LOGIC_VECTOR(2 downto 1);
	A_K1_D : inout STD_LOGIC_VECTOR(15 downto 0);
	A_nK1_CTRL : inout STD_LOGIC_VECTOR(2 downto 1);
	A_nK2_CTRL : inout STD_LOGIC_VECTOR(2 downto 1);
	A_nK3_CTRL : inout STD_LOGIC_VECTOR(2 downto 1);
	A_Test : inout STD_LOGIC_VECTOR(15 downto 0);
	A_VG_IO_Res : inout STD_LOGIC_VECTOR(1 downto 0);
	Loader_DB : inout STD_LOGIC_VECTOR(3 downto 0);
	Loader_Misc : inout STD_LOGIC_VECTOR(3 downto 0);
	TP : inout STD_LOGIC_VECTOR(12 downto 1)	);
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
CONSTANT 	C_TK8_Delay_RW				: std_logic_vector(7 downto 0) := X"7C";


CONSTANT	WR	: STD_LOGIC := '0';
CONSTANT	RD	: STD_LOGIC := '1';

SIGNAL	S_Last_Rd_Memory	: STD_LOGIC_VECTOR(15 DOWNTO 0);


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
		EXIT w_dtack1 when (t_sig_A_nDTACKA = '0') AND (Async_On = 1);
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
		EXIT w_dtack2 when (t_sig_A_nDTACKA = '0') AND (Async_On = 1);
	END LOOP w_dtack2;
--	wait for 10 ns;
	nDS <= '1';
	wait for 100 ns;	
	Mod_Adr_Out <= (OTHERS => 'H');
	Sub_Adr_Out <= (OTHERS => 'H');
	wait for 100 ns;
	END MB_Cycle;
	

BEGIN
--	tb : Kicker_Vers_1_Rev_0	PORT MAP (
	tb : chopper_m1	PORT MAP (
-- list connections between master ports and signals
	A_K0D_SPG => t_sig_A_K0D_SPG,
	A_K0C_SPG => t_sig_A_K0C_SPG,
	A_K1D_SPG => t_sig_A_K1D_SPG,
	A_K1C_SPG => t_sig_A_K1C_SPG,
	A_K2D_SPG => t_sig_A_K2D_SPG,
	A_K2C_SPG => t_sig_A_K2C_SPG,
	A_K3D_SPG => t_sig_A_K3D_SPG,
	A_K3C_SPG => t_sig_A_K3C_SPG,
	CTRL_LOAD => t_sig_CTRL_LOAD,
	CTRL_RES => t_sig_CTRL_RES,
--	Loader_CLK => t_sig_Loader_CLK,
	SEL_B => t_sig_SEL_B,
	A_nDS => t_sig_A_nDS,
	A_VG_A => t_sig_A_VG_A,
	A_A => t_sig_A_A,
	chopper_clk => t_sig_chopper_clk,
	A_nMB_Reset => t_sig_A_nMB_Reset,
	A_VG_ID => t_sig_A_VG_ID,
	A_RDnWR => t_sig_A_RDnWR,
	A_Sub_Adr => t_sig_A_Sub_Adr,
	F_TCXO_In => t_sig_F_TCXO_In,
	A_VG_K3_INP => t_sig_A_VG_K3_INP,
	nSEL_LED_GRP => t_sig_nSEL_LED_GRP,
	A_VG_K1_INP => t_sig_A_VG_K1_INP,
	A_VG_K0_INP => t_sig_A_VG_K0_INP,
	A_VG_K2_INP => t_sig_A_VG_K2_INP,
	A_GR1_APK_ID => t_sig_A_GR1_APK_ID,
	A_VG_Log_ID => t_sig_A_VG_Log_ID,
	A_GR0_APK_ID => t_sig_A_GR0_APK_ID,
	A_GR1_16BIT => t_sig_A_GR1_16BIT,
	A_GR0_16BIT => t_sig_A_GR0_16BIT,
	A_VG_K3_MOD => t_sig_A_VG_K3_MOD,
	A_VG_K1_MOD => t_sig_A_VG_K1_MOD,
	A_VG_K2_MOD => t_sig_A_VG_K2_MOD,
	A_VG_K0_MOD => t_sig_A_VG_K0_MOD,
	nPowerup_Led => t_sig_nPowerup_Led,
	nExt_Data_En => t_sig_nExt_Data_En,
	nDT_Led => t_sig_nDT_Led,
	nSkal_OK_Led => t_sig_nSkal_OK_Led,
	nID_OK_Led => t_sig_nID_OK_Led,
	A_nK0_ID_EN => t_sig_A_nK0_ID_EN,
	A_nK1_ID_EN => t_sig_A_nK1_ID_EN,
	A_nK2_ID_EN => t_sig_A_nK2_ID_EN,
	A_nK3_ID_EN => t_sig_A_nK3_ID_EN,
	A_nINTERLOCKA => t_sig_A_nINTERLOCKA,
	A_nINTERLOCKB => t_sig_A_nINTERLOCKB,
	A_nSRQA => t_sig_A_nSRQA,
	A_nSRQB => t_sig_A_nSRQB,
	A_nDRQB => t_sig_A_nDRQB,
	A_nDRQA => t_sig_A_nDRQA,
	A_nDTACKA => t_sig_A_nDTACKA,
	A_nDTACKB => t_sig_A_nDTACKB,
	nMaster_Clk_ENA => t_sig_nMaster_Clk_ENA,
	nSlave_Clk_ENA => t_sig_nSlave_Clk_ENA,
	nIndepend_Clk_Ena => t_sig_nIndepend_Clk_Ena,
	nK0_SWITCH_ENA => t_sig_nK0_SWITCH_ENA,
	nK1_SWITCH_ENA => t_sig_nK1_SWITCH_ENA,
	nK2_SWITCH_ENA => t_sig_nK2_SWITCH_ENA,
	nK3_SWITCH_ENA => t_sig_nK3_SWITCH_ENA,
	Loader_WRnRD => t_sig_Loader_WRnRD,
	INIT_DONE => t_sig_INIT_DONE,
	LOAD_OK => t_sig_LOAD_OK,
	LOAD_ERROR => t_sig_LOAD_ERROR,
	RELOAD => t_sig_RELOAD,
--	chopper_clk => 	t_sig_chopper_clk,
	A_Master_Clk_Out => t_sig_A_Master_Clk_Out,
	A_nGR0_ID_SEL => t_sig_A_nGR0_ID_SEL,
	A_nGR1_ID_SEL => t_sig_A_nGR1_ID_SEL,
	A_LA_CLK => t_sig_A_LA_CLK,
	A_nMANUAL_RES => t_sig_A_nMANUAL_RES,
	nLED => t_sig_nLED,
	nLED_Skal => t_sig_nLED_Skal,
	A_I2C_SDA => t_sig_A_I2C_SDA,
	A_I2C_SCL => t_sig_A_I2C_SCL,
	A_K3_D => t_sig_A_K3_D,
	A_K2_D => t_sig_A_K2_D,
	A_Mod_Data => t_sig_A_Mod_Data,
	A_AUX_A => t_sig_A_AUX_A,
	A_AUX_B => t_sig_A_AUX_B,
	A_AUX_C => t_sig_A_AUX_C,
	A_Bus_IO => t_sig_A_Bus_IO,
	A_K0_D => t_sig_A_K0_D,
	A_nK0_CTRL => t_sig_A_nK0_CTRL,
	A_K1_D => t_sig_A_K1_D,
	A_nK1_CTRL => t_sig_A_nK1_CTRL,
	A_nK2_CTRL => t_sig_A_nK2_CTRL,
	A_nK3_CTRL => t_sig_A_nK3_CTRL,
	A_Test => t_sig_A_Test,
	A_VG_IO_Res => t_sig_A_VG_IO_Res,
	Loader_DB => t_sig_Loader_DB,
	Loader_Misc => t_sig_Loader_Misc,
	TP => t_sig_TP
);


Mux_In_K0_D(0) <= t_sig_A_K3_D(0) after 5 us;  	--HSI_ACT <= Chop_HSI_On
Mux_In_K0_D(1) <= t_sig_A_K3_D(6) after 5 us;	--HLI-ACT <= Chop_HLI_On



P_TCXO: Process
	begin
		loop
	       t_sig_F_TCXO_In <= NOT t_sig_F_TCXO_In;
	       wait for 25 ns;
		end loop;
	end process P_TCXO;

P_chopper_clk: Process
	begin
		loop
	       t_sig_chopper_clk <= NOT t_sig_chopper_clk;
	       wait for 25 ns;
		end loop;
	end process P_chopper_clk;

P_K3_ID: PROCESS (t_sig_A_nK3_ID_EN, t_sig_A_nGR1_ID_SEL, t_sig_A_VG_K3_INP, Mux_In_K3_D)
	BEGIN
	IF ((t_sig_A_VG_K3_INP = '1' AND t_sig_A_nK3_ID_EN = '1') OR (t_sig_A_VG_K3_INP = '0' AND t_sig_A_nK3_ID_EN = '0')) AND (t_sig_A_nGR1_ID_SEL /= '0') THEN
		t_sig_A_K3_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((t_sig_A_VG_K3_INP = '1' AND t_sig_A_nK3_ID_EN = '1') OR (t_sig_A_VG_K3_INP = '0' AND t_sig_A_nK3_ID_EN = '0')) AND (t_sig_A_nGR1_ID_SEL = '0') THEN
		t_sig_A_K3_D <= X"ED30";
	ELSIF t_sig_A_VG_K3_INP = '1' THEN
		t_sig_A_K3_D <=  Mux_In_K3_D;
	ELSE
		t_sig_A_K3_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;

P_K2_ID: PROCESS (t_sig_A_nK2_ID_EN, t_sig_A_nGR1_ID_SEL, t_sig_A_VG_K2_INP, Mux_In_K2_D)
	BEGIN
	IF ((t_sig_A_VG_K2_INP = '1' AND t_sig_A_nK2_ID_EN = '1') OR (t_sig_A_VG_K2_INP = '0' AND t_sig_A_nK2_ID_EN = '0')) AND (t_sig_A_nGR1_ID_SEL /= '0') THEN
		t_sig_A_K2_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((t_sig_A_VG_K2_INP = '1' AND t_sig_A_nK2_ID_EN = '1') OR (t_sig_A_VG_K2_INP = '0' AND t_sig_A_nK2_ID_EN = '0')) AND (t_sig_A_nGR1_ID_SEL = '0') THEN
		t_sig_A_K2_D <= X"ED30";
	ELSIF t_sig_A_VG_K2_INP = '1' THEN
		t_sig_A_K2_D <=  Mux_In_K2_D;
	ELSE
		t_sig_A_K2_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;
	
P_K1_ID: PROCESS (t_sig_A_nK1_ID_EN, t_sig_A_nGR0_ID_SEL, t_sig_A_VG_K1_INP, Mux_In_K1_D)
	BEGIN
	IF ((t_sig_A_VG_K1_INP = '1' AND t_sig_A_nK1_ID_EN = '1') OR (t_sig_A_VG_K1_INP = '0' AND t_sig_A_nK1_ID_EN = '0')) AND (t_sig_A_nGR0_ID_SEL /= '0') THEN
		t_sig_A_K1_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((t_sig_A_VG_K1_INP = '1' AND t_sig_A_nK1_ID_EN = '1') OR (t_sig_A_VG_K1_INP = '0' AND t_sig_A_nK1_ID_EN = '0')) AND (t_sig_A_nGR0_ID_SEL = '0') THEN
		t_sig_A_K1_D <= X"D047";
	ELSIF t_sig_A_VG_K1_INP = '1' THEN
		t_sig_A_K1_D <=  Mux_In_K1_D;
	ELSE
		t_sig_A_K1_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;

	
P_K0_ID: PROCESS (t_sig_A_nK0_ID_EN, t_sig_A_nGR0_ID_SEL, t_sig_A_VG_K0_INP, Mux_In_K0_D)
	BEGIN
	IF ((t_sig_A_VG_K0_INP = '1' AND t_sig_A_nK0_ID_EN = '1') OR (t_sig_A_VG_K0_INP = '0' AND t_sig_A_nK0_ID_EN = '0')) AND (t_sig_A_nGR0_ID_SEL /= '0') THEN
		t_sig_A_K0_D <= "LLLLLLLLLLLLLLLL";
	ELSIF ((t_sig_A_VG_K0_INP = '1' AND t_sig_A_nK0_ID_EN = '1') OR (t_sig_A_VG_K0_INP = '0' AND t_sig_A_nK0_ID_EN = '0')) AND (t_sig_A_nGR0_ID_SEL = '0') THEN
		t_sig_A_K0_D <= X"D040";
	ELSIF t_sig_A_VG_K0_INP = '1' THEN
		t_sig_A_K0_D <=  Mux_In_K0_D;
	ELSE
		t_sig_A_K0_D <=  "LLLLLLLLLLLLLLLL";
	END IF; 
	END PROCESS;


P_Dtack_Test: PROCESS (t_sig_A_nDS)
	BEGIN
	IF t_sig_A_nDS'EVENT AND t_sig_A_nDS = '1' THEN
		IF t_sig_A_nDTACKA = '1' THEN
			ASSERT FALSE REPORT "Kein Dtack " SEVERITY Failure;
		END IF;
	END IF; 
	END PROCESS;
	
P_Data_Stable_Test: PROCESS (t_sig_A_Mod_Data)
	BEGIN
		IF ((t_sig_A_nDTACKA = '0') OR (t_sig_A_nDTACKB = '0')) AND t_sig_A_RDnWR = '1' THEN
			ASSERT FALSE REPORT "Datenbus nicht stabil" SEVERITY Failure;
		END IF;
	END PROCESS;
	
P_MB_RD_Memory: PROCESS (t_sig_A_RDnWR, t_sig_A_nDTACKA, t_sig_A_nDTACKB, t_sig_A_Sub_Adr(0))
	BEGIN
	
		IF ((t_sig_A_nDTACKA = '0') OR (t_sig_A_nDTACKB = '0')) AND (t_sig_A_RDnWR = '1') THEN
			IF t_sig_A_Sub_Adr(0) = '0' THEN
				S_Last_Rd_Memory(15 DOWNTO 8) <= t_sig_A_Mod_Data;
			ELSIF t_sig_A_Sub_Adr(0) = '1' THEN
				S_Last_Rd_Memory(7 DOWNTO 0) <= t_sig_A_Mod_Data;
			END IF;
		END IF;
	END PROCESS;

check_interlock: process(t_sig_A_K3_D(5), t_sig_A_K3_D(10))
begin
	if t_sig_A_K3_D(5) = '0' or t_sig_A_K3_D(10) = '0' then
		ASSERT FALSE REPORT "Interlock HSI or HLI occured" SEVERITY Failure;
	end if;
end process;

check_chopper_version: process(t_sig_A_Sub_Adr, t_sig_A_Mod_Data, t_sig_A_nDTACKA)
begin
	if t_sig_A_Sub_Adr = C_Global_Status_RD + 1 and t_sig_A_nDTACKA = '0' then
		if t_sig_A_Mod_Data < x"11" then
			ASSERT FALSE REPORT "Chopper Version failure" SEVERITY Failure;
		end if;
	end if;
end process;

--check_timestamps: process(t_sig_A_Sub_Adr, t_sig_A_Mod_Data, t_sig_A_nDTACKA)
--begin
--	if t_sig_A_Sub_Adr = C_HSI_act_pos_edge_RD and t_sig_A_nDTACKA = '0' then
--		if t_sig_A_Mod_Data(t_sig_A_Mod_Data'high) /= '1' then
--			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
--			REPORT "Timestamp ungueltig!";
--		end if;
--		if t_sig_A_Mod_Data(0) = '1' then
--			ASSERT FALSE REPORT "Soll-Signal zu frueh" SEVERITY Failure;
--		end if;
--		if t_sig_A_Mod_Data(1) = '1' then
--			ASSERT FALSE REPORT "Signal unterbrochen" SEVERITY Failure;
--		end if;
--	end if;
--	if t_sig_A_Sub_Adr = C_HSI_neg_edge_RD and t_sig_A_nDTACKA = '0' then
--		if t_sig_A_Mod_Data(t_sig_A_Mod_Data'high) /= '1' then
--			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
--			REPORT "Timestamp ungueltig!";
--		end if;
--	end if;
--	if t_sig_A_Sub_Adr = C_HSI_act_neg_edge_RD and t_sig_A_nDTACKA = '0' then
--		if t_sig_A_Mod_Data(t_sig_A_Mod_Data'high) /= '1' then
--			--ASSERT FALSE REPORT "Timestamp ungueltig!" SEVERITY Failure;
--			REPORT "Timestamp ungueltig!";
--		end if;
--	end if;
--end process;
            	
                                         
always : PROCESS                                              
	BEGIN

	t_sig_A_VG_A <= "11000";
	t_sig_A_VG_ID <= conv_std_logic_vector(39,t_sig_A_VG_ID'length);
	t_sig_A_VG_Log_ID <= "000001";
	t_sig_A_GR0_APK_ID <= '1';
	t_sig_A_GR1_APK_ID <= '1';
	t_sig_A_GR0_16BIT	<= '1';
	t_sig_A_GR1_16BIT <= '1';
	t_sig_A_VG_K0_INP <= '1';
	t_sig_A_VG_K1_INP <= '1';
	t_sig_A_VG_K0_MOD <= "11";
	t_sig_A_VG_K1_MOD <= "11";
	
	t_sig_nSEL_LED_GRP <= "10";
	
	t_sig_Loader_DB <= "HLHL";
	t_sig_Loader_Misc <= "LLLL";
	
	t_sig_A_VG_K2_INP <= '0';
	t_sig_A_VG_K3_INP <= '0';	
	t_sig_A_VG_K2_MOD <= "11";
	t_sig_A_VG_K3_MOD <= "11";
	
	
	--Mux_In_K0_D <= x"0000";
	Mux_In_K1_D <= x"FFFF";	-- Strahlalarm Input (Activ Low)
	--Mux_In_K2_D <= X"0000";
	--Mux_In_K3_D <= X"0000";
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011";
	
	
	
	
	t_sig_A_nMB_Reset <= '1';
	wait for 100 ns;
	t_sig_A_nMB_Reset <= '0';
	wait for 100 ns;
	t_sig_A_nMB_Reset <= '1';
	wait for 10 ns;	
	t_sig_A_VG_Log_ID <= "000000";
	wait for 10 ns;	
	t_sig_A_VG_Log_ID <= "000001";
	wait for 10 ns;	
	t_sig_A_VG_Log_ID <= "000011";
	wait for 10 ns;
	t_sig_A_VG_Log_ID <= "000010";
	wait for 10 ns;	
	t_sig_A_VG_Log_ID <= "000001";
			

	-- Chopper_HLI (active high)
	Mux_In_K0_D(15 downto 2) <= x"000" & "00";  

	
	REPORT "Test 1: Anforderung vor Strahlpuls, laenger als 10us";
	
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test2: Anforderung vor Strahlpuls, kuerzer als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for  30 us;
	
	REPORT "Test3: Anforderung nach Strahlpuls, laenger als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00001"; -- kein Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test4: Anforderung vor Strahlpuls beendet";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 40 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 30 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test5: Anforderung vor Strahlpuls, waehrend Strahlpus unterbrochen";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00001"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; --  Strahl anfordern
	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	wait for 40 us;

	REPORT "Test mit UU einzeln";
	
	REPORT "Test 1: Anforderung vor Strahlpuls, laenger als 10us";
	
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00010"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test2: Anforderung vor Strahlpuls, kuerzer als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00010"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for  30 us;
	
	REPORT "Test3: Anforderung nach Strahlpuls, laenger als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00010"; -- kein Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test4: Anforderung vor Strahlpuls beendet";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 40 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00010"; -- kein Strahl anfordern

	
	wait for 30 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test5: Anforderung vor Strahlpuls, waehrend Strahlpus unterbrochen";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00010"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; --  Strahl anfordern
	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	
	wait for 10 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	wait for 40 us;
	
	REPORT "Test mit beiden Anforder Signalen";
	
	REPORT "Test 1: Anforderung vor Strahlpuls, laenger als 10us";
	
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test2: Anforderung vor Strahlpuls, kuerzer als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for  30 us;
	
	REPORT "Test3: Anforderung nach Strahlpuls, laenger als 10 us nach Strahlbeginn";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- kein Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 55 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern

	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test4: Anforderung vor Strahlpuls beendet";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 40 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- kein Strahl anfordern

	
	wait for 30 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	REPORT "Test5: Anforderung vor Strahlpuls, waehrend Strahlpus unterbrochen";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- Strahl anfordern
	wait for 8*zeit_10ns;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 70 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- kein Strahl anfordern

	
	wait for 10 us;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; --  kein Strahl anfordern
	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	

	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	wait for 40 us;
	
	
	REPORT "Test6: Anforderung vor Strahlpuls aktiv und wieder inaktiv";
	
	-- !Interlock_HSI (4) & !Interlock_HLI (5) & HLI_ALV (3) & HSI_ALV(2)
	MB_Cycle("11000", C_Strahlweg_Reg_RW, Wr, X"000C", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Global_Status_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;

	

	
	
	wait for 10 us;
	
	Mux_In_K0_D(15 downto 2) <= x"F00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15) & Strahl_HLI(12) & Strahl_HSI(14)
	
	wait for 40 us;
	

	
	Mux_In_K0_D(15 downto 2) <= x"A00" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)
	wait for 8*zeit_10ns;
	
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- kein Strahl anfordern
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00011"; -- Strahl anfordern
	wait for 5 us;
	--Off_Anforderung_In (0) und Off_UU_In (1)
	t_sig_A_Bus_IO <= "00000"; -- kein Strahl anfordern
	wait for 10 us;

	Mux_In_K0_D(15 downto 2) <= x"000" & "00"; -- Chopper_HLI(13) & Chopper_HLI(15)

	wait for 20 us;
	
	MB_Cycle("11000", C_HSI_act_pos_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_HSI_act_neg_edge_RD, Rd, X"0000", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	MB_Cycle("11000", C_Interlock_Reg_RW, Wr, X"FFFF", C_Std_DS, C_Std_Gap, t_sig_A_A, t_sig_A_Sub_Adr, t_sig_A_MOD_DATA, t_sig_A_RDnWR, t_sig_A_nDS);
	wait for 8*zeit_10ns;
	
	wait for 30 us;
	
	

	ASSERT FALSE REPORT "Testbench beendet" SEVERITY Failure;
	
	WAIT;                                                        
	END PROCESS always;
                                          
--END Kicker_Vers_1_Rev_0_arch;
END chopper_arch;
