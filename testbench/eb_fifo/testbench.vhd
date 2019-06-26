library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.genram_pkg.all;


entity testbench is
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  signal rst           : std_logic := '1';
  signal rst_n         : std_logic := '0';
  constant clk_period  : time      := 20 ns;
  signal clk           : std_logic := '1';


  -- fifo constants
  constant fifo_width : natural :=  8;
  constant fifo_size  : natural := 32;
  -- fifo signals
  signal w_dat     : std_logic_vector(fifo_width-1 downto 0) := (others => '0');
  signal r_dat     : std_logic_vector(fifo_width-1 downto 0) := (others => '0');

  signal push, pop   : std_logic := '0';
  signal full, empty : std_logic := '0';

  -- signals for alternative implementation of eb_fifo
  signal my_r_dat          : std_logic_vector(fifo_width-1 downto 0) := (others => '0');
  signal my_full, my_empty : std_logic := '0';

  procedure wait_clock_cycles(constant cycles : in natural; signal clk : in std_logic) is
  begin
    for i in 1 to cycles loop
      wait until rising_edge(clk);
    end loop;
  end procedure;

begin
  ---- generate clock and reset signal -------
  clk   <= not clk  after clk_period/2;
  rst   <= '0'      after clk_period*20;
  rst_n <= not rst;
  --------------------------------------------

  reference_fifo: entity work.eb_fifo
    generic map(
      g_width => fifo_width,
      g_size  => fifo_size)
    port map(
      clk_i     => clk,
      rstn_i    => rst_n,
      w_full_o  => full,
      w_push_i  => push,
      w_dat_i   => w_dat,
      r_empty_o => empty,
      r_pop_i   => pop,
      r_dat_o   => r_dat);

  my_fifo: entity work.my_eb_fifo
    generic map(
      g_width => fifo_width,
      g_size  => fifo_size)
    port map(
      clk_i     => clk,
      rstn_i    => rst_n,
      w_full_o  => my_full,
      w_push_i  => push,
      w_dat_i   => w_dat,
      r_empty_o => my_empty,
      r_pop_i   => pop,
      r_dat_o   => my_r_dat);


  test_cycle: process 
  begin
    -- initialize
    push <= '0';
    pop  <= '0';

    wait_clock_cycles(2, clk);

    -- completely fill fifo
    push  <= '1';
    wait_clock_cycles(fifo_size, clk);
    push <= '0';

    -- wait a bit
    wait_clock_cycles(2, clk);

    -- half empty fifo
    pop  <= '1';
    wait_clock_cycles(fifo_size/2, clk);
    pop <= '0';

    -- wait a bit
    wait_clock_cycles(2, clk);

    -- fill fifo again
    push  <= '1';
    wait_clock_cycles(fifo_size/2, clk);
    push <= '0';

    -- wait a bit
    wait_clock_cycles(2, clk);

    -- completely empty fifo
    pop  <= '1';
    wait_clock_cycles(fifo_size, clk);
    pop <= '0';

    -- write an read at the same time 
    push <= '1';
    wait until falling_edge(empty);
    pop <= '1';

    wait;
  end process;

  generate_input_data: process
  begin
    wait until rising_edge(clk);
    w_dat <= std_logic_vector(unsigned(w_dat)+1);
  end process;

  verify_identical_output: process
  begin
    wait until rising_edge(clk);
    assert full = my_full                     report "full differs";
    assert empty = my_empty                   report "empty differs";
    assert r_dat = my_r_dat or my_empty = '1' report "data differs while empty is low";
  end process;

end architecture;