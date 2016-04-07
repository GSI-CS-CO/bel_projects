LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Loader_IFA IS
	GENERIC(
			Loader_CLK_in_Hz		: INTEGER := 50000000
			);
			
	PORT
	(
		Data_In				: IN STD_LOGIC_VECTOR(15 downto 0);
		
		WR_FWL_Data			: IN STD_LOGIC;
		WR_FWL_Ctrl			: IN STD_LOGIC;
		RD_FWL_Data			: IN STD_LOGIC;
		RD_FWL_STS			: IN STD_LOGIC;
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
	
END Loader_IFA;



ARCHITECTURE arch_Loader_IFA OF Loader_IFA IS

	SIGNAL		S_Rd_Loader			: STD_LOGIC;
	SIGNAL		S_Wr_Loader			: STD_LOGIC;
	
	SIGNAL		S_Dtack_Loader_Reg	: STD_LOGIC;

	SIGNAL		S_Data_In			: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- s. o.
	SIGNAL		S_Sync_Wr_Activ		: STD_LOGIC_VECTOR(3 DOWNTO 0);		-- Zur Synchronisation und Pulsgenerierung.
	SIGNAL		S_Sync_Rd_Activ		: STD_LOGIC_VECTOR(3 DOWNTO 0);		-- Zur Synchronisation und Pulsgenerierung.
	SIGNAL		S_clrn				: STD_LOGIC;

	SIGNAL		S_Q					: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Rd_Latch			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Rd_Activ			: STD_LOGIC;
	
	SIGNAL		S_Data_Access		: STD_LOGIC;
																			
component K_EPCS_IF
	GENERIC(Loader_CLK_in_Hz	: INTEGER);
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

S_Data_Access <= WR_FWL_Data OR RD_FWL_Data;

EPCS_IF_1:	K_EPCS_IF
GENERIC MAP(Loader_CLK_in_Hz => Loader_CLK_in_Hz)
PORT MAP
		(
		clk 			=> Clk,
		clrn 			=> S_clrn,
		we 				=> S_Wr_Loader,		-- nur ein Taktpuls
		rd				=> S_Rd_Loader,		-- nur ein Taktpuls
		Adr 			=> S_Data_Access,
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
		

P_Loader_IFA_Adr_Deco:	PROCESS (clk, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Rd_Loader			<= '0';
			S_Wr_Loader			<= '0';
			S_Dtack_Loader_Reg	<= '0';
			S_Sync_Wr_Activ	 	<= (OTHERS => '0');
			S_Sync_Rd_Activ		<= (OTHERS => '0');
			S_Data_In			<= (OTHERS => '0');
			S_Rd_Latch			<= (OTHERS => '0');
			S_Rd_Activ			<= '0';
			
		ELSIF clk'EVENT AND clk = '1' THEN
		
			S_Rd_Loader			<= '0';
			S_Wr_Loader			<= '0';
			S_Dtack_Loader_Reg	<= '0';
			S_Sync_Wr_Activ 	<= (S_Sync_Wr_Activ(2 DOWNTO 0) & (WR_FWL_Ctrl OR WR_FWL_Data));	-- Schieberegister
			S_Sync_Rd_Activ 	<= (S_Sync_Rd_Activ(2 DOWNTO 0) & (RD_FWL_Data OR RD_FWL_Sts));		-- Schieberegister
			S_Data_In			<= Data_In;
			S_Rd_Activ			<= '0';
		
			IF S_Sync_Rd_Activ(3 DOWNTO 0) = "0011" THEN
				S_Rd_Loader			<= '1';		-- nur ein Taktpuls
				S_Rd_Latch			<= S_Q;
			END IF;
			IF S_Sync_Wr_Activ(3 DOWNTO 0) = "0011" THEN
					S_Wr_Loader			<= '1';		-- nur ein Taktpuls
			END IF;
			IF S_Sync_Rd_Activ(S_Sync_Rd_Activ'high) = '1' OR S_Sync_Wr_Activ(S_Sync_Wr_Activ'high) = '1' THEN
				S_Dtack_Loader_Reg	<= '1';
			END IF;
			IF S_Sync_Rd_Activ(0) = '1' THEN
				S_Rd_Activ	<= '1';
			END IF;
		END IF;
	END PROCESS P_Loader_IFA_Adr_Deco;

Loader_Rd_Port <= S_Rd_Latch;
	
Dtack_Loader_Reg <= S_Dtack_Loader_Reg AND ((RD_FWL_Data OR RD_FWL_Sts) OR (RD_FWL_Data OR RD_FWL_Sts));

Rd_Loader_Activ <= S_Rd_Activ AND (RD_FWL_Data OR RD_FWL_Sts);

END arch_Loader_IFA;
