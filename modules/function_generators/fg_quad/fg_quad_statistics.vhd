library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;


entity fg_quad_statistics is
  generic (
    clk_in_hz        : integer
  );
  port (
    clk              : in std_logic;
    reset            : in std_logic;
    data_in          : in std_logic_vector(7 downto 0);
    reset_statistics : in std_logic;
    read_from_buffer : in std_logic;
    min_value        : out std_logic_vector(15 downto 0);
    max_value        : out std_logic_vector(15 downto 0);
    avg_value        : out std_logic_vector(15 downto 0);
    buffered_values  : out std_logic_vector(15 downto 0);
    fill_level       : out std_logic_vector(15 downto 0)

  );
end entity;

architecture arch of fg_quad_statistics is
  signal dreq      : std_logic;
  signal ack       : std_logic;
  signal min_time  : integer := 0;
  signal max_time  : integer := 0;
  signal avg_time  : integer := 0;
  type state_type is (idle, wait_for_ack, timeout);
  signal sm_state              : state_type;
  constant timeout_cycles      : integer                                   := 400;
  signal count                 : integer;
  signal wr_value_to_buffer    : std_logic                                 := '0';
  signal fill_count            : integer;
  constant c_prescaler_cnt_1us : integer                                   := (clk_in_hz / 1000000) - 2;
  constant c_prescaler_width   : integer                                   := integer(floor(log2(real(c_prescaler_cnt_1us)))) + 2;
  signal prescale_cnt          : unsigned(c_prescaler_width - 1 downto 0);
  signal s_1us_en              : std_logic;
begin

  prescale_1us: process (clk, reset)
  begin
    if reset = '1' then
      prescale_cnt <= to_unsigned(c_prescaler_cnt_1us, c_prescaler_width);
    elsif rising_edge(clk) then
      if prescale_cnt(prescale_cnt'high) = '1' then
        prescale_cnt <= to_unsigned(c_prescaler_cnt_1us, c_prescaler_width);
      else
        prescale_cnt <= prescale_cnt - 1;
      end if;
    end if;
  end process;
  s_1us_en <= prescale_cnt(prescale_cnt'high);

  sync_reg: process (clk, reset, data_in)
  begin
    if reset = '1' then
      dreq     <= '0';
      ack      <= '0';
    elsif rising_edge(clk) then
      dreq <= data_in(0);
      ack  <= data_in(1);
    end if;
  end process;

  rb: ring_buffer
    generic map (
      RAM_WIDTH => 16,
      RAM_DEPTH => 512
    )
    port map (
      clk        => clk,
      rst        => reset,
      wr_en      => wr_value_to_buffer,
      wr_data    => std_logic_vector(to_unsigned(count, 16)),
      rd_en      => read_from_buffer,
      rd_valid   => open,
      rd_data    => buffered_values,

      empty      => open,
      empty_next => open,
      full       => open,
      full_next  => open,

      fill_count => fill_count
    );

  sm: process (clk, reset, ack)
  begin
    if reset = '1' then
      sm_state <= idle;
      min_time <= 0;
      max_time <= 0;
      avg_time <= 0;
      wr_value_to_buffer <= '0';
    elsif rising_edge(clk) then
      wr_value_to_buffer <= '0';
      case sm_state is
        when idle =>
          count <= 0;
          if dreq = '1' then
            sm_state <= wait_for_ack;
          end if;

        when wait_for_ack =>
          if ack = '1' then
            sm_state <= idle;
            wr_value_to_buffer <= '1';
            if count > max_time then
              max_time <= count;
            end if;
            -- init min_time with a plausible count value
            if min_time > 0 then
              if count < min_time then
                min_time <= count;
              end if;
            else
              min_time <= count;
            end if;
          else
            if s_1us_en = '1' then
              count <= count + 1;
            end if;
          end if;
          
        when timeout =>
      end case;
    end if;

  end process;
  min_value <= std_logic_vector(to_unsigned(min_time, 16));
  max_value <= std_logic_vector(to_unsigned(max_time, 16));
  avg_value <= std_logic_vector(to_unsigned(avg_time, 16));
  fill_level <= std_logic_vector(to_unsigned(fill_count, 16));

    
end architecture;
