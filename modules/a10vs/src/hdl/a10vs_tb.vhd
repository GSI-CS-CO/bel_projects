--------------------------------------------------------------------------------
-- Title         : Testbench for the WB slave
-- Project       : Wishbone vB.4
--------------------------------------------------------------------------------
-- File          : a10vs_tb.vhd
-- Author        : Enkhbold Ochirsuren
-- Organisation  : GSI, TOS
-- Created       : 2025-02-17
-- Platform      : Arria 10
-- Standard      : VHDL'93
-- Repository    : https://github.com/GSI-CS-CO/bel_projects
--------------------------------------------------------------------------------
--
-- Description: Testbench for the Wishbone slave that interfaces with the
-- Altera/Intel Arria 10 voltage sensor IP core.
-- Credits to the i2c_wrapper module.
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;
use work.a10vs_pkg.all;
-- avalon
use work.avalon_vs_pkg.all;

entity a10vs_tb is
end a10vs_tb;

architecture a10vs_tb_rtl of a10vs_tb is

    -- Testbench settings
    constant c_reset_time   : time    := 200 ns;
    constant c_clock_cycle  : time    := 16 ns;

    -- Other constants
    constant c_reg_all_zero : std_logic_vector(31 downto 0) := x"00000000";
    constant c_cyc_on       : std_logic := '1';
    constant c_cyc_off      : std_logic := '0';
    constant c_str_on       : std_logic := '1';
    constant c_str_off      : std_logic := '0';
    constant c_we_on        : std_logic := '1';
    constant c_we_off       : std_logic := '0';

    -- voltage sensor registers
    constant c_vs_sample0_addr   : std_logic_vector(31 downto 0)  := x"00000000";
    constant c_vs_sample1_addr   : std_logic_vector(31 downto 0)  := x"00000004";
    constant c_vs_sample2_addr   : std_logic_vector(31 downto 0)  := x"00000008";
    constant c_vs_sample3_addr   : std_logic_vector(31 downto 0)  := x"0000000c";
    constant c_vs_sample4_addr   : std_logic_vector(31 downto 0)  := x"00000010";
    constant c_vs_sample5_addr   : std_logic_vector(31 downto 0)  := x"00000014";
    constant c_vs_sample6_addr   : std_logic_vector(31 downto 0)  := x"00000018";
    constant c_vs_sample7_addr   : std_logic_vector(31 downto 0)  := x"0000001c";
    constant c_vs_cmd_addr       : std_logic_vector(31 downto 0)  := x"00000028";
    constant c_vs_invalid_addr   : std_logic_vector(31 downto 0)  := x"0000002c";

    signal s_clk       : std_logic := '0';
    signal s_rst_n     : std_logic := '0';
    signal s_rst       : std_logic := '1';

    signal s_slave_in  : t_wishbone_slave_in;
    signal s_slave_out : t_wishbone_slave_out;

    -- avalon
    signal s_avs_ctrl_csr_addr        : std_logic;
    signal s_avs_ctrl_csr_rd          : std_logic;
    signal s_avs_ctrl_csr_readdata    : std_logic_vector(31 downto 0);
    signal s_avs_ctrl_csr_wr          : std_logic;
    signal s_avs_ctrl_csr_writedata   : std_logic_vector(31 downto 0);

    signal s_avs_sample_csr_addr      : std_logic_vector(3 downto 0);
    signal s_avs_sample_csr_rd        : std_logic;
    signal s_avs_sample_csr_readdata  : std_logic_vector(31 downto 0);
    signal s_avs_sample_csr_wr        : std_logic;
    signal s_avs_sample_csr_writedata : std_logic_vector(31 downto 0);

    -- Functions
    -- Function wb_stim -> Helper function to create a human-readable testbench
    function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic;
                adr : t_wishbone_address; dat : t_wishbone_data) return t_wishbone_slave_in is
        variable v_setup : t_wishbone_slave_in;
    begin
        v_setup.cyc := cyc;
        v_setup.stb := stb;
        v_setup.we  := we;
        v_setup.adr := adr;
        v_setup.dat := dat;
        v_setup.sel := (others => '0'); -- Don't care
        return v_setup;
    end function wb_stim;

    -- Procedures
    -- Procedure wb_expect -> Check WB slave answer
    procedure wb_expect(msg : string; dat_from_slave : t_wishbone_data;
                compare_value : t_wishbone_data) is
    begin
        if (to_integer(unsigned(dat_from_slave)) = to_integer(unsigned(compare_value))) then
            report "Test passed: " & msg;
        else
            report "Test errored: " & msg;
            report "-> Info:  Answer from slave:          " & integer'image(to_integer(unsigned(dat_from_slave)));
            report "-> Error: Expected answer from slave: " & integer'image(to_integer(unsigned(compare_value)));
        end if;
    end procedure wb_expect;

begin

    -- Clock generator
    p_clock : process
    begin
        s_clk <= '0';
        wait for c_clock_cycle/2;
        s_clk <= '1' and s_rst_n;
        wait for c_clock_cycle/2;
    end process;

    -- Reset controller
    p_reset : process
    begin
        wait for c_reset_time;
        s_rst_n <= '1';
    end process;

    s_rst <= not s_rst_n;

    -- a10vs_wb instance
    a10vs_dut: a10vs_wb
        port map (
            -- wishbone syscon
            clk_i   => s_clk,
            rst_n_i => s_rst_n,

            -- wishbone slave interface
            slave_i => s_slave_in,
            slave_o => s_slave_out,

            -- voltage sensor IP core interface (Avalon-MM)
            vs_ctrl_csr_addr     => s_avs_ctrl_csr_addr,
            vs_ctrl_csr_rd       => s_avs_ctrl_csr_rd,
            vs_ctrl_csr_wr       => s_avs_ctrl_csr_wr,
            vs_ctrl_csr_wrdata   => s_avs_ctrl_csr_writedata,
            vs_ctrl_csr_rddata   => s_avs_ctrl_csr_readdata,
            vs_sample_csr_addr   => s_avs_sample_csr_addr,
            vs_sample_csr_rd     => s_avs_sample_csr_rd,
            vs_sample_csr_wr     => s_avs_sample_csr_wr,
            vs_sample_csr_wrdata => s_avs_sample_csr_writedata,
            vs_sample_csr_rddata => s_avs_sample_csr_readdata,
            vs_sample_irq        => '0'
        );

    -- Avalon_vs instance
    voltage_sensor_0: avalon_vs
        generic map (
            g_addr_width => 4
        )
        port map (
            avs_clk                  => s_clk,
            avs_rst                  => s_rst,
            avs_ctrl_csr_addr        => s_avs_ctrl_csr_addr,
            avs_ctrl_csr_rd          => s_avs_ctrl_csr_rd,
            avs_ctrl_csr_readdata    => s_avs_ctrl_csr_readdata,
            avs_ctrl_csr_wr          => s_avs_ctrl_csr_wr,
            avs_ctrl_csr_writedata   => s_avs_ctrl_csr_writedata,
            avs_sample_csr_addr      => s_avs_sample_csr_addr,
            avs_sample_csr_rd        => s_avs_sample_csr_rd,
            avs_sample_csr_readdata  => s_avs_sample_csr_readdata,
            avs_sample_csr_wr        => s_avs_sample_csr_wr,
            avs_sample_csr_writedata => s_avs_sample_csr_writedata
        );

    -- Wishbone controller
    p_wishbone_stim : process
    begin
        -- Reset
        s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
        wait until rising_edge(s_rst_n);
        wait until rising_edge(s_clk);

        -- Check if core is disabled
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); wb_expect("Core disabled", s_slave_out.dat, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_on,  c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); wb_expect("Core disabled", s_slave_out.dat, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); wb_expect("Core disabled", s_slave_out.dat, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Write 0x1 to the sample register 0 (dissalowed access)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_vs_sample0_addr, x"00000001");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wb_expect("Write 0x1 to sample reg 0", c_reg_all_zero, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read from the sample register 0 (0x11 is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wb_expect("Read from sample reg 0: 0x11", s_slave_out.dat, x"00000011");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Write 0x8 to the sample register 7 (dissalowed access)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_vs_sample7_addr, x"00000008");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wb_expect("Write 0x8 to sample reg 7", c_reg_all_zero, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read from the sample register 7 (0x88 is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wb_expect("Read from sample reg 7: 0x88", s_slave_out.dat, x"00000088");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Idle
        wait until rising_edge(s_clk);
        wait until rising_edge(s_clk);

        -- Write 0xff to the command register
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_vs_cmd_addr, x"000000ff");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wb_expect("Write 0xff to command reg", c_reg_all_zero, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read from the command register (0xff is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wb_expect("Read from command reg: 0xff", s_slave_out.dat, x"000000ff");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);

        wait until rising_edge(s_clk);

        -- Read from the sample register 0 (0x11 is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wb_expect("Read from sample reg 0: 0x11", s_slave_out.dat, x"00000011");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample0_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read from the sample register 7 (0x88 is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wb_expect("Read from sample reg 7: 0x88", s_slave_out.dat, x"00000088");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample7_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read access from an invalid address
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_invalid_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_invalid_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_invalid_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_invalid_addr, c_reg_all_zero);
        wb_expect("Read from invalid address", s_slave_out.dat, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_invalid_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read access from the sample register 6 (0x77 is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_sample6_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample6_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample6_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_sample6_addr, c_reg_all_zero);
        wb_expect("Read from sample reg 6: 0x77", s_slave_out.dat, x"00000077");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_sample6_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Read access from the command register (0xff is expected)
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wb_expect("Read from command reg: 0xff", s_slave_out.dat, x"000000ff");
        wait until rising_edge(s_clk); s_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_vs_cmd_addr, c_reg_all_zero);
        wait until rising_edge(s_clk);

        -- Idle
        wait until rising_edge(s_clk);
        wait until rising_edge(s_clk);

        -- Finish simulation
        wait for c_clock_cycle*10000;
        -- Using STOP_TIME from settings.sh here
        --report "Simulation done!" severity failure;
    end process;

end a10vs_tb_rtl;