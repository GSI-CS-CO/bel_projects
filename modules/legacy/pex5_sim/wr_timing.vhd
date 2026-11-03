library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wr_timing is
  port(
    
    dac_hpll_load_p1_i : in std_logic;
    dac_hpll_data_i    : in std_logic_vector(15 downto 0);

    dac_dpll_load_p1_i : in std_logic;
    dac_dpll_data_i    : in std_logic_vector(15 downto 0);

    clk_ref_125_o      : out std_logic;
    clk_sys_62_5_o     : out std_logic;
    clk_dmtd_20_o      : out std_logic
    );

end entity;

architecture simulation of wr_timing is
  signal dac_hpll_set : unsigned(15 downto 0) := ('0', others => '1');
  signal dac_dpll_set : unsigned(15 downto 0) := ('0', others => '1');

  signal dac_hpll     : integer := 0;
  signal dac_dpll     : integer := 0;


  signal clk_ref_125   : std_logic := '1';
  signal clk_sys_62_5  : std_logic := '1';
  signal clk_dmtd_20   : std_logic := '1';


  constant period_ref  : integer :=  8000000/2; -- fs
  constant period_sys  : integer := 16000000/2; -- fs
  constant period_dmtd : integer := 50000000/2; -- fs

  constant tau_max : integer := 1024;
  constant tau     : integer := 1;
  constant delta_t : time    := 1 ns;
begin

  process begin 
    wait until rising_edge(dac_hpll_load_p1_i);
    dac_hpll_set <= unsigned(dac_hpll_data_i);
    report "wr_timing dac_hpll_set <= " & integer'image(to_integer(unsigned(dac_hpll_data_i)));
  end process;

  process begin
    wait until rising_edge(dac_dpll_load_p1_i);
    dac_dpll_set <= unsigned(dac_dpll_data_i);
    report "wr_timing dac_dpll_set <= " & integer'image(to_integer(unsigned(dac_dpll_data_i)));
  end process;

  dynamics: process begin
    dac_hpll <= ((tau_max - tau)*dac_hpll + tau*to_integer(dac_hpll_set)) / tau_max;
    dac_dpll <= ((tau_max - tau)*dac_dpll + tau*to_integer(dac_dpll_set)) / tau_max;
    wait for delta_t;
  end process;

  clk_ref_125  <= not clk_ref_125  after (period_ref  + 10*dac_dpll - 327680) * 1 fs;
  clk_sys_62_5 <= not clk_sys_62_5 after (period_sys  + 10*dac_dpll - 327680) * 1 fs;
  clk_dmtd_20  <= not clk_dmtd_20  after (period_dmtd + 10*dac_hpll - 327680) * 1 fs;


  clk_ref_125_o  <= clk_ref_125;
  clk_sys_62_5_o <= clk_sys_62_5;
  clk_dmtd_20_o  <= clk_dmtd_20;
end architecture;
