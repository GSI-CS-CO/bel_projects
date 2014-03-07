-------------------------------------------------------------------------------
-- Title      : BuTiS T0 clock generator
-- Project    : White Rabbit generator
-------------------------------------------------------------------------------
-- File       : BuTiS_T0_generator.vhd
-- Author     : Peter Schakel
-- Company    : KVI
-- Created    : 2013-10-16
-- Last update: 2013-11-12
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description:
--
-- Generates BuTis T0 clock from white rabbit clock signals
-- Input White Rabbit 125MHz clock and 1 second PPS pulse, and 200MHz BuTis clock,synchronized to White Rabbit clock.
-- On every 2000 clock cycles a BuTis_T0 signal is generated (100kHz). This is synchronized to the PPS-pulse.
-- A timestamp (64 bits) is sent as serial burst on each BuTis_T0 signal.
-- The serial burst has 2 or 4 clock cycles per bit. The timestamp is extended with a CRC code.
-- 
-- Generics
--     g_clockcyclesperbit : number 200MHz clock cycles for each serial bit (default 4)
--
-- Inputs
--     wr_clock_i : White Rabbit 125MHz clock
--     wr_rst_n_i  reset synchronous with white rabbit clock: low active
--     wr_PPSpulse_i : White Rabbit PPS pulse
--     BuTis_rst_n_i : reset synchronous on 200MHz clock: low active
--     timestamp_i : 64-bit timestamp
--     BuTis_C2_i : BuTiS 200 MHz clock
--
-- Outputs
--     BuTis_T0_o : BuTis T0 100kHz signal, 1 clock cycle high
--     BuTis_T0_timestamp_o : BuTis T0 100kHz signal with serial encoded 64-bits timestamp
--     error_o : error detected: wr_PPSpulse_i signal period is not exactly 1 second
--
-- Components
--     TimestampEncoder : Encoder for 64-bits timestamp into serial burst
--
--
-------------------------------------------------------------------------------
-- Copyright (c) 2013 KVI / Peter Schakel
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
USE ieee.std_logic_unsigned.all ;
USE ieee.std_logic_arith.all ;

library work;
use work.wr_serialtimestamp_pkg.all;

entity BuTiS_T0_generator is
  generic(
		g_clockcyclesperbit                      : integer := 4);
  port(
		wr_clock_i                             : in  std_logic; -- 125MHz
		wr_rst_n_i                             : in  std_logic;
		wr_PPSpulse_i                          : in  std_logic; -- 1s
		BuTis_rst_n_i                          : in std_logic; -- /reset
		timestamp_i                            : in  std_logic_vector(63 downto 0);
		BuTis_C2_i                             : in std_logic; -- 200MHz
		BuTis_T0_o                             : out std_logic; -- 100kHz
		BuTis_T0_timestamp_o                   : out std_logic; -- 100kHz with timestamp
		error_o                                : out std_logic);
end BuTiS_T0_generator;

architecture rtl of BuTiS_T0_generator is

  signal BuTis_rst_s                           : std_logic;
  signal wr_BuTis_T0_s                         : std_logic := '0';
  signal wr_timestamp_nextT0_s                 : std_logic_vector(63 downto 0);
  signal wr_PPS_error_s                        : std_logic := '0';
  signal wr_PPSpulse_prev_s                    : std_logic := '0';
  signal wr_counter_10us_s                     : integer range 0 to 1250 := 0;  
  signal wr_counter_T0_s                       : integer range 0 to 100000 := 0;

  signal BuTis_T0_s                            : std_logic := '0';
  signal BuTis_T0_aftr1clk_s                   : std_logic := '0'; 
  signal BuTis_T0_aftr2clk_s                   : std_logic := '0'; 
  signal PPS_error_s                           : std_logic := '0';
  signal encoder_error_s                       : std_logic := '0';
  signal BuTis_T0_error_s                      : std_logic := '0';
  signal counter_10us_s                        : integer range 0 to 2000 := 0;  

attribute keep: boolean;
attribute keep of wr_timestamp_nextT0_s: signal is true;
		
  begin

error_o <= '1' when (encoder_error_s='1') or (BuTis_T0_error_s='1') else '0';

-- process for checking wr_PPSulse period and for generating BuTis_T0 with 125MHz clock and the right phase
wr_PPScheck_process : process(wr_clock_i)
begin
	if rising_edge(wr_clock_i) then
		wr_PPS_error_s <= '0';
		wr_BuTis_T0_s <= '0';
		if wr_rst_n_i='0' then
			wr_counter_10us_s <= 0;
			wr_counter_T0_s <= 0;
		else
			if wr_counter_10us_s=1248 then 
				wr_BuTis_T0_s <= '1';
				wr_timestamp_nextT0_s <= timestamp_i+10000+8;
			end if;
			if wr_PPSpulse_i='1' and wr_PPSpulse_prev_s='0' then
				if wr_counter_T0_s/=99999 or wr_counter_10us_s/=1249 then
					wr_PPS_error_s <= '1';
				end if;
				wr_counter_10us_s <= 0;
				wr_counter_T0_s <= 0;
			else
				if wr_counter_10us_s<1249 then
					wr_counter_10us_s <= wr_counter_10us_s+1;
				else
					wr_counter_10us_s <= 0;
					if wr_counter_T0_s<99999 then
						wr_counter_T0_s <= wr_counter_T0_s+1;
					else
						wr_counter_T0_s <= 0;
					end if;
				end if;
			end if;
			wr_PPSpulse_prev_s <= wr_PPSpulse_i;
		end if;
	end if;
end process;

-- process for checking wr_PPSulse period and for generating BuTis_T0 with the right phase
BuTis_process : process(BuTis_C2_i)
begin
	if rising_edge(BuTis_C2_i) then
		BuTis_T0_error_s <= '0';
		BuTis_T0_s <= '0';
		if BuTis_rst_n_i='0' then
			counter_10us_s <= 0;
			BuTis_T0_s <= '0';
		else
			if counter_10us_s=1996 then -- or 1996 ????????
				BuTis_T0_s <= '1';
			end if;
			if BuTis_T0_aftr1clk_s='1' and BuTis_T0_aftr2clk_s='0' then
				if counter_10us_s/=1999 then
					BuTis_T0_error_s <= '1';
				end if;
				counter_10us_s <= 0;
			else
				if counter_10us_s<1999 then
					counter_10us_s <= counter_10us_s+1;
				else
					counter_10us_s <= 0;
				end if;
			end if;
			BuTis_T0_aftr2clk_s <= BuTis_T0_aftr1clk_s;
			BuTis_T0_aftr1clk_s <= wr_BuTis_T0_s;
		end if;
		BuTis_T0_o <= BuTis_T0_s; -- 1 clock later
	end if;
end process;

		
TimestampEncoder1: TimestampEncoder 
	generic map(
		g_clockcyclesperbit => g_clockcyclesperbit)
	port map(
		BuTis_C2_i => BuTis_C2_i,
		BuTis_T0_i => BuTis_T0_s,
		reset_i => BuTis_rst_s,
		timestamp_on_next_T0_i => wr_timestamp_nextT0_s,
		serial_o => BuTis_T0_timestamp_o,
		error_o => encoder_error_s);
BuTis_rst_s <= not BuTis_rst_n_i;


end;

