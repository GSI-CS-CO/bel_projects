-----------------------------------------------------------------------
	-- Counter für Soll- und Istwertmessung von Chopper Signal
	-- Bei Flanken wird der aktuelle Counter in Latches geschrieben
	-- und weitergezählt (bis OV)
	-- Rücksetzen des Counters durch Lesezugriff auf StrahlwegReg
	-----------------------------------------------------------------------

	--     act_pos_latch
	--   |  15..8 | 7|6..0|
	--   | Status |OV| Cnt|

	-- act_pos_latch latched:	Bit 15
	-- act_pos_edge > 127:		Bit 11
	-- Counter Overflow:			Bit 10
	-- act high vor Start:		Bit 9
	-- act nicht durchgängig: 	Bit 8


	--  neg_latch
	--   |    15   |14|13..0|
	--   | latched |OV| Cnt |

	--  act_neg_latch
	--   |   15    |14|13..0|
	--   | latched |OV| Cnt |
	-----------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.math_real.all;

library lpm;
use lpm.lpm_components.all;


entity chopper_monitoring is
  generic (
    C_Chop_Count_Width:		integer := 15;
    C_Chop_neg_Diff:		integer := 20;
    C_inl_cnt_value:		integer := 3
  );
  port (
    clk               : in std_logic;
    reset             : in std_logic;
    s_1us_en          : in std_logic;
    clear             : in std_logic;
    chopp_signal_on   : in std_logic;
    chopp_signal_act  : in std_logic;
    act_pos_latch_out : out std_logic_vector(C_Chop_Count_Width  downto 0);
    neg_latch_out     : out std_logic_vector(C_Chop_Count_Width  downto 0);
    act_neg_latch_out : out std_logic_vector(C_Chop_Count_Width  downto 0)
  );
end entity chopper_monitoring;


architecture chopper_monitoring_arch of chopper_monitoring is

constant C_inl_cnt_width:			integer := integer(floor(log2(real(C_inl_cnt_value)))) + 2;


signal s_cnt_started   : std_logic;
signal s_cnt_en        : std_logic;
signal s_cnt_value     : std_logic_vector(C_Chop_Count_Width - 1 downto 0);

signal s_act_pos_latch : std_logic_vector(C_Chop_Count_Width downto 0);
signal s_neg_latch     : std_logic_vector(C_Chop_Count_Width downto 0);
signal s_act_neg_latch : std_logic_vector(C_Chop_Count_Width downto 0);

type state_type is ( idle, cnt_started, cnt_reached );
signal state_inl       : state_type;

signal s_act_late      : std_logic;
signal s_inl_start_en  : std_logic;
signal s_inl_s_en      : std_logic;
signal s_inl_end_en    : std_logic;
signal s_inl_s_value   : std_logic_vector(C_inl_cnt_width - 1 downto 0);
signal s_inl_sset      : std_logic;

signal chopper_pos     : std_logic;
signal chopper_neg     : std_logic;
signal chopper_act_pos : std_logic;
signal chopper_act_neg : std_logic;


component edge_detection is
  port (
    Clk      : in std_logic;
    Reset    : in std_logic;
    input    : in std_logic;
    pos_edge : out std_logic;
    neg_edge : out std_logic
  );
end component;

begin

-- Flankenerkennung für Chopper

chopp_pos: edge_detection
port map (
  clk      => clk,
  reset    => reset,
  input    => chopp_signal_on,
  pos_edge => chopper_pos
);

chopp_neg: edge_detection
port map (
  clk      => clk,
  reset    => reset,
  input    => chopp_signal_on,
  neg_edge => chopper_neg
);

chopp_act_pos: edge_detection
port map (
  clk      => clk,
  reset    => reset,
  input    => chopp_signal_act,
  pos_edge => chopper_act_pos
);

chopp_act_neg: edge_detection
port map (
  clk      => clk,
  reset    => reset,
  input    => chopp_signal_act,
  neg_edge => chopper_act_neg
);
---------------------------------------------------------------------------------------------
-- Starten / Stoppen des Zählers
-- 19.02.2008 Zähler darf erst nach Zeitfenster starten, damit der Chopper ausschwingen kann.
-- Vorschlag: Abhängig von Quellen HSI
---------------------------------------------------------------------------------------------
monitor_cnt_en: process(clk, reset, chopper_pos, clear)
begin
  if clear = '1' or reset = '1' then
    s_cnt_started <= '0';
  elsif rising_edge(clk) then
    if chopper_pos = '1' then
      s_cnt_started <= '1';
    end if;
  end if;
end process;

-- Speichern der Zeitmarken
cnt_latch: process(clk, reset, chopper_act_pos, chopper_neg, chopper_act_neg, clear)
begin
  if rising_edge(clk) then
    if clear = '1' or reset = '1' then
      s_act_pos_latch <= (OTHERS => '0');
      s_neg_latch     <= (OTHERS => '0');
      s_act_neg_latch <= (OTHERS => '0');
    else

      -- Pos Edge Istwert
      -- geändert am 19.09.06: chopper_act_pos
      if chopper_act_pos = '1' then
        -- "high before"
        if s_cnt_started = '0' then
          s_act_pos_latch(9) <= '1';
        -- Pegel nicht durchgängig
        elsif s_act_neg_latch(s_act_neg_latch'HIGH) = '1' then
          s_act_pos_latch(8) <= '1';
        else
          s_act_pos_latch(7 downto 0)           <= s_cnt_value(7 downto 0);
          s_act_pos_latch(s_act_pos_latch'HIGH) <= '1';
        end if;

      -- Neg Edge Vorgabe
      elsif chopper_neg = '1' then
        s_neg_latch(s_neg_latch'HIGH-1 downto 0) <= s_cnt_value;
        s_neg_latch(s_neg_latch'HIGH)            <= '1';
      -- Neg Edge Istwert
      -- geändert am 19.09.06: chopper_act_neg
      elsif chopper_act_neg = '1' then
        s_act_neg_latch(s_act_neg_latch'HIGH-1 downto 0) <= s_cnt_value;
        s_act_neg_latch(s_act_neg_latch'HIGH)            <= '1';
      -- Overflow
      elsif s_cnt_value(s_cnt_value'HIGH) = '1' then
        s_act_pos_latch(10) <= '1';
      -- Overflow act_pos_edge
      elsif s_cnt_value(7) = '1' and not s_act_pos_latch(s_act_pos_latch'HIGH) = '1' then
        s_act_pos_latch(11) <= '1';
      end if;
    end if;
  end if;
end process;

act_pos_latch_out <= s_act_pos_latch;
neg_latch_out     <= s_neg_latch;
act_neg_latch_out <= s_act_neg_latch;

-- Stoppen bei Overflow
s_cnt_en <= s_cnt_started and s_1us_en and not s_cnt_value(s_cnt_value'HIGH);

-- Rücksetzen des Zählerstandes bei clear
timestamp: lpm_counter
generic map (
  LPM_WIDTH     => C_Chop_Count_Width,
  LPM_TYPE      => "LPM_COUNTER",
  LPM_DIRECTION => "UP"
)
port map (
  clock  => Clk,
  cnt_en => s_cnt_en,
  q      => s_cnt_value,
  sclr   => clear
);

-----------------------------------------------------------------------
-- Test auf Interlock Bedingung für Chopper
-- Es wird auf drei Fälle getestet:
-- Act Signal zu stark verzögert (Start)
-- Act Signal zu stark verzögert (Ende)
-- s_act_late
-----------------------------------------------------------------------

inl_err: process (Clk, Reset)
begin
  if clear = '1' or Reset = '1' then
    s_inl_sset <= '1';
    s_act_late <= '0';
    state_inl  <= idle;
  elsif rising_edge(Clk) then
    s_inl_start_en <= '0';
    s_inl_sset     <= '0';

    case (state_inl) is
      when idle =>
        if chopper_pos = '1' or chopper_neg = '1' then
          s_inl_start_en <= '1';
          state_inl      <= cnt_started;
        end if;

      when cnt_started =>
        s_inl_start_en <= '1';
        if s_inl_s_value(s_inl_s_value'HIGH) = '1' then
          state_inl <= cnt_reached;
        elsif chopper_act_pos = '1' or chopper_act_neg = '1' then
          s_inl_sset <= '1';
          state_inl  <= idle;
        end if;

      when cnt_reached =>
        if chopper_act_pos = '1' or chopper_act_neg = '1' then
          s_act_late <= '1';
        end if;
    end case;
  end if;
end process;

-- Stoppen bei Overflow
s_inl_s_en <= s_inl_start_en and s_1us_en and not s_inl_s_value(s_inl_s_value'HIGH);


hsi_inl_start: lpm_counter
generic map (
  LPM_WIDTH     => C_inl_cnt_width,
  LPM_DIRECTION => "DOWN",
  LPM_SVALUE    => integer'image(C_inl_cnt_value)
)
port map (
  clock  => Clk,
  cnt_en => s_inl_s_en,
  q      => s_inl_s_value,
  sset   => s_inl_sset
);
end architecture;
