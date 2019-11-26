library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity top is
  port (
    clk_base_i    : in    std_logic;
    rst_n_i       : in    std_logic;  
    scu_cb_revision   : in  std_logic_vector(3 downto 0); -- must be assigned with weak pull ups
    fpga_con_io   : inout std_logic_vector(7 downto 0);
    led_status_o  : out   std_logic_vector(1 downto 0)
  );
end top;
--
architecture rtl of top is
  signal countx  : std_logic_vector(15 downto 0);
begin

  process(clk_base_i, rst_n_i) begin
    if (rst_n_i = '0') then
      countx <= (others => '0');
    elsif (rising_edge(clk_base_i)) then
      countx <= countx + 1;
    end if;
  end process;

  led_status_o(0) <= countx(15);
  led_status_o(1) <= countx(0);

  fpga_con_io <= (others => 'Z');

end;
