library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_bus_pkg.all;

entity detect_backplane is
  generic (
    Clk_in_Hz      : integer;
    Time_out_in_ms : integer
  );
  port (
    clk_i         : in std_logic;
    rst_n_i       : in std_logic;
    trigger       : in std_logic;
    is_standalone : out std_logic
  );
end entity;

architecture arch of detect_backplane is

begin
  trigger_cnt: process (clk_i, rst_n_i, trigger)
    variable cnt : unsigned(20 downto 0) := to_unsigned(312500, 21);
    variable cnt_en : std_logic;
  begin
    if rst_n_i = '0' then
      cnt := to_unsigned(312500, 21);
      is_standalone <= '0';
      cnt_en := '1';
    elsif rising_edge(clk_i) then
      -- stop counting when timeout is reached
      if cnt(cnt'high) = '1' then
        cnt_en := '0';
      elsif trigger = '0' and cnt_en = '1' then
        cnt := cnt - 1;
      else
        cnt := to_unsigned(312500, 21);
      end if;
    is_standalone <= cnt(cnt'high);
    end if;
  end process;

  
end architecture;
