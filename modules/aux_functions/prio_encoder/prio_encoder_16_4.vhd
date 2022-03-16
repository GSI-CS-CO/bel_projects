library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity prio_encoder_16_4 is
  port (
    input : in std_logic_vector(15 downto 0);
    index : out std_logic_vector(3 downto 0);
    valid : out std_logic
  );
end entity;

architecture arch of prio_encoder_16_4 is
begin
  prio_encode: process(input)
  begin
    if input(0) = '1' then
      index <= x"0";
      valid <= '1';
    elsif input(1) = '1' then
      index <= x"1";
      valid <= '1';
    elsif input(2) = '1' then
      index <= x"2";
      valid <= '1';
    elsif input(3) = '1' then
      index <= x"3";
      valid <= '1';
    elsif input(4) = '1' then
      index <= x"4";
      valid <= '1';
    elsif input(5) = '1' then
      index <= x"5";
      valid <= '1';
    elsif input(6) = '1' then
      index <= x"6";
      valid <= '1';
    elsif input(7) = '1' then
      index <= x"7";
      valid <= '1';
    elsif input(8) = '1' then
      index <= x"8";
      valid <= '1';
    elsif input(9) = '1' then
      index <= x"9";
      valid <= '1';
    elsif input(10) = '1' then
      index <= x"a";
      valid <= '1';
    elsif input(11) = '1' then
      index <= x"b";
      valid <= '1';
    elsif input(12) = '1' then
      index <= x"c";
      valid <= '1';
    elsif input(13) = '1' then
      index <= x"d";
      valid <= '1';
    elsif input(14) = '1' then
      index <= x"e";
      valid <= '1';
    elsif input(15) = '1' then
      index <= x"f";
      valid <= '1';
    else
      index <= "0000";
      valid <= '0';
    end if;

  end process;
end architecture;
