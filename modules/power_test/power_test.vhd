LIBRARY ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;
use work.power_test_pkg.all;


entity power_test is
  generic(
    Clk_in_Hz:  integer := 125_000_000;
    pwm_width:  integer := 16;
    row_width:  integer := 64;
    row_cnt:    integer := 64
    );
  port(
    clk_i:          in    std_logic;
    nRst_i:         in    std_logic;
    slave_i:        in    t_wishbone_slave_in;
    slave_o:        out   t_wishbone_slave_out;
    pwm_o:          out   std_logic;
    or_o:           out   std_logic
    );
  end power_test;


architecture arch_power_test of power_test is



  constant  rdwr_pwm_cnt_a:   integer := 16#00#;  -- read/write pwm counter:   wb_power_test_offset + 16#00#

  type   toggle_states is (init, to_one, to_zero);
  signal sm_toggle:     toggle_states;
  signal toggle_row:    unsigned(row_width-1 downto 0);


  signal  ex_stall, ex_ack, ex_err, intr: std_logic;

  signal  rdwr_pwm_active_cnt:  std_logic;
  signal  pwm_active_cnt:       unsigned(pwm_width-1 downto 0);
  signal  s_pwm_o:              std_logic;

  begin

  slave_o.stall    <= ex_stall;
  slave_o.ack      <= ex_ack;
  slave_o.err      <= ex_err;
  slave_o.rty      <= '0';
  
  pwm_ist: entity work.pwm
    generic map(
      pwm_width => pwm_width
      )
    port map(
      clk_i     => clk_i,
      nrst_i    => nrst_i,
      enable    => '1',
      pwm_active_cnt  => pwm_active_cnt,
      pwm_o     => s_pwm_o
      );

  row_array_ins: entity work.row_array
    generic map(
      row_width   => row_width,
      row_cnt     => row_cnt
      )
    port map(
      clk_i   => clk_i,
      nrst_i  => nrst_i,
      row_i   => toggle_row,
      or_o    => or_o
      );

  p_toggle_data: process (clk_i, nrst_i)
    begin
      if nrst_i = '0' then           -- asynchronous reset
        sm_toggle <= init;
        toggle_row <= (others => '0');

      elsif rising_edge(clk_i) then  -- rising clock edge

        case sm_toggle is

          when init =>
            toggle_row <= (others => '0');
            if s_pwm_o = '1' then
              sm_toggle <= to_one;
            end if;

          when to_one =>
            toggle_row <= (others =>'1');
              sm_toggle <= to_zero;

          when to_zero =>
            toggle_row <= (others =>'0');
            if s_pwm_o = '1' then
              sm_toggle <= to_one;
            else
              sm_toggle <= init;
            end if;

          when others =>
            sm_toggle <= init;

        end case;
      end if;
  end process p_toggle_data;


  p_regs_acc: process (clk_i, nrst_i)
    begin
      if nrst_i = '0' then
        ex_stall        <= '1';
        ex_ack          <= '0';
        ex_err          <= '0';
        pwm_active_cnt  <= to_unsigned(0, pwm_active_cnt'length);

      elsif rising_edge(clk_i) then
        ex_stall        <= '1';
        ex_ack          <= '0';
        ex_err          <= '0';
        slave_o.dat <= (others => '0');

        if slave_i.cyc = '1' and slave_i.stb = '1' and ex_stall = '1' then

        -- begin of wishbone cycle
          case to_integer(unsigned(slave_i.adr(c_power_test_addr_width-1 downto 2))) is

          -- check existing register
            when rdwr_pwm_cnt_a =>         -- read or write pwm counter
              if slave_i.sel = "1111" then -- only modulo-4 address allowed
                if slave_i.we = '1' then
                  -- write pwm counter
                  pwm_active_cnt <= unsigned(slave_i.dat(pwm_width-1 downto 0));
                  ex_stall <= '0';
                  ex_ack <= '1';
                else
                  -- read status register
                  slave_o.dat(pwm_width-1 downto 0) <= std_logic_vector(pwm_active_cnt);
                  ex_stall <= '0';
                  ex_ack <= '1';
                end if;
              else
                -- access to high word or unaligned word is not allowed
                ex_stall <= '0';
                ex_err <= '1';
              end if;

            when others =>
              ex_stall <= '0';
              ex_err <= '1';

          end case;
        end if;
      end if;
    end process p_regs_acc;

  pwm_o <= s_pwm_o;

  end arch_power_test;
