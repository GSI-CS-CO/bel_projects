library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.prio_pkg.all;
use work.matrix_pkg.all;


entity min3 is
generic(
  g_width   : natural := 64
);
port(
  clk_i     : in  std_logic;
  rst_n_i   : in  std_logic;
  
  a_i       : in  std_logic_vector(g_width-1 downto 0)  := (others => '0'); -- number to compare
  b_i       : in  std_logic_vector(g_width-1 downto 0)  := (others => '0'); -- ""
  c_i       : in  std_logic_vector(g_width-1 downto 0)  := (others => '0'); -- ""
  e_abc_i   : in std_logic_vector(2 downto 0)           := (others => '1'); -- empty a, b, c
  
  
  min_o     : out std_logic_vector(g_width-1 downto 0); -- min 
  e_o       : out std_logic;                            -- convenience: show '1' if empty, show '0' if we have a min
  y_abc_o   : out std_logic_vector(2 downto 0)          -- min a, b, c 1-hot
  
);
begin
assert (g_width >= 1 and g_width <= 64) report "Comparator must be 1-64 Bits wide" severity failure;
end min3;

architecture behavioral of min3 is

  signal s_a, s_b, s_c : std_logic_vector(g_width downto 0); -- underflow + empty flag + N bit TS
  signal s_min : std_logic_vector(g_width-1 downto 0); -- N bit TS
  signal s_a_le_b, s_b_le_c, s_a_le_c, s_e : std_logic; --comparator outputs : a less or equal b, ...
  signal s_y_abc  : std_logic_vector(2 downto 0);
  
  constant c_xdummy : std_logic_vector(g_width downto 0) := (others => '1');
   --arrange comparator outputs as matrix, get 1-hot from column ANDs
  function min_one_hot( a_le_b, b_le_c, a_le_c : std_logic)
      return std_logic_vector
  is
    variable res : std_logic_vector(c_a downto c_c);
    variable m : matrix(c_a downto c_c, c_a downto c_c);
    variable x, y : natural;
  begin
    m := mrst(m);
    --fill in comparator values
    m(c_a, c_b) := a_le_b;
    m(c_b, c_c) := b_le_c;
    m(c_a, c_c) := a_le_c;
    
    --fill in inverted values and '1' diagonal
    for x in c_a downto c_c loop
      for y in c_a downto x loop
        m(x, y) :=  not m(y, x);
      end loop;
    end loop;
    
    -- get the column (a, b, c) ANDs
    for x in c_a downto c_c loop
      res(x) := '1';
      for y in c_a downto c_c loop
        res(x) := res(x) and m(x, y);
      end loop;

    end loop;
     
    return res; 
  end min_one_hot;

begin

  --comparator outputs : A_Less or Equal_B, B_Less...
  s_a_le_b <= '1' when unsigned(s_a) <= unsigned(s_b) else '0';
  s_b_le_c <= '1' when unsigned(s_b) <= unsigned(s_c) else '0';
  s_a_le_c <= '1' when unsigned(s_a) <= unsigned(s_c) else '0';
  
--FIXME this X stuff is dangerous
  s_a <= e_abc_I(c_a) & a_i when a_i(0) /= 'X' else c_xdummy; -- if input is empty, MSB is goes high so it can not be the minimum
  s_b <= e_abc_I(c_b) & b_i when b_i(0) /= 'X' else c_xdummy; -- 
  s_c <= e_abc_I(c_c) & c_i when c_i(0) /= 'X' else c_xdummy; -- 
  
  s_e <= e_abc_i(c_a) and e_abc_i(c_b) and e_abc_i(c_c); -- show if all inputs are empty
  
  s_y_abc  <= min_one_hot(s_a_le_b, s_b_le_c, s_a_le_c); -- find minimum and show as one-hot
  
  MUX : with s_y_abc select
  s_min <=  a_i when "100",
            b_i when "010",
            c_i when "001",
            a_i when others;
  
--register output
cmp : process(clk_i)
begin
  if(rising_edge(clk_i)) then
    if(rst_n_i = '0') then
      y_abc_o <= "000"; 
      e_o     <= '1'; 
    else
      e_o     <= s_e; 
      y_abc_o <= s_y_abc; 
      min_o   <= s_min;
    end if;
  end if;

end process cmp;

    
end behavioral;

