LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Loader_MB IS
	GENERIC(
			CLK_in_Hz				: INTEGER := 50000000;
			Loader_Base_Adr			: INTEGER := 240	-- X"F0" --
			);
			
	PORT
	(
		Sub_Adr_La			: IN STD_LOGIC_VECTOR(7 downto 1);
		Data_In				: IN STD_LOGIC_VECTOR(15 downto 0);
		Extern_Wr_Activ		: IN STD_LOGIC;
		Extern_Wr_Fin		: IN STD_LOGIC;
		Extern_Rd_Activ		: IN STD_LOGIC;
		Extern_Rd_Fin		: IN STD_LOGIC;
		Powerup_Res			: IN STD_LOGIC;
		Clk					: IN STD_LOGIC;
		CTRL_LOAD			: IN  STD_LOGIC := 'L';
		CTRL_RES			: IN  STD_LOGIC := 'L';
		LOADER_DB			: INOUT  STD_LOGIC_VECTOR(3 downto 0);
		LOADER_WRnRD		: OUT  STD_LOGIC;
		RELOAD				: OUT  STD_LOGIC;
		LOAD_OK				: OUT  STD_LOGIC;
		LOAD_ERROR			: OUT  STD_LOGIC;
		INIT_DONE			: OUT  STD_LOGIC;
		Loader_Rd_Port		: OUT STD_LOGIC_VECTOR(15 downto 0);
		Rd_Loader_Activ		: OUT STD_LOGIC;
		Dtack_Loader_Reg	: OUT STD_LOGIC
	);
	
END Loader_mb;



ARCHITECTURE arch_Loader_MB OF Loader_MB IS

	CONSTANT 	C_Loader_Base_Adr	: STD_LOGIC_VECTOR(7 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Loader_Base_Adr, 8);
	SIGNAL		S_Rd_Loader			: STD_LOGIC;
	SIGNAL		S_Wr_Loader			: STD_LOGIC;
	
	SIGNAL		S_Dtack_Loader_Reg	: STD_LOGIC;

	SIGNAL		S_Sub_Adr_La		: STD_LOGIC_VECTOR(7 DOWNTO 1);		-- Das 'Sub_Adr_La' im Modul_Bus_Macro wird hoeher getaktet,
																		-- deshalb 'S_Sub_Adr_La'. Synchronisiert werden nur die
																		-- Signale 'Extern_Wr_Activ', 'Extern_Rd_Activ' gleichzeitg
																		-- werden die mehrere Takte langen Signale auf die Laenge von
																		-- einem Takt verkuerzt.
	SIGNAL		S_Data_In			: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- s. o.
	SIGNAL		S_Sync_Extern_Wr_Activ	: STD_LOGIC_VECTOR(2 DOWNTO 0);	-- Zur Synchronisation und Pulsgenerierung.
	SIGNAL		S_Sync_Extern_Rd_Activ	: STD_LOGIC_VECTOR(2 DOWNTO 0);	-- Zur Synchronisation und Pulsgenerierung.
	SIGNAL		S_clrn				: STD_LOGIC;

	SIGNAL		S_Q					: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Rd_Latch			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Rd_Activ			: STD_LOGIC;
																			
component K_EPCS_IF
	port
	(
		clk			: IN  STD_LOGIC;
		clrn		: IN  STD_LOGIC;
		we			: IN  STD_LOGIC;
		Adr			: IN  STD_LOGIC;
		rd			: IN  STD_LOGIC;
		CTRL_LOAD	: IN  STD_LOGIC;
		CTRL_RES	: IN  STD_LOGIC;
		D			: IN  STD_LOGIC_VECTOR(15 downto 0);
		LOADER_DB	: INOUT  STD_LOGIC_VECTOR(3 downto 0);
		LOADER_WRnRD : OUT  STD_LOGIC;
		RELOAD		: OUT  STD_LOGIC;
		LOAD_OK		: OUT  STD_LOGIC;
		LOAD_ERROR	: OUT  STD_LOGIC;
		INIT_DONE	: OUT  STD_LOGIC;
		Q			: OUT  STD_LOGIC_VECTOR(15 downto 0)
	);
end component;

BEGIN

S_clrn <= not Powerup_Res;

EPCS_IF_1:	K_EPCS_IF
PORT MAP
		(
		clk 			=> Clk,
		clrn 			=> S_clrn,
		we 				=> S_Wr_Loader,		-- nur ein Taktpuls
		rd				=> S_Rd_Loader,		-- nur ein Taktpuls
		Adr 			=> S_Sub_Adr_La(1),
		D				=> S_Data_In,
		CTRL_LOAD		=> CTRL_LOAD,
		CTRL_RES		=> CTRL_RES,
		LOADER_DB		=> LOADER_DB,
		LOADER_WRnRD	=> LOADER_WRnRD,
		RELOAD			=> RELOAD,
		LOAD_OK			=> LOAD_OK,
		LOAD_ERROR		=> LOAD_ERROR,
		INIT_DONE		=> INIT_DONE,
		Q				=> S_Q
		);
		

P_Loader_MB_Adr_Deco:	PROCESS (clk, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Rd_Loader			<= '0';
			S_Wr_Loader			<= '0';
			S_Dtack_Loader_Reg	<= '0';
			S_Sync_Extern_Wr_Activ <= (OTHERS => '0');
			S_Sync_Extern_Rd_Activ <= (OTHERS => '0');
			S_Data_In			<= (OTHERS => '0');
			S_Sub_Adr_La		<= (OTHERS => '0');
			S_Rd_Latch			<= (OTHERS => '0');
			S_Rd_Activ			<= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
		
			S_Rd_Loader			<= '0';
			S_Wr_Loader			<= '0';
			S_Dtack_Loader_Reg	<= '0';
			S_Sync_Extern_Wr_Activ <= (S_Sync_Extern_Wr_Activ(1 DOWNTO 0) & Extern_Wr_Activ);
			S_Sync_Extern_Rd_Activ <= (S_Sync_Extern_Rd_Activ(1 DOWNTO 0) & Extern_Rd_Activ);
			S_Data_In			<= Data_In;
			S_Sub_Adr_La		<= Sub_Adr_La;
			S_Rd_Activ			<= '0';
		
			CASE S_Sub_Adr_La(7 DOWNTO 2) IS
				WHEN C_Loader_Base_Adr(7 DOWNTO 2) =>
					IF S_Sync_Extern_Rd_Activ(2 DOWNTO 0) = "001" THEN
						S_Rd_Loader			<= '1';		-- nur ein Taktpuls
--						IF S_Sub_Adr_La(1) = '1' THEN
							S_Rd_Latch			<= S_Q;
--						END IF;
					END IF;
					IF S_Sync_Extern_Wr_Activ(2 DOWNTO 0) = "001" THEN
						S_Wr_Loader			<= '1';		-- nur ein Taktpuls
					END IF;
					IF S_Sync_Extern_Rd_Activ(2) = '1' OR S_Sync_Extern_Wr_Activ(2) = '1' THEN
						S_Dtack_Loader_Reg	<= '1';
					END IF;
					IF S_Sync_Extern_Rd_Activ(0) = '1' THEN
						S_Rd_Activ	<= '1';
					END IF;
				WHEN OTHERS =>
					S_Rd_Loader			<= '0';
					S_Wr_Loader			<= '0';
					S_Rd_Activ			<= '0';
					S_Dtack_Loader_Reg	<= '0';
			END CASE;
		END IF;
	END PROCESS P_Loader_MB_Adr_Deco;

--Loader_Rd_Port <= S_Rd_Latch WHEN S_Sub_Adr_La(1) = '1' ELSE S_Q;	-- Status-Lesen immer direkt
Loader_Rd_Port <= S_Rd_Latch;
	
Dtack_Loader_Reg <= S_Dtack_Loader_Reg AND (Extern_Wr_Activ OR Extern_Rd_Activ);

Rd_Loader_Activ <= S_Rd_Activ AND Extern_Rd_Activ;

END arch_Loader_MB;
