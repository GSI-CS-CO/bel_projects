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
    s_read_enable_o             : out std_logic;
    s_store_read_op_o           : out std_logic_vector[2*c_wishbone_address_width downto 0];
    s_data_cache_write_enable_i : out std_logic
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
  
  -- increases the memory address for the read and write operations
  p_incr_addr: process (clk_i)
  begin
    if s_start_desc then
      s_read_addr <= s_read_init_address;
    else
      s_read_addr <= std_logic_vector(unsigned(s_read_addr) + unsigned(4));
    end if;
  end process;

  p_descriptor_handler: process (clk_i)
  begin
    
  end process;
  
  -- manages the fifo cache with the read ops
  p_READ_MNGR: process (clk_i, rstn_i)
  begin
    s_read_enable_o <= '0';
    s_data_cache_write_enable_i <= '0';
    if not rstn_i then
      s_read_state <= IDLE;
    else
      case s_read_state is
        when IDLE =>
          if (not s_queue_empty_i) then
             s_read_state <= READ; -- if there is a read op in the fifo cache start reading
          end if;
        when READ =>
          s_read_state <= READ;
          if s_queue_empty_i then
            s_read_state <= IDLE;
          else
            s_read_enable_o <= '1';
            if (s_read_ack) then
              s_write_data_cache_enable <= '1'; -- when the data is available on the bus, write it to the data cache
            end if;
          end if;
      end case;
    end if;
  end process READ_MNGR;


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