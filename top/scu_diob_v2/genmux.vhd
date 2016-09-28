library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_diob_pkg.all;
use work.scu_bus_slave_pkg.all;

entity genmux is
  generic (
    width :     natural
  );
  port (
    din:      in   t_scu_slave_o_array := (others => scu_dummy_slave_o);
    dout:     out  std_logic_vector(15 downto 0) := (others => 'X');
    dout_val: out  std_logic := '0';
    dtack:    out  std_logic := '0'
  );
end genmux;

--architecture tri of genmux is
--begin
--  mux: for i in din'range generate
--    dout <= din(i).dat when din(i).dat_val ='1' else (others => 'Z');
--  end generate mux;
--  dout_val <= '1' when unsigned(sel) > 0 else '0';
--end tri;

architecture logic of genmux is
begin
  process (din)
    variable tmp :    std_logic_vector(15 downto 0);
    variable tmp_val: std_logic;
    variable tmp_ack: std_logic;
  begin
    tmp := (others => '0'); tmp_val := '0'; tmp_ack := '0';
    for i in din'range loop
      if din(i).dat_val = '1' then
        tmp := tmp or (din(i).dat and x"ffff");
        tmp_val := tmp_val or '1';
      else
        tmp := tmp or (din(i).dat and x"0000");
        tmp_val := tmp_val or '0';      
      end if;
      if din(i).dtack = '1' then
        tmp_ack := tmp_ack or '1';
      else
        tmp_ack := tmp_ack or '0';
      end if;
    end loop;
    dout      <= tmp;
    dout_val  <= tmp_val;
    dtack     <= tmp_ack;
  end process;
  
end logic;