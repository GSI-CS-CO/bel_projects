--------------------------------------------------------------------------------
-- Title         : WB slave interface for the voltage sensor IP core
-- Project       : Wishbone vB.4
--------------------------------------------------------------------------------
-- File          : a10vs.vhd
-- Author        : Enkhbold Ochirsuren
-- Organisation  : GSI, TOS
-- Created       : 2025-02-14
-- Platform      : Arria 10
-- Standard      : VHDL'93
-- Repository    : https://github.com/GSI-CS-CO/bel_projects
--------------------------------------------------------------------------------
--
-- Description: Wishbone slave for interfacing the Altera/Intel Arria 10 voltage
-- sensor IP core. The instantiated IP core includes only the controller core
-- and provides Avalon-ST complaint interface. This slave is responsible for
-- controlling core and storing the sampled sensor values.
--
--------------------------------------------------------------------------------

-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- general-cores
library work;
use work.wishbone_pkg.all;

-- gsi
use work.a10vs_pkg.all;

entity a10vs is
    generic (
        g_data_size : natural := 32
    );
    port (
        -- wishbone syscon
        clk_i   : in  std_logic := '0';
        rst_n_i : in  std_logic := '1';

        -- wishbone slave interface
        slave_i : in  t_wishbone_slave_in;
        slave_o : out t_wishbone_slave_out;

        -- voltage sensor (IP core) interface (Avalon-ST)
        vs_clk             : out std_logic;                     --          clock.clk
        vs_rst             : out std_logic;                     --     reset_sink.reset
	    vs_ctrl_csr_addr   : out std_logic;                     -- controller_csr.address
	    vs_ctrl_csr_rd     : out std_logic;                     --               .read
	    vs_ctrl_csr_wr     : out std_logic;                     --               .write
	    vs_ctrl_csr_wrdata : out std_logic_vector(31 downto 0); --               .writedata
	    vs_ctrl_csr_rddata : in  std_logic_vector(31 downto 0); --               .readdata
        vs_rsp_valid       : in  std_logic;                     --       response.valid
	    vs_rsp_channel     : in  std_logic_vector(2 downto 0);  --               .channel
	    vs_rsp_data        : in  std_logic_vector(5 downto 0);  --               .data
	    vs_rsp_start_pkt   : in  std_logic;                     --               .startofpacket
	    vs_rsp_end_pkt     : in  std_logic                      --               .endofpacket

        -- voltage sensor (IP core) interface (Avalon-MM)
--        vs_clk               : out std_logic;                     -- clk
--		vs_rst               : out std_logic;                     -- reset
--		vs_ctrl_csr_addr     : out std_logic;                     -- address
--		vs_ctrl_csr_rd       : out std_logic;                     -- read
--		vs_ctrl_csr_wr       : out std_logic;                     -- write
--		vs_ctrl_csr_wrdata   : out std_logic_vector(31 downto 0); -- writedata
--		vs_ctrl_csr_rddata   : in  std_logic_vector(31 downto 0); -- readdata
--		vs_sample_csr_addr   : out std_logic_vector(3 downto 0);  -- address
--		vs_sample_csr_rd     : out std_logic;                     -- read
--		vs_sample_csr_wr     : out std_logic;                     -- write
--		vs_sample_csr_wrdata : out std_logic_vector(31 downto 0); -- writedata
--		vs_sample_csr_rddata : in  std_logic_vector(31 downto 0); -- readdata
--		vs_sample_irq        : in  std_logic                      -- irq
    );
end a10vs;

architecture a10vs_rtl of a10vs is

    constant c_sample_size       : integer                       := 6;              -- bits
    constant c_offs_vs_rw_regs   : integer                       := 8;              -- offset for the read-write registers

    -- voltage sensor register address (5 downto 2)                                 -- direction    address     component       register, bits
    constant c_vs_sample0_addr   : std_logic_vector(3 downto 0)  := "0000";         -- rd           0x00        sample store    voltage sample, 5..0
    constant c_vs_sample1_addr   : std_logic_vector(3 downto 0)  := "0001";         -- rd           0x04        sample store    voltage sample, 5..0
    constant c_vs_sample2_addr   : std_logic_vector(3 downto 0)  := "0010";         -- rd           0x08        sample store    voltage sample, 5..0
    constant c_vs_sample3_addr   : std_logic_vector(3 downto 0)  := "0011";         -- rd           0x0c        sample store    voltage sample, 5..0
    constant c_vs_sample4_addr   : std_logic_vector(3 downto 0)  := "0100";         -- rd           0x10        sample store    voltage sample, 5..0
    constant c_vs_sample5_addr   : std_logic_vector(3 downto 0)  := "0101";         -- rd           0x14        sample store    voltage sample, 5..0
    constant c_vs_sample6_addr   : std_logic_vector(3 downto 0)  := "0110";         -- rd           0x18        sample store    voltage sample, 5..0
    constant c_vs_sample7_addr   : std_logic_vector(3 downto 0)  := "0111";         -- rd           0x1c        sample store    voltage sample, 5..0
    constant c_vs_irq_en_addr    : std_logic_vector(3 downto 0)  := "1000";         -- rd/wr        0x20        sample store    interrupt enable, 0
    constant c_vs_irq_sts_addr   : std_logic_vector(3 downto 0)  := "1001";         -- rd/wr        0x24        sample store    interrupt status, 0
    constant c_vs_cmd_addr       : std_logic_vector(3 downto 0)  := "1010";         -- rd/wr        0x28        control core    command, 12..0

    -- voltage sensor
    signal s_vs_clk              : std_logic                     := '0';             --          clock.clk
    signal s_vs_rst              : std_logic                     := '0';             --     reset_sink.reset
	signal s_vs_ctrl_csr_addr    : std_logic                     := '0';             -- controller_csr.address
	signal s_vs_ctrl_csr_rd      : std_logic                     := '0';             --               .read
	signal s_vs_ctrl_csr_wr      : std_logic                     := '0';             --               .write
	signal s_vs_ctrl_csr_wrdata  : std_logic_vector(31 downto 0) := (others => '0'); --               .writedata

    signal s_vs_ctrl_csr_rddata  : std_logic_vector(31 downto 0);                    --               .readdata
    signal s_vs_rsp_valid        : std_logic;                                        --       response.valid
	signal s_vs_rsp_channel      : std_logic_vector(2 downto 0);                     --               .channel
	signal s_vs_rsp_data         : std_logic_vector(c_sample_size - 1 downto 0);     --               .data
	signal s_vs_rsp_start_pkt    : std_logic;                                        --               .startofpacket
	signal s_vs_rsp_end_pkt      : std_logic;                                        --               .endofpacket

    -- voltage sensor registers
    type vs_reg_array is array (0 to c_vs_reg_n - 1) of std_logic_vector(c_vs_reg_size - 1 downto 0);
    signal s_regs                : vs_reg_array;

    -- internal
    signal s_adr                 : std_logic_vector(3 downto 0);
    signal s_dat                 : std_logic_vector(c_vs_reg_size - 1 downto 0);
    signal s_ack                 : std_logic;
    signal s_re                  : std_logic_vector(0 to c_vs_reg_n - 1);            -- register enable, s_re(0) for s_regs(0)

begin

    -- fixed/asynchronous assignments
    slave_o.err    <= '0';               -- no abnormal cycle termination
    slave_o.rty    <= '0';               -- no cycle retry
    slave_o.stall  <= '0';               -- no pipeline

    slave_o.dat(g_data_size - 1 downto c_vs_reg_size) <= (others => '0');   -- no high word

    -- WB ack
    p_wb_ack: process(clk_i, rst_n_i)
    begin
        if rst_n_i = '0' then
            s_ack <= '0';
        else
            if rising_edge(clk_i) then
                if slave_i.cyc = '1' and slave_i.stb = '1' then
                    s_ack <= '1';
                else
                    s_ack <= '0';
                end if;
            end if;
        end if;
    end process;

    slave_o.ack <= s_ack and slave_i.stb;

    -- address decoder for the voltage sensor registers
    s_adr <= slave_i.adr(5 downto 2);

    p_decode: process(s_adr, slave_i.stb, slave_i.cyc)
    begin
        l_decode_we: for i in 0 to c_vs_reg_n - 1 loop
            if slave_i.stb = '1' and slave_i.cyc = '1' then
                if to_integer(unsigned(s_adr)) = i then
                    s_re(i) <= '1';
                else
                    s_re(i) <= '0';
                end if;
            else
                s_re(i) <= '0';
            end if;
        end loop;
    end process;

    -- WB write
    p_wb_wr: process(clk_i, rst_n_i)
    begin
        l_reg_wr: for i in 0 to c_vs_reg_n -1 loop
            if rst_n_i = '0' then
                s_regs(i) <= (others => '0');
            elsif rising_edge(clk_i) then
                --if i >= c_offs_vs_rw_regs then      -- do not allow wishbone write access for sample registers
                    if s_re(i) = '1' and slave_i.we = '1' then
                        s_regs(i) <= slave_i.dat(c_vs_reg_size - 1 downto 0);
                    end if;
                --end if;
            end if;
        end loop;
    end process;

    -- WB read
    p_rd_mux: process(s_re, s_regs)
        variable v_dat : std_logic_vector(c_vs_reg_size - 1 downto 0);
    begin
        v_dat := (others => '0');
        l_mux: for i in 0 to c_vs_reg_n - 1 loop
            v_dat := v_dat or (s_regs(i) and (s_regs(i)'range => s_re(i)));
        end loop;
        s_dat <= v_dat;
    end process;

    slave_o.dat(c_vs_reg_size - 1 downto 0) <= s_dat;

    --p_wb_rd: process(clk_i, rst_n_i)
    --begin
    --    if rst_n_i = '0' then
    --       slave_o.dat <= (others => '0');
    --    elsif rising_edge(clk_i) then
    --        slave_o.dat(c_vs_reg_size - 1 downto 0) <= s_dat;
    --    end if;
    --end process;

end a10vs_rtl;