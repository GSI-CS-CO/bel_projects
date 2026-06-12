library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;

use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.scu_bus_pkg.all;
use work.scu_bus_slave_pkg.all;

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
  signal scu_srq_active        : std_logic_vector(12 downto 0);
  signal is_standalone         : std_logic;
  signal scu_slave_o_from_scub : t_wishbone_master_in;
  signal scu_slave_i_to_scub   : t_wishbone_master_out;
  signal s_scub_data           : std_logic_vector(15 downto 0);
  signal s_scub_addr           : std_logic_vector(15 downto 0);
  signal s_scub_rdnwr          : std_logic;
  signal s_nscub_ds            : std_logic;
  signal s_nscub_dtack         : std_logic;
  signal virtual_scub_srq      : std_logic;
  signal s_nscub_slave_sel     : std_logic_vector(12 downto 0);
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
     nSCUB_DS           => s_nscub_ds,
     nSCUB_Dtack        => nscub_dtack or s_nscub_dtack,
     SCUB_Addr          => s_scub_addr,
     SCUB_RDnWR         => s_scub_rdnwr,
     nSCUB_SRQ_Slaves   => virtual_scub_srq & nscub_srq_slaves,
     nSCUB_Slave_Sel    => s_nscub_slave_sel,
     nSCUB_Timing_Cycle => nscub_timing_cycle,
     nSel_Ext_Data_Drv  => nsel_ext_data_drv
   );
  
  scub_irq_master: wb_irq_master
  generic map (
    g_channels => 13,
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

  s_scub_data <= scub_data_out when scub_data_tri_out = '1' else (others => 'Z');
  scub_virtual_slave: scu_bus_slave
  generic map (
    Clk_in_Hz        => 62_500_000,
    Firmware_Release => 1,
    Firmware_Version => 1,
    CID_System       => 55,
    Intr_Enable      => b"0000_0000_0000_0001")
  port map (
    SCUB_Addr          => s_scub_addr,
    nSCUB_Timing_Cyc   => tag_valid,
    SCUB_Data          => s_scub_data,
    nSCUB_Slave_Sel    => s_nscub_slave_sel(12),
    nSCUB_DS           => s_nscub_ds,
    SCUB_RDnWR         => s_scub_rdnwr,
    clk                => clk_i,
    nSCUB_Reset_in     => rst_n_i,
    Data_to_SCUB       => x"0000",
    Dtack_to_SCUB      => '0',
    Intr_In            => b"0000_0000_0000_000",
    User_Ready         => '1',
    CID_Group          => 55,
    Data_from_SCUB_LA  => open,                -- out,   latched data from SCU_Bus for external user functions
    ADR_from_SCUB_LA   => open,                -- out,   latched address from SCU_Bus for external user functions
    Timing_Pattern_LA  => open,                -- out,   latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV => open,                -- out,   timing pattern received
    nSCUB_Dtack_Opdrn  => open,                -- out,   for direct connect to SCU_Bus opendrain signal
                                               --        '0' => slave give dtack to SCU master
    SCUB_Dtack         => s_nscub_dtack,       -- out,   for connect via ext. open collector driver
                                               --        '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn    => open,                -- out,   for direct connect to SCU_Bus opendrain signal
                                               --        '0' => slave service request to SCU ma
    SCUB_SRQ           => virtual_scub_srq,    -- out,   for connect via ext. open collector driver
                                               --        '1' => slave service request to SCU master
    nSel_Ext_Data_Drv  => open,                -- out,   '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd    => open,                -- out,   '1' => direction of the external data driver on the
                                               --        SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc   => open,                -- out,   '1' => mark the access to register of this macro
    Ext_Adr_Val        => open,                -- out,   for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active      => open,                -- out,   '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin         => open,                -- out,   marks end of read cycle, active one for one clock period
                                               --        of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl     => open,                -- out,   marks end of read cycle, active one for one clock period
                                               --        of clk during cycle end (overlap)
    Ext_Wr_active      => open,                -- out,   '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin         => open,                -- out,   marks end of write cycle, active high for one clock period
                                               --        of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl     => open,                -- out,   marks end of write cycle, active high for one clock period
                                               --        of clk before write cycle finished (with overlap)
    Deb_SCUB_Reset_out => open,                -- out,   the debounced 'nSCUB_Reset_In'-signal, is active high,
                                               --        can be used to reset
                                               --        external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res       => open,                -- out,   this macro generated a power up reset
    Powerup_Done       => open                 -- out    this memory is set to one if an Powerup is done.
                                               --        Only the SCUB-Master can clear this bit.
    );
   
  is_rmt          <= is_standalone;
  scub_addr       <= s_scub_addr;
  scub_rdnwr      <= s_scub_rdnwr;
  nscub_ds        <= s_nscub_ds;
  nscub_slave_sel <= s_nscub_slave_sel(11 downto 0);
end architecture;
