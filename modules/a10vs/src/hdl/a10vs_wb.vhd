--------------------------------------------------------------------------------
-- Title         : WB slave interface for the voltage sensor IP core
-- Project       : Wishbone vB.4
--------------------------------------------------------------------------------
-- File          : a10vs_wb.vhd
-- Author        : Enkhbold Ochirsuren
-- Organisation  : GSI, TOS
-- Created       : 2025-02-14
-- Platform      : Arria 10
-- Standard      : VHDL'93
-- Repository    : https://github.com/GSI-CS-CO/bel_projects
--------------------------------------------------------------------------------
--
-- Description: Wishbone slave for interfacing the Altera/Intel Arria 10 voltage
-- sensor IP core. This slave module is responsible for accessing to the
-- instantiated control and sample store cores via the Avalon-MM complaint
-- interface.
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

entity a10vs_wb is
    port (
        -- wishbone syscon
        clk_i   : in  std_logic := '0';
        rst_n_i : in  std_logic := '1';

        -- wishbone slave interface
        slave_i : in  t_wishbone_slave_in;
        slave_o : out t_wishbone_slave_out;

        -- voltage sensor (IP core) interface (Avalon-MM)
        vs_ctrl_csr_addr     : out std_logic;                     -- address
        vs_ctrl_csr_rd       : out std_logic;                     -- read
        vs_ctrl_csr_wr       : out std_logic;                     -- write
        vs_ctrl_csr_wrdata   : out std_logic_vector(31 downto 0); -- writedata
        vs_ctrl_csr_rddata   : in  std_logic_vector(31 downto 0); -- readdata
        vs_sample_csr_addr   : out std_logic_vector(3 downto 0);  -- address
        vs_sample_csr_rd     : out std_logic;                     -- read
        vs_sample_csr_wr     : out std_logic;                     -- write
        vs_sample_csr_wrdata : out std_logic_vector(31 downto 0); -- writedata
        vs_sample_csr_rddata : in  std_logic_vector(31 downto 0); -- readdata
        vs_sample_irq        : in  std_logic                      -- irq
    );
end a10vs_wb;

architecture a10vs_wb_rtl of a10vs_wb is

    constant c_adr_width     : integer                       := 4;           -- sample address width

    -- Avalon control
    signal s_av_rd           : std_logic;                                    -- read
    signal s_av_wr           : std_logic;                                    -- write

    -- other
    signal s_adr             : std_logic_vector(c_adr_width - 1 downto 0);
    signal s_ack             : std_logic;                                    -- ack for wishbone
    signal s_ack2            : std_logic;                                    -- ack delayed by 2 cycles (data from Avalon-MM is valid at 4th cycle)

begin

    -- WB fixed/asynchronous assignments
    slave_o.err    <= '0';               -- no abnormal cycle termination
    slave_o.rty    <= '0';               -- no cycle retry
    slave_o.stall  <= '0';               -- no pipeline

    -- WB ack
    p_wb_ack: process(clk_i, rst_n_i)
    begin
        if rst_n_i = '0' then
            s_ack <= '0';
            s_ack2 <= '0';
            slave_o.ack <= '0';
        else
            if rising_edge(clk_i) then
                if slave_i.cyc = '1' and slave_i.stb = '1' then
                    s_ack <= '1';
                else
                    s_ack <= '0';
                end if;
                s_ack2 <= s_ack;
                slave_o.ack <= s_ack2;
            end if;
        end if;
    end process;

    -- Avalon-MM interface (voltage sensor controller)

    -- Avalon-MM address decoder
    s_adr <= slave_i.adr(5 downto 2);
    vs_sample_csr_addr <= s_adr;

    p_ctrl_addr_decode: process(s_adr)
    begin
        case s_adr is
            when x"A" =>
                vs_ctrl_csr_addr <= '1';
            when others =>
                vs_ctrl_csr_addr <= '0';
        end case;
    end process;

    -- Avalon-MM data bus
    p_av_readdata: process(rst_n_i, clk_i, s_adr)
        variable v_adr_int : integer;
    begin
        if (rst_n_i = '0') then
            slave_o.dat <= (others => '0');
        else
            if rising_edge(clk_i) then
                v_adr_int := to_integer(unsigned(s_adr));

                case v_adr_int is
                    when 10   =>
                        slave_o.dat <= vs_ctrl_csr_rddata;
                    when 0 to 9 =>
                        slave_o.dat <= vs_sample_csr_rddata;
                    when others       =>
                        slave_o.dat <= (others => '0');
                end case;
            end if;
        end if;
    end process;

    p_av_writedata: process(clk_i, s_adr)
    begin
        if rising_edge(clk_i) then
            case s_adr is
                when x"A"   =>
                    vs_ctrl_csr_wrdata   <= slave_i.dat;
                when x"9" | x"8" =>
                    vs_sample_csr_wrdata <= slave_i.dat;
                when others =>
                    vs_ctrl_csr_wrdata   <= (others => '0');
                    vs_sample_csr_wrdata <= (others => '0');
            end case;
        end if;
    end process;

    -- Avalon-MM control bus
    s_av_rd <= slave_i.cyc and not slave_i.we;
    s_av_wr <= slave_i.cyc and slave_i.we;

    p_av_rd_wr: process(s_av_rd, s_av_wr, s_adr)
        variable v_adr_int : integer;
    begin
        v_adr_int := to_integer(unsigned(s_adr));

        case v_adr_int is
            when 10     =>
                vs_ctrl_csr_rd   <= s_av_rd;
                vs_ctrl_csr_wr   <= s_av_wr;
            when 8 to 9 =>
                vs_sample_csr_rd <= s_av_rd;
                vs_sample_csr_wr <= s_av_wr;  -- allow write-access to the irq registers
            when 0 to 7 =>
                vs_sample_csr_rd <= s_av_rd;
                vs_sample_csr_wr <= '0';      -- disallow write-access to the sample registers
            when others =>
                vs_ctrl_csr_wr   <= '0';
                vs_ctrl_csr_rd   <= '0';
                vs_sample_csr_wr <= '0';
                vs_sample_csr_rd <= '0';
        end case;
    end process;

end a10vs_wb_rtl;