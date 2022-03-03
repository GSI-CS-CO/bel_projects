library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity prio_encoder_256_8 is
  port (
    input : in std_logic_vector(255 downto 0);
    index : out std_logic_vector(7 downto 0);
    valid : out std_logic
  );
end entity;

architecture arch of prio_encoder_256_8 is
  component prio_encoder_64_6 is
  port (
    input : in std_logic_vector(63 downto 0);
    index : out std_logic_vector(5 downto 0);
    valid : out std_logic
  );
  end component;


  signal index0, index1, index2, index3: std_logic_vector(5 downto 0);
  signal valid0, valid1, valid2, valid3: std_logic;
begin
  p_e_0: prio_encoder_64_6 
  port map (
    input => input(63 downto 0),
    index => index0,
    valid => valid0);
  p_e_1: prio_encoder_64_6 
  port map (
    input => input(127 downto 64),
    index => index1,
    valid => valid1);
  p_e_2: prio_encoder_64_6 
  port map (
    input => input(191 downto 128),
    index => index2,
    valid => valid2);
  p_e_3: prio_encoder_64_6 
  port map (
    input => input(255 downto 192),
    index => index3,
    valid => valid3);


  out_mux: process(index0, index1, index2, index3, valid0, valid1, valid2, valid3)
  begin
    if valid0 = '1' then
      index <= "00" & index0;
      valid <= '1';
    elsif valid1 = '1' then
      index <= "01" & index1;
      valid <= '1';
    elsif valid2 = '1' then
      index <= "10" & index2;
      valid <= '1';
    elsif valid3 = '1' then
      index <= "11" & index3;
      valid <= '1';
    else
      index <= "00000000";
      valid <= '0';
    end if;
  end process;


end architecture;
