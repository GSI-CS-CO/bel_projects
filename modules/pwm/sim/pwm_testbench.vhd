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
  constant c_reg_width      : positive  := 16; -- up to 16
  constant c_channel_num    : positive  := 32; -- up to 32

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
  signal s_tb_pwm_enable: std_logic_vector(c_channel_num-1 downto 0) := (others => '0');
  signal s_tb_pwm_out   : std_logic_vector(c_channel_num-1 downto 0) := (others => '0');

  -- PWM reg addresses
  --constant c_ch_0_high  : std_logic_vector(31 downto 0) := "00000000000000000000000000000000"; -- 0x0200
  constant c_ch_00   : std_logic_vector(31 downto 0) :=x"00000000";
  constant c_ch_01   : std_logic_vector(31 downto 0) :=x"00000004";
  constant c_ch_02   : std_logic_vector(31 downto 0) :=x"00000008";
  constant c_ch_03   : std_logic_vector(31 downto 0) :=x"0000000C";

  constant c_ch_04   : std_logic_vector(31 downto 0) :=x"00000010";
  constant c_ch_05   : std_logic_vector(31 downto 0) :=x"00000014";
  constant c_ch_06   : std_logic_vector(31 downto 0) :=x"00000018";
  constant c_ch_07   : std_logic_vector(31 downto 0) :=x"0000001C";

  constant c_ch_08   : std_logic_vector(31 downto 0) :=x"00000020";
  constant c_ch_09   : std_logic_vector(31 downto 0) :=x"00000024";
  constant c_ch_10   : std_logic_vector(31 downto 0) :=x"00000028";
  constant c_ch_11   : std_logic_vector(31 downto 0) :=x"0000002C";

  constant c_ch_12   : std_logic_vector(31 downto 0) :=x"00000030";
  constant c_ch_13   : std_logic_vector(31 downto 0) :=x"00000034";
  constant c_ch_14   : std_logic_vector(31 downto 0) :=x"00000038";
  constant c_ch_15   : std_logic_vector(31 downto 0) :=x"0000003C";

  constant c_test_value     : std_logic_vector(31 downto 0) :=x"DEADBEEF";

  constant c_ch_0_value     : std_logic_vector(31 downto 0) :=x"00020004";
  constant c_ch_1_value     : std_logic_vector(31 downto 0) :=x"000F001E";
  constant c_ch_2_value     : std_logic_vector(31 downto 0) :=x"002D003C";
  constant c_ch_3_value     : std_logic_vector(31 downto 0) :=x"004B005A";
  constant c_ch_4_value     : std_logic_vector(31 downto 0) :=x"00690078";
  constant c_ch_5_value     : std_logic_vector(31 downto 0) :=x"00D200F0";
  
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic; adr: t_wishbone_address;
                   dat : t_wishbone_data) return t_wishbone_slave_in is
  variable v_setup : t_wishbone_slave_in;
  begin
    v_setup.cyc := cyc;
    v_setup.stb := stb;
    v_setup.we  := we;
    v_setup.adr := adr;
    v_setup.dat := dat;
    v_setup.sel := (others => '0'); -- Don't care
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
      g_pwm_channel_num : natural;
      g_pwm_regs_size   : natural
  );

    port(
      clk_sys_i         : in std_logic;
      rst_sys_n_i       : in std_logic;

      t_wb_o            : out t_wishbone_slave_out;
      t_wb_i            : in  t_wishbone_slave_in;

      pwm_enable_i      : in std_logic_vector(g_pwm_channel_num-1 downto 0);
      pwm_o             : out std_logic_vector(g_pwm_channel_num-1 downto 0)
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

        pwm_enable_i    => s_tb_pwm_enable,
        pwm_o           => s_tb_pwm_out);


    -- generate clock
    p_clock : process
      begin
        s_clk <= '0';
        wait for c_clock_cycle/2;
        s_clk <= '1';
        wait for c_clock_cycle/2;
      end process;

    -- reset controller
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
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
            report("Resetting...");
            wait until rising_edge(s_rst_n);
            -- RESET inactive
            --
            --
            -- WRITE CHANNEL 0
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_00,c_ch_0_value);
            report("WRITE: c_ch_0");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            -- start counter with enable
            s_tb_pwm_enable(0) <= '1';
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- READ CHANNEL 0
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_off, c_ch_00, c_reg_all_zero);
            report("READ: c_ch_00");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero,c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 1
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_01, c_ch_1_value);
            -- start counter with enable
            s_tb_pwm_enable(1) <= '1';
            report("WRITE: c_ch_01");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 2
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_02, c_ch_2_value);
            -- start counter with enable
            s_tb_pwm_enable(2) <= '1';
            report("WRITE: c_ch_02");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 3
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_03, c_ch_3_value);
            -- start counter with enable
            s_tb_pwm_enable(3) <= '1';
            report("WRITE: c_ch_03");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 4
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_04, c_ch_4_value);
            -- start counter with enable
            s_tb_pwm_enable(4) <= '1';
            report("WRITE: c_ch_04");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            --
            -- WRITE CHANNEL 5
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_05, c_ch_5_value);
            -- start counter with enable
            s_tb_pwm_enable(5) <= '1';
            report("WRITE: c_ch_05");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            --
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --

      end process;

end architecture;




