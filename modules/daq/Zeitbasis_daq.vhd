
-- TITLE "'zeitbasis' => Autor: R.Hartmann, Stand: 06.04.2009, Vers: V01 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.ALL;

			
entity zeitbasis_daq is
	generic
    (
    CLK_in_Hz: INTEGER := 125_000_000;
    diag_on:   INTEGER range 0 to 1 := 0
    );
	port
    (
		-- Input ports
			Res				: IN	STD_LOGIC := '0';
			Clk				: IN	STD_LOGIC;

		-- Output ports
      --	Ena_every_100ns:  OUT	STD_LOGIC;	
      --	Ena_every_166ns:  OUT	STD_LOGIC;	
      --	Ena_every_250ns:  OUT	STD_LOGIC;
      --	Ena_every_500ns:  OUT	STD_LOGIC;
      Ena_every_1us     :  out std_logic;
      Ena_every_10us    :  out std_logic;
      Ena_every_100us   :  out std_logic;
      Ena_every_1ms     :  out std_logic;
      Ena_every_250ns   :  out std_logic
      --	Ena_Every_20ms:   OUT	STD_LOGIC
    );
end zeitbasis_daq;
			
			
architecture arch_zeitbasis_daq of zeitbasis_daq is

	CONSTANT	CLK_in_ps			: INTEGER	:= (1_000_000_000 / (CLK_in_Hz / 1000));

   --CONSTANT	c_Ena_every_100ns_cnt	: INTEGER	:= 100	  * 1000 / CLK_in_ps;
   --CONSTANT	c_Ena_every_166ns_cnt	: INTEGER	:= 166	  * 1000 / CLK_in_ps;
   --CONSTANT	c_Ena_every_500ns_cnt	: INTEGER	:= 500  	  * 1000 / CLK_in_ps;
   CONSTANT	c_Ena_every_1us_cnt		: INTEGER	:= 1000 	* 1000 / CLK_in_ps;        --for real   

   CONSTANT	c_Ena_every_250ns_cnt	: INTEGER	:= 250        * 1000 / CLK_in_ps;   --for real  
   CONSTANT	c_Ena_every_10us_cnt  	: INTEGER	:= 10_000     * 1000 / CLK_in_ps;   --for real 
   CONSTANT	c_Ena_every_100us_cnt	: INTEGER	:= 100_000    * 1000 / CLK_in_ps;   --for real 
   CONSTANT	c_Ena_every_1ms_cnt	   : INTEGER	:= 1_000_000  * 1000 / CLK_in_ps;   --for real 
   --CONSTANT	c_Ena_every_250ns_cnt	: INTEGER	:= 50   	* 1000 / CLK_in_ps;   --for simulation
   --CONSTANT	c_Ena_every_10us_cnt	   : INTEGER	:= 80   	* 1000 / CLK_in_ps;   --for simulation
   --CONSTANT	c_Ena_every_100us_cnt	: INTEGER	:= 120   * 1000 / CLK_in_ps;   --for simulation
   --CONSTANT	c_Ena_every_1ms_cnt	   : INTEGER	:= 160   * 1000 / CLK_in_ps;   --for simulation
  

  
  
 	--SIGNAL	s_every_100ns	: STD_LOGIC;
	--SIGNAL	s_every_166ns	: STD_LOGIC;
	--SIGNAL	s_every_20ms	: STD_LOGIC;
	--SIGNAL	s_every_500ns	: STD_LOGIC;
	SIGNAL	s_every_1us		: STD_LOGIC;
   SIGNAL	s_every_250ns	: STD_LOGIC;
   SIGNAL	s_every_10us	: STD_LOGIC;
	SIGNAL	s_every_100us	: STD_LOGIC;
 	SIGNAL	s_every_1ms	   : STD_LOGIC; 

  

  component div_n
    generic
      (
      n:  integer := 2;
      diag_on: integer range 0 to 1 := 0
      );
    port
      (
      res:    in	std_logic := '0';
      clk:    in	std_logic;
      ena:    in	std_logic := '1';
      div_o:  out	std_logic
      );
  end component;



BEGIN

--sel_every_100ns: div_n
--  generic map (n => c_Ena_every_100ns_cnt, diag_on => diag_on)
--    port map  ( res => res,
--                clk => clk,
--                ena => '1',
--                div_o => s_every_100ns
--              );
--
--sel_every_166ns: div_n
--  generic map (n => c_Ena_every_166ns_cnt, diag_on => diag_on)
--    port map  ( res => res,
--                clk => clk,
--                ena => '1',
--                div_o => s_every_166ns
--              );
--              
--sel_every_250ns: div_n
--  generic map (n => c_Ena_every_250ns_cnt, diag_on => diag_on)
--    port map  ( res => res,
--                clk => clk,
--                ena => '1',
--                div_o => s_every_250ns
--              );
--
--sel_every_500ns: div_n
--  generic map (n => c_Ena_every_500ns_cnt, diag_on => diag_on)
--    port map  ( res => res,
--                clk => clk,
--                ena => '1',
--                div_o => s_every_500ns
--              );
--
  sel_every_1us: div_n
    generic map (n => c_Ena_every_1us_cnt, diag_on => diag_on)
      port map  ( res => res,
                  clk => clk,
                  ena => '1',
                  div_o => s_every_1us
                 );

sel_every_250ns: div_n
  generic map (n => c_Ena_every_250ns_cnt, diag_on => diag_on)
    port map  ( res => res,
                clk => clk,
                ena => '1',
                div_o => s_every_250ns
              );


sel_every_10us: div_n
  generic map (n => c_Ena_every_10us_cnt, diag_on => diag_on)
    port map  ( res => res,
                clk => clk,
                ena => '1',
                div_o => s_every_10us
              );
              
sel_every_100us: div_n
  generic map (n => c_Ena_every_100us_cnt, diag_on => diag_on)
    port map  ( res => res,
                clk => clk,
                ena => '1',
                div_o => s_every_100us
              );
sel_every_1ms: div_n
  generic map (n => c_Ena_every_1ms_cnt, diag_on => diag_on)
    port map  ( res => res,
                clk => clk,
                ena => '1',
                div_o => s_every_1ms
              );
              
              
--sel_every_20ms: div_n
--  generic map (n => integer(20.0e-3 / 1.0e-6), diag_on => diag_on)  -- ena nur jede us für einen Takt aktiv, deshalb n = 20000
--    port map  ( res => res,
--                clk => clk,
--                ena => s_every_1us,
--                div_o => s_every_20ms
--              );

	
--Ena_every_100ns	<= 	s_every_100ns;		
--Ena_every_166ns	<= 	s_every_166ns;		
--Ena_every_250ns	<= 	s_every_250ns;		
--Ena_every_500ns	<= 	s_every_500ns;		
Ena_every_1us	   <= 	s_every_1us;	
Ena_every_250ns   <=    s_every_250ns;
Ena_every_10us	   <= 	s_every_10us;	
Ena_every_100us	<= 	s_every_100us;	
Ena_every_1ms  	<= 	s_every_1ms;
--Ena_every_20ms	<= 	s_every_20ms;		

END arch_zeitbasis_daq;

