-------------------------------------------------------------------------------
-- Title      : Timestamp Encoder
-- Project    : White Rabbit generator
-------------------------------------------------------------------------------
-- File       : TimestampEncoder.vhd
-- Author     : Peter Schakel
-- Company    : KVI
-- Created    : 2012-08-27
-- Last update: 2013-11-12
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description:
--
-- Translates a Timestamp in serial burst with crcc-check code. 
-- The timestamp is a linear counter (i.e. 64-bits) that counts on every BuTiS C2 clock (200MHz)
-- The code: 
--     the timestamp is divided in bytes.
--     the bytes ares sent serially, 4 clock cycles for each bit, Most Significant Bit first
--     the first byte is 0xaa
--     then the timestamp bytes are tranceived, Most Significant Byte first
--     after this the crc8 check byte is sent. (crc8 : Polynomial : x^8 + x^2 + x^1 + 1, init=x"0")
--     
--     Between the last bit and the next BuTiS T0 with code the signal is zero
-- 
-- Generics
--     g_clockcyclesperbit : number 200MHz clock cycles for each serial bit (default 4)
--
-- Inputs
--     BuTis_C2_i : BuTiS 200 MHz clock
--     BuTis_T0_i : BuTis T0 100kHz signal
--     reset_i : reset
--     timestamp_on_next_T0_i : timestamp valid on next BuTis T0 pulse
--
-- Outputs
--     serial_o : BuTis T0 100kHz signal with encoded timestamp
--     error_o : error detected: BuTis_T0_i signal period is not exactly 2000 clock cycles
--
-- Components
--     crc8_data8 : calculates crc8 check 
--
-------------------------------------------------------------------------------
-- Copyright (c) 2012 KVI / Peter Schakel
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-10-17  0.9      Peter Schakel   Fixed to 64 bit timestamp in ns with CRC check
-------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
USE ieee.std_logic_unsigned.all ;
USE ieee.std_logic_arith.all ;


entity TimestampEncoder is
  generic(
		g_clockcyclesperbit                      : integer := 4);
  port(
		BuTis_C2_i                               : in  std_logic;
		BuTis_T0_i                               : in  std_logic;
		reset_i                                  : in  std_logic;
		timestamp_on_next_T0_i                   : in  std_logic_vector(63 downto 0);
		serial_o                                 : out std_logic;
		error_o                                  : out std_logic);
end TimestampEncoder;

architecture rtl of TimestampEncoder is

component crc8_data8 IS 
   PORT(           
           clock      : IN  STD_LOGIC; 
           reset      : IN  STD_LOGIC; 
           soc        : IN  STD_LOGIC; 
           data       : IN  STD_LOGIC_VECTOR(7 DOWNTO 0); 
           data_valid : IN  STD_LOGIC; 
           eoc        : IN  STD_LOGIC; 
           crc        : OUT STD_LOGIC_VECTOR(7 DOWNTO 0); 
           crc_valid  : OUT STD_LOGIC 
       );
END component; 


type encoder_mode_type is (STARTBYTE,PAR2SER,NEXTTIMESTAMP);
signal encoder_mode_s                        : encoder_mode_type := NEXTTIMESTAMP;

signal crc_data_in_s                         : std_logic_vector(7 downto 0) := (others => '0');
signal crc_data_out_s                        : std_logic_vector(7 downto 0) := (others => '0');
signal crc_start_s                           : std_logic := '0';
signal crc_data_valid_s                      : std_logic := '0';
signal crc_last_s                            : std_logic := '0';
signal crc_valid_s                           : std_logic := '0';

signal BuTis_T0_s                            : std_logic := '0';
signal serial_s                              : std_logic := '0';

signal clockcounter_s                        : integer range 0 to g_clockcyclesperbit-1 := 0;
signal bitcounter_s                          : integer range 0 to 7 := 0;
signal bytecounter_s                         : integer range 0 to 8 := 0;
signal tsbytecounter_s                       : integer range 0 to 8 := 0;
signal BuTis_T0_counter_s                    : integer range 0 to 2000 := 0;

begin
serial_o <= serial_s;

crc8_data8_inst: crc8_data8 port map(
	clock => BuTis_C2_i,
	reset => reset_i,
	soc => crc_start_s,
	data => crc_data_in_s,
	data_valid => crc_data_valid_s,
	eoc => crc_last_s,
	crc => crc_data_out_s,
	crc_valid => crc_valid_s);


	 
 -- process check for time between BuTiS_T0 100kHz pulses
timestamp_process : process(BuTis_C2_i)
begin
    if rising_edge(BuTis_C2_i) then
		crc_start_s <= '0';
		crc_last_s <= '0';
		crc_data_valid_s <= '0';
		if reset_i = '1' then
			error_o <= '0';
		else
			if BuTis_T0_i='1' then -- rising edge BuTis_T0
				BuTis_T0_counter_s <= 0;
				tsbytecounter_s <= 0;
			else -- increment counter for BuTiS_T0 period check
				if BuTis_T0_counter_s<=1999 then
					BuTis_T0_counter_s <= BuTis_T0_counter_s+1;
				end if;					
				if tsbytecounter_s=0 then
					crc_data_in_s <= timestamp_on_next_T0_i(64-tsbytecounter_s*8-1 downto 64-tsbytecounter_s*8-8);
					crc_data_valid_s <= '1';
					crc_start_s <= '1';
					tsbytecounter_s <= tsbytecounter_s+1;
				elsif tsbytecounter_s<8 then
					crc_data_in_s <= timestamp_on_next_T0_i(64-tsbytecounter_s*8-1 downto 64-tsbytecounter_s*8-8);
					crc_data_valid_s <= '1';
					tsbytecounter_s <= tsbytecounter_s+1;
					if tsbytecounter_s=7 then
						crc_last_s <= '1';
					end if;
				else 
					crc_data_in_s <= (others => '0');
				end if;
			end if;
		end if;
	end if;
end process;

 -- process with statemachine to encode byte-wide data and serialize the result
BuTis_process : process(BuTis_C2_i)
begin
    if rising_edge(BuTis_C2_i) then
		if reset_i = '1' then
			BuTis_T0_s <= '0';
			encoder_mode_s <= NEXTTIMESTAMP;
			serial_s <= '0';
		else
			BuTis_T0_s <= BuTis_T0_i;			
			if BuTis_T0_i='1' and BuTis_T0_s='0' then -- check for rising edge BuTis_T0 (100kHz pulse)
				encoder_mode_s <= STARTBYTE; -- go to state STARTBYTE: send one byte serially 
				clockcounter_s <= 1;
				bitcounter_s <= 0;
				bytecounter_s <= 0;
				serial_s <= '1';
			elsif encoder_mode_s=STARTBYTE then -- state STARTBYTE: send one byte serially startbyte is 0xaa
				if clockcounter_s=0 then
					serial_s <= not serial_s;
				end if;
				if clockcounter_s<g_clockcyclesperbit-1 then
					clockcounter_s <= clockcounter_s+1;
				else
					clockcounter_s <= 0;
					if bitcounter_s<7 then
						bitcounter_s <= bitcounter_s+1;
					else
						bitcounter_s <= 0;
						encoder_mode_s <= PAR2SER;
					end if;
				end if;
				bytecounter_s <= 0;
			elsif encoder_mode_s=PAR2SER then -- state PAR2SER: translate byte-wise encoded data to serial
				if clockcounter_s=0 then
					if bytecounter_s<8 then
						serial_s <= timestamp_on_next_T0_i(63-8*bytecounter_s-bitcounter_s);
					else
						serial_s <= crc_data_out_s(7-bitcounter_s);  -- last byte is crc check
					end if;
				end if;
				if clockcounter_s<g_clockcyclesperbit-1 then
					clockcounter_s <= clockcounter_s+1;
				else
					clockcounter_s <= 0;
					if bitcounter_s<7 then
						bitcounter_s <= bitcounter_s+1;
					else
						bitcounter_s <= 0;
						if bytecounter_s<8 then
							bytecounter_s <= bytecounter_s+1;
						else
							encoder_mode_s <= NEXTTIMESTAMP;
							bytecounter_s <= 0;
						end if;
					end if;
				end if;
			elsif encoder_mode_s=NEXTTIMESTAMP then -- state NEXTTIMESTAMP: serial output is 0, rising edge BuTis_T0 will leave this state
				serial_s <= '0';
			end if;
		end if;
   end if;
end process;

  
end;

