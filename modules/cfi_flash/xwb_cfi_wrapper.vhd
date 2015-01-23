library ieee;
use ieee.std_logic_1164.all;
-------------------------------------------------------------------------------
-- Title      : Simple CFI Parallel Flash Wrapper - Arria 2
-- Project    : SCU
-------------------------------------------------------------------------------
-- File       : xwb_cfi_wrapper.vhd
-- Author     : K. Kaiser
-- Company    : GSI
-- Created    : 2014-09-03
-- Last update: 2014-09-03
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Wrapper for OpenCore CFI Flash Interface
-------------------------------------------------------------------------------
--
-- Copyright (c) 2014 GSI / K.Kaiser
--
-- This source file is free software; you can redistribute it   
-- and/or modify it under the terms of the GNU Lesser General   
-- Public License as published by the Free Software Foundation; 
-- either version 2.1 of the License, or (at your option) any   
-- later version.                                               
--
-- This source is distributed in the hope that it will be       
-- useful, but WITHOUT ANY WARRANTY; without even the implied   
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
-- PURPOSE.  See the GNU Lesser General Public License for more 
-- details.                                                     
--
-- You should have received a copy of the GNU Lesser General    
-- Public License along with this source; if not, download it   
-- from http://www.gnu.org/licenses/lgpl-2.1.html
-- 
--
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author    Description
-- 2014-09-03   1.0     kkaiser   Intitial Version
-------------------------------------------------------------------------------

use IEEE.STD_LOGIC_UNSIGNED.all;
use IEEE.NUMERIC_STD.all;

library WORK;
use work.cfi_flash_pkg.all;
use work.wishbone_pkg.all;


entity XWB_CFI_WRAPPER is

  port(
    clk_i      : in    std_logic;
    rst_n_i    : in    std_logic;

    -- Wishbone
    slave_i    : in    t_wishbone_slave_in;    -- to Slave
    slave_o    : out   t_wishbone_slave_out;   -- to WB
	  
	-- External Parallel Flash Pins
    AD         : out   std_logic_vector(25 downto 1);
    DF         : inout std_logic_vector(15 downto 0);
    ADV_FSH    : out   std_logic;
    nCE_FSH    : out   std_logic;
    CLK_FSH    : out   std_logic;
    nWE_FSH    : out   std_logic;
    nOE_FSH    : out   std_logic;
    nRST_FSH   : out   std_logic;
    WAIT_FSH   : in    std_logic

   );
end entity;--XWB_CFI_WRAPPER

architecture rtl of XWB_CFI_WRAPPER is
	
signal  rst_i   : std_logic;
signal  ack_int : std_logic;
signal  remapped_adr : std_logic_vector (31 downto 0);
begin

slave_o.stall <= not ack_int;
slave_o.ack <= ack_int;

AD(25) <='0';   -- A25 (Pin B6) is a no connect for 256 MB Densities

rst_i <= not rst_n_i;

--remapping due to SEL Lines 
--In case of 16 bit access cfi_cntrl_engine uses adr 24:1
--In case of 32 bit access cfi_cntrl_engine uses adr 24:2

process (slave_i.sel)
begin
if slave_i.sel = "1100" then
    remapped_adr(1 downto 0)<="00";
  elsif slave_i.sel = "0011" then
    remapped_adr(1 downto 0)<="10";
  else -- in this case addresses are not in the byte bounds  
    remapped_adr(1 downto 0)<=slave_i.adr (1 downto 0); 
end if;
end process;

remapped_adr (31 downto 2)<= slave_i.adr (31 downto 2);

-- flash write and read cycle counts are changeable in cfi_ctrl

CFI_PARALLEL_FLASH: cfi_ctrl port map (  
    wb_clk_i        =>  clk_i,
    wb_rst_i        =>  rst_i,
 
    wb_dat_i        =>  slave_i.dat,    --(31 downto 0);
    wb_adr_i        =>  remapped_adr,   --(31 downto 0);
    wb_stb_i        =>  slave_i.stb,
    wb_cyc_i        =>  slave_i.cyc,
    wb_we_i         =>  slave_i.we,
    wb_sel_i        =>  slave_i.sel,    --(3 downto 0);
    wb_dat_o        =>  slave_o.dat,    --(31 downto 0);
    wb_ack_o        =>  ack_int,
    wb_err_o        =>  slave_o.err,    --static '0' in cfi_ctrl
    wb_rty_o        =>  slave_o.rty,    --static '0' in cfi_ctrl
    wb_stall_o      =>  open,
  
  
    flash_dq_io     =>  DF,				  --(15 downto 0);
    flash_adr_o     =>  AD (24 downto 1),--(SCU AD1 is LSB, mapped to 23 : 0 from cfi_ctrl);
    flash_adv_n_o   =>  ADV_FSH,
    flash_ce_n_o    =>  nCE_FSH,
    flash_clk_o     =>  CLK_FSH,         -- CLK connected, but static '1' in operation
    flash_oe_n_o    =>  nOE_FSH,
    flash_rst_n_o   =>  nRST_FSH,
    flash_wait_i    =>  WAIT_FSH,
    flash_we_n_o    =>  nWE_FSH,
    flash_wp_n_o    =>  open             --static '1' in cfi_ctrl, not used in SCU3

	); -- cfi_ctrl
	
end rtl;