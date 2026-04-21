library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
USE IEEE.numeric_std.all;

entity clk_divider_by_5 is

Port (
    clk_in: in std_logic;
    nrst: in std_logic;
    clk_out: out std_logic
    );

end entity clk_divider_by_5;

architecture behavioral of clk_divider_by_5 is
    signal re :STD_LOGIC;
    signal fe :STD_LOGIC;
    signal cnt_re, cnt_fe: integer range 0 to 5;

begin

rising_edge_proc: process(clk_in,nrst)

begin

  if nrst = '0' then
    re <= '0';
  elsif rising_edge(clk_in) then
    if cnt_re = 5 then
        cnt_re <= 1;
        re <= '1';
    elsif cnt_re >= 2 then
        re <= '0';
        cnt_re <= cnt_re + 1;
    else
        re <= '1';
        cnt_re <= cnt_re + 1;
    end if;
  end if;

end process rising_edge_proc;


falling_ege_proc: process(clk_in,nrst)

begin

  if nrst = '1' then
    fe <= '0';
    cnt_fe <= 0;
  elsif falling_edge(clk_in) then
    if cnt_fe = 5 then
        cnt_fe <= 1;
        fe <= '1';
    elsif cnt_fe >= 2 then
        fe <= '0';
        cnt_fe <= cnt_fe + 1;
    else
        fe <= '1';
        cnt_fe <= cnt_fe + 1;
    end if;
  end if;

end process falling_ege_proc;

clk_out <= re or fe;

end Behavioral;
