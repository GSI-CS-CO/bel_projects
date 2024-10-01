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
    s_queue_full_i              : in std_logic;
    s_queue_empty_i             : in std_logic;
    s_read_enable_o             : out std_logic := '0';
    s_data_cache_write_enable_o : out std_logic := '0';
    s_read_ack                  : in std_logic;

    --only for testing!!!!
    s_start_desc                : in std_logic;
    s_read_init_address         : in std_logic_vector(c_wishbone_address_width-1 downto 0);
    s_descriptor_active         : in std_logic
  );
end entity;

architecture behavioral of wb_dma_engine is

  -- read op contstructor signals
  signal s_read_addr    : std_logic_vector(c_wishbone_address_width-1 downto 0);

  -- read op FIFO FSM signals
  type t_read_state is (IDLE, READ);
  signal s_read_state : t_read_state := IDLE;
  
  type t_state is (IDLE, WRITE, UPDATE, LD_DESC1, LD_DESC2, LD_DESC3, LD_DESC4, LD_DESC5, WB, PAUSE);
  signal s_state : t_state := IDLE;

begin

  -- manages the fifo cache with the read ops
  p_cache_manager: process (clk_i, rstn_i)
  begin
    if rstn_i = '0' then
      s_read_state <= IDLE;
    else
      if rising_edge(clk_i) then
        s_read_enable_o <= '0';
        s_data_cache_write_enable_o <= '0';
        case s_read_state is
          when IDLE =>
            if not s_queue_empty_i = '1' then
              s_read_state <= READ; -- if there is a read op in the fifo cache start reading
            end if;
          when READ =>
            s_read_state <= READ;
            if s_queue_empty_i = '1' then
              s_read_state <= IDLE;
            else
              s_read_enable_o <= '1';
              if s_read_ack = '1' then
                s_data_cache_write_enable_o <= '1'; -- when the data is available on the bus, write it to the data cache
              end if;
            end if;
        end case;
      end if;
    end if;
  end process p_cache_manager;

end architecture;
