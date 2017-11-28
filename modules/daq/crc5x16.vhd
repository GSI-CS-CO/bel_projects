-------------------------------------------------------------------------------
-- Copyright (C) 2009 OutputLogic.com
-- This source file may be used and distributed without restriction
-- provided that this copyright statement is not removed from the file
-- and that any derivative work contains the original copyright notice
-- and the associated disclaimer.
-- 
-- THIS SOURCE FILE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
-- WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
-------------------------------------------------------------------------------
-- CRC module for data(15:0)
--   lfsr(4:0)=1+x^2+x^5;
-------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

ENTITY crc5x16 IS
	PORT (
      nReset           : IN std_logic;
      clk_i            : IN std_logic;
  	   data_in          : IN std_logic_vector (15 DOWNTO 0);
		crc_start_pulse  : IN std_logic;                       -- Set CRC to its Start value on transmission of a new packet
		crc_en_pulse     : IN std_logic;                       -- Enables CRC calculation on stored previous CRC and data_in 
		crc_out          : OUT std_logic_vector (15 DOWNTO 0)
	);
END crc5x16;

ARCHITECTURE outputlogic_implementation OF crc5x16 IS

	SIGNAL lfsr_q : std_logic_vector (4 DOWNTO 0);
	SIGNAL lfsr_c : std_logic_vector (4 DOWNTO 0);

BEGIN
	crc_out   <= b"0000_0000_000"& lfsr_q;

	lfsr_c(0) <= lfsr_q(0) XOR lfsr_q(1) XOR lfsr_q(2)  XOR data_in(0) XOR data_in(3) XOR data_in(5) XOR data_in(6) XOR data_in(9)  XOR data_in(10) XOR data_in(11) XOR data_in(12) XOR data_in(13);
	lfsr_c(1) <= lfsr_q(0) XOR lfsr_q(1) XOR lfsr_q(2)  XOR lfsr_q(3)  XOR data_in(1) XOR data_in(4) XOR data_in(6) XOR data_in(7)  XOR data_in(10) XOR data_in(11) XOR data_in(12) XOR data_in(13) XOR data_in(14);
	lfsr_c(2) <= lfsr_q(3) XOR lfsr_q(4) XOR data_in(0) XOR data_in(2) XOR data_in(3) XOR data_in(6) XOR data_in(7) XOR data_in(8)  XOR data_in(9)  XOR data_in(10) XOR data_in(14) XOR data_in(15);
	lfsr_c(3) <= lfsr_q(0) XOR lfsr_q(4) XOR data_in(1) XOR data_in(3) XOR data_in(4) XOR data_in(7) XOR data_in(8) XOR data_in(9)  XOR data_in(10) XOR data_in(11) XOR data_in(15);
	lfsr_c(4) <= lfsr_q(0) XOR lfsr_q(1) XOR data_in(2) XOR data_in(4) XOR data_in(5) XOR data_in(8) XOR data_in(9) XOR data_in(10) XOR data_in(11) XOR data_in(12);
	
  result_latch: PROCESS (clk_i, nReset) 
  BEGIN
	  IF (nReset = '0') THEN
		    lfsr_q <= b"11111";
  	ELSIF rising_edge(clk_i) THEN
  	  IF crc_start_pulse = '1' THEN
	    	lfsr_q <= b"11111";
	  	ELSIF (crc_en_pulse = '1') THEN
		  	lfsr_q <= lfsr_c;
		  END IF;
	  END IF;
  END PROCESS result_latch;

END ARCHITECTURE outputlogic_implementation;
