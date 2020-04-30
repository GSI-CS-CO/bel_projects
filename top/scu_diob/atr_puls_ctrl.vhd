--TITLE "'atr_puls_ctrl' Autor: R.Hartmann";

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;

ENTITY atr_puls_ctrl IS
	generic
		(
		Base_addr:	               INTEGER := 16#0618#
		);

	port(
		Adr_from_SCUB_LA:		       IN  STD_LOGIC_VECTOR(15 DOWNTO 0);	  -- Latched Adr SCU_Bus Extension
		Data_from_SCUB_LA:	       IN  STD_LOGIC_VECTOR(15 DOWNTO 0);	  -- Latched Data SCU_Bus Extension
		Ext_Adr_Val:			         iN  STD_LOGIC;								        -- '1':"ADR_from_SCUB_LA" valid
		Ext_Rd_active:			       iN  STD_LOGIC;								        -- '1':Rd-Cycle active
		Ext_Rd_fin:				         iN  STD_LOGIC;								        -- End of read cycle, high for one sysclk period
		Ext_Wr_active:			       iN  STD_LOGIC;								        -- '1':Wr-Cycle active
		Ext_Wr_fin:				         iN  STD_LOGIC;								        -- End of write cycle, high for one sysclk period
		clk:						           in  STD_LOGIC;								        -- SHOULD be the same clk as used by SCU_Bus_Slave macro
		nReset:					           in  STD_LOGIC;
		clk_250mhz:			           IN  STD_LOGIC;								        -- Signaltap clk 250Mhz
		nReset_250mhz:	           IN  STD_LOGIC;
--
    atr_puls_start:            IN  STD_LOGIC;                       -- ATR Triggerpuls aus fallender Flanke (Lemo ATR IN TRIGGER)
    ATR_largepulse_en_7_0 :    IN  STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Kanalweises Enable für Largepulse Option Zündpulslänge (wirkt nicht auf Delay!)
    ATR_Tag_X_En_8_1:          IN  STD_LOGIC_VECTOR(8 DOWNTO 1);    -- Selektiert Timing Tags 8..1 als Triggerquelle für ATR Zündpuls Trigger
    ATR_TRIG_IN_Dis :          IN  STD_LOGIC;                       -- Disabled Lemo "ATR IN Trigger" als Zündquelle, stattdessen Timing Tags 1..8 oder Lemo "ATR In 1..8"
    ATR_TimingTags_8_1 :       IN  STD_LOGIC_VECTOR(8 DOWNTO 1);    -- Matching Timing (1 Sysclk lang) Tag Pulse als Triggerquelle
    Syn_ATR_Comp_in_puls_8_1:  IN  STD_LOGIC_VECTOR(8 DOWNTO 1);    -- Trigger Pulse (1 Sysclk lang) aus ATR In Lemos  1..8
    Tags_Only:                 IN  STD_LOGIC;

    atr_puls_out:              OUT STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Ausgangspuls Kanal 1..8
    atr_puls_config_err:       OUT STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Config-Error: Pulsbreite/Pulsverzögerung

    ATR_comp_puls:             IN  STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Ausgänge von den Comperatoren für die Triggereingänge
    ATR_to_conf_err_7_0:       OUT STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Time-Out: Configurations-Error Keine Timeout-Vorgabe
		ATR_Timeout_7_0:  		     OUT STD_LOGIC_VECTOR(7 DOWNTO 0);    -- Time-Out: Maximalzeit zwischen Zündpuls und Rückmeldung überschritten.
    ATR_Timeout_err_res:       IN  STD_LOGIC;                       -- Reset Error-Flag
--
		Reg_rd_active:		         OUT STD_LOGIC;								        -- Read data available at 'Data_to_SCUB'
		Data_to_SCUB:		           OUT STD_LOGIC_VECTOR(15 DOWNTO 0);	  -- Lesebus für SCU_BUS_Slave Macro
		Dtack_to_SCUB:		         OUT STD_LOGIC								        -- Dtack für SCU_Bus_Slave Macro
		);
	end atr_puls_ctrl;



ARCHITECTURE Arch_atr_puls_ctrl OF atr_puls_ctrl IS

CONSTANT	Addr_width:					        INTEGER := Adr_from_SCUB_LA'length;
CONSTANT	ATR_verz_0_addr_offset:	    INTEGER := 0;		-- Offset zur Base_Adr WR/RD ATR_verz_0 Reg Ch1
CONSTANT	ATR_verz_1_addr_offset:	    INTEGER := 1;		-- Offset zur Base_Adr WR/RD ATR_verz_1 Reg Ch2
CONSTANT	ATR_verz_2_addr_offset:	    INTEGER := 2;		-- Offset zur Base_Adr WR/RD ATR_verz_2 Reg Ch3
CONSTANT	ATR_verz_3_addr_offset:	    INTEGER := 3;		-- Offset zur Base_Adr WR/RD ATR_verz_3 Reg Ch4
CONSTANT	ATR_verz_4_addr_offset:	    INTEGER := 4;		-- Offset zur Base_Adr WR/RD ATR_verz_4 Reg Ch5
CONSTANT	ATR_verz_5_addr_offset:	    INTEGER := 5;		-- Offset zur Base_Adr WR/RD ATR_verz_5 Reg Ch6
CONSTANT	ATR_verz_6_addr_offset:	    INTEGER := 6;		-- Offset zur Base_Adr WR/RD ATR_verz_6 Reg Ch7
CONSTANT	ATR_verz_7_addr_offset:	    INTEGER := 7;		-- Offset zur Base_Adr WR/RD ATR_verz_7 Reg Ch8

CONSTANT	ATR_pulsw_0_addr_offset:	  INTEGER := 8;		-- Offset zur Base_Adr WR/RD ATR_pulsw-0 Reg Ch1
CONSTANT	ATR_pulsw_1_addr_offset:	  INTEGER := 9;		-- Offset zur Base_Adr WR/RD ATR_pulsw-1 Reg Ch2
CONSTANT	ATR_pulsw_2_addr_offset:	  INTEGER := 10;	-- Offset zur Base_Adr WR/RD ATR_pulsw-2 Reg Ch3
CONSTANT	ATR_pulsw_3_addr_offset:	  INTEGER := 11;	-- Offset zur Base_Adr WR/RD ATR_pulsw-3 Reg Ch4
CONSTANT	ATR_pulsw_4_addr_offset:	  INTEGER := 12;	-- Offset zur Base_Adr WR/RD ATR_pulsw-4 Reg Ch5
CONSTANT	ATR_pulsw_5_addr_offset:	  INTEGER := 13;	-- Offset zur Base_Adr WR/RD ATR_pulsw-5 Reg Ch6
CONSTANT	ATR_pulsw_6_addr_offset:	  INTEGER := 14;	-- Offset zur Base_Adr WR/RD ATR_pulsw-6 Reg Ch7
CONSTANT	ATR_pulsw_7_addr_offset:	  INTEGER := 15;	-- Offset zur Base_Adr WR/RD ATR_pulsw-7 Reg Ch8


CONSTANT	ATR_to_count_0_addr_offset:	INTEGER := 16;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch1
CONSTANT	ATR_to_count_1_addr_offset:	INTEGER := 17;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch2
CONSTANT	ATR_to_count_2_addr_offset:	INTEGER := 18;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch3
CONSTANT	ATR_to_count_3_addr_offset:	INTEGER := 19;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch4
CONSTANT	ATR_to_count_4_addr_offset:	INTEGER := 20;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch5
CONSTANT	ATR_to_count_5_addr_offset:	INTEGER := 21;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch6
CONSTANT	ATR_to_count_6_addr_offset:	INTEGER := 22;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch7
CONSTANT	ATR_to_count_7_addr_offset:	INTEGER := 23;	-- Offset zur Base_Adr WR/RD ATR_Timeout_Counter Ch8

--
CONSTANT	C_ATR_verz_0_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_verz_0_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_0 Reg Ch1
CONSTANT	C_ATR_verz_1_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_1_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_1 Reg Ch2
CONSTANT	C_ATR_verz_2_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_verz_2_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_2 Reg Ch3
CONSTANT	C_ATR_verz_3_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_3_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_3 Reg Ch4
CONSTANT	C_ATR_verz_4_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_4_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_4 Reg Ch5
CONSTANT	C_ATR_verz_5_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_5_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_5 Reg Ch6
CONSTANT	C_ATR_verz_6_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_6_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_6 Reg Ch7
CONSTANT	C_ATR_verz_7_Addr: 	        UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_7_addr_offset), addr_width);	  -- Adr WR/RD ATR_verz_7 Reg Ch8

CONSTANT	C_ATR_pulsw_0_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_pulsw_0_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_0 Reg Ch1
CONSTANT	C_ATR_pulsw_1_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_1_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_1 Reg Ch2
CONSTANT	C_ATR_pulsw_2_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_pulsw_2_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_2 Reg Ch3
CONSTANT	C_ATR_pulsw_3_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_3_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_3 Reg Ch4
CONSTANT	C_ATR_pulsw_4_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_4_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_4 Reg Ch5
CONSTANT	C_ATR_pulsw_5_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_5_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_5 Reg Ch6
CONSTANT	C_ATR_pulsw_6_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_6_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_6 Reg Ch7
CONSTANT	C_ATR_pulsw_7_Addr:         UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_7_addr_offset), addr_width);	-- Adr WR/RD ATR_pulsw_7 Reg Ch8


CONSTANT	C_ATR_to_count_0_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_0_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch1
CONSTANT	C_ATR_to_count_1_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_1_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch2
CONSTANT	C_ATR_to_count_2_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_2_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch3
CONSTANT	C_ATR_to_count_3_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_3_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch4
CONSTANT	C_ATR_to_count_4_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_4_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch5
CONSTANT	C_ATR_to_count_5_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_5_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch6
CONSTANT	C_ATR_to_count_6_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_6_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch7
CONSTANT	C_ATR_to_count_7_Addr:     	UNSIGNED(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_7_addr_offset), addr_width);	-- Adr WR/RD ATR_Timeout_Counter  Ch8

SIGNAL		S_ATR_verz_rd:	            STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_verz_wr:	            STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_pulsw_rd:	            STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_pulsw_wr:	            STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_to_Count_rd:	        STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_to_Count_wr:	        STD_LOGIC_VECTOR(15 DOWNTO 0);



SIGNAL		S_ATR_to_kanal_rd:	        STD_LOGIC;
SIGNAL		S_ATR_to_kanal_wr:	        STD_LOGIC;
SIGNAL		S_ATR_to_kanal:	            STD_LOGIC_VECTOR(15 DOWNTO 0);


SIGNAL		ATR_TRIGGER_8_1:	          STD_LOGIC_VECTOR(8 DOWNTO 1);
SIGNAL		ATR_TRIGGER_TEMP_8_1:	      STD_LOGIC_VECTOR(8 DOWNTO 1);
SIGNAL    Syn_ATR_Comp_in_puls_8_1_gated : STD_LOGIC_VECTOR(8 DOWNTO 1);

SIGNAL		S_Dtack:				            STD_LOGIC;
SIGNAL		S_Read_Port:		            STD_LOGIC_VECTOR(DATa_to_SCUB'RANGE);

SIGNAL		ATR_verz:                   STD_LOGIC_VECTOR(15 DOWNTO 0);          -- Counter für Verzögerung
SIGNAL		ATR_pulsw:                  STD_LOGIC_VECTOR(15 DOWNTO 0);          -- Counter für Pulsbreite


SIGNAL Puls_i:                        STD_LOGIC_VECTOR(7 DOWNTO 0);           -- Input  "Strobe-Signal"
SIGNAL Strobe_o:                      STD_LOGIC_VECTOR(7 DOWNTO 0);           -- Output "Strobe-Signal (1 CLK breit)"
SIGNAL Strobe_o_timeout:              STD_LOGIC_VECTOR(7 DOWNTO 0);           -- Output "Strobe-Signal (1 CLK breit)"

SIGNAL shift0:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift1:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift2:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift3:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift4:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift5:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift6:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.
SIGNAL shift7:                        STD_LOGIC_VECTOR(2 DOWNTO 0);           -- Shift-Reg.

SIGNAL atr_puls_start_7_0:            STD_LOGIC_VECTOR(7 DOWNTO 0);           -- Kanalweise Startpulse (Ch8=Index 7.... Ch1= Index 0)




TYPE      t_Word_Array    IS ARRay (0 to 7) of STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL		S_ATR_verz:     t_WORD_ARRAY;                                       -- Speicherarray für Zündpulsverzögerungszeitvorgabe
SIGNAL		S_ATR_pulsw:    t_WORD_ARRAY;                                       -- Speicherarray für Zündpulsbreite
SIGNAL		S_ATR_to_Count: t_WORD_ARRAY;                                       -- Speicherarray für Timeoutvorgabe (Rückmeldung

------------------------------------------------------------------------------------------

COMPONENT atr_puls_n
  PORT
  (
    clk:                  IN  STD_LOGIC;
    nReset:               IN  STD_LOGIC;
--
    atr_puls_start:       IN  STD_LOGIC;                      -- Trigger für Start Zündpulsgenerierung
  	ATR_verz:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);  -- Counter für Verzögerung
  	ATR_pulsw:            IN  STD_LOGIC_VECTOR(15 DOWNTO 0);  -- Counter für Pulsbreite
		largepulse_en:        IN  STD_LOGIC;                      -- ermöglich 1000fache Pulsbreite bei PB_Cnt
--
    atr_puls_out:         OUT STD_LOGIC;                      -- Zündpuls Kanal n
    atr_puls_config_err:  OUT STD_LOGIC);                     -- Config-Error:  Zündpulsbreite/Zündpulsverzögerung

END COMPONENT atr_puls_n;



COMPONENT atr_timeout IS
	PORT (
		nReset              : IN STD_LOGIC;
		ATR_Timeout_err_res : IN STD_LOGIC;
		clk                 : IN STD_LOGIC;
		atr_puls_start      : IN STD_LOGIC;                      -- Triggerpuls
    Strobe_o            : IN STD_LOGIC;                      -- Puls aus Rückmeldung (Komparatoreingang)
		S_ATR_to_Count      : IN STD_LOGIC_VECTOR(15 DOWNTO 0);  -- Registervorgabe Timeoutzeit
		ATR_Timeout         : OUT STD_LOGIC;                     -- ATR Timeout Error zwischen Trigger und Zündpulsrückmeldung
		ATR_to_conf_err     : OUT STD_LOGIC                      -- ATR keine Zeitvorgabe für Timeout eingetragen
	);
END COMPONENT atr_timeout;
---------------------------------------------------------------------------------------------------



BEGIN

------------------------------------ Multiplexer für Option "kanalweise getrennte ATR Triggerpulse" ------------------------



atr_puls_start_7_0 <= ATR_TRIGGER_8_1;  --Also: Index 7 gehört zu Startpuls Ch.8 , Index 0 zu Ch.1




Trigger_Input_Select : FOR i IN 1 TO 8 GENERATE
  Trigger_Inputs_MUX : PROCESS (Syn_ATR_Comp_in_puls_8_1_gated, ATR_TimingTags_8_1, ATR_Tag_X_En_8_1)
  BEGIN
    IF ATR_Tag_X_En_8_1(i) = '1' THEN
      ATR_TRIGGER_TEMP_8_1(i) <= ATR_TimingTags_8_1(i);
    ELSE
      ATR_TRIGGER_TEMP_8_1(i) <= Syn_ATR_Comp_in_puls_8_1_gated(i);
    END IF;
  END PROCESS Trigger_Inputs_MUX;
  END GENERATE Trigger_Input_Select;

ATR_TRIGGER_PULS_Select : FOR i IN 1 TO 8 GENERATE
  ATR_TRIGGER_MUX : PROCESS (ATR_TRIGGER_TEMP_8_1, ATR_TRIG_IN_Dis, atr_puls_start)
  BEGIN
    IF ATR_TRIG_IN_Dis = '0' THEN
      ATR_TRIGGER_8_1(i) <= atr_puls_start;
    ELSE
      ATR_TRIGGER_8_1(i) <= ATR_TRIGGER_TEMP_8_1(i);
    END IF;
  END PROCESS ATR_TRIGGER_MUX;
END GENERATE ATR_TRIGGER_PULS_Select;


----- Multiplexer für die Zuordnung von Rückmelde Strobes zu den Timeout Überwachungen
  ATR_Strobe_o_MUX : PROCESS (Tags_Only,strobe_o,Syn_ATR_Comp_in_puls_8_1,ATR_TRIG_IN_Dis)
  BEGIN
    IF Tags_Only = '1' or ATR_TRIG_IN_Dis = '0' THEN                   -- Timing Tags genutzt oder gemeinsamer Trigger genutzt
      strobe_o_timeout               <= strobe_o;
      Syn_ATR_Comp_in_puls_8_1_gated <= Syn_ATR_Comp_in_puls_8_1;
    ELSE                                                               -- Alle anderen Fälle wie Mischbetrieb gemappt
      strobe_o_timeout(7 downto 4) <= "0000";                          -- Pulsausgänge für Kanal 5..8 geerdet
      strobe_o_timeout(3 downto 0) <= strobe_o(7 downto 4);
      Syn_ATR_Comp_in_puls_8_1_gated(8 downto 5) <= "0000";
      Syn_ATR_Comp_in_puls_8_1_gated(4 downto 1) <= Syn_ATR_Comp_in_puls_8_1 (4 downto 1);

    END IF;
  END PROCESS ATR_Strobe_o_MUX;





------------------------------------- Outpuls und Timeout  Kanal 1-8 -----------------------------------------



ATR_Puls:  for I in 0 to 7 generate
    Puls_I:  atr_puls_n
          port map
                 (clk                   => clk_250mhz,                -- Clock
                  nReset                => nReset_250mhz,             -- Powerup-Reset (250Mhz)
                  atr_puls_start        => atr_puls_start_7_0(i),     -- Triggerpuls
                  ATR_verz              => s_ATR_verz(i),             -- Counter für Verzögerung
                  ATR_pulsw             => s_ATR_pulsw(i),            -- Counter für Pulsbreite
                  largepulse_en         => ATR_largepulse_en_7_0(i),  -- ermöglich 1000fache Pulsbreite an atr_puls_out(i)
                                                                      -- die Triggerverzögerung s_ATR_verz bleibt davon unberührt
                                                                      -- ATR_largepulse_en_7_0 wird durch AW-Config2(0) eingestellt
                                                                      -- und ist nach Reset disabled
                                                                      -- Es werden nur die Kanäle 7,6 und 4,3 beeinflusst.

                  atr_puls_out          => atr_puls_out(i),           -- Ausgangspuls Kanal n
                  atr_puls_config_err   => atr_puls_config_err(i)     -- Config-Error: Pulsbreite/Pulsverzögerung
           );
          end generate ATR_Puls;



ATR_Timeout_gen : For I in 0 to 7 generate
            timeout: atr_timeout
               port map(
                 clk                   => clk_250mhz,              -- Clock
                 nReset                => nReset_250mhz,           -- Powerup-Reset (250Mhz)
                 ATR_Timeout_err_res   => ATR_Timeout_err_res,     -- Aus AW_Config1(6) Register
                 atr_puls_start        => atr_puls_start_7_0(i),   -- Triggerpuls
                 strobe_o              => strobe_o_timeout(i),     -- Strobe aus Rückmeldung (Komparatoreingang)
                 S_ATR_to_Count        => S_ATR_to_Count(i),       -- Registervorgabe Timeoutzeit

                 ATR_Timeout           => ATR_Timeout_7_0 (i),     -- ATR Timeout Error zwischen Trigger und Zündpulsrückmeldung
                 ATR_to_conf_err       => ATR_to_conf_err_7_0(i)   -- ATR Falsche Registervorgabe für Timeout Zeit
                );
end generate;

---------------------------------Ein Clock breite "Strobe_o" Pulse für 8 Kanäle aus Input ATR_comp_puls -----------------------------------------

Puls_i  <=  ATR_comp_puls;   -- ATR_comp_puls[0..7] (Komparator-Ausgänge) = Kanal-Nummer[1..8]

p_Strobe:  PROCESS (clk, nReset)
  BEGIN
    IF nReset        = '0' THEN
      shift0        <= (OTHERS => '0');
      shift1        <= (OTHERS => '0');
      shift2        <= (OTHERS => '0');
      shift3        <= (OTHERS => '0');
      shift4        <= (OTHERS => '0');
      shift5        <= (OTHERS => '0');
      shift6        <= (OTHERS => '0');
      shift7        <= (OTHERS => '0');
      Strobe_o      <= (OTHERS => '0');
    ELSIF rising_edge(clk) THEN
      shift0        <= (shift0(shift0'high-1 downto 0) & (Puls_i(0)));
      shift1        <= (shift1(shift1'high-1 downto 0) & (Puls_i(1)));
      shift2        <= (shift2(shift2'high-1 downto 0) & (Puls_i(2)));
      shift3        <= (shift3(shift3'high-1 downto 0) & (Puls_i(3)));
      shift4        <= (shift4(shift4'high-1 downto 0) & (Puls_i(4)));
      shift5        <= (shift5(shift5'high-1 downto 0) & (Puls_i(5)));
      shift6        <= (shift6(shift6'high-1 downto 0) & (Puls_i(6)));
      shift7        <= (shift7(shift7'high-1 downto 0) & (Puls_i(7)));
      IF shift0(shift0'high) = '0' AND shift0(shift0'high-1) = '1' THEN Strobe_o(0) <= '1';ELSE Strobe_o(0) <= '0'; END IF;
      IF shift1(shift1'high) = '0' AND shift1(shift1'high-1) = '1' THEN Strobe_o(1) <= '1';ELSE Strobe_o(1) <= '0'; END IF;
      IF shift2(shift2'high) = '0' AND shift2(shift2'high-1) = '1' THEN Strobe_o(2) <= '1';ELSE Strobe_o(2) <= '0'; END IF;
      IF shift3(shift3'high) = '0' AND shift3(shift3'high-1) = '1' THEN Strobe_o(3) <= '1';ELSE Strobe_o(3) <= '0'; END IF;
      IF shift4(shift4'high) = '0' AND shift4(shift4'high-1) = '1' THEN Strobe_o(4) <= '1';ELSE Strobe_o(4) <= '0'; END IF;
      IF shift5(shift5'high) = '0' AND shift5(shift5'high-1) = '1' THEN Strobe_o(5) <= '1';ELSE Strobe_o(5) <= '0'; END IF;
      IF shift6(shift6'high) = '0' AND shift6(shift6'high-1) = '1' THEN Strobe_o(6) <= '1';ELSE Strobe_o(6) <= '0'; END IF;
      IF shift7(shift7'high) = '0' AND shift7(shift7'high-1) = '1' THEN Strobe_o(7) <= '1';ELSE Strobe_o(7) <= '0'; END IF;

    END IF;
END PROCESS p_Strobe;




----------------------------------------Register Vorgaben für ATR Pulserzeugung und Timeout Überwachung ---------------------------


P_Adr_Deco:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
      S_ATR_verz_rd            <= (OTHERS => '0');
      S_ATR_verz_wr            <= (OTHERS => '0');
      S_ATR_pulsw_rd           <= (OTHERS => '0');
      S_ATR_pulsw_wr           <= (OTHERS => '0');
			S_ATR_to_Count_rd        <= (OTHERS => '0');
			S_ATR_to_Count_wr        <= (OTHERS => '0');
			S_Dtack                  <= '0';
			Reg_rd_active            <= '0';

		ELSIF RISING_EDGE(clk) THEN
      S_ATR_verz_rd            <= (OTHERS => '0');   --Zündpuls Delay Vorgabe
      S_ATR_verz_wr            <= (OTHERS => '0');
      S_ATR_pulsw_rd           <= (OTHERS => '0');   --Zündpuls Width Vorgabe
      S_ATR_pulsw_wr           <= (OTHERS => '0');
			S_ATR_to_Count_rd        <= (OTHERS => '0');   --Timeout Counter Vrgabe
			S_ATR_to_Count_wr        <= (OTHERS => '0');
			S_Dtack                  <= '0';
			Reg_rd_active            <= '0';

			IF Ext_Adr_Val = '1' THEN

				CASE UNSIGNED(ADR_from_SCUB_LA) IS

					WHEN C_ATR_verz_0_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(0) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_rd(0) <= '1';    Reg_rd_active <= '1';  END IF;

          WHEN C_ATR_verz_1_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(1) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(1) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_2_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(2) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(2) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_3_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(3) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(3) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_4_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(4) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(4) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_5_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(5) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(5) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_6_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(6) <= '1';                           END IF;
						IF EXT_Rd_active = '1' THEN S_DTAck <= '1'; S_ATR_verz_rd(6) <= '1';    Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_verz_7_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_wr(7) <= '1';                           END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_verz_rd(7) <= '1';    Reg_rd_active <= '1';  END IF;

					WHEN C_ATR_pulsw_0_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(0) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(0) <= '1';   Reg_rd_active <= '1';  END IF;

          when C_ATR_pulsw_1_Addr =>
            if Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(1) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(1) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_2_Addr =>
            if Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(2) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(2) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_3_Addr =>
            if Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(3) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(3) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_4_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(4) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(4) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_5_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(5) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(5) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_6_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(6) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(6) <= '1';   Reg_rd_active <= '1';  END IF;

 					WHEN C_ATR_pulsw_7_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_wr(7) <= '1';                          END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_pulsw_rd(7) <= '1';   Reg_rd_active <= '1';  END IF;


 					WHEN C_ATR_to_count_0_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(0) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(0) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_1_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(1) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(1) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_2_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(2) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(2) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_3_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(3) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(3) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_4_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(4) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(4) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_5_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(5) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(5) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_6_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(6) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(6) <= '1'; Reg_rd_active <= '1'; END IF;

 					WHEN C_ATR_to_count_7_Addr =>
            IF Ext_Wr_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_wr(7) <= '1';                       END IF;
						IF Ext_Rd_active = '1' THEN S_Dtack <= '1'; S_ATR_to_count_rd(7) <= '1'; Reg_rd_active <= '1'; END IF;


					WHEN others =>

            S_ATR_verz_rd     <= (OTHERS => '0');
            S_ATR_verz_wr     <= (OTHERS => '0');
            S_ATR_pulsw_rd    <= (OTHERS => '0');
            S_ATR_pulsw_wr    <= (OTHERS => '0');
            S_ATR_to_count_wr <= (OTHERS => '0');
            S_ATR_to_count_rd <= (OTHERS => '0');

            S_Dtack           <= '0';
            Reg_rd_active     <= '0';
				END CASE;
			END IF;
		END IF;
END PROCESS P_Adr_Deco;



P_Reg_Write:	PROCESS (nReset, clk)
	BEGIN
		IF nReset = '0' THEN
			s_ATR_verz                                            <=(OTHERS =>(OTHERS => '0'));
			s_ATR_pulsw                                           <=(OTHERS =>(OTHERS => '0'));
			S_ATR_to_Count                                        <=(OTHERS =>(OTHERS => '0'));


		ELSIF rising_edge(clk) THEN
			IF S_ATR_verz_wr(0)     = '1' THEN	s_ATR_verz(0)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(1)     = '1' THEN	s_ATR_verz(1)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(2)     = '1' THEN	s_ATR_verz(2)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(3)     = '1' THEN	s_ATR_verz(3)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(4)     = '1' THEN	s_ATR_verz(4)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(5)     = '1' THEN	s_ATR_verz(5)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(6)     = '1' THEN	s_ATR_verz(6)     <= Data_from_SCUB_LA; END IF;
			IF S_ATR_verz_wr(7)     = '1' THEN	s_ATR_verz(7)     <= Data_from_SCUB_LA; END IF;

			IF S_ATR_pulsw_wr(0)    = '1' THEN	s_ATR_pulsw(0)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(1)    = '1' THEN	s_ATR_pulsw(1)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(2)    = '1' THEN	s_ATR_pulsw(2)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(3)    = '1' THEN	s_ATR_pulsw(3)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(4)    = '1' THEN	s_ATR_pulsw(4)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(5)    = '1' THEN	s_ATR_pulsw(5)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(6)    = '1' THEN	s_ATR_pulsw(6)    <= Data_from_SCUB_LA; END IF;
			IF S_ATR_pulsw_wr(7)    = '1' THEN	s_ATR_pulsw(7)    <= Data_from_SCUB_LA; END IF;

			IF S_ATR_to_count_wr(0) = '1' THEN	s_ATR_to_count(0) <= Data_from_SCUB_LA; END IF;
      IF S_ATR_to_count_wr(1) = '1' THEN	s_ATR_to_count(1) <= Data_from_SCUB_LA; END IF;
			IF S_ATR_to_count_wr(2) = '1' THEN	s_ATR_to_count(2) <= Data_from_SCUB_LA; END IF;
      IF S_ATR_to_count_wr(3) = '1' THEN	s_ATR_to_count(3) <= Data_from_SCUB_LA; END IF;
			IF S_ATR_to_count_wr(4) = '1' THEN	s_ATR_to_count(4) <= Data_from_SCUB_LA; END IF;
      IF S_ATR_to_count_wr(5) = '1' THEN	s_ATR_to_count(5) <= Data_from_SCUB_LA; END IF;
			IF S_ATR_to_count_wr(6) = '1' THEN	s_ATR_to_count(6) <= Data_from_SCUB_LA; END IF;
      IF S_ATR_to_count_wr(7) = '1' THEN	s_ATR_to_count(7) <= Data_from_SCUB_LA; END IF;
    END IF;
END PROCESS P_Reg_Write;



P_Reg_Read_mux:	PROCESS (S_ATR_verz_rd,    S_ATR_pulsw_rd,  S_ATR_to_count_rd,
                         s_ATR_verz,       s_ATR_pulsw,     s_ATR_to_count    )

	BEGIN
		IF 	  S_ATR_verz_rd(0)     = '1' THEN	S_Read_port <= s_ATR_verz(0);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(1)     = '1' THEN	S_Read_port <= s_ATR_verz(1);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(2)     = '1' THEN	S_Read_port <= s_ATR_verz(2);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(3)     = '1' THEN	S_Read_port <= s_ATR_verz(3);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(4)     = '1' THEN	S_Read_port <= s_ATR_verz(4);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(5)     = '1' THEN	S_Read_port <= s_ATR_verz(5);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(6)     = '1' THEN	S_Read_port <= s_ATR_verz(6);     -- Reg Vorgabe für Zündpulsverzögerung
		ELSIF S_ATR_verz_rd(7)     = '1' THEN	S_Read_port <= s_ATR_verz(7);     -- Reg Vorgabe für Zündpulsverzögerung

		ELSIF S_ATR_pulsw_rd(0)    = '1' THEN	S_Read_port <= s_ATR_pulsw(0);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(1)    = '1' THEN	S_Read_port <= s_ATR_pulsw(1);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(2)    = '1' THEN	S_Read_port <= s_ATR_pulsw(2);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(3)    = '1' THEN	S_Read_port <= s_ATR_pulsw(3);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(4)    = '1' THEN	S_Read_port <= s_ATR_pulsw(4);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(5)    = '1' THEN	S_Read_port <= s_ATR_pulsw(5);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(6)    = '1' THEN	S_Read_port <= s_ATR_pulsw(6);    -- Reg Vorgabe für Zündpulsbreite
		ELSIF S_ATR_pulsw_rd(7)    = '1' THEN	S_Read_port <= s_ATR_pulsw(7);    -- Reg Vorgabe für Zündpulsbreite


		ELSIF S_ATR_to_count_rd(0) = '1' THEN	S_Read_port <= s_ATR_to_count(0); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(1) = '1' THEN	S_Read_port <= s_ATR_to_count(1); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(2) = '1' THEN	S_Read_port <= s_ATR_to_count(2); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(3) = '1' THEN	S_Read_port <= s_ATR_to_count(3); -- Reg Vorgabe für Timeout Zündpuls Rückmeldungt
		ELSIF S_ATR_to_count_rd(4) = '1' THEN	S_Read_port <= s_ATR_to_count(4); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(5) = '1' THEN	S_Read_port <= s_ATR_to_count(5); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(6) = '1' THEN	S_Read_port <= s_ATR_to_count(6); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung
    ELSIF S_ATR_to_count_rd(7) = '1' THEN	S_Read_port <= s_ATR_to_count(7); -- Reg Vorgabe für Timeout Zündpuls Rückmeldung

    ELSE
			S_Read_Port <= (OTHERS => '-');
		END IF;
	END PROCESS P_Reg_Read_mux;


-- KK es gibt jetzt 8 config errors und 8 timeouts

--  ATR_to_conf_err <= s_config_err;	-- Time-Out: Configurations-Error
--  ATR_Timeout  		<= s_timeout;			-- Time-Out: Maximalzeit zwischen Start und Zündpuls überschritten.

--P_Save_Config_err:	PROCESS (clk_250mhz, nReset_250mhz, ATR_Timeout_err_res)
--	BEGIN
--		IF  ((nReset_250mhz = '0') or (ATR_Timeout_err_res = '1')) THEN
--       ATR_to_conf_err <= '0';

--		ELSIF rising_edge(clk_250mhz) THEN
--			IF s_config_err      = '1'  THEN  -- Save Error
--        ATR_to_conf_err   <= '1';       -- set Error-Flag
--			END IF;
--		END IF;
--	END PROCESS P_Save_Config_err;

--P_Save_Timeout:	PROCESS (clk_250mhz, nReset_250mhz, ATR_Timeout_err_res)
--	BEGIN
--		IF  ((nReset_250mhz = '0') or (ATR_Timeout_err_res = '1')) THEN
--          ATR_Timeout <= '0';

--		ELSIF rising_edge(clk_250mhz) THEN
--			IF s_timeout      = '1'     THEN -- Save Error
--        ATR_Timeout    <= '1';         -- set Error-Flag
--			END IF;
--		END IF;
--	END PROCESS P_Save_Timeout;



Dtack_to_SCUB   <= S_Dtack;
Data_to_SCUB    <= S_Read_Port;


END Arch_atr_puls_ctrl;