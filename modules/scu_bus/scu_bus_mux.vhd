library ieee;
use ieee.std_logic_1164.all;

library work;

use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.scu_bus_pkg.all;

entity scu_bus_mux is
  port (
        clk                 : in std_logic;
        rst_n_i             : in std_logic;
        is_standalone       : in std_logic;
        scu_slave_o         : buffer t_wishbone_slave_out;
        scu_slave_i         : in t_wishbone_slave_in;
        ac_output           : in std_logic_vector(31 downto 0);

        scu_slave_out       : in t_wishbone_slave_out;
        scu_slave_in        : out t_wishbone_slave_in
       );
end entity;

architecture arch of scu_bus_mux is
begin

  mx: process (clk, rst_n_i, is_standalone, scu_slave_i)
  begin
    if rising_edge(clk) then
      if is_standalone = '1' then
        scu_slave_o.ack   <= '0';
        scu_slave_o.stall <= '0';
        scu_slave_o.err <= scu_slave_i.cyc and scu_slave_i.stb;
        scu_slave_o.dat <= x"deadbeef";
      else
        scu_slave_o <= scu_slave_out;
        scu_slave_in <= scu_slave_i;
      end if;
    end if;

  end process;

end architecture;
