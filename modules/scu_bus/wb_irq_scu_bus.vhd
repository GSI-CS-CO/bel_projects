library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.scu_bus_pkg.all;

entity wb_irq_scu_bus is
  generic (
            g_interface_mode      : t_wishbone_interface_mode       := PIPELINED;
            g_address_granularity : t_wishbone_address_granularity  := BYTE;
            clk_in_hz             : integer := 62_500_000;
            time_out_in_ns        : integer := 250;
            test                  : integer range 0 to 1 := 0);
  port (
        clk_i               : std_logic;
        rst_n_i             : std_logic;
        
        tag                 : in std_logic_vector(31 downto 0);
        tag_valid           : in std_logic;
        
        irq_master_o        : out t_wishbone_master_out;
        irq_master_i        : in t_wishbone_master_in;

        ctrl_irq_o          : out t_wishbone_slave_out;
        ctrl_irq_i          : in t_wishbone_slave_in;
        
        scu_slave_o         : buffer t_wishbone_slave_out;
        scu_slave_i         : in t_wishbone_slave_in;
        
        scub_data_out       : out std_logic_vector(15 downto 0);
        scub_data_in        : in std_logic_vector(15 downto 0);
        scub_data_tri_out   : out std_logic;
        nscub_ds            : out std_logic;
        nscub_dtack         : in std_logic;
        scub_addr           : out std_logic_vector(15 downto 0);
        scub_rdnwr          : out std_logic;
        nscub_srq_slaves    : in std_logic_vector(11 downto 0);
        nscub_slave_sel     : out std_logic_vector(11 downto 0);
        nscub_timing_cycle  : out std_logic;
        nsel_ext_data_drv   : out std_logic;
        is_rmt              : out std_logic);
end entity;


architecture wb_irq_scu_bus_arch of wb_irq_scu_bus is
  signal scu_srq_active : std_logic_vector(11 downto 0);
  signal is_standalone : std_logic;
  signal scu_slave_o_from_scub : t_wishbone_master_in;
  signal scu_slave_i_to_scub : t_wishbone_master_out;
begin
  mx: scu_bus_mux
  port map(
    clk           => clk_i,
    rst_n_i       => rst_n_i,
    is_standalone => is_standalone,
    scu_slave_o   => scu_slave_o,
    scu_slave_i   => scu_slave_i,
    ac_output     => x"00000000",
    scu_slave_out => scu_slave_o_from_scub,
    scu_slave_in  => scu_slave_i_to_scub
  );

  scub_master : wb_scu_bus 
    generic map(
      g_interface_mode      => g_interface_mode,
      g_address_granularity => g_address_granularity,
      CLK_in_Hz             => 62_500_000,
      Test                  => 0,
      Time_Out_in_ns        => 350)
   port map(
     clk                => clk_i,
     nrst               => rst_n_i,
     Timing_In          => tag,
     Start_Timing_Cycle => tag_valid,
     slave_i            => scu_slave_i_to_scub,
     slave_o            => scu_slave_o_from_scub,
     srq_active         => scu_srq_active,
     
     SCUB_Data_Out      => scub_data_out,
     SCUB_Data_In       => scub_data_in,
     SCUB_Data_Tri_Out  => scub_data_tri_out,
     nSCUB_DS           => nscub_ds,
     nSCUB_Dtack        => nscub_dtack,
     SCUB_Addr          => scub_addr,
     SCUB_RDnWR         => scub_rdnwr,
     nSCUB_SRQ_Slaves   => nscub_srq_slaves,
     nSCUB_Slave_Sel    => nscub_slave_sel,
     nSCUB_Timing_Cycle => nscub_timing_cycle,
     nSel_Ext_Data_Drv  => nsel_ext_data_drv
   );
  
  scub_irq_master: wb_irq_master
  generic map (
    g_channels => 12,
    g_round_rb => true,
    g_det_edge => true) 
  port map (
    clk_i   => clk_i,
    rst_n_i => rst_n_i,
    
    -- msi if
    irq_master_o => irq_master_o,
    irq_master_i => irq_master_i,

    -- ctrl if
    ctrl_slave_o => ctrl_irq_o,
    ctrl_slave_i => ctrl_irq_i,

    -- irq lines
    irq_i        => scu_srq_active);                    

  scub_or_standalone: detect_backplane
  generic map (
    Clk_in_Hz      => 62_500_000,
    Time_out_in_ms => 3)
  port map (
    clk_i         => clk_i,
    rst_n_i       => rst_n_i,
    trigger       => nscub_dtack,
    is_standalone => is_standalone);

  is_rmt <= is_standalone;
end architecture;
