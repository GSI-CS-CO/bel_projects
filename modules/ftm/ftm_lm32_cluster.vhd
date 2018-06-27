--! @file        dm_lm32_cluster.vhd
--  DesignUnit   dm_lm32_cluster
--! @author      M. Kreider <>
--! @date        25/02/2014
--! @version     0.0.3
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief LM32 Cluster. Instantiates desired number of LM32 CPUs + Periphery 
--!
--! Cluster Info ROM Registers:
--! 0x00 Number of Cores
--! 0x04 MSI Endpoints per Core
--! 0x08 RAM size per Core
--! 0x0C Configured as DataMaster?
--
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------
--


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.ftm_pkg.all;
use work.prio_pkg.all;
use work.etherbone_pkg.all;


entity ftm_lm32_cluster is
generic(
  g_is_dm         : boolean := false;
  g_cores         : natural := 1;
  g_ram_per_core  : natural := 32768/4;
  g_profiles      : string  := "medium_icache_debug";
  g_init_files    : string;   
  g_world_bridge_sdb : t_sdb_bridge;   -- inferior sdb crossbar         
  g_clu_msi_sdb      : t_sdb_msi    -- superior msi crossbar          
);
port(
  clk_ref_i      : in  std_logic;
  rst_ref_n_i    : in  std_logic;

  clk_sys_i      : in  std_logic;
  rst_sys_n_i    : in  std_logic;
  rst_lm32_n_i   : in  std_logic_vector(g_cores-1 downto 0); 

  tm_tai8ns_i    : in std_logic_vector(63 downto 0);

  -- lm32 core interfaces       
  lm32_masters_o    : out t_wishbone_master_out_array(g_cores-1 downto 0); 
  lm32_masters_i    : in  t_wishbone_master_in_array(g_cores-1 downto 0);
  lm32_msi_slaves_o : out t_wishbone_slave_out_array(g_cores-1 downto 0); 
  lm32_msi_slaves_i : in  t_wishbone_slave_in_array(g_cores-1 downto 0);  

  -- cluster crossbar interface
  clu_slave_o       : out t_wishbone_slave_out; 
  clu_slave_i       : in  t_wishbone_slave_in := ('0', '0', x"00000000", x"F", '0', x"00000000");
  clu_msi_master_o  : out t_wishbone_master_out;
  clu_msi_master_i  : in t_wishbone_master_in;

  -- optional prioq interface
  dm_prioq_master_o : out t_wishbone_master_out; 
  dm_prioq_master_i : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000")
  -- no msi required 
  
   
);
end ftm_lm32_cluster;

architecture rtl of ftm_lm32_cluster is 

  --LM32 direct connections to/from outside world

  signal lm32_masters_in      : t_wishbone_master_in_array   (g_cores-1 downto 0);
  signal lm32_masters_out     : t_wishbone_master_out_array  (g_cores-1 downto 0);
  signal lm32_msi_slaves_in   : t_wishbone_slave_in_array   (g_cores-1 downto 0);
  signal lm32_msi_slaves_out  : t_wishbone_slave_out_array  (g_cores-1 downto 0); 


  --**************************************************************************--
  -- Cluster CROSSBAR
  ------------------------------------------------------------------------------
  -- can't be done in package :(
  constant c_clu_slaves     : natural := c_static_cluster_slaves + g_cores; -- info rom, prioq ctrl, diag, rams
   
  

  --layout
  constant c_clu_layout_req_slaves  : t_sdb_record_array(c_clu_slaves-1 downto 0)  :=
           f_cluster_sdb(g_cores, g_ram_per_core, g_is_dm);

  constant c_clu_layout_req_masters : t_sdb_record_array(c_clu_masters-1 downto 0) := 
           (c_msi_slave =>  f_sdb_auto_msi(g_clu_msi_sdb, true));

  constant c_clu_sdb_address : t_wishbone_address :=
           f_sdb_auto_sdb(c_clu_layout_req_slaves, c_clu_layout_req_masters);

  constant c_clu_layout : t_sdb_record_array(c_clu_slaves + c_clu_masters -1 downto 0) :=
           f_sdb_auto_layout(c_clu_layout_req_slaves, c_clu_layout_req_masters);

  signal clu_cb_masterport_in    : t_wishbone_master_in_array  (c_clu_slaves-1 downto 0);
  signal clu_cb_masterport_out   : t_wishbone_master_out_array (c_clu_slaves-1 downto 0);
  signal clu_cb_slaveport_in     : t_wishbone_slave_in_array   (c_clu_masters-1 downto 0);
  signal clu_cb_slaveport_out    : t_wishbone_slave_out_array  (c_clu_masters-1 downto 0);
  signal clu_msi_masterport_in   : t_wishbone_master_in_array  (c_clu_masters-1 downto 0);
  signal clu_msi_masterport_out  : t_wishbone_master_out_array (c_clu_masters-1 downto 0);
  --**************************************************************************--


  -- prioq to sync
  signal s_prio_data_in   : t_wishbone_master_in;
  signal s_prio_data_out  : t_wishbone_master_out;

  -- LM32s to prioq
  signal prioq_slaves_in  : t_wishbone_slave_in_array(g_cores-1 downto 0);
  signal prioq_slaves_out : t_wishbone_slave_out_array(g_cores-1 downto 0);

  signal s_rst_lm32_n,
         r_rst_lm32_n0,
         r_rst_lm32_n1    : std_logic_vector(g_cores-1 downto 0);            
  signal s_clu_info       : t_wishbone_master_in;
  signal stall_diag       : std_logic_vector(g_cores-1 downto 0); 
  signal cycle_diag       : std_logic_vector(g_cores-1 downto 0);

begin

  G1: for I in 0 to g_cores-1 generate
    --instantiate an dm-lm32 (LM32 core with its own DPRAM and 2..n msi queues)
    LM32 : ftm_lm32
    generic map(
      g_cpu_id                         => x"BBEE" & std_logic_vector(to_unsigned(I, 16)),
      g_size                           => g_ram_per_core,
      g_world_bridge_sdb               => g_world_bridge_sdb,
      g_profile                        => f_substr(g_profiles, I, ';'),
      g_init_file                      => f_substr(g_init_files, I, ';'),
		  g_is_dm                          => g_is_dm
    ) 
    port map(
      clk_sys_i      => clk_ref_i,
      rst_n_i        => rst_ref_n_i,
      rst_lm32_n_i   => s_rst_lm32_n(I),

      tm_tai8ns_i    => tm_tai8ns_i,

      stall_diag_o   => stall_diag(I),          
      cycle_diag_o   => cycle_diag(I),
      --LM32               
      world_master_o => lm32_masters_out(I),
      world_master_i => lm32_masters_in(I),
      --optional prioq interface for DM
      prioq_master_o => prioq_slaves_in (I),
      prioq_master_i => prioq_slaves_out(I),
      -- MSI
      msi_slave_i    => lm32_msi_slaves_in (I),
      msi_slave_o    => lm32_msi_slaves_out (I),       
      --2nd RAM port               
      ram_slave_o    => clu_cb_masterport_in(c_static_cluster_slaves + I),                      
      ram_slave_i    => clu_cb_masterport_out(c_static_cluster_slaves + I)
    );
   
    -- CPUs, RAMs and PrioQ live in Ref domain. Sync CPU bus to Sys domain - wb master & MSI slave.
    master_ref2sys : xwb_clock_crossing
    port map(
      -- Slave control port
      slave_clk_i    => clk_ref_i,
      slave_rst_n_i  => rst_ref_n_i,
      slave_i        => lm32_masters_out(I),
      slave_o        => lm32_masters_in(I),
      -- Master reader port
      master_clk_i   => clk_sys_i,
      master_rst_n_i => rst_sys_n_i,
      master_i       => lm32_masters_i(I),
      master_o       => lm32_masters_o(I)
    );

    msi_sys2ref : xwb_clock_crossing
    port map(
      -- Slave control port
      slave_clk_i    => clk_sys_i,
      slave_rst_n_i  => rst_sys_n_i,
      slave_i        => lm32_msi_slaves_i(I),
      slave_o        => lm32_msi_slaves_o(I),
      -- Master reader port
      master_clk_i   => clk_ref_i,
      master_rst_n_i => rst_ref_n_i,
      master_i       => lm32_msi_slaves_out(I),
      master_o       => lm32_msi_slaves_in(I)
    );  

  end generate G1;  


  CLU_CON : xwb_sdb_crossbar
  generic map(
    g_num_masters => c_clu_masters,
    g_num_slaves  => c_clu_slaves,
    g_registered  => true,
    g_wraparound  => true,
    g_layout      => c_clu_layout,
    g_sdb_addr    => c_clu_sdb_address)
  port map(
    clk_sys_i     => clk_ref_i,
    rst_n_i       => rst_ref_n_i,
      -- Master connections (INTERCON is a slave)
    slave_i       => clu_cb_slaveport_in,
    slave_o       => clu_cb_slaveport_out,
    msi_master_i  => clu_msi_masterport_in,
    msi_master_o  => clu_msi_masterport_out,
    -- Slave connections (INTERCON is a master)
    master_i      => clu_cb_masterport_in,
    master_o      => clu_cb_masterport_out);

  --Cluster CB lives in Ref domain. Sync wb slave in and msi master out to Sys domain
  clu_sdb_sys2ref : xwb_clock_crossing
  port map(
    -- Slave control port
    slave_clk_i    => clk_sys_i,
    slave_rst_n_i  => rst_sys_n_i,
    slave_i        => clu_slave_i,
    slave_o        => clu_slave_o,
    -- Master reader port
    master_clk_i   => clk_ref_i,
    master_rst_n_i => rst_ref_n_i,
    master_i       => clu_cb_slaveport_out(c_clu_info_rom),
    master_o       => clu_cb_slaveport_in(c_clu_info_rom));

  clu_msi_sys2ref : xwb_clock_crossing
  port map(
    -- Slave control port
    slave_clk_i    => clk_ref_i,
    slave_rst_n_i  => rst_ref_n_i,
    slave_i        => clu_msi_masterport_out(c_msi_slave),
    slave_o        => clu_msi_masterport_in(c_msi_slave),
    -- Master reader port
    master_clk_i   => clk_sys_i,
    master_rst_n_i => rst_sys_n_i,
    master_i       => clu_msi_master_i,
    master_o       => clu_msi_master_o);

--******************************************************************************
-- dm Prio Queue
--------------------------------------------------------------------------------
  prioQ : if(g_is_dm) generate
    prio_queue : prio
     generic map(
      g_ebm_bits    => f_hi_adr_bits(c_ebm_sdb),
      g_depth       => 16,
      g_num_masters => g_cores
    )
    port map(
      clk_i         => clk_ref_i,
      rst_n_i       => rst_ref_n_i,

      time_i        => tm_tai8ns_i,

      ctrl_i        => clu_cb_masterport_out(c_clu_pq_ctrl),
      ctrl_o        => clu_cb_masterport_in(c_clu_pq_ctrl),
      slaves_i      => prioq_slaves_in,
      slaves_o      => prioq_slaves_out,
      master_o      => s_prio_data_out,
      master_i      => s_prio_data_in

    );


    -- Prioq output lives in Ref domain, sync to Sys domain
    priossrc_ref2sys : xwb_clock_crossing
    port map(
      -- Slave control port
      slave_clk_i    => clk_ref_i,
      slave_rst_n_i  => rst_ref_n_i,
      slave_i        => s_prio_data_out,
      slave_o        => s_prio_data_in,
      -- Master reader port
      master_clk_i   => clk_sys_i,
      master_rst_n_i => rst_sys_n_i,
      master_i       => dm_prioq_master_i,
      master_o       => dm_prioq_master_o
    );

    -- Diagnostic module lives completely in REF domain, no sync to clu CB, TAI time or LM32 bus lines necessary

    diagnostics :  dm_diag
    generic map(
      g_cores => g_cores --CPU cores
    )
    port map(
      clk_ref_i      => clk_ref_i,
      rst_ref_n_i    => rst_ref_n_i,
      tm_tai8ns_i    => tm_tai8ns_i,
      cyc_diag_i     => cycle_diag,
      stall_diag_i   => stall_diag,
      
      ctrl_i         => clu_cb_masterport_out(c_clu_diag),
      ctrl_o         => clu_cb_masterport_in(c_clu_diag)
    );

  end generate;
 
  cluster_info_rom : process(clk_ref_i)
  variable vIdx : natural;
  begin
    vIdx := c_clu_info_rom;
    if rising_edge(clk_ref_i) then
      if(rst_ref_n_i = '0') then
        s_clu_info <= ('0', '0', '0', '0', '0', (others => '0'));
      else
        -- rom is an easy solution for a device that never stalls:
        s_clu_info.dat <= (others => '0');
        -- read only
        s_clu_info.ack <= clu_cb_masterport_out(vIdx).cyc and clu_cb_masterport_out(vIdx).stb and not   clu_cb_masterport_out(vIdx).we;
        s_clu_info.err <= clu_cb_masterport_out(vIdx).cyc and clu_cb_masterport_out(vIdx).stb and       clu_cb_masterport_out(vIdx).we;

        if(clu_cb_masterport_out(vIdx).cyc = '1' and clu_cb_masterport_out(vIdx).stb = '1') then
          case(to_integer(unsigned(clu_cb_masterport_out(vIdx).adr(4 downto 2)))) is
            when 0 => s_clu_info.dat <= std_logic_vector(to_unsigned(g_cores,32));
            when 1 => s_clu_info.dat <= std_logic_vector(to_unsigned(1,32));
            when 2 => s_clu_info.dat <= std_logic_vector(to_unsigned(g_ram_per_core*4,32));
            when 3 => s_clu_info.dat <= std_logic_vector(to_unsigned(f_bool2int(g_is_dm),32));
             -- unmapped addresses return error
            when others =>  s_clu_info.ack <= '0';
                            s_clu_info.err <= '1';
           end case;
        end if;
      end if;
    end if;
  end process;

  clu_cb_masterport_in(c_clu_info_rom) <= s_clu_info;

  
  sync_individual_resets : process(clk_ref_i)
  begin
    -- no need to sync vector, just individual bits.
    -- CPU's shouldn't rely on simultaneous reset anyway
    if rising_edge(clk_ref_i) then
       r_rst_lm32_n0 <= rst_lm32_n_i;
       r_rst_lm32_n1 <= r_rst_lm32_n0;
    end if;
  end process;  

  s_rst_lm32_n <= r_rst_lm32_n1;

end architecture rtl;
