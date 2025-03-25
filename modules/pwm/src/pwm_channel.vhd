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
    
        clk_sys_i           : in std_logic;
        rst_sys_n_i         : in std_logic;

        low                 : in unsigned(g_pwm_counter_width-1 downto 0);
        high                : in unsigned(g_pwm_counter_width-1 downto 0);

        pwm_start_phase_i   : in std_logic  := '1'; -- `1` for high

        pwm_enable_i        : in std_logic  := '0';
        pwm_o               : out std_logic := '0'
    );
    
end entity pwm_channel;

architecture pwm_channel_arch of pwm_channel is
    
    signal s_pwm_counter : unsigned(g_pwm_counter_width-1 downto 0);

    -- PWM channel state machine
    type t_pwm_channel_state is (   s_pwm_idle,
                                    s_pwm_running
                                );
    -- PWM channel phase
    type t_pwm_channel_phase is (   s_pwm_low,
                                    s_pwm_high
                                );

    signal pwm_channel_state    : t_pwm_channel_state := s_pwm_idle;
    signal pwm_channel_phase    : t_pwm_channel_phase := s_pwm_low;

    -- intermediate output signal
    signal s_pwm_o  : std_logic := '0';

begin

    p_counter: process (clk_sys_i)
    begin
        if (rst_sys_n_i = '0') then
            s_pwm_counter <= (others => '0');
        elsif rising_edge(clk_sys_i) then

            if pwm_channel_state = s_pwm_idle then
            
                if pwm_enable_i = '1' then

                    pwm_channel_state <= s_pwm_running;

                    if pwm_start_phase_i = '0' then 
                        pwm_channel_phase <= s_pwm_low;
                        s_pwm_o           <= '0';
                    else
                        pwm_channel_phase <= s_pwm_high;
                        s_pwm_o           <= '1';
                    end if;

                    s_pwm_counter  <= (others => '0');

                else

                    pwm_channel_state <= s_pwm_idle;
                    s_pwm_counter <= (others => '0');
                    
                end if;

            else

                s_pwm_counter <= s_pwm_counter + 1;

                if pwm_channel_phase = s_pwm_low then
                    if s_pwm_counter >= low  then
                        s_pwm_counter       <= (others => '0');
                        s_pwm_o             <= '1';
                        pwm_channel_phase   <= s_pwm_high;
                    end if;

                else
                    if s_pwm_counter >= high then
                        s_pwm_counter       <= (others => '0');
                        s_pwm_o             <= '0';
                        pwm_channel_phase   <= s_pwm_low;
                    end if;

                end if;

            end if; --if pwm_channel_state = s_pwm_idle

        end if; -- if (rst_sys_n_i = '0')
    end process p_counter;

    pwm_o <= s_pwm_o;

end pwm_channel_arch;
