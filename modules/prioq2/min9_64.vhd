library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.prio_pkg.all;

entity min9_64 is
port(
  clk_i     : in  std_logic;
  rst_n_i   : in  std_logic;
  
  in_i      : in slv64_array(8 downto 0)      ; -- numbers to compare
  e_abc_i   : in std_logic_vector(8 downto 0) ; -- empty a, b, c, ....
  ts_o      : out std_logic_vector(63 downto 0); -- min out
  y_o       : out std_logic_vector(8 downto 0)          -- min a, b, c 1-hot
  
);
end min9_64;

architecture behavioral of min9_64 is

  signal s_y_top, 
         s_e_xyz  : std_logic_vector(2 downto 0);
  signal s_y      : std_logic_vector(8 downto 0);
  signal s_xyz    : slv64_array(2 downto 0); 
  signal s_e_top  : std_logic;
  
begin  

  G1: for I in c_x downto c_z generate
    inst : min3
    generic map ( g_width => 64)
    port map(
      clk_i   => clk_i,
      rst_n_i => rst_n_i,
      a_i     => in_i(I*3+c_a),
      b_i     => in_i(I*3+c_b),
      c_i     => in_i(I*3+c_c),
      e_abc_i => e_abc_i(I*3+c_a downto I*3+c_c),
      min_o   => s_xyz(I),
      e_o     => s_e_xyz(I),
      y_abc_o => s_y(I*3+c_a downto I*3+c_c)
    );
    
    --and the 1-hot outputs of the first layer with the 1-hot mask of the second layer and global empty
    --    eee eee eee
    --    xxx yyy zzz 
    -- &  abc def hij
    G2: for J in c_a downto c_c generate
      y_o(I*3+J) <= s_y(I*3+J) and not s_e_top and s_y_top(I);
    end generate;
  end generate;
    
  top : min3
  generic map ( g_width => 64)
  port map(
    clk_i   => clk_i,
    rst_n_i => rst_n_i,
    a_i     => s_xyz(c_x),
    b_i     => s_xyz(c_y),
    c_i     => s_xyz(c_z),
    e_abc_i => s_e_xyz,
    min_o   => ts_o,
    e_o     => s_e_top,
    y_abc_o => s_y_top
  );
  
  
    
end behavioral;
