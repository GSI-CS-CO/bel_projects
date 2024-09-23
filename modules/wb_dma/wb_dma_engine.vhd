library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;

entity wb_dma_engine is
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic
  );
end entity;

architecture behavioral of wb_dma_engine is

  type t_state is (IDLE, READ, WRITE, UPDATE, LD_DESC1, LD_DESC2, LD_DESC3, LD_DESC4, LD_DESC5, WB, PAUSE);
  signal s_state : t_state := IDLE;

begin

  FSM: process(clk, rstn_i) begin
    if not rstn_i then
      state <= IDLE;
    else
      case s_state is
        when IDLE =>
      
        when READ =>

        when WRITE =>
        
        when UPDATE =>

        when LD_DESC1 =>

        when LD_DESC2 =>

        when LD_DESC3 =>

        when LD_DESC4 =>

        when LD_DESC5 =>

        when WB =>

        when PAUSE =>

      end case;  
    end if;
  end process FSM;

end architecture;