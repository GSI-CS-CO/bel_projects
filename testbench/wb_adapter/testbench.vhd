library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
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


  signal classic_mosi_stb_d1: std_logic := '0';
  signal classic_mosi_stb_d2: std_logic := '0';

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


    master_pipelined: process
    begin
                      -- cyc  stb      adr              sel          we       dat
      pipelined_mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
      wait until falling_edge(rst);
      for i in 0 to 100 loop 
        wait until rising_edge(clk_sys);
        pipelined_mosi <= ('1', '1', std_logic_vector(to_unsigned(10*i,32)), x"f", '0', std_logic_vector(to_unsigned(i,32)));
        classic_miso.dat <= std_logic_vector(to_unsigned(i,32));
      end loop;
      wait;
    end process;

    slave_classic: process
    begin
      wait until rising_edge(clk_sys);
      if rst = '1' then
      else 
        -- delayed strobe
        classic_mosi_stb_d1 <= classic_mosi.stb;
        classic_mosi_stb_d2 <= classic_mosi_stb_d1;

        -------------- CASE 1 (working) -------------
         --ack is exacly one clock cycle long
        --if classic_mosi_stb_d1 = '0' and classic_mosi.stb = '1' then -- rising edge of stb
        --  classic_miso.ack <= '1';
        --else
        --  classic_miso.ack <= '0';
        --end if;

        -------------- CASE 2 (working) -------------
        ----same as CASE1 but slave needs one clock cycle longer to ack
        --if classic_mosi_stb_d2 = '0' and classic_mosi_stb_d1 = '1' then 
        --  classic_miso.ack <= '1';
        --else
        --  classic_miso.ack <= '0';
        --end if;



        -------------- CASE 3 (not working) -------------
        --classic_miso.ack <= classic_mosi_stb_d2;


        ------------ CASE 4 (not working) -------------
        --like CASE 5 but ack is delayed by 1 clock cycle
        classic_miso.ack <= classic_mosi.stb and classic_mosi.cyc;

      end if;
    end process;

    ------------ CASE 5 (working) -------------
    --classic_miso.ack <= classic_mosi.stb and classic_mosi.cyc;



end architecture;



