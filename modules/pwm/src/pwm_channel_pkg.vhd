library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

package pwm_channel_pkg is

    constant g_pwm_counter_width : natural range 1 to 16 := 16;

    component pwm_channel is

        generic (
            g_simulation                : in boolean;
            g_pwm_counter_width         : natural
        );
        port(
            clk_sys_i       : in std_logic;
            rst_sys_n_i     : in std_logic;

            low             : in unsigned(g_pwm_counter_width-1 downto 0);
            high            : in unsigned(g_pwm_counter_width-1 downto 0);

            pwm_start_phase_i   : in std_logic;
            pwm_enable_i        : in std_logic;
            pwm_o               : out std_logic
        );

    end component;

end package;