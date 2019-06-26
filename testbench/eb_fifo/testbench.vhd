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
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';
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

  signal data : std_logic_vector(fifo_width-1 downto 0) := (others => '0');

  -- my implementation of eb_fifo
  signal my_r_dat          : std_logic_vector(fifo_width-1 downto 0) := (others => '0');
  signal my_full, my_empty : std_logic := '0';

begin
  ---- generate clock and reset signal -------
  clk   <= not clk  after clk_period/2;
  rst   <= '0'      after clk_period*20;
  rst_n <= not rst;
  --------------------------------------------
  fifo: entity work.eb_fifo
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


  testing: process 
  begin
    -- initialize
    push <= '0';
    pop  <= '0';

    wait until rising_edge(rst_n);
    wait until rising_edge(clk);

    w_dat <= (others=>'0');
    push  <= '1';
    for i in 1 to fifo_size loop
      wait until rising_edge(clk);
      w_dat <= std_logic_vector(unsigned(w_dat)+1);
    end loop;
    push <= '0';

    wait until rising_edge(clk);
    wait until rising_edge(clk);

    pop  <= '1';
    for i in 1 to fifo_size/2 loop
      wait until rising_edge(clk);
      --w_dat <= std_logic_vector(unsigned(w_dat)+1);
    end loop;
    pop <= '0';

    wait until rising_edge(clk);
    wait until rising_edge(clk);

    w_dat <= (others=>'0');
    push  <= '1';
    for i in 1 to fifo_size/2 loop
      wait until rising_edge(clk);
      w_dat <= std_logic_vector(unsigned(w_dat)+1);
    end loop;
    push <= '0';


    wait until rising_edge(clk);
    wait until rising_edge(clk);

    pop  <= '1';
    for i in 1 to fifo_size loop
      wait until rising_edge(clk);
      --w_dat <= std_logic_vector(unsigned(w_dat)+1);
    end loop;
    pop <= '0';

    w_dat <= data;
    push <= '1';
    wait until falling_edge(empty);
    pop <= '1';


    wait;
  end process;

  makedata: process
  begin
    wait until rising_edge(clk);
    data <= std_logic_vector(unsigned(data)+1);

  end process;


end architecture;