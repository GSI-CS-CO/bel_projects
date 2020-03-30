--    /*  -------------------------------------------------------------------------

--    This program is free software: you can redistribute it and/or modify
--    it under the terms of the GNU General Public License as published by
--    the Free Software Foundation, either version 3 of the License, or
--    any later version.
--
--    This program is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--    GNU General Public License for more details.
--    
--    You should have received a copy of the GNU General Public License
--    along with this program.  If not, see <http://www.gnu.org/licenses/>.
--    
--    Copyright: Levent Ozturk crc@leventozturk.com
--    https://leventozturk.com/engineering/crc/
--    Polynomial: x16+x15+x2+1
--    d0 is the first data


--    c is internal LFSR state and the CRC output. Not needed for other modules than CRC.
--    c width is always same as polynomial width.
--    o is the output of all modules except CRC. Not needed for CRC.
--    o width is always same as data width width
-------------------------------------------------------------------------*/

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity crc16_usb is generic (
	SEED : in std_logic_vector( 15 downto 0) := b"1111111111111111"
); port (
	clk   : in std_logic;
	reset : in std_logic;
	fd    : in std_logic; -- First data. 1: SEED is used (initialise and calculate), 0 : Previous CRC is used (continue and calculate)
	nd    : in std_logic; -- New Data. d input has a valid data. Calculate new CRC
	rdy   : out std_logic;
	d     : in std_logic_vector( 15 downto 0);  -- Data in
	c     : out std_logic_vector( 15 downto 0); -- CRC output
	o     : out std_logic_vector( 15 downto 0) -- Data output
); end crc16_usb;

architecture a1 of crc16_usb is
	signal nd_q : std_logic;
	signal fd_q : std_logic;
	signal dq   : std_logic_vector ( 15 downto 0);
	signal ca   : std_logic_vector( 15 downto 0);
	signal oa   : std_logic_vector( 15 downto 0);
begin
	process (clk)
	begin
		if (rising_edge(clk)) then
			nd_q <= nd;
			fd_q <= fd;
                -- d(15) processed first
                        --dq(  0) <= d( 15) xor d( 13) xor d( 12) xor d( 11) xor d( 10) xor d(  9) xor d(  8) xor d(  7) xor d(  6) xor d(  5) xor d(  4) xor d(  3) xor d(  2) xor d(  1) xor d(  0);
			--dq(  1) <= d( 14) xor d( 13) xor d( 12) xor d( 11) xor d( 10) xor d(  9) xor d(  8) xor d(  7) xor d(  6) xor d(  5) xor d(  4) xor d(  3) xor d(  2) xor d(  1);
			--dq(  2) <= d( 14) xor d(  1) xor d(  0);
			--dq(  3) <= d( 15) xor d(  2) xor d(  1);
			--dq(  4) <= d(  3) xor d(  2);
			--dq(  5) <= d(  4) xor d(  3);
			--dq(  6) <= d(  5) xor d(  4);
			--dq(  7) <= d(  6) xor d(  5);
			--dq(  8) <= d(  7) xor d(  6);
			--dq(  9) <= d(  8) xor d(  7);
			--dq( 10) <= d(  9) xor d(  8);
			--dq( 11) <= d( 10) xor d(  9);
			--dq( 12) <= d( 11) xor d( 10);
			--dq( 13) <= d( 12) xor d( 11);
			--dq( 14) <= d( 13) xor d( 12);
			--dq( 15) <= d( 15) xor d( 14) xor d( 12) xor d( 11) xor d( 10) xor d(  9) xor d(  8) xor d(  7) xor d(  6) xor d(  5) xor d(  4) xor d(  3) xor d(  2) xor d(  1) xor d(  0);
                        
                -- d(0) processed first
			dq(  0) <= d(  0) xor d(  2) xor d(  3) xor d(  4) xor d(  5) xor d(  6) xor d(  7) xor d(  8) xor d(  9) xor d( 10) xor d( 11) xor d( 12) xor d( 13) xor d( 14) xor d( 15);
			dq(  1) <= d(  1) xor d(  2) xor d(  3) xor d(  4) xor d(  5) xor d(  6) xor d(  7) xor d(  8) xor d(  9) xor d( 10) xor d( 11) xor d( 12) xor d( 13) xor d( 14);
                        dq(  2) <= d(  1) xor d( 14) xor d( 15);
			dq(  3) <= d(  0) xor d( 13) xor d( 14);
			dq(  4) <= d( 12) xor d( 13);
			dq(  5) <= d( 11) xor d( 12);
			dq(  6) <= d( 10) xor d( 11);
			dq(  7) <= d(  9) xor d( 10);
			dq(  8) <= d(  8) xor d(  9);
			dq(  9) <= d(  7) xor d(  8);
			dq( 10) <= d(  6) xor d(  7);
			dq( 11) <= d(  5) xor d(  6);
			dq( 12) <= d(  4) xor d(  5);
			dq( 13) <= d(  3) xor d(  4);
			dq( 14) <= d(  2) xor d(  3);
			dq( 15) <= d(  0) xor d(  1) xor d(  3) xor d(  4) xor d(  5) xor d(  6) xor d(  7) xor d(  8) xor d(  9) xor d( 10) xor d( 11) xor d( 12) xor d( 13) xor d( 14) xor d( 15);

		end if;
	end process;

	process (clk, reset)
	begin
		if (reset= '0') then
			ca <= SEED;
			rdy <= '0';
		elsif (rising_edge(clk)) then
			rdy <= nd_q;
			if(nd_q= '1') then
				if (fd_q= '1') then
					ca(  0) <= SEED(  0) xor SEED(  1) xor SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 15) xor dq(  0);
					ca(  1) <= SEED(  1) xor SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor dq(  1);
					ca(  2) <= SEED(  0) xor SEED(  1) xor SEED( 14) xor dq(  2);
					ca(  3) <= SEED(  1) xor SEED(  2) xor SEED( 15) xor dq(  3);
					ca(  4) <= SEED(  2) xor SEED(  3) xor dq(  4);
					ca(  5) <= SEED(  3) xor SEED(  4) xor dq(  5);
					ca(  6) <= SEED(  4) xor SEED(  5) xor dq(  6);
					ca(  7) <= SEED(  5) xor SEED(  6) xor dq(  7);
					ca(  8) <= SEED(  6) xor SEED(  7) xor dq(  8);
					ca(  9) <= SEED(  7) xor SEED(  8) xor dq(  9);
					ca( 10) <= SEED(  8) xor SEED(  9) xor dq( 10);
					ca( 11) <= SEED(  9) xor SEED( 10) xor dq( 11);
					ca( 12) <= SEED( 10) xor SEED( 11) xor dq( 12);
					ca( 13) <= SEED( 11) xor SEED( 12) xor dq( 13);
					ca( 14) <= SEED( 12) xor SEED( 13) xor dq( 14);
					ca( 15) <= SEED(  0) xor SEED(  1) xor SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 14) xor SEED( 15) xor dq( 15);


					oa(  0) <= SEED( 15) xor dq(  0);
					oa(  1) <= SEED( 14) xor SEED( 15) xor dq(  1);
					oa(  2) <= SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  2);
					oa(  3) <= SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  3);
					oa(  4) <= SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  4);
					oa(  5) <= SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  5);
					oa(  6) <= SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  6);
					oa(  7) <= SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  7);
					oa(  8) <= SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  8);
					oa(  9) <= SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq(  9);
					oa( 10) <= SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq( 10);
					oa( 11) <= SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq( 11);
					oa( 12) <= SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq( 12);
					oa( 13) <= SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor SEED( 15) xor dq( 13);
					oa( 14) <= SEED(  1) xor SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 14) xor dq( 14);
					oa( 15) <= SEED(  0) xor SEED(  1) xor SEED(  2) xor SEED(  3) xor SEED(  4) xor SEED(  5) xor SEED(  6) xor SEED(  7) xor SEED(  8) xor SEED(  9) xor SEED( 10) xor SEED( 11) xor SEED( 12) xor SEED( 13) xor SEED( 15) xor dq( 15);
				else
					ca(  0) <= ca(  0) xor ca(  1) xor ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 15) xor dq(  0);
					ca(  1) <= ca(  1) xor ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor dq(  1);
					ca(  2) <= ca(  0) xor ca(  1) xor ca( 14) xor dq(  2);
					ca(  3) <= ca(  1) xor ca(  2) xor ca( 15) xor dq(  3);
					ca(  4) <= ca(  2) xor ca(  3) xor dq(  4);
					ca(  5) <= ca(  3) xor ca(  4) xor dq(  5);
					ca(  6) <= ca(  4) xor ca(  5) xor dq(  6);
					ca(  7) <= ca(  5) xor ca(  6) xor dq(  7);
					ca(  8) <= ca(  6) xor ca(  7) xor dq(  8);
					ca(  9) <= ca(  7) xor ca(  8) xor dq(  9);
					ca( 10) <= ca(  8) xor ca(  9) xor dq( 10);
					ca( 11) <= ca(  9) xor ca( 10) xor dq( 11);
					ca( 12) <= ca( 10) xor ca( 11) xor dq( 12);
					ca( 13) <= ca( 11) xor ca( 12) xor dq( 13);
					ca( 14) <= ca( 12) xor ca( 13) xor dq( 14);
					ca( 15) <= ca(  0) xor ca(  1) xor ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 14) xor ca( 15) xor dq( 15);


					oa(  0) <= ca( 15) xor dq(  0);
					oa(  1) <= ca( 14) xor ca( 15) xor dq(  1);
					oa(  2) <= ca( 13) xor ca( 14) xor ca( 15) xor dq(  2);
					oa(  3) <= ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  3);
					oa(  4) <= ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  4);
					oa(  5) <= ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  5);
					oa(  6) <= ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  6);
					oa(  7) <= ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  7);
					oa(  8) <= ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  8);
					oa(  9) <= ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq(  9);
					oa( 10) <= ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq( 10);
					oa( 11) <= ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq( 11);
					oa( 12) <= ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq( 12);
					oa( 13) <= ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor ca( 15) xor dq( 13);
					oa( 14) <= ca(  1) xor ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 14) xor dq( 14);
					oa( 15) <= ca(  0) xor ca(  1) xor ca(  2) xor ca(  3) xor ca(  4) xor ca(  5) xor ca(  6) xor ca(  7) xor ca(  8) xor ca(  9) xor ca( 10) xor ca( 11) xor ca( 12) xor ca( 13) xor ca( 15) xor dq( 15);
				end if;
			end if;
		end if;
	end process;
	--c <= ca;
        c <= not(ca(0) & ca(1) & ca(2) & ca(3) & ca(4) & ca(5) & ca(6) & ca(7) & ca(8) & ca(9) & ca(10) & ca(11) & ca(12) & ca(13) & ca(14) & ca(15)); -- reflected and inverted
	o <= oa;
end a1;
