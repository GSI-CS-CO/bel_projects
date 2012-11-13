
LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;


LIBRARY work;

ENTITY lemo_io IS
generic	(
		Opendrain_is_1: integer range 0 to 1 := 1;						-- 1 => activity_led_n wird als Opendrain gebaut; 0 => activity_led_n ist normaler Push-Pull Ausgang
		stretch_cnt:	integer := 3									-- Anzahl der Taktperioden von led_clk, um die die activity_led_n nach einer Aktivität (Flanke) an lemo_io gestretcht wird.
		);
 port	(
		reset:					in		std_logic;
		clk:					in		std_logic;
		led_clk:				in		std_logic;
		lemo_io_is_output:		in		std_logic;						-- '0' => lemo_io ist Eingang; '1' => lemo_io ist Ausgang
		stretch_led_off:		in		std_logic;						-- '0' => activity_led_n ist gestretcht; '1' => activity_led_n nicht gestretcht
		to_lemo:				in		std_logic;						-- wenn lemo_io ein Ausgang ist, wird das Signal to_lemo ausgegeben
		lemo_io:				inout	std_logic;
 		lemo_en_in:				out		std_logic;						-- '1' => lemo_io ist Eingang; '0' => lemo_io ist Ausgang
 		activity_led_n:			out		std_logic						-- für Aktivitätsanzeige von lemo_io vorgesehen
		);
end lemo_io;

architecture arch_lemo_io of lemo_io is

component led_n
	generic (
			stretch_cnt:	integer := 3
			); 
	port	(
			ena:		in	STD_LOGIC := '1';
			CLK:		in	STD_LOGIC;
			Sig_In:		in	STD_LOGIC;
			nLED:		out	STD_LOGIC
			);
END component led_n;


signal		edge_detect:			std_logic_vector(1 downto 0);
signal		pos_edge, neg_edge:		std_logic;
signal		stretched_led_n:		std_logic;
signal		s_activity_led_n:		std_logic;


begin

p_edge_detect:	process (clk, reset)
	begin
		if reset = '1' then
			edge_detect <= (OTHERS => '0');
		elsif rising_edge(clk) then
			edge_detect <= (edge_detect(0) & lemo_io);
		end if;
	end process p_edge_detect;

pos_edge <=		lemo_io and not edge_detect(1);
neg_edge <= not lemo_io and		edge_detect(1);

p_led: led_n
	generic	map	(
				stretch_cnt		=> stretch_cnt
				)
	port map	(
				ena			=> '1',
				CLK			=> led_clk,
				Sig_In		=> pos_edge or neg_edge,
				nLED		=> stretched_led_n
				);


p_lemo_io_tristate:	process (lemo_io_is_output)
	begin
		if lemo_io_is_output = '0' then
			lemo_en_in <= '1';
			lemo_io <= 'Z';
		else
			lemo_en_in <= '0';
			lemo_io <= to_lemo;
		end if;
	end process p_lemo_io_tristate;


p_lemo_led:	process (stretch_led_off, lemo_io)
	begin
		if stretch_led_off = '0' then
			s_activity_led_n <= stretched_led_n;
		else
			s_activity_led_n <= pos_edge or neg_edge;
		end if;
	end process p_lemo_led;

opendrain:
if Opendrain_is_1 = 1 generate
	activity_led_n <= '0' when (s_activity_led_n = '0') else 'Z';
end generate;

pushpull:
if Opendrain_is_1 = 0 generate
	activity_led_n <= '0' when (s_activity_led_n = '0') else '1';
end generate;

end arch_lemo_io;
