library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity avalon_vs is
    generic (
        g_addr_width:             natural := 4 -- bits
    );
    port (
        avs_clk:                  in  std_logic;
        avs_rst:                  in  std_logic;
        avs_ctrl_csr_addr:        in  std_logic;
        avs_ctrl_csr_rd:          in  std_logic;
        avs_ctrl_csr_readdata:    out std_logic_vector(31 downto 0);
        avs_ctrl_csr_wr:          in  std_logic;
        avs_ctrl_csr_writedata:   in std_logic_vector(31 downto 0);

        avs_sample_csr_addr:      in  std_logic_vector(g_addr_width - 1 downto 0);
        avs_sample_csr_rd:        in  std_logic;
        avs_sample_csr_readdata:  out std_logic_vector(31 downto 0);
        avs_sample_csr_wr:        in  std_logic;
        avs_sample_csr_writedata: in std_logic_vector(31 downto 0)
    );
end avalon_vs;

architecture avalon_vs_rtl of avalon_vs is

    constant c_sample_n: integer := 10;    -- 10x sample (2 of them interrupt) register

    -- registers
    type sample_reg_array is array (0 to c_sample_n - 1) of std_logic_vector(31 downto 0);
    signal s_sample_reg : sample_reg_array;
    signal s_ctrl_reg   : std_logic_vector(31 downto 0);

    -- register enable
    signal s_re         : std_logic_vector(0 to c_sample_n - 1);

begin

    address_decode: process(avs_rst, avs_sample_csr_addr)
    begin
        for i in 0 to c_sample_n - 1 loop
            if avs_rst = '1' then
                s_re(i) <= '0';
            elsif to_integer(unsigned(avs_sample_csr_addr)) = i then
                s_re(i) <= '1';
            else
                s_re(i) <= '0';
            end if;
        end loop;
    end process;

    sample_access: process(avs_clk, avs_rst)
    begin
        if avs_rst = '1' then
            s_sample_reg(0) <= x"00000011";
            s_sample_reg(1) <= x"00000022";
            s_sample_reg(2) <= x"00000033";
            s_sample_reg(3) <= x"00000044";
            s_sample_reg(4) <= x"00000055";
            s_sample_reg(5) <= x"00000066";
            s_sample_reg(6) <= x"00000077";
            s_sample_reg(7) <= x"00000088";
            s_sample_reg(8) <= x"000000aa";
            s_sample_reg(9) <= x"000000bb";
        elsif rising_edge(avs_clk) then
            avs_sample_csr_readdata <= (others => '0');
            for i in 0 to c_sample_n - 1 loop
                if s_re(i) = '1' then
                    if avs_sample_csr_rd = '1' then
                        avs_sample_csr_readdata <= s_sample_reg(i);
                    end if;
                    if avs_sample_csr_wr = '1' then
                        s_sample_reg(i) <= avs_sample_csr_writedata;
                    end if;
                end if;
            end loop;
        end if;
    end process;

    control_access: process(avs_clk, avs_rst)
    begin
        if avs_rst = '1' then
            s_ctrl_reg <= x"0000cafe";
        elsif rising_edge(avs_clk) then
            avs_ctrl_csr_readdata <= (others => '0');
            if avs_ctrl_csr_addr = '1' then
                if avs_ctrl_csr_rd = '1' then
                    avs_ctrl_csr_readdata <= s_ctrl_reg;
                end if;
                if avs_ctrl_csr_wr = '1' then
                    s_ctrl_reg <= avs_ctrl_csr_writedata;
                end if;
            end if;
        end if;
    end process;

end avalon_vs_rtl;