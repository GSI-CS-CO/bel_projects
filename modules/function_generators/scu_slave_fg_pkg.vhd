library IEEE;
use IEEE.STD_LOGIC_1164.all;

library work;

package scu_slave_fg_pkg is

component scu_slave_fg is
generic	
    (
		sys_clk_in_hz:  		         	    natural	:= 100_000_000;
		broadcast_start_addr:	            natural	:= 16#1030#;
		base_addr:    				            natural := 16#100#;
		This_macro_vers_dont_change_from_outside: integer	range 0 to 2**3-1 := 3;	-- only change here! increment by major changes of this macro
		This_macro_revi_dont_change_from_outside: integer	range 0 to 2**4-1 := 0	-- only change here! increment by minor changes of this macro
		);
port
    (
		fg_clk:				      in		std_logic;					      		-- attention, fg_clk is 8.192 Mhz
		ext_clk:			      in		std_logic;						      	--
		sys_clk:		    	  in		std_logic;				  		  	  -- should be the same clk, used by SCU_Bus_Slave
		ADR_from_SCUB_LA:	  in		std_logic_vector(15 DOWNTO 0);-- latched address from SCU_Bus
		Ext_Adr_Val:		    in		std_logic;				    	  		-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:		  in		std_logic;						      	-- '1' => Rd-Cycle is active
		Ext_Rd_fin:		  	  in		std_logic;					      		-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:		  in		std_logic;			    			  	-- '1' => Wr-Cycle is active
		Ext_Wr_fin:			    in		std_logic;							      -- marks end of write cycle, active one for one clock period of sys_clk
		Data_from_SCUB_LA:	in		std_logic_vector(15 DOWNTO 0);-- latched data from SCU_Bus 
		nPowerup_Res:		    in		std_logic;				       			-- '0' => the FPGA make a powerup
		nStartSignal:		    in		std_logic;					  	    	-- '0' => started FG if broadcast_en = '1'
		Data_to_SCUB:		    out		std_logic_vector(15 DOWNTO 0);-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		  out		std_logic;	    					  	-- connect Dtack to SCUB-Macro
		dreq:				        out		std_logic;			    				  -- data request interrupt for new SW3
		read_cycle_act:		  out		std_logic;					      		-- this macro has data for the SCU-Bus
		sw_out:				      out		std_logic_vector(31 downto 0);-- 
		new_data_strobe:	  out		std_logic;						      	-- sw_out changed
		data_point_strobe:	out		std_logic						    	    -- data point reached
		);
end component;

end package scu_slave_fg_pkg;