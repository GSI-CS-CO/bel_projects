library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.dm_diag_auto_pkg.c_dm_diag_ctrl_sdb;

package ftm_pkg is


------------------------------------------------------------------------------
-- Components declaration
-------------------------------------------------------------------------------
  

  function f_bool2int(x : boolean) return integer;
  function f_xwb_dpram_aux(g_size : natural; product : t_sdb_product) return t_sdb_device;
  function f_xwb_dpram_userlm32(g_size : natural) return t_sdb_device;
  function f_xwb_dpram_shared(g_size : natural) return t_sdb_device;

  function f_substr(s : string; idx : natural; p : character) return string;  
  function f_substr_start(s : string; idx : natural; p : character) return integer;
  function f_substr_end(s : string; idx : natural; p : character) return integer; 
  
  function f_lm32_slaves_req(g_size : natural; g_world_bridge_sdb : t_sdb_bridge; is_dm : boolean) return t_sdb_record_array; 
  function f_lm32_masters_req return t_sdb_record_array; 

  function f_lm32_masters_bridge_msis(cores : natural) return t_sdb_record_array;
  function f_cluster_sdb(cores : natural; ramPerCore  : natural;  is_dm : boolean ) return t_sdb_record_array;
  function f_cluster_bridge(msi_slave : t_sdb_msi; cores : natural; ramPerCore  : natural;  is_dm : boolean ) return t_sdb_bridge;

  constant c_static_cluster_slaves : natural := 3;                        
  --sadly, we can't push generics into packages. declare in ftm_lm32_cluster.vhd
  --constant c_clu_slaves     : natural := c_static_cluster_slaves + g_cores; -- info rom, prioq ctrl, diag, rams
  constant c_clu_masters    : natural := 1;

  --sdb
  constant c_clu_info_rom   : natural := 0;
  constant c_clu_pq_ctrl    : natural := 1;
  constant c_clu_diag       : natural := 2;                    
  constant c_msi_slave      : natural := 0;


  constant c_dummy_bridge : t_sdb_bridge := (
  sdb_child    => (others => '0'),
  sdb_component => (
  addr_first   => (others => '0'),
  addr_last    => (others => '0'),
  product => (
  vendor_id    => (others => '0'), 
  device_id    => (others => '0'),
  version     => (others => '0'),
  date       => (others => '0'),
  name       => "DUMMYDUMMYDUMMYDUMM")));
  
  constant c_dummy_slave_in : t_wishbone_slave_in := ('0', '0', x"00000000", x"F", '0', x"00000000"); 
  constant c_dummy_slave_out : t_wishbone_slave_out := ('0', '0', '0', '0', '0', x"00000000");
  
  

  component ftm_lm32 is
  generic(
    g_cpu_id              : t_wishbone_data := x"CAFEBABE";
    g_size                : natural := 65536;                 -- size of the dpram
    g_world_bridge_sdb    : t_sdb_bridge;                     -- record for superior bridge
    g_is_dm               : boolean := false;  
    g_profile             : string  := "medium_icache_debug"; -- lm32 profile
    g_init_file           : string);                    -- number of msi queues connected to the lm32
  port(
    clk_sys_i       : in  std_logic;  -- system clock 
    rst_n_i         : in  std_logic;  -- reset, active low 
    rst_lm32_n_i    : in  std_logic;  -- reset, active low
    tm_tai8ns_i     : in std_logic_vector(63 downto 0) := (others => '0');

    stall_diag_o   : out std_logic;
    cycle_diag_o   : out std_logic;
      
    -- wb world interface of the lm32
    world_master_o  : out t_wishbone_master_out; 
    world_master_i  : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000");

    -- optional wb interface to prioq for DM
    prioq_master_o  : out t_wishbone_master_out; 
    prioq_master_i  : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000");
    
    -- msi interface
    msi_slave_o     : out t_wishbone_slave_out;  
    msi_slave_i     : in  t_wishbone_slave_in;
    -- port B of the LM32s DPRAM 
    ram_slave_o     : out t_wishbone_slave_out;                           
    ram_slave_i     : in  t_wishbone_slave_in
  );
  end component;
  
  component ftm_lm32_cluster is
  generic(
    g_is_dm         : boolean := false;
    g_cores         : natural := 1;
    g_ram_per_core  : natural := 32768/4;
    g_profiles      : string  := "medium_icache_debug";
    g_init_files    : string;   
    g_world_bridge_sdb : t_sdb_bridge;   -- inferior sdb crossbar         
    g_clu_msi_sdb      : t_sdb_msi       -- superior msi crossbar          
  );
  port(
    clk_ref_i      : in  std_logic;
    rst_ref_n_i    : in  std_logic;

    clk_sys_i      : in  std_logic;
    rst_sys_n_i    : in  std_logic;
    rst_lm32_n_i   : in  std_logic_vector(g_cores-1 downto 0); 

    tm_tai8ns_i    : in std_logic_vector(63 downto 0);

    -- lm32 core interfaces       
    lm32_masters_o   : out t_wishbone_master_out_array(g_cores-1 downto 0); 
    lm32_masters_i   : in  t_wishbone_master_in_array(g_cores-1 downto 0);
    lm32_msi_slaves_o  : out t_wishbone_slave_out_array(g_cores-1 downto 0); 
    lm32_msi_slaves_i  : in  t_wishbone_slave_in_array(g_cores-1 downto 0);  

    -- cluster crossbar interface
    clu_slave_o  : out t_wishbone_slave_out; 
    clu_slave_i  : in  t_wishbone_slave_in := ('0', '0', x"00000000", x"F", '0', x"00000000");
    clu_msi_master_o : out t_wishbone_master_out;
    clu_msi_master_i : in t_wishbone_master_in;

    -- optional prioq interface
    dm_prioq_master_o : out t_wishbone_master_out; 
    dm_prioq_master_i : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000") 
  );
  end component;
  
  component time_clk_cross is
  generic (g_delay_comp    : natural := 16);
  port   (clk_ref_i        : in std_logic;
        rst_ref_n_i       : in std_logic;
        clk_sys_i        : in std_logic;        
        rst_sys_n_i       : in std_logic;         
          
        tm_time_valid_i    : in  std_logic;                -- timestamp valid flag
        tm_tai_i         : in  std_logic_vector(39 downto 0);  -- TAI Timestamp
        tm_cycles_i       : in  std_logic_vector(27 downto 0);  -- refclock cycle count
      
        tm_ref_tai_cycles_o  : out std_logic_vector(63 downto 0);
        tm_sys_tai_cycles_o  : out std_logic_vector(63 downto 0)      
  );
  end component;
  
  component ftm_priority_queue is
  generic(
    g_is_ftm     : boolean := false;  
    g_idx_width   : natural := 8;
    g_key_width   : natural := 64;
    g_val_width   : natural := 192  
  );        
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    time_sys_i  : in  std_logic_vector(63 downto 0) := (others => '1');

    ctrl_i    : in  t_wishbone_slave_in;
    ctrl_o    : out t_wishbone_slave_out;
    
    snk_i     : in  t_wishbone_slave_in;
    snk_o     : out t_wishbone_slave_out;
    
    src_o     : out t_wishbone_master_out;
    src_i     : in  t_wishbone_master_in
    
  );
  end component;

  component dm_diag is
  generic(
    g_cores : natural := 16 --CPU cores
  );
  Port(
    clk_ref_i                     : std_logic;                            -- Clock input for ref domain
    rst_ref_n_i                   : std_logic;                            -- Reset input (active low) for ref domain
    tm_tai8ns_i                   : std_logic_vector(63 downto 0) := (others => '0');
    cyc_diag_i                    : std_logic_vector(g_cores-1 downto 0);
    stall_diag_i                  : std_logic_vector(g_cores-1 downto 0);
    
    ctrl_i                        : in  t_wishbone_slave_in;
    ctrl_o                        : out t_wishbone_slave_out
  );
  end component;
  
   -- crossbar layout
  constant c_lm32_slaves          : natural := 7;
  constant c_lm32_masters         : natural := 2;

  --indices  
  constant c_lm32_ram             : natural := 0;
  constant c_lm32_msi_ctrl        : natural := 1;
  constant c_lm32_cpu_info        : natural := 2;
  constant c_lm32_sys_time        : natural := 3;
  constant c_lm32_atomic          : natural := 4;
  constant c_lm32_prioq           : natural := 5;
  constant c_lm32_world_bridge    : natural := 6;

  constant c_msi_lm32_real        : natural := 0; -- lm32 is no native MSI device, we have to hide its 2nd Master port
  constant c_msi_lm32_fake        : natural := 1;  

  constant c_ram_offset           : std_logic_vector(31 downto 0) := x"10000000"; -- not 0x0 or automap will interfere

  constant c_atomic_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000000000003",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040100",
    version       => x"00000001",
    date          => x"20131119",
    name          => "ATOMIC CYCLINE CTRL")));
  
  constant c_sys_time_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000000000007",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040084",
    version       => x"00000001",
    date          => x"20131009",
    name          => "TAI-Time-8ns       ")));
  
  constant c_cpu_info_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040085",
    version       => x"00000001",
    date          => x"20131009",
    name          => "CPU-Info-ROM       ")));

  constant c_pq_data_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000003f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040201",
    version       => x"00000001",
    date          => x"20131009",
    name          => "DM-Prio-Queue-Data ")));
  
  constant c_pq_ctrl_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000007f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040200",
    version       => x"00000001",
    date          => x"20131009",
    name          => "DM-Prio-Queue-Ctrl ")));

  constant c_cluster_info_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000001F",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040086",
    version       => x"00000001",
    date          => x"20131009",
    name          => "Cluster-Info-ROM   ")));
  
  constant c_cluster_cb_product : t_sdb_product := (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10041000",
    version       => x"00000001",
    date          => x"20140515",
    name          => "LM32-CB-Cluster    ");
  
  constant c_msi_lm32_sdb : t_sdb_msi := (
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"1f1a4e39",
    version       => x"00000001",
    date          => x"20160425",
    name          => "LM32-MSI-Tgt       ")));

  constant c_userlm32_irq_ep_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device        
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10050083",
    version       => x"00000001",
    date          => x"20150128",
    name          => "LM32-MSI-Tgt       ")));

end ftm_pkg;

package body ftm_pkg is

  function f_bool2int(x : boolean) return integer is
    variable res : integer;
  begin
    if(x) then
      res := 1;
    else
      res := 0;
    end if;
    return res;
  end f_bool2int; 

  function f_xwb_dpram_userlm32(g_size : natural) return t_sdb_device is
    variable product : t_sdb_product;
  begin
    product.vendor_id := x"0000000000000651";  -- GSI
    product.device_id := x"54111351";
    product.version   := x"00000001";
    product.date      := x"20140128";
    product.name      := "LM32-RAM-User      ";
    return f_xwb_dpram_aux(g_size, product);
  end f_xwb_dpram_userlm32;
  
  function f_xwb_dpram_shared(g_size : natural) return t_sdb_device is
    variable product : t_sdb_product;
  begin
    product.vendor_id := x"0000000000000651";  -- GSI
    product.device_id := x"81111444";
    product.version   := x"00000001";
    product.date      := x"20140128";
    product.name      := "LM32-RAM-Shared    ";
    return f_xwb_dpram_aux(g_size, product);
  end f_xwb_dpram_shared;
  
  function f_xwb_dpram_aux(g_size : natural; product : t_sdb_product) return t_sdb_device is
    variable result : t_sdb_device;
  begin
    result.abi_class     := x"0001";   -- RAM device
    result.abi_ver_major := x"01";
    result.abi_ver_minor := x"00";
    result.wbd_width     := x"7";     -- 32/16/8-bit supported
    result.wbd_endian    := c_sdb_endian_big;

    result.sdb_component.addr_first := (others => '0');
    result.sdb_component.addr_last  := std_logic_vector(to_unsigned(g_size*4-1, 64));

    result.sdb_component.product := product; 

    return result;
  end f_xwb_dpram_aux;

  function f_substr(s : string; idx : natural; p : character) return string is
    constant startp : integer := f_substr_start(s, idx, p);
    constant endp : integer := f_substr_end(s, idx, p);
    variable result : string(1 to endp - startp +1);
    variable i : natural;
  begin
    --report "Calling substr function" severity warning;
    if(result = "" or startp=endp) then
      report "CPU" & integer'image(idx) & " has no init file!" severity error;
      return "";
    end if;  
    for i in startp to endp loop
      result(i - startp +1) := s(i);
    end loop;  
    report "CPU" & integer'image(idx) & " InitFile: " & result severity note;    
    return result;
  end f_substr;  
  
  function f_substr_start(s : string; idx : natural; p : character) return integer is
    variable i, j : integer;
  begin
    if(idx = 0) then
      return 1;
    end if;
      
    j := 0;
    for i in s'range loop
      if(j = idx) then
        return i;
      end if;
      if s(i) = p then
        j := j +1;
      end if;  
    end loop;
    return s'high;
  end f_substr_start;
  
  function f_substr_end(s : string; idx : natural; p : character) return integer is
    variable i, j : integer;
  begin
    j := 0;
    for i in s'range loop
      if s(i) = p then
        if(j = idx) then
          return i-1;
        end if; 
        j := j +1;
      end if;  
    end loop;
    return s'high;
  end f_substr_end;   
  

  

  function f_cluster_sdb(cores : natural; ramPerCore  : natural;  is_dm : boolean )
  return t_sdb_record_array is
    variable v_clu_req :  t_sdb_record_array((c_static_cluster_slaves + cores) -1 downto 0);
    variable i : natural;
  begin
    -- add info rom, diagnostics, prioq ctrl, rams 
    v_clu_req(c_clu_info_rom) := f_sdb_auto_device(c_cluster_info_sdb,        true);
    v_clu_req(c_clu_pq_ctrl)  := f_sdb_auto_device(c_pq_ctrl_sdb,             is_dm);
    v_clu_req(c_clu_diag)     := f_sdb_auto_device(c_dm_diag_ctrl_sdb,        true);
    for i in c_static_cluster_slaves to v_clu_req'length-1 loop
      v_clu_req(i) := f_sdb_auto_device( f_xwb_dpram_userlm32(ramPerCore), true);
    end loop;

    return v_clu_req;
  end f_cluster_sdb;

  function f_lm32_masters_req
  return t_sdb_record_array is
    variable v_req :  t_sdb_record_array(1 downto 0);
  begin
    v_req := (c_msi_lm32_real => f_sdb_auto_msi(c_msi_lm32_sdb,           true),
              c_msi_lm32_fake => f_sdb_auto_msi(c_null_msi,               false));
    return v_req;
  end f_lm32_masters_req;

  function f_lm32_slaves_req(g_size : natural; g_world_bridge_sdb : t_sdb_bridge; is_dm : boolean)
  return t_sdb_record_array is
    variable v_req :  t_sdb_record_array(c_lm32_slaves-1 downto 0);
  begin
    v_req := (c_lm32_ram                => f_sdb_embed_device(f_xwb_dpram_userlm32(g_size),   c_ram_offset), -- this CPU's RAM
              c_lm32_msi_ctrl           => f_sdb_auto_device(c_irq_slave_ctrl_sdb,  true),
              c_lm32_cpu_info           => f_sdb_auto_device(c_cpu_info_sdb,        true),
              c_lm32_sys_time           => f_sdb_auto_device(c_sys_time_sdb,        true),
              c_lm32_atomic             => f_sdb_auto_device(c_atomic_sdb,          true),
              c_lm32_prioq              => f_sdb_auto_device(c_pq_data_sdb,         is_dm),             
              c_lm32_world_bridge       => f_sdb_embed_bridge(g_world_bridge_sdb,   x"80000000")
            );
    return v_req;
  end f_lm32_slaves_req;

  function f_lm32_masters_bridge_msis(cores : natural)
  return t_sdb_record_array is
    constant c_req         : t_sdb_record_array(1 downto 0) := f_lm32_masters_req;
    constant c_layout     : t_sdb_record_array := f_sdb_auto_layout(c_req);
    constant c_bridge_msi : t_sdb_msi          := f_xwb_msi_layout_sdb(c_layout);
    constant c_result : t_sdb_record_array(cores-1 downto 0) := 
       (others => f_sdb_auto_msi(c_bridge_msi, true));
  begin
    return c_result;
  end f_lm32_masters_bridge_msis;


  function f_cluster_bridge(msi_slave : t_sdb_msi; cores : natural; ramPerCore  : natural;  is_dm : boolean )
  return t_sdb_bridge is 
    variable v_ret      :  t_sdb_bridge;
    variable v_clu_req_slaves  :  t_sdb_record_array(c_static_cluster_slaves + cores-1 downto 0);
	  variable v_clu_req_masters :  t_sdb_record_array(c_clu_masters-1 downto 0); 
  begin
    v_clu_req_slaves  :=  f_cluster_sdb(cores, ramPerCore, is_dm);
    v_clu_req_masters :=  (c_msi_slave =>  f_sdb_auto_msi(msi_slave, true));

    v_ret  := f_xwb_bridge_layout_sdb(
            true, 
              --FIXME: this is borderline and only works bevause this CB is last in line. separate mastre and slave layout as done in the monster
            f_sdb_auto_layout(v_clu_req_slaves, v_clu_req_masters),  
            f_sdb_auto_sdb(v_clu_req_slaves, v_clu_req_masters)
          );
    v_ret.sdb_component.product := c_cluster_cb_product;
    return v_ret;      
  end f_cluster_bridge;
  
end ftm_pkg;
