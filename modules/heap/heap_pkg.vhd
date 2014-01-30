library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;

package heap_pkg is

   constant c_val_len  : natural := 32;
   constant c_key_len  : natural := 8;
   constant c_data_len : natural := c_key_len + c_val_len;
   
   subtype t_val is std_logic_vector(c_val_len -1 downto 0);
   subtype t_key   is std_logic_vector(c_key_len -1 downto 0);
   subtype t_data  is std_logic_vector(c_data_len -1 downto 0);
   
   subtype t_skey  is std_logic_vector(c_data_len -1 downto c_val_len);
   subtype t_sval  is t_val;
   
   
   function f_get_parent(idxChild : unsigned) return unsigned;
   
   function f_get_l_child(idxParent : unsigned) return unsigned;
   
   function f_get_r_child(idxParent : unsigned) return unsigned;

   function f_is_lowest_level(ptr : unsigned; last : unsigned) return boolean;
   
      
  component heap_pathfinder is
  generic(
    g_idx_width    : natural := 8
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    push_i     : in  std_logic;
    pop_i      : in  std_logic;
    movkey_i   : in  t_key;
    
    -- to writer core
    final_o : out std_logic;
    idx_o      : out std_logic_vector(g_idx_width-1 downto 0);
    last_o     : out std_logic_vector(g_idx_width-1 downto 0);
    valid_o    : out std_logic;
    push_op_o  : out std_logic; 
    
    --from writer core
    wr_key_i   : in  t_key;     -- writes
    wr_idx_i   : in  std_logic_vector(g_idx_width-1 downto 0);
    we_i       : in  std_logic
    );
   end component;
   
  component heap_writer is
  generic(
    g_idx_width    : natural := 8
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    dbg_show_i : in std_logic;
    -- from LM32s
    data_i     : in  t_data;
    -- to EBM
    data_o     : out t_data;
    
    -- from pathfinder core
    idx_i      : in std_logic_vector(g_idx_width-1 downto 0);
    last_i     : in std_logic_vector(g_idx_width-1 downto 0);
    final_i    : in std_logic;
    push_op_i  : in std_logic; 
    en_i       : in std_logic;
    
    -- to pathfinder core
    wr_key_o   : out  t_key;     -- writes
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
