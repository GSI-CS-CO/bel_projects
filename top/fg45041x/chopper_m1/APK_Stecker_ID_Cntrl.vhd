LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;


ENTITY APK_Stecker_ID_Cntrl IS
	GENERIC (
			K3_APK_ST_ID	: INTEGER := 65535;
			K2_APK_ST_ID	: INTEGER := 65535;
			K1_APK_ST_ID	: INTEGER := 65535;
			K0_APK_ST_ID	: INTEGER := 65535;
			ST_160_Pol		: INTEGER := 1;
			Clk_in_Hz		: INTEGER := 300000000;
			Wait_in_ns		: INTEGER := 200;
			Use_LPM			: INTEGER := 0
			);
	port
	(
		Start_ID_Cntrl	: IN		STD_LOGIC;
		Update_Apk_ID	: IN		STD_LOGIC;
		Reset			: IN		STD_LOGIC;
		clk				: IN		STD_LOGIC;
		K3_K2_Skal		: IN		STD_LOGIC_VECTOR (7 DOWNTO 0);
		A_K3_D			: IN		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_K2_D			: IN		STD_LOGIC_VECTOR (15 DOWNTO 0);
		DB_K3_INP		: IN		STD_LOGIC;
		DB_K2_INP		: IN		STD_LOGIC;
		DB_GR1_APK_ID	: IN		STD_LOGIC;						-- muss das gelatchte Signal sein
		K1_K0_Skal		: IN		STD_LOGIC_VECTOR (7 DOWNTO 0);
		A_K1_D			: IN		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_K0_D			: IN		STD_LOGIC_VECTOR (15 DOWNTO 0);
		DB_K1_INP		: IN		STD_LOGIC;
		DB_K0_INP		: IN		STD_LOGIC;
		DB_GR0_APK_ID	: IN		STD_LOGIC;						-- muss das gelatchte Signal sein
		ID_Cntrl_Done	: OUT		STD_LOGIC;
		APK_ST_ID_OK	: OUT		STD_LOGIC;
		La_Ena_Skal_In	: OUT		STD_LOGIC;
		La_Ena_Port_In	: OUT		STD_LOGIC;
		K3_ID			: OUT		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_nK3_ID_En		: OUT		STD_LOGIC;
		K2_ID			: OUT		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_nK2_ID_En		: OUT		STD_LOGIC;
		A_nGR1_ID_Sel	: OUT		STD_LOGIC;
		K1_ID			: OUT		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_nK1_ID_En		: OUT		STD_LOGIC;
		K0_ID			: OUT		STD_LOGIC_VECTOR (15 DOWNTO 0);
		A_nK0_ID_En		: OUT		STD_LOGIC;
		A_nGR0_ID_Sel	: OUT		STD_LOGIC
	);
END APK_Stecker_ID_Cntrl;



ARCHITECTURE Arch_APK_Stecker_ID_Cntrl OF APK_Stecker_ID_Cntrl IS


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


	COMPONENT lpm_counter
		GENERIC (
				lpm_width		: NATURAL;
				lpm_type		: STRING;
				lpm_direction	: STRING;
				lpm_svalue		: STRING
				);
		PORT(
			clock	: IN STD_LOGIC ;
			cnt_en	: IN STD_LOGIC := '1';
			q		: OUT STD_LOGIC_VECTOR (lpm_width-1 DOWNTO 0);
			sset	: IN STD_LOGIC
			);
	END COMPONENT;

	CONSTANT	Clk_in_ps		: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	CONSTANT	C_Wait_Cnt		: INTEGER	:= Wait_in_ns * 1000 / Clk_in_ps;

	SIGNAL		S_Wait_Cnt			: STD_LOGIC_VECTOR(How_many_Bits(C_Wait_Cnt) DOWNTO 0);
	SIGNAL		S_Ld_Wait			: STD_LOGIC;
	SIGNAL		S_Wait_Fin			: STD_LOGIC_VECTOR (1 DOWNTO 0);
	SIGNAL		S_Sel_Wait_Cnt		: STD_LOGIC;

	SIGNAL		S_Start_Edge		: STD_LOGIC;
	SIGNAL		S_Start_ID_Sync		: STD_LOGIC_VECTOR(1 DOWNTO 0);

	SIGNAL		S_K3_ID				: STD_LOGIC_VECTOR (15 DOWNTO 0);
	SIGNAL		S_K2_ID				: STD_LOGIC_VECTOR (15 DOWNTO 0);
	SIGNAL		S_K1_ID				: STD_LOGIC_VECTOR (15 DOWNTO 0);
	SIGNAL		S_K0_ID				: STD_LOGIC_VECTOR (15 DOWNTO 0);

	SIGNAL		S_La_Ena_ID			: STD_LOGIC;

	SIGNAL		S_ID_Cntrl_Done		: STD_LOGIC;


	TYPE	T_ID_SM	IS	(
						ID_Idle,
						GR_ID_Sel,
						ID_En,
						GR_ID_Dis,
						ID_Done
						);

	SIGNAL	S_ID_SM	:	T_ID_SM;


BEGIN

P_Edge: PROCESS (clk, Reset)
	BEGIN
		IF Reset = '1' THEN
			S_Start_Edge <= '0';
			S_Start_ID_Sync <= "00";
		ELSIF rising_edge(clk) THEN
			S_Start_ID_Sync <= (S_Start_ID_Sync(0) & Start_ID_Cntrl);
			IF ((S_Start_ID_Sync(1) = '0') AND (S_Start_ID_Sync(0) = '1')) OR Update_Apk_ID = '1' THEN
				S_Start_Edge <= '1';
			ELSE
				S_Start_Edge <= '0';
			END IF;
		END IF;
	END PROCESS P_Edge;



wait_with_lpm: IF Use_LPM = 1 GENERATE -----------------------------------------------

BEGIN

wait_cnt : lpm_counter
	GENERIC MAP (
				lpm_width	=> S_Wait_Cnt'LENGTH,
				lpm_type	=> "LPM_COUNTER",
				lpm_direction => "DOWN",
				lpm_svalue	=> integer'image(C_Wait_Cnt)
				)
	PORT MAP(
			clock	=> clk,
			sset	=> S_Ld_Wait,
			cnt_en	=> S_Sel_Wait_Cnt,
			q		=> S_Wait_Cnt
			);

END GENERATE wait_with_lpm; ----------------------------------------------------------


wait_without_lpm: IF Use_LPM = 0 GENERATE --------------------------------------------

BEGIN
P_Wait:	PROCESS (clk, Reset)
	BEGIN
		IF Reset = '1' THEN
			S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'LENGTH);
		ELSIF rising_edge(clk) THEN
			IF S_Ld_Wait = '1' THEN
				S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'LENGTH);
			ELSIF S_Sel_Wait_Cnt = '1' THEN
				S_Wait_Cnt <= S_Wait_Cnt - 1;
			ELSE
				S_Wait_Cnt <= S_Wait_Cnt;
			END IF;
		END IF;
	END PROCESS;

END GENERATE wait_without_lpm; -------------------------------------------------------


P_ID_SM:	PROCESS (clk, Reset)
	BEGIN
	    IF Reset = '1' THEN
			S_ID_SM <= ID_Idle;
			S_ID_Cntrl_Done <= '0';

	    ELSIF rising_edge(clk) THEN

			S_Wait_Fin <= (S_Wait_Fin(0) & S_Wait_Cnt(S_Wait_Cnt'LEFT)); -- Schiebe-Reg. wird zur Flankenerkennung genutzt.

			S_Ld_Wait <= '0';
			S_La_Ena_ID <= '0';
			S_Sel_Wait_Cnt <= '0';

			CASE S_ID_SM IS

	                        WHEN ID_Idle =>
					S_Ld_Wait <= '1';
					IF S_Start_Edge = '1' THEN
						S_ID_SM <= GR_ID_Sel;
					END IF;

				WHEN GR_ID_Sel =>
					S_ID_Cntrl_Done <= '0';
					S_Sel_Wait_Cnt <= '1';
					IF S_Wait_Fin = "01" THEN
						S_Ld_Wait <= '1';
					ElSIF S_Wait_Fin = "10" THEN
						S_ID_SM <= ID_En;
					END IF;

				WHEN ID_En =>
					S_Sel_Wait_Cnt <= '1';
					IF S_Wait_Fin = "01" THEN
						S_Ld_Wait <= '1';
						S_La_Ena_ID <= '1';
					ElSIF S_Wait_Fin = "10" THEN
						S_ID_SM <= GR_ID_Dis;
					END IF;

				WHEN GR_ID_Dis =>
					S_Sel_Wait_Cnt <= '1';
					IF S_Wait_Fin = "01" THEN
						S_Ld_Wait <= '1';
					ElSIF S_Wait_Fin = "10" THEN
						S_ID_SM <= ID_Done;
					END IF;

				WHEN ID_Done =>
					S_Sel_Wait_Cnt <= '1';
					IF S_Wait_Fin = "01" THEN
						S_ID_Cntrl_Done <= '1';
						S_ID_SM <= ID_Idle;
					END IF;

			END CASE;

		END IF;
	END PROCESS P_ID_SM;


P_ID_La: PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF S_La_Ena_ID = '1' THEN
				IF DB_GR1_APK_ID = '1' THEN
					S_K3_ID <= A_K3_D;
					S_K2_ID <= A_K2_D;
				ELSE
					S_K3_ID <= (X"00" & K3_K2_Skal);
					S_K2_ID <= (X"00" & K3_K2_Skal);
				END IF;
				IF DB_GR0_APK_ID = '1' THEN
					S_K1_ID <= A_K1_D;
					S_K0_ID <= A_K0_D;
				ELSE
					S_K1_ID <= (X"00" & K1_K0_Skal);
					S_K0_ID <= (X"00" & K1_K0_Skal);
				END IF;
			END IF;
		END IF;
	END PROCESS P_ID_La;

K3_ID <= S_K3_ID;
K2_ID <= S_K2_ID;
K1_ID <= S_K1_ID;
K0_ID <= S_K0_ID;


P_ID_Compare: PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF St_160_Pol = 0 THEN
				IF (S_K1_ID = conv_std_logic_vector(K1_APK_ST_ID, 16)) AND (S_K0_ID = conv_std_logic_vector(K0_APK_ST_ID, 16)) THEN
					APK_ST_ID_OK <= S_ID_Cntrl_Done;
				ELSE
					APK_ST_ID_OK <= '0';
				END IF;
			ELSE
				IF  (S_K3_ID =  conv_std_logic_vector(K3_APK_ST_ID, 16)) AND (S_K2_ID = conv_std_logic_vector(K2_APK_ST_ID, 16))
				  AND (S_K1_ID = conv_std_logic_vector(K1_APK_ST_ID, 16)) AND (S_K0_ID = conv_std_logic_vector(K0_APK_ST_ID, 16)) THEN
					APK_ST_ID_OK <= S_ID_Cntrl_Done;
				ELSE
					APK_ST_ID_OK <= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS P_ID_Compare;

La_Ena_Skal_In <= '1' WHEN (S_ID_SM /= ID_Idle) ELSE '0';
La_Ena_Port_In <= '1' WHEN (S_ID_SM /= ID_Idle) ELSE '0';

A_nGR1_ID_Sel <= '0' WHEN (S_ID_SM = ID_En) ELSE '1';												-- ID-Strobe für K3+K2
A_nGR0_ID_Sel <= '0' WHEN (S_ID_SM = ID_En) ELSE '1';												-- ID-Strobe für K1+K0
A_nK3_ID_En <= DB_K3_INP WHEN NOT ((S_ID_SM = ID_Idle) OR (S_ID_SM = ID_Done)) ELSE NOT DB_K3_INP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK2_ID_En <= DB_K2_INP WHEN NOT ((S_ID_SM = ID_Idle) OR (S_ID_SM = ID_Done)) ELSE NOT DB_K2_INP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK1_ID_En <= DB_K1_INP WHEN NOT ((S_ID_SM = ID_Idle) OR (S_ID_SM = ID_Done)) ELSE NOT DB_K1_INP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK0_ID_En <= DB_K0_INP WHEN NOT ((S_ID_SM = ID_Idle) OR (S_ID_SM = ID_Done)) ELSE NOT DB_K0_INP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!

ID_Cntrl_Done <= '1' WHEN S_ID_Cntrl_Done = '1' AND S_ID_SM = ID_Idle ELSE '0';

END;
