library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pwm is
  generic(
    pwm_width: integer := 16
  );
  port(
    clk_i:          in  std_logic;
    nrst_i:         in  std_logic;
    enable:         in  std_logic := '1';
    pwm_active_cnt: in  unsigned (pwm_width-1 downto 0) := to_unsigned(1, pwm_width);
    pwm_o:          out std_logic
  );
end entity;

architecture pwm_arch of pwm is

signal  cnt:  unsigned (pwm_width-1 downto 0);

begin

p_pwm: process (clk_i, nrst_i)
  begin
    if nrst_i = '0' then
      cnt <= to_unsigned(0, pwm_width);
    elsif rising_edge(clk_i) then

      if enable = '1' then
        cnt <= cnt + 1;
      end if;

      if pwm_active_cnt = 0 then
        pwm_o <= '0';
      elsif cnt <= pwm_active_cnt then
        pwm_o <= '1';
      else
        pwm_o <= '0';
      end if;

    end if;
  end process p_pwm;

end pwm_arch;
