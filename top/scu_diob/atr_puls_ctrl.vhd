--TITLE "'atr_puls_ctrl' Autor: R.Hartmann";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY atr_puls_ctrl IS
	generic
		(
		Base_addr:	INTEGER := 16#0618#
		);
		
	port(
		Adr_from_SCUB_LA:		  in  std_logic_vector(15 downto 0);	-- latched address from SCU_Bus
		Data_from_SCUB_LA:	  in  std_logic_vector(15 downto 0);	-- latched data from SCU_Bus 
		Ext_Adr_Val:			    in  std_logic;								      -- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			  in  std_logic;								      -- '1' => Rd-Cycle is active
		Ext_Rd_fin:				    in  std_logic;								      -- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			  in  std_logic;								      -- '1' => Wr-Cycle is active
		Ext_Wr_fin:				    in  std_logic;								      -- marks end of write cycle, active one for one clock period of sys_clk
		clk:						      in  std_logic;								      -- should be the same clk, used by SCU_Bus_Slave
		nReset:					      in  std_logic;
		clk_250mhz:			      in  std_logic;								      -- signal_tap_clk_250mhz
		nReset_250mhz:	      in  std_logic;
--
    atr_puls_start:       IN  STD_LOGIC;                      -- Starte Ausgangspuls
    atr_puls_out:         out STD_LOGIC_VECTOR(7 DOWNTO 0);   -- Ausgangspuls Kanal 1..8
    atr_puls_config_err:  out STD_LOGIC_VECTOR(7 DOWNTO 0);   -- Config-Error: Pulsbreite/Pulsverzögerung

    ATR_comp_puls:        in  STD_LOGIC_VECTOR(7 DOWNTO 0);   -- Ausgänge von den Comperatoren für die Triggereingänge
    ATR_to_conf_err:  		out std_logic;								      -- Time-Out: Configurations-Error
		ATR_Timeout:  		    out	std_logic;								      -- Time-Out: Maximalzeit zwischen Start und Zündpuls überschritten.
    ATR_Timeout_err_res:  IN  STD_LOGIC;                      -- Reset Error-Flags
--
		Reg_rd_active:		    out	std_logic;								      -- read data available at 'Data_to_SCUB'-INL_Out
		Data_to_SCUB:		      out	std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:		    out	std_logic								        -- connect Dtack to SCUB-Macro
		);	
	end atr_puls_ctrl;

	

ARCHITECTURE Arch_atr_puls_ctrl OF atr_puls_ctrl IS

constant	addr_width:					      INTEGER := Adr_from_SCUB_LA'length;
constant	ATR_verz_0_addr_offset:	  INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_0 Registers
constant	ATR_verz_1_addr_offset:	  INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_1 Registers
constant	ATR_verz_2_addr_offset:	  INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_2 Registers
constant	ATR_verz_3_addr_offset:	  INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_3 Registers
constant	ATR_verz_4_addr_offset:	  INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_4 Registers
constant	ATR_verz_5_addr_offset:	  INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_5 Registers
constant	ATR_verz_6_addr_offset:	  INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_6 Registers
constant	ATR_verz_7_addr_offset:	  INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_verz_7 Registers

constant	ATR_pulsw_0_addr_offset:	INTEGER := 8;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-0 Registers
constant	ATR_pulsw_1_addr_offset:	INTEGER := 9;		-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-1 Registers
constant	ATR_pulsw_2_addr_offset:	INTEGER := 10;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-2 Registers
constant	ATR_pulsw_3_addr_offset:	INTEGER := 11;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-3 Registers
constant	ATR_pulsw_4_addr_offset:	INTEGER := 12;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-4 Registers
constant	ATR_pulsw_5_addr_offset:	INTEGER := 13;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-5 Registers
constant	ATR_pulsw_6_addr_offset:	INTEGER := 14;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-6 Registers
constant	ATR_pulsw_7_addr_offset:	INTEGER := 15;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_pulsw-7 Registers

constant	ATR_to_kanal_addr_offset:	INTEGER := 16;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_Timeout_Kanals
constant	ATR_to_count_addr_offset:	INTEGER := 17;	-- Offset zur Base_addr zum Setzen oder Rücklesen des ATR_Timeout_Counters
--
--
constant	C_ATR_verz_0_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_verz_0_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_0 Registers
constant	C_ATR_verz_1_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_1_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_1 Registers
constant	C_ATR_verz_2_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_verz_2_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_2 Registers
constant	C_ATR_verz_3_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_3_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_3 Registers
constant	C_ATR_verz_4_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_4_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_4 Registers
constant	C_ATR_verz_5_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_5_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_5 Registers
constant	C_ATR_verz_6_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_6_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_6 Registers
constant	C_ATR_verz_7_Addr: 	  unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_verz_7_addr_offset), addr_width);	  -- Adresse zum Setzen oder Rücklesen des ATR_verz_7 Registers

constant	C_ATR_pulsw_0_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_pulsw_0_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_0 Registers
constant	C_ATR_pulsw_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_1 Registers
constant	C_ATR_pulsw_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ATR_pulsw_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_2 Registers
constant	C_ATR_pulsw_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_3 Registers
constant	C_ATR_pulsw_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_4 Registers
constant	C_ATR_pulsw_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_5 Registers
constant	C_ATR_pulsw_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_6 Registers
constant	C_ATR_pulsw_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_pulsw_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_pulsw_7 Registers

constant	C_ATR_to_kanal_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_kanal_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_Timeout_Kanals
constant	C_ATR_to_count_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + ATR_to_count_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des ATR_Timeout_Counters


signal		S_ATR_verz_rd:	    std_logic_VECTOR(15 DOWNTO 0);
signal		S_ATR_verz_wr:	    std_logic_VECTOR(15 DOWNTO 0);
signal		S_ATR_pulsw_rd:	    std_logic_VECTOR(15 DOWNTO 0);
signal		S_ATR_pulsw_wr:	    std_logic_VECTOR(15 DOWNTO 0);

signal		S_ATR_to_kanal_rd:	std_logic;
signal		S_ATR_to_kanal_wr:	std_logic;
signal		S_ATR_to_kanal:	    std_logic_VECTOR(15 DOWNTO 0);

signal		S_ATR_to_Count_rd:	std_logic;
signal		S_ATR_to_Count_wr:	std_logic;
signal		S_ATR_to_Count:	    std_logic_VECTOR(15 DOWNTO 0);


TYPE      t_Word_Array    is array (0 to 7) of std_logic_vector(15 downto 0);
signal		S_ATR_verz:     t_Word_Array;   -- Zähler für Verzögerungszeit  
signal		S_ATR_pulsw:    t_Word_Array;   -- Zähler für Pulsbreite

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);

signal		ATR_verz:       std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Pulsbreite
signal		ATR_pulsw:      std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Pulsbreite




signal  Loop_Counter:    integer range 0 to 65535;    -- 0-FFFF -- Counter
signal  Timeout_Cnt:     integer range 0 to 65535;    -- 0-FFFF -- Counter
signal  Kanalnummer:     integer range 0 to 15;       -- Nummer des Timeout-Kanals

signal  s_config_err:    std_logic;                   -- Config-Error
signal  s_timeout:       std_logic;                   -- Timout-Flag


signal Puls_i:           std_logic_VECTOR(7 DOWNTO 0);   -- input  "Strobe-Signal"
signal Strobe_o:         std_logic_VECTOR(7 DOWNTO 0);   -- Output "Strobe-Signal (1 CLK breit)"
signal shift0:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift1:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift2:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift3:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift4:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift5:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift6:           std_logic_vector(2 downto 0);   -- Shift-Reg.
signal shift7:           std_logic_vector(2 downto 0);   -- Shift-Reg.



type type_t is   ( sm_idle, sm_start, sm_count, sm_loop, sm_config_err, sm_timeout, sm_end );
                  
signal sm_state:  type_t := sm_idle;


---------------------------------------------------------------------------------------------------         

COMPONENT atr_puls_n
  PORT
  (
    clk:                  IN  STD_LOGIC;
    nReset:               IN  STD_LOGIC;
--
    atr_puls_start:       IN  STD_LOGIC;                      -- Starte Ausgangspuls
  	ATR_verz:             IN  std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Verzögerung
  	ATR_pulsw:            IN  std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Pulsbreite
--
    atr_puls_out:         out STD_LOGIC;                      -- Ausgangspuls Kanal n
    atr_puls_config_err:  out STD_LOGIC);                     -- Config-Error: Pulsbreite/Pulsverzögerung

END COMPONENT atr_puls_n;







begin

------------------------------------- Outpuls, Kanlal 1-8 -----------------------------------------         
  
ATR_Puls:  for I in 0 to 7 generate
    Puls_I:  atr_puls_n
          port map
                 (clk                   => clk_250mhz,              -- Clock
                  nReset                => nReset_250mhz,           -- Powerup-Reset (250Mhz)
                  atr_puls_start        => atr_puls_start,          -- Starte Ausgangspuls
                  ATR_verz              => s_ATR_verz(i),           -- Counter für Verzögerung
                  ATR_pulsw             => s_ATR_pulsw(i),          -- Counter für Pulsbreite
                  atr_puls_out          => atr_puls_out(i),         -- Ausgangspuls Kanal n
                  atr_puls_config_err   => atr_puls_config_err(i)); -- Config-Error: Pulsbreite/Pulsverzögerung
          end generate ATR_Puls;

          
------------------------------------- Puls aus Input ATR_comp_puls -----------------------------------------         




--------- Strobes aus Pulsen (1 Clock breit) --------------------

p_Strobe0:  PROCESS (clk, nReset, Puls_i(0))
  BEGin
    IF nReset        = '0' THEN
      shift0        <= (OTHERS => '0');
      Strobe_o(0)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift0 <= (shift0(shift0'high-1 downto 0) & (Puls_i(0)));

      IF shift0(shift0'high) = '0' AND shift0(shift0'high-1) = '1' THEN
        Strobe_o(0) <= '1';
      ELSE
        Strobe_o(0) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe0;

p_Strobe1:  PROCESS (clk, nReset, Puls_i(1))
  BEGin
    IF nReset        = '0' THEN
      shift1        <= (OTHERS => '0');
      Strobe_o(1)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift1 <= (shift1(shift1'high-1 downto 0) & (Puls_i(1)));

      IF shift1(shift1'high) = '0' AND shift1(shift1'high-1) = '1' THEN
        Strobe_o(1) <= '1';
      ELSE
        Strobe_o(1) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe1;

p_Strobe2:  PROCESS (clk, nReset, Puls_i(2))
  BEGin
    IF nReset        = '0' THEN
      shift2        <= (OTHERS => '0');
      Strobe_o(2)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift2 <= (shift2(shift2'high-1 downto 0) & (Puls_i(2)));

      IF shift2(shift2'high) = '0' AND shift2(shift2'high-1) = '1' THEN
        Strobe_o(2) <= '1';
      ELSE
        Strobe_o(2) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe2;

p_Strobe3:  PROCESS (clk, nReset, Puls_i(3))
  BEGin
    IF nReset        = '0' THEN
      shift3        <= (OTHERS => '0');
      Strobe_o(3)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift3 <= (shift3(shift3'high-1 downto 0) & (Puls_i(3)));

      IF shift3(shift3'high) = '0' AND shift3(shift3'high-1) = '1' THEN
        Strobe_o(3) <= '1';
      ELSE
        Strobe_o(3) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe3;

p_Strobe4:  PROCESS (clk, nReset, Puls_i(4))
  BEGin
    IF nReset        = '0' THEN
      shift4        <= (OTHERS => '0');
      Strobe_o(4)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift4 <= (shift4(shift4'high-1 downto 0) & (Puls_i(4)));

      IF shift4(shift4'high) = '0' AND shift4(shift4'high-1) = '1' THEN
        Strobe_o(4) <= '1';
      ELSE
        Strobe_o(4) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe4;

p_Strobe5:  PROCESS (clk, nReset, Puls_i(5))
  BEGin
    IF nReset        = '0' THEN
      shift5        <= (OTHERS => '0');
      Strobe_o(5)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift5 <= (shift5(shift5'high-1 downto 0) & (Puls_i(5)));

      IF shift5(shift5'high) = '0' AND shift5(shift5'high-1) = '1' THEN
        Strobe_o(5) <= '1';
      ELSE
        Strobe_o(5) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe5;

p_Strobe6:  PROCESS (clk, nReset, Puls_i(6))
  BEGin
    IF nReset        = '0' THEN
      shift6        <= (OTHERS => '0');
      Strobe_o(6)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift6 <= (shift6(shift6'high-1 downto 0) & (Puls_i(6)));

      IF shift6(shift6'high) = '0' AND shift6(shift6'high-1) = '1' THEN
        Strobe_o(6) <= '1';
      ELSE
        Strobe_o(6) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe6;

p_Strobe7:  PROCESS (clk, nReset, Puls_i(7))
  BEGin
    IF nReset        = '0' THEN
      shift7        <= (OTHERS => '0');
      Strobe_o(7)   <= '0';

    ELSIF rising_edge(clk) THEN
      shift7 <= (shift7(shift7'high-1 downto 0) & (Puls_i(7)));

      IF shift7(shift7'high) = '0' AND shift7(shift7'high-1) = '1' THEN
        Strobe_o(7) <= '1';
      ELSE
        Strobe_o(7) <= '0';
      END IF;
    END IF;
  END PROCESS p_Strobe7;


--------- Umwadlung: Inputpulse --> Strobes --------------------


  Puls_i  <=  ATR_comp_puls;   -- ATR_comp_puls[0..7] (Comperator-Ausgänge) = Kanal-Nummer[1..8]



          
------------------------------------- Timeout -----------------------------------------         
          

P_Timeout_SM:  process (clk, nReset,  sm_state, S_ATR_to_Kanal,  S_ATR_to_Count)
                      
    begin
      if (nReset = '0') then

          Timeout_Cnt             <=  0;       -- Timoutzeit
          Loop_Counter            <=  0;       -- Timout_Counter
          Kanalnummer             <=  0;       -- Nummer des Timeout-Kanals

          
          s_config_err            <= '0';      -- Config-Error: Pulsbreite/Pulsverzögerung
          s_timeout               <= '0';      -- Timout-Flag
          sm_state                <= sm_idle;
        
  ELSIF rising_edge(clk) then

      case sm_state is
        when sm_idle    =>    if atr_puls_start = '1' then

                                Kanalnummer     <= to_integer(unsigned(S_ATR_to_Kanal)( 3 downto 0));   -- Nummer des Timeout-Kanals
                                Timeout_Cnt     <= to_integer(unsigned(S_ATR_to_Count)(15 downto 0));   -- Timoutzeit
                                Loop_Counter    <=  0;                                                  -- Timeout_Counter
                                sm_state        <= sm_start;
                              end if;

                              
                      ------------ Kanal-Nummer --------------             

       when sm_start     =>  IF      (Kanalnummer = 0)    THEN                        -- Compare-Kanal-Nr = 0
                                  sm_state          <=  sm_end;                       -- kein Compare-Kanal ausgewählt
                             ELSIF  (Kanalnummer > 8)    THEN                         -- Compare-Kanal-Nr = größer 8
                                 sm_state           <=  sm_config_err;                -- unzulässige Compare-Kanal-Nr.
                             ELSE
                                 sm_state           <=  sm_count;                     
                             end if;

       when sm_count     =>  IF (Timeout_Cnt = 0)         THEN                        -- Timeout-Zeit = 0
                                  sm_state          <=  sm_config_err;                     
                             ELSE
                                 sm_state           <=  sm_loop;                     
                             end if;

       when sm_loop      =>  IF (Loop_Counter < Timeout_Cnt) THEN            -- LoopCounter kleiner Timeout-Zeit
                               IF (Strobe_o(Kanalnummer-1) = '1') THEN       -- Strobe aus ATR_comp_puls[0..7] (Comperator-Ausgänge) = Kanal-Nummer[1..8]
                                 sm_state           <=  sm_end;              -- Puls kommt vor der Timeout-Zeit ==> ok
                               ELSE
                                 Loop_Counter       <= (Loop_Counter+1);
                                  sm_state          <=  sm_loop;                     
                               end if;  
                             ELSE 
                                  sm_state          <=  sm_timeout;                     
                             end if;  
  
       when sm_config_err =>      s_config_err      <= '1';      -- Set: Config-Error: Pulsbreite/Pulsverzögerung
                                  sm_state          <=  sm_end;                     
  
       when sm_timeout    =>      s_timeout         <= '1';      -- Set: Timout-Flag
                                  sm_state          <=  sm_end;                     
  
       when sm_end        =>    s_config_err        <= '0';      -- Reset: Config-Error: Pulsbreite/Pulsverzögerung
                                s_timeout           <= '0';      -- Reset: Timout-Flag
                                sm_state            <=  sm_idle;                     
  
       when others        =>  sm_state              <= sm_idle;
      end case;
    end if;
  end process P_Timeout_SM;
          
          
          
          
          
P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then
 
      S_ATR_verz_rd     <= (others => '0'); 
      S_ATR_verz_wr     <= (others => '0'); 
      S_ATR_pulsw_rd    <= (others => '0');
      S_ATR_pulsw_wr    <= (others => '0');
			S_ATR_to_kanal_rd <= '0';
			S_ATR_to_kanal_wr <= '0';
			S_ATR_to_Count_rd <= '0';
			S_ATR_to_Count_wr <= '0';
			S_Dtack           <= '0';
			Reg_rd_active     <= '0';
		
		elsif rising_edge(clk) then

      S_ATR_verz_rd     <= (others => '0'); 
      S_ATR_verz_wr     <= (others => '0'); 
      S_ATR_pulsw_rd    <= (others => '0');
      S_ATR_pulsw_wr    <= (others => '0');
			S_ATR_to_kanal_rd <= '0';
			S_ATR_to_kanal_wr <= '0';
			S_ATR_to_Count_rd <= '0';
			S_ATR_to_Count_wr <= '0';
			S_Dtack           <= '0';
			Reg_rd_active     <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_ATR_verz_0_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(0) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(0) <= '1';
							Reg_rd_active <= '1';
						end if;

          when C_ATR_verz_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(1) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(1) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(2) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(2) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(3) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(3) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(4) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(4) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(5) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(5) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(6) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(6) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_verz_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_verz_wr(7) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_verz_rd(7) <= '1';
							Reg_rd_active <= '1';
						end if;
            
					when C_ATR_pulsw_0_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(0) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(0) <= '1';
							Reg_rd_active <= '1';
						end if;

          when C_ATR_pulsw_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(1) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(1) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(2) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(2) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(3) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(3) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(4) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(4) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(5) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(5) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(6) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(6) <= '1';
							Reg_rd_active <= '1';
						end if;

 					when C_ATR_pulsw_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_pulsw_wr(7) <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_pulsw_rd(7) <= '1';
							Reg_rd_active <= '1';
						end if;

            
 					when C_ATR_to_kanal_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_to_kanal_wr <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_to_kanal_rd <= '1';
							Reg_rd_active <= '1';
						end if;
            
 					when C_ATR_to_count_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
							S_ATR_to_count_wr <= '1';
            end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_ATR_to_count_rd <= '1';
							Reg_rd_active <= '1';
						end if;
 
 
					when others => 

            S_ATR_verz_rd     <= (others => '0'); 
            S_ATR_verz_wr     <= (others => '0'); 
            S_ATR_pulsw_rd    <= (others => '0');
            S_ATR_pulsw_wr    <= (others => '0');
            S_ATR_to_kanal_rd <= '0';
            S_ATR_to_kanal_wr <= '0';
            S_ATR_to_Count_rd <= '0';
            S_ATR_to_Count_wr <= '0';
            S_Dtack           <= '0';
            Reg_rd_active     <= '0';

				end CASE;
			end if;
		end if;
	
	end process P_Adr_Deco;


  
  
	
P_Reg_IO:	process (nReset, clk)
	begin
		if nReset = '0' then
			s_ATR_verz      <= (others => (others => '0'));
			s_ATR_pulsw     <= (others => (others => '0'));

      s_ATR_to_kanal  <= (others => '0');
      s_ATR_to_Count  <= (others => '0');

      
		elsif rising_edge(clk) then
			if S_ATR_verz_wr(0)  = '1' then	s_ATR_verz(0)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(1)  = '1' then	s_ATR_verz(1)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(2)  = '1' then	s_ATR_verz(2)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(3)  = '1' then	s_ATR_verz(3)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(4)  = '1' then	s_ATR_verz(4)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(5)  = '1' then	s_ATR_verz(5)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(6)  = '1' then	s_ATR_verz(6)  <= Data_from_SCUB_LA; end if;
			if S_ATR_verz_wr(7)  = '1' then	s_ATR_verz(7)  <= Data_from_SCUB_LA; end if;

			if S_ATR_pulsw_wr(0) = '1' then	s_ATR_pulsw(0) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(1) = '1' then	s_ATR_pulsw(1) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(2) = '1' then	s_ATR_pulsw(2) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(3) = '1' then	s_ATR_pulsw(3) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(4) = '1' then	s_ATR_pulsw(4) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(5) = '1' then	s_ATR_pulsw(5) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(6) = '1' then	s_ATR_pulsw(6) <= Data_from_SCUB_LA; end if;
			if S_ATR_pulsw_wr(7) = '1' then	s_ATR_pulsw(7) <= Data_from_SCUB_LA; end if;

			if S_ATR_to_kanal_wr = '1' then	s_ATR_to_kanal <= Data_from_SCUB_LA; end if;
			if S_ATR_to_count_wr = '1' then	s_ATR_to_count <= Data_from_SCUB_LA; end if;
      
 end if;
end process P_Reg_IO;

	

P_read_mux:	process (	s_ATR_verz_rd,     s_ATR_verz,        s_ATR_pulsw_rd, s_ATR_pulsw,
                      S_ATR_to_kanal_rd, s_ATR_to_kanal,
                      S_ATR_to_count_rd, s_ATR_to_count)

	begin
		if 	  S_ATR_verz_rd(0)  = '1' then	S_Read_port <= s_ATR_verz(0);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(1)  = '1' then	S_Read_port <= s_ATR_verz(1);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(2)  = '1' then	S_Read_port <= s_ATR_verz(2);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(3)  = '1' then	S_Read_port <= s_ATR_verz(3);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(4)  = '1' then	S_Read_port <= s_ATR_verz(4);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(5)  = '1' then	S_Read_port <= s_ATR_verz(5);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(6)  = '1' then	S_Read_port <= s_ATR_verz(6);  -- Counter für Verzögerung
		elsif S_ATR_verz_rd(7)  = '1' then	S_Read_port <= s_ATR_verz(7);  -- Counter für Verzögerung

		elsif S_ATR_pulsw_rd(0) = '1' then	S_Read_port <= s_ATR_pulsw(0); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(1) = '1' then	S_Read_port <= s_ATR_pulsw(1); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(2) = '1' then	S_Read_port <= s_ATR_pulsw(2); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(3) = '1' then	S_Read_port <= s_ATR_pulsw(3); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(4) = '1' then	S_Read_port <= s_ATR_pulsw(4); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(5) = '1' then	S_Read_port <= s_ATR_pulsw(5); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(6) = '1' then	S_Read_port <= s_ATR_pulsw(6); -- Counter für Pulsbreite
		elsif S_ATR_pulsw_rd(7) = '1' then	S_Read_port <= s_ATR_pulsw(7); -- Counter für Pulsbreite

		elsif S_ATR_to_kanal_rd = '1' then	S_Read_port <= s_ATR_to_kanal; -- Kanalnummer für Timeout
		elsif S_ATR_to_count_rd = '1' then	S_Read_port <= s_ATR_to_count; -- Counter für Timeout
    
 else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;




--  ATR_to_conf_err <= s_config_err;	-- Time-Out: Configurations-Error
--  ATR_Timeout  		<= s_timeout;			-- Time-Out: Maximalzeit zwischen Start und Zündpuls überschritten.

P_Save_Config_err:	PROCESS (clk_250mhz, nReset_250mhz, ATR_Timeout_err_res)
	BEGIN
		IF  ((nReset_250mhz = '0') or (ATR_Timeout_err_res = '1')) THEN
          ATR_to_conf_err <= '0';     

		ELSIF rising_edge(clk_250mhz) THEN
			IF s_config_err      = '1'  THEN  -- Save Error
        ATR_to_conf_err   <= '1';       -- set Error-Flag 
			END IF;
		END IF;
	END PROCESS P_Save_Config_err;

P_Save_Timeout:	PROCESS (clk_250mhz, nReset_250mhz, ATR_Timeout_err_res)
	BEGIN
		IF  ((nReset_250mhz = '0') or (ATR_Timeout_err_res = '1')) THEN
          ATR_Timeout <= '0';     

		ELSIF rising_edge(clk_250mhz) THEN
			IF s_timeout      = '1'     THEN -- Save Error
        ATR_Timeout    <= '1';         -- set Error-Flag 
			END IF;
		END IF;
	END PROCESS P_Save_Timeout;


	
Dtack_to_SCUB   <= S_Dtack;
Data_to_SCUB    <= S_Read_Port;



end Arch_atr_puls_ctrl;