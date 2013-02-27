library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity postcode is
	port (
		lclk:	in std_logic;
		pdata_valid:	in std_logic;
		paddr_valid:	in std_logic;
		
		paddr:		in std_logic_vector(15 downto 0);
		pdata:		in std_logic_vector(7 downto 0);
		
		addr_hit:	out std_logic;
		data_valid:	in std_logic;
		
		      
		seven_seg_L:	out std_logic_vector(7 downto 0);    -- SSeg Data output
		seven_seg_H:	out std_logic_vector(7 downto 0)    -- SSeg Data output  
	
	);
end entity;



architecture postcode_arch of postcode is
   
   signal postcode_data: 	std_logic_vector(7 downto 0);
   constant postcode_addr: 	std_logic_vector(15 downto 0) := x"0080";
begin

postcode_reg: process(lclk)
begin
	if rising_edge(lclk) then
		if (paddr = postcode_addr and data_valid = '1') then
			addr_hit <= '1';
			postcode_data <= pdata;
		else
			addr_hit <= '0';
		end if;
	end if;
end process;


P_sseg_decode: process(lclk) -- decode section for 7 seg displays
   begin
      if rising_edge(lclk) then
         case postcode_data(7 downto 4) is -- Most sig digit for display
            when "0000" => seven_seg_H <= "00000011"; -- Hex 03 displays a 0
            when "0001" => seven_seg_H <= "10011111"; -- Hex 9f displays a 1
            when "0010" => seven_seg_H <= "00100101"; -- Hex 25 displays a 2
            when "0011" => seven_seg_H <= "00001101"; -- Hex 0d displays a 3
            when "0100" => seven_seg_H <= "10011001"; -- Hex 99 displays a 4
            when "0101" => seven_seg_H <= "01001001"; -- Hex 49 displays a 5
            when "0110" => seven_seg_H <= "01000001"; -- Hex 41 displays a 6
            when "0111" => seven_seg_H <= "00011111"; -- Hex 1f displays a 7
            when "1000" => seven_seg_H <= "00000001"; -- Hex 01 displays a 8
            when "1001" => seven_seg_H <= "00001001"; -- Hex 09 displays a 9
            when "1010" => seven_seg_H <= "00010001"; -- Hex 11 displays a A
            when "1011" => seven_seg_H <= "11000001"; -- Hex c1 displays a b
            when "1100" => seven_seg_H <= "01100011"; -- Hex 63 displays a C
            when "1101" => seven_seg_H <= "10000101"; -- Hex 85 displays a d
            when "1110" => seven_seg_H <= "01100001"; -- Hex 61 displays a E
            when "1111" => seven_seg_H <= "01110001"; -- Hex 71 displays a F
            when others => seven_seg_H <= "00000001"; -- Hex 01 displays a 8
         end case;
         case postcode_data(3 downto 0) is -- Least sig digit for display
            when "0000" => seven_seg_L <= "00000011"; -- Hex 03 displays a 0
            when "0001" => seven_seg_L <= "10011111"; -- Hex 9f displays a 1
            when "0010" => seven_seg_L <= "00100101"; -- Hex 25 displays a 2
            when "0011" => seven_seg_L <= "00001101"; -- Hex 0d displays a 3
            when "0100" => seven_seg_L <= "10011001"; -- Hex 99 displays a 4
            when "0101" => seven_seg_L <= "01001001"; -- Hex 49 displays a 5
            when "0110" => seven_seg_L <= "01000001"; -- Hex 41 displays a 6
            when "0111" => seven_seg_L <= "00011111"; -- Hex 1f displays a 7
            when "1000" => seven_seg_L <= "00000001"; -- Hex 01 displays a 8
            when "1001" => seven_seg_L <= "00001001"; -- Hex 09 displays a 9
            when "1010" => seven_seg_L <= "00010001"; -- Hex 11 displays a A
            when "1011" => seven_seg_L <= "11000001"; -- Hex c1 displays a b
            when "1100" => seven_seg_L <= "01100011"; -- Hex 63 displays a C
            when "1101" => seven_seg_L <= "10000101"; -- Hex 85 displays a d
            when "1110" => seven_seg_L <= "01100001"; -- Hex 61 displays a E
            when "1111" => seven_seg_L <= "01110001"; -- Hex 71 displays a F
            when others => seven_seg_L <= "00000001"; -- Hex 01 displays a 8
         end case;
      end if;
   end process;



end architecture;