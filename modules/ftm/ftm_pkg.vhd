library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package ftm_pkg is



  


------------------------------------------------------------------------------
-- Components declaration
-------------------------------------------------------------------------------

   component ftm_lm32 is
   generic(g_cpu_id        : t_wishbone_data := x"CAFEBABE";
      g_size          : natural := 16384;                 -- size of the dpram
      g_bridge_sdb    : t_sdb_bridge;                     -- record for the superior bridge
      g_profile       : string := "medium_icache_debug";  -- lm32 profile
      g_init_file     : string := "";          -- memory init file - binary for lm32
      g_addr_ext_bits : natural := 1;                     -- address extension bits (starting from MSB)
      g_msi_queues    : natural := 3);                    -- number of msi queues connected to the lm32
   port(
      clk_sys_i      : in  std_logic;  -- system clock 
      rst_n_i        : in  std_logic;  -- reset, active low 
      rst_lm32_n_i   : in  std_logic;  -- reset, active low
      
      tm_tai8ns_i    : in std_logic_vector(63 downto 0);
      -- wb master interface of the lm32
      lm32_master_o  : out t_wishbone_master_out; 
      lm32_master_i  : in  t_wishbone_master_in;  
      -- wb msi interfaces
      irq_slaves_o   : out t_wishbone_slave_out_array(g_msi_queues-1 downto 0);  
      irq_slaves_i   : in  t_wishbone_slave_in_array(g_msi_queues-1 downto 0);
      -- port B of the LM32s DPRAM 
      ram_slave_o    : out t_wishbone_slave_out;                           
      ram_slave_i    : in  t_wishbone_slave_in
   );
   end component;

   
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
    name          => "TAI_TIME_8NS       ")));
  
   constant c_cpu_info_sdb : t_sdb_device := (
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
    device_id     => x"10040085",
    version       => x"00000001",
    date          => x"20131009",
    name          => "CPU_INFO_ROM       ")));

   component ftm_lm32_cluster is
   generic(g_cores         : natural := 3;
        g_ram_per_core  : natural := 32768/4;
        g_msi_per_core  : natural := 4;
        g_profile       : string  := "medium_icache_debug";
        g_init_file     : string  := "";   
        g_bridge_sdb    : t_sdb_bridge                      -- periphery crossbar         
   );
   port(
      clk_sys_i         : in  std_logic;
      rst_n_i           : in  std_logic;
      rst_lm32_n_i      : in  std_logic;

      tm_tai8ns_i       : in std_logic_vector(63 downto 0);

      ext_irq_slave_o   : out t_wishbone_slave_out; 
      ext_irq_slave_i   : in  t_wishbone_slave_in;
            
      ext_lm32_master_o : out t_wishbone_master_out; 
      ext_lm32_master_i : in  t_wishbone_master_in;  

      ext_ram_slave_o   : out t_wishbone_slave_out;                            
      ext_ram_slave_i   : in  t_wishbone_slave_in
   );
   end component;
      

    constant c_cluster_info_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000FF",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040086",
    version       => x"00000001",
    date          => x"20131009",
    name          => "CLUSTER_INFO_ROM   "))); 

  

  
end ftm_pkg;
