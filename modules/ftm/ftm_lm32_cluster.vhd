library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.eca_pkg.all;
use work.ftm_pkg.all;

entity ftm_lm32_cluster is
generic(g_cores         : natural := 3;
        g_ram_per_core  : natural := 32768/4;
        g_msi_per_core  : natural := 4;
        g_profile       : string  := "medium_icache_debug";
        g_init_file     : string  := "msidemo.mif";   
        g_bridge_sdb    : t_sdb_bridge                      -- periphery crossbar         
   );
port(
clk_sys_i      : in  std_logic;
rst_n_i        : in  std_logic;
rst_lm32_n_i   : in  std_logic;

tm_tai8ns_i    : in std_logic_vector(63 downto 0);

ext_irq_slave_o    : out t_wishbone_slave_out; 
ext_irq_slave_i    : in  t_wishbone_slave_in;
         
ext_lm32_master_o   : out t_wishbone_master_out; 
ext_lm32_master_i : in  t_wishbone_master_in;  

ext_ram_slave_o    : out t_wishbone_slave_out;                            
ext_ram_slave_i    : in  t_wishbone_slave_in

);
end ftm_lm32_cluster;

architecture rtl of ftm_lm32_cluster is 

   --**************************************************************************--
   -- dummy periphery crossbar for testing
   ------------------------------------------------------------------------------
   --constant c_per_slaves   : natural := 2;
   --constant c_per_masters  : natural := 2;
   --constant c_per_layout   : t_sdb_record_array(c_per_slaves-1 downto 0) :=
   --(0 => f_sdb_embed_device(f_xwb_dpram(4096/4),   x"00000000"),
   -- 1 => f_sdb_embed_device(f_xwb_dpram(16384/4),   x"00000000"));

  --constant c_per_sdb_address : t_wishbone_address := x"000F0000";
  --constant c_per_bridge_sdb  : t_sdb_bridge       :=
  --  f_xwb_bridge_layout_sdb(true, c_per_layout, c_per_sdb_address);   
   ------------------------------------------------------------------------------

   --**************************************************************************--
   -- LM32 CROSSBAR. this is the main crossbar of the FTM, and it's BIG ...
   ------------------------------------------------------------------------------
   constant c_local_periphery : t_sdb_record_array(3 downto 0) :=
   (  0 => f_sdb_embed_device(c_cluster_info_sdb,           x"00000000"),
      1 => f_sdb_embed_device(c_eca_sdb,                    x"00000000"),
      2 => f_sdb_embed_device(c_eca_evt_sdb,                x"00000000"),
      3 => f_sdb_embed_bridge(g_bridge_sdb,                 x"00000000"));
   
   constant c_lm32_slaves   : natural := g_cores+c_local_periphery'length; -- an irq queue per lm32 + eca + ext interface out
   constant c_lm32_masters  : natural := g_cores; -- lm32's
   constant c_lm32_layout   : t_sdb_record_array(c_lm32_slaves-1 downto 0) := 
   f_sdb_automap_array(f_sdb_join_arrays(f_sdb_create_array(  device            => c_irq_ep_sdb, 
                        instances         => g_cores,
                        g_enum_dev_id     => true,
                        g_dev_id_offs     => 0,
                        g_enum_dev_name   => true,
                        g_dev_name_offs   => 0), 
                        c_local_periphery), x"00000000");

   constant c_lm32_sdb_address   : t_wishbone_address := f_sdb_create_rom_addr(c_lm32_layout);
   constant c_lm32_bridge_sdb  : t_sdb_bridge       :=
    f_xwb_bridge_layout_sdb(true, c_lm32_layout, c_lm32_sdb_address);       

   signal lm32_cbar_masterport_in   : t_wishbone_master_in_array  (c_lm32_slaves-1 downto 0);
   signal lm32_cbar_masterport_out  : t_wishbone_master_out_array (c_lm32_slaves-1 downto 0);
   signal lm32_cbar_slaveport_in    : t_wishbone_slave_in_array   (c_lm32_masters-1 downto 0);
   signal lm32_cbar_slaveport_out   : t_wishbone_slave_out_array  (c_lm32_masters-1 downto 0);
   ------------------------------------------------------------------------------
   
   --**************************************************************************--
   -- IRQ CROSSBAR
   ------------------------------------------------------------------------------
   constant c_ext_msi      : natural := g_msi_per_core -1;  -- lm32 irq sources are not masters of irq crossbar to reduce fan out
   constant c_irq_slaves   : natural := g_cores*c_ext_msi;  -- all but one irq queue per lm32 are connected here
   constant c_irq_masters  : natural := 2;                  -- eca action queues, interlocks
   
   ------------------------------------------------------------------------------
   -- there is no 'reverse' generic. this is awkward: since the master if(s) of
   -- the IRQ crossbar are slaves to the outside  world , all this might need 
   -- to be done in the top file as well so a possible higher level IRQ crossbar
   -- can insert us as a bridge.   
   constant c_irq_layout   : t_sdb_record_array(c_irq_slaves-1 downto 0) := 
   f_sdb_automap_array(f_sdb_create_array(device            => c_irq_ep_sdb, 
                                          instances         => c_irq_slaves,
                                          g_enum_dev_name   => true,
                                          g_dev_name_offs   => (g_cores*c_ext_msi-1)*10),  x"00000000");
   
   constant c_irq_sdb_address       : t_wishbone_address := f_sdb_create_rom_addr(c_irq_layout);

   signal irq_cbar_masterport_in    : t_wishbone_master_in_array  (c_irq_slaves-1 downto 0);
   signal irq_cbar_masterport_out   : t_wishbone_master_out_array (c_irq_slaves-1 downto 0);
   signal irq_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_irq_masters-1 downto 0);
   signal irq_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_irq_masters-1 downto 0);

   --**************************************************************************--
   -- RAM CROSSBAR
   ------------------------------------------------------------------------------
   constant c_ram_slaves   : natural := g_cores;  
   constant c_ram_masters  : natural := 1;       
   
   ------------------------------------------------------------------------------
   -- there is no 'reverse' generic. this is awkward: since the master of the  
   -- RAM crossbar is a slave to the outside  world (top crossbar), all this 
   -- needs to be done in the top file as well so the top crossbar can insert us
   -- as a bridge.          
    constant c_ram_layout   : t_sdb_record_array(c_ram_slaves-1 downto 0) := 
    f_sdb_automap_array(f_sdb_create_array(device            => f_xwb_dpram(g_ram_per_core), 
                                           instances         => g_cores,
                                           g_enum_dev_name   => true,
                                           g_dev_name_offs   => g_cores*10),  x"00000000");
   
    constant c_ram_sdb_address       : t_wishbone_address := f_sdb_create_rom_addr(c_ram_layout);
   ------------------------------------------------------------------------------
  
   signal ram_cbar_masterport_in    : t_wishbone_master_in_array  (c_ram_slaves-1 downto 0);
   signal ram_cbar_masterport_out   : t_wishbone_master_out_array (c_ram_slaves-1 downto 0);
   signal ram_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_ram_masters-1 downto 0);
   signal ram_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_ram_masters-1 downto 0);
   
   signal irq_rewire_out            : t_wishbone_slave_out_array  (g_msi_per_core*g_cores-1 downto 0);
   signal irq_rewire_in             : t_wishbone_slave_in_array   (g_msi_per_core*g_cores-1 downto 0);


  begin

   G1: for I in 0 to g_cores-1 generate
      --instantiate an ftm-lm32 (LM32 core with its own DPRAM and 4..n msi queues)
      LM32 : ftm_lm32
      generic map(g_cpu_id                         => x"BBEE" & std_logic_vector(to_unsigned(I, 16)),
                  g_size                           => g_ram_per_core,
                  g_bridge_sdb                     => c_lm32_bridge_sdb,
                  g_profile                        => g_profile,
                  g_init_file                      => g_init_file,
                  g_msi_queues                     => g_msi_per_core)
      port map(clk_sys_i                           => clk_sys_i,
               rst_n_i                             => rst_n_i,
               rst_lm32_n_i                        => rst_lm32_n_i,

               tm_tai8ns_i                         => tm_tai8ns_i,            
               --LM32               
               lm32_master_o => lm32_cbar_slaveport_in  (I),
               lm32_master_i => lm32_cbar_slaveport_out (I), 
               -- MSI
               irq_slaves_o  => irq_rewire_out((I+1)*g_msi_per_core-1 downto I*g_msi_per_core),
               irq_slaves_i  => irq_rewire_in ((I+1)*g_msi_per_core-1 downto I*g_msi_per_core),       
               --2nd RAM port               
               ram_slave_o   => ram_cbar_masterport_in(I),                      
               ram_slave_i   => ram_cbar_masterport_out(I));
   
               --------------------------------------------------------------------------------------------------------------------------------------   
               -- Wire MSI Queues (workaround, not possible in instanciation)              
               -- Prio 0: ECA               
               irq_cbar_masterport_in(I*c_ext_msi+0)                             <= irq_rewire_out(I*g_msi_per_core+0);
               irq_rewire_in(I*g_msi_per_core+0)                                 <= irq_cbar_masterport_out(I*c_ext_msi+0);
               -- Prio 1: Other LM32s 
               lm32_cbar_masterport_in(I)                                        <= irq_rewire_out(I*g_msi_per_core+1);
               irq_rewire_in(I*g_msi_per_core+1)                                 <= lm32_cbar_masterport_out(I); 
               -- Prio 2: Interlocks 
               irq_cbar_masterport_in(I*c_ext_msi+1)                             <= irq_rewire_out(I*g_msi_per_core+2);
               irq_rewire_in(I*g_msi_per_core+2)                                 <= irq_cbar_masterport_out(I*c_ext_msi+1);
               -- Prio 3-n: Others 
               irq_cbar_masterport_in((I+1)*c_ext_msi-1 downto I*c_ext_msi+2)    <= irq_rewire_out((I+1)*g_msi_per_core-1 downto I*g_msi_per_core+3);
               irq_rewire_in((I+1)*g_msi_per_core-1 downto I*g_msi_per_core+3)   <= irq_cbar_masterport_out((I+1)*c_ext_msi-1 downto I*c_ext_msi+2);
               -------------------------------------------------------------------------------------------------------------------------------------- 
         
      end generate G1;  
  
   LM32_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_lm32_masters,
     g_num_slaves  => c_lm32_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_lm32_layout,
     g_sdb_addr    => c_lm32_sdb_address)
   port map(
     clk_sys_i     => clk_sys_i,
     rst_n_i       => rst_n_i,
     -- Master connections (INTERCON is a slave)
     slave_i       => lm32_cbar_slaveport_in,
     slave_o       => lm32_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => lm32_cbar_masterport_in,
     master_o      => lm32_cbar_masterport_out);

   -- last slave on the lm32 crossbar is the connection to the periphery crossbar
   ext_lm32_master_o                         <= lm32_cbar_masterport_out(c_lm32_slaves-1);
   lm32_cbar_masterport_in(c_lm32_slaves-1)  <= ext_lm32_master_i;  

   --------------------------------------------------------------------------------
-- Slave - CLUSTER INFO ROM 
--------------------------------------------------------------------------------  
   cluster_info_rom : process(clk_sys_i)
   begin
      if rising_edge(clk_sys_i) then
         -- This is an easy solution for a device that never stalls:
         lm32_cbar_masterport_in(g_cores).ack <= lm32_cbar_masterport_out(g_cores).cyc and lm32_cbar_masterport_out(g_cores).stb;
         lm32_cbar_masterport_in(g_cores).dat <= std_logic_vector(to_unsigned(g_cores,32));
      end if;
   end process;   
   
   
   IRQ_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_irq_masters,
     g_num_slaves  => c_irq_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_irq_layout,
     g_sdb_addr    => c_irq_sdb_address)
   port map(
     clk_sys_i     => clk_sys_i,
     rst_n_i       => rst_n_i,
     -- Master connections (INTERCON is a slave)
     slave_i       => irq_cbar_slaveport_in,
     slave_o       => irq_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => irq_cbar_masterport_in,
     master_o      => irq_cbar_masterport_out);

   ext_irq_slave_o            <= irq_cbar_slaveport_out(0);
   irq_cbar_slaveport_in(0)   <= ext_irq_slave_i;

   RAM_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_ram_masters,
     g_num_slaves  => c_ram_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_ram_layout,
     g_sdb_addr    => c_ram_sdb_address)
   port map(
     clk_sys_i     => clk_sys_i,
     rst_n_i       => rst_n_i,
        -- Master connections (INTERCON is a slave)
     slave_i       => ram_cbar_slaveport_in,
     slave_o       => ram_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => ram_cbar_masterport_in,
     master_o      => ram_cbar_masterport_out);

     ext_ram_slave_o          <= ram_cbar_slaveport_out(0);                           
     ram_cbar_slaveport_in(0) <= ext_ram_slave_i; 
 
  end architecture rtl;
