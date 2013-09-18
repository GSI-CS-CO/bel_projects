--_______________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--________________________________________________________________________________________
-- File:                           VME_Init.vhd
--________________________________________________________________________________________
-- Description: Read important CR data (like FUNC_ADEMs etc.) and store it locally
-- This important CR data will be used in the decoder.
--________________________________________________________________________________________
-- Authors:                                     
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
-- Version      v0.03  
--________________________________________________________________________________________
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
------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.xvme64x_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================
entity VME_Init is
   Port ( clk_i            : in    std_logic;
          RSTedge_i        : in    std_logic;
          CRAddr_i         : in    std_logic_vector (18 downto 0);
          CRdata_i         : in    std_logic_vector (7 downto 0);
          InitReadCount_o  : out   std_logic_vector (8 downto 0);
          InitInProgress_o : out   std_logic;
          BEG_USR_CR_o     : out   std_logic_vector (23 downto 0);
          END_USR_CR_o     : out   std_logic_vector (23 downto 0);
          BEG_USR_CSR_o    : out   std_logic_vector (23 downto 0);
          END_USR_CSR_o    : out   std_logic_vector (23 downto 0);
          BEG_CRAM_o       : out   std_logic_vector (23 downto 0);
          END_CRAM_o       : out   std_logic_vector (23 downto 0);
          FUNC0_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC1_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC2_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC3_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC4_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC5_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC6_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC7_ADEM_o     : out   std_logic_vector (31 downto 0);
          FUNC0_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC1_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC2_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC3_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC4_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC5_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC6_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC7_AMCAP_o    : out   std_logic_vector (63 downto 0);
          FUNC0_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC1_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC2_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC3_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC4_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC5_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC6_XAMCAP_o   : out   std_logic_vector (255 downto 0);
          FUNC7_XAMCAP_o   : out   std_logic_vector (255 downto 0));
end VME_Init;

--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_Init is

   signal s_initReadCounter : unsigned(8 downto 0);
   signal s_initState       : t_initState;
   signal s_latchCRdata     : std_logic;  -- Stores read CR data
   signal s_initInProgress  : std_logic;
   signal s_CRadd_offset    : unsigned(18 downto 0);
   signal s_CRaddr_base     : unsigned(18 downto 0);
   signal s_CRaddr          : unsigned(18 downto 0);
   signal s_latchCRdataPos  : std_logic_vector(BEG_USER_CR to FUNC_ADEM); 
-- CR image registers
   signal s_FUNC_ADEM       : t_FUNC_32b_array;
   signal s_FUNC_AMCAP      : t_FUNC_64b_array;
   signal s_FUNC_XAMCAP     : t_FUNC_256b_array;
   signal s_BEG_USER_CSR    : unsigned(23 downto 0);
   signal s_END_USER_CSR    : unsigned(23 downto 0);
   signal s_BEG_USER_CR     : unsigned(23 downto 0);
   signal s_END_USER_CR     : unsigned(23 downto 0);
   signal s_BEG_CRAM        : unsigned(23 downto 0);
   signal s_END_CRAM        : unsigned(23 downto 0); 
	
--===========================================================================
-- Architecture begin
--===========================================================================
begin
   InitReadCount_o <= std_logic_vector(s_initReadCounter);
   s_CRaddr <= unsigned(CRAddr_i);

   p_coreInit : process(clk_i)  
   begin
      if rising_edge(clk_i) then
         if RSTedge_i = '1' then
            s_initState       <= IDLE;
            s_initReadCounter <= to_unsigned(0, s_initReadCounter'length);
            s_latchCRdata     <= '0';

         else
            case s_initState is
               when IDLE =>
                  s_initReadCounter <= to_unsigned(0, s_initReadCounter'length);
                  s_latchCRdata     <= '0';
                  s_initState       <= SET_ADDR;
						
               when SET_ADDR =>
                  s_initReadCounter <= s_initReadCounter+1;
                  s_latchCRdata     <= '0';
                  s_initState       <= GET_DATA;

               when GET_DATA =>
                  s_initReadCounter <= s_initReadCounter;
                  s_latchCRdata     <= '1';
                  if s_initInProgress = '1' then
                     s_initState <= SET_ADDR;
                  else
                     s_initState <= END_INIT;
                  end if;

               when END_INIT =>              -- will wait in this state until reset
                  s_initReadCounter <= s_initReadCounter;
                  s_latchCRdata     <= '0';
                  s_initState       <= END_INIT;

               when others =>
                  s_initState       <= IDLE;
                  s_initReadCounter <= to_unsigned(0, s_initReadCounter'length);
                  s_latchCRdata     <= '0';
            end case;

         end if;
      end if;
   end process;

   s_initInProgress <= '1' when (s_initReadCounter <= (424)) else '0';      
   InitInProgress_o <= s_initInProgress;
   s_CRadd_offset   <= s_CRaddr - s_CRaddr_base;

   process(s_latchCRdata, s_initReadCounter)
   begin
      s_latchCRdataPos <= (others => '0');
      s_CRaddr_base    <= (others => '0');
      for I in c_CRinitAddr'range loop
         if (s_initReadCounter >= c_CRinitAddr(I).add) and 
            (s_initReadCounter <= (c_CRinitAddr(I).add+(c_CRinitAddr(I).len-1))) then
             s_CRaddr_base       <= to_unsigned(c_CRinitAddr(I).add, s_CRaddr_base'length);
             s_latchCRdataPos(I) <= s_latchCRdata;
             exit;
         end if;
      end loop;
   end process;

   process(clk_i)
   begin
      if rising_edge(clk_i) then
         for I in 0 to 2 loop
            if (s_latchCRdataPos(BEG_USER_CR) = '1') and (unsigned(s_CRadd_offset) = I) then      
               s_BEG_USER_CR(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i);         
            end if;

            if s_latchCRdataPos(END_USER_CR) = '1' and (unsigned(s_CRadd_offset) = I) then  
               s_END_USER_CR(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i);         
            end if;

            if (s_latchCRdataPos(BEG_USER_CSR) = '1') and (unsigned(s_CRadd_offset) = I) then   
               s_BEG_USER_CSR(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i);         
            end if;

            if (s_latchCRdataPos(END_USER_CSR) = '1') and (unsigned(s_CRadd_offset) = I) then   
               s_END_USER_CSR(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i);         
            end if;

            if (s_latchCRdataPos(BEG_CRAM) = '1') and (unsigned(s_CRadd_offset) = I) then   
               s_BEG_CRAM(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i); 		 
            end if;

            if (s_latchCRdataPos(END_CRAM) = '1') and (unsigned(s_CRadd_offset) = I) then   
               s_END_CRAM(((3-I)*8 - 1) downto (2-I)*8) <= unsigned(CRdata_i);         
            end if;
         end loop;

         for I in 0 to 7 loop

            if (s_latchCRdataPos(FUNC_AMCAP) = '1') and (unsigned(s_CRadd_offset(5 downto 3)) = I) then    
               for H in 0 to 7 loop
                  if (unsigned(s_CRadd_offset(2 downto 0)) = H) then
                     s_FUNC_AMCAP(I)(((8-h)*8 - 1) downto (7-h)*8) <= unsigned(CRdata_i);         
                  end if;
               end loop;
            end if;
            if (s_latchCRdataPos(FUNC_ADEM) = '1') and (unsigned(s_CRadd_offset(5 downto 2)) = I) then    
               for H in 0 to 3 loop
                  if (unsigned(s_CRadd_offset(1 downto 0)) = H) then
                     s_FUNC_ADEM(I)(((4-h)*8 - 1) downto (3-h)*8) <= unsigned(CRdata_i);           				  
                  end if;
               end loop;
            end if;
            if (s_latchCRdataPos(FUNC_XAMCAP) = '1') and (unsigned(s_CRadd_offset(7 downto 5)) = I) then
               for H in 0 to 31 loop
                  if (unsigned(s_CRadd_offset(4 downto 0)) = H) then
                     s_FUNC_XAMCAP(I)(((32-h)*8 - 1) downto (31-h)*8) <= unsigned(CRdata_i);         
                  end if;
               end loop;
            end if;
         end loop;
      end if;
   end process; 

   BEG_USR_CR_o   <= std_logic_vector(s_BEG_USER_CR);
   END_USR_CR_o   <= std_logic_vector(s_END_USER_CR);
   BEG_USR_CSR_o  <= std_logic_vector(s_BEG_USER_CSR);
   END_USR_CSR_o  <= std_logic_vector(s_END_USER_CSR);
   BEG_CRAM_o     <= std_logic_vector(s_BEG_CRAM);
   END_CRAM_o     <= std_logic_vector(s_END_CRAM);
   FUNC0_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(0));
   FUNC1_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(1));
   FUNC2_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(2));
   FUNC3_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(3));
   FUNC4_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(4));
   FUNC5_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(5));
   FUNC6_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(6));
   FUNC7_ADEM_o   <= std_logic_vector(s_FUNC_ADEM(7));
   FUNC0_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(0));
   FUNC1_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(1));
   FUNC2_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(2));
   FUNC3_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(3));
   FUNC4_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(4));
   FUNC5_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(5));
   FUNC6_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(6));
   FUNC7_AMCAP_o  <= std_logic_vector(s_FUNC_AMCAP(7));
   FUNC0_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(0));
   FUNC1_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(1));
   FUNC2_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(2));
   FUNC3_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(3));
   FUNC4_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(4));
   FUNC5_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(5));
   FUNC6_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(6));
   FUNC7_XAMCAP_o <= std_logic_vector(s_FUNC_XAMCAP(7));

end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
