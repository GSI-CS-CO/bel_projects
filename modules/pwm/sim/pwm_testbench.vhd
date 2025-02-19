--wishbone secondary testbecnh
--bare bones testbench to test slave communication
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library std;
use std.textio.all;

library work;
use work.wishbone_pkg.all;


--Entity
entity pwm_testbench is
generic (g_local_pwm_channel_num : integer := 8);
end;

architecture pwm_testbench_architecture of pwm_testbench is

  --Testbench settings
  constant c_reset_time   : time  := 200 ns;
  constant c_clock_cycle  : time  := 16 ns;
  constant c_light_time   : time  := 32 ns;
  constant c_sim_time     : time  := 20 ms;

  -- Other constants
  constant c_reg_all_zero                : std_logic_vector(31 downto 0) := ( others => '0');
  constant c_cyc_on                      : std_logic := '1';
  constant c_cyc_off                     : std_logic := '0';
  constant c_str_on                      : std_logic := '1';
  constant c_str_off                     : std_logic := '0';
  constant c_we_on                       : std_logic := '1';
  constant c_we_off                      : std_logic := '0';

  constant c_pwm_allzero   : std_logic_vector(31 downto 0) := ( others => '0');
  constant c_pwm_allone    : t_wishbone_data := ( others => '1');
  constant c_pwm_fifteen   : t_wishbone_data := ( 0 => '1', 1 => '1', 2 => '1', 3 => '1',others => '0');

  constant c_pwm_0_1       : t_wishbone_data := x"00000001";
  constant c_pwm_0_2       : t_wishbone_data := x"00000002";

  constant c_pwm_1_1       : t_wishbone_data := x"00010001";
  constant c_pwm_1_2       : t_wishbone_data := x"00010002";
  constant c_pwm_1_4       : t_wishbone_data := x"00010004";

  constant c_pwm_2_0       : t_wishbone_data := x"00020000";
  constant c_pwm_2_1       : t_wishbone_data := x"00020001";
  constant c_pwm_2_2       : t_wishbone_data := x"00020002";
  constant c_pwm_2_10      : t_wishbone_data := x"0002000A";
  constant c_pwm_2_254     : t_wishbone_data := x"0200FE00";

  constant c_pwm_3_1       : t_wishbone_data := x"00030001";
  constant c_pwm_3_2       : t_wishbone_data := x"00030002";
  
  constant c_pwm_4_1       : t_wishbone_data := x"00040001";
  constant c_pwm_4_2       : t_wishbone_data := x"00040002";
  constant c_pwm_4_3       : t_wishbone_data := x"00040003";
  constant c_pwm_4_8       : t_wishbone_data := x"00040008";

  constant c_pwm_5_3       : t_wishbone_data := x"00050003";
  constant c_pwm_10_1024   : t_wishbone_data := x"A0000040";

  constant c_pwm_adr_config       : t_wishbone_address := x"00000000";
  constant c_pwm_adr_channel_num  : t_wishbone_address := x"00000004";
  constant c_pwm_adr_dc_ch_0      : t_wishbone_address := x"00000008";
  constant c_pwm_adr_dc_ch_1      : t_wishbone_address := x"0000000C";
  constant c_pwm_adr_dc_ch_2      : t_wishbone_address := x"00000010";
  constant c_pwm_adr_dc_ch_3      : t_wishbone_address := x"00000014";
  constant c_pwm_adr_dc_ch_4      : t_wishbone_address := x"00000018";
  constant c_pwm_adr_dc_ch_5      : t_wishbone_address := x"0000001C";
  constant c_pwm_adr_dc_ch_6      : t_wishbone_address := x"00000020";
  constant c_pwm_adr_dc_ch_7      : t_wishbone_address := x"00000024";

  constant c_pwm_adr_test  : t_wishbone_address := x"DEADBEEF";
  
  constant c_pwm_ch_0      : t_wishbone_byte_select := ( 0 => '0', 1 => '0', 2 => '0', 3 => '0');
  constant c_pwm_ch_1      : t_wishbone_byte_select := ( 0 => '1', 1 => '0', 2 => '0', 3 => '0');
  constant c_pwm_ch_2      : t_wishbone_byte_select := ( 0 => '0', 1 => '1', 2 => '0', 3 => '0');

  constant c_test_data    : std_logic_vector(31 downto 0) := x"DEADBEEF";
  
  -- Basic device signals
  signal s_clk        : std_ulogic := '0';
  signal s_rst_n      : std_ulogic := '1';
  
  -- Wishbone connections
  signal s_wb_master_in  : t_wishbone_slave_out;  -- equal to t_wishbone_master_in
  signal s_wb_master_out : t_wishbone_slave_in;   -- equal to t_wishbone_master_out

  -- PWM specific signals
  --signal s_tb_pwm_out : std_logic;
  signal s_tb_pwm_out: std_logic_vector(7 downto 0);
  
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic; adr: t_wishbone_address;
                   sel : t_wishbone_byte_select; dat : t_wishbone_data) return t_wishbone_slave_in is
  variable v_setup : t_wishbone_slave_in;
  begin
    v_setup.cyc := cyc;
    v_setup.stb := stb;
    v_setup.we  := we;
    v_setup.adr := adr;
    v_setup.dat := dat;
    v_setup.sel := sel;
    return v_setup;
  end function wb_stim;

function to_integer( s : std_logic ) return natural is
begin
      if s = '1' then
      return 1;
   else
      return 0;
   end if;
end function;

  --! @brief Return a intrepresentation of a logic value
function to_logic_to_int(x : std_logic) return natural is
  begin
    return to_integer(x);
  end function;


  component pwm is

    generic (
      g_simulation    : in boolean
  );

    port(
    s_clk_sys_i       : in std_logic;
    s_rst_sys_n_i     : in std_logic;

    t_wb_out          : out t_wishbone_slave_out;
    t_wb_in           : in  t_wishbone_slave_in;

    s_pwm_o           : out std_logic_vector(7 downto 0)
    );

  end component;

  begin

    dut : pwm
    generic map
      (
        g_simulation      => true
        --g_pwm_channel_num => g_local_pwm_channel_num 
      )
      port map (
        s_clk_sys_i     => s_clk,
        s_rst_sys_n_i   => s_rst_n,

        t_wb_out        => s_wb_master_in,
        t_wb_in         => s_wb_master_out,

        s_pwm_o         => s_tb_pwm_out);


    -- generate clock
    p_clock : process
      begin
        s_clk <= '0';
        wait for c_clock_cycle/2;
        s_clk <= '1';
        wait for c_clock_cycle/2;
      end process;

    --Reset controller
    p_reset : process
    begin
       s_rst_n <= '0';
       wait for c_reset_time;
       s_rst_n <= '1';
       wait for c_sim_time;
    end process;

    p_test: process
        begin
            -- RESET active
            --
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            report("Resetting...");
            wait until rising_edge(s_rst_n);
            -- RESET inactive
            --
            --
            -- WRITE CONFIG REGISTER 1
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_config, c_pwm_ch_0, c_pwm_4_2);
            report("WRITE: Writing c_pwm_4_2  at c_pwm_adr_config");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CONFIG REGISTER 1
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_config");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 0
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_0, c_pwm_ch_0, c_pwm_0_1);
            report("WRITE: Trigger load Channel 0 - Writing c_pwm_0_1 at c_pwm_adr_dc_ch_0");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 0
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_0, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_0");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 1
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_1, c_pwm_ch_0, c_pwm_0_2);
            report("WRITE: Trigger load Channel 1 - Writing c_pwm_0_2 at c_pwm_adr_dc_ch_1");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 1
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_1, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_1");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 2
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_2, c_pwm_ch_0, c_pwm_4_2 );
            report("WRITE: Trigger load Channel 2 - Writing c_pwm_4_2 at c_pwm_adr_dc_ch_2");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 2
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_2, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_2");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 3
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_3, c_pwm_ch_0, c_pwm_4_1 );
            report("WRITE: Trigger load Channel 3 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_3");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 3
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_3, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_3");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 4
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_4, c_pwm_ch_0, c_pwm_4_1 );
            report("WRITE: Trigger load Channel 4 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_4");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 4
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_4, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_4");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 5
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_5, c_pwm_ch_0, c_pwm_4_1 );
            report("WRITE: Trigger load Channel 5 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_5");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 5
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_5, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_5");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 6
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_6, c_pwm_ch_0, c_pwm_4_1 );
            report("WRITE: Trigger load Channel 6 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_6");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 6
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_6, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_6");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 7
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_7, c_pwm_ch_0, c_pwm_4_1 );
            report("WRITE: Trigger load Channel 7 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_7");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 7
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_7, c_pwm_ch_0, c_pwm_allzero);
            report("READ: c_pwm_adr_dc_ch_7");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            --
            for i in 0 to 100 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --

      end process;

end architecture;




