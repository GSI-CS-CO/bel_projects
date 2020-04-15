library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;

entity wb_minislave is
    port(
        clk_i   : in  std_logic;
        rst_n_i : in  std_logic;
        slave_i : in  t_wishbone_slave_in;
        slave_o : out t_wishbone_slave_out;
        msi_master_o : out t_wishbone_master_out;
        msi_master_i : in  t_wishbone_master_in
    );
end entity;

architecture rtl of wb_minislave is
    signal wb_register : std_logic_vector(31 downto 0) := x"00000100";
    signal stall : std_logic := '1';
    signal n_rd_access : integer := 0;
    signal n_wr_access : integer := 0;
    signal ack : std_logic := '0';
    signal countdown : unsigned(15 downto 0) := x"0800";
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


    msi_master_o.cyc <= '1' when countdown < 3 or countdown > x"07ff" else '0';
    msi_master_o.sel <= (others => '1');
    msi_master_o.we  <= '1';
    msi_emitter: process
    begin
        wait until rising_edge(clk_i);
        msi_master_o.stb <= '0';
        if n_wr_access > 0 and slave_i.stb = '0' and slave_i.we = '0' then 
            if countdown >= 3 then 
                countdown <= countdown -1 ;
            elsif countdown < 3 then
                msi_master_o.dat <= x"0000" & std_logic_vector(countdown);
                msi_master_o.adr <= x"000000aa";
                msi_master_o.stb <= '1';
                if msi_master_i.stall = '0' then
                    if countdown > 0 then 
                        countdown <= countdown - 1;
                    else 
                        msi_master_o.stb <= '0';
                        countdown <= x"0800";
                    end if;
                end if;
            end if;
        end if;
    end process;

end architecture;
