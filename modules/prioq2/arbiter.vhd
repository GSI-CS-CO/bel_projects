library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.prio_pkg.all;
use work.matrix_pkg.all;


entity arbiter is
generic(
  g_depth         : natural := 16;
  g_num_masters   : natural := 8 
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
  
-- ts_out
  ts_o          : out std_logic_vector(63 downto 0);
  ts_valid_o    : out std_logic;
  

-- wb ifs
  slaves_i      : in t_wishbone_slave_in_array(g_num_masters-1 downto 0);
  slaves_o      : out t_wishbone_slave_out_array(g_num_masters-1 downto 0);
  master_o      : out t_wishbone_master_out;
  master_i      : in t_wishbone_master_in
  
);
begin
assert (g_num_masters >= 1 or g_num_masters <= 9) report "Timing Priority Queue must have 1-9 slave ports" severity failure;
assert (g_depth >= 8) report "Timing Priority Queue depth must be >= 8" severity failure;
end entity;

architecture behavioral of arbiter is

  -- we have N masters - 1 slave  
  constant c_num_slaves : natural := 1;
  constant c_slave : natural := 0; 


 
  subtype slave_row is std_logic_vector(g_num_masters-1 downto 0);
  type slave_matrix is array (natural range <>) of slave_row;

  signal s_masters_o    : t_wishbone_master_out_array(g_num_masters-1 downto 0);
  signal s_masters_i    : t_wishbone_master_in_array(g_num_masters-1 downto 0);
  signal s_slaves_o     : t_wishbone_slave_out_array(g_num_masters-1 downto 0);
  signal master_ie      : t_wishbone_master_in_array(c_num_slaves-1 downto 0);

  signal s_master_o     : t_wishbone_master_out;
  signal r_master_o_cyc : std_logic; 
  signal s_emptys       : std_logic_vector(8 downto 0);
  signal s_valids,      
         r_valids       : std_logic_vector(8 downto 0);
        
  signal s_ts_prio      : std_logic_vector(8 downto 0);
  signal s_ts_array     : slv64_array(8 downto 0);
  signal matrix_new,
         matrix_old,
         granted        : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);
  
  
  signal s_full         : std_logic_vector(8 downto 0);

  --statistics counters
  signal r_aux_in       : slv35_array(g_num_masters-1 downto 0);
  signal r_aux_out      : slv32_array(g_num_masters-1 downto 0);

  signal r_count_all  : std_logic_vector(63 downto 0);
  
  signal ts_debug0 : std_logic_vector(63 downto 0);
  signal ts_debug1 : std_logic_vector(63 downto 0);
  signal ts_debug2 : std_logic_vector(63 downto 0);
  signal ts_debug3 : std_logic_vector(63 downto 0);
  signal ts_debug4 : std_logic_vector(63 downto 0);
  signal ts_debug5 : std_logic_vector(63 downto 0);  
  signal ts_debug6 : std_logic_vector(63 downto 0);
  signal ts_debug7 : std_logic_vector(63 downto 0);
  signal ts_debug8 : std_logic_vector(63 downto 0);

  constant c_zero : std_logic_vector(31 downto 0) := (others => '0');


---------------------------------------------------------------------------
-- Based on Wesley Terpstra's fast Wishbone Crossbar Code, thanks 
---------------------------------------------------------------------------
  
  -- If any of the bits are '1', the whole thing is '1'
  -- This function makes the check explicitly have logarithmic depth.
  function vector_OR(x : std_logic_vector)
    return std_logic 
  is
    constant len : integer := x'length;
    constant mid : integer := len / 2;
    alias y : std_logic_vector(len-1 downto 0) is x;
  begin
    if len = 1 
    then return y(0);
    else return vector_OR(y(len-1 downto mid)) or
                vector_OR(y(mid-1 downto 0));
    end if;
  end vector_OR;
  
  
  
  function slave_matrix_OR(x : slave_matrix)
    return std_logic_vector is
    variable result : std_logic_vector(x'LENGTH-1 downto 0);
  begin
    for i in x'LENGTH-1 downto 0 loop
      result(i) := vector_OR(x(i));
    end loop;
    return result;
  end slave_matrix_OR;
  
  
  
  -- Select the master pins the slave will receive
  function slave_logic(slave   : integer;
                       adr     : t_wishbone_address;
                       granted : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);
                       slave_i : t_wishbone_slave_in_array(g_num_masters-1 downto 0))
    return t_wishbone_master_out
  is
    variable CYC_row    : slave_row;
    variable STB_row    : slave_row;
    variable ADR_matrix : slave_matrix(c_wishbone_address_width-1 downto 0);
    variable SEL_matrix : slave_matrix((c_wishbone_address_width/8)-1 downto 0);
    variable WE_row     : slave_row;
    variable DAT_matrix : slave_matrix(c_wishbone_data_width-1 downto 0);
  begin
    -- Rename all the signals ready for big_or
    for master in g_num_masters-1 downto 0 loop
      CYC_row(master) := slave_i(master).CYC and granted(master, slave);
      STB_row(master) := slave_i(master).STB and granted(master, slave);
      for bit in c_wishbone_address_width-1 downto 0 loop
        ADR_matrix(bit)(master) := slave_i(master).ADR(bit) and granted(master, slave);
      end loop;
      for bit in (c_wishbone_address_width/8)-1 downto 0 loop
        SEL_matrix(bit)(master) := slave_i(master).SEL(bit) and granted(master, slave);
      end loop;
      WE_row(master) := slave_i(master).WE and granted(master, slave);
      for bit in c_wishbone_data_width-1 downto 0 loop
        DAT_matrix(bit)(master) := slave_i(master).DAT(bit) and granted(master, slave);
      end loop;
    end loop;
    
    return (
       CYC => vector_OR(CYC_row),
       STB => vector_OR(STB_row),
       ADR => adr, --only one target allowed
       SEL => slave_matrix_OR(SEL_matrix),
       WE  => vector_OR(WE_row),
       DAT => slave_matrix_OR(DAT_matrix));
  end slave_logic;

  subtype master_row is std_logic_vector(c_num_slaves-1 downto 0);
  type master_matrix is array (natural range <>) of master_row;
  
  function master_matrix_OR(x : master_matrix)
    return std_logic_vector is
    variable result : std_logic_vector(x'LENGTH-1 downto 0);
  begin
    for i in x'LENGTH-1 downto 0 loop
      result(i) := vector_OR(x(i));
    end loop;
    return result;
  end master_matrix_OR;
  
  -- Select the slave pins the master will receive
  function master_logic(master    : integer;
                        granted   : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);
                        master_ie : t_wishbone_master_in_array(c_num_slaves-1 downto 0))
    return t_wishbone_slave_out
  is
    variable ACK_row    : master_row;
    variable ERR_row    : master_row;
    variable RTY_row    : master_row;
    variable STALL_row  : master_row;
    variable DAT_matrix : master_matrix(c_wishbone_data_width-1 downto 0);
  begin
    -- We use inverted logic on STALL so that if no slave granted => stall
    for slave in c_num_slaves-1 downto 0 loop
      ACK_row(slave) := master_ie(slave).ACK and granted(master, slave);
      ERR_row(slave) := master_ie(slave).ERR and granted(master, slave);
      RTY_row(slave) := master_ie(slave).RTY and granted(master, slave);
      STALL_row(slave) := not master_ie(slave).STALL and granted(master, slave);
      for bit in c_wishbone_data_width-1 downto 0 loop
        DAT_matrix(bit)(slave) := master_ie(slave).DAT(bit) and granted(master, slave);
      end loop;
    end loop;
    
    return (
      ACK => vector_OR(ACK_row),
      ERR => vector_OR(ERR_row),
      RTY => vector_OR(RTY_row),
      STALL => not vector_OR(STALL_row),
      DAT => master_matrix_OR(DAT_matrix),
      INT => '0');
  end master_logic;
  

  function matrix_logic(
    ts_priority : slave_row;
    en          : std_logic;
    matrix_old  : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);
    slave_i     : t_wishbone_slave_in_array(g_num_masters-1 downto 0))
    return matrix
  is
    subtype row    is std_logic_vector(g_num_masters-1 downto 0);
    subtype column is std_logic_vector(c_num_slaves-1  downto 0);
    
    variable tmp_column : column;
    variable tmp_row    : row;
    
    variable selected   : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);  -- Which master wins arbitration  log(M) request
    variable sbusy      : column;  -- Does the slave's  previous connection persist?
    variable mbusy      : row;     -- Does the master's previous connection persist?
    variable matrix_new : matrix(g_num_masters-1 downto 0, c_num_slaves-1 downto 0);
  begin
    -- The slave is busy if ... it's busy or if the previously sending channel did not have time to reload yet
      for master in g_num_masters-1 downto 0 loop
        tmp_row(master) := matrix_old(master, c_slave) and slave_i(master).CYC;
      end loop;
      sbusy(c_slave) := vector_OR(tmp_row);
    
    -- A master is busy iff it services an in-progress cycle
    for master in g_num_masters-1 downto 0 loop
      tmp_column(c_slave) := matrix_old(master, c_slave);
      mbusy(master) := vector_OR(tmp_column) and slave_i(master).CYC;
    end loop;

    -- Decode the selection and grant priority - if arbiter is enabled
    for master in g_num_masters-1 downto 0 loop
      selected(master, c_slave) := slave_i(master).CYC and slave_i(master).STB and ts_priority(master) and en;
    end loop;
     
    -- Determine the master granted access
    -- Policy: if cycle still in progress, preserve the previous choice
    for master in g_num_masters-1 downto 0 loop
      if sbusy(c_slave) = '1' or mbusy(master) = '1' then
        matrix_new(master, c_slave) := matrix_old(master, c_slave);
      else
        matrix_new(master, c_slave) := selected(master, c_slave);
      end if;
    end loop;

    
    return matrix_new;
  end matrix_logic;
  --------------------------------------------------------

begin

  ---------------------------------------------------------------------------
-- Generate some statistics 
---------------------------------------------------------------------------    

Count : for i in g_num_masters-1 downto 0 generate   

  cnt_ch_out : process(clk_i)
    variable v_inc : unsigned(31 downto 0);
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' or cnt_clear_i = '1' then
        r_valids(i) <= '0';
        r_aux_out(i) <= (others => '0');
      else
        r_valids(i) <= s_valids(i);        
        v_inc := (others => '0');
        v_inc(0) := s_valids(i) and not r_valids(i);
        r_aux_out(i) <= std_logic_vector(unsigned(r_aux_out(i)) + v_inc); 
      end if;
    end if;
  end process cnt_ch_out;

  -- fucking awkward, but best I can do. An assignment procedure would create drivers for all bits at once - can't have that 
  bitsO : for j in 31 downto 0 generate 
    cnt_ch_out_o(i, j)   <= r_aux_out(i)(j);
  end generate;
  
  cnt_ch_in : process(clk_i)
    variable v_inc : unsigned(34 downto 0);
  begin
    if rising_edge(clk_i) then
      v_inc := (others => '0');
      v_inc(0) := slaves_i(i).cyc and slaves_i(i).stb and not s_slaves_o(i).stall;

      
      if rst_n_i = '0' or cnt_clear_i = '1' then
        r_aux_in(i) <= (others => '0');
      else
        r_aux_in(i) <= std_logic_vector(unsigned(r_aux_in(i)) + v_inc); 
      end if;
      
    end if;
  end process cnt_ch_in;


  -- again: fuckin awkward and ugly...
  bitsI : for j in 31+3 downto 3 generate
          -- each telegram has 8 words. Divide by 8 - right shift by 3b 
    cnt_ch_in_o(i, j-3)   <= r_aux_in(i)(j);
  end generate;




end generate;


  cnt_all_out : process(clk_i)
    variable v_inc : unsigned(63 downto 0);
  begin
    if rising_edge(clk_i) then
      v_inc := (others => '0');
      v_inc(0) := s_master_o.cyc and not r_master_o_cyc;
      if rst_n_i = '0' or cnt_clear_i = '1' then
        r_count_all   <= (others => '0');
      else
        r_count_all <= std_logic_vector(unsigned(r_count_all) + v_inc);
        ts_valid_o  <= v_inc(0);
      end if;
      r_master_o_cyc <= s_master_o.cyc;
    end if;
  end process cnt_all_out;


  cnt_all_out_o <= r_count_all;
---------------------------------------------------------------------------


  --the sorting tree
  sorter :  min9_64
  port map(
    clk_i       => clk_i,
    rst_n_i     => rst_n_i,
    in_i        => s_ts_array,
    e_abc_i     => s_emptys,
    ts_o        => ts_o,
    y_o         => s_ts_prio
  );

  s_emptys <= not s_valids;

  --the input queues
  queues : for master in 0 to g_num_masters-1 generate 
    qu : queue_unit 
    generic map(
      g_depth => g_depth,
      g_words => 8
    )
    port map(
      clk_i       => clk_i,
      rst_n_i     => rst_n_i,
      master_o    => s_masters_o(master),
      master_i    => s_masters_i(master),
      slave_i     => slaves_i(master),
      slave_o     => s_slaves_o(master),
      full_o      => full_o(master),
      ts_o        => s_ts_array(master),
      ts_valid_o  => s_valids(master)
    );
  end generate;

  slaves_o <= s_slaves_o;

  nc : if g_num_masters < 9 generate
    s_valids(8 downto g_num_masters)    <= (others => '0');
    s_ts_array(8 downto g_num_masters)  <= (others => (others => '0'));
  end generate;

  -- Copy the matrix to a register:
  matrix_new <= matrix_logic(s_ts_prio(g_num_masters-1 downto 0), en_i, matrix_old, s_masters_o);
  main : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        matrix_old <= (others => (others => '0'));
      else
        matrix_old <= matrix_new;
      end if;
    end if;
  end process main;
  
  -- Make the "crossbar" registered
  granted <= matrix_old;
  
  master_o <= s_master_o;
  --- Master Muxing ---  
  -- n - 1 master out
  s_master_o         <= slave_logic(c_slave, dst_adr_i, granted, s_masters_o);
  -- 1 - n master in
  master_ie(c_slave) <= master_i;
  master_ins : for master in g_num_masters-1 downto 0 generate
    s_masters_i(master) <= master_logic(master, granted, master_ie);
  end generate;  
 
  ts_debug0 <= s_ts_array(0);
  ts_debug1 <= s_ts_array(1);
  ts_debug2 <= s_ts_array(2);
  ts_debug3 <= s_ts_array(3);
  ts_debug4 <= s_ts_array(4);
  ts_debug5 <= s_ts_array(5);
  ts_debug6 <= s_ts_array(6);
  ts_debug7 <= s_ts_array(7);
  ts_debug8 <= s_ts_array(8);





end architecture;
