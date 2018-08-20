library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- for PEXARIA5
entity prog1 is
    Port (	
--........................................
		cdone	  	: in std_logic; -- from FPGA, low during configuration
		confix	: inout std_logic; -- input from reset chip (on schematics CONFIG)
		config1	: in std_logic; -- input from reset chip (on schematics CONFIG1)
		con	  	: in std_logic_vector(5 downto 1); -- connection to/from fpga
		pgclk 	: in std_logic; -- clock from 50 MHz oscillator
--
		sel_clk	: inout std_logic_vector(3 downto 0); -- output to gbit switch
--		
		hsw	  	: in std_logic_vector(4 downto 1); -- input from hex switch
		m	  		: out std_logic_vector(4 downto 0); -- config mode to FPGA 
		fpga_res	: in std_logic; -- output to FPGA, optional reset
		pled	  	: inout std_logic_vector(5 downto 1); -- 4 leds
		nstat 	: in std_logic; -- status to/from fpga
		mres	  	: in std_logic; -- reset output to reset chip then to FPGA reconfig
		mres1	  	: in std_logic; -- reset output to reset chip then to FPGA "nres"
		pbs1 		: in std_logic -- input from push button


--.............................................................
		);
end prog1;
--
	architecture rtl of prog1 is
	signal countx 		: std_logic_vector(26 downto 0); -- counter	
---------------------------------------------------------------------------------------
	begin
--......................................................................................
-- mode ASx1 and x4 fast 		= 10010
-- mode ASx1 and x4 standard 	= 10011
		m 					<= b"10010"; -- master SPI
--		m 					<= b"01000"; -- master BPI
		sel_clk(0)		<= '1'; -- in1 to q0 SW1
		sel_clk(1)		<= '1'; -- in1 to q1 SW1
		sel_clk(2)		<= '0'; -- in0 to q0 SW2
		sel_clk(3)		<= '0'; --	in1 to q1 SW2	
--
--		confix		<= config1; -- 200ms reset pulse to FPGA
		confix		<= '1'; -- immediaty ready
--		mres				<= '1';
--.......................................................................................
--
	process(pgclk) begin
		if (rising_edge(pgclk)) then 
   			countx <= countx + 1;
		end if;
	end process;
	--
	pled(1)  <= not cdone;	-- yellow
	pled(2)  <= confix;		-- red
--	pled(1)  <= countx(22);	-- yellow
--	pled(2)  <= countx(23);		-- red
	pled(3)  <= '1'; -- countx(24); -- white
	pled(4)  <= '1'; -- countx(25); -- blue
	pled(5)  <= '1'; -- countx(26); -- green
--	
----------------------------------------------------------------------------------------
	end;	
