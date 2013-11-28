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
        
        irq_master_o        : out t_wishbone_master_out;
        irq_master_i        : in t_wishbone_master_in;
        
        scu_slave_o         : buffer t_wishbone_slave_out;
        scu_slave_i         : in t_wishbone_slave_in;
        
        scub_data           : inout std_logic_vector(15 downto 0);
        nscub_ds            : out std_logic;
        nscub_dtack         : in std_logic;
        scub_addr           : out std_logic_vector(15 downto 0);
        scub_rdnwr          : out std_logic;
        nscub_srq_slaves    : in std_logic_vector(11 downto 0);
        nscub_slave_sel     : out std_logic_vector(11 downto 0);
        nscub_timing_cycle  : out std_logic;
        nsel_ext_data_drv   : out std_logic);
end entity;


architecture wb_irq_scu_bus_arch of wb_irq_scu_bus is
  signal s_int      : std_logic_vector(1 downto 0);
  signal s_int_edge : std_logic;
begin
  scub_master : wb_scu_bus 
    generic map(
      g_interface_mode      => g_interface_mode,
      g_address_granularity => g_address_granularity,
      CLK_in_Hz             => 62_500_000,
      Test                  => 0,
      Time_Out_in_ns        => 350)
   port map(
     clk     =>   clk_i,
     nrst    =>   rst_n_i,
     slave_i =>   scu_slave_i,
     slave_o =>   scu_slave_o,
     
     SCUB_Data          => scub_data,
     nSCUB_DS           => nscub_ds,
     nSCUB_Dtack        => nscub_dtack,
     SCUB_Addr          => scub_addr,
     SCUB_RDnWR         => scub_rdnwr,
     nSCUB_SRQ_Slaves   => nscub_srq_slaves,
     nSCUB_Slave_Sel    => nscub_slave_sel,
     nSCUB_Timing_Cycle => nscub_timing_cycle,
     nSel_Ext_Data_Drv  => nsel_ext_data_drv);
     
  edge: process (clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      s_int <= "00";
    elsif rising_edge(clk_i) then
      s_int(0) <= scu_slave_o.int;
      s_int(1) <= s_int(0);
    end if;
  end process;
  
  s_int_edge <= not s_int(1) and s_int(0);
     
--  irq_master: wb_irq_master
--    port map (
--          clk_i   => clk_i,
--          rst_n_i => rst_n_i,
--          
--          master_o  => irq_master_o,
--          master_i  => irq_master_i,
--          
--          irq_i     => s_int_edge,
--          adr_i     => x"00000104",
--          msg_i     => x"DEADBEEF");


end architecture;
