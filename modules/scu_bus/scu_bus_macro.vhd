library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_bus_slave_pkg.all;

entity scu_slave_macro is
  generic (
    clk_in_hz:        integer  := 100_000_000;             -- frequency of the "SCU_Bus_Slave" clock in Hz,
                                                          -- should be higher then 100 Mhz
    slave_id:         integer range 0 to 16#ffff# := 0;   -- ID of the realisied slave board function
    fw_version:       integer range 0 to 16#ffff# := 0;
    fw_release:       integer range 0 to 16#ffff# := 0;

    -- "CSCOHW" hat z.B. die "CID_System"-Kennung dezimal 55. Baugruppen anderer Gewerke-Hersteller sollten verbindlich
    -- ihre "CID_System"-Kennung eintragen. Falls keine vorhanden ist, darf der Defaultwert 0 nicht verändert werden.
    cid_system:       integer range 0 to 16#ffff# := 0

    -- Jede Baugruppe die diesen Macro verwendet sollte durch die "CID-Group" zusammen mit "CID-System" eine eindeutige
    -- Identifizierug der Hardware-Funktion und deren Revision ermöglichen. Die "CID-Group"-Nummer wird bei jeder neuen Karte
    -- oder jeder neuen Revision hochgezählt.Z.B. "CSCOHW" hat die Karte "FG900160_SCU_ADDAC1" entwickelt,
    -- für die die "CID_Group"-Nummer 0003 dezimal vergeben wurde.
    -- Eine neue Version der Baugruppe, z.B. "FG900161_SCU_ADDAC2" könnte die "CID-Group"-Nummer 0011 haben, da
    -- zwischenzeitlich  "CID-Group"-Nummern für andere Projekte (Funktionen) vergeben wurden, und die "CID-Group"-Nummer
    -- kontinuierlich hochgezählt werden soll.                                  --
    -- Falls keine verbindliche "CID-Group"-Nummer vorliegt, muss der Defaultwert 0 stehen bleiben!
    -- CID_Group: integer range 0 to 16#FFFF# := 0;
  );
  port (
    scub_addr:          in    std_logic_vector(15 downto 0);  -- SCU_Bus: address bus
    nscub_timing_cyc:   in    std_logic;                      -- SCU_Bus signal: low active SCU_Bus runs timing cycle
    scub_data:          inout std_logic_vector(15 downto 0);  -- SCU_Bus: data bus (FPGA tri state buffer)
    nscub_slave_sel:    in    std_logic;                      -- SCU_Bus: '0' => SCU master select slave
    nscub_ds:           in    std_logic;                      -- SCU_Bus: '0' => SCU master activate data strobe
    scub_rdnwr:         in    std_logic;                      -- SCU_Bus: '1' => SCU master read slave
    clk:                in    std_logic;                      -- clock of "SCU_Bus_Slave"
    nscub_reset_in:     in    std_logic;                      -- SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    scub_dtack:         out   std_logic;                      -- for connect via ext. open collector driver - '1' => slave give
                                                              -- dtack to SCU master
    scub_srq:           out   std_logic;                      -- for connect via ext. open collector driver - '1' => slave
                                                              -- service request to SCU master
    nsel_ext_data_drv:  out   std_logic;                      -- '0' => select the external data driver on the SCU_Bus slave
    ext_data_drv_rd:    out   std_logic;
    scu_master_o:       out   t_scu_local_master_o;
    scu_master_i:       in    t_scu_local_master_i;

    -- 15 interrupts from external user functions
    irq_i:              in    std_logic_vector(15 downto 1); 
    
    -- CID Group has default as 0x0000, or mapped to actual parameter as defined by hex dial in top level.
    cid_group:            in    integer range 0 to 16#ffff# := 0;
    
    -- if an extension card is connected to the slave card, than you can map cid_system of this extension
    -- (vorausgesetzt der Typ der Extension-Card ist über diese Verbindung eindeutig bestimmbar).
    extension_cid_system: in  integer range 0 to 16#ffff# := 0;
    
    -- if an extension card is connected to the slave card, than you can map cid_group of this extension
    -- (vorausgesetzt der Typ der Extension-Card ist über diese Verbindung eindeutig bestimmbar).
    extension_cid_group:  in  integer range 0 to 16#ffff# := 0;

    -- timing tag from scu bus
    tag_i:                out std_logic_vector(31 downto 0);
    tag_valid:            out   std_logic;    -- timing pattern received
    npowerup_res:         out   std_logic    -- '0' => the FPGA make a powerup
  );
end entity;


architecture arch of scu_slave_macro is
begin
  SCU_Slave: SCU_Bus_Slave
  generic map (
    CLK_in_Hz               => 125_000_000,
    Firmware_Release        => fw_version,
    Firmware_Version        => fw_release,
    CID_System              => cid_system,
    intr_Enable             => b"0000_0000_0000_0001")
  port map (
    SCUB_Addr               => scub_addr,
    nSCUB_Timing_Cyc        => nscub_timing_cyc,
    SCUB_Data               => scub_data,
    nSCUB_Slave_Sel         => nscub_slave_sel,
    nSCUB_DS                => nscub_ds,
    SCUB_RDnWR              => scub_rdnwr,
    clk                     => clk,
    nSCUB_Reset_in          => nscub_reset_in,
    Data_to_SCUB            => scu_master_i.dat,
    Dtack_to_SCUB           => scu_master_i.dtack,
    intr_in                 => irq_i,
    User_Ready              => '1',
    CID_GROUP               => cid_group,
    extension_cid_system    => extension_cid_system,                  -- in, extension card: cid_system
    extension_cid_group     => extension_cid_group,                   -- in, extension card: cid_group
    Data_from_SCUB_LA       => scu_master_o.dat,
    ADR_from_SCUB_LA        => scu_master_o.adr,
    Timing_Pattern_LA       => tag_i,
    Timing_Pattern_RCV      => tag_valid,
    nSCUB_Dtack_Opdrn       => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                      -- '0' => slave give dtack to SCU master
    SCUB_Dtack              => scub_dtack,                            -- out, for connect via ext. open collector driver
                                                                      -- '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn         => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                      -- '0' => slave service request to SCU ma
    SCUB_SRQ                => scub_srq,                              -- out, for connect via ext. open collector driver
                                                                      -- '1' => slave service request to SCU master
    nSel_Ext_Data_Drv       => nsel_ext_data_drv,                     -- out, '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd         => ext_data_drv_rd,
    Standard_Reg_Acc        => open,
    Ext_Adr_Val             => scu_master_o.adr_val,
    Ext_Rd_active           => scu_master_o.rd_act,                   -- out, '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin              => open,
    Ext_Rd_Fin_ovl          => open,
    Ext_Wr_active           => scu_master_o.wr_act,                         -- out, '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin              => open,                                  -- out, marks end of write cycle, active high for one clock period
                                                                      -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl          => open,                                  -- out, marks end of write cycle, active high for one clock period
                                                                      -- of clk before write cycle finished (with overlap)
    Deb_SCUB_Reset_out      => open,                                  -- out, the debounced 'nSCUB_Reset_in'-signal, is active high,
                                                                      -- can be used to reset
                                                                      -- external macros, when 'nSCUB_Reset_in' is '0'
    nPowerup_Res            => npowerup_res,                          -- out, this macro generates a power up reset
    Powerup_Done            => open);                                 -- out, this signal is set after powerup. Only the SCUB-Master can clear this bit.


end architecture;