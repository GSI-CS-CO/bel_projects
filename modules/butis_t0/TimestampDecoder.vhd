-------------------------------------------------------------------------------
-- Title      : Timestamp Decoder
-- Project    : White Rabbit generator
-------------------------------------------------------------------------------
-- File       : TimestampDecoder.vhd
-- Author     : Peter Schakel
-- Company    : KVI
-- Created    : 2012-08-29
-- Last update: 2014-02-20
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
--
-------------------------------------------------------------------------------
--
-- rev 2014-02-20 : bug fix for not showing error/uncertain after cutting serial_i
--     autor: Peter Schakel
--
-------------------------------------------------------------------------------
-- Description:
--
-- Decodes a serial burst that contains a timestamp with CRC check. 
-- The first byte is 0xAA, but this is not part of the CRC data.
-- The timestamp is a 64-bits value that contains number of ns since 1970.
-- The serial timestamp is received each 10us (100kHz).
-- The timestamp value is the exact time value on the next 100kHz start.
-- The input clock is 200MHz.
-- If an CRC error occurs the value will be ignored and the timestamp counter will continue counting.
-- If several CRC errors occur or the time between the serial timestamps is not 10us, then an certain bit is set.
-- 
--
-- Generics
--     g_clockcyclesperbit : number 200MHz clock cycles for each serial bit (default 4)
--     g_BuTis_T0_precision : defines the precision on the check on the period of BuTiS T0 signal (+/- number of clock cycles).
--
-- Inputs
--     BuTis_C2_i : BuTiS 200 MHz clock
--     reset_i : reset
--     serial_i : BuTis T0 100kHz signal with encoded timestamp
--
-- Outputs
--     BuTis_T0_o : BuTis T0 100kHz signal, 1 clock pulse without timestamp
--     timestamp_o : Timestamp value
--     error_o : error detected that could not be corrected
--
-- Components
--     crc8_data8 : calculates crc8 value on a byte wide data stream
--
--
-------------------------------------------------------------------------------
-- Copyright (c) 2012 KVI / Peter Schakel
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-10-17  0.9      Peter Schakel   continuous timestamp output in ns
-------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
USE ieee.std_logic_unsigned.all ;
USE ieee.std_logic_arith.all ;


entity TimestampDecoder is
  generic(
	g_clockcyclesperbit                      : integer := 4;
	g_BuTis_T0_precision                     : integer := 3);
  port(
	BuTis_C2_i                               : in std_logic;
	reset_i                                  : in std_logic;
	serial_i                                 : in std_logic;
	BuTis_T0_o                               : out std_logic;
	timestamp_o                              : out std_logic_vector(63 downto 0);
	uncertain_o                              : out std_logic;
	error_o                                  : out std_logic);
end TimestampDecoder;

architecture rtl of TimestampDecoder is

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

type decoder_mode_type is (WAITFORSIGNAL,SER2PAR,CRCRESULT,WAITFORZEROS);
signal decoder_mode_s                     : decoder_mode_type := WAITFORSIGNAL;
signal prev_decoder_mode_s,prevprev_decoder_mode_s                : decoder_mode_type := WAITFORSIGNAL;

signal serial1_s                             : std_logic := '0';
signal serial2_s                             : std_logic := '0';
signal error_s                               : std_logic := '0';
signal error_nosignal_s                      : std_logic := '0';
signal error_counter_s                       : integer range 0 to 3 := 3;
signal period_error_s                        : std_logic := '1';
signal crc_data_in_s                         : std_logic_vector(7 downto 0) := (others => '0');
signal crc_data_out_s                        : std_logic_vector(7 downto 0) := (others => '0');
signal crc_start_s                           : std_logic := '0';
signal crc_data_valid_s                      : std_logic := '0';
signal crc_last_s                            : std_logic := '0';
signal crc_valid_s                           : std_logic := '0';
signal timestamp_s                           : std_logic_vector(63 downto 0) := (others => '0');
signal timestamp_rec_s                       : std_logic_vector(63 downto 0) := (others => '0');
signal phaseadjust_s                         : integer range 0 to 2;
signal phasecheck_counter_s                  : integer range 0 to 8;

signal clockcounter_s                        : integer range 0 to g_clockcyclesperbit-1 := 0;
signal bitcounter_s                          : integer range 0 to 7 := 0;
signal bytecounter_s                         : integer range 0 to 9 := 0;
signal ratiocounter_s                        : integer range 0 to 2000+g_BuTis_T0_precision := 0;
signal zeroscounter_s                        : integer range 0 to 1000 := 0;
signal butis_T0_counter                      : integer range 0 to 1999 := 0;
			
attribute syn_encoding : string;
attribute syn_encoding of decoder_mode_type : type is "safe";

begin

error_o <= '1' when (error_s='1') or (error_nosignal_s='1') or (period_error_s='1') else '0';
uncertain_o <= '1' when (error_counter_s=3) or (period_error_s='1') or (error_nosignal_s='1') else '0';
timestamp_o <= timestamp_s;

butis_T0_process: process(BuTis_C2_i)
begin
	if rising_edge(BuTis_C2_i) then
		BuTis_T0_o <= '0';
		if reset_i = '1' then
			error_counter_s <= 3;
		else
			if (decoder_mode_s=WAITFORSIGNAL) and (serial1_s='1') and (serial2_s='0') then
				if error_s='0' then
					timestamp_s <= timestamp_rec_s+10; -- 2 clock-cycle later
					butis_T0_counter <= 0;
					error_counter_s <= 0;
				else
					if error_counter_s<3 then
						error_counter_s <= error_counter_s+1;
					end if;
					timestamp_s <= timestamp_s+5;
				end if;
			else
				timestamp_s <= timestamp_s+5;
				if butis_T0_counter=1997 then
					BuTis_T0_o <= '1';
				end if;
				if butis_T0_counter=1999 then
					butis_T0_counter <= 0;
				else
					butis_T0_counter <= butis_T0_counter+1;
				end if;
			end if;
		end if;
	end if;
end process;

crc8_data8_inst: crc8_data8 port map(
	clock => BuTis_C2_i,
	reset => reset_i,
	soc => crc_start_s,
	data => crc_data_in_s,
	data_valid => crc_data_valid_s,
	eoc => crc_last_s,
	crc => crc_data_out_s,
	crc_valid => crc_valid_s);
	

-- process with state machine to translate serial data to parallel, feed it to the decoder and combine the result to one timestamp
BuTis_process : process(BuTis_C2_i)
begin
    if rising_edge(BuTis_C2_i) then
		crc_data_valid_s <= '0';
		crc_start_s <= '0';
		crc_last_s <= '0';
		if reset_i = '1' then
			period_error_s <= '1';
			ratiocounter_s <= 0;
			decoder_mode_s <= WAITFORSIGNAL;
			error_s <= '1';
		else
			if (decoder_mode_s=WAITFORSIGNAL) and (serial1_s='1') and (serial2_s='0') then -- new serial input pulse
				ratiocounter_s <= 0;
			elsif ratiocounter_s<2000+g_BuTis_T0_precision then -- ratiocounter to check 100kHz period
				ratiocounter_s <= ratiocounter_s+1;
				error_nosignal_s <= '0';
			else
				error_nosignal_s <= '1';
			end if;
			case decoder_mode_s is
				when WAITFORSIGNAL => -- Wait for next BuTis_T0 signal with serial encoded timestamp
					clockcounter_s <= 0;
					bitcounter_s <= 0;
					bytecounter_s <= 0;
					if serial1_s='1' then
						if serial2_s='0' then
							if ratiocounter_s<2000-g_BuTis_T0_precision then
								period_error_s <= '1';
							else
								period_error_s <= '0';
							end if;
							decoder_mode_s <= SER2PAR;
						else
							period_error_s <= '1';
						end if;
					end if;
					phaseadjust_s <= 1;
					phasecheck_counter_s <= 0;
				when SER2PAR => -- do the serial to parallel conversion and feed the second and further bytes to the decoder
					zeroscounter_s <= 0;
					if clockcounter_s<g_clockcyclesperbit-1 then
						if clockcounter_s=g_clockcyclesperbit/2-1 then  -- phaseadjust_s then -- bit phase adjustment disabled
							crc_data_in_s(7-bitcounter_s) <= serial1_s; -- next bit in the middle of the g_clockcyclesperbit clock-cycles for each serial data-bit
							if (bytecounter_s>0) and (bytecounter_s<9) then
								timestamp_rec_s(63-(bytecounter_s-1)*8-bitcounter_s) <= serial1_s;
							end if;
						end if;
						if (clockcounter_s=g_clockcyclesperbit-2) and (serial1_s='1') and (phasecheck_counter_s<8)
							and (bitcounter_s=0 or bitcounter_s=2 or bitcounter_s=4 or bitcounter_s=6) then
							phasecheck_counter_s <= phasecheck_counter_s+1; -- simple test to improve bit phase, doesn't improve!
						end if;					
						clockcounter_s <= clockcounter_s+1;
					else
						if (serial1_s='1') and (phasecheck_counter_s<8) 
							and (bitcounter_s=0 or bitcounter_s=2 or bitcounter_s=4 or bitcounter_s=6) then
							phasecheck_counter_s <= phasecheck_counter_s+1; -- simple test to improve bit phase, doesn't improve!
						end if;
						clockcounter_s <= 0;
						if bitcounter_s<7 then
							bitcounter_s <= bitcounter_s+1;
						else
							bitcounter_s <= 0;
							if bytecounter_s=0 then  -- simple test to improve bit phase, doesn't improve! (disabled now)
								if phasecheck_counter_s>7 then -- first 8 bits is startbyte (0xaa)
									phaseadjust_s <= 0; -- 0;
								elsif phasecheck_counter_s<1 then 
									phaseadjust_s <= 2; 
								else
									phaseadjust_s <= 1; 
								end if;
							elsif bytecounter_s=1 then
								crc_data_valid_s <= '1';
								crc_start_s <= '1';
							elsif bytecounter_s=8 then
								crc_data_valid_s <= '1';
								crc_last_s <= '1';
							elsif bytecounter_s/=9 then
								crc_data_valid_s <= '1';
							end if;
							if bytecounter_s<9 then -- until all bytes are received
								bytecounter_s <= bytecounter_s+1;
							else 
								bytecounter_s <= 0;
								decoder_mode_s <= CRCRESULT;
							end if;
						end if;
					end if;
				when CRCRESULT => 
					if crc_data_in_s/=crc_data_out_s then -- invalid crc
						error_s <= '1';
					else
						error_s <= '0';
					end if;
					decoder_mode_s <= WAITFORZEROS;
					clockcounter_s <= 0;
					bitcounter_s <= 0;
					bytecounter_s <= 0;
					zeroscounter_s <= 0;
				when WAITFORZEROS => -- after the data there should always be a gap filled with 0 before the next BuTiS_T0 pulse arrives
					if serial1_s='0' then
						if zeroscounter_s<500 then 
							zeroscounter_s <= zeroscounter_s+1;
						else
							zeroscounter_s <= 0;
							decoder_mode_s <= WAITFORSIGNAL;
						end if;
					else
						zeroscounter_s <= 0;
					end if;
				when others =>
					decoder_mode_s <= WAITFORSIGNAL;
			end case;
			serial2_s <= serial1_s;
			serial1_s <= serial_i;
		end if;
   end if;
end process;

  
end;

