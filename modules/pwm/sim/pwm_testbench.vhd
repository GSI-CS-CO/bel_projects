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

  -- PWM settings
  constant c_reg_width      : positive  := 16;
  constant c_channel_num    : positive  := 8;

  -- Other constants
  constant c_reg_all_zero                : std_logic_vector(31 downto 0) := ( others => '0');
  constant c_sel_all_zero                : std_logic_vector(3 downto 0) := ( others => '0');
  constant c_cyc_on                      : std_logic := '1';
  constant c_cyc_off                     : std_logic := '0';
  constant c_str_on                      : std_logic := '1';
  constant c_str_off                     : std_logic := '0';
  constant c_we_on                       : std_logic := '1';
  constant c_we_off                      : std_logic := '0';

  constant c_test_data    : std_logic_vector(31 downto 0) := x"DEADBEEF";
  
  -- Basic device signals
  signal s_clk        : std_ulogic := '0';
  signal s_rst_n      : std_ulogic := '1';
  
  -- Wishbone connections
  signal s_wb_master_in  : t_wishbone_slave_out;  -- equal to t_wishbone_master_in
  signal s_wb_master_out : t_wishbone_slave_in;   -- equal to t_wishbone_master_out

  -- PWM specific signals
  signal s_tb_pwm_out: std_logic_vector(c_channel_num-1 downto 0);

  -- PWM reg addresses
  
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
  end function to_integer;

  --! @brief Return a intrepresentation of a logic value
  function to_logic_to_int(x : std_logic) return natural is
    begin
      return to_integer(x);
  end function to_logic_to_int;


  component pwm is

  generic (
      g_simulation      : in boolean;
      g_pwm_channel_num : positive;
      g_pwm_regs_size   : positive
  );

    port(
      clk_sys_i       : in std_logic;
      rst_sys_n_i     : in std_logic;

      t_wb_o          : out t_wishbone_slave_out;
      t_wb_i          : in  t_wishbone_slave_in;

      pwm_o           : out std_logic_vector(g_pwm_channel_num-1 downto 0)
    );

  end component;

  begin

    dut : pwm
    generic map
      (
        g_simulation      => true,
        g_pwm_channel_num => c_channel_num,
        g_pwm_regs_size   => c_reg_width
      )
      port map (
        clk_sys_i       => s_clk,
        rst_sys_n_i     => s_rst_n,

        t_wb_o          => s_wb_master_in,
        t_wb_i          => s_wb_master_out,

        pwm_o           => s_tb_pwm_out);


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
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_sel_all_zero, c_reg_all_zero);
            report("Resetting...");
            wait until rising_edge(s_rst_n);
            -- RESET inactive
            --
            --
            -- WRITE
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_reg_all_zero, c_sel_all_zero, c_reg_all_zero);
            report("WRITE: Writing c_pwm_4_2  at c_pwm_adr_config");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_sel_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_reg_all_zero, c_sel_all_zero, c_reg_all_zero);
            report("READ: c_pwm_adr_config");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_sel_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            ----
            ---- WRITE CHANNEL 0
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_0, c_pwm_ch_0, c_pwm_0_1);
            --report("WRITE: Trigger load Channel 0 - Writing c_pwm_0_1 at c_pwm_adr_dc_ch_0");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 0
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_0, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_0");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 1
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_1, c_pwm_ch_0, c_pwm_0_2);
            --report("WRITE: Trigger load Channel 1 - Writing c_pwm_0_2 at c_pwm_adr_dc_ch_1");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 1
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_1, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_1");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 2
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_2, c_pwm_ch_0, c_pwm_4_2 );
            --report("WRITE: Trigger load Channel 2 - Writing c_pwm_4_2 at c_pwm_adr_dc_ch_2");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 2
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_2, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_2");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 3
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_3, c_pwm_ch_0, c_pwm_4_1 );
            --report("WRITE: Trigger load Channel 3 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_3");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 3
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_3, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_3");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 4
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_4, c_pwm_ch_0, c_pwm_4_1 );
            --report("WRITE: Trigger load Channel 4 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_4");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 4
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_4, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_4");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 5
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_5, c_pwm_ch_0, c_pwm_4_1 );
            --report("WRITE: Trigger load Channel 5 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_5");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 5
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_5, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_5");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 6
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_6, c_pwm_ch_0, c_pwm_4_1 );
            --report("WRITE: Trigger load Channel 6 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_6");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 6
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_6, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_6");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 7
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_pwm_adr_dc_ch_7, c_pwm_ch_0, c_pwm_4_1 );
            --report("WRITE: Trigger load Channel 7 - Writing c_pwm_4_1 at c_pwm_adr_dc_ch_7");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- READ CHANNEL 7
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_pwm_adr_dc_ch_7, c_pwm_ch_0, c_pwm_allzero);
            --report("READ: c_pwm_adr_dc_ch_7");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_pwm_adr_config, c_pwm_ch_0, c_pwm_allzero);
            ----
            --for i in 0 to 100 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----

      end process;

end architecture;




