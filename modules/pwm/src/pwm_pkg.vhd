library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package pwm_pkg is

    component pwm_channel is

        generic (
            g_simulation                : in boolean := false;

            g_pwm_channel_num           : natural range 1 to 32 := 8;
            g_pwm_regs_size             : natural range 1 to 16 := 16;

            g_pwm_interface_mode        : t_wishbone_interface_mode := PIPELINED;
            g_pwm_address_granularity   : t_wishbone_address_granularity := BYTE
        );
        port(

            clk_sys_i       : in std_logic;
            rst_sys_n_i     : in std_logic;


            t_wb_o           : out t_wishbone_slave_out;

            t_wb_i           : in  t_wishbone_slave_in;

            pwm_o           : out std_logic_vector((g_pwm_channel_num-1) downto 0)
        );

    end component;

    type t_pwm_values is record
        low     : unsigned(g_pwm_regs_size-1 downto 0);
        high    : unsigned(g_pwm_regs_size-1 downto 0);
    end record t_pwm_values;

end package;