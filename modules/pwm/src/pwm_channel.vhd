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
        pwm_mode_i          : in std_logic  := '1'; -- `1` for free-running
        pwm_latch_i         : in std_logic  := '0'; -- latch input
        pwm_o               : out std_logic := '0'
    );
    
end entity pwm_channel;

architecture pwm_channel_arch of pwm_channel is
    
    signal s_pwm_counter : unsigned(g_pwm_counter_width-1 downto 0);

    -- PWM channel state machine
    type t_pwm_channel_state is (   pwm_idle,
                                    pwm_set,
                                    pwm_waiting,
                                    pwm_running
                                );
    -- PWM channel phase
    type t_pwm_channel_phase is (   phase_low,
                                    phase_high
                                );

    -- PWM channel mode
    type t_pwm_channel_mode  is (   pwm_latched,
                                    pwm_free
                                );

    signal pwm_channel_state    : t_pwm_channel_state   := pwm_idle;
    signal pwm_channel_phase    : t_pwm_channel_phase   := phase_low;
    signal pwm_channel_mode     : t_pwm_channel_mode    := pwm_free;

    -- intermediate output signal
    signal s_pwm_o  : std_logic := '0';

    -- remember last trigger status for latched mode
    signal v_last_latch         : std_logic := '0';

begin

    p_counter: process (clk_sys_i)

    begin
        if (rst_sys_n_i = '0') then
            s_pwm_counter <= (others => '0');
        elsif rising_edge(clk_sys_i) then

            if pwm_mode_i = '0' then 
                pwm_channel_mode <= pwm_latched;
            else
                pwm_channel_mode <= pwm_free;
            end if;

            --if pwm_start_phase_i = '0' then 
            --    pwm_channel_phase <= phase_low;
            --    else
            --    pwm_channel_phase <= phase_high;
            --end if;

            case( pwm_channel_state ) is
            
                when pwm_idle =>

                    if ((low > 0) or (high > 0)) then

                        s_pwm_counter  <= (others => '0');

                        if pwm_channel_mode = pwm_free 
                        then
                            if pwm_channel_phase = phase_low then
                                s_pwm_o           <= '0';
                            else
                                s_pwm_o           <= '1';
                            end if;
                            pwm_channel_state <= pwm_running;
                        else
                            pwm_channel_state <= pwm_waiting;
                        end if;

                    end if;
    
                when pwm_waiting =>

                    if ((pwm_latch_i = '1') and (v_last_latch = '0')) then 

                        pwm_channel_state <= pwm_running;

                    end if;

                when pwm_running =>

                    s_pwm_counter <= s_pwm_counter + 1;

                    if ((low = 0) or (high = 0)) then

                        pwm_channel_state <= pwm_idle;
                        s_pwm_o           <= '0';

                    else

                        if pwm_channel_phase = phase_low then
                            s_pwm_o           <= '0';
                            if s_pwm_counter >= low  then
                                s_pwm_counter       <= (others => '0');
                                s_pwm_o             <= '1';
                                pwm_channel_phase   <= phase_high;
                            end if;

                        else
                            s_pwm_o           <= '1';
                            if s_pwm_counter >= high then
                                s_pwm_counter       <= (others => '0');
                                s_pwm_o             <= '0';
                                pwm_channel_phase   <= phase_low;
                            end if;
                        end if;
                    end if;
                        
                when others =>

                    pwm_channel_state <= pwm_running;
                    s_pwm_counter  <= (others => '0');

            end case ;

            v_last_latch <= pwm_latch_i;

        end if; -- if (rst_sys_n_i = '0')

    end process p_counter;

    pwm_o <= s_pwm_o;

end pwm_channel_arch;
