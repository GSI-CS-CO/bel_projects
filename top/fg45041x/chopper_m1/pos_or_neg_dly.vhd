-- configurable delay, triggered by positive or negative edge

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.MATH_REAL.ALL;


LIBRARY lpm;
USE lpm.lpm_components.all;

entity pos_or_neg_dly is
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

CONSTANT c_delay_start:		integer := (Edge_delay_cnt - 1);
CONSTANT LPM_WIDTH:			integer := (integer(ceil(log2(real(c_delay_start)))) + 1);

end pos_or_neg_dly;

architecture  pos_or_neg_dly_arch of pos_or_neg_dly is

type state_type is (
  Idle,
  pos_dly_activ,
  wait_neg_edge,
  neg_dly_activ,
  in_is_pos
);

signal 	s_sync_sig_pos: std_logic;
signal	s_sync_sig_neg: std_logic;
signal 	state:	state_type;

signal	counter_cnt_en:		std_logic;
signal	counter_sset:		std_logic;	-- sconst veraltet, ersetzt durch sset
signal	counter_q:			std_logic_vector(LPM_WIDTH-1 DOWNTO 0);


begin

ASSERT NOT(Edge_delay_cnt < 3)
	REPORT "   Edge_delay_cnt sollte > 3 sein!"
SEVERITY ERROR;


delay_cnt: lpm_counter GENERIC MAP (
  LPM_WIDTH     => LPM_WIDTH,
  LPM_SVALUE    => integer'image(c_delay_start),
  LPM_DIRECTION => "DOWN"
)
PORT MAP (
  clock  => Clk,
  cnt_en => counter_cnt_en,
  q      => counter_q,
  sset   => counter_sset
);


sync: process (Clk, Reset)
begin
  if Reset = '1' then
    s_sync_sig_pos <= '0';
    s_sync_sig_neg <= '0';
  elsif (rising_edge(Clk)) then
    s_sync_sig_pos <= Edge_pos;
    s_sync_sig_neg <= Edge_neg;
  end if;
end process;



delay_sm: process (Clk, Reset)
begin

	if Reset = '1' then
		state <= Idle;
		Sig_Out <= '0';
	elsif (clk'event and clk = '1') then
		counter_cnt_en <= '0';
		counter_sset <= '0';
		--s_Sig_out_s <= '0';
		--s_Sig_out_r <= '0';

		CASE (state) IS

			WHEN Idle =>
				counter_sset <= '1';
				If (s_sync_sig_pos = '1' AND s_sync_sig_neg = '0') THEN
					state <= pos_dly_activ;
				ELSIF (s_sync_sig_pos = '0' AND s_sync_sig_neg = '1') THEN
					Sig_out <= '1';
					state <= wait_neg_edge;
				END IF;

		WHEN pos_dly_activ =>
			counter_cnt_en <= '1';
			IF (counter_q = conv_std_logic_vector(2, LPM_WIDTH)) THEN
				Sig_out <= '1';
				state <= in_is_pos;
			ELSIF s_sync_sig_pos = '0' THEN
				Sig_out <= '0';
				state <= Idle;
			END IF;

		WHEN wait_neg_edge =>
			IF s_sync_sig_neg = '0' THEN
				--counter_cnt_en <= '1';
				state <= neg_dly_activ;
			END IF;

		WHEN neg_dly_activ =>
			counter_cnt_en <= '1';
			IF (counter_q = conv_std_logic_vector(2, LPM_WIDTH)) THEN
				Sig_out <= '0';
				state <= Idle;
			END IF;

		WHEN in_is_pos =>
			IF s_sync_sig_pos = '0' THEN
				Sig_out <= '0';
				state <= Idle;
			END IF;

		END CASE;
	end if;
end process delay_sm;


end pos_or_neg_dly_arch;
