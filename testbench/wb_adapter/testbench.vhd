library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  
use ieee.math_real.all;

-- package with component to test on this testbench
use work.wishbone_pkg.all;


entity testbench is
    shared variable seed1 : integer := 999;
    shared variable seed2 : integer := 999;
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


  impure function rand_std_logic_vector(len : integer) return std_logic_vector is
    variable r : real;
    variable result : std_logic_vector(len - 1 downto 0);
  begin
    for i in result'range loop
      uniform(seed1, seed2, r);
      if r > 0.5 then result(i) := '1';
                 else result(i) := '0'; 
                 end if;
    end loop;
    return result;
  end function;  
  impure function rand_std_logic return std_logic is
  begin
    return rand_std_logic_vector(1)(0);
  end;

  impure function rand_integer(min_val, max_val : integer) return integer is
  variable r : real;
  begin
    uniform(seed1, seed2, r);
    return integer( round(r * real(max_val - min_val + 1) + real(min_val) - 0.5) );
  end function;

  procedure pipelined_wb_write_cycle(signal clk  : in std_logic;
                                     signal mosi : out t_wishbone_master_out;
                                     signal miso : in  t_wishbone_master_in;
                                     num_strobes : in integer) is
    variable cyc_to_stb_delay : integer;
    variable dat : std_logic_vector(31 downto 0);
    variable adr : std_logic_vector(31 downto 0);
    variable sel : std_logic_vector( 3 downto 0);
  begin
          -- cyc  stb      adr              sel          we       dat
    mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    wait until rising_edge(clk);
    mosi.cyc <= '1';
    for s in 1 to num_strobes loop
      cyc_to_stb_delay := rand_integer(0,2);
      dat := rand_std_logic_vector(32);
      adr := rand_std_logic_vector(30) & "00";
      sel := rand_std_logic_vector(4);
      for i in 0 to cyc_to_stb_delay loop
        wait until rising_edge(clk);
      end loop;
      -- do the strobe 
      mosi <= (cyc=>'1', stb=>'1', adr=>adr, sel=>sel, we=>'1', dat=>dat);
      if (miso.stall /= '0') then 
        wait until rising_edge(clk);
        mosi <= ('1', '0', (others => '0'), (others => '0'), rand_std_logic, (others => '0'));
        wait until miso.stall = '0'; 
      end if;
      if (miso.ack /= '1') then 
        wait until rising_edge(clk);
        mosi <= ('1', '0', (others => '0'), (others => '0'), rand_std_logic, (others => '0'));
        wait until miso.ack = '1'; 
      end if;
      wait until rising_edge(clk);
      mosi <= ('1', '0', (others => '0'), (others => '0'), rand_std_logic, (others => '0'));
      mosi.stb <= '0';
    end loop;
          -- cyc  stb      adr              sel          we       dat
    mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    wait until rising_edge(clk);
  end procedure;    


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
      --pipelined_wb_write_cycle(clk_sys, pipelined_mosi, pipelined_miso, 5);
      --                -- cyc  stb      adr              sel          we       dat
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
        classic_miso.ack <= classic_mosi_stb_d2;


        ------------ CASE 4 (not working) -------------
        --like CASE 5 but ack is delayed by 1 clock cycle
        --classic_miso.ack <= classic_mosi.stb and classic_mosi.cyc;

      end if;
    end process;

    ------------ CASE 5 (working) -------------
    --classic_miso.ack <= classic_mosi.stb and classic_mosi.cyc;



end architecture;



