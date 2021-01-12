-- Erzeugt nach einer Zeit von delay_cnt einen High Pegel am Ausgang trunc
-- 25.03.09 SR

library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library lpm;
use lpm.lpm_components.all;


entity trunc is
		generic	(
					delay_cnt:		integer := 20
				);
		port (
					neg_edge:		in std_logic;
					clk:			in std_logic;
					reset:			in std_logic;

					trunc:			out std_logic
			);

	constant c_delay : integer := delay_cnt - 2;
	constant C_LPM_WIDTH:	integer	:=	integer(floor(log2(real(c_delay)))) + 2;
end trunc;

architecture trunc_arch of trunc is

signal		s_sync_pos1, s_sync_pos2: std_logic;
signal		q: std_logic;
signal   	counter_q: std_logic_vector(C_LPM_WIDTH-1 downto 0);
type		sm_type is (idle, wait_cnt, trunc_active);
signal		state:	sm_type;
signal		s_frame_edge:	std_logic;
signal		s_trunc:		std_logic;
signal		s_sset:			std_logic;
signal		s_cnt_en:		std_logic;





begin

	Sync:	process(clk, reset)
	begin
		if reset = '1' then
			s_sync_pos1 <= '0';
			s_sync_pos2 <= '0';
		elsif rising_edge(clk) then
			s_sync_pos1 <= neg_edge;
			s_sync_pos2 <= s_sync_pos1;
		end if;
	end process;

	s_frame_edge <=  not s_sync_pos1 and s_sync_pos2;

	cnt: lpm_counter generic map (
											LPM_WIDTH => C_LPM_WIDTH,
											LPM_TYPE => "LPM_COUNTER",
											LPM_SVALUE => integer'image(c_delay),
											LPM_DIRECTION => "DOWN"
										)
							port map	(	clock => Clk,
						 					cnt_en => s_cnt_en,
											q => counter_q,
											sset => s_sset
										);

	trunc_sm: process (clk, reset)
	begin
		if reset='1' then
			state <= idle;
			s_trunc <= '0';
		elsif rising_edge(clk) then
			s_trunc <= '0';
			s_sset <= '0';
			s_cnt_en <= '0';

			case (state) is

				when idle =>
					s_sset <= '1';
					if s_frame_edge = '1' then
						state <= wait_cnt;
					end if;

				when wait_cnt =>
					s_cnt_en <= '1';
					if counter_q(counter_q'high) = '1' then
						state <= trunc_active;
					end if;

				when trunc_active =>
					s_trunc <= '1';
			end case;
		end if;

	end process;


	trunc <= s_trunc;


end trunc_arch;
