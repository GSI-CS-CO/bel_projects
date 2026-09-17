--TITLE "'io_spi_dac_8420' Autor: R.Hartmann";


Library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use IEEE.STD_LOGIC_unsigned.ALL;


library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.fg_quad_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;




ENTITY io_spi_dac_8420 IS
	generic
		(
      Base_addr:	      INTEGER := 0;
			CLK_in_Hz:				INTEGER := 50000000;
			SPI_CLK_in_Hz:		INTEGER := 9000000;
			Clr_Midscale:			INTEGER := 1
		);

	port(
		Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);	-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);	-- latched data from SCU_Bus
		Ext_Adr_Val:			  in		std_logic;								      -- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:		  in		std_logic;								      -- '1' => Rd-Cycle is active
		Ext_Rd_fin:				  in		std_logic;								      -- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:		  in		std_logic;								      -- '1' => Wr-Cycle is active
		Ext_Wr_fin:				  in		std_logic;								      -- marks end of write cycle, active one for one clock period of sys_clk
		clk:						    in		std_logic;								      -- should be the same clk, used by SCU_Bus_Slave
		nReset:					    in		std_logic;
--
		SPI_DO:				      out	std_logic;
		SPI_CLK:			      out	std_logic;
		nCS_DAC1:			      out	std_logic;
		nCS_DAC2:			      out	std_logic;
		nLD_DAC:		        out	std_logic;
		CLR_Sel_DAC:	      out	std_logic;
		nCLR_DAC:			      out	std_logic;
--
		DAC_Status:		      out	std_logic_vector( 7 downto 0);	  -- Busy-Flag's
--
		Reg_rd_active:		  out	std_logic;								        -- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		    out	std_logic_vector(15 downto 0);	  -- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		  out	std_logic								          -- connect Dtack to SCUB-Macro
		);
	end io_spi_dac_8420;



ARCHITECTURE Arch_io_spi_dac_8420 OF io_spi_dac_8420 IS

constant	addr_width:					  INTEGER := Adr_from_SCUB_LA'length;
constant	DAC_IO_1_addr_offset:	INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_1 Registers
constant	DAC_IO_2_addr_offset:	INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_2 Registers
constant	DAC_IO_3_addr_offset:	INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_3 Registers
constant	DAC_IO_4_addr_offset:	INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_4 Registers
constant	DAC_IO_5_addr_offset:	INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_5 Registers
constant	DAC_IO_6_addr_offset:	INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_6 Registers
constant	DAC_IO_7_addr_offset:	INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_7 Registers
constant	DAC_IO_8_addr_offset:	INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des DAC_IO_8 Registers
--
constant	C_DAC_IO_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC_IO_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_1 Registers
constant	C_DAC_IO_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_2 Registers
constant	C_DAC_IO_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC_IO_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_3 Registers
constant	C_DAC_IO_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_4 Registers
constant	C_DAC_IO_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_5 Registers
constant	C_DAC_IO_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_6 Registers
constant	C_DAC_IO_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_7 Registers
constant	C_DAC_IO_8_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + DAC_IO_8_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des DAC_IO_8 Registers



TYPE      t_Dac_Data    is array (0 to 15) of std_logic_vector(15 downto 0);
TYPE      t_Bit_Array   is array (0 to 7)  of std_logic;

signal		s_DAC_IO:		      t_Dac_Data;                       -- Array für DAC-Sollwerte
signal		s_DAC_new_Data:		std_logic_vector(7 downto 0);     -- Array für die DAC-Update-Status
signal		S_DAC_SPI_send:		std_logic_vector(7 downto 0);     -- Array, wenn die DAC-Daten gesendet wurden

TYPE		  T_DAC_Data_SM	IS	(Idle, DAC_Load, DAC_1_4, DAC_5_8, DAC_wait_lo, DAC_wait_hi, DAC_End);
SIGNAL		DAC_Data_SM			  : T_DAC_Data_SM;

signal		S_ena_nCS_DAC1:	std_logic;
signal		S_ena_nCS_DAC2:	std_logic;


signal		S_DAC_IO_1_Rd:	std_logic;
signal		S_DAC_IO_1_Wr:	std_logic;

signal		S_DAC_IO_2_Rd:	std_logic;
signal		S_DAC_IO_2_Wr:	std_logic;

signal		S_DAC_IO_3_Rd:	std_logic;
signal		S_DAC_IO_3_Wr:	std_logic;

signal		S_DAC_IO_4_Rd:	std_logic;
signal		S_DAC_IO_4_Wr:	std_logic;

signal		S_DAC_IO_5_Rd:	std_logic;
signal		S_DAC_IO_5_Wr:	std_logic;

signal		S_DAC_IO_6_Rd:	std_logic;
signal		S_DAC_IO_6_Wr:	std_logic;

signal		S_DAC_IO_7_Rd:	std_logic;
signal		S_DAC_IO_7_Wr:	std_logic;

signal		S_DAC_IO_8_Rd:	std_logic;
signal		S_DAC_IO_8_Wr:	std_logic;

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);


------------------------------------------------------------------------------------------------------------------------------
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

	SIGNAL		S_Shift_Data_Input  : STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Shift_Reg			    : STD_LOGIC_VECTOR(16 DOWNTO 0);
	SIGNAL		S_Wr_Shift_Reg		  : STD_LOGIC;
	SIGNAL		S_Wr_DAC_Cntrl		  : STD_LOGIC;
	SIGNAL		S_Rd_DAC_Cntrl		  : STD_LOGIC;

	TYPE		  T_SPI_SM	IS	(Idle, Sel_On, Clk_Lo, Clk_Hi, Sel_Off, Load, Load_End);

	SIGNAL		SPI_SM			    : T_SPI_SM;

	SIGNAL		S_Bit_Cnt		    : STD_LOGIC_VECTOR(4 DOWNTO 0);

	SIGNAL		S_CS_DAC		    : STD_LOGIC;
	SIGNAL		S_SPI_CLK		    : STD_LOGIC;
	SIGNAL		S_nLD_DAC		    : STD_LOGIC;
	SIGNAL		S_CLR_Midscale	: STD_LOGIC;
	SIGNAL		S_nCLR_DAC		  : STD_LOGIC;
	SIGNAL		S_t_nCLR_DAC	  : STD_LOGIC;

	CONSTANT	C_SPI_CLK_Ena_Cnt		    : INTEGER := CLK_in_Hz / SPI_CLK_in_Hz / 2;
	CONSTANT	C_SPI_CLK_Ena_Cnt_width : INTEGER := How_many_Bits(C_SPI_CLK_Ena_Cnt);
--	SIGNAL		S_SPI_CLK_Ena_Cnt		    : STD_LOGIC_VECTOR(C_SPI_CLK_Ena_Cnt_width DOWNTO 0) := unsigned(0, C_SPI_CLK_Ena_Cnt_width + 1); --conv_std_logic_vector

	SIGNAL		S_SPI_CLK_Ena_Cnt		    : STD_LOGIC_VECTOR(C_SPI_CLK_Ena_Cnt_width DOWNTO 0) := STD_LOGIC_VECTOR(to_unsigned(0, C_SPI_CLK_Ena_Cnt_width + 1)); --conv_std_logic_vector

	SIGNAL		S_SPI_TRM	      : STD_LOGIC;


------------------------------------------------------------------------------------------------------------------------------
begin

--  S_DAC_IO <= (others => (others => '0'));
--	S_CLR_Midscale <= '0';




P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_DAC_IO_1_Rd <= '0';
			S_DAC_IO_1_Wr <= '0';
			S_DAC_IO_2_Rd <= '0';
			S_DAC_IO_2_Wr <= '0';
			S_DAC_IO_3_Rd <= '0';
			S_DAC_IO_3_Wr <= '0';
			S_DAC_IO_4_Rd <= '0';
			S_DAC_IO_4_Wr <= '0';
			S_DAC_IO_5_Rd <= '0';
			S_DAC_IO_5_Wr <= '0';
			S_DAC_IO_6_Rd <= '0';
			S_DAC_IO_6_Wr <= '0';
			S_DAC_IO_7_Rd <= '0';
			S_DAC_IO_7_Wr <= '0';
			S_DAC_IO_8_Rd <= '0';
			S_DAC_IO_8_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';

		elsif rising_edge(clk) then
			S_DAC_IO_1_Rd <= '0';
			S_DAC_IO_1_Wr <= '0';
			S_DAC_IO_2_Rd <= '0';
			S_DAC_IO_2_Wr <= '0';
			S_DAC_IO_3_Rd <= '0';
			S_DAC_IO_3_Wr <= '0';
			S_DAC_IO_4_Rd <= '0';
			S_DAC_IO_4_Wr <= '0';
			S_DAC_IO_5_Rd <= '0';
			S_DAC_IO_5_Wr <= '0';
			S_DAC_IO_6_Rd <= '0';
			S_DAC_IO_6_Wr <= '0';
			S_DAC_IO_7_Rd <= '0';
			S_DAC_IO_7_Wr <= '0';
			S_DAC_IO_8_Rd <= '0';
			S_DAC_IO_8_Wr <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';

			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_DAC_IO_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_1_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_1_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_DAC_IO_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_2_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_2_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_DAC_IO_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_3_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_3_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_DAC_IO_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_4_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_4_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_DAC_IO_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_5_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_5_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_DAC_IO_6_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_6_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_6_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_DAC_IO_7_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_7_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_7_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_DAC_IO_8_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_8_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_DAC_IO_8_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when others =>

						S_DAC_IO_1_Rd <= '0';
						S_DAC_IO_1_Wr <= '0';
						S_DAC_IO_2_Rd <= '0';
						S_DAC_IO_2_Wr <= '0';
						S_DAC_IO_3_Rd <= '0';
						S_DAC_IO_3_Wr <= '0';
						S_DAC_IO_4_Rd <= '0';
						S_DAC_IO_4_Wr <= '0';
						S_DAC_IO_5_Rd <= '0';
						S_DAC_IO_5_Wr <= '0';
						S_DAC_IO_6_Rd <= '0';
						S_DAC_IO_6_Wr <= '0';
						S_DAC_IO_7_Rd <= '0';
						S_DAC_IO_7_Wr <= '0';
						S_DAC_IO_8_Rd <= '0';
						S_DAC_IO_8_Wr <= '0';

						S_Dtack <= '0';
						Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;

	end process P_Adr_Deco;


P_DAC_IO:	process (nReset, clk)
	begin
		if nReset = '0' then
    -- KK Default 2.5V um Fehlfunktion (Trigger auf Null Volt) zu unterdruecken
		  S_DAC_IO(0) <=  "0000110000000000";
		  S_DAC_IO(1) <=  "0000110000000000";
		  S_DAC_IO(2) <=  "0000110000000000";
		  S_DAC_IO(3) <=  "0000110000000000";
		  S_DAC_IO(4) <=  "0000110000000000";
		  S_DAC_IO(5) <=  "0000110000000000";
		  S_DAC_IO(6) <=  "0000110000000000";
		  S_DAC_IO(7) <=  "0000110000000000";
		elsif rising_edge(clk) then
			if S_DAC_IO_1_Wr = '1' then	S_DAC_IO(0) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_2_Wr = '1' then	S_DAC_IO(1) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_3_Wr = '1' then	S_DAC_IO(2) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_4_Wr = '1' then	S_DAC_IO(3) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_5_Wr = '1' then	S_DAC_IO(4) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_6_Wr = '1' then	S_DAC_IO(5) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_7_Wr = '1' then	S_DAC_IO(6) <= Data_from_SCUB_LA;
			end if;
			if S_DAC_IO_8_Wr = '1' then	S_DAC_IO(7) <= Data_from_SCUB_LA;
			end if;
	end if;
	end process P_DAC_IO;


	P_read_mux:	process (S_DAC_IO_1_Rd, S_DAC_IO_2_Rd, S_DAC_IO_3_Rd, S_DAC_IO_4_Rd,
                       S_DAC_IO_5_Rd, S_DAC_IO_6_Rd, S_DAC_IO_7_Rd, S_DAC_IO_8_Rd, S_DAC_IO)
	begin
		if 	  S_DAC_IO_1_Rd = '1' then	S_Read_port <= S_DAC_IO(0);
		elsif S_DAC_IO_2_Rd = '1' then	S_Read_port <= S_DAC_IO(1);
		elsif S_DAC_IO_3_Rd = '1' then	S_Read_port <= S_DAC_IO(2);
		elsif S_DAC_IO_4_Rd = '1' then	S_Read_port <= S_DAC_IO(3);
		elsif S_DAC_IO_5_Rd = '1' then	S_Read_port <= S_DAC_IO(4);
		elsif S_DAC_IO_6_Rd = '1' then	S_Read_port <= S_DAC_IO(5);
		elsif S_DAC_IO_7_Rd = '1' then	S_Read_port <= S_DAC_IO(6);
		elsif S_DAC_IO_8_Rd = '1' then	S_Read_port <= S_DAC_IO(7);
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;


Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;

--------------------------------------------------------------------------------------------------


P_DAC_Update_Sts:	PROCESS (clk, nReset, S_DAC_IO_1_Wr, S_DAC_SPI_send)
	BEGIN
		IF nReset = '0' THEN
      s_DAC_new_Data      <=  (OTHERS => '0');     -- Array für die DAC-Update-Status
		ELSIF rising_edge(clk) THEN

      if 	  (S_DAC_IO_1_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(0) <= '1'; -- set Daten-Flag nach dem schreiben
      elsif (S_DAC_IO_2_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(1) <= '1';
      elsif (S_DAC_IO_3_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(2) <= '1';
      elsif (S_DAC_IO_4_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(3) <= '1';
      elsif (S_DAC_IO_5_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(4) <= '1';
      elsif (S_DAC_IO_6_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(5) <= '1';
      elsif (S_DAC_IO_7_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(6) <= '1';
      elsif (S_DAC_IO_8_Wr and Ext_Wr_fin)   then	S_DAC_new_Data(7) <= '1';
      ELSE
      if 	  S_DAC_SPI_send(0) = '1' then	S_DAC_new_Data(0) <= '0'; -- reset Daten-Flag nach dem Start (SPI-send)
      elsif S_DAC_SPI_send(1) = '1' then	S_DAC_new_Data(1) <= '0';
      elsif S_DAC_SPI_send(2) = '1' then	S_DAC_new_Data(2) <= '0';
      elsif S_DAC_SPI_send(3) = '1' then	S_DAC_new_Data(3) <= '0';
      elsif S_DAC_SPI_send(4) = '1' then	S_DAC_new_Data(4) <= '0';
      elsif S_DAC_SPI_send(5) = '1' then	S_DAC_new_Data(5) <= '0';
      elsif S_DAC_SPI_send(6) = '1' then	S_DAC_new_Data(6) <= '0';
      elsif S_DAC_SPI_send(7) = '1' then	S_DAC_new_Data(7) <= '0';
      end if;
    end if;
		END IF;
	END PROCESS P_DAC_Update_Sts;




P_Dac_Data_Loop:  process (clk, nReset, DAC_Data_SM)

    begin
      if (nReset = '0') then
        DAC_Data_SM         <= Idle;
        S_Wr_Shift_Reg	    <= '0';               ----- Reset Start SPI
        S_Shift_Data_Input  <= (OTHERS => '0');   ----- Reset SPI Daten
        S_DAC_SPI_send	    <= (OTHERS => '0');   ----- Reset Daten gesendet
        S_ena_nCS_DAC1	    <= '0';
        S_ena_nCS_DAC2	    <= '0';

    ELSIF rising_edge(clk) then
      case DAC_Data_SM is

        when Idle   =>        S_Wr_Shift_Reg	    <= '0';               ----- Reset Start SPI
                              S_Shift_Data_Input  <= (OTHERS => '0');   ----- Reset SPI Daten
                              S_DAC_SPI_send	    <= (OTHERS => '0');   ----- Reset Daten gesendet

                              if  (S_DAC_new_Data /= x"00")  then
                                  DAC_Data_SM   <= DAC_Load;
                              else
                                  DAC_Data_SM          <= Idle;
                              end if;


        when DAC_Load    =>   if    S_DAC_new_Data(0)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('0'&'0'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(0)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(0)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_1_4;

                              elsif S_DAC_new_Data(1)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('0'&'1'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(1)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(1)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_1_4;

                              elsif S_DAC_new_Data(2)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('1'&'0'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(2)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(2)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_1_4;

                              elsif S_DAC_new_Data(3)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('1'&'1'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(3)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(3)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_1_4;

                              elsif S_DAC_new_Data(4)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('0'&'0'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(4)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(4)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_5_8;

                              elsif S_DAC_new_Data(5)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('0'&'1'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(5)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(5)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_5_8;

                              elsif S_DAC_new_Data(6)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('1'&'0'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(6)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(6)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_5_8;

                              elsif S_DAC_new_Data(7)    = '1'  then --
                                    S_Shift_Data_Input(15 downto 12) <= ('1'&'1'&'0'&'0');             -- [15..14]=DAC-Adresse, [13..12]=frei
                                    S_Shift_Data_Input(11 downto 0)  <= S_DAC_IO(7)(11 downto 0);      -- SPI Daten
                                    S_DAC_SPI_send(7)	  <= '1';                                        -- Reset Daten Send Flag
                                    DAC_Data_SM         <= DAC_5_8;
                              else
                                 DAC_Data_SM         <= Idle;
                              end if;


        when DAC_1_4     =>   S_Wr_Shift_Reg	    <= '1';            ----- Set Start, S_Wr_Shift_Reg=1
                              S_ena_nCS_DAC1	    <= '1';            ----- Enable CS_DAC1
                              DAC_Data_SM         <= DAC_wait_lo;


        when DAC_5_8     =>   S_Wr_Shift_Reg	    <= '1';            ----- Set Start, S_Wr_Shift_Reg=1
                              S_ena_nCS_DAC2	    <= '1';            ----- Enable CS_DAC2
                              DAC_Data_SM         <= DAC_wait_lo;


        when DAC_wait_lo =>   S_Wr_Shift_Reg	    <= '0';            ----- Set Stop, S_Wr_Shift_Reg=0
                              if (S_nLD_DAC = '1') then              ----- warte bis Ende der Übertragung, Load=0
                                DAC_Data_SM       <= DAC_wait_lo;
                              else
                                DAC_Data_SM       <= DAC_wait_hi;
                              end if;

        when DAC_wait_hi =>   if (S_nLD_DAC = '0') then              ----- warte bis Ende der Übertragung, Load=1
                                DAC_Data_SM       <= DAC_wait_hi;
                              else
                                  S_ena_nCS_DAC1	<= '0';            ----- Disable CS_DAC1
                                  S_ena_nCS_DAC2	<= '0';            ----- Disable CS_DAC2
                                  DAC_Data_SM     <= DAC_End;
                              end if;


        when DAC_End     =>   DAC_Data_SM         <= Idle;



        when others =>        DAC_Data_SM         <= Idle;

      end case;
    end if;
  end process P_Dac_Data_Loop;




---------------------------- SPI ----------------------------

P_SPI_CLK_Ena:	PROCESS (clk, nReset)
	BEGIN
		IF nReset = '0' THEN
			S_SPI_CLK_Ena_Cnt <= std_logic_vector(to_unsigned(C_SPI_CLK_Ena_Cnt-1, S_SPI_CLK_Ena_Cnt'length));
--		S_SPI_CLK_Ena_Cnt <= conv_std_logic_vector(C_SPI_CLK_Ena_Cnt-1, S_SPI_CLK_Ena_Cnt'length);
		ELSIF rising_edge(clk) THEN
			IF S_SPI_CLK_Ena_Cnt(S_SPI_CLK_Ena_Cnt'high) = '1' THEN
				S_SPI_CLK_Ena_Cnt <= std_logic_vector(to_unsigned(C_SPI_CLK_Ena_Cnt-1, S_SPI_CLK_Ena_Cnt'length));
--			S_SPI_CLK_Ena_Cnt <= conv_std_logic_vector(C_SPI_CLK_Ena_Cnt-1, S_SPI_CLK_Ena_Cnt'length);
			ELSE
				S_SPI_CLK_Ena_Cnt <= S_SPI_CLK_Ena_Cnt - 1;				-- count down
			END IF;
		END IF;
	END PROCESS P_SPI_CLK_Ena;

P_SPI_SM:	PROCESS (clk, nReset, S_nCLR_DAC)

	BEGIN
	    IF  nReset = '0' OR S_nCLR_DAC = '0' THEN
			SPI_SM <= Idle;
			S_Bit_Cnt <= (OTHERS => '0');
			S_CS_DAC  <= '0';
			S_SPI_CLK <= '1';
			S_nLD_DAC <= '1';
			S_SPI_TRM <= '0';

	    ELSIF rising_edge(clk) THEN

			IF S_Wr_Shift_Reg = '1' THEN
				S_SPI_TRM <= '1';
			END IF;

			IF S_SPI_CLK_Ena_Cnt(S_SPI_CLK_Ena_Cnt'high) = '1' THEN

			 	CASE SPI_SM IS

				   	WHEN Idle =>
						S_Bit_Cnt <= (OTHERS => '0');
						S_CS_DAC  <= '0';
						S_SPI_CLK <= '1';
						S_nLD_DAC <= '1';
						IF S_SPI_TRM = '1' THEN
							SPI_SM <= Sel_On;
						END IF;

					WHEN Sel_On =>
						S_CS_DAC  <= '1';
						SPI_SM <= CLK_Lo;

					WHEN CLK_Lo =>
						IF S_Bit_Cnt <= 15 THEN
							S_SPI_CLK <= '0';
							SPI_SM <= CLK_Hi;
						ELSE
							S_SPI_CLK <= '1';
							SPI_SM <= Sel_Off;
						END IF;

					WHEN CLK_Hi =>
						S_SPI_CLK <= '1';
						S_Bit_Cnt <= S_Bit_Cnt + 1;
						SPI_SM <= CLK_Lo;

					WHEN Sel_Off =>
						S_CS_DAC  <= '0';
						SPI_SM <= Load;

					WHEN Load =>
						S_nLD_DAC <= '0';
						SPI_SM <= Load_End;

					WHEN Load_End =>
						S_SPI_TRM <= '0';
						S_nLD_DAC <= '0';
						SPI_SM <= Idle;

			   	END CASE;

			END IF;

	    END IF;

	END PROCESS P_SPI_SM;


P_Shift_Reg:	PROCESS (clk, nReset)
	BEGIN
		IF  nReset = '0' THEN
			S_Shift_Reg <= (OTHERS => '0');
		ELSIF rising_edge(clk) THEN
			IF S_Wr_Shift_Reg = '1' THEN
				S_Shift_Reg <= ('0' & S_Shift_Data_Input);
			ELSIF SPI_SM = CLK_Lo AND S_SPI_CLK_Ena_Cnt(S_SPI_CLK_Ena_Cnt'high) = '1' THEN
				S_Shift_Reg <= (S_Shift_Reg(S_Shift_Reg'high-1 DOWNTO 0) & '0');
			END IF;
		END IF;
	END PROCESS P_Shift_Reg;



P_DAC_Cntrl:	PROCESS (clk, nReset)
	BEGIN
		IF  nReset          = '0' THEN
				S_CLR_Midscale <= '0';
			S_nCLR_DAC <= '0';
			IF CLR_Midscale = 1 THEN
				S_CLR_Midscale <= '1';
			ELSE
				S_CLR_Midscale <= '0';
			END IF;
		ELSIF rising_edge(clk) THEN
			S_nCLR_DAC <= '1';                        -- +
			IF S_Wr_DAC_Cntrl = '1' THEN              -- |
				IF S_Shift_Data_Input(1) = '1' THEN     -- |
					S_nCLR_DAC <= '0';                    -- |
				END IF;                                 -- +--> wird z.Z. nicht verwendet
				IF S_Shift_Data_Input(2) = '1' THEN     -- |
					S_CLR_Midscale <= '1';                -- |
				ELSE                                    -- |
					S_CLR_Midscale <= '0';                -- +
				END IF;
			END IF;
		END IF;
	END PROCESS P_DAC_Cntrl;




  nCS_DAC1	  <= not (S_CS_DAC and S_ena_nCS_DAC1);
  nCS_DAC2	  <= not (S_CS_DAC and S_ena_nCS_DAC2);
  SPI_DO	    <= S_Shift_Reg(16);
  SPI_CLK	    <= S_SPI_CLK;
  nLD_DAC	    <= S_nLD_DAC;
  CLR_Sel_DAC <= '1'; ------------------- DAC-Output = 0V, nach dem Reset (set int. Reg. = 0800Hex)
--CLR_Sel_DAC <= S_CLR_Midscale;
  nCLR_DAC    <= S_nCLR_DAC;


	DAC_Status(7 downto 0)  <= S_DAC_new_Data(7 downto 0);	  -- Busy-Flag's


end Arch_io_spi_dac_8420;
