-- EE 552 Student Application Note
--
-- Autononmous Linear Feedback Shift Register
--
-- Description: Implementation of a Linear Feedback Shift Register
-- that generates a sequence of 2^N-1 non-repeating pseudo-random numbers.
-- 
-- The Bit-Width or Length of the Pseudo Random Sequence
-- can be instantiated with the generic "Width."  
-- 
-- The length of the pseudorandom sequence can be anywhere from 2 to 32.  
-- (2^2 = 4 numbers in sequence and 2^32 = 4,294,967,295 numbers sequence !!
-- 
-- The seed input variable allows one to start the pseudo-random sequence at
-- a certain position in the pseudo-random sequence.
-- 
-- Uses: 	Digital Signal Processing
-- 		Wireless Communications
-- 		Encryption/Decryption
-- 		Direct Sequence Spread Spectrum
-- 		Pseudo-Random Number Generation
-- 		Scrambler/Descrambler
-- 		Built-In Self Test
-- 
-- References: 	"HDL Chip Design" - Douglas J. Smith
--				"Linear Feedback Shift Register MegaFunction" - Nova Engineering


library IEEE;
use IEEE.STD_Logic_1164.all;

entity LFSR_GENERIC is
  generic(Width: positive := 8);		-- length of pseudo-random sequence
  port 	(clock: in std_logic;
	 resetn: in std_logic;	-- active low reset
	 random_out: out std_logic_vector(Width-1 downto 0) -- parallel data out
	);

end entity LFSR_GENERIC;

architecture RTL of LFSR_GENERIC is

type TapsArrayType is array(2 to 32) of std_logic_vector(31 downto 0);
signal Taps: std_logic_vector(Width-1 downto 0);

begin

  LFSR: process (clock)
  
  -- internal registers and signals
  variable LFSR_Reg: std_logic_vector(Width-1 downto 0);
  variable Feedback: std_logic;
  variable TapsArray: TapsArrayType;
  
  begin
  
    Taps <= TapsArray(Width)(Width-1 downto 0);	-- get tap points from lookup table
    
    if resetn='0' then
    
      LFSR_Reg := (others=>'1');
      
      -- Look-Up Table for Tap points to insert XOR gates as feedback into D-FF 
      -- outputs.  Taps are designed so that 2^N-1 (N=Width of Register) numbers 
      -- are cycled through before the sequence is repeated
      	
      TapsArray(2)      :=	        "00000000000000000000000000000011";
      TapsArray(3)      :=		"00000000000000000000000000000101";
      TapsArray(4)      := 		"00000000000000000000000000001001";
      TapsArray(5)      :=		"00000000000000000000000000010010";
      TapsArray(6)      := 		"00000000000000000000000000100001";
      TapsArray(7)      :=		"00000000000000000000000001000001";
      TapsArray(8)      :=		"00000000000000000000000010001110";
      TapsArray(9)	:=		"00000000000000000000000100001000";
      TapsArray(10)	:=		"00000000000000000000001000000100";
      TapsArray(11)	:=		"00000000000000000000010000000010";
      TapsArray(12)	:=		"00000000000000000000100000101001";
      TapsArray(13)	:=		"00000000000000000001000000001101";
      TapsArray(14)	:=		"00000000000000000010000000010101";
      TapsArray(15)	:=		"00000000000000000100000000000001";
      TapsArray(16)	:=		"00000000000000001000000000010110";
      TapsArray(17)	:=		"00000000000000010000000000000100";
      TapsArray(18)	:=		"00000000000000100000000001000000";
      TapsArray(19)	:=		"00000000000001000000000000010011";			
      TapsArray(20)	:=		"00000000000010000000000000000100";
      TapsArray(21)	:=		"00000000000100000000000000000010";
      TapsArray(22)	:=		"00000000001000000000000000000001";
      TapsArray(23)	:=		"00000000010000000000000000010000";
      TapsArray(24)	:=		"00000000100000000000000000001101";
      TapsArray(25)	:=		"00000001000000000000000000000100";
      TapsArray(26)	:=		"00000010000000000000000000100011";
      TapsArray(27)	:=		"00000100000000000000000000010011";
      TapsArray(28)	:=		"00001000000000000000000000000100";
      TapsArray(29)	:=		"00010000000000000000000000000010";
      TapsArray(30)	:=		"00100000000000000000000000101001";
      TapsArray(31)	:=		"01000000000000000000000000000100";
      TapsArray(32)	:=		"10000000000000000000000001100010";
      
    
    -- Use RTL to construct linear feedback shift register network as described in
    -- appnote diagram
    
    elsif rising_edge(clock) then
    				
      Feedback := LFSR_Reg(Width-1); 			
      
      for N in Width-1 downto 1 loop
      
        if (Taps(N-1)='1') then
      
      	 LFSR_Reg(N) := LFSR_Reg(N-1) xor Feedback;
      
        else
      
          LFSR_Reg(N) := LFSR_Reg(N-1);
      
        end if;
      
      end loop;
      
      LFSR_Reg(0) := Feedback;
    
    end if;
   
    random_out <= LFSR_Reg;	-- parallel data out
  
  end process;

end RTL;


		
		

