-- Copyright (C) 1991-2011 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files from any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, Altera MegaCore Function License 
-- Agreement, or other applicable license agreement, including, 
-- without limitation, that your use is for the sole purpose of 
-- programming logic devices manufactured by Altera and sold by 
-- Altera or its authorized distributors.  Please refer to the 
-- applicable agreement for further details.

-- ***************************************************************************
-- This file contains a Vhdl test bench template that is freely editable to   
-- suit user's needs .Comments are provided in each section to help the user  
-- fill out necessary details.                                                
-- ***************************************************************************
-- Generated on "07/23/2012 09:59:09"
                                                            
-- Vhdl Test Bench template for design  :  scu_bus_master
-- 
-- Simulation tool : ModelSim-Altera (VHDL)
-- 

LIBRARY ieee;                                               
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;
use work.scu_bus_pkg.all;                           

ENTITY scu_bus_master_vhd_tst IS
END scu_bus_master_vhd_tst;
ARCHITECTURE scu_bus_master_arch OF scu_bus_master_vhd_tst IS
-- constants                                                 
-- signals                                                   
SIGNAL clk : STD_LOGIC := '0';
SIGNAL nrst : STD_LOGIC := '0';
SIGNAL nSCUB_DS : STD_LOGIC;
SIGNAL nSCUB_Dtack : STD_LOGIC := '1';
SIGNAL nSCUB_Slave_Sel : STD_LOGIC_VECTOR(11 DOWNTO 0);
SIGNAL nSCUB_SRQ_Slaves : STD_LOGIC_VECTOR(11 DOWNTO 0) := "110111111111";
SIGNAL nSCUB_Timing_Cycle : STD_LOGIC;
SIGNAL nSel_Ext_Data_Drv : STD_LOGIC;
SIGNAL SCUB_Addr : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL SCUB_Data : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL SCUB_RDnWR : STD_LOGIC;

signal slave_i: t_wishbone_slave_in;
signal slave_o: t_wishbone_slave_out;

type data_reg_array is array (0 to 15) of std_logic_vector(15 downto 0);
signal reg_block : data_reg_array;



component WBDebugMaster is
  generic(
    g_data_width  : integer := 32;
    g_burst       : integer := 1;
    g_addr_start  : unsigned(31 downto 0) := x"00000000";
    g_addr_end    : unsigned(31 downto 0) := x"00000010"
    );
  port(
    -- Common wishbone signals
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    -- Slave control port
    master_i     : in  t_wishbone_master_in;
    master_o     : out t_wishbone_master_out;
    -- Current timestamp
    received_o  : out std_logic_vector(35 downto 0)
    );
end component;

BEGIN
	i1 : scu_bus_master
	generic map (
    CLK_in_Hz => 125_000_000,
    Test => 0
  )
	PORT MAP (
-- list connections between master ports and signals
	clk => clk,
	nrst => nrst,
	nSCUB_DS => nSCUB_DS,
	nSCUB_Dtack => nSCUB_Dtack,
	nSCUB_Slave_Sel => nSCUB_Slave_Sel,
	nSCUB_SRQ_Slaves => nSCUB_SRQ_Slaves,
	nSCUB_Timing_Cycle => nSCUB_Timing_Cycle,
	nSel_Ext_Data_Drv => nSel_Ext_Data_Drv,
	SCUB_Addr => SCUB_Addr,
	SCUB_Data => SCUB_Data,
	SCUB_RDnWR => SCUB_RDnWR,
	slave_i => slave_i,
	slave_o => slave_o
	);
	
	wb_master: WBDebugMaster
	generic map (
	   g_data_width => 32,
	   g_burst => 1,
	   g_addr_start => x"00080000",
	   g_addr_end => x"0008000f"
	)
	port map (
	  clk_i => clk,
	  rst_n_i => nrst,
	  master_i => slave_o,
	  master_o => slave_i,
	  received_o => open
	);

init : PROCESS                                               
-- variable declarations                                     
BEGIN                                                        
        -- code that executes only once                      
WAIT;                                                       
END PROCESS init;

                                       
always : PROCESS                                              
-- optional sensitivity list                                  
-- (        )                                                 
-- variable declarations                                      
BEGIN
  --slave_i.cyc <= '0';
  --slave_i.stb <= '0';
  --slave_i.adr <= x"0000000f", x"00000002" after 9 * 8 ns;
  --slave_i.sel <= "0000";
  --slave_i.we    <= '0';
  --slave_i.dat   <= x"00000000";                                     
  nrst <= '0', '1' after 2 * 8 ns;
  --slave_i.stb <= '0', '1' after 6 * 8 ns, '0' after 7 * 8 ns, '1' after 9 * 8 ns, '0' after 10 * 8 ns;
  --slave_i.cyc <= '0', '1' after 6 * 8 ns;
  --slave_i.sel <= "0000", "0011" after 6 * 8 ns;
 
  --wait for 20 * 8 ns;
  
  --ASSERT (False)
	--REPORT "end simulation"
  --SEVERITY failure;


  
WAIT;                                                        
END PROCESS always;


scu_slave: process(clk)
variable cnt : integer := 0;
begin
  if nSCUB_DS = '0' and SCUB_RDnWR = '1' then
    SCUB_Data <= reg_block(to_integer(unsigned(SCUB_Addr)));
  elsif nSCUB_DS = '0' and SCUB_RDnWR = '0' then
    reg_block(to_integer(unsigned(SCUB_Addr))) <= SCUB_Data;
  else
    SCUB_Data <= x"ZZZZ";
  end if;
  
  if rising_edge(clk) then
    if nSCUB_DS = '0'  then
      cnt := cnt + 1;
      if (cnt = 3) then
        cnt := 0;
        nSCUB_Dtack <= '0', '1' after 2 * 8 ns;
      end if;
    end if;
  end if;
end process;
  
clk_125: Process
begin
  loop
	 clk <= NOT clk;
	 wait for 4 ns;
	end loop;
end process clk_125;

                                        
END scu_bus_master_arch;
