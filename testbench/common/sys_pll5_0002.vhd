library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library altera_lnsim;
use altera_lnsim.altera_lnsim_components.all;

entity sys_pll5_0002 is 
port (
	refclk   :  in std_logic;
	rst      :  in std_logic;
	outclk_0 : out std_logic;
	outclk_1 : out std_logic;
	outclk_2 : out std_logic;
	outclk_3 : out std_logic;
	outclk_4 : out std_logic;
	locked   : out std_logic
);
end entity;

architecture rtl of sys_pll5_0002 is
	signal outclk : std_logic_vector(4 downto 0);
begin
	pll: altera_pll
		generic map (
		fractional_vco_multiplier => "false",
		reference_clock_frequency => "125.0 MHz",
		operation_mode            => "direct",
		number_of_clocks          => 5,
		output_clock_frequency0   => "62.500000 MHz",
		phase_shift0              => "0 ps",
		duty_cycle0               => 50,
		output_clock_frequency1   => "100.000000 MHz",
		phase_shift1              => "0 ps",
		duty_cycle1               => 50,
		output_clock_frequency2   => "20.000000 MHz",
		phase_shift2              => "0 ps",
		duty_cycle2               => 50,
		output_clock_frequency3   => "10.000000 MHz",
		phase_shift3              => "0 ps",
		duty_cycle3               => 50,
		output_clock_frequency4   => "50.000000 MHz",
		phase_shift4              => "0 ps",
		duty_cycle4               => 50,
		output_clock_frequency5   => "0 MHz",
		phase_shift5              => "0 ps",
		duty_cycle5               => 50,
		output_clock_frequency6   => "0 MHz",
		phase_shift6              => "0 ps",
		duty_cycle6               => 50,
		output_clock_frequency7   => "0 MHz",
		phase_shift7              => "0 ps",
		duty_cycle7               => 50,
		output_clock_frequency8   => "0 MHz",
		phase_shift8              => "0 ps",
		duty_cycle8               => 50,
		output_clock_frequency9   => "0 MHz",
		phase_shift9              => "0 ps",
		duty_cycle9               => 50,
		output_clock_frequency10  => "0 MHz",
		phase_shift10             => "0 ps",
		duty_cycle10              => 50,
		output_clock_frequency11  => "0 MHz",
		phase_shift11             => "0 ps",
		duty_cycle11              => 50,
		output_clock_frequency12  => "0 MHz",
		phase_shift12             => "0 ps",
		duty_cycle12              => 50,
		output_clock_frequency13  => "0 MHz",
		phase_shift13             => "0 ps",
		duty_cycle13              => 50,
		output_clock_frequency14  => "0 MHz",
		phase_shift14             => "0 ps",
		duty_cycle14              => 50,
		output_clock_frequency15  => "0 MHz",
		phase_shift15             => "0 ps",
		duty_cycle15              => 50,
		output_clock_frequency16  => "0 MHz",
		phase_shift16             => "0 ps",
		duty_cycle16              => 50,
		output_clock_frequency17  => "0 MHz",
		phase_shift17             => "0 ps",
		duty_cycle17              => 50,
		pll_type                  => "General",
		pll_subtype               => "General"
	) port map (
		rst      => rst,
		outclk   => outclk,
		locked   => locked,
		fboutclk => open,
		fbclk    => '1',
		refclk   => refclk
	);
	outclk_4 <= outclk(4);
	outclk_3 <= outclk(3);
	outclk_2 <= outclk(2);
	outclk_1 <= outclk(1);
	outclk_0 <= outclk(0);
end architecture;

