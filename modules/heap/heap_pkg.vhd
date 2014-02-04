library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;

package heap_pkg is

   function f_get_parent(idxChild : unsigned) return unsigned;
   
   function f_get_l_child(idxParent : unsigned) return unsigned;
   
   function f_get_r_child(idxParent : unsigned) return unsigned;

   function f_is_lowest_level(ptr : unsigned; last : unsigned) return boolean;
   
  
   component xwb_heap is
   generic(
      g_is_ftm       : boolean := false;  
      g_idx_width    : natural := 8;
      g_key_width    : natural := 64;
      g_val_width    : natural := 192  
   );            
   port(
      clk_sys_i   : in  std_logic;
      rst_n_i     : in  std_logic;

      time_sys_i  : std_logic_vector(63 downto 0) := (others => '1');

      ctrl_i      : in  t_wishbone_slave_in;
      ctrl_o      : out t_wishbone_slave_out;
      
      snk_i       : in  t_wishbone_slave_in;
      snk_o       : out t_wishbone_slave_out;
      
      src_o       : out t_wishbone_master_out;
      src_i       : in  t_wishbone_master_in
     
   );
   end component;
  
  component heap_top is
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
    
    data_i     : in  std_logic_vector(g_key_width + g_val_width-1 downto 0);
    data_o     : out std_logic_vector(g_key_width + g_val_width-1 downto 0);
    out_o      : out std_logic
    
    );
   end component;
      
  component heap_pathfinder is
  generic(
    g_idx_width    : natural := 8;
    g_key_width    : natural := 64
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    push_i     : in  std_logic;
    pop_i      : in  std_logic;
    movkey_i   : in  std_logic_vector(g_key_width-1 downto 0);
    busy_o     : out std_logic;
    empty_o    : out std_logic; 
    full_o     : out std_logic;
    
    -- to writer core
    final_o    : out std_logic;
    idx_o      : out std_logic_vector(g_idx_width-1 downto 0);
    last_o     : out std_logic_vector(g_idx_width-1 downto 0);
    valid_o    : out std_logic;    
    
    --from writer core
    wr_key_i   : in  std_logic_vector(g_key_width-1 downto 0);     -- writes
    wr_idx_i   : in  std_logic_vector(g_idx_width-1 downto 0);
    we_i       : in  std_logic
    );
   end component;
   
  component heap_writer is
  generic(
    g_idx_width    : natural := 8;
    g_key_width    : natural := 64;
    g_val_width    : natural := 192 
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;
   
    busy_o     : out std_logic;  
   
    dbg_show_i : in std_logic;
    dbg_err_o  : out std_logic;
    dbg_ok_o   : out std_logic;
    
    data_i     : in  std_logic_vector(g_key_width + g_val_width-1 downto 0);
    data_o     : out std_logic_vector(g_key_width + g_val_width-1 downto 0);
    out_o      : out std_logic; 
    
    idx_i      : in std_logic_vector(g_idx_width-1 downto 0);
    last_i     : in std_logic_vector(g_idx_width-1 downto 0);
    final_i    : in std_logic;
    en_i       : in std_logic;
    push_i     : in std_logic;
    
    wr_key_o   : out  std_logic_vector(g_key_width-1 downto 0);     -- writes
    wr_idx_o   : out  std_logic_vector(g_idx_width-1 downto 0);
    we_o       : out  std_logic
    );
   end component;
   
end heap_pkg;

  
  package body heap_pkg is
    
   function f_get_parent(idxChild : unsigned) return unsigned is
      variable v_res : unsigned(idxChild'range);
   begin
      if(idxChild = 1) then
         v_res := idxChild;
      else
         v_res := '0' & idxChild(idxChild'left downto 1);
      end if;
      return v_res;  
   end f_get_parent;
   
   function f_get_l_child(idxParent : unsigned) return unsigned is
      variable v_res : unsigned(idxParent'range);
   begin
      v_res := (idxParent(idxParent'left-1 downto 0) & '0') +0;
      return v_res;
   end f_get_l_child;
   
   function f_get_r_child(idxParent : unsigned) return unsigned is
      variable v_res : unsigned(idxParent'range);
   begin
      v_res := (idxParent(idxParent'left-1 downto 0) & '0') +1;
      return v_res;
   end f_get_r_child;

   function f_is_lowest_level(ptr : unsigned; last : unsigned) return boolean is
      variable i : natural;
   begin
      for i in last'left downto 0 loop
         if(last(i) = '1') then
            if(ptr(i) = '1') then
               return true;
            else
               return false;
            end if;   
         end if;
      end loop;
      return false;
      
   end f_is_lowest_level;


end heap_pkg;
