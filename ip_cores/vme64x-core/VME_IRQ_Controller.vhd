--_________________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--_________________________________________________________________________________________
-- File:                      VME_IRQ_Controller.vhd
--_________________________________________________________________________________________
-- Description:
-- This block acts as Interrupter. Phases of an interrupt cycle:
-- 1) The Interrupt Controller receives an interrupt request by the WB bus; 
--    this request is a pulse on the INT_Req input 
-- 2) The Interrupt Controller asserts ('0') one of the 7 VME_IRQ lines; --> request of a service.
--    The Interrupt priority is specificated by the Master writing the INT_Level register 
--    in the CR/CSR space
-- 3) The Interrupter Controller wait for the falling edge on the VME_IACKIN line.
-- 4) When detects VME_IACKIN_n_i = '0' and the Interrupt Handler initiates the Interrupt 
--    cycle by asserting AS,the Interrupt Controller check if it is the responding interrupter.
--    Indeed before responding to an interrupt acknowledge cycle the interrupter shall have 
--    an interrupt request pending, shall check if the level of that request match the level 
--    indicated on the address lines A1, A2 and A3,the data transfer width during the interrupt 
--    acknowledge cycle should be equal or greater than the size the it can respond with, and 
--    it shall receive a falling edge on its IACKIN*.
-- 5) If it is the responding interrupter should send the source/ID on the VME_DATA lines 
--    (in our case the source/ID is the INT_Vector that the Master can write in the corresponding 
--    register in the CR/CSR space) and it terminates the interrupt cycle with an acknowledge before 
--    releasing the IRQ lines. If it isn't the responding interrupter, it should pass a falling edge on 
--    down the daisy-chain so other interrupters can respond.
--     
-- All the output signals are registered   
-- To implement the 5 phases before mentioned the follow FSM has been implemented:

--	     __________
--	 |--| IACKOUT2 |<-|
--  |  |__________|  |
--  |                |
--  |    _________   |  _________     _________     _________              
--  |-->|  IDLE   |--->|  IRQ    |-->| WAIT_AS |-->| WAIT_DS |---------------->|        
--      |_________|    |_________|   |_________|   |_________|                 | 
--         |             |                                                     |
--         |             |                       _________      _________      |
--         |             |---------<------------| IACKOUT1| <--| CHECK   |<----|
--         |                                    |_________|    |_________|     
--         |                     __________     __________         |
--         |--<-----------------|  DTACK   |<--| DATA_OUT |---<----|
--                              |__________|   |__________|   
--
-- The interrupter wait the IACKIN falling edge in the IRQ state, so if the interrupter
-- don't have interrupt pending for sure it will not respond because it is in IDLE.
-- If the slave module does not have an interrupt pending (IDLE state) and it receives
-- a falling edge on the IACKIN, it shall pass the falling edge through the daisy chain.
-- To obtain this the IACKOUT2 state has been added.
-- Time constraint:
--                      
--  Time constraint n° 35:
--       Clk   _____       _____       _____       _____       _____       _____      
--       _____|     |_____|     |_____|     |_____|     |_____|     |_____|     |_____     
--  VME_AS1_n_i   ____________________________________________________________________
--       ________|
--       VME_AS_n_i                                ___________________________________
--       _________________________________________|
--       s_AS_RisingEdge                                       ___________
--       _____________________________________________________|           |___________
--      s_IACKOUT ____________________________________________________________________
--       ________|          
-- VME_IACKOUT_o  ____________________________________________________________________
--       ________|
--
--       _________________________________________________________________  __________
--                                             IACKOUT 1/2                \/ IDLE/IRQ
--       -----------------------------------------------------------------/\----------
--
--  To respect the time constraint indicated with the number 35 fig. 55 pag. 183 in the
--  "VMEbus Specification" ANSI/IEEE STD1014-1987, is necessary to generate the VME_AS1_n_i 
--  signal which is the AS signal not sampled, and assign this signal to the s_IACKOUT 
--  signal when the fsm is in the IACKOUTx state.
--
--  The LWORD* input is not used now, since this is a D08(O) Interrupter (see Table 31
--  page 157 VMEbus specification).
--  Since this is a D08 interrupter we do not need to monitor the LWORD* and DS1* lines
--  and the Vector (1 byte) is outputted in the D00-D07 data lines. 
--____________________________________________________________________________________
-- Authors:       
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                                                          
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date          11/2012                                                                           
-- Version       v0.03  
--_____________________________________________________________________________________
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
---------------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;
use work.xvme64x_pack.all;
--===========================================================================
-- Entity declaration
--===========================================================================
entity VME_IRQ_Controller is
   Port ( clk_i            : in   std_logic;
          reset_n_i        : in   std_logic;  
          VME_IACKIN_n_i   : in   std_logic;
          VME_AS_n_i       : in   std_logic;
			 VME_AS1_n_i      : in   std_logic;  -- this is the AS* not triple sampled
          VME_DS_n_i       : in   std_logic_vector (1 downto 0);
          VME_LWORD_n_i    : in   std_logic;
          VME_ADDR_123_i   : in   std_logic_vector (2 downto 0);
          INT_Level_i      : in   std_logic_vector (7 downto 0);
          INT_Vector_i     : in   std_logic_vector (7 downto 0);
          INT_Req_i        : in   std_logic;
          VME_IRQ_n_o      : out  std_logic_vector (6 downto 0);
          VME_IACKOUT_n_o  : out  std_logic;
          VME_DTACK_n_o    : out  std_logic;
          VME_DTACK_OE_o   : out  std_logic;
          VME_DATA_o       : out  std_logic_vector (31 downto 0);
--        VME_DATA_DIR_o   : out  std_logic);
          VME_BUFFER_o     : out  t_VME_BUFFER);
end VME_IRQ_Controller;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_IRQ_Controller is
--input signals
   signal s_INT_Req_sample          : std_logic;
--output signals
	signal s_DTACK_OE_o              : std_logic;
	signal s_enable                  : std_logic;
   signal s_IRQ                     : std_logic_vector(6 downto 0);
   signal s_Data                    : std_logic_vector(31 downto 0);
--
   signal s_AS_FallingEdge          : std_logic;
	signal s_AS_RisingEdge           : std_logic;     
   type t_MainFSM is (IDLE, IRQ, WAIT_AS, WAIT_DS, LATCH_DS, CHECK, DATA_OUT, DTACK,IACKOUT1,IACKOUT2);
   signal s_currs, s_nexts          : t_MainFSM;
   signal s_ack_int                 : std_logic;
   signal s_VME_ADDR_123_latched    : std_logic_vector(2 downto 0);
   signal s_VME_DS_latched          : std_logic_vector(1 downto 0);
   signal s_ADDRmatch               : std_logic;
	signal s_FSM_IRQ                 : t_FSM_IRQ;
--===========================================================================
-- Architecture begin
--===========================================================================
begin

-- Input sampling and edge detection
   ASrisingEdge : RisEdgeDetection
      port map (
               sig_i      => VME_AS_n_i,
               clk_i      => clk_i,
               RisEdge_o  => s_AS_RisingEdge
             );
				 
   ASfallingEdge : FallingEdgeDetection
      port map (
               sig_i      => VME_AS_n_i,
               clk_i      => clk_i,
               FallEdge_o => s_AS_FallingEdge
             );

   INT_ReqinputSample : process(clk_i)
	begin
		if rising_edge(clk_i) then
		 if s_enable = '1' then	
			s_INT_Req_sample <= INT_Req_i;
		 end if;
		end if;	
	end process;
	
--Output registers:
   DTACKOutputSample : process(clk_i)
	begin
		if rising_edge(clk_i) then	 
			VME_DTACK_n_o <= s_FSM_IRQ.s_DTACK;
		end if;	
	end process;

   DataDirOutputSample : process(clk_i)
	begin
		if rising_edge(clk_i) then	 
			--VME_DATA_DIR_o <= s_FSM_IRQ.s_DataDir;
		end if;	
	end process; 
	
   DTACKOEOutputSample : process(clk_i)
	begin
		if rising_edge(clk_i) then	 
			s_DTACK_OE_o <= s_FSM_IRQ.s_DTACK_OE;
		end if;	
	end process;

   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if s_FSM_IRQ.s_resetIRQ = '1' then
            VME_IRQ_n_o <= (others => '1');
         elsif s_FSM_IRQ.s_enableIRQ = '1' then	 
            VME_IRQ_n_o <= s_IRQ; 
         end if;
      end if;	 
   end process;		

   process(clk_i)
   begin
      if rising_edge(clk_i) then
         VME_DATA_o <= s_Data; 
      end if;
   end process;		

-- Update current state
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if reset_n_i = '0' then
            s_currs <= IDLE;
         else
            s_currs <= s_nexts;
         end if;	
     end if;
  end process;		
-- Update next state
  process(s_currs,s_INT_Req_sample,VME_AS_n_i,VME_DS_n_i,s_ack_int,VME_IACKIN_n_i,s_AS_RisingEdge)
  begin
    case s_currs is 
      when IDLE =>
         if s_INT_Req_sample = '1' and VME_IACKIN_n_i = '1' then
            s_nexts <= IRQ;
         elsif VME_IACKIN_n_i = '0' then
            s_nexts <= IACKOUT2;
			else
			   s_nexts <= IDLE;
         end if;

      when IRQ => 
         if VME_IACKIN_n_i = '0' then  -- Each Interrupter who is driving an interrupt request line
                                       -- low waits for a falling edge on IACKIN input -->
                                       -- the IRQ_Controller have to detect a falling edge on the IACKIN.
            s_nexts <= WAIT_AS;
         else 
            s_nexts <= IRQ;
         end if;

      when WAIT_AS =>
         if VME_AS_n_i = '0' then  -- NOT USE FALLING EDGE HERE!
            s_nexts <= WAIT_DS;
         else 
            s_nexts <= WAIT_AS;
         end if;

      when WAIT_DS =>
         if VME_DS_n_i /= "11" then
            s_nexts <= CHECK;
         else 
            s_nexts <= WAIT_DS;
         end if;
--      when LATCH_DS =>	      -- this state is necessary only for D16 ans D32 Interrupters
--         s_nexts <= CHECK;
-- If the interrupter is D16 or D32 add a generic number of LATCH_DS state like in the VME_bus component.
      when CHECK =>	  
         if s_ack_int = '1' then
            s_nexts <= DATA_OUT;  -- The Interrupter send the INT_Vector
         else 
            s_nexts <= IACKOUT1;   -- the Interrupter must pass a falling edge on the IACKOUT output
         end if;

      when IACKOUT1 =>	 
		   if s_AS_RisingEdge = '1' then  
            s_nexts <= IRQ;
         else 
            s_nexts <= IACKOUT1;
         end if;	
          
			
      when  DATA_OUT=>	  
         s_nexts <= DTACK;	 
      
		when IACKOUT2 =>	
         if s_AS_RisingEdge = '1' then  
            s_nexts <= IDLE;
         else 
            s_nexts <= IACKOUT2;
         end if;			         
      	
      when  DTACK=>	
         if s_AS_RisingEdge = '1' then  
            s_nexts <= IDLE;
         else 
            s_nexts <= DTACK;
         end if;		 	  
      when others => null;
    end case;

  end process;
-- Update Outputs
-- Mealy FSM
  process(s_currs,VME_AS1_n_i)
  begin
	 s_FSM_IRQ   <= c_FSM_IRQ;
	 
    case s_currs is 
      when IDLE =>
		    s_FSM_IRQ   <= c_FSM_IRQ;
      
       when IRQ => 
		    s_FSM_IRQ             <= c_FSM_IRQ;
			 s_FSM_IRQ.s_enableIRQ <= '1';
			 s_FSM_IRQ.s_resetIRQ  <= '0';

      when WAIT_AS =>
		    s_FSM_IRQ             <= c_FSM_IRQ;
		    s_FSM_IRQ.s_resetIRQ  <= '0';
			 
      when WAIT_DS =>
		    s_FSM_IRQ             <= c_FSM_IRQ;
			 s_FSM_IRQ.s_resetIRQ  <= '0';
			 
--      when LATCH_DS =>	  
--          s_IACKOUT   <= '1';
--          s_DataDir   <= '0'; 
--          s_DTACK     <= '1';
--          s_enableIRQ <= '0';
--          s_resetIRQ  <= '0';
--          s_DSlatch   <= '1';
--          s_DTACK_OE  <= '0';

      when CHECK =>	
          s_FSM_IRQ             <= c_FSM_IRQ;
			 s_FSM_IRQ.s_resetIRQ  <= '0';	

      when  IACKOUT1 =>
		    s_FSM_IRQ             <= c_FSM_IRQ;
			 s_FSM_IRQ.s_resetIRQ  <= '0';
		    s_FSM_IRQ.s_IACKOUT   <= VME_AS1_n_i; 
			 
		when  IACKOUT2 =>
		    s_FSM_IRQ             <= c_FSM_IRQ;
			 s_FSM_IRQ.s_resetIRQ  <= '0';
		    s_FSM_IRQ.s_IACKOUT   <= VME_AS1_n_i; 
          
      when  DATA_OUT=>	  
		    s_FSM_IRQ             <= c_FSM_IRQ;
		    --s_FSM_IRQ.s_DataDir   <= '1';
			 s_FSM_IRQ.s_resetIRQ  <= '0';
			 s_FSM_IRQ.s_DTACK_OE   <= '1';

      when  DTACK=>	
	     	 s_FSM_IRQ             <= c_FSM_IRQ;
			 --s_FSM_IRQ.s_DataDir   <= '1';
			 s_FSM_IRQ.s_DTACK     <= '0';
			 s_FSM_IRQ.s_DTACK_OE   <= '1';
			 		
      when others => null;
    end case;
  end process;

-- This process provides the IRQ vector
  process(INT_Level_i)
  begin
    case (INT_Level_i) is
      when "00000001" => s_IRQ <= "1111110";
      when "00000010" => s_IRQ <= "1111101";
      when "00000011" => s_IRQ <= "1111011";
      when "00000100" => s_IRQ <= "1110111";
      when "00000101" => s_IRQ <= "1101111";
      when "00000110" => s_IRQ <= "1011111";
      when "00000111" => s_IRQ <= "0111111";
      when others     => s_IRQ <= "1111111";
    end case;
  end process;

-- This process sampling the address lines on AS falling edge
  process(clk_i)
  begin
    if rising_edge(clk_i) then
      if reset_n_i = '0' then 
         s_VME_ADDR_123_latched <= (others => '0');
      elsif s_AS_FallingEdge = '1' then  
         s_VME_ADDR_123_latched <= VME_ADDR_123_i;
      end if;	
    end if;
  end process;	

-- Data strobo latch 
  process(clk_i)
  begin
    if rising_edge(clk_i) then
      if reset_n_i = '0' then 
         s_VME_DS_latched <= (others => '0');
      elsif s_FSM_IRQ.s_DSlatch = '1' then  
         s_VME_DS_latched <= VME_DS_n_i;
      end if;	
    end if;
  end process;	

--This process check the A01 A02 A03:
  process(clk_i)
  begin
    if rising_edge(clk_i) then
      if reset_n_i = '0' then 
         s_ADDRmatch <= '0';
      elsif unsigned(INT_Level_i) = unsigned(s_VME_ADDR_123_latched) then  
         s_ADDRmatch <= '1';
		else 	
		   s_ADDRmatch <= '0';
      end if;	
    end if;
  end process;	
  s_ack_int <= s_ADDRmatch;  --D08 Interrupter
  -- s_ack_int <= (not(s_VME_DS_latched(1))) and s_ADDRmatch and (not(VME_LWORD_n_i)) 
  -- for a D32 Interrupter
     
  s_Data <= x"000000" & INT_Vector_i;  
  s_enable <= (not s_INT_Req_sample) or ((not s_FSM_IRQ.s_DTACK) and (s_AS_RisingEdge)); 
  -- the INT_Vector is in the D0:D7 lines (byte3 in big endian order)  
  VME_DTACK_OE_o  <= s_DTACK_OE_o;
  VME_IACKOUT_n_o <= s_FSM_IRQ.s_IACKOUT;
end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================

