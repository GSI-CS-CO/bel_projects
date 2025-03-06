library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.pwm_channel_pkg.all;

entity pwm is

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
        -- type t_wishbone_slave_out is record
        -- ack   : std_logic;
        -- err   : std_logic;
        -- rty   : std_logic;
        -- stall : std_logic;
        -- dat   : t_wishbone_data;
        -- end record t_wishbone_slave_out;
        -- equal to t_wishbone_master_in

    t_wb_i           : in  t_wishbone_slave_in;
        --type t_wishbone_slave_in is record
        --cyc : std_logic;
        --stb : std_logic;
        --adr : t_wishbone_address;
        --sel : t_wishbone_byte_select;
        --we  : std_logic;
        --dat : t_wishbone_data;
        --end record t_wishbone_slave_in;
        -- equal to t_wishbone_master_out

    pwm_o           : out std_logic_vector((g_pwm_channel_num-1) downto 0)
    );

end entity;

architecture pwm_arch of pwm is

    signal s_ack_state        : std_logic := '0';
    signal s_led_state        : std_logic := '0';

    signal s_stall_state      : std_logic := '0';
    signal s_retry_state      : std_logic := '0';
    signal s_error_state      : std_logic := '0';

    -- TODO put into type
    --type t_wb_state is (s_wb_idle,  
    --                    s_wb_read,
    --                    s_wb_write,
    --                    );
    --signal s_wb_state : t_wb_state := s_wb_idle;

    signal s_state_machine_vector : std_logic_vector(3 downto 0) := "0000";

    -- all vectors are downto-range positions are 3210
    constant mode_write   : std_logic_vector(3 downto 0) := "1111";
    constant mode_read    : std_logic_vector(3 downto 0) := "0111";

    type t_pwm_value_array is array(0 to (g_pwm_channel_num-1)) of t_pwm_values;
    signal s_pwm_values : t_pwm_value_array := (others => (others => 1));


    signal s_current_high       : std_logic_vector(15 downto 0) := (others => '0');
    signal s_current_low        : std_logic_vector(15 downto 0) := (others => '0');

begin

    t_wb_o.ack <= s_ack_state;

    -- fill the pseudo state machine
    s_state_machine_vector(0) <= rst_sys_n_i;
    s_state_machine_vector(1) <= t_wb_i.cyc;
    s_state_machine_vector(2) <= t_wb_i.stb;
    s_state_machine_vector(3) <= t_wb_i.we;

    -- for every wanted channel generate a PWM channel
    pwm_channels: for i in 0 to g_pwm_channel_num-1 generate
    begin
        channel : pwm_channel
            -- <entity_signal_name> => <local_signal_name>
            generic map(
                g_simulation            => g_simulation,
                g_pwm_counter_width     => g_pwm_regs_size,
                g_pwm_values            => s_pwm_values(i)
            )
            port map(
                rst_sys_n_i             => rst_sys_n_i,
                clk_sys_i               => clk_sys_i,
                
                pwm_o                   => pwm_o(i)
            );
    end generate pwm_channels;

    p_wb_ack: process(clk_sys_i)
    begin
        if rising_edge(clk_sys_i) then
            if rst_sys_n_i = '0' then
                s_ack_state <= '0';
            else
                if s_ack_state = '0' and t_wb_i.stb = '1' and t_wb_i.cyc = '1' then
                    s_ack_state <= '1';
                else
                    s_ack_state <= '0';
                end if;
            end if;
        end if;
    end process p_wb_ack;

    p_wb_reg: process(clk_sys_i)
    begin
        if rising_edge(clk_sys_i) then


        end if; -- of rising_edge
    end process p_wb_reg;

    p_wb_write: process(clk_sys_i)
    begin
        if rising_edge(clk_sys_i) then
            if rst_sys_n_i = '0' then
                s_stall_state   <= '0';
                s_error_state   <= '0';
                s_retry_state   <= '0';
            elsif s_state_machine_vector = mode_write then
                -- todo reg write
            end if;
        end if;
    end process p_wb_write;

    p_wb_read: process(clk_sys_i)
    begin
        if rising_edge(clk_sys_i) then
            if rst_sys_n_i = '0' then
                t_wb_o.dat    <= (others => '0');
            elsif s_state_machine_vector = mode_read then
                t_wb_o.dat <= (others => '1');
            end if;
        end if;
    end process p_wb_read;


end pwm_arch;
