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
-- sensor IP core. The instantiated IP core includes only the controller core
-- and provides Avalon-MM complaint interface. This slave is responsible for
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

entity a10vs_wb is
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

    -- control or sample store selection
    constant c_ctrl_sel      : std_logic_vector(1 downto 0)  := "01";        -- control
    constant c_sample_sel    : std_logic_vector(1 downto 0)  := "10";        -- sample store

    constant c_adr_width     : integer                       := 4;           -- sample address width

    signal s_vs_sel          : std_logic_vector(1 downto 0);                 -- select (control or sample)

    -- Avalon control
    signal s_av_rd           : std_logic;                                    -- read
    signal s_av_wr           : std_logic;                                    -- write

    -- other
    signal s_adr             : std_logic_vector(c_adr_width - 1 downto 0);
    signal s_ack             : std_logic;                                    -- ack for wishbone
    signal s_re              : std_logic_vector(0 to c_vs_reg_n - 1);        -- enable for the voltage sensor registers

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

    slave_o.ack <= s_ack;

    -- address decoder for the voltage sensor registers
    s_adr <= slave_i.adr(5 downto 2);

    p_decode_latch: process(rst_n_i, s_adr)
    begin
        if (rst_n_i = '0') then
            s_re <= (others => '0');
        else
            for i in 0 to c_vs_reg_n - 1 loop
                if slave_i.stb = '1' and slave_i.cyc = '1' then
                    if to_integer(unsigned(s_adr)) = i then
                        s_re(i) <= '1';
                    else
                        s_re(i) <= '0';
                    end if;
                end if;
            end loop;
        end if;
    end process;

    -- Avalon-MM interface (voltage sensor controller)

    -- Avalon-MM address
    p_av_address_decode: process(s_re)
        variable v_sel : std_logic;
    begin
        v_sel := '0';

        for i in 0 to c_vs_reg_n - 2 loop
            v_sel := v_sel or s_re(i);
        end loop;

        s_vs_sel(0) <= s_re(c_vs_reg_n - 1);    -- ctrl
        s_vs_sel(1) <= v_sel;                   -- sample
    end process;

    vs_ctrl_csr_addr   <= s_vs_sel(0);
    vs_sample_csr_addr <= s_adr;

    -- Avalon-MM data access
    p_av_readdata: process(rst_n_i, s_vs_sel, s_ack)
    begin
        if (rst_n_i = '0') then
            slave_o.dat <= (others => '0');
        else
            case s_vs_sel is
                when c_ctrl_sel   =>
                    slave_o.dat <= vs_ctrl_csr_rddata;
                when c_sample_sel =>
                    slave_o.dat <= vs_sample_csr_rddata;
                when others       =>
                    -- none
            end case;
        end if;
    end process;

    p_av_writedata: process(s_vs_sel)
    begin
        vs_ctrl_csr_wrdata   <= (others => '0');
        vs_sample_csr_wrdata <= (others => '0');
        case s_vs_sel is
            when c_ctrl_sel   =>
                vs_ctrl_csr_wrdata   <= slave_i.dat;
            when c_sample_sel =>
                vs_sample_csr_wrdata <= slave_i.dat;
            when others =>
                -- none
        end case;
    end process;

    -- Avalon-MM control
    s_av_rd <= slave_i.cyc and slave_i.stb and not slave_i.we;
    s_av_wr <= slave_i.cyc and slave_i.stb and slave_i.we;

    p_av_rd_wr: process(s_vs_sel, s_av_rd, s_av_wr)
    begin
        vs_ctrl_csr_wr   <= '0';
        vs_ctrl_csr_rd   <= '0';
        vs_sample_csr_wr <= '0';
        vs_sample_csr_rd <= '0';

        case s_vs_sel is
            when c_ctrl_sel   =>
                vs_ctrl_csr_rd   <= s_av_rd;
                vs_ctrl_csr_wr   <= s_av_wr;
            when c_sample_sel =>
                vs_sample_csr_rd <= s_av_rd;
                if to_integer(unsigned(s_adr)) > 7 then
                    vs_sample_csr_wr <= s_av_wr;  -- allow write-access to the irq registers
                else
                    vs_sample_csr_wr <= '0';      -- disallow write-access to the sample registers
                end if;
            when others =>
                -- none
        end case;
    end process;

end a10vs_wb_rtl;