library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.matrix_pkg.all;

package prio_pkg is

   -- input order
  constant c_a : natural := 2;
  constant c_b : natural := 1;
  constant c_c : natural := 0;
  constant c_x : natural := c_a;
  constant c_y : natural := c_b;
  constant c_z : natural := c_c;
  
  subtype slv64 is std_logic_vector(63 downto 0);
  type slv64_array is array (natural range <>) of slv64;
  
  subtype slv32 is std_logic_vector(31 downto 0);
  type slv32_array is array (natural range <>) of slv32;

  subtype slv35 is std_logic_vector(34 downto 0);
  type slv35_array is array (natural range <>) of slv35;

  component min3 is
  generic(
    g_width   : natural
  );
  port(
    clk_i     : in  std_logic;
    rst_n_i   : in  std_logic;

    a_i       : in  std_logic_vector(g_width-1 downto 0);  -- number to compare
    b_i       : in  std_logic_vector(g_width-1 downto 0);  -- ""
    c_i       : in  std_logic_vector(g_width-1 downto 0);  -- ""
    e_abc_i   : in std_logic_vector(2 downto 0); -- empty a, b, c

    min_o     : out std_logic_vector(g_width-1 downto 0); -- min 
    e_o       : out std_logic;                            -- convenience: show '1' if empty, show '0' if we have a min
    y_abc_o   : out std_logic_vector(2 downto 0)          -- min a, b, c 1-hot
  );
  end component;
  
  component min9_64 is
  port(
    clk_i     : in  std_logic;
    rst_n_i   : in  std_logic;
    
    in_i      : in slv64_array(8 downto 0); -- numbers to compare
    e_abc_i   : in std_logic_vector(8 downto 0); -- empty a, b, c, ....
    ts_o      : out std_logic_vector(63 downto 0); -- min out
    y_o       : out std_logic_vector(8 downto 0)          -- min a, b, c 1-hot
    
  );
  end component;
  
  component queue_unit is
  generic(
    g_depth     : natural := 16;
    g_words     : natural := 8
  );
  port(
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    
    slave_i     : in t_wishbone_slave_in;
    slave_o     : out t_wishbone_slave_out;
    
    master_o    : out t_wishbone_master_out;
    master_i    : in t_wishbone_master_in;
    full_o      : out std_logic;
    ts_o        : out std_logic_vector(63 downto 0);
    ts_valid_o  : out std_logic

  );
  end component;
  
  component arbiter is
  generic(
    g_depth       : natural := 16;
    g_num_masters : natural := 8 
  );
  port(
    clk_i         : in  std_logic;
    rst_n_i       : in  std_logic;

    en_i          : in std_logic;
    cnt_clear_i   : in std_logic;
    dst_adr_i     : in std_logic_vector(31 downto 0);  
  -- status and statistic
    full_o        : out std_logic_vector(g_num_masters-1 downto 0);
    cnt_ch_in_o   : out matrix(g_num_masters-1 downto 0, 31 downto 0);
    cnt_ch_out_o  : out matrix(g_num_masters-1 downto 0, 31 downto 0);
    cnt_all_out_o : out std_logic_vector(63 downto 0);
    
    ts_o          : out std_logic_vector(63 downto 0);
    ts_valid_o    : out std_logic;
  -- wb ifs
    slaves_i      : in t_wishbone_slave_in_array(g_num_masters-1 downto 0);
    slaves_o      : out t_wishbone_slave_out_array(g_num_masters-1 downto 0);
    master_o      : out t_wishbone_master_out;
    master_i      : in t_wishbone_master_in
    
  );
  end component;

  component prio is
  generic(
    g_ebm_bits    : natural := 10;
    g_depth       : natural := 16;    
    g_num_masters : natural := 8
  );
  Port(
    clk_i         : std_logic;                                          -- Clock input for sys domain
    rst_n_i       : std_logic;                                          -- Reset input (active low) for sys domain

    time_i        : std_logic_vector(64-1 downto 0);    

    ctrl_i        : in  t_wishbone_slave_in;
    ctrl_o        : out t_wishbone_slave_out;
    slaves_i      : in  t_wishbone_slave_in_array(g_num_masters-1 downto 0);
    slaves_o      : out t_wishbone_slave_out_array(g_num_masters-1 downto 0);
    master_o      : out t_wishbone_master_out;
    master_i      : in  t_wishbone_master_in
    
  );
  end component;



end prio_pkg;


