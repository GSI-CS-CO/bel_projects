--TITLE "'atr_timeout' Autor: K.Kaiser";

LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.ALL;


ENTITY atr_timeout IS
	PORT (
		nReset              : IN std_logic;
		ATR_Timeout_err_res : IN std_logic;
		clk                 : IN STD_LOGIC;
		atr_puls_start      : IN STD_LOGIC;                      -- Triggerpuls
		Strobe_o            : IN STD_LOGIC;                      -- Puls aus Rückmeldung (Komparatoreingang)
		S_ATR_to_Count      : IN STD_LOGIC_VECTOR(15 DOWNTO 0);  -- Registervorgabe Timeoutzeit

		ATR_Timeout         : OUT STD_LOGIC;                     -- ATR Timeout Error zwischen Trigger und Zündpulsrückmeldung
		ATR_to_conf_err     : OUT STD_LOGIC                      -- ATR Falsche Registervorgabe für Timeout Zeit
	);
END atr_timeout;

ARCHITECTURE Arch_atr_timeout OF atr_timeout IS
	SIGNAL Loop_Counter : INTEGER RANGE 0 TO 65535; -- 0-FFFF -- Counter
	SIGNAL Timeout_Cnt  : INTEGER RANGE 0 TO 65535; -- 0-FFFF -- Counter
	SIGNAL s_config_err : std_logic; -- Config-Error
	SIGNAL s_timeout    : std_logic; -- Timout-Flag
	TYPE   type_t IS (sm_idle, sm_start, sm_count, sm_loop, sm_config_err, sm_timeout, sm_end);
	SIGNAL sm_state : type_t := sm_idle;
	---------------------------------------------------------------------------------------------------
BEGIN
------------------------------------- Timeout -----------------------------------------
P_Timeout_SM : PROCESS (clk, nReset)
BEGIN
	IF (nReset = '0') THEN
		Timeout_Cnt  <= 0;   -- Registervorgabe Timoutzeit
		Loop_Counter <= 0;   -- Timout_Counter
		s_config_err <= '0'; -- Config-Error: Pulsbreite/Pulsverzögerung
		s_timeout    <= '0'; -- Timout-Flag
		sm_state     <= sm_idle;
	ELSIF rising_edge(clk) THEN
		CASE sm_state IS
			WHEN sm_idle =>
			  IF atr_puls_start = '1' THEN
				  Timeout_Cnt  <= to_integer(unsigned(S_ATR_to_Count)(15 DOWNTO 0)); -- Registervorgabe Timoutzeit
			  	Loop_Counter <= 0;                                                 -- Timeout_Counter
			  	sm_state     <= sm_start;
        ELSE
          sm_state     <= sm_idle;
        END IF;

			WHEN sm_start =>
				sm_state <= sm_count;

			WHEN sm_count =>
			  IF (Timeout_Cnt = 0) THEN -- Timeout-Zeit = 0
			    sm_state <= sm_config_err;
			  ELSE
			    sm_state <= sm_loop;
	      END IF;

			WHEN sm_loop =>
			  IF (Loop_Counter < Timeout_Cnt) THEN -- LoopCounter kleiner Timeout-Zeit
		      IF (Strobe_o = '1') THEN -- Strobe aus ATR_comp_puls
			      sm_state <= sm_end; -- Puls kommt vor der Timeout-Zeit ==> ok
			    ELSE
		      	Loop_Counter <= (Loop_Counter + 1);
			      sm_state     <= sm_loop;
		      END IF;
        ELSE
	        sm_state <= sm_timeout;
        END IF;

      WHEN sm_config_err => s_config_err <= '1'; -- Set: Config-Error: Pulsbreite/Pulsverzögerung
			  sm_state          <= sm_end;

			WHEN sm_timeout =>
			  s_timeout         <= '1'; -- Set: Timout-Flag
			  sm_state          <= sm_end;

      WHEN sm_end =>
        s_config_err      <= '0'; -- Reset: Config-Error: Pulsbreite/Pulsverzögerung
			  s_timeout         <= '0'; -- Reset: Timout-Flag
			  sm_state          <= sm_idle;
      WHEN OTHERS =>
        sm_state           <= sm_idle;
    END CASE;
  END IF;
END PROCESS P_Timeout_SM;


-- ATR_to_conf_err <= s_config_err; -- Time-Out: Configurations-Error
-- ATR_Timeout <= s_timeout; -- Time-Out: Maximalzeit zwischen Start und Zündpuls überschritten.


P_SaveErrors : PROCESS (clk, nReset, ATR_Timeout_err_res)
BEGIN
	IF ((nReset = '0') OR (ATR_Timeout_err_res = '1')) THEN
		ATR_to_conf_err   <= '0';
    ATR_Timeout       <= '0';
	ELSIF rising_edge(clk) THEN
		IF s_config_err = '1' THEN
			ATR_to_conf_err <= '1';
		END IF;
    IF s_timeout = '1' THEN
			ATR_Timeout <= '1';
		END IF;
	END IF;
END PROCESS P_SaveErrors;



END Arch_atr_timeout;