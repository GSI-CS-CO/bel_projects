
LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;


ENTITY f_divider IS
	GENERIC(
			cnt			: INTEGER := 4;
			Use_LPM		: INTEGER := 1
			);

    PORT ( clk  		: IN  STD_LOGIC;
           ena_cnt		: IN  STD_LOGIC;
		   sclr			: IN  STD_LOGIC;
           f_div        : OUT STD_LOGIC
         );    
END f_divider;    

ARCHITECTURE arch_f_divider OF f_divider IS

    FUNCTION	How_many_Bits  (int: INTEGER) RETURN INTEGER IS

		VARIABLE i, tmp : INTEGER;

	BEGIN
		tmp		:= int;
		i		:= 0;
		WHILE tmp > 0 LOOP
			tmp := tmp / 2;
			i := i + 1;
		END LOOP;
		RETURN i;
	END How_many_bits;
	
	COMPONENT lpm_counter
		GENERIC (
				lpm_width		: NATURAL;
				lpm_type		: STRING;
				lpm_direction	: STRING;
				lpm_svalue		: STRING
				);
		PORT(
			clock	: IN STD_LOGIC ;
			cnt_en	: IN STD_LOGIC := '1';
			q		: OUT STD_LOGIC_VECTOR (lpm_width-1 DOWNTO 0);
			sset	: IN STD_LOGIC
			);
	END COMPONENT;
	
	
	CONSTANT	c_counter_width 	: INTEGER := How_many_Bits(cnt);
	
	SIGNAL		s_counter			: STD_LOGIC_VECTOR(c_counter_width downto 0) := conv_std_logic_vector(0 ,c_counter_width+1);
	SIGNAL		s_cnt_ld			: STD_LOGIC;
	
	CONSTANT	c_ld_value			: INTEGER := cnt - 2;

BEGIN

div_without_lpm: IF Use_LPM = 0 GENERATE
BEGIN
p_f_divider:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk='1' THEN
			IF sclr = '1' THEN
				s_counter <= conv_std_logic_vector(c_ld_value, s_counter'length);
			ELSIF s_counter(s_counter'high) = '1' THEN
				s_counter <= conv_std_logic_vector(c_ld_value, s_counter'length);
			ELSIF ena_cnt = '1' THEN
				IF s_counter(s_counter'high) = '0' THEN		-- nur wenn MSB gelöscht ist, wird gezaehlt
					s_counter <= s_counter - 1;				-- count down
				END IF;
			END IF;
		END IF;
	END PROCESS p_f_divider;
END GENERATE div_without_lpm;

s_cnt_ld <= s_counter(s_counter'high) OR sclr;

div_with_lpm: IF Use_LPM = 1 GENERATE
f_divider : lpm_counter
	GENERIC MAP (
				lpm_width	=> s_counter'length,
				lpm_type	=> "LPM_COUNTER",
				lpm_direction => "DOWN",
				lpm_svalue	=> integer'image(c_ld_value) --"conv_std_logic_vector(c_ld_value, s_counter'length)"
				)
	PORT MAP(
			clock	=> clk,
			sset	=> s_cnt_ld,
			cnt_en	=> ena_cnt,
			q		=> s_counter
			);
END GENERATE div_with_lpm;

f_div <= s_counter(s_counter'high);

END arch_f_divider;    
