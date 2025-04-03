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
  constant c_channel_num    : positive  := 2; -- up to 32

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
  signal s_tb_pwm_latch : std_logic := '0';
  signal s_tb_pwm_out   : std_logic_vector(c_channel_num-1 downto 0) := (others => '0');

  -- PWM reg addresses
  --type t_pwm_ch_addr_array is array(0 to (c_channel_num-1)) of std_logic_vector(31 downto 0);
  --signal pwm_addr_array : t_pwm_ch_addr_array := (others => (others => '0'));

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

  constant c_ch_16   : std_logic_vector(31 downto 0) :=x"00000040";
  constant c_ch_17   : std_logic_vector(31 downto 0) :=x"00000044";
  constant c_ch_18   : std_logic_vector(31 downto 0) :=x"00000048";
  constant c_ch_19   : std_logic_vector(31 downto 0) :=x"0000004C";

  constant c_ch_20   : std_logic_vector(31 downto 0) :=x"00000050";
  constant c_ch_21   : std_logic_vector(31 downto 0) :=x"00000054";
  constant c_ch_22   : std_logic_vector(31 downto 0) :=x"00000058";
  constant c_ch_23   : std_logic_vector(31 downto 0) :=x"0000005C";

  constant c_ch_24   : std_logic_vector(31 downto 0) :=x"00000060";
  constant c_ch_25   : std_logic_vector(31 downto 0) :=x"00000064";
  constant c_ch_26   : std_logic_vector(31 downto 0) :=x"00000068";
  constant c_ch_27   : std_logic_vector(31 downto 0) :=x"0000006C";

  constant c_ch_28   : std_logic_vector(31 downto 0) :=x"00000070";
  constant c_ch_29   : std_logic_vector(31 downto 0) :=x"00000074";
  constant c_ch_30   : std_logic_vector(31 downto 0) :=x"00000078";
  constant c_ch_31   : std_logic_vector(31 downto 0) :=x"0000007C";

  constant c_test_value     : std_logic_vector(31 downto 0) :=x"DEADBEEF";

  constant c_ch_0_value     : std_logic_vector(31 downto 0) :=x"00020004";
  constant c_ch_1_value     : std_logic_vector(31 downto 0) :=x"000F001E";
  constant c_ch_2_value     : std_logic_vector(31 downto 0) :=x"002D003C";
  constant c_ch_3_value     : std_logic_vector(31 downto 0) :=x"004B005A";
  constant c_ch_4_value     : std_logic_vector(31 downto 0) :=x"00690078";
  constant c_ch_5_value     : std_logic_vector(31 downto 0) :=x"00D200F0";

  --constant c_ch_00_mode     : std_logic_vector(31 downto 0) :=x"00000100";
  constant c_ch_00_mode     : std_logic_vector(31 downto 0) :=x"00000010";
  constant c_mode_latch     : std_logic_vector(31 downto 0) :=x"00000000";
  
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

      pwm_latch_i       : in std_logic;
      pwm_o             : out std_logic_vector(c_channel_num-1 downto 0)
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

        pwm_latch_i     => s_tb_pwm_latch,
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
            report("Test in free-running mode");
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
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_00, c_ch_0_value);
            report("WRITE: c_ch_00");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            for i in 0 to 5 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
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
            ----
            ----
            ---- WRITE CHANNEL 1
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_01, c_ch_1_value);
            --report("WRITE: c_ch_01");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 2
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_02, c_ch_2_value);
            --report("WRITE: c_ch_02");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 3
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_03, c_ch_3_value);
            --report("WRITE: c_ch_03");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 4
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_04, c_ch_4_value);
            --report("WRITE: c_ch_04");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 5
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_05, c_ch_5_value);
            --report("WRITE: c_ch_05");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 6
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_06, c_ch_0_value);
            --report("WRITE: c_ch_06");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 7
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_07, c_ch_1_value);
            --report("WRITE: c_ch_07");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 8
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_08, c_ch_2_value);
            --report("WRITE: c_ch_08");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 9
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_09, c_ch_3_value);
            --report("WRITE: c_ch_09");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 10
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_10, c_ch_4_value);
            --report("WRITE: c_ch_10");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 11
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_11, c_ch_5_value);
            --report("WRITE: c_ch_11");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 12
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_12, c_ch_0_value);
            --report("WRITE: c_ch_12");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 13
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_13, c_ch_1_value);
            --report("WRITE: c_ch_13");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 14
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_14, c_ch_2_value);
            --report("WRITE: c_ch_14");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 15
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_15, c_ch_3_value);
            --report("WRITE: c_ch_15");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 16
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_16, c_ch_4_value);
            --report("WRITE: c_ch_16");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 17
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_17, c_ch_5_value);
            --report("WRITE: c_ch_17");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 18
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_18, c_ch_0_value);
            --report("WRITE: c_ch_18");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 19
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_19, c_ch_1_value);
            --report("WRITE: c_ch_19");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 20
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_20, c_ch_2_value);
            --report("WRITE: c_ch_20");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 21
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_21, c_ch_3_value);
            --report("WRITE: c_ch_21");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 22
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_22, c_ch_4_value);
            --report("WRITE: c_ch_22");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 23
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_23, c_ch_5_value);
            --report("WRITE: c_ch_23");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 24
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_24, c_ch_0_value);
            --report("WRITE: c_ch_24");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 25
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_25, c_ch_1_value);
            --report("WRITE: c_ch_25");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 26
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_26, c_ch_2_value);
            --report("WRITE: c_ch_26");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 27
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_27, c_ch_3_value);
            --report("WRITE: c_ch_27");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 28
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_28, c_ch_4_value);
            --report("WRITE: c_ch_28");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 29
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_29, c_ch_5_value);
            --report("WRITE: c_ch_29");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 30
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_30, c_ch_0_value);
            --report("WRITE: c_ch_30");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            ----
            ---- WRITE CHANNEL 31
            --wait until rising_edge(s_clk);
            --s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_31, c_ch_1_value);
            ---- start counter with enable
            --s_tb_pwm_latch <= '1';
            --report("WRITE: c_ch_31");
            --while (s_wb_master_in.ack = '0') loop
            --  wait until rising_edge(s_clk);
            --end loop;
            --s_wb_master_out  <= wb_stim(c_cyc_off, c_str_off, c_we_off,  c_reg_all_zero, c_reg_all_zero);
            ----
            --for i in 0 to 5 loop
            --  wait until rising_edge(s_clk);
            --end loop; -- Waiter
            ----
            ----
            --
            --
            report("Test in latched mode");
            -- 
            --
            -- WRITE CHANNEL 0
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_00, c_reg_all_zero);
            report("Turn Channel 0 off");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            --
            --
            -- WRITE CHANNEL 0
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_00_mode, c_mode_latch);
            report("WRITE: c_ch_0 -> mode latched");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            --
            --
            wait until rising_edge(s_clk);
            s_wb_master_out  <= wb_stim(c_cyc_on, c_str_on, c_we_on, c_ch_00, c_ch_0_value);
            report("WRITE: c_ch_00");
            while (s_wb_master_in.ack = '0') loop
              wait until rising_edge(s_clk);
            end loop;
            --
            --
            for i in 0 to 20 loop
              wait until rising_edge(s_clk);
            end loop; -- Waiter
            --
            --
            -- Now show a rising edge on the trigger line
            s_tb_pwm_latch <= '1';
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
      end process;

end architecture;




