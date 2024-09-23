library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;

entity wb_dma_engine is
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- read logic
    s_queue_full  : in std_logic;
    s_write_data_cache_enable : out std_logic
  );
end entity;

architecture behavioral of wb_dma_engine is

  type t_read_state is (IDLE, READ);
  signal s_read_state : t_read_state := IDLE;
  signal s_read_enable : std_logic := '0';
  
  type t_state is (IDLE, WRITE, UPDATE, LD_DESC1, LD_DESC2, LD_DESC3, LD_DESC4, LD_DESC5, WB, PAUSE);
  signal s_state : t_state := IDLE;
  signal s_write_data_cache_enable : std_logic := '0';

begin

  READ_FSM: process (clk, rstn_i)
  begin
    s_read_enable <= '0';
    s_write_data_cache_enable <= '0';
    if not rstn_i then
      s_read_state <= IDLE;
    else
      case s_read_state is
        when IDLE =>
          if ((not s_queue_full) and read_enable) s_read_state <= READ;
        when READ =>
          s_read_state <= READ;
          if (s_queue_full or (not s_read_enable)) then
            s_read_state <= IDLE;
          else
            s_read_enable <= '1';
            if (s_read_ack) s_write_data_cache_enable <= '1';
          end if;
      end case;
    end if;
    
  end process READ_FSM;

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