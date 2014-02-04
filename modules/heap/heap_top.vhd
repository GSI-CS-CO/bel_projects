
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity heap_top is
  generic(
    g_idx_width    : natural := 8;
    g_key_width    : natural := 64;
    g_val_width    : natural := 192 
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    dbg_show_i : in std_logic := '0';
    dbg_ok_o   : out std_logic;         
    dbg_err_o  : out std_logic; 

    push_i     : in std_logic;
    pop_i      : in std_logic;
    
    busy_o     : out std_logic;
    full_o     : out std_logic;
    empty_o    : out std_logic;
    count_o    : out std_logic_vector(g_idx_width-1 downto 0);
    
    data_i     : in  t_data;
    data_o     : out t_data;
    out_o      : out std_logic
    
    );
end heap_top;

architecture behavioral of heap_top is

   signal s_busy, s_busy_pf, s_busy_wr,s_push, s_pop, s_dbg : std_logic;
   
   signal s_valid,   s_final, s_we     : std_logic;
   signal s_idx,     s_widx,  s_last   : std_logic_vector(g_idx_width-1 downto 0);
   signal s_key,     s_wkey            : t_key;
   
begin
  
   s_key    <= data_i(t_skey'range);
   
   s_busy   <= s_busy_pf   or s_busy_wr;
   s_push   <= push_i      and not s_busy;
   s_pop    <= pop_i       and not s_busy;
   s_dbg    <= dbg_show_i  and not s_busy;
   count_o  <= s_last;
   busy_o   <= s_busy;  
     
   PF : heap_pathfinder
   generic map (
      g_idx_width    => g_idx_width,
      g_key_width    => g_key_width 
   )            
   port map(clk_sys_i   => clk_sys_i,
            rst_n_i     => rst_n_i,
            push_i      => s_push,
            pop_i       => s_pop,
            movkey_i    => s_key,
            busy_o      => s_busy_pf,
            empty_o     => empty_o,
            full_o      => full_o,
            
            idx_o       => s_idx,
            last_o      => s_last,
            final_o     => s_final,
            valid_o     => s_valid,
                  
            wr_key_i    => s_wkey,
            wr_idx_i    => s_widx,
            we_i        => s_we
    );
    
   WR : heap_writer
   generic map (
      g_idx_width    => g_idx_width,
      g_key_width    => g_key_width,
      g_val_width    => g_val_width 
   )            
   port map(clk_sys_i   => clk_sys_i,
            rst_n_i     => rst_n_i,

            busy_o      => s_busy_wr,
            dbg_show_i  => s_dbg,
            dbg_err_o   => dbg_err_o,
            dbg_ok_o    => dbg_ok_o,
            data_i      => data_i,
            data_o      => data_o, 
            out_o       => out_o,
              
            idx_i       => s_idx,
            last_i      => s_last,
            final_i     => s_final,
            en_i        => s_valid,
            
            push_i      => s_push,

            wr_key_o    => s_wkey,
            wr_idx_o    => s_widx,
            we_o        => s_we
    );
   
end behavioral;      
      
      
