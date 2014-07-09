-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- entity
entity generic_fifo is
  Generic (
    g_data_width : natural := 8;  -- bit(s)
    g_fifo_depth : natural := 256 -- g_data_width*bit(s)
  );
	Port ( 
    clk_sys_i    : in  std_logic;
    rst_n_i      : in  std_logic;
    write_en_i   : in  std_logic;
    data_i       : in  std_logic_vector (g_data_width-1 downto 0);
    read_en_i    : in  std_logic;
    data_o       : out std_logic_vector (g_data_width-1 downto 0);
    flag_empty_o : out std_logic;
    flag_full_o  : out std_logic;
    fill_level_o : out std_logic_vector (g_data_width-1 downto 0)
  );
end generic_fifo;

-- architecture
architecture rtl of generic_fifo is
begin

  -- fifo management process
  p_fifo_management : process(clk_sys_i, rst_n_i)
    
    -- internal signals and variables
    type t_fifo_memory is array (0 to g_fifo_depth-1) of std_logic_vector (g_data_width-1 downto 0);
    
    variable v_memory       : t_fifo_memory;
    variable v_head_pointer : natural range 0 to g_fifo_depth-1;
    variable v_tail_pointer : natural range 0 to g_fifo_depth-1;
    variable v_looped       : boolean;
    
  begin
  
    -- process reset
    if (rst_n_i = '0') then
      v_head_pointer := 0;
      v_tail_pointer := 0;
      v_looped       := false;
      flag_full_o    <= '0';
      flag_empty_o   <= '1';
      data_o         <= (others => '0');
      fill_level_o   <= (others => '0');
      
    -- process fifo management
    elsif rising_edge(clk_sys_i) then
      -- update empty and full flags
      if (v_head_pointer = v_tail_pointer) then
        if v_looped then
          flag_full_o  <= '1';
        else
          flag_empty_o <= '1';
        end if;
      else
        flag_empty_o   <= '0';
        flag_full_o	   <= '0';
      end if;
      
      -- update fifo fill level
      if (v_looped=true) then
        fill_level_o <= std_logic_vector(to_unsigned((v_tail_pointer-v_head_pointer), fill_level_o'length));
      else
        fill_level_o <= std_logic_vector(to_unsigned((v_head_pointer-v_tail_pointer), fill_level_o'length));
      end if;  
      
      -- read from fifo
      if (read_en_i = '1') then
        if ((v_looped = true) or (v_head_pointer /= v_tail_pointer)) then
        -- update data output
          data_o           <= v_memory(v_tail_pointer);
          -- update v_tail_pointer pointer as needed
          if (v_tail_pointer = g_fifo_depth-1) then
            v_tail_pointer := 0;
            v_looped       := false;
          else
            v_tail_pointer := v_tail_pointer+1;
          end if;
        end if;
      end if;
      
      -- write to fifo
      if (write_en_i = '1') then
        if ((v_looped = false) or (v_head_pointer /= v_tail_pointer)) then
          --write data to memory
          v_memory(v_head_pointer) := data_i;
          -- increment v_head_pointer pointer as needed
          if (v_head_pointer = g_fifo_depth-1) then
            v_head_pointer         := 0;
            v_looped               := true;
          else
            v_head_pointer         := v_head_pointer+1;
          end if;
        end if;
      end if;
      
    end if; -- end check reset
  end process;

end rtl;
