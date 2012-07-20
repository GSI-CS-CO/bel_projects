library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;


package scu_bus_pkg is

component scu_bus_master is

	generic(
      g_interface_mode        : t_wishbone_interface_mode      := CLASSIC;
      g_address_granularity   : t_wishbone_address_granularity := WORD;
			CLK_in_Hz			          : INTEGER := 100000000;
			Time_Out_in_ns		      : INTEGER := 250;
			dly_multicast_dt_in_ns	: INTEGER := 200;
			Sel_dly_in_ns		        : INTEGER := 30;							-- delay to the I/O pins is not included
			Sel_release_in_ns	      : INTEGER := 30;							-- delay to the I/O pins is not included
			D_Valid_to_DS_in_ns	    : INTEGER := 30;							-- delay to the I/O pins is not included
			Timing_str_in_ns	      : INTEGER := 80;							-- delay to the I/O pins is not included
			Test				            : INTEGER RANGE 0 TO 1 := 0
			);

  port(
  
    -- Wishbone
    slave_i                 : in  t_wishbone_slave_in;
    slave_o                 : out t_wishbone_slave_out;
    

    clk                     : IN		STD_LOGIC;
    nrst                    : IN		STD_LOGIC;

    SCUB_Data				        : INOUT		STD_LOGIC_VECTOR(15 DOWNTO 0);
    nSCUB_DS				        : OUT		STD_LOGIC;							        -- SCU_Bus Data Strobe, low active.
    nSCUB_Dtack				      : IN		STD_LOGIC;							        -- SCU_Bus Data Acknowledge, low active.
    SCUB_Addr				        : OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Address Bus of SCU_Bus
    SCUB_RDnWR				      : OUT		STD_LOGIC;							        -- Read/Write Signal of SCU_Bus. Read is active high.
                                                                    -- Direction seen from this marco.
    nSCUB_SRQ_Slaves		: IN		STD_LOGIC_VECTOR(11 DOWNTO 0);		  -- Input of service requests up to 12 SCU_Bus slaves, active low.
    nSCUB_Slave_Sel			: OUT		STD_LOGIC_VECTOR(11 DOWNTO 0);		  -- Output select one or more of 12 SCU_Bus slaves, active low.
    nSCUB_Timing_Cycle	: OUT		STD_LOGIC;							            -- Strobe to signal a timing cycle on SCU_Bus, active low.
    nSel_Ext_Data_Drv		: OUT		STD_LOGIC   						            -- select for external data transceiver to the SCU_Bus, active low.

    );
end component;

end package;