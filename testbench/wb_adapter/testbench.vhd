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

  signal classic_mosi_1   : t_wishbone_master_out;
  signal classic_miso_1   : t_wishbone_master_in := ('0', '0', '0', '0', (others => '0'));

  signal classic_mosi_2   : t_wishbone_master_out;
  signal classic_miso_2   : t_wishbone_master_in := ('0', '0', '0', '0', (others => '0'));

  signal classic_mosi_3   : t_wishbone_master_out;
  signal classic_miso_3   : t_wishbone_master_in := ('0', '0', '0', '0', (others => '0'));

  signal classic_mosi_4   : t_wishbone_master_out;
  signal classic_miso_4   : t_wishbone_master_in := ('0', '0', '0', '0', (others => '0'));

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
    variable valid_stb : integer := 0;
    variable num_ack   : integer := 0;
    variable stb : std_logic := '0';
  begin
          -- cyc  stb      adr              sel          we       dat
    mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    wait until rising_edge(clk);
    --mosi.cyc <= '1';
    while valid_stb < num_strobes loop
      dat := rand_std_logic_vector(32);
      adr := rand_std_logic_vector(30) & "00";
      sel := rand_std_logic_vector(4);
      stb := rand_std_logic;
      mosi <= (cyc=>'1', stb=>stb, adr=>adr, sel=>sel, we=>'1', dat=>dat);
      wait until rising_edge(clk);
      if stb = '1' and miso.stall = '0' then valid_stb := valid_stb + 1; end if;
      if miso.ack = '1' then num_ack := num_ack   + 1; end if;
    end loop;

    mosi.stb <= '0';
    while num_ack < num_strobes loop
      wait until rising_edge(clk);
      if miso.ack   = '1' then num_ack   := num_ack   + 1; end if;
    end loop;
          -- cyc  stb      adr              sel          we       dat
    mosi <= ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    wait until rising_edge(clk);
  end procedure;    


  signal stb_d1, stb_d2 : std_logic := '0';

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
      for i in 1 to 10 loop
        pipelined_wb_write_cycle(clk_sys, pipelined_mosi, pipelined_miso, rand_integer(1,5));
        wait until rising_edge(clk_sys);
      end loop;
      wait;
    end process;


    classic_mosi_1 <= classic_mosi when classic_mosi.adr(31 downto 30) = "00"
                else ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    classic_mosi_2 <= classic_mosi when classic_mosi.adr(31 downto 30) = "01"
               else ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    classic_mosi_3 <= classic_mosi when classic_mosi.adr(31 downto 30) = "10"
                else ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));
    classic_mosi_4 <= classic_mosi when classic_mosi.adr(31 downto 30) = "11"
               else ('0', '0', (others => '0'), (others => '0'), '0', (others => '0'));

    classic_miso <= classic_miso_1 when classic_mosi.adr(31 downto 30) = "00"
               else classic_miso_2 when classic_mosi.adr(31 downto 30) = "01"
               else classic_miso_3 when classic_mosi.adr(31 downto 30) = "10"
               else classic_miso_4;


    -- slave 1
    classic_miso_1.ack <= classic_mosi_1.stb and classic_mosi_1.cyc;

    slave_2: process
    begin
      wait until rising_edge(clk_sys);
      if classic_mosi_2.cyc = '1' then
        classic_miso_2.ack <= classic_mosi_2.stb;
      end if;
    end process;

    slave_3: process
    begin
      wait until rising_edge(clk_sys);
      stb_d1 <= classic_mosi_3.stb;
      stb_d2 <= stb_d1;
      if classic_mosi_3.cyc = '1' then
        classic_miso_3.ack <= stb_d2;
      end if;
    end process;

    slave_4: process
    begin
      wait until rising_edge(clk_sys);
      if classic_mosi_4.cyc = '1' then
        classic_miso_4.ack <= classic_mosi_4.stb;
      end if;
    end process;


    check: process(pipelined_mosi, clk_sys) 
      variable stb_counter : integer := 0;
      variable ack_counter : integer := 0;
    begin
      if rising_edge(clk_sys) then
        if pipelined_mosi.cyc = '0' then
          assert(pipelined_mosi.stb /= '1');
          assert(pipelined_miso.ack /= '1');
        end if;
        if classic_mosi.cyc = '1' then
          if classic_mosi.stb = '1' then 
            stb_counter := stb_counter + 1;
          end if;
          if classic_miso.ack = '1' then
            ack_counter := ack_counter + 1;
          end if;
          assert (stb_counter-ack_counter = 0 or 
                  stb_counter-ack_counter = 1);
        end if;
      end if;
    end process;


end architecture;



