library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

entity pwm_channel is

    generic (
        g_simulation                : in boolean := false;

        g_pwm_counter_width         : integer

        -- TODO: set range
        --g_pwm_counter_low           : natural := 100;
        --g_pwm_counter_high          : natural := 100
    );

    port(
    
        clk_sys_i       : in std_logic;
        rst_sys_n_i     : in std_logic;

        pwm_values      : in t_pwm_values;

        pwm_o           : out std_logic := '0'
    );

    type t_pwm_values is record
        low     : unsigned(g_pwm_counter_width-1 downto 0);
        high    : unsigned(g_pwm_counter_width-1 downto 0);
    end record t_pwm_values;

end entity pwm_channel;

architecture pwm_channel_arch of pwm_channel is

    constant g_pwm_counter_width : natural range 1 to 16 := 16;
    
    type t_pwm_values is record
        low     : unsigned(g_pwm_counter_width-1 downto 0);
        high    : unsigned(g_pwm_counter_width-1 downto 0);
    end record t_pwm_values;

    --type t_pwm_values is record
    --    low     : unsigned(g_pwm_counter_width-1 downto 0);
    --    high    : unsigned(g_pwm_counter_width-1 downto 0);
    --end record t_pwm_values;

    --signal t_this_channel : t_pwm_values := (
    --    low     => (others => '0'),
    --    high    => (others => '0')
    --    );
    
    signal s_pwm_counter : unsigned(g_pwm_counter_width-1 downto 0);
    -- PWM channel state machine
    type t_pwm_channel_state is (s_pwm_low, -- on high
                                s_pwm_high  -- on low
                                );
    signal pwm_channel_state : t_pwm_channel_state := s_pwm_low;


begin

    p_counter: process (clk_sys_i)
    begin
        if (rst_sys_n_i = '0') then
            s_pwm_counter <= (others => '0');
        elsif rising_edge(clk_sys_i) then
            if s_pwm_counter = 2**g_pwm_counter_width-1 then
                -- roll over
                s_pwm_counter <= (others => '0');
            else
                s_pwm_counter <= s_pwm_counter + 1;
                if pwm_channel_state = s_pwm_low then
                    if s_pwm_counter = t_this_channel.low then
                        s_pwm_counter   <= (others => '0');
                        pwm_o           <= '1';
                    end if;
                else
                    if s_pwm_counter = t_this_channel.high then
                        s_pwm_counter   <= (others => '0');
                        pwm_o           <= '0';
                    end if;                   
                end if;
            end if;
        end if;
    end process p_counter;

end pwm_channel_arch;
