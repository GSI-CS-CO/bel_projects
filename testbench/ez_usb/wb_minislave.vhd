library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;

entity wb_minislave is
    port(
        clk_i   : in  std_logic;
        rst_n_i : in  std_logic;
        slave_i : in  t_wishbone_slave_in;
        slave_o : out t_wishbone_slave_out
    );
end entity;

architecture rtl of wb_minislave is
    signal wb_register : std_logic_vector(31 downto 0) := x"00000100";
    signal stall : std_logic := '1';
    signal n_rd_access : integer := 0;
    signal n_wr_access : integer := 0;
    signal ack : std_logic := '0';
begin

    minislave: process
    begin
        wait until rising_edge(clk_i);
        slave_o.ack <= slave_i.stb and ack;
        slave_o.err <= '0';
        slave_o.rty <= '0';
        stall <= '1';
        if slave_i.cyc = '1' 
        then
            ack <= slave_i.stb;
            stall <= not slave_i.stb;
        end if;
        if slave_i.stb = '1' and stall = '0' 
        then
            if slave_i.we = '1' 
            then 
                n_wr_access <= n_wr_access + 1;
                wb_register <= slave_i.dat;

            else
                n_rd_access <= n_rd_access + 1;
            end if;
        end if;
    end process;
    slave_o.dat <= wb_register when ack = '1' else "--------------------------------";
    slave_o.stall <= stall;


end architecture;
