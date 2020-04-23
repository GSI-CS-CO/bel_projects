library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

entity testbench is
end entity;

architecture simulation of testbench is

  signal clk_sys           : std_logic;
  signal clk_ref           : std_logic;
  signal clk_dmtd          : std_logic;

  signal dac_hpll_load_p1  : std_logic;
  signal dac_hpll_data     : std_logic_vector(15 downto 0);
  signal dac_dpll_load_p1  : std_logic;
  signal dac_dpll_data     : std_logic_vector(15 downto 0);

begin

  timing: entity work.wr_timing
  port map (
    dac_hpll_load_p1_i => dac_hpll_load_p1,
    dac_hpll_data_i    => dac_hpll_data,
    dac_dpll_load_p1_i => dac_dpll_load_p1,
    dac_dpll_data_i    => dac_dpll_data,
    clk_ref_125_o      => clk_ref,
    clk_sys_62_5_o     => clk_sys,
    clk_dmtd_20_o      => clk_dmtd
    );


    process
    begin
      dac_hpll_load_p1 <= '1' , '0' after 10 ns;
      dac_dpll_load_p1 <= '1' , '0' after 10 ns;
      wait for 5 us;
      dac_hpll_data    <= std_logic_vector(to_unsigned(0,16));
      dac_hpll_load_p1 <= '1' , '0' after 10 ns;
      dac_dpll_data    <= std_logic_vector(to_unsigned(0,16));
      dac_dpll_load_p1 <= '1' , '0' after 10 ns;

    end process;


end architecture;



