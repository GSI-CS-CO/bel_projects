library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.scu_diob_pkg.all;
use work.scu_bus_slave_pkg.all;

entity user_io_reg is
  generic (
    base_addr:  unsigned(15 downto 0);
    count:      natural
  );
  port (
    clk:            std_logic;
    rstn:           std_logic;
    scu_slave_i:    in  t_scu_local_slave_i;
    scu_slave_o:    out t_scu_local_slave_o;
    
    user_in_reg:    in t_user_io_array(0 to count-1);
    user_out_reg:   out t_user_io_array(0 to count-1);
    
    user_io_wr:     out t_user_io_rw_array(0 to count-1)
  );
end entity user_io_reg;

architecture arch of user_io_reg is
  signal user_reg:  t_user_io_array(0 to count-1);
begin
  regfile: process (clk, rstn)
  begin
    if rstn = '0' then
      for i in 0 to count-1 loop
        user_reg(i) <= (others => '0'); 
      end loop;
    elsif rising_edge(clk) then
      scu_slave_o.dtack   <= '0';
      scu_slave_o.dat     <= (others => '0');
      scu_slave_o.dat_val <= '0';
      user_io_wr          <= (others => '0');
    
      if scu_slave_i.adr_val = '1' and scu_slave_i.wr_act = '1' then
        for i in 0 to count-1 loop
          if unsigned(scu_slave_i.adr) = base_addr + i then
            user_reg(i) <= scu_slave_i.dat;
            user_io_wr(i) <= '1';
            scu_slave_o.dtack <= '1';
          end if;
        end loop;
      end if;
    end if;
  
  end process regfile;
  
  
  out_reg:
  for i in 0 to count-1 generate
    user_out_reg(i) <= user_reg(i);
  end generate out_reg;

end architecture arch;