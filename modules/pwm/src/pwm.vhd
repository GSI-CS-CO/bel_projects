library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.spwm_wbgen2_pkg.all; 

entity pwm is

    -- This is just a wrapper for the PWM module from general_cores

    generic (
        g_simulation                : in boolean := false;
        g_pwm_channel_num           : integer range 1 to 8 := 8;
        g_pwm_regs_size             : integer range 1 to 16 := 16;
        
        g_pwm_default_period        : integer range 1 to 16 := 16;
        g_pwm_default_presc         : integer range 0 to 255 := 0;
        g_pwm_default_val           : integer range 0 to 255 := 0;

        g_pwm_interface_mode        : t_wishbone_interface_mode := PIPELINED;
        g_pwm_address_granularity   : t_wishbone_address_granularity := BYTE
    );

    port(
    
    -- these two sys signals come from SysCon
    s_clk_sys_i       : in std_logic;
    s_rst_sys_n_i     : in std_logic;


    t_wb_out          : out t_wishbone_slave_out;
        -- type t_wishbone_slave_out is record
        -- ack   : std_logic;
        -- err   : std_logic;
        -- rty   : std_logic;
        -- stall : std_logic;
        -- dat   : t_wishbone_data;
        -- end record t_wishbone_slave_out;
        -- equal to t_wishbone_master_in

    t_wb_in            : in  t_wishbone_slave_in;
        --type t_wishbone_slave_in is record
        --cyc : std_logic;
        --stb : std_logic;
        --adr : t_wishbone_address;
        --sel : t_wishbone_byte_select;
        --we  : std_logic;
        --dat : t_wishbone_data;
        --end record t_wishbone_slave_in;
        -- equal to t_wishbone_master_out

    s_pwm_o           : out std_logic_vector(g_pwm_channel_num-1 downto 0)
    -- start with only one channel
    );

end entity;

architecture pwm_arch of pwm is

    signal s_tb_pwm_o : std_logic;

    --signal t_adr_temp_i : t_wishbone_address;

begin

    --t_adr_temp_i <= "0" & t_wb_in.adr(7 downto 2);

    PWM_WB : wb_simple_pwm
    --component wb_simple_pwm
    --    generic (
    --      g_num_channels        : integer range 1 to 8;
    --      g_regs_size           : integer range 1 to 16 := 16;
    --      g_default_period      : integer range 0 to 255 := 0;
    --      g_default_presc       : integer range 0 to 255 := 0;
    --      g_default_val         : integer range 0 to 255 := 0;
    --      g_interface_mode      : t_wishbone_interface_mode      := PIPELINED;
    --      g_address_granularity : t_wishbone_address_granularity := BYTE);
    --    port (
    --      clk_sys_i  : in  std_logic;
    --      rst_n_i    : in  std_logic;
    --      wb_adr_i   : in  std_logic_vector(5 downto 0);
    --      wb_dat_i   : in  std_logic_vector(31 downto 0);
    --      wb_dat_o   : out std_logic_vector(31 downto 0);
    --      wb_cyc_i   : in  std_logic;
    --      wb_sel_i   : in  std_logic_vector(3 downto 0);
    --      wb_stb_i   : in  std_logic;
    --      wb_we_i    : in  std_logic;
    --      wb_ack_o   : out std_logic;
    --      wb_stall_o : out std_logic;
    --      pwm_o      : out std_logic_vector(g_num_channels-1 downto 0));
    --end component;
    generic map(
        g_num_channels        =>  g_pwm_channel_num,
        g_regs_size           =>  g_pwm_regs_size,
        g_default_period      =>  g_pwm_default_period,
        g_default_presc       =>  g_pwm_default_presc,
        g_default_val         =>  g_pwm_default_val,
        g_interface_mode      =>  g_pwm_interface_mode,
        g_address_granularity =>  g_pwm_address_granularity
    )
    port map (
        rst_n_i     =>  s_rst_sys_n_i,
        clk_sys_i   =>  s_clk_sys_i,

        -- as defined in the general_cores module:
        -- wb_simple_pwm only takes the lower 6 bits
        -- inner wb_adr_i takes only 4 bits of these
        -- and we need to stay aligned
        wb_adr_i(5) => t_wb_in.adr(5),
        wb_adr_i(4) => t_wb_in.adr(4),
        wb_adr_i(3) => t_wb_in.adr(3),
        wb_adr_i(2) => t_wb_in.adr(2),
        wb_adr_i(1) => '0',
        wb_adr_i(0) => '0',
        wb_dat_i    => t_wb_in.dat,    
        wb_dat_o    => t_wb_out.dat,
        wb_cyc_i    => t_wb_in.cyc,
        wb_sel_i    => t_wb_in.sel,
        wb_stb_i    => t_wb_in.stb,
        wb_we_i     => t_wb_in.we,
        wb_ack_o    => t_wb_out.ack,
        wb_stall_o  => t_wb_out.stall,

        pwm_o       =>  s_pwm_o

    );

end pwm_arch;
