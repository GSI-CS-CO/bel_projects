library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

package pwm_channel_pkg is

    constant g_pwm_counter_width : natural range 1 to 16 := 16;

    type t_pwm_values is record
        low     : unsigned(g_pwm_counter_width-1 downto 0);
        high    : unsigned(g_pwm_counter_width-1 downto 0);
    end record t_pwm_values;

    component pwm_channel is

        generic (
            g_simulation                : in boolean;
            g_pwm_counter_width         : natural
        );
        port(
            clk_sys_i       : in std_logic;
            rst_sys_n_i     : in std_logic;

            pwm_values      : in t_pwm_values;

            pwm_o           : out std_logic
        );

    end component;

end package;