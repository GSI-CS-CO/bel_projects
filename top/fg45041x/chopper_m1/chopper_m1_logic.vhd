--+-----------------------------------------------------------------------------------------------------------------+
--| Vers_13:																										|
--|		Autor:	W. Panschow																							|
--|		Datum:	25.08.2010																							|
--|		Grund:	Korrektur eines Fehlers.																			|
--|		Beschreibung: Der Fehler wurde bei Zweistrahlbetrieb festgestellt. Da wurde der HSI-Chopper nur dann		|
--|				aufgemacht,	wenn M3 anfordert, dieser wurde aber vom HLI-Chopper gesteuert.							|
--|		Ursache: Bei der grundlegenden Überarbeitung des Makros "chopper_m1_logic" in Vers_12 wurde das Signal		|
--|				"Off_Anforderung_In" nicht mehr abgefragt ob es durch den "HSI_ALV" und "HLI_ALV" gültig werden		|
--|				darf.																								|
--|		Korrektur: Im Prozess "align_anf_hsi" wird "Off_Anforderung_In" mit "HSI_ALV" maskiert.						|
--|				Und im Prozess "align_anf_hli" wird "Off_Anforderung_In" mit "HLI_ALV" maskiert.					|
--+-----------------------------------------------------------------------------------------------------------------+
--| Vers_14:																										|
--| 	Autor: S. Rauch																								|
--|		Datum: 11.10.2010																							|
--|		Grund: Fehlerbehebung																						|
--|		Beschreibung: Beschleuniger mit Strahlziel vor Alvarez werden nicht erlaubt									|
--|		Ursache: Durch die alleinige Qualifizierung mit HSI_ALV und HLI_ALV werden die Strahlziele davor			|
--|				ausmaskiert.																						|
--|				Die Anforderung für UU muss gesondert abgefragt werden.												|
--|		Korrektur: Ein weiterer Prozess für s_anfoderung_UU_aligned wird eingefügt.									|
--|					Die s_anforderung_aligned_ Signale wirken jetzt auf die Verkürzungsbedingung.					|
--|					Diese verkürzt dann den Rahmenpulse Chopper_HSI/HLI_delayed.									|												|
--+-----------------------------------------------------------------------------------------------------------------+

library IEEE;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.ALL;

LIBRARY lpm;
USE lpm.lpm_components.all;

entity chopper_m1_logic is
		GENERIC ( 	Clk_in_HZ : Integer := 200000000;	--! Frequenz des Clock Signals in Herz
					Test : Integer := 1		--! Beim Test werden kürzere Verzögerungswerte eingesetzt, nicht für Release verwenden!
				);
		PORT (
			Skal_OK:					IN std_logic;
			Data_WR:					IN std_logic_vector(15 downto 0);
			Strahlweg_Reg_WR:			IN std_logic;
			Strahlweg_Reg_RD:			IN std_logic;
			Strahlweg_Maske_WR:			IN std_logic;
			Interlock_Reg_WR:			IN std_logic;
			TK8_Delay_WR:				IN std_logic;
			CLK:						IN std_logic;		--! Sollte der Takt sein, der auch in den anderen Macros verwendet wird.

			Reset:						IN std_logic;

			Beam_Control_In:			IN std_logic_vector(15 downto 0) := x"0000"; --! Eingang für schnelle Signale
			Strahlalarm_In:				IN std_logic_vector(15 downto 0) := x"FFFF"; --! Eingang für schnelle Signale

															--! Off Signale sind Active high
			Off_Anforderung_In:			IN std_logic;		--! Ergebnis vom Chopper_Macro2, wird auf der zweiten MB64-Karte aktiviert und über die Bus_IO(0)-Leitung angeliefert.
			Off_UU_In:					IN std_logic;		--! Ergebnis vom Chopper_Macro2, wird auf der zweiten MB64-Karte aktiviert und	über die Bus_IO(1)-Leitung angeliefert.										--

			Strahlweg_Reg:				OUT std_logic_vector(15 downto 0);	--! Strahlwegregister
			Strahlweg_Maske:			OUT std_logic_vector(15 downto 0);	--! Strahlwegmaske

			Interlock_to_SE:			OUT std_logic_vector(15 downto 0);

			Beam_Control_Out:			OUT std_logic_vector(15 downto 0);	--! Hardware Ausgang
			Trafo_Timing_Out:			OUT std_logic_vector(15 downto 0);	--! Hardware Ausgang

			Interlock_Reg:				OUT std_logic_vector(15 downto 0);
			TK8_Delay:					OUT std_logic_vector(15 downto 0);


			HSI_act_pos_latch_out:		out std_logic_vector(15 downto 0);	--! Ausgang für die HSI Istwertmessung
			HSI_neg_latch_out:			out	std_logic_vector(15 downto 0);	--! Ausgang für die HSI Istwertmessung
			HSI_act_neg_latch_out:		out	std_logic_vector(15 downto 0);	--! Ausgang für die HSI Istwertmessung

			HLI_act_pos_latch_out:		out std_logic_vector(15 downto 0);	--! Ausgang für die HLI Istwertmessung
			HLI_neg_latch_out:			out	std_logic_vector(15 downto 0);	--! Ausgang für die HLI Istwertmessung
			HLI_act_neg_latch_out:		out	std_logic_vector(15 downto 0);	--! Ausgang für die HLI Istwertmessung

			Chop_m1_LEDs:				OUT std_logic_vector(15 downto 0)	--! LEDs auf der Frontseite der Logikkarte
		);


		--constant  	CLK_in_Hz : integer := 150000000;		-- Damit das Design schnell auf eine andere Frequenz umgestellt werden kann, wird diese		--
															-- in Hz festgelegt. Zähler die betimmte Zeiten realisieren sollen z.B. 'Edge_delay_50us'	--
															-- werden entprechend berechnet.															--
		--! 14 Bit + Overflow
		--!
		--! |A |OV|Counter|
		--! |15|14|13 .. 0|
		constant C_Chop_Count_Width:	Integer := 15;
		constant C_Chop_neg_Diff:		Integer := 20;


end chopper_m1_logic;

architecture chopper_m1_logic_arch of chopper_m1_logic is

component pos_or_neg_dly is
	generic (
		Edge_delay_cnt:		in integer := 200
		);
	port (
		Edge_pos:			in std_logic := '0';
		Edge_neg:			in std_logic := '0';
		Clk:				in std_logic;
		Reset:				in std_logic;

		Sig_out:			out std_logic
		);
	end component;

component pos_or_neg_dly_trunc is
	generic (
		Edge_delay_cnt:		in integer := 200;
		min_cnt_value:		in integer
		);
	port (
		Edge_pos:			in std_logic := '0';
		Edge_neg:			in std_logic := '0';
		trunc:				in std_logic;
		Clk:				in std_logic;
		Reset:				in std_logic;

		Sig_out:			out std_logic
		);
	end component;

component Puls is
		generic	(
					delay_cnt:		integer := 10
				);
		port (
					Pos_Edge:		IN std_logic;
					Clk:			IN std_logic;
					Reset:			IN std_logic;

					Puls:			Out std_logic
			);
end component;

component trunc is
		generic	(
					delay_cnt:		integer := 20
				);
		port (
					neg_edge:		in std_logic;
					clk:			in std_logic;
					reset:			in std_logic;

					trunc:			out std_logic
			);
end component;

component edge_detection is
	port (
				Clk:		in std_logic;
				Reset:		in std_logic;
				input:		in std_logic;
				pos_edge:	out std_logic;
				neg_edge:	out std_logic
			);
end component;

component var_delay is
	port (
			Signal_in:	in std_logic;
			delay:		in std_logic_vector(15 downto 0);
			en_100ns:	in std_logic;
			Clk:		in std_logic;
			Reset:		in std_logic;

			Signal_out:	out std_logic
		);
end component;

component chopper_monitoring is
		generic (
					C_Chop_Count_Width:		integer := 15;
					C_Chop_neg_Diff:		integer := 20;
					C_inl_cnt_value:		integer := 3
				);
		port (
				clk					: in std_logic;
				reset				: in std_logic;
				s_1us_en			: in std_logic;
				clear				: in std_logic;
				chopp_signal_on		: in std_logic;
				chopp_signal_act	: in std_logic;
				act_pos_latch_out	: out std_logic_vector(C_Chop_Count_Width  downto 0);
				neg_latch_out		: out std_logic_vector(C_Chop_Count_Width  downto 0);
				act_neg_latch_out	: out std_logic_vector(C_Chop_Count_Width  downto 0)


			);
end component chopper_monitoring;

function prod_or_test(production, test_data, test : integer) return integer is
	variable data: integer;
begin
	if Test = 1 then
		data := test_data;
	else
		data := production;
	end if;

	return data;
end prod_or_test;

--+-----------------------------------------------------------------------------+
--|				Definition der Delay Werte										|
--+-----------------------------------------------------------------------------+
									-- Produktion: 2000 * 0.1 us  ==> 200 us
									-- Test:  20 * 0.1us => 2 us
constant C_Edge_delay_cnt_200us:	integer	:= prod_or_test((CLK_in_Hz / 1000000) * 200, (CLK_in_Hz / 1000000) * 2, Test);

    								-- Produktion:  500 * 0.1 us  ==>  50 us
									-- Test:   5 * 0.1us => 0.5 us
constant C_Edge_delay_cnt_50us:		integer := prod_or_test((CLK_in_Hz / 1000000) *  50, (CLK_in_Hz / 1000000) / 2, Test);  			--

constant C_Edge_delay_cnt_30us:		integer := prod_or_test((CLK_in_Hz / 1000000) *  30, (CLK_in_Hz / 1000000) / 3, Test);



									--V04  C_Edge_delay_cnt_5us in ..._10us geändert.
									-- Produktion:   100 * 0.1 us  ==> 10 us
									-- Test:   3 * 0.1us => 0.3 us
constant C_Edge_delay_cnt_10us:		integer := prod_or_test((CLK_in_Hz / 1000000) *  10, (CLK_in_Hz / 1000000) / 3, Test);

constant C_Edge_delay_cnt_20us:		integer := prod_or_test((CLK_in_Hz / 1000000) *  20, (CLK_in_Hz / 1000000) , Test);




constant C_Prescaler_cnt_1us:		integer := (Clk_in_HZ / 1000000) - 2;
constant C_Prescaler_cnt_100ns:		integer := (Clk_in_HZ / 10000000) - 2;

constant C_Prescaler_Width:			integer := integer(floor(log2(real(C_Prescaler_cnt_1us)))) + 2;

									-- Zeit in Mikrosekunden
constant C_Watchdog_Value:			integer := prod_or_test(25000, 20, Test);
constant C_Watchdog_Width:			integer := integer(floor(log2(real(C_Watchdog_Value)))) + 2;

constant C_inl_cnt_value:			integer := 3;
constant C_inl_cnt_width:			integer := integer(floor(log2(real(C_inl_cnt_value)))) + 2;

--+-----------------------------------------------------------------------------+
--|	Register																	|
--+-----------------------------------------------------------------------------+
signal s_Logik_not_Sel_or_Reset : 	std_logic;
signal s_Strahlweg_Reg : 			std_logic_vector(10 downto 0);
signal s_Strahlweg_Maske : 			std_logic_vector(7 downto 0);
signal s_Interlock_Reg: 			std_logic_vector(1 downto 0);
signal s_TK8_Delay:					std_logic_vector(15 downto 0);


--+-----------------------------------------------------------------------------+
--|	Interne-Signale, werden in verschiedenen Logik-Gleichungen verwendet		|
--+-----------------------------------------------------------------------------+
signal	Off_ALV: 			std_logic;
signal	Off_ALV_min:		std_logic;
signal	Off_ALV_Verlust:	std_logic;
signal	Off_HSI_Verlust:	std_logic;
signal	Off_ALV_PG:			std_logic;
signal	Off_HSI_PG:			std_logic;
signal	Off_HSI:			std_logic;
signal 	Off_HSI_min:		std_logic;
signal	No_Beam_HSI:		std_logic;
signal	No_Beam_HLI:		std_logic;

--+-----------------------------------------------------------------------------+
--|	Strahlweg und Langsames Strahl-Interlock und Blockierung (von Interlock-SE)	|
--+-----------------------------------------------------------------------------+
signal	Qu_R_HSI:			std_logic;
signal	Qu_L_HSI:			std_logic;
signal	HSI_ALV:			std_logic;
signal	HLI_ALV:			std_logic;
signal	INL_HSI_BL:			std_logic;
signal	INL_HLI_BL:			std_logic;
signal	Block_HSI:			std_logic;
signal	Block_HLI:			std_logic;

--+-----------------------------------------------------------------------------+
--|			Kontrollbits für Trafo-Interlock (von Interlock-SE)				 	|
--+-----------------------------------------------------------------------------+
signal	Mask_TK8DT2V:		std_logic;
signal	Mask_UH4DT4P:		std_logic;
signal	Mask_US4DT7P:		std_logic;
signal	Mask_UT1DT0P:		std_logic;
signal	Mask_TK3DT4P:		std_logic;
signal	Mask_US_DT_E:		std_logic;
signal	Mask_UA_DT_E:		std_logic;
signal	Mask_TK_DT_E:		std_logic;
signal  Mask_TK3DT3P:		std_logic;
signal	Mask_TK_DT2V:		std_logic;

--+---------------------------------------------------------------------------------+
--|							Zeitsteuerpulse: Eingänge 								|
--+---------------------------------------------------------------------------------+
signal	Strahl_R:			std_logic;
signal	IQ_R:				std_logic;
signal	Strahl_L:			std_logic;
signal	IQ_L:				std_logic;
signal	Strahl_HLI:			std_logic;
signal	Chopper_HLI:		std_logic;
signal	Strahl_HSI:			std_logic;
signal	Chopper_HSI:		std_logic;
signal	Request_SIS:		std_logic;
signal	TK9DC9_out:			std_logic;
signal	Enable_TK:			std_logic;

--+---------------------------------------------------------------------------------+
--|			Trafo-Verlust/PG-IL: Eingänge von der Strahlverlustüberwachung			|
--+---------------------------------------------------------------------------------+
signal	UH_DT_V:					std_logic;
signal	UA_DT_V:					std_logic;
signal	TK_DT1V:					std_logic;
signal	UXZDT_V:					std_logic;
signal	TK8DT2V:					std_logic;
signal	SISDT_V:					std_logic;
signal	Strahlalarm_Frei1:			std_logic;
signal	Strahlalarm_Frei2:			std_logic;
signal	UH4DT4P:					std_logic;
signal	US4DT7P:					std_logic;
signal	UT1DT0P:					std_logic;
signal	TK3DT4P:					std_logic;
signal	US_DT_E:					std_logic;
signal	UA_DT_E:					std_logic;
signal	TK_DT_E:					std_logic;
signal	Chop_TK:					std_logic;
signal	TK3DT3P:					std_logic;
signal	TK_DT2V:					std_logic;

--+---------------------------------------------------------------------------------+
--|				Trafosteuerpulse: Ausgänge zur Strahlverlustüberwachung				|
--+---------------------------------------------------------------------------------+
signal	Rahmen_UL:					std_logic;
signal	Rahmen_UR:					std_logic;
signal	Klemm_UL:					std_logic;
signal	Klemm_UR:					std_logic;
signal	Klemm_UH1:					std_logic;
signal	Rahmen_UH:					std_logic;	--	..UH => ..HSI
signal	Rahmen_UH_min:				std_logic;
signal	mask_10us_hli:				std_logic;
signal	mask_10us_hsi:				std_logic;
signal	Klemm_UH:					std_logic;
signal 	Klemm_UH_delayed:			std_logic;
signal 	Klemm_UN_delayed:			std_logic;
signal	Rahmen_UN:					std_logic;	--	..UN => ..HLI
signal	Klemm_UN:					std_logic;
signal	Rahmen_UA:					std_logic;	--	..UA => ..ALV
signal	Rahmen_UA_after_20us:		std_logic;	-- 20us nach Ende Rahmenpuls
signal	Rahmen_UN_after_20us:		std_logic;	-- 20us nach Ende Rahmenpuls
signal	Rahmen_UH_after_20us:		std_logic;	-- 20us nach Ende Rahmenpuls
signal	Klemm_UA:					std_logic;
signal	Rahmen_TK8:					std_logic;
signal	Rahmen_TK8_delayed:			std_logic;
signal	s_R_TK8:					std_logic;
signal 	Klemm_TK8:					std_logic;
signal	Rahmen_TK:					std_logic;

--+---------------------------------------------------------------------------------+
--|			Stahlabschalt- und Chopperstatussignale: Ausgänge zu Chopper + HF		|
--+---------------------------------------------------------------------------------+
signal	Chopp_HSI_On:				std_logic;
signal	Chopp_HLI_On:				std_logic;
signal	HF_HSI_On:					std_logic;
signal	HF_ALV_Off:					std_logic;
signal	Chopper_HSI_delayed:		std_logic;
signal	Chopper_HLI_delayed:		std_logic;
signal	No_ERR_HSI:					std_logic;
signal	No_ERR_HLI:					std_logic;
signal 	Verkuerzung_HSI:			std_logic;
signal	Verkuerzung_HLI:			std_logic;

signal Chopper_HSI_act:				std_logic;
signal Chopper_HLI_act:				std_logic;


--------------------------------------------------------------------------------------
-- Signale für den 1us Prescaler
--------------------------------------------------------------------------------------
signal s_prscl_1us_out:				std_logic_vector(C_Prescaler_Width - 1 downto 0);
signal s_1us_en:					std_logic;
signal s_1us_set:					std_logic;

--------------------------------------------------------------------------------------
-- Signale für den 1us Prescaler
--------------------------------------------------------------------------------------
signal s_prscl_100ns_out:			std_logic_vector(C_Prescaler_Width - 1 downto 0);
signal s_100ns_en:					std_logic;
signal s_100ns_set:					std_logic;

--------------------------------------------------------------------------------------
-- Signale für den 25ms Watchdog Counter
--------------------------------------------------------------------------------------

signal s_watch_out:					std_logic_vector(C_Watchdog_Width - 1 downto 0);
signal s_watch_timeout:				std_logic;
signal s_watch_reset:				std_logic;
signal s_w_enable:					std_logic;
signal s_watch_en:					std_logic;

type state_type is ( Run, Stop, watch_Reset );
signal state:						state_type;

--signal s_tcnt_rst:					std_logic;
--signal s_tcnt_en:					std_logic;
--signal s_tallowed:					std_logic;
signal s_trunc_reset:				std_logic;
--------------------------------------------------------------------------------------
-- Ausrichten Anforderung / Verhindern von Retriggern
--------------------------------------------------------------------------------------

signal puls_10us_after_chopp_hsi:	std_logic;
signal chopp_hsi_pos:				std_logic;
signal s_anforderung_aligned_hsi:	std_logic;

signal puls_10us_after_chopp_hli:	std_logic;
signal chopp_hli_pos:				std_logic;
signal s_anforderung_aligned_hli:	std_logic;

signal s_anforderung_aligned_uu:	std_logic;


begin

	Logik_not_Sel_or_Reset: process (clk)
	begin
			if rising_edge(clk) then
				if Reset = '1' or  Skal_OK = '0' then
					s_Logik_not_Sel_or_Reset <= '1';
				else
					s_Logik_not_Sel_or_Reset <= '0';
				end if;
			end if;
	end process;

	--+-----------------------------------------------------------------------------+
	--|	Strahlweg und Langsames Strahl-Interlock und Blockierung (von Interlock-SE)	|
	--+-----------------------------------------------------------------------------+

	Strahlweg_Reg_ff: process (clk, Reset, s_Strahlweg_Reg)
	begin
		if Reset = '1' then
			s_Strahlweg_Reg <= (others => '0');
		elsif rising_edge(clk) then
			if Strahlweg_Reg_WR = '1' then
				s_Strahlweg_Reg <= Data_WR(10 downto 0);
			end if;
		end if;

		Qu_R_HSI 		<= s_Strahlweg_Reg(0);
		Qu_L_HSI 		<= s_Strahlweg_Reg(1);
		HSI_ALV 		<= s_Strahlweg_Reg(2);
		HLI_ALV 		<= s_Strahlweg_Reg(3);
		INL_HSI_BL 		<= s_Strahlweg_Reg(4);
		INL_HLI_BL		<= s_Strahlweg_Reg(5);
		Block_HSI		<= s_Strahlweg_Reg(6);
		Block_HLI		<= s_Strahlweg_Reg(7);
		Request_SIS		<= s_Strahlweg_Reg(8);
		TK9DC9_out		<= s_Strahlweg_Reg(9);
		Enable_TK		<= s_Strahlweg_Reg(10);

		Strahlweg_Reg(10 downto 0) <= s_Strahlweg_Reg;
		Strahlweg_Reg(15 downto 11) <= "00000";

	end process;

	--+-----------------------------------------------------------------------------+
	--|			Kontrollbits für Trafo-Interlock (von Interlock-SE)				 	|
	--+-----------------------------------------------------------------------------+

	Strahlweg_Maske_ff: process (clk, Reset, s_Strahlweg_Maske)
	begin
		if Reset = '1' then
			s_Strahlweg_Maske <= "00000000";
		elsif rising_edge(clk) then
			if Strahlweg_Maske_WR = '1' then
				s_Strahlweg_Maske <= Data_WR(7 downto 0);
			end if;
		end if;

		Mask_UH4DT4P 	<= s_Strahlweg_Maske(0);
		Mask_US_DT_E 	<= s_Strahlweg_Maske(1);
		Mask_US4DT7P 	<= s_Strahlweg_Maske(2);
		Mask_UA_DT_E 	<= s_Strahlweg_Maske(3);
		Mask_UT1DT0P	<= s_Strahlweg_Maske(4);
		Mask_TK_DT_E	<= s_Strahlweg_Maske(5);
		Mask_TK3DT4P	<= s_Strahlweg_Maske(6);
		Mask_TK3DT3P	<= s_Strahlweg_Maske(7); -- neu am 12.11.2007

		Strahlweg_Maske(7 downto 0)		<= s_Strahlweg_Maske;
		Strahlweg_Maske(15 downto 8) 	<= "00000000";

	end process;


	--+-----------------------------------------------------------------------------+
	--|			 Register für Zeitverletzungs Interlock				 				|
	--+-----------------------------------------------------------------------------+

	Interlock_Reg_ff: process ( clk, Strahlweg_Reg_WR, Reset)
	begin
		if Reset = '1' or Strahlweg_Reg_WR = '1' then
			s_Interlock_Reg <= "11";
		elsif rising_edge(Clk) then
			if Interlock_Reg_WR = '1' then
				s_Interlock_Reg <= Data_WR(1 downto 0);
			end if;
		end if;


	end process;

		No_ERR_HSI <= s_Interlock_Reg(0);
		No_ERR_HLI <= s_Interlock_Reg(1);


		Interlock_Reg(1 downto 0) <= s_Interlock_Reg;
		Interlock_Reg(15 downto 2) <= X"FFF" & "11";


	--+-----------------------------------------------------------------------------+
	--|			 Register für Rahmen_TK8_Delay				 						|
	--+-----------------------------------------------------------------------------+

	TK8_Delay_Reg: process ( clk, TK8_Delay_WR, Reset, s_TK8_Delay)
	begin
		if Reset = '1' then
			s_TK8_Delay <= conv_std_logic_vector(150, 16);	-- 150 * 100ns = 15us
		elsif rising_edge(Clk) then
			if TK8_Delay_WR = '1' then
				s_TK8_Delay <= Data_WR(15 downto 0);
			end if;
		end if;

		TK8_Delay(15 downto 0) <= s_TK8_Delay;
	end process;


	--------------------------------------------------------------
	-- Prescaler für 1us
	--------------------------------------------------------------

	Prescale_1us: lpm_counter generic map (
          lpm_width     => C_Prescaler_Width,
          lpm_type      => "LPM_COUNTER",
          lpm_direction => "DOWN",
          lpm_svalue    => integer'image(C_Prescaler_cnt_1us)
        )
        port map (
          clock  => Clk,
          cnt_en => '1',
          q      => s_prscl_1us_out,
          sset   => s_1us_set
        );

	s_1us_en  <= s_prscl_1us_out(s_prscl_1us_out'HIGH);
        s_1us_set <= s_prscl_1us_out(s_prscl_1us_out'HIGH);


	--------------------------------------------------------------
	-- Prescaler für 100ns
	--------------------------------------------------------------

	Prescale_100n: lpm_counter generic map (
          lpm_width     => C_Prescaler_Width,
          lpm_direction => "DOWN",
          lpm_svalue    => integer'image(C_Prescaler_cnt_100ns)
	)
	port map (
          clock  => Clk,
          cnt_en => '1',
          q      => s_prscl_100ns_out,
          sset   => s_100ns_set
	);

	s_100ns_en  <= s_prscl_100ns_out(s_prscl_100ns_out'HIGH);
	s_100ns_set <= s_prscl_100ns_out(s_prscl_100ns_out'HIGH);


	--------------------------------------------------------------
	-- Watchdog Counter mit 25 ms
	-- benutzt s_1us_en
	--------------------------------------------------------------

	s_w_enable    <= s_1us_en and not s_watch_timeout;
	s_watch_reset <= Strahlweg_Reg_WR or Reset;

	Watch_25ms: lpm_counter generic map (
          lpm_width     => C_Watchdog_Width,
          lpm_direction => "DOWN",
          lpm_svalue    => integer'image(C_Watchdog_Value)
	)
	PORT MAP (
          clock  => Clk,
          cnt_en => s_w_enable,
          q      => s_watch_out,
          sset   => s_watch_reset
	);

	s_watch_timeout <=  s_watch_out(s_watch_out'HIGH);


	--------------------------------------------------------------
	-- monitoring for chopper HSI
	--------------------------------------------------------------

	hsi_mon: chopper_monitoring generic map (
          C_Chop_Count_Width => C_Chop_Count_Width,
          C_Chop_neg_Diff    => C_Chop_neg_Diff,
          C_inl_cnt_value    => C_inl_cnt_value
        )
        port map (
          clk               => clk,
          reset             => reset,
          s_1us_en          => s_1us_en,
          clear             => Strahlweg_Reg_WR,
          chopp_signal_on   => chopp_hsi_on,                -- Sollwert
          chopp_signal_act  => Chopper_HSI_act,             -- Istwert
          act_pos_latch_out => HSI_act_pos_latch_out,       -- Timestamp
          neg_latch_out     => HSI_neg_latch_out,           -- Timestamp
          act_neg_latch_out => HSI_act_neg_latch_out        -- Timestamp
        );

	--------------------------------------------------------------
	-- monitoring for chopper HLI
	--------------------------------------------------------------

	hli_mon: chopper_monitoring generic map (
          C_Chop_Count_Width => C_Chop_Count_Width,
          C_Chop_neg_Diff    => C_Chop_neg_Diff,
          C_inl_cnt_value    => C_inl_cnt_value
        )
        port map (
          clk               => clk,
          reset             => reset,
          s_1us_en          => s_1us_en,
          clear             => Strahlweg_Reg_WR,
          chopp_signal_on   => chopp_hli_on,                -- Sollwert
          chopp_signal_act  => Chopper_HLI_act,             -- Istwert
          act_pos_latch_out => HLI_act_pos_latch_out,       -- Timestamp
          neg_latch_out     => HLI_neg_latch_out,           -- Timestamp
          act_neg_latch_out => HLI_act_neg_latch_out        -- Timestamp
        );



	--+---------------------------------------------------------------------------------+
	--|	Signale der Strahlverlustüberwachung zur Interlock-SE (Lese-Register)			|
	--+---------------------------------------------------------------------------------+
          Interlock_to_SE(0)		<= UH_DT_V;
	Interlock_to_SE(1)		<= UA_DT_V;
	Interlock_to_SE(2)		<= TK_DT1V;
	Interlock_to_SE(3)		<= UXZDT_V;
	Interlock_to_SE(4)		<= TK8DT2V;
	Interlock_to_SE(5)		<= SISDT_V;
	Interlock_to_SE(6)		<= TK_DT2V;	-- neu am 12.11.2007
	Interlock_to_SE(7)		<= TK3DT3P;	-- neu am 10.12.2007
	Interlock_to_SE(8)		<= UH4DT4P;
	Interlock_to_SE(9)		<= US4DT7P;
	Interlock_to_SE(10)		<= UT1DT0P;
	Interlock_to_SE(11)		<= TK3DT4P;
	Interlock_to_SE(12)		<= US_DT_E;
	Interlock_to_SE(13)		<= UA_DT_E;
	Interlock_to_SE(14)		<= TK_DT_E;
	Interlock_to_SE(15)		<= Chop_TK;


	--+-----------------------------------------------------------------------------+
	--|			Strahlalarm_In(15..0)' mit den Signalnamen belegen					|
	--+-----------------------------------------------------------------------------+
	--+---------------------------------------------+
	--|   Trafo-Verlust/Profilgitter-Interlock		|
	--+---------------------------------------------+
	UH_DT_V					<= not Strahlalarm_In(0);	-- Kein Strom an			--
	UA_DT_V					<= not Strahlalarm_In(1);	-- der Optokoppler-			--
	TK_DT1V					<= not Strahlalarm_In(2);	-- Anpasskarte 'OIKUI' 		--
	UXZDT_V					<= not Strahlalarm_In(3);	-- soll die Verriegelung	--
	TK8DT2V					<= not Strahlalarm_In(4);	-- auslösen. 				--
	SISDT_V					<= not Strahlalarm_In(5);	-- Dadurch ist bei			--
	TK_DT2V					<= not Strahlalarm_In(6);	-- abgezogen Kabel die		--
	Strahlalarm_Frei2		<= not Strahlalarm_In(7);	-- Schutzfunktion gewähr-	--
	UH4DT4P					<= not Strahlalarm_In(8);	-- leistet. Die Logik ist	--
	US4DT7P					<= not Strahlalarm_In(9);	-- für positive Signale		--
	UT1DT0P					<= not Strahlalarm_In(10);	-- definiert, deshalb sind	--
	TK3DT4P					<= not Strahlalarm_In(11);	-- alle Verlust-Eingänge	--
	US_DT_E					<= not Strahlalarm_In(12);	-- negiert.					--
	UA_DT_E					<= not Strahlalarm_In(13);	--							--
	TK_DT_E					<= not Strahlalarm_In(14);	--							--
	--Chop_TK					<= Strahlalarm_In(15);	--	not... entfernt 01.09.03, umgelegt am 12.11.2007
	TK3DT3P					<= not Strahlalarm_In(15);  --

	--+-----------------------------------------------------------------------------+
	--|	Trafosteuerpulse ('Trafo_Timing_Out(15..0)') mit den Signalnamen belegen	|
	--+-----------------------------------------------------------------------------+


	Trafo_Timing_Out(0)				<= Rahmen_UL when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(1)				<= Klemm_UL when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(2)				<= Rahmen_UR when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(3)				<= Klemm_UR when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(4)				<= Klemm_UH1 when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(5)				<= Rahmen_UH when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(6)				<= Klemm_UH when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(7)				<= Rahmen_UN when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(8)				<= Klemm_UN when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(9)				<= Rahmen_UA when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(10)			<= Klemm_UA when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(11)			<= Rahmen_TK8 when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(12)			<= Klemm_TK8 when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(13)			<= Rahmen_TK when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(14)			<= '0' when Skal_Ok = '1' else 'Z';
	Trafo_Timing_Out(15)			<= (IQ_R and Qu_R_HSI) or (IQ_L and Qu_L_HSI) when Skal_Ok = '1' else 'Z';	-- W.P. zur Diagnose ob Quelle Links	--
																				-- oder Rechts den HSI bedient.			--

	--+-----------------------------------------------------------------------------+
	--|				Beam_Control_In(15..0) mit den Signalnamen belegen				|
	--+-----------------------------------------------------------------------------+

	Chopper_HSI_act				<= Beam_Control_In(0); -- liefert den Istwert
	Chopper_HLI_act				<= Beam_Control_In(1); -- liefert den Istwert
	--TK_DT2V					<= Beam_Control_In(6); -- vorläufiger Name für Verlustüberwacher
	Chop_TK						<= Beam_Control_In(7); -- umgelegt von Strahlalarm_In(15) 12.11.2007

	--+-------------------------------------------------------------------------+
	--|   Signale der vier Pulszentralen (von Gatepulsgeneratoren erzeugt)		|
	--+-------------------------------------------------------------------------+

	Strahl_R					<= Beam_Control_In(8);
	IQ_R						<= Beam_Control_In(9);
	Strahl_L					<= Beam_Control_In(10);
	IQ_L						<= Beam_Control_In(11);
	Strahl_HLI					<= Beam_Control_In(12);
	Chopper_HLI					<= Beam_Control_In(13);
	Strahl_HSI					<= Beam_Control_In(14);
	Chopper_HSI					<= Beam_Control_In(15);



	--+-----------------------------------------------------------------------------+
	--|				Beam_Control_Out(15..0) mit den Signalnamen belegen				|
	--|				neue Belegung vom 12.09.06										|
	--+-----------------------------------------------------------------------------+



	Beam_Control_Out(0)				<= Chopp_HSI_On when Skal_Ok = '1' else 'Z';		-- Soll Signal an Netzgerät
	Beam_Control_Out(1)				<= HF_HSI_On when Skal_Ok = '1' else 'Z';
	Beam_Control_Out(2)				<= Chopper_HSI_act when Skal_Ok = '1' else 'Z';		-- Diagnose
	Beam_Control_Out(3)				<= INL_HSI_BL when Skal_Ok = '1' else 'Z';			-- Diagnose
	Beam_Control_Out(4)				<= No_Beam_HSI when Skal_Ok = '1' else 'Z';			-- Diagnose
	Beam_Control_Out(5)				<= No_ERR_HSI when Skal_Ok = '1' else 'Z';			-- Interlock Signal an Netzgerät
	Beam_Control_Out(6)				<= Chopp_HLI_On when Skal_Ok = '1' else 'Z';		-- Soll SIgnal an Netzgerät
	Beam_Control_Out(7)				<= Chopper_HLI_act when Skal_Ok = '1' else 'Z';		-- Diagnose
	Beam_Control_Out(8)				<= INL_HLI_BL when Skal_Ok = '1' else 'Z';			-- Diagnose
	Beam_Control_Out(9)				<= No_Beam_HLI when Skal_Ok = '1' else 'Z';			-- Diagnose
	Beam_Control_Out(10)			<= No_ERR_HLI when Skal_Ok = '1' else 'Z';			-- Interlock Signal an Netzgerät
	Beam_Control_Out(11)			<= HF_ALV_Off when Skal_Ok = '1' else 'Z';
	Beam_Control_Out(12)			<= Off_HSI when Skal_Ok = '1' else 'Z';				-- Diagnose
	Beam_Control_Out(13)			<= Off_ALV_PG when Skal_Ok = '1' else 'Z';			-- Diagnose
	Beam_Control_Out(14)			<= Off_ALV_Verlust when Skal_Ok = '1' else 'Z';		-- Diagnose
	Beam_Control_Out(15)			<= Off_Anforderung_In when Skal_Ok = '1' else 'Z';	-- Diagnose



	Klemm_UR_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_200us)
										PORT MAP	(	Clk => Clk,
														Reset => s_Logik_not_Sel_or_Reset,
														Edge_neg => IQ_R,
														Sig_out =>	Klemm_UR);

	Klemm_UL_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_200us)
										PORT MAP	(	Clk => Clk,
														Reset => s_Logik_not_Sel_or_Reset,
														Edge_neg => IQ_L,
														Sig_out =>	Klemm_UL);


	Klemm_UH1	<= (Qu_R_HSI and Klemm_UR) or (Qu_L_HSI and Klemm_UL);


	Klemm_UH_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_50us)
										PORT MAP	(	Clk => Clk,
														Reset => s_Logik_not_Sel_or_Reset,
														Edge_neg => Chopper_HSI,
														Sig_out =>	Klemm_UH_delayed);


	Klemm_UH <=  Klemm_UH_delayed and not Rahmen_UH_after_20us;
	--Klemm_UH <=  Klemm_UH_delayed or Rahmen_UH_after_20us;


	Klemm_UN_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_50us)
											PORT MAP	(	Clk => Clk,
															Reset => s_Logik_not_Sel_or_Reset,
															Edge_neg => Chopper_HLI,
															Sig_out =>	Klemm_UN_delayed);

	Klemm_UN <= Klemm_UN_delayed and not Rahmen_UN_after_20us;


	Chopper_HSI_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_50us)
											PORT MAP	(	Clk => Clk,
															Reset => s_Logik_not_Sel_or_Reset,
															Edge_pos => Chopper_HSI,
															Sig_out =>	Chopper_HSI_delayed);

	Chopper_HLI_dly:		pos_or_neg_dly	GENERIC MAP (	Edge_delay_cnt => C_Edge_delay_cnt_50us)
											PORT MAP	(	Clk => Clk,
															Reset => s_Logik_not_Sel_or_Reset,
															Edge_pos => Chopper_HLI,
															Sig_out =>	Chopper_HLI_delayed);


	-- 10us puls after Strahl_HSI
	Rahmen_UH_min_dly:	Puls	 		GENERIC MAP (	delay_cnt => C_Edge_delay_cnt_10us)
										PORT MAP	( 	Clk => Clk,
														Reset => s_Logik_not_Sel_or_Reset,
														Pos_Edge => Strahl_HSI,
														Puls => Rahmen_UH_min );
	-- 10us puls after Strahl_HSI
	puls_hsi:	Puls	 		GENERIC MAP (	delay_cnt => C_Edge_delay_cnt_10us)
										PORT MAP	( 	Clk => Clk,
														Reset => s_trunc_reset,
														Pos_Edge => Chopper_HSI_delayed,
														Puls => mask_10us_hsi );

	-- 10us puls after Strahl_HLI
	puls_hli:	Puls	 		GENERIC MAP (	delay_cnt => C_Edge_delay_cnt_10us)
										PORT MAP	( 	Clk => Clk,
														Reset => s_trunc_reset,
														Pos_Edge => Chopper_HLI_delayed,
														Puls => mask_10us_hli );

	-- 10us puls after Chopper_HSI
	puls_hsi_anf:	Puls	 		GENERIC MAP (	delay_cnt => C_Edge_delay_cnt_10us)
										PORT MAP	( 	Clk => Clk,
														Reset => s_trunc_reset,
														Pos_Edge => Chopper_HSI_delayed,
														Puls => puls_10us_after_chopp_hsi );

	-- 10us puls after Chopper_HLI
	puls_hli_anf:	Puls	 		GENERIC MAP (	delay_cnt => C_Edge_delay_cnt_10us)
										PORT MAP	( 	Clk => Clk,
														Reset => s_trunc_reset,
														Pos_Edge => Chopper_HLI_delayed,
														Puls => puls_10us_after_chopp_hli );

	edge_chopp_hsi: edge_detection port map (
							clk => clk,
							reset => s_trunc_reset,
							input => Chopper_HSI_delayed,
							pos_edge => chopp_hsi_pos
						);

	edge_chopp_hli: edge_detection port map (
							clk => clk,
							reset => s_trunc_reset,
							input => Chopper_HLI_delayed,
							pos_edge => chopp_hli_pos
						);


	s_trunc_reset <= s_Logik_not_Sel_or_Reset or Strahlweg_Reg_WR;

	-- 20us nach Ende Rahmenpuls High Pegel
	trunc_ua:	trunc			generic map (	delay_cnt => c_edge_delay_cnt_20us )
								port	map (	clk => clk,
												reset => s_trunc_reset,
												neg_edge => Rahmen_UA,
												trunc => Rahmen_UA_after_20us );

	-- 20us nach Ende Rahmenpuls High Pegel
	trunc_un:	trunc			generic map (	delay_cnt => c_edge_delay_cnt_20us )
								port	map (	clk => clk,
												reset => s_trunc_reset,
												neg_edge => Rahmen_UN,
												trunc => Rahmen_UN_after_20us );

	-- 20us nach Ende Rahmenpuls High Pegel
	trunc_uh:	trunc			generic map (	delay_cnt => c_edge_delay_cnt_20us )
								port	map (	clk => clk,
												reset => s_trunc_reset,
												neg_edge => Rahmen_UH,
												trunc => Rahmen_UH_after_20us );
	--------------------------------------------------------------------------------------


	--------------------------------------------------------------------------------------
	-- Entprellung und Ausrichtung der Anforder Signale
	-- Off_Anforderung_In und Off_Anfoderung_UU aus der Logikkarte 2
	--------------------------------------------------------------------------------------

	align_anf_uu: process (clk)
	begin
		if rising_edge(clk) then
			if s_trunc_reset = '1' then
				s_anforderung_aligned_uu <= '0';
			elsif (Off_UU_In = '0') and chopp_hli_pos = '1' then
				s_anforderung_aligned_uu <= '1';
			elsif (Off_UU_In = '1') and puls_10us_after_chopp_hli = '0' then
				s_anforderung_aligned_uu <= '0';
			elsif Chopper_HLI_delayed = '0' and puls_10us_after_chopp_hli = '0' then
				s_anforderung_aligned_uu <= '0';
			end if;
		end if;
	end process align_anf_uu;


	align_anf_hsi: process (clk)
	begin
		if rising_edge(clk) then
			if s_trunc_reset = '1' then
				s_anforderung_aligned_hsi <= '0';
			elsif (Off_Anforderung_In = '0' and HSI_ALV = '1') and chopp_hsi_pos = '1' then					-- Vers_13: ...and HSI_ALV = '1'
				s_anforderung_aligned_hsi <= '1';
			elsif (Off_Anforderung_In = '1' and HSI_ALV = '1') and puls_10us_after_chopp_hsi = '0' then		-- Vers_13: ...and HSI_ALV = '1'
				s_anforderung_aligned_hsi <= '0';
			elsif Chopper_HSI_delayed = '0' and puls_10us_after_chopp_hsi = '0' then
				s_anforderung_aligned_hsi <= '0';
			end if;
		end if;
	end process align_anf_hsi;

	align_anf_hli: process (clk)
	begin
		if rising_edge(clk) then
			if s_trunc_reset = '1' then
				s_anforderung_aligned_hli <= '0';
			elsif (Off_Anforderung_In = '0' and HLI_ALV = '1') and chopp_hli_pos = '1' then					-- Vers_13: ...and HLI_ALV = '1'
				s_anforderung_aligned_hli <= '1';
			elsif (Off_Anforderung_In = '1' and HLI_ALV = '1') and puls_10us_after_chopp_hli = '0' then		-- Vers_13: ...and HLI_ALV = '1'
				s_anforderung_aligned_hli <= '0';
			elsif Chopper_HLI_delayed = '0' and puls_10us_after_chopp_hli = '0' then
				s_anforderung_aligned_hli <= '0';
			end if;
		end if;
	end process align_anf_hli;


	Off_ALV_PG	<=	(US4DT7P and not Mask_US4DT7P)
				or	(UT1DT0P and not Mask_UT1DT0P)
				or	(TK3DT4P and not Mask_TK3DT4P)
				or	(UA_DT_E and not Mask_UA_DT_E)		-- die... E-Trafos machen PG-Schutz
				or	(TK_DT_E and not Mask_TK_DT_E)
				or	(TK3DT3P and not Mask_TK3DT3P);

	Off_HSI_PG	<=	(UH4DT4P and not Mask_UH4DT4P)
				or	(US_DT_E and not Mask_US_DT_E);


	Off_ALV_Verlust	<=	(UA_DT_V or TK_DT1V or UA_DT_E or TK_DT_E or TK_DT2V)
					or	(TK8DT2V and not Mask_TK8DT2V)
					or	(SISDT_V and Request_SIS);

	Off_ALV			<=	Off_ALV_PG or ( Off_ALV_Verlust AND HSI_ALV);


	-- Die Verkürzung durch die Strahldiagnose wird auf 10µs Mindestpulslänge beschränkt
	Off_ALV_min		<= Off_ALV and not (mask_10us_hli or mask_10us_hsi);	-- mask Off_ALV for 10us

	Off_HSI_Verlust	<= UH_DT_V or US_DT_E;

	Off_HSI			<=	Off_HSI_Verlust or Off_HSI_PG;

	-- Die Verkürzung durch die Strahldiagnose wird auf 10µs Mindestpulslänge beschränkt
	Off_HSI_min		<= Off_HSI and not mask_10us_hsi;	-- mask Off_HSI for 10us

	-- Bei Strahlzielen hinter Alvarez können die Anforderungs-Signale auf die Verkürzung einwirken
	-- Ansonsten wirkt nur die Verkürzung der Strahldiagnose (Verlust und PG-Schutz)
	Verkuerzung_HSI <= ((Off_ALV_min or not s_anforderung_aligned_hsi) and HSI_ALV) or Off_HSI_min;

	-- Bei Strahlzielen hinter Alvarez können die Anforder Signale auf die Verkürzung einwirken
	-- Ansonsten wirkt das Anfoderungs-Signal von UU
	Verkuerzung_HLI <= ((Off_ALV_min or not s_anforderung_aligned_hli) and HLI_ALV) or not s_anforderung_aligned_uu;

	-- Die Signale Block_* und INL_* kommen von der PZU und können den Strahl für den Zyklus abschalten
	-- Das Signal s_watch_timeout stammt vom Watchdog und hat den selben Effekt
	No_Beam_HSI	<=	Verkuerzung_HSI or Block_HSI or INL_HSI_BL or s_watch_timeout;

	No_Beam_HLI	<=	Verkuerzung_HLI or Block_HLI or INL_HLI_BL or s_watch_timeout;

	Rahmen_UH	<=	((Strahl_HSI  and not((Off_ALV and HSI_ALV) or Off_HSI)) or Rahmen_UH_min) and not Block_HSI;

	Rahmen_UN	<=	Strahl_HLI and not((Off_ALV and HLI_ALV) or Block_HLI);

	-- KLemm_UA überlappt den Rahmen_UA um 20us
	Klemm_UA	<=	((Klemm_UN and HLI_ALV) or (Klemm_UH and HSI_ALV)) and not rahmen_ua_after_20us;

	Rahmen_UA	<=	(Rahmen_UN and HLI_ALV) or (Rahmen_UH and HSI_ALV);

	Rahmen_UR	<=	Strahl_R;

	Rahmen_UL	<=	Strahl_L;

-- 	V03  25.9.06
	s_R_TK8 <= Rahmen_TK and not Off_ALV;	-- z.B. Ziel_TK

	-- TK9DC9_out ist 0-Aktiv
	Rahmen_TK8 <= (Rahmen_UA and Chop_TK and not Off_ALV) when TK9DC9_out = '1' and Enable_TK = '1' else
					Rahmen_TK8_delayed when TK9DC9_out = '0' else
					'0';

	TK8_delayed:	var_delay  port map (   Signal_in => s_R_TK8,
											delay => s_TK8_delay,
											Clk => Clk,
											Reset => Reset,
											en_100ns => s_100ns_en,
											Signal_out => Rahmen_TK8_delayed
										);


	Klemm_TK8 <= Klemm_UA when ( Klemm_UA = '1' and not (Request_SIS = '0' and TK9DC9_out = '0')) else '0';

	Chopp_HSI_On	<= Chopper_HSI_delayed and not No_beam_HSI;	-- Vers_14 Verknüpfung mit dem Event

	Chopp_HLI_On	<= Chopper_HLI_delayed and not No_Beam_HLI; -- Vers_14 Verknüpfung mit dem Event

--	HF_HSI_On	<=	not Off_HSI; ------------------------------ V01
	HF_HSI_On	<=	not ((Off_HSI or Off_ALV) and KLEMM_UH); -- V02

--	HF_ALV_On	<=	not Off_ALV; ----------- V01
	HF_ALV_Off	<=	Off_ALV AND KLEMM_UA; -- V02

	Mask_TK8DT2V	<= Request_SIS and TK9DC9_out;

	Rahmen_TK	<= Rahmen_UA and Enable_TK; -- neu am 12.11.2007


	---------------------------------------------------------------
	-- Signale für die 16 Leds
	---------------------------------------------------------------

	Chop_m1_LEDs(15) <= Mask_TK8DT2V;
	Chop_m1_LEDs(14) <= Mask_TK3DT4P;
	Chop_m1_LEDs(13) <= Mask_US4DT7P;
	Chop_m1_LEDs(12) <= Mask_UT1DT0P;
	Chop_m1_LEDs(11) <= Mask_UH4DT4P;
	Chop_m1_LEDs(10) <= Mask_TK_DT_E;
	Chop_m1_LEDs(9)	 <= Mask_UA_DT_E;
	Chop_m1_LEDs(8)  <= Mask_US_DT_E;
	Chop_m1_LEDs(7)  <= Off_UU_In;
	Chop_m1_LEDs(6)  <= Off_Anforderung_In;
	Chop_m1_LEDs(5)  <= Block_HSI;
	Chop_m1_LEDs(4)  <= Block_HLI;
	Chop_m1_LEDs(3)	 <= INL_HLI_BL;
	Chop_m1_LEDs(2)	 <= INL_HSI_BL;
	Chop_m1_LEDs(1)	 <= HLI_ALV;
	Chop_m1_LEDs(0)	 <= HSI_ALV;

end chopper_m1_logic_arch;


