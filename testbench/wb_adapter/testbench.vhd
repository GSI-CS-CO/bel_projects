library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;


entity testbench is
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  constant clk_ext_period : time      := 83.33333333 ns;
  constant clk_sys_period : time      :=  8.00000000 ns;
  signal clk_ext          : std_logic := '1';
  signal clk_sys          : std_logic := '1';
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';

  signal pipelined_mosi   : t_wishbone_slave_in;
  signal pipelined_miso   : t_wishbone_slave_out;

  signal classic_mosi     : t_wishbone_master_out;
  signal classic_miso     : t_wishbone_master_in;

begin

  ---- generate clock and reset signal -------
  clk_ext <= not clk_ext  after clk_ext_period/2;
  clk_sys <= not clk_sys  after clk_sys_period/2;
  rst     <= '0'         after clk_sys_period;
  rst_n   <= not rst;
  --------------------------------------------


  adapter  : entity work.wb_slave_adapter 
    generic map(
        g_master_use_struct  => true,
        g_master_mode        => CLASSIC,
        g_master_granularity => WORD,
        g_slave_use_struct   => true,
        g_slave_mode         => PIPELINED,
        g_slave_granularity  => WORD
      )
    port map (
        clk_sys_i => clk_sys,
        rst_n_i   => rst_n,

    -- slave port (i.e. wb_slave_adapter is slave)
        slave_i  => pipelined_mosi,
        slave_o  => pipelined_miso,

    -- master port (i.e. wb_slave_adapter is master)
        master_i => classic_miso,
        master_o => classic_mosi
      );


    controller_pipelined: process
    begin
      wait until falling_edge(rst);
                      -- cyc  stb      adr              sel          we       dat
      pipelined_mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
                  --   ack  err  rty stall     dat
      wait until rising_edge(clk_sys);
      pipelined_mosi <= ('1', '1', (others => '0'), (others => '0'), '0', (others => '0'));

    end process;


    device_classic: process
    begin
      wait until rising_edge(clk_sys);
      if rst = '1' then
        classic_miso <= ('0', '0', '0', '0', (others => '0'));
      else 
        classic_miso.ack <= classic_mosi.cyc and classic_mosi.stb;
      end if;
    end process;



end architecture;



