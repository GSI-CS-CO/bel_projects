-- TITLE "'PU_Reset' => Autor: R.Hartmann, Stand: 15.04.09, Vers: V02 ";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_unsigned.all;

entity PU_Reset is
	generic
	(
			PU_Reset_in_clks	: INTEGER := 5
	);

	port
	(
			Clk				: IN	STD_LOGIC;
			PU_Res			: OUT	STD_LOGIC
	);

	FUNCTION How_many_Bits (int: INTEGER) RETURN INTEGER IS

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


	CONSTANT	C_PU_Reset_cnt			: INTEGER	:= PU_Reset_in_clks;
	CONSTANT	C_cnt_pu_Reset_width	: INTEGER	:= How_many_bits(C_PU_Reset_cnt);
	SIGNAL		S_cnt_pu_Reset			: STD_LOGIC_VECTOR(C_cnt_pu_Reset_width DOWNTO 0) := (OTHERS => '0');
	SIGNAL		S_PU_Res				: STD_LOGIC := '0';

END PU_Reset;



architecture arch_PU_Reset of PU_Reset is


BEGIN

ASSERT NOT(PU_Reset_in_clks < 2 )
	REPORT " PU-Reset-Pulsbreite muss >= 2 CLK's sein"
SEVERITY ERROR;


	p_PU_Reset_cnt:	PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF S_cnt_pu_Reset < C_PU_Reset_cnt THEN
				S_cnt_pu_Reset <= S_cnt_pu_Reset + 1;
				S_PU_Res <= '1';			-- Zähler hat C_PU_Reset_cnt noch nicht erreicht -> S_PU_Res ist aktiv.
			ELSE
				S_PU_Res <= '0';			-- Zähler hat C_PU_Reset_cnt erreicht S_PU_Res wird inaktiv.
			END IF;
		END IF;
	END PROCESS p_PU_Reset_cnt;


PU_Res <= S_PU_Res;							-- Getaktetes Register S_PU_Res mit dem Ausgang PU_Res verbinden

END arch_PU_Reset;


















