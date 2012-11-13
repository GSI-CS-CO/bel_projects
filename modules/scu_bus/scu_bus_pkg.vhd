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

component scu_bus_slave_v2r1
generic	(
		CLK_in_Hz:			integer;
		Firmware_Release:	integer;
		Firmware_Version:	integer;
		Hardware_Release:	integer;
		Hardware_Version:	integer;
		Intr_Edge_Trig:		std_logic_vector(14 downto 0);
		Intr_Enable:		std_logic_vector(14 downto 0);
		Intr_Level_Neg:		std_logic_vector(14 downto 0);
		Slave_ID:			integer
		);
port	(
		nSCUB_Timing_Cyc:	in		std_logic;
		nSCUB_Slave_Sel:	in		std_logic;
		nSCUB_DS:			in		std_logic;
		SCUB_RDnWR:			in		std_logic;
		clk:				in		std_logic;
		nReset:				in		std_logic;
		Dtack_to_SCUB:		in		std_logic;
		User_Ready:			in		std_logic;
		Data_to_SCUB:		in		std_logic_vector(15 downto 0);
		Intr_In:			in		std_logic_vector(15 downto 1);
		SCUB_Addr:			in		std_logic_vector(15 downto 0);
		SCUB_Data:			inout	std_logic_vector(15 downto 0);
		Timing_Pattern_RCV:	out		std_logic;
		nSCUB_Dtack_Opdrn:	out		std_logic;
		SCUB_Dtack:			out		std_logic;
		nSCUB_SRQ_Opdrn:	out		std_logic;
		SCUB_SRQ:			out		std_logic;
		nSel_Ext_Data_Drv:	out		std_logic;
		Ext_Data_Drv_Rd:	out		std_logic;
		Standard_Reg_Acc:	out		std_logic;
		Ext_Adr_Val:		out		std_logic;
		Ext_Rd_active:		out		std_logic;
		Ext_Rd_fin:			out		std_logic;
		Ext_Rd_Fin_ovl:		out		std_logic;
		Ext_Wr_active:		out		std_logic;
		Ext_Wr_fin:			out		std_logic;
		Ext_Wr_fin_ovl:		out		std_logic;
		nPowerup_Res:		out		std_logic;
		ADR_from_SCUB_LA:	out		std_logic_vector(15 downto 0);
		Data_from_SCUB_LA:	out		std_logic_vector(15 downto 0);
		Timing_Pattern_LA:	out		std_logic_vector(31 downto 0)
		);
end component;

component SCU_Slave_FG_V2R2 is
generic	(
		sys_clk_in_hz:	natural	:= 100_000_000;
		base_addr:		natural := 16#100#
		);
port	(
		fg_clk:				in		std_logic;							-- attention, fg_clk is 8.192 Mhz
		ext_clk:			in		std_logic;							--
		sys_clk:			in		std_logic;							-- should be the same clk, used by SCU_Bus_Slave
		ADR_from_SCUB_LA:	in		std_logic_vector(15 DOWNTO 0);		-- latched address from SCU_Bus
		Ext_Adr_Val:		in		std_logic;							-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:		in		std_logic;							-- '1' => Rd-Cycle is active
		Ext_Rd_fin:			in		std_logic;							-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:		in		std_logic;							-- '1' => Wr-Cycle is active
		Ext_Wr_fin:			in		std_logic;							-- marks end of write cycle, active one for one clock period of sys_clk
		Data_from_SCUB_LA:	in		std_logic_vector(15 DOWNTO 0);		-- latched data from SCU_Bus 
		nPowerup_Res:		in		std_logic;							-- '0' => the FPGA make a powerup
		nStartSignal:		in		std_logic;							-- '0' => started FG if broadcast_en = '1'
		Data_to_SCUB:		out		std_logic_vector(15 DOWNTO 0);		-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		out		std_logic;							-- connect Dtack to SCUB-Macro
		dreq:				out		std_logic;							-- data request interrupt for new SW3
		read_cycle_act:		out		std_logic;							-- this macro has data for the SCU-Bus
		sw_out:				out		std_logic_vector(31 downto 0);		-- 
		new_data_strobe:	out		std_logic;							-- sw_out changed
		data_point_strobe:	out		std_logic							-- data point reached
		);
end component;

end package;