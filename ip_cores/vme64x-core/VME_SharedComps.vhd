--_______________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--_______________________________________________________________________________________
-- File:                       VME_SharedComps.vhd
--_______________________________________________________________________________________
-- Description: This component implements the rising and falling edge detection and the 
-- tripple and double sample entities
--_______________________________________________________________________________________
-- Authors:                                     
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
-- Version      v0.03  
--_______________________________________________________________________________________
--                               GNU LESSER GENERAL PUBLIC LICENSE                                
--                              ------------------------------------    
-- Copyright (c) 2009 - 2011 CERN                           
-- This source file is free software; you can redistribute it and/or modify it 
-- under the terms of the GNU Lesser General Public License as published by the 
-- Free Software Foundation; either version 2.1 of the License, or (at your option) 
-- any later version. This source is distributed in the hope that it will be useful, 
-- but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
-- FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for 
-- more details. You should have received a copy of the GNU Lesser General Public 
-- License along with this source; if not, download it from 
-- http://www.gnu.org/licenses/lgpl-2.1.html                     
----------------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- tripple sample sig_i signals to avoid metastable states
entity SigInputSample is
	port (
		sig_i, clk_i: in std_logic;
		sig_o: out std_logic );
end SigInputSample;

architecture RTL of SigInputSample is
	signal s_1: std_logic;
	signal s_2: std_logic;
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			s_1   <= sig_i;
			s_2   <= s_1;
			sig_o <= s_2;
		end if;
	end process;
end RTL;

-- *************************************************** 

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- double sample sig_i signals to avoid metastable states
entity DoubleSigInputSample is
	port (
		sig_i, clk_i: in std_logic;
		sig_o: out std_logic );
end DoubleSigInputSample;

architecture RTL of DoubleSigInputSample is
	signal s_1: std_logic;
--	signal s_2: std_logic;
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			s_1     <= sig_i;
			sig_o   <= s_1;
		end if;
	end process;
end RTL;
--***************************************************
library IEEE;
use IEEE.STD_LOGIC_1164.all;

entity SingleRegInputSample is 
	generic(
		width: natural:=8
		);
	port (
		reg_i: in std_logic_vector(width-1 downto 0);
		reg_o: out std_logic_vector(width-1 downto 0);
		clk_i: in std_logic 
		);
end SingleRegInputSample;

architecture RTL of SingleRegInputSample is
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			reg_o <= reg_i;		 
		end if;
	end process;
end RTL;
-- ***************************************************
--FlipFlopD
library IEEE;
use IEEE.STD_LOGIC_1164.all;
entity FlipFlopD is
	port (
		reset, sig_i, clk_i, enable: in std_logic;
		sig_o: out std_logic );
end FlipFlopD;

architecture RTL of FlipFlopD is
--	signal s_1: std_logic;
--	signal s_2: std_logic;
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
		   if reset = '1' then
			   sig_o <= '0';
			elsif enable = '1' then	
			sig_o     <= sig_i;
			--sig_o   <= s_1;
		   end if;
		end if;	
	end process;
end RTL;
--Register 32 bits
library IEEE;
use IEEE.STD_LOGIC_1164.all;
entity Reg32bit is
	port (
		reset, clk_i, enable: in std_logic;
		di : in std_logic_vector(31 downto 0);
		do: out std_logic_vector(31 downto 0)
		);
end Reg32bit;
architecture RTL of Reg32bit is
--signal s_reg : std_logic_vector(31 downto 0);
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
		   if reset = '0' then 
			     do <= (others => '0');
				  --s_reg <= (others => '0');
			elsif enable = '1' then	
			     do <= di;
			--s_reg <= di;
			end if;
		end if;	
		--do <= s_reg;
	end process;
end RTL;

--
library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- detect rising edge
entity RisEdgeDetection is
	port (
		sig_i, clk_i: in std_logic;
		RisEdge_o: out std_logic );
end RisEdgeDetection;

architecture RTL of RisEdgeDetection is
	signal s_1: std_logic;
	constant delay_c :integer := 4;
	signal cnt :integer:=0;
	type state_type is (idle,delay); 
	signal next_s : state_type :=idle; 
	
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
		
		s_1 <= sig_i;
		
--		case  next_s	is
--			when idle =>
--				if s_1 = '0' and sig_i = '1' then
--					cnt <= 0;
--					--RisEdge_o <= '1';
--					next_s <= delay;
--				else
--					RisEdge_o <= '0';
--				end if;
--				
--			when delay =>
--				
--				if(cnt = delay_c) then
--					RisEdge_o <= '1';
--					next_s <= idle;
--				else
--					cnt <= cnt +1;
--				end if;
--			end case;
			if s_1 = '0' and sig_i = '1' then
				RisEdge_o <= '1';
			else
				RisEdge_o <= '0';
			end if;
			
		end if;
	end process;
end RTL;   

-- ***************************************************   

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- detect falling edge
entity FallingEdgeDetection is
	port (
		sig_i, clk_i: in std_logic;
		FallEdge_o: out std_logic );
end FallingEdgeDetection;

architecture RTL of FallingEdgeDetection is
	signal s_1: std_logic;
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			
			s_1 <= sig_i;
			
			if s_1 = '1' and sig_i = '0' then
				FallEdge_o <= '1';
			else
				FallEdge_o <= '0';
			end if;
			
		end if;
	end process;
end RTL;

-- ***************************************************

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- give pulse (sigEdge_o) at rising and falling edge
entity EdgeDetection is
	port (
		sig_i, 
		clk_i: in std_logic;
		sigEdge_o: out std_logic
		);
end EdgeDetection;

architecture RTL of EdgeDetection is
	signal s_1: std_logic;
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			
			s_1 <= sig_i;
			
			if (s_1 = '0' and sig_i = '1') or (s_1 = '1' and sig_i = '0') then
				sigEdge_o <= '1';
			else
				sigEdge_o <= '0';
			end if;
			
		end if;
	end process;
end RTL;

-- ***************************************************

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- triple sample input register reg_i to avoid metastable states
-- and catching of transition values
entity RegInputSample is 
	generic(
		width: natural:=8
		);
	port (
		reg_i: in std_logic_vector(width-1 downto 0);
		reg_o: out std_logic_vector(width-1 downto 0);
		clk_i: in std_logic 
		);
end RegInputSample;

architecture RTL of RegInputSample is
	signal reg_1, reg_2: std_logic_vector(width-1 downto 0);
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			reg_1 <= reg_i;
			reg_2 <= reg_1;	
			reg_o <= reg_2;	 

		end if;
	end process;
end RTL;


-- ***************************************************

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- triple sample input register reg_i to avoid metastable states
-- and catching of transition values
entity DoubleRegInputSample is 
	generic(
		width: natural:=8
		);
	port (
		reg_i: in std_logic_vector(width-1 downto 0);
		reg_o: out std_logic_vector(width-1 downto 0);
		clk_i: in std_logic 
		);
end DoubleRegInputSample;

architecture RTL of DoubleRegInputSample is
	signal reg_1, reg_2: std_logic_vector(width-1 downto 0);
begin
	process(clk_i)
	begin
		if rising_edge(clk_i) then
			reg_1 <= reg_i;
			reg_o <= reg_1;		 

		end if;
	end process;
end RTL;