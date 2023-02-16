--TITLE "'atr_comp_ctrl' Autor: R.Hartmann";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY atr_comp_ctrl IS
	generic
		(
		Base_addr:	INTEGER := 16#0610#
		);

	port(
		Adr_from_SCUB_LA:		in  std_logic_vector(15 downto 0);	-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in  std_logic_vector(15 downto 0);	-- latched data from SCU_Bus
		Ext_Adr_Val:			  in  std_logic;								      -- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in  std_logic;								      -- '1' => Rd-Cycle is active
		Ext_Rd_fin:				  in  std_logic;								      -- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in  std_logic;								      -- '1' => Wr-Cycle is active
		Ext_Wr_fin:				  in  std_logic;								      -- marks end of write cycle, active one for one clock period of sys_clk
		clk:						    in  std_logic;								      -- should be the same clk, used by SCU_Bus_Slave
		nReset:					    in  std_logic;
		clk_250mhz:			    in  std_logic;								      -- signal_tap_clk_250mhz
		nReset_250mhz:	    in  std_logic;
--
    ATR_comp_puls:        in  STD_LOGIC_VECTOR(7 DOWNTO 0);   -- Ausgänge von den Comperatoren
    ATR_comp_cnt_err_res: in  STD_LOGIC;                      -- Reset Counter und Error-Flags
    ATR_comp_cnt_error:   out std_logic_VECTOR(7 DOWNTO 0);
--
		Reg_rd_active:		  out	std_logic;								      -- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		    out	std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		  out	std_logic								        -- connect Dtack to SCUB-Macro
		);
	end atr_comp_ctrl;



ARCHITECTURE Arch_atr_comp_ctrl OF atr_comp_ctrl IS

constant	addr_width:					INTEGER := Adr_from_SCUB_LA'length;
constant	ATR_comp_cnt_1_addr_offset:	INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_1 Registers
constant	ATR_comp_cnt_2_addr_offset:	INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_2 Registers
constant	ATR_comp_cnt_3_addr_offset:	INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_3 Registers
constant	ATR_comp_cnt_4_addr_offset:	INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_4 Registers
constant	ATR_comp_cnt_5_addr_offset:	INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_5 Registers
constant	ATR_comp_cnt_6_addr_offset:	INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_6 Registers
constant	ATR_comp_cnt_7_addr_offset:	INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_7 Registers
constant	ATR_comp_cnt_8_addr_offset:	INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_comp_cnt_8 Registers
--
constant	C_ATR_comp_cnt_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_comp_cnt_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_1 Registers
constant	C_ATR_comp_cnt_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_2 Registers
constant	C_ATR_comp_cnt_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_comp_cnt_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_3 Registers
constant	C_ATR_comp_cnt_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_4 Registers
constant	C_ATR_comp_cnt_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_5 Registers
constant	C_ATR_comp_cnt_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_6 Registers
constant	C_ATR_comp_cnt_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_7 Registers
constant	C_ATR_comp_cnt_8_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_comp_cnt_8_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_comp_cnt_8 Registers


signal		S_ATR_comp_cnt_1:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_1_Rd:	std_logic;

signal		S_ATR_comp_cnt_2:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_2_Rd:	std_logic;

signal		S_ATR_comp_cnt_3:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_3_Rd:	std_logic;

signal		S_ATR_comp_cnt_4:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_4_Rd:	std_logic;

signal		S_ATR_comp_cnt_5:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_5_Rd:	std_logic;

signal		S_ATR_comp_cnt_6:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_6_Rd:	std_logic;

signal		S_ATR_comp_cnt_7:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_7_Rd:	std_logic;

signal		S_ATR_comp_cnt_8:		  std_logic_vector(15 downto 0);
signal		S_ATR_comp_cnt_8_Rd:	std_logic;

signal		S_Dtack:				      std_logic;
signal		S_Read_Port:		      std_logic_vector(Data_to_SCUB'range);



TYPE      t_Word_Array          is array (0 to 7) of std_logic_vector(15 downto 0);
signal		ATR_comp_cnt:         t_Word_Array;   -- Counter für Pulsbreite

signal		ATR_cnt_puls:         std_logic;
signal		ATR_counter:          std_logic_vector(15 downto 0);




COMPONENT atr_cnt_n
  PORT
  (
    clk:                  IN  STD_LOGIC;
    nReset:               IN  STD_LOGIC;
    clk_250mhz:           IN  STD_LOGIC;
    nReset_250mhz:        IN  STD_LOGIC;

    ATR_cnt_puls:         IN  STD_LOGIC;
    ATR_comp_cnt_err_res: in  STD_LOGIC;   -- Reset Counter und Error-Flags
--
    ATR_counter:          OUT std_logic_VECTOR(15 DOWNTO 0);
    ATR_cnt_err:          OUT STD_LOGIC
  );
END COMPONENT atr_cnt_n;


begin


ATR_Cnt:  for I in 0 to 7 generate
    Cnt_I:  atr_cnt_n
          port map(clk                  => clk,                   -- Sys-Clock
                  nReset                => nReset,                 -- Powerup-Reset
                  clk_250mhz            => clk_250mhz,             -- Clock
                  nReset_250mhz         => nReset_250mhz,          -- Powerup-Reset (250Mhz)
                  ATR_cnt_puls          => ATR_comp_puls(i),       -- Comperatorpuls
                  ATR_comp_cnt_err_res  => ATR_comp_cnt_err_res,   -- Reset Counter und Error-Flags
                  ATR_counter           => ATR_comp_cnt(i),        -- Counterstand der Pulsbreite (4ns)
                  ATR_cnt_err           => ATR_comp_cnt_error(i)); -- Counterstand größer 16Bit
          end generate ATR_Cnt;



P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_ATR_comp_cnt_1_Rd <= '0';
			S_ATR_comp_cnt_2_Rd <= '0';
			S_ATR_comp_cnt_3_Rd <= '0';
			S_ATR_comp_cnt_4_Rd <= '0';
			S_ATR_comp_cnt_5_Rd <= '0';
			S_ATR_comp_cnt_6_Rd <= '0';
			S_ATR_comp_cnt_7_Rd <= '0';
			S_ATR_comp_cnt_8_Rd <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';

		elsif rising_edge(clk) then
			S_ATR_comp_cnt_1_Rd <= '0';
			S_ATR_comp_cnt_2_Rd <= '0';
			S_ATR_comp_cnt_3_Rd <= '0';
			S_ATR_comp_cnt_4_Rd <= '0';
			S_ATR_comp_cnt_5_Rd <= '0';
			S_ATR_comp_cnt_6_Rd <= '0';
			S_ATR_comp_cnt_7_Rd <= '0';
			S_ATR_comp_cnt_8_Rd <= '0';

			S_Dtack <= '0';
			Reg_rd_active <= '0';

			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_ATR_comp_cnt_1_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_1_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_ATR_comp_cnt_2_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_2_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_ATR_comp_cnt_3_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_3_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when C_ATR_comp_cnt_4_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_4_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_ATR_comp_cnt_5_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_5_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_ATR_comp_cnt_6_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_6_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_ATR_comp_cnt_7_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_7_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

						when C_ATR_comp_cnt_8_Addr =>
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_comp_cnt_8_Rd <= '1';
							Reg_rd_active <= '1';
						end if;

					when others =>

						S_ATR_comp_cnt_1_Rd <= '0';
						S_ATR_comp_cnt_2_Rd <= '0';
						S_ATR_comp_cnt_3_Rd <= '0';
						S_ATR_comp_cnt_4_Rd <= '0';
						S_ATR_comp_cnt_5_Rd <= '0';
						S_ATR_comp_cnt_6_Rd <= '0';
						S_ATR_comp_cnt_7_Rd <= '0';
						S_ATR_comp_cnt_8_Rd <= '0';

						S_Dtack <= '0';
						Reg_rd_active <= '0';

				end CASE;
			end if;
		end if;

	end process P_Adr_Deco;




P_read_mux:	process (	S_ATR_comp_cnt_1_Rd, S_ATR_comp_cnt_2_Rd,
                      S_ATR_comp_cnt_3_Rd, S_ATR_comp_cnt_4_Rd,
                      S_ATR_comp_cnt_5_Rd, S_ATR_comp_cnt_6_Rd,
                      S_ATR_comp_cnt_7_Rd, S_ATR_comp_cnt_8_Rd, ATR_comp_cnt )

	begin
		if 	  S_ATR_comp_cnt_1_Rd = '1' then	S_Read_port <= ATR_comp_cnt(0); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_2_Rd = '1' then	S_Read_port <= ATR_comp_cnt(1); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_3_Rd = '1' then	S_Read_port <= ATR_comp_cnt(2); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_4_Rd = '1' then	S_Read_port <= ATR_comp_cnt(3); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_5_Rd = '1' then	S_Read_port <= ATR_comp_cnt(4); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_6_Rd = '1' then	S_Read_port <= ATR_comp_cnt(5); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_7_Rd = '1' then	S_Read_port <= ATR_comp_cnt(6); -- Counter für Pulsbreite
		elsif S_ATR_comp_cnt_8_Rd = '1' then	S_Read_port <= ATR_comp_cnt(7); -- Counter für Pulsbreite
	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;


Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB  <= S_Read_Port;

end Arch_atr_comp_ctrl;