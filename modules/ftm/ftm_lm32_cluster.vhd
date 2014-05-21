library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.ftm_pkg.all;

entity ftm_lm32_cluster is
generic(g_is_ftm        : boolean := false;
        g_cores         : natural := 1;
        g_ram_per_core  : natural := 32768/4;
        g_shared_mem    : natural := 32768/4;
        g_msi_per_core  : natural := 2;
        g_profile       : string  := "medium_icache_debug";
        g_init_file     : string  := "msidemo.mif";   
        g_world_bridge_sdb    : t_sdb_bridge                      -- superior crossbar         
   );
port(
   clk_sys_i      : in  std_logic;
   rst_n_i        : in  std_logic;
   rst_lm32_n_i   : in  std_logic;

   tm_tai8ns_i    : in std_logic_vector(63 downto 0);

   irq_slave_o    : out t_wishbone_slave_out; 
   irq_slave_i    : in  t_wishbone_slave_in;

   -- optional cluster ctrl slave interface
   cluster_slave_o  : out t_wishbone_slave_out; 
   cluster_slave_i  : in  t_wishbone_slave_in := ('0', '0', x"00000000", x"F", '0', x"00000000");

   -- optional FTM ebm queue master interface
   ftm_queue_master_o : out t_wishbone_master_out; 
   ftm_queue_master_i : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000"); 
            
   master_o   : out t_wishbone_master_out; 
   master_i   : in  t_wishbone_master_in  
);
end ftm_lm32_cluster;

architecture rtl of ftm_lm32_cluster is 

   --**************************************************************************--
   -- WORLD CROSSBAR. mux CPUs to world IF (not an SDB crossbar!)
   ------------------------------------------------------------------------------
   constant c_world_slaves  : natural := 1;
   constant c_world_masters : natural := g_cores;
   signal world_cbar_masterport_in   : t_wishbone_master_in_array  (c_world_slaves-1 downto 0);
   signal world_cbar_masterport_out  : t_wishbone_master_out_array (c_world_slaves-1 downto 0);
   signal world_cbar_slaveport_in    : t_wishbone_slave_in_array   (c_world_masters-1 downto 0);
   signal world_cbar_slaveport_out   : t_wishbone_slave_out_array  (c_world_masters-1 downto 0);

      --**************************************************************************--
   -- IRQ CROSSBAR
   ------------------------------------------------------------------------------
   constant c_irq_slaves   : natural := g_cores*g_msi_per_core;  -- all but one irq queue per lm32 are connected here
   constant c_irq_masters  : natural := 2;                  -- eca action queues, interlocks
   constant c_irq_layout_aux  : t_sdb_record_array(c_irq_slaves-1 downto 0) := f_cluster_irq_sdb(g_cores, g_msi_per_core);
   constant c_irq_sdb_address : t_wishbone_address := f_sdb_auto_sdb(c_irq_layout_aux);
   constant c_irq_layout      : t_sdb_record_array(c_irq_slaves-1 downto 0) := f_sdb_auto_layout(c_irq_layout_aux);
   
   signal irq_cbar_masterport_in    : t_wishbone_master_in_array  (c_irq_slaves-1 downto 0);
   signal irq_cbar_masterport_out   : t_wishbone_master_out_array (c_irq_slaves-1 downto 0);
   signal irq_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_irq_masters-1 downto 0);
   signal irq_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_irq_masters-1 downto 0);

   --**************************************************************************--
   -- RAM CROSSBAR
   ------------------------------------------------------------------------------
   constant c_ram_slaves      : natural := g_cores;  
   constant c_ram_masters     : natural := 1;       
   constant c_ram_layout_aux  : t_sdb_record_array(c_ram_slaves-1 downto 0) := f_cluster_ram_sdb(c_ram_slaves, g_ram_per_core);
   constant c_ram_sdb_address : t_wishbone_address := f_sdb_auto_sdb(c_ram_layout_aux);
   constant c_ram_layout      : t_sdb_record_array(c_ram_slaves-1 downto 0) := f_sdb_auto_layout(c_ram_layout_aux);
   signal ram_cbar_masterport_in    : t_wishbone_master_in_array  (c_ram_slaves-1 downto 0);
   signal ram_cbar_masterport_out   : t_wishbone_master_out_array (c_ram_slaves-1 downto 0);
   signal ram_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_ram_masters-1 downto 0);
   signal ram_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_ram_masters-1 downto 0);

   --**************************************************************************--
   -- LM32 CROSSBAR. this is the main crossbar of the FTM
   ------------------------------------------------------------------------------
   --
   -- <--- World <--- All LM32s master ports 
   --
   -- ---> LM32 <--- All LM32s Cluster ports
   --       |
   --       |---> RAM ---> All LM32 RAM ports
   --       |---> IRQ ---> All LM32 IRQ ports 
   --             ^
   -- ---->-------|
   
   --constant c_clu_slaves    -> pkg  : natural := 7; 
   constant c_clu_masters     : natural := g_cores+1; -- lm32's + top
   constant c_cluster_ext_if  : natural := c_clu_masters-1; -- last master is ext if
   
   constant c_clu_layout_aux  : t_sdb_record_array(c_clu_slaves-1 downto 0)
   := f_cluster_main_sdb(c_irq_layout_aux, c_ram_layout_aux, g_shared_mem, g_is_ftm);
   constant c_clu_sdb_address : t_wishbone_address := f_sdb_auto_sdb(c_clu_layout_aux);
   constant c_clu_layout      : t_sdb_record_array(c_clu_slaves-1 downto 0)
   := f_sdb_auto_layout(c_clu_layout_aux);
   
   constant c_clu_bridge_sdb  : t_sdb_bridge       
   := f_lm32_main_bridge_sdb(g_cores, g_msi_per_core, g_ram_per_core, g_shared_mem, g_is_ftm);
   
   signal clu_cbar_masterport_in   : t_wishbone_master_in_array  (c_clu_slaves-1 downto 0);
   signal clu_cbar_masterport_out  : t_wishbone_master_out_array (c_clu_slaves-1 downto 0);
   signal clu_cbar_slaveport_in    : t_wishbone_slave_in_array   (c_clu_masters-1 downto 0);
   signal clu_cbar_slaveport_out   : t_wishbone_slave_out_array  (c_clu_masters-1 downto 0);
   ------------------------------------------------------------------------------
   
   signal r_rst_lm32_n,
          s_rst_lm32_n,
          s_rst_lm32_n_aux           : std_logic_vector(31 downto 0);            

   begin

   G1: for I in 0 to g_cores-1 generate
      --instantiate an ftm-lm32 (LM32 core with its own DPRAM and 2..n msi queues)
      LM32 : ftm_lm32
      generic map(g_cpu_id                         => x"BBEE" & std_logic_vector(to_unsigned(I, 16)),
                  g_size                           => g_ram_per_core,
                  g_is_in_cluster                  => true,
                  g_cluster_bridge_sdb             => c_clu_bridge_sdb,
                  g_world_bridge_sdb               => g_world_bridge_sdb,
                  g_profile                        => g_profile,
                  g_init_file                      => g_init_file,
                  g_msi_queues                     => g_msi_per_core) -- 1 for inter CPU communication
      port map(clk_sys_i      => clk_sys_i,
               rst_n_i        => rst_n_i,
               rst_lm32_n_i   => s_rst_lm32_n(I),

               tm_tai8ns_i    => tm_tai8ns_i,            
               
               clu_master_o   => clu_cbar_slaveport_in (I), 
               clu_master_i   => clu_cbar_slaveport_out (I),
               --LM32               
               world_master_o => world_cbar_slaveport_in  (I),
               world_master_i => world_cbar_slaveport_out (I), 
               -- MSI
               irq_slaves_o   => irq_cbar_masterport_in ((I+1)*g_msi_per_core-1 downto I*g_msi_per_core),
               irq_slaves_i   => irq_cbar_masterport_out ((I+1)*g_msi_per_core-1 downto I*g_msi_per_core),       
               --2nd RAM port               
               ram_slave_o    => ram_cbar_masterport_in(I),                      
               ram_slave_i    => ram_cbar_masterport_out(I));
   
               -------------------------------------------------------------------------------------------------------------------------------------- 
         
      end generate G1;  

-- must be transparent, NOT SDB
  WORLD_CON : xwb_crossbar 
  generic map(
    g_num_masters => c_world_masters,
    g_num_slaves  => c_world_slaves,
    g_registered  => true,
    -- Address of the slaves connected
    g_address     => (0 => x"00000000"),
    g_mask        => (0 => x"00000000"))
  port map(
     clk_sys_i     => clk_sys_i,
     rst_n_i       => rst_n_i,
        -- Master connections (INTERCON is a slave)
     slave_i       => world_cbar_slaveport_in,
     slave_o       => world_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => world_cbar_masterport_in,
     master_o      => world_cbar_masterport_out);

   -- 1st slave is external world IF
   world_cbar_masterport_in(0) <= master_i; 
   master_o <= world_cbar_masterport_out(0);  
  

   
   CLUSTER_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_clu_masters,
     g_num_slaves  => c_clu_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_clu_layout,
     g_sdb_addr    => c_clu_sdb_address)
   port map(
     clk_sys_i     => clk_sys_i,
     rst_n_i       => rst_n_i,
     -- Master connections (INTERCON is a slave)
     slave_i       => clu_cbar_slaveport_in,
     slave_o       => clu_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => clu_cbar_masterport_in,
     master_o      => clu_cbar_masterport_out);

   -- <--- World <--- All LM32s master ports 
   --
   -- ---> CLU <--- All LM32s Cluster ports
   --       |
   --       |---> RAM ---> All LM32 RAM ports
   --       |---> IRQ ---> All LM32 IRQ ports 
   --             ^
   -- ---->-------|

   -- the first n masters are the lm32 cores. c_cluster_slave_if is the outside world and master to LM32_CON
   cluster_slave_o                           <= clu_cbar_slaveport_out(c_cluster_ext_if);
   clu_cbar_slaveport_in(c_cluster_ext_if)   <= cluster_slave_i;  


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

   -- 1st master is cluster crossbar
   clu_cbar_masterport_in(c_clu_irq_bridge)  <= irq_cbar_slaveport_out(0);                           
   irq_cbar_slaveport_in(0)                  <= clu_cbar_masterport_out(c_clu_irq_bridge);

   -- 2nd master is external irq slave if
   irq_slave_o                               <= irq_cbar_slaveport_out(1);
   irq_cbar_slaveport_in(1)                  <= irq_slave_i;

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

   -- 1st master is cluster crossbar
   clu_cbar_masterport_in(c_clu_ram_bridge)  <= ram_cbar_slaveport_out(0);                           
   ram_cbar_slaveport_in(0)                  <= clu_cbar_masterport_out(c_clu_ram_bridge);  

--------------------------------------------------------------------------------
-- Slave - CLUSTER INFO ROM 
--------------------------------------------------------------------------------  
   cluster_info_rom : process(clk_sys_i)
   variable vIdx : natural;
   begin
      vIdx := c_clu_cluster_info;
      if rising_edge(clk_sys_i) then
         -- This is an easy solution for a device that never stalls:
         clu_cbar_masterport_in(vIdx).ack <= clu_cbar_masterport_out(vIdx).cyc and clu_cbar_masterport_out(vIdx).stb;
         clu_cbar_masterport_in(vIdx).dat <= std_logic_vector(to_unsigned(g_cores,32));
      end if;
   end process;
   
  clu_cbar_masterport_in(c_clu_cluster_info).stall <= '0';
  clu_cbar_masterport_in(c_clu_cluster_info).err   <= '0';
   
   --------------------------------------------------------------------------------
-- SHARED MEMORY
--------------------------------------------------------------------------------   
   SHARED_MEM : xwb_dpram
   generic map(
      g_size                  => g_shared_mem,
      g_init_file             => "",
      g_must_have_init_file   => false,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => BYTE)  
   port map(
      clk_sys_i   => clk_sys_i,
      rst_n_i     => rst_n_i,
      slave1_i    => clu_cbar_masterport_out(c_clu_shared_mem),
      slave1_o    => clu_cbar_masterport_in(c_clu_shared_mem),
      slave2_i    => c_dummy_slave_in,
      slave2_o    => open);


--******************************************************************************
-- FTM Prio Queue
--------------------------------------------------------------------------------
 prioQ : if(g_is_ftm) generate  
   prio_queue : ftm_priority_queue
   generic map(
      g_idx_width    => 7,
      g_key_width    => 64, 
      g_val_width    => 192 -- 2**7 -> 128 entries, 8 * 32b per entry (64b key, 192b value)
   )           
   port map(
      clk_sys_i   => clk_sys_i,
      rst_n_i     => rst_n_i,

      time_sys_i  => tm_tai8ns_i,

      ctrl_i      => clu_cbar_masterport_out(c_clu_ebm_queue_c),
      ctrl_o      => clu_cbar_masterport_in(c_clu_ebm_queue_c),
      
      snk_i       => clu_cbar_masterport_out(c_clu_ebm_queue_d),
      snk_o       => clu_cbar_masterport_in(c_clu_ebm_queue_d),
      
      src_o       => ftm_queue_master_o,
      src_i       => ftm_queue_master_i
     
   );
 end generate;
 
    
--******************************************************************************
-- makeshift ftm load manager / rst control
--------------------------------------------------------------------------------
   rst_ctrl : process(clk_sys_i)
   variable vIdx : natural; 
   begin
    vIdx := c_clu_load_mgr;
    
    if rising_edge(clk_sys_i) then
      if(rst_n_i = '0') then
        r_rst_lm32_n <= (others => '1');
      else
    
        clu_cbar_masterport_in(vIdx).dat <= (others => '0');      
        clu_cbar_masterport_in(vIdx).ack <= clu_cbar_masterport_out(vIdx).cyc and clu_cbar_masterport_out(vIdx).stb;
         
        if(clu_cbar_masterport_out(vIdx).cyc = '1' and clu_cbar_masterport_out(vIdx).stb = '1') then         
           case(to_integer(unsigned(clu_cbar_masterport_out(vIdx).adr(7 downto 2)) & "00")) is
              when 0 => clu_cbar_masterport_in(vIdx).dat <= r_rst_lm32_n;
                        if(clu_cbar_masterport_out(vIdx).we = '1') then
                          r_rst_lm32_n <= clu_cbar_masterport_out(vIdx).dat;
                        end if; 
                           
              when others => null;
           end case;
        end if;
      end if;
    end if;
  end process;   

  clu_cbar_masterport_in(c_clu_load_mgr).stall <= '0';
  clu_cbar_masterport_in(c_clu_load_mgr).err   <= '0';   
  
  s_rst_lm32_n_aux <= (others => rst_lm32_n_i);
  s_rst_lm32_n <= r_rst_lm32_n and s_rst_lm32_n_aux;
   
   


      
 
  end architecture rtl;
