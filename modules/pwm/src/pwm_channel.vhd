library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

entity pwm_channel is

    generic (
        g_simulation                : in boolean := false;

        g_pwm_counter_width         : natural
    );

    port(
    
        clk_sys_i       : in std_logic;
        rst_sys_n_i     : in std_logic;


        low             : in unsigned(g_pwm_counter_width-1 downto 0);
        high            : in unsigned(g_pwm_counter_width-1 downto 0);

        pwm_o           : out std_logic := '0'
    );
    
end entity pwm_channel;

architecture pwm_channel_arch of pwm_channel is
    
    signal s_pwm_counter : unsigned(g_pwm_counter_width-1 downto 0);

    -- PWM channel state machine
    type t_pwm_channel_state is (s_pwm_low, -- on high
                                s_pwm_high  -- on low
                                );
    signal pwm_channel_state : t_pwm_channel_state := s_pwm_low;

    -- intermediate output signal
    signal s_pwm_o  : std_logic := '0';
    signal s_enable : boolean := false;


begin

    p_counter: process (clk_sys_i)
    begin
        if (rst_sys_n_i = '0') then
            s_pwm_counter <= (others => '0');
            s_enable      <= false;
        elsif rising_edge(clk_sys_i) then

            s_enable <= high > 0;
            
            if s_enable then 
                s_pwm_counter <= s_pwm_counter + 1;

                if pwm_channel_state = s_pwm_low then
                    if s_pwm_counter >= low  then
                        s_pwm_counter       <= (others => '0');
                        s_pwm_o             <= '1';
                        pwm_channel_state   <= s_pwm_high;
                    end if;

                else
                    if s_pwm_counter >= high then
                        s_pwm_counter   <= (others => '0');
                        s_pwm_o         <= '0';
                        pwm_channel_state   <= s_pwm_low;
                    end if;                   
                end if;
            end if;
        end if;
    end process p_counter;

    pwm_o <= s_pwm_o;

end pwm_channel_arch;
