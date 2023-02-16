--TITLE "'Scalable Control Unit Bus Interface' Autor: W.Panschow, Stand: 17.08.09, Version = 1, Revision = 0";

--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--+ Beschreibung:																											+
--+																															+
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY SCU_Bus_Master IS

	GENERIC(
			CLK_in_Hz			: INTEGER := 100000000;
			Time_Out_in_ns		: INTEGER := 250;
			Sel_dly_in_ns		: INTEGER := 30;							-- delay to the I/O pins is not included
			Sel_release_in_ns	: INTEGER := 30;							-- delay to the I/O pins is not included
			D_Valid_to_DS_in_ns	: INTEGER := 30;							-- delay to the I/O pins is not included
			Timing_str_in_ns	: INTEGER := 80;							-- delay to the I/O pins is not included
			Test				: INTEGER RANGE 0 TO 1 := 0
			);

PORT(
	Wr_Data					: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- wite data to SCU_Bus, or internal FPGA register
	Rd_Data					: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- read data from SCU_Bus, or internal FPGA register
	Adr						: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);
	Slave_Nr				: IN		STD_LOGIC_VECTOR(3 DOWNTO 0);		-- 0x0 => internal access, 0x1 to 0xC => slave 1 to 12 access
	Start_Cycle				: IN		STD_LOGIC;							-- start data access from/to SCU_Bus
	Wr_Cycle				: IN		STD_LOGIC;							-- direction of SCU_Bus data access, write is active high.
	Rd_Cycle				: IN		STD_LOGIC;							
	Timing_In				: IN		STD_LOGIC_VECTOR(31 DOWNTO 0);
	Start_Timing_Cycle		: IN		STD_LOGIC;							-- start timing cycle to SCU_Bus
	clk						: IN		STD_LOGIC;
	Reset					: IN		STD_LOGIC;
	SCU_Bus_Access_Active	: OUT		STD_LOGIC;							-- active high signal: read or write access to SCUB is not finished
																			-- the access can be terminated bei an error, so look also to the access error bis in the status register
	Intr					: OUT		STD_LOGIC;							-- One or more slave interrupts, or internal Interrupts (like
																			-- SCU_Bus-busy or SCU_Bus-timeout) are active. Intr is ative high.
	SCUB_Data				: INOUT		STD_LOGIC_VECTOR(15 DOWNTO 0);
	nSCUB_DS				: OUT		STD_LOGIC;							-- SCU_Bus Data Strobe, low active.
	nSCUB_Dtack				: IN		STD_LOGIC;							-- SCU_Bus Data Acknowledge, low active.
	SCUB_Addr				: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- Address Bus of SCU_Bus
	SCUB_RDnWR				: OUT		STD_LOGIC;							-- Read/Write Signal of SCU_Bus. Read is active high.
																			-- Direction seen from this marco.
	nSCUB_SRQ_Slaves		: IN		STD_LOGIC_VECTOR(11 DOWNTO 0);		-- Input of service requests up to 12 SCU_Bus slaves, active low.
	nSCUB_Slave_Sel			: OUT		STD_LOGIC_VECTOR(11 DOWNTO 0);		-- Output select one or more of 12 SCU_Bus slaves, active low.
	nSCUB_Timing_Cycle		: OUT		STD_LOGIC;							-- Strobe to signal a timing cycle on SCU_Bus, active low.
	nSel_Ext_Data_Drv		: OUT		STD_LOGIC;							-- select for external data transceiver to the SCU_Bus, active low.
	nStart_DB_Trm			: OUT		STD_LOGIC;

	SCUB_Rd_Err_no_Dtack	: OUT		STD_LOGIC;
	SCUB_Rd_Fin				: OUT		STD_LOGIC;
	SCUB_Rd_active			: OUT		STD_LOGIC;
	SCUB_Wr_Err_no_Dtack	: OUT		STD_LOGIC;
	SCUB_Wr_Fin				: OUT		STD_LOGIC;
	SCUB_Wr_active			: OUT		STD_LOGIC;
	SCUB_Ti_Cyc_Err			: OUT		STD_LOGIC;
	SCUB_Ti_Fin				: OUT		STD_LOGIC;
	SCU_Wait_Request		: OUT		STD_LOGIC
	);
		
	FUNCTION set_vers_or_revi( vers_or_revi, Test: INTEGER) RETURN INTEGER IS
		BEGIN
			IF test > 0 THEN
				RETURN 0;
			ELSE
				RETURN vers_or_revi;
			END IF;
		END set_vers_or_revi;

	CONSTANT	C_SCUB_Version	: INTEGER RANGE 0 TO 255 := set_vers_or_revi(1, Test);		-- define the version of this macro
	CONSTANT	C_SCUB_Revision	: INTEGER RANGE 0 TO 255 := set_vers_or_revi(0, Test);		-- define the revision of this macro
	
	CONSTANT	Clk_in_ps			: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	CONSTANT	Clk_in_ns			: INTEGER	:= 1000000000 / Clk_in_Hz;


	FUNCTION set_ge_1  (a : INTEGER) RETURN INTEGER IS
		BEGIN
			IF a > 1 THEN
				RETURN a;
			ELSE
				RETURN 1;
			END IF;
		END set_ge_1;



	FUNCTION prod_or_test (production, test_data, test : INTEGER) RETURN INTEGER IS
		BEGIN
			IF Test = 1 THEN
				RETURN test_data;
			ELSE
				RETURN production;
			END IF;
		END prod_or_test;


	FUNCTION How_many_Bits (int: INTEGER) RETURN INTEGER IS

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


	FUNCTION return_max (a, b : INTEGER) RETURN INTEGER IS
		BEGIN
			IF a >= b THEN
				RETURN a;
			ELSE
				RETURN b;
			END IF;
		END return_max;


	CONSTANT	C_Sel_dly_cnt			: INTEGER	:= set_ge_1(Sel_dly_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Sel_dly_cnt			: STD_LOGIC_VECTOR(How_many_Bits(C_Sel_dly_cnt) DOWNTO 0);

	CONSTANT	C_Sel_release_cnt		: INTEGER	:= set_ge_1(Sel_release_in_ns * 1000 / Clk_in_ps)-2;	--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Sel_release_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Sel_release_cnt) DOWNTO 0);

	CONSTANT	C_Timing_str_cnt		: INTEGER	:= set_ge_1(Timing_str_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Timing_str_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Timing_str_cnt) DOWNTO 0);

	CONSTANT	C_D_Valid_to_DS_cnt		: INTEGER	:= set_ge_1(D_Valid_to_DS_in_ns * 1000 / Clk_in_ps)-2;	--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_D_Valid_to_DS_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_D_Valid_to_DS_cnt) DOWNTO 0);

	CONSTANT	C_time_out_cnt			: INTEGER	:= set_ge_1(time_out_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		s_time_out_cnt			: STD_LOGIC_VECTOR(How_many_Bits(C_time_out_cnt) DOWNTO 0);
	
	CONSTANT	C_Internal_Adr_Width	: INTEGER	:= 4;					-- define how many address bits are used to decode the internal FPGA-register
	CONSTANT	C_Status_Adr			: STD_LOGIC_VECTOR(C_Internal_Adr_Width-1 DOWNTO 0) := CONV_STD_LOGIC_VECTOR( 0, C_Internal_Adr_Width);
	CONSTANT	C_Vers_Revi_Adr			: STD_LOGIC_VECTOR(C_Internal_Adr_Width-1 DOWNTO 0) := CONV_STD_LOGIC_VECTOR( 1, C_Internal_Adr_Width);
	CONSTANT	C_SRQ_Ena_Adr			: STD_LOGIC_VECTOR(C_Internal_Adr_Width-1 DOWNTO 0) := CONV_STD_LOGIC_VECTOR( 2, C_Internal_Adr_Width);
	CONSTANT	C_SRQ_Active_Adr		: STD_LOGIC_VECTOR(C_Internal_Adr_Width-1 DOWNTO 0) := CONV_STD_LOGIC_VECTOR( 3, C_Internal_Adr_Width);
	CONSTANT	C_SRQ_In_Adr			: STD_LOGIC_VECTOR(C_Internal_Adr_Width-1 DOWNTO 0) := CONV_STD_LOGIC_VECTOR( 4, C_Internal_Adr_Width);
	
	
	SIGNAL		s_reset					: STD_LOGIC;
	SIGNAL		S_First_Sync_Reset		: STD_LOGIC;

	SIGNAL		S_SCUB_Addr				: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_SCUB_RDnWR			: STD_LOGIC;
	SIGNAL		S_SCUB_DS				: STD_LOGIC;

	SIGNAL		S_Slave_Nr				: STD_LOGIC_VECTOR(3 DOWNTO 0);
	SIGNAL		S_SCUB_Slave_Sel		: STD_LOGIC_VECTOR(11 DOWNTO 0);
	SIGNAL		S_Slave_Sel				: STD_LOGIC_VECTOR(11 DOWNTO 0);
	
	SIGNAL		S_Start_Cycle			: STD_LOGIC;

	SIGNAL		S_Sel_Ext_Data_Drv		: STD_LOGIC;

	SIGNAL		S_Rd_Data				: STD_LOGIC_VECTOR(15 DOWNTO 0);

	SIGNAL		S_Start_SCUB_Rd			: STD_LOGIC;

	SIGNAL		S_Start_SCUB_Wr			: STD_LOGIC;
	SIGNAL		S_Wr_Data				: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- store write pattern

	SIGNAL		S_Ti_Cy					: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- shift reg to generate pulse
	SIGNAL		S_Start_Ti_Cy			: STD_LOGIC;

	SIGNAL		S_nSync_Dtack			: STD_LOGIC;
	SIGNAL		S_Last_Cycle_Timing		: STD_LOGIC;
	SIGNAL		S_SCUB_Timing_Cycle		: STD_LOGIC;

	SIGNAL		S_SCUB_Rd_Err_no_Dtack	: STD_LOGIC;
	SIGNAL		S_SCUB_Wr_Err_no_Dtack	: STD_LOGIC;

	SIGNAL		S_Ti_Cyc_Err			: STD_LOGIC;
	SIGNAL		S_Timing_In				: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- store input timing_in
	SIGNAL		S_SCUB_Ti_Fin			: STD_LOGIC;

	SIGNAL		S_SRQ_Ena				: STD_LOGIC_VECTOR(nSCUB_SRQ_Slaves'range);
	SIGNAL		S_SRQ_Sync				: STD_LOGIC_VECTOR(nSCUB_SRQ_Slaves'range);
	SIGNAL		S_SRQ_active			: STD_LOGIC_VECTOR(nSCUB_SRQ_Slaves'range);
	SIGNAL		S_one_or_more_SRQs_act	: STD_LOGIC;

	SIGNAL		S_Status				: STD_LOGIC_VECTOR(15 DOWNTO 0);

	SIGNAL		S_SCUB_Version			: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL		S_SCUB_Revision			: STD_LOGIC_VECTOR(7 DOWNTO 0);
	
	SIGNAL		S_SCU_Bus_Access_Active	: STD_LOGIC;
	SIGNAL		S_Wait_Request			: STD_LOGIC;

	SIGNAL		S_Invalid_Slave_Nr		: STD_LOGIC;
	SIGNAL		S_Invalid_Intern_Acc	: STD_LOGIC;


	END SCU_Bus_Master;


ARCHITECTURE Arch_SCU_Bus_Master OF SCU_Bus_Master IS


	TYPE	T_SCUB_SM	IS	(
							Idle,
							S_Rd_Cyc,		-- start read SCU_Bus cycle
							Rd_Cyc,			-- read SCU_Bus read active
							E_Rd_Cyc,		-- end read SCU_Bus
							F_Rd_Cyc,		-- finish read SCU_Bus
							TO_Rd_Cyc,		-- time out read cycle
							S_Wr_Cyc,		-- start write SCU_Bus cycle
							Wr_Cyc,			-- write SCU_Bus active
							E_Wr_Cyc,		-- end write SCU_Bus
							F_Wr_Cyc,		-- finish write SCU_Bus
							TO_Wr_Cyc,		-- time out write cycle
							S_Ti_Cyc,		-- start Timing cycle
							Ti_Cyc,			-- Timing cycle active
							E_Ti_Cyc,		-- end Timing cycle
							F_Ti_Cyc		-- finish time cycle
							);

	SIGNAL	SCUB_SM	: T_SCUB_SM;


BEGIN

S_SCUB_Version	<= conv_std_logic_vector(C_SCUB_Version, S_SCUB_Version'length);	-- set the version of this macro
S_SCUB_Revision	<= conv_std_logic_vector(C_SCUB_Revision, S_SCUB_Revision'length);	-- set the revision of this macro

ASSERT (False)
	REPORT "SCU_Bus_Master_Macro: Version --> " & integer'image(C_SCUB_Version)
			& ", Revision is --> " & integer'image(C_SCUB_Revision)
SEVERITY Warning;


ASSERT NOT (Clk_in_Hz < 100000000)
	REPORT "Achtung Generic Clk_in_Hz ist auf " & integer'image(Clk_in_Hz)
			& " gesetzt. Mit der Periodendauer von " & integer'image(Clk_in_ns)
			& " ns lassen sich keine genauen Verzoegerungen erzeugen!"
SEVERITY Warning;

ASSERT (False)
	REPORT "time_out_in_ns = " & integer'image(time_out_in_ns)
			& ",   Clk_in_ns = " & integer'image(Clk_in_ns)
			& ",   C_time_out_cnt = " & integer'image(C_time_out_cnt+2)
SEVERITY NOTE;

ASSERT (False)
	REPORT "Sel_dly_in_ns = " & integer'image(Sel_dly_in_ns)
			& ",   C_Sel_dly_cnt = " & integer'image(C_Sel_dly_cnt+2)
			& ",   Sel_release_in_ns = " & integer'image(Sel_release_in_ns)
			& ",   Sel_release_cnt = " & integer'image(C_Sel_release_cnt+2)
SEVERITY NOTE;

ASSERT (False)
	REPORT "Timing_str_in_ns = " & integer'image(Timing_str_in_ns)
			& ",   C_Timing_str_cnt = " & integer'image(C_Timing_str_cnt+2)
			& ",   D_Valid_to_DS_in_ns = " & integer'image(D_Valid_to_DS_in_ns)
			& ",   C_D_Valid_to_DS_cnt = " & integer'image(C_D_Valid_to_DS_cnt+2)
SEVERITY NOTE;


P_Reset:	PROCESS	(clk, Reset)
	BEGIN
		IF rising_edge(clk) THEN
			S_First_Sync_Reset <= reset;
			s_reset <= S_First_Sync_Reset;
		END IF;
	END PROCESS P_Reset;


S_Status(15)	<= '0';
S_Status(14)	<= '0';
S_Status(13)	<= '0';
S_Status(12)	<= '0';
S_Status(11)	<= '0';
S_Status(10)	<= '0';
S_Status(9)		<= '0';
S_Status(8)		<= S_one_or_more_SRQs_act;
S_Status(7)		<= '0';
S_Status(6)		<= '0';
S_Status(5)		<= '0';
S_Status(4)		<= S_Invalid_Slave_Nr;
S_Status(3)		<= S_Invalid_Intern_Acc;
S_Status(2)		<= S_Ti_Cyc_Err;
S_Status(1)		<= S_SCUB_Rd_Err_no_Dtack;
S_Status(0)		<= S_SCUB_Wr_Err_no_Dtack;


P_SCUB_Cntrl: Process (clk, s_reset)
	BEGIN
		IF s_reset = '1' THEN
			S_Start_Cycle			<= '0';
			S_Start_SCUB_Rd			<= '0';						-- reset start SCU_Bus read
			S_Start_SCUB_Wr			<= '0';						-- reset start SCU_Bus write
			S_Start_Ti_Cy			<= '0';						-- reset start SCU_Bus timing cycle
			S_Ti_Cyc_Err			<= '0';						-- reset timing error flag
			S_SCUB_Rd_Err_no_Dtack	<= '0';						-- reset read timeout flag
			S_SCUB_Wr_Err_no_Dtack	<= '0';						-- reset write timeout flag
			S_Ti_Cy(S_Ti_Cy'range)	<= (OTHERS => '0');			-- shift reg to generate pulse
			S_SRQ_Ena				<= "111111111111";			-- all SRQs[12..1] are enabled
			S_Wait_Request			<= '0';
			S_Invalid_Slave_Nr		<= '0';
			S_Invalid_Intern_Acc	<= '0';

		ELSIF rising_edge(clk) THEN
		
			S_Wait_Request <= '1';

			S_Ti_Cy(S_Ti_Cy'range) <= (S_Ti_Cy(S_Ti_Cy'high-1 DOWNTO 0) & Start_Timing_Cycle);		-- shift reg to generate pulse

			IF Start_Cycle = '1' THEN
				S_Start_Cycle <= '1';
			END IF;

			CASE Slave_Nr IS

				WHEN X"0" =>
					IF S_Start_Cycle = '1' THEN
						CASE Adr(C_Internal_Adr_Width-1 DOWNTO 0) IS
							WHEN C_Status_Adr =>
								IF Wr_Cycle = '1' THEN
									IF Wr_Data(0) = '1' THEN								-- look to the bit position in status
										S_SCUB_Wr_Err_no_Dtack <= '0';						-- SCU_Bus write error no dtack
									END IF;
									IF Wr_Data(1) = '1' THEN								-- look to the bit position in status!
										S_SCUB_Rd_Err_no_Dtack <= '0';						-- reset SCU_Bus read error no dtack
									END IF;
									IF Wr_Data(2) = '1' THEN								-- look to the bit position in status!
										S_Ti_Cyc_Err <= '0';								-- reset SCU_Bus timing error
									END IF;
									IF Wr_Data(3) = '1' THEN								-- look to the bit position in status!
										S_Invalid_Intern_Acc <= '0';						-- reset invalid internal register access error
									END IF;
									IF Wr_Data(4) = '1' THEN								-- look to the bit position in status!
										S_Invalid_Slave_Nr <= '0';							-- reset invalid slave number error
									END IF;
								ELSIF Rd_Cycle = '1' THEN
									Rd_Data <= S_Status;
								END IF;
							WHEN C_SRQ_Ena_Adr =>
								IF Wr_Cycle = '1' THEN
									S_SRQ_Ena <= Wr_Data(nSCUB_SRQ_Slaves'high DOWNTO 0);
								ELSIF Rd_Cycle = '1' THEN
									Rd_Data <= ("0000" & S_SRQ_Ena);
								END IF;
							WHEN C_Srq_active_Adr =>
								IF Wr_Cycle = '1' THEN
									S_Invalid_Intern_Acc <= '1';
								ELSIF Rd_Cycle = '1' THEN
									Rd_Data <= ("0000" & S_SRQ_Active);
								END IF;
							WHEN C_Srq_In_Adr =>
								IF Wr_Cycle = '1' THEN
									S_Invalid_Intern_Acc <= '1';
								ELSIF Rd_Cycle = '1' THEN
									Rd_Data <= ("0000" & S_SRQ_Sync);
								END IF;
							WHEN C_Vers_Revi_Adr =>
								IF Wr_Cycle = '1' THEN
									S_Invalid_Intern_Acc <= '1';
								ELSIF Rd_Cycle = '1' THEN
									Rd_Data <= (S_SCUB_Version & S_SCUB_Revision);
								END IF;
							WHEN OTHERS => 
								S_Invalid_Intern_Acc <= '1';
						END CASE;
						S_Invalid_Intern_Acc <= '1';
						S_Wait_Request <= '0';
						S_Start_Cycle <= '0';
					END IF;
				WHEN X"1" | X"2" | X"3" | X"4" | X"5" | X"6" | X"7" | X"8" | X"9" | X"A" | X"B" | X"C" =>
					IF S_Start_Cycle = '1' THEN
						IF Wr_Cycle = '1' THEN
							S_Start_SCUB_Wr <= '1';									-- store write request
							S_Start_SCUB_Rd <= '0';
							S_Wr_Data <= Wr_Data;									-- store write pattern
						ELSIF Rd_Cycle = '1' THEN
							S_Start_SCUB_Rd <= '1';									-- store read request
							S_Start_SCUB_Wr <= '0';
						END IF;
					END IF;
					Rd_Data <= S_Rd_Data;
				WHEN OTHERS =>
					S_Invalid_Slave_Nr <= '1';
  					Rd_Data <= S_Rd_Data;
					S_Wait_Request <= '0';
					S_Start_Cycle <= '0';
			END CASE;


			IF SCUB_SM = E_Wr_Cyc THEN
				S_Start_SCUB_Wr <= '0';												-- write request finished
				S_Wait_Request <= '0';
				S_Start_Cycle <= '0';
			END IF;
			
			
			IF SCUB_SM = TO_Wr_Cyc THEN
				S_SCUB_Wr_Err_no_Dtack <= '1';										-- SCU_Bus write error no dtack
			END IF;

			IF  SCUB_SM = E_Rd_Cyc THEN
				S_Start_SCUB_Rd <= '0';												-- read request finished
				S_Wait_Request <= '0';
				S_Start_Cycle <= '0';
			END IF;
			
			IF SCUB_SM = TO_Rd_Cyc THEN
				S_SCUB_Rd_Err_no_Dtack <= '1';										-- SCU_Bus read error no dtack
			END IF;		

			IF S_Ti_Cy = "01" THEN													-- positive edge off start_timing_cycle
				IF S_Start_Ti_Cy = '1' THEN
					S_Ti_Cyc_Err <= '1';											-- SCU_Bus timing error, new request but old request not finished
				ELSE
					S_Start_Ti_Cy <= '1';											-- store timing request
					S_Timing_In <= Timing_In(15 DOWNTO 0);							-- store timing pattern
				END IF;
			END IF;
			
			IF SCUB_SM = E_Ti_Cyc THEN
			   S_Start_Ti_Cy <= '0';
			END IF;

		END IF;

	END PROCESS P_SCUB_CNTRL;


P_SCUB_SM:	PROCESS (clk, s_reset)

BEGIN
	IF s_reset = '1' THEN
		SCUB_SM <= Idle;
		S_Last_Cycle_Timing	<= '0';
		S_SCUB_Timing_Cycle	<= '0';
		S_SCUB_RDnWR		<= '1';
		S_SCUB_DS			<= '0';
		S_SCUB_Slave_Sel	<= (OTHERS => '0');
		S_Sel_Ext_Data_Drv	<= '0';

	ELSIF rising_edge(clk) THEN

		IF Test = 0 THEN
			S_nSync_Dtack <= nSCUB_Dtack;	-- SCU_Bus_Dtack is an asynchronous Signal. S_nSync_Dtack is the synchronized nSCU_Bus_Dtack
		ELSE
			S_nSync_Dtack <= not S_SCUB_DS; -- during test mode S_nSync_dtack is gererated with the S_SCUB_DS signal
		END IF;
		
		CASE SCUB_SM IS					-- = SCU_Bus State Machine

			WHEN Idle =>
				S_Sel_dly_cnt		<= conv_std_logic_vector(C_Sel_dly_cnt, S_Sel_dly_cnt'length);
				S_D_Valid_to_DS_cnt	<= conv_std_logic_vector(C_D_Valid_to_DS_cnt, S_D_Valid_to_DS_cnt'length);
				S_Sel_release_cnt	<= conv_std_logic_vector(C_Sel_release_cnt, S_Sel_release_cnt'length);
				S_SCUB_Slave_Sel	<= (OTHERS => '0');
				S_SCUB_Addr			<= (OTHERS => '1');
				S_SCUB_RDnWR		<= '1';
				S_SCUB_Timing_Cycle	<= '0';
				S_SCUB_DS			<= '0';
				S_Sel_Ext_Data_Drv	<= '0';


				IF ((S_Start_SCUB_Rd = '1') AND (S_Start_Ti_Cy = '0')) THEN
					S_SCUB_Addr	<= Adr;										-- store slave address
					S_Slave_Nr <= Slave_Nr;									-- store slave number
					SCUB_SM <= S_Rd_Cyc;									-- jump to start read cycle
				ELSIF ((S_Start_SCUB_Wr = '1') AND (S_Start_Ti_Cy = '0')) THEN
					S_SCUB_Addr	<= Adr;										-- store slave address
					S_Slave_Nr <= Slave_Nr;									-- store slave number
					S_SCUB_RDnWR <= '0';									-- set master writes
					SCUB_SM <= S_Wr_Cyc;									-- jump to start write cycle
				ELSIF ((S_Start_SCUB_Rd = '0') AND (S_Start_SCUB_Wr = '0') AND (S_Start_Ti_Cy = '1')) THEN
					S_SCUB_RDnWR <= '0';									-- set master writes
					SCUB_SM <= S_Ti_Cyc;										-- jump to start Timing cycle
				ELSIF ((S_Start_SCUB_Wr = '1') AND (S_Start_Ti_Cy = '1')) THEN
					IF (S_Last_Cycle_Timing = '1') THEN
						S_SCUB_Addr	<= Adr;									-- store slave address
						S_Slave_Nr <= Slave_Nr;								-- store slave number
						S_SCUB_RDnWR <= '0';								-- set master writes
						SCUB_SM <= S_Wr_Cyc;								-- jump to start write cycle
					ELSE
						S_SCUB_RDnWR <= '0';								-- set master writes
						SCUB_SM <= S_Ti_Cyc;								-- jump to start Timing cycle
					END IF;
				ELSIF ((S_Start_SCUB_Rd = '1') AND (S_Start_Ti_Cy = '1')) THEN
					IF (S_Last_Cycle_Timing = '1') THEN
						S_SCUB_Addr	<= Adr;									-- store slave address
						S_Slave_Nr <= Slave_Nr;								-- store slave number
						SCUB_SM <= S_Rd_Cyc;								-- jump to start read cycle
					ELSE
						S_SCUB_RDnWR <= '0';								-- set master writes
						SCUB_SM <= S_Ti_Cyc;								-- jump to start Timing cycle
					END IF;
				ELSE
					NULL;
				END IF;

			WHEN S_Rd_Cyc =>											-- start read cycle
				S_Sel_Ext_Data_Drv <= '1';
				S_Last_Cycle_Timing <= '0';								-- last SCU_Bus cycle is a data transfer cycle
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_SCUB_Slave_Sel <= S_Slave_Sel(11 DOWNTO 0);		-- select slave
					SCUB_SM <= Rd_Cyc;									-- jump to active read cycle
				END IF;

			WHEN Rd_Cyc =>												-- read cycle active
				IF S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' THEN
					S_SCUB_DS <= '1';
					IF S_nSync_Dtack = '0' THEN							-- wait for Dtack
						IF Test = 0 THEN
							S_Rd_Data <= SCUB_Data;						-- during production: read the SCUB_Data bidir buffer
						ELSE
							S_Rd_Data <= S_Wr_Data;						-- during test: return the last written data
						END IF;
						S_SCUB_DS <= '0';
						S_SCUB_Slave_Sel <= (OTHERS => '0');
						SCUB_SM <= E_Rd_Cyc;							-- jump to end read cycle
					ELSIF s_time_out_cnt(s_time_out_cnt'high) = '1' THEN
						S_SCUB_DS <= '0';
						S_SCUB_Slave_Sel <= (OTHERS => '0');
						SCUB_SM <= TO_Rd_Cyc;							-- jump to read timeout
					END IF;
				END IF;

			WHEN TO_Rd_Cyc =>											-- read timeout
				SCUB_SM <= E_Rd_Cyc;									-- jump to E_Rd_Cyc

			WHEN E_Rd_Cyc =>											-- end read cycle
				S_Sel_Ext_Data_Drv <= '0';
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					SCUB_SM <= F_Rd_Cyc;								-- jump to finish read cycle
				END IF;

			WHEN F_Rd_Cyc =>
				SCUB_SM <= Idle;										-- jump to Idle

			WHEN S_Wr_Cyc =>											-- start write cycle
				S_Last_Cycle_Timing <= '0';								-- last SCU_Bus cycle is a data transfer cycle
				S_Sel_Ext_Data_Drv <= '1';
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_SCUB_Slave_Sel <= S_Slave_Sel(11 DOWNTO 0);		-- select slave
					SCUB_SM <= Wr_Cyc;									-- jump to active write cycle
				END IF;

			WHEN Wr_Cyc =>												-- write cycle active
				IF S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' THEN
					S_SCUB_DS <= '1';
					IF (S_nSync_Dtack = '0') OR (s_time_out_cnt(s_time_out_cnt'high) = '1') THEN	-- wait for Dtack or timeout
						S_SCUB_DS <= '0';
						S_Sel_Ext_Data_Drv <= '0';
						S_SCUB_Slave_Sel <= (OTHERS => '0');
						IF s_time_out_cnt(s_time_out_cnt'high) = '0' THEN	-- no timeout
							SCUB_SM <= E_Wr_Cyc;							-- jump to end write cycle
						ELSE
							SCUB_SM <= TO_Wr_Cyc;							-- jump to write timeout
						END IF;
					END IF;
				END IF;

			WHEN TO_Wr_Cyc =>											-- write timeout
					S_SCUB_RDnWR <= '1';								-- set master reades
					SCUB_SM <= E_Wr_Cyc;								-- jump to Idle

			WHEN E_Wr_Cyc =>											-- end write cycle
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					S_SCUB_RDnWR <= '1';								-- set master reades
					SCUB_SM <= F_Wr_Cyc;								-- jump to finish write cycle
				END IF;

			WHEN F_Wr_Cyc =>
					SCUB_SM <= Idle;									-- jump to Idle

			WHEN S_Ti_Cyc =>											-- start Timing cycle
				S_Last_Cycle_Timing <= '1';								-- last SCU_Bus cycle is a timing cycle
				S_SCUB_Addr	<= Timing_In(31 DOWNTO 16);					-- Timing to S_SCUB_Addr
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_Sel_Ext_Data_Drv <= '1';
					S_SCUB_Slave_Sel <= (OTHERS => '1');				-- in this version select all slaves.
					S_Timing_str_cnt <= conv_std_logic_vector(C_Timing_str_cnt, S_Timing_str_cnt'length);
					SCUB_SM <= Ti_Cyc;									-- jump to active Timing cycle
				END IF;

			WHEN Ti_Cyc =>												-- Timing cycle active
				S_SCUB_Timing_Cycle <= '1';								-- timing cycle signal active
				IF S_Timing_str_cnt(S_Timing_str_cnt'high) = '1' THEN
					S_SCUB_Timing_Cycle <= '0';							-- timing cycle signal inactive
					S_SCUB_Slave_Sel <= (OTHERS => '0');				-- deselect all slaves.
					SCUB_SM <= E_Ti_Cyc;								-- jump to end Timing cycle
				END IF;

			WHEN E_Ti_Cyc =>											-- end Timing cycle
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					S_SCUB_RDnWR <= '1';								-- set master reades
					S_Sel_Ext_Data_Drv <= '0';
					SCUB_SM <= F_Ti_Cyc;								-- jump to finish time cycle
				END IF;

			WHEN F_Ti_Cyc =>
					SCUB_SM <= Idle;									-- jump to Idle

			WHEN OTHERS =>
				SCUB_SM <= Idle;

		END CASE;


		IF ((SCUB_SM = S_Wr_Cyc) OR (SCUB_SM = S_Rd_Cyc) OR (SCUB_SM = S_Ti_Cyc)) AND S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '0' THEN
			S_Sel_dly_cnt <= S_Sel_dly_cnt - 1;
		END IF;

		IF ((SCUB_SM = Wr_Cyc) OR (SCUB_SM = Rd_Cyc)) AND S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '0' THEN
			S_D_Valid_to_DS_cnt <= S_D_Valid_to_DS_cnt - 1;
		END IF;

		IF ((SCUB_SM = E_Wr_Cyc) OR (SCUB_SM = E_Rd_Cyc) OR (SCUB_SM = E_Ti_Cyc)) AND S_Sel_release_cnt(S_Sel_release_cnt'high) = '0' THEN
			S_Sel_release_cnt <= S_Sel_release_cnt - 1;
		END IF;

		IF SCUB_SM = Ti_Cyc AND S_Timing_str_cnt(S_Timing_str_cnt'high) = '0' THEN
			S_Timing_str_cnt <= S_Timing_str_cnt - 1;
		END IF;

	END IF;
END PROCESS P_SCUB_SM;


p_board_sel:	PROCESS (clk, s_reset)
	BEGIN
		IF s_reset = '1' THEN
			S_Slave_Sel <= "000000000000";						-- no board select
		ELSIF rising_edge(clk) THEN
			CASE S_Slave_Nr IS
				WHEN X"0" =>	S_Slave_Sel <= "000000000000";
				WHEN X"1" =>	S_Slave_Sel <= "000000000001";	-- select board 1
				WHEN X"2" =>	S_Slave_Sel <= "000000000010";
				WHEN X"3" =>	S_Slave_Sel <= "000000000100";
				WHEN X"4" =>	S_Slave_Sel <= "000000001000";
				WHEN X"5" =>	S_Slave_Sel <= "000000010000";
				WHEN X"6" =>	S_Slave_Sel <= "000000100000";
				WHEN X"7" =>	S_Slave_Sel <= "000001000000";
				WHEN X"8" =>	S_Slave_Sel <= "000010000000";
				WHEN X"9" =>	S_Slave_Sel <= "000100000000";
				WHEN X"A" =>	S_Slave_Sel <= "001000000000";
				WHEN X"B" =>	S_Slave_Sel <= "010000000000";
				WHEN X"C" =>	S_Slave_Sel <= "100000000000";	-- select board 12
				WHEN OTHERS =>  S_Slave_Sel <= "000000000000";	-- no board select
			END CASE;
		END IF;
	END PROCESS p_board_sel;


p_intr:	PROCESS (clk, s_reset)
	BEGIN
		IF s_reset = '1' THEN
			S_SRQ_Sync		<= "000000000000";					-- set synchronized SRQs to no SRQ
			S_SRQ_active	<= "000000000000";					-- set active SRQs to no SRQ
			S_one_or_more_SRQs_act <= '0';
			Intr			<= '0';

		ELSIF rising_edge(clk) THEN

			S_SRQ_Sync <= NOT nSCUB_SRQ_Slaves;					-- synchronize and change level of nSCUB_SRQ_Slave signals
																-- S_SRQ_Sync(n) = '1' => nSCUB_SRQ_Slaves(n) is active
			FOR i IN nSCUB_SRQ_Slaves'range LOOP
				IF S_SRQ_Ena(i) = '1' THEN
					IF S_SRQ_Sync(i) = '1' THEN
						S_SRQ_active(i) <= '1';
					ELSE
						S_SRQ_active(i) <= '0';
					END IF;
				ELSE
					S_SRQ_active(i) <= '0';	-- ???
				END IF;
			END LOOP;

			IF S_SRQ_active /= CONV_STD_LOGIC_VECTOR( 0, nSCUB_SRQ_Slaves'length) THEN
				S_one_or_more_SRQs_act <= '1';
			ELSE
				S_one_or_more_SRQs_act <= '0';
			END IF;

			IF S_one_or_more_SRQs_act = '1' OR S_Ti_Cyc_Err = '1' OR S_SCUB_Rd_Err_no_Dtack = '1' OR S_SCUB_Wr_Err_no_Dtack = '1' THEN
				Intr <= '1';
			ELSE
				Intr <= '0';
			END IF;

		END IF;
	END PROCESS p_intr;


P_SCUB_Tri_State:	PROCESS (SCUB_SM, S_Wr_Data, S_Timing_In)
	BEGIN
		IF (SCUB_SM = S_Wr_Cyc) OR (SCUB_SM = Wr_Cyc) THEN
			SCUB_Data <= S_Wr_Data;
		ELSIF (SCUB_SM = Ti_Cyc) OR (SCUB_SM = E_Ti_Cyc) THEN
			SCUB_Data <= S_Timing_In(15 DOWNTO 0);
		ELSE
			SCUB_Data <= (OTHERS => 'Z');
		END IF;
	END PROCESS P_SCUB_Tri_State;


p_time_out:	PROCESS (Clk, s_reset)
	BEGIN
		IF s_reset = '1' THEN
			s_time_out_cnt <= conv_std_logic_vector(C_time_out_cnt, s_time_out_cnt'length);
		ELSIF rising_edge(Clk) THEN
			IF NOT ((SCUB_SM = Rd_Cyc) OR (SCUB_SM = Wr_Cyc)) THEN
				s_time_out_cnt <= conv_std_logic_vector(C_time_out_cnt, s_time_out_cnt'length);
			ELSIF s_time_out_cnt(s_time_out_cnt'high) = '0' THEN									-- no underflow
				s_time_out_cnt <= s_time_out_cnt - 1;												-- count down
			END IF;
		END IF;
	END PROCESS p_time_out;


SCUB_Addr				<= S_SCUB_Addr;
SCUB_RDnWR				<= S_SCUB_RDnWR;
nSCUB_DS				<= NOT S_SCUB_DS;
nSCUB_Slave_Sel			<= NOT S_SCUB_Slave_Sel;

nSCUB_Timing_Cycle		<= NOT S_SCUB_Timing_Cycle;

nSel_Ext_Data_Drv		<= NOT S_Sel_Ext_Data_Drv;

nStart_DB_Trm			<= '0' WHEN (SCUB_SM = F_Rd_Cyc)  ELSE '1';

SCUB_Rd_active			<= S_Start_SCUB_Rd;
SCUB_Rd_Fin				<= '1' WHEN SCUB_SM = F_Rd_Cyc ELSE '0';
SCUB_Rd_Err_no_Dtack	<= S_SCUB_Rd_Err_no_Dtack;

SCUB_Wr_active			<= S_Start_SCUB_Wr;
SCUB_Wr_Fin				<= '1' WHEN SCUB_SM = F_Wr_Cyc ELSE '0';
SCUB_Wr_Err_no_Dtack	<= S_SCUB_Wr_Err_no_Dtack;

S_SCUB_Ti_Fin			<= '1' WHEN SCUB_SM = F_Ti_Cyc ELSE '0';
SCUB_Ti_Fin				<= S_SCUB_Ti_Fin;

SCUB_Ti_Cyc_Err			<= S_Ti_Cyc_Err;

S_SCU_Bus_Access_Active <= '1' WHEN (S_Start_SCUB_Wr = '1') OR (S_Start_SCUB_Rd = '1') ELSE '0';
SCU_Bus_Access_Active <= S_SCU_Bus_Access_Active;

SCU_Wait_Request		<= 	S_Wait_Request;

END Arch_SCU_Bus_Master;
