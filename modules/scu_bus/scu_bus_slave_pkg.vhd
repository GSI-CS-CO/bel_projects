library IEEE;
use IEEE.STD_LOGIC_1164.all;

library work;

package scu_bus_slave_pkg is

component SCU_Bus_Slave

generic
    (
    CLK_in_Hz:        integer := 100_000_000;             -- frequency of the "SCU_Bus_Slave" clock in Hz,
                                                          -- should be higher then 100 Mhz
    Slave_ID:         integer range 0 TO 16#FFFF# := 0;   -- ID of the realisied slave board function
    Firmware_Version: integer range 0 to 16#FFFF# := 0;
    Firmware_Release: integer range 0 to 16#FFFF# := 0;
    Hardware_Version: integer range 0 to 16#FFFF# := 0;
    Hardware_Release: integer range 0 to 16#FFFF# := 0;

    -- the bit positions are corresponding to Intr_In.
    -- A '1' set default level of this Intr_In(n) to neg. level or neg. edge
    Intr_Level_Neg:   std_logic_vector(15 DOWNTO 1) := B"0000_0000_0000_000";

    -- the bit positions are corresponding to Intr_In.
    -- A '1' set default of this Intr_In(n) to edge triggered, '0' is level triggered
    Intr_Edge_Trig:   std_logic_vector(15 DOWNTO 1) := B"1111_1111_1111_110";
    
    -- the bit positions are corresponding to Intr_In.
    -- A '1' enable Intr_In(n), '0' disable Intr_In(n)
    Intr_Enable:      std_logic_vector(15 DOWNTO 1) := B"0000_0000_0000_001";
                                                                              
    -- change only here! increment by major changes of this macro
    This_macro_vers_dont_change_from_outside: integer range 0 to 16#FF# := 3;
    
    -- change only here! increment by minor changes of this macro
    This_macro_revi_dont_change_from_outside: integer range 0 to 16#FF# := 0
    );
port
    (
    SCUB_Addr:          in    std_logic_vector(15 DOWNTO 0);  -- SCU_Bus: address bus
    nSCUB_Timing_Cyc:   in    std_logic;                      -- SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data:          inout std_logic_vector(15 DOWNTO 0);  -- SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel:    in    std_logic;                      -- SCU_Bus: '0' => SCU master select slave
    nSCUB_DS:           in    std_logic;                      -- SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR:         in    std_logic;                      -- SCU_Bus: '1' => SCU master read slave
    clk:                in    std_logic;                      -- clock of "SCU_Bus_Slave"
    nSCUB_Reset_in:     in    std_logic;                      -- SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB:       in    std_logic_vector(15 DOWNTO 0);  -- connect read sources from external user functions
    Dtack_to_SCUB:      in    std_logic;                      -- connect Dtack from from external user functions

    -- 15 interrupts from external user functions
    Intr_In:            in    std_logic_vector(15 DOWNTO 1) := NOT Intr_Level_Neg(15 Downto 1);
    
    -- '1' => the user function(s), device, is ready to work with the control system
    User_Ready:         in    std_logic;

    -- latched data from SCU_Bus for external user functions
    Data_from_SCUB_LA:  out   std_logic_vector(15 DOWNTO 0);

    -- latched address from SCU_Bus for external user functions
    ADR_from_SCUB_LA:   out   std_logic_vector(15 DOWNTO 0);

    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_LA:  out   std_logic_vector(31 DOWNTO 0);

    Timing_Pattern_RCV: out   std_logic;    -- timing pattern received
    nSCUB_Dtack_Opdrn:  out   std_logic;    -- for direct connect to SCU_Bus opendrain signal - '0' => slave give
                                            -- dtack to SCU master
    SCUB_Dtack:         out   std_logic;    -- for connect via ext. open collector driver - '1' => slave give
                                            -- dtack to SCU master
    nSCUB_SRQ_Opdrn:    out   std_logic;    -- for direct connect to SCU_Bus opendrain signal - '0' => slave
                                            -- service request to SCU master
    SCUB_SRQ:           out   std_logic;    -- for connect via ext. open collector driver - '1' => slave
                                            -- service request to SCU master
    nSel_Ext_Data_Drv:  out   std_logic;    -- '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd:    out   std_logic;    -- '1' => direction of the external data driver on the SCU_Bus slave
                                            -- is to the SCU_Bus
    Standard_Reg_Acc:   out   std_logic;    -- '1' => mark the access to register of this macro
    Ext_Adr_Val:        out   std_logic;    -- for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      out   std_logic;    -- '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin:         out   std_logic;    -- marks end of read cycle, active one for one clock period
                                            -- of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl:     out   std_logic;    -- marks end of read cycle, active one for one clock period
                                            -- of clk during cycle end (overlap)
    Ext_Wr_active:      out   std_logic;    -- '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin:         out   std_logic;    -- marks end of write cycle, active one for one clock period
                                            -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl:     out   std_logic;    -- marks end of write cycle, active one for one clock period
                                            -- of clk during cycle end (overlap)
    Deb_SCUB_Reset_out: out   std_logic;    -- the debounced SCU-Bus signal 'nSCUB_Reset_In'. Use for other macros.
    nPowerup_Res:       out   std_logic     -- '0' => the FPGA make a powerup
    );
end component;

end package scu_bus_slave_pkg;