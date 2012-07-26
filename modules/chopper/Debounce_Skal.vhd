LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Debounce_Skal IS
	GENERIC	(
			Clk_in_Hz		: INTEGER := 20000000;
			DB_in_ns		: INTEGER := 200;
			Test 			: INTEGER := 0
			);
	PORT
	(
		A_VG_K1_INP			: IN STD_LOGIC;
		A_VG_K0_INP			: IN STD_LOGIC;
		A_GR0_APK_ID		: IN STD_LOGIC;
		A_GR0_16BIT			: IN STD_LOGIC;
		A_VG_K1_MOD			: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
		A_VG_K0_MOD			: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
		A_VG_K3_INP			: IN STD_LOGIC;
		A_VG_K2_INP			: IN STD_LOGIC;
		A_GR1_APK_ID		: IN STD_LOGIC;
		A_GR1_16BIT			: IN STD_LOGIC;
		A_VG_K3_MOD			: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
		A_VG_K2_MOD			: IN STD_LOGIC_VECTOR(1 DOWNTO 0);
		Logic				: IN STD_LOGIC_VECTOR(5 DOWNTO 0);
		Latch_Inputs		: IN STD_LOGIC := '0';
		Reset				: IN STD_LOGIC;
		clk					: IN STD_LOGIC;
		K1_K0_Skal			: OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		DB_Mod_Skal			: OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		DB_K1_INP			: OUT STD_LOGIC;
		DB_K0_INP			: OUT STD_LOGIC;
		DB_GR0_APK_ID		: OUT STD_LOGIC;
		K3_K2_Skal			: OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		DB_160_Skal			: OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		DB_K3_INP			: OUT STD_LOGIC;
		DB_K2_INP			: OUT STD_LOGIC;
		DB_GR1_APK_ID		: OUT STD_LOGIC;
		DB_Logic			: OUT STD_LOGIC_VECTOR(5 DOWNTO 0);
		DB_Valid			: OUT STD_LOGIC
	);
END Debounce_Skal;


ARCHITECTURE Arch_Debounce_Skal OF Debounce_Skal IS


    FUNCTION	How_many_Bits  (int: INTEGER) RETURN INTEGER IS

	VARIABLE i, tmp : INTEGER;

	BEGIN
		tmp		:= int;
		i		:= 0;
		WHILE tmp > 0 LOOP
			tmp := tmp / 2;
			i := i + 1;
		END LOOP;
		RETURN i;
	END How_many_bits;

	COMPONENT	Debounce
		GENERIC(
				DB_Cnt		: INTEGER := 3;	
				DB_Tst_Cnt	: INTEGER := 3;
				Test		: INTEGER := 0
				);
		PORT(
			DB_In		: IN	STD_LOGIC;
			Reset		: IN	STD_LOGIC;
			Clk			: IN	STD_LOGIC;
			DB_Out		: OUT	STD_LOGIC
			);
	END COMPONENT;
	
	CONSTANT	Clk_in_ns		: INTEGER	:= 1000000000 / Clk_in_Hz;
	CONSTANT	C_DB_Cnt		: INTEGER	:= DB_in_ns / Clk_in_ns;

	SIGNAL		S_VG_Mod_Skal	: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL		S_ST_160_Skal	: STD_LOGIC_VECTOR(7 DOWNTO 0);
		
	SIGNAL		S_DB_Logic		: STD_LOGIC_VECTOR(Logic'range);
	SIGNAL		S_DB_Mod_Skal	: STD_LOGIC_VECTOR(S_VG_Mod_Skal'range);
	SIGNAL		S_DB_160_Skal	: STD_LOGIC_VECTOR(S_ST_160_Skal'range);
	
BEGIN


P_Skal_La: PROCESS (Clk)

	BEGIN
	IF rising_edge(Clk) THEN
		IF Latch_Inputs = '0' THEN
			S_VG_Mod_Skal <= (	A_VG_K1_INP & A_VG_K0_INP & A_GR0_APK_ID & A_GR0_16BIT &
								A_VG_K1_MOD(1 DOWNTO 0) & A_VG_K0_MOD(1 DOWNTO 0)
							 );
			S_ST_160_Skal <= (	A_VG_K3_INP & A_VG_K2_INP &	A_GR1_APK_ID & A_GR1_16BIT &	
								A_VG_K3_MOD(1 DOWNTO 0) & A_VG_K2_MOD(1 DOWNTO 0)
							 );
		END IF;
	END IF;
	
END PROCESS P_Skal_La;

DB_LO: FOR i IN Logic'range GENERATE
	DBL: Debounce GENERIC MAP	(
								DB_Cnt		=> C_DB_Cnt,	
								DB_Tst_Cnt	=> 3,
								Test		=> 0
								)
					PORT MAP	(
								DB_In		=> Logic(i),
								Reset		=> '0',
								Clk			=> Clk,
								DB_Out		=> S_DB_Logic(i)
								);
	END GENERATE;

DB_Logic <= S_DB_Logic;

DB_SK: FOR i IN S_VG_Mod_Skal'range GENERATE
	DBS: Debounce GENERIC MAP	(
								DB_Cnt		=> C_DB_Cnt,	
								DB_Tst_Cnt	=> 3,
								Test		=> 0
								)
					PORT MAP	(
								DB_In		=> S_VG_Mod_Skal(i),
								Reset		=> '0',
								Clk			=> Clk,
								DB_Out		=> S_DB_Mod_Skal(i)
								);
	END GENERATE;

DB_Mod_Skal <= S_DB_Mod_Skal;
DB_K1_INP	<= S_DB_Mod_Skal(7);
DB_K0_INP	<= S_DB_Mod_Skal(6);
DB_GR0_APK_ID <=  S_DB_Mod_Skal(5);

K1_K0_Skal <= (	A_VG_K1_INP & A_VG_K0_INP & A_GR0_APK_ID & A_GR0_16BIT &
				A_VG_K1_MOD(1 DOWNTO 0) & A_VG_K0_MOD(1 DOWNTO 0)
			  );

DB_ST: FOR i IN S_ST_160_Skal'range GENERATE
	DBT: Debounce GENERIC MAP	(
								DB_Cnt		=> C_DB_Cnt,	
								DB_Tst_Cnt	=> 3,
								Test		=> 0
								)
					PORT MAP	(
								DB_In		=> S_ST_160_Skal(i),
								Reset		=> '0',
								Clk			=> Clk,
								DB_Out		=> S_DB_160_Skal(i)
								);
	END GENERATE;

DB_160_Skal <= S_DB_160_Skal;
DB_K3_INP <= S_DB_160_Skal(7);
DB_K2_INP <= S_DB_160_Skal(6);
DB_GR1_APK_ID <=  S_DB_160_Skal(5);

K3_K2_Skal <= (	A_VG_K3_INP & A_VG_K2_INP &	A_GR1_APK_ID & A_GR1_16BIT &	
				A_VG_K3_MOD(1 DOWNTO 0) & A_VG_K2_MOD(1 DOWNTO 0)
			   );

DB_V: Debounce GENERIC MAP	(
								DB_Cnt		=> C_DB_Cnt+2,	
								DB_Tst_Cnt	=> 3+2,
								Test		=> 0
								)
					PORT MAP	(
								DB_In		=> '1',
								Reset		=> Reset,
								Clk			=> Clk,
								DB_Out		=> DB_Valid
								);



END Arch_Debounce_Skal;
