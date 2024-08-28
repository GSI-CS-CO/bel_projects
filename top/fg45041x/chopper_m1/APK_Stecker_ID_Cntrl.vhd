library ieee;
use ieee.std_logic_1164.all; 
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
library lpm;
use lpm.lpm_components.all;


entity APK_Stecker_ID_Cntrl is
  generic (
    K3_APK_ST_ID : integer := 65535;
    K2_APK_ST_ID : integer := 65535;
    K1_APK_ST_ID : integer := 65535;
    K0_APK_ST_ID : integer := 65535;
    ST_160_Pol   : integer := 1;
    Clk_in_Hz    : integer := 300000000;
    Wait_in_ns   : integer := 200;
    Use_LPM      : integer := 0
  );
  port (
    Start_ID_Cntrl : in std_logic;
    Update_Apk_ID  : in std_logic;
    Reset          : in std_logic;
    clk            : in std_logic;
    K3_K2_Skal     : in std_logic_vector(7 downto 0);
    A_K3_D         : in std_logic_vector(15 downto 0);
    A_K2_D         : in std_logic_vector(15 downto 0);
    DB_K3_inP      : in std_logic;
    DB_K2_inP      : in std_logic;
    DB_GR1_APK_ID  : in std_logic;                      -- muss das gelatchte Signal sein
    K1_K0_Skal     : in std_logic_vector(7 downto 0);
    A_K1_D         : in std_logic_vector(15 downto 0);
    A_K0_D         : in std_logic_vector(15 downto 0);
    DB_K1_inP      : in std_logic;
    DB_K0_inP      : in std_logic;
    DB_GR0_APK_ID  : in std_logic;                      -- muss das gelatchte Signal sein
    ID_Cntrl_Done  : out std_logic;
    APK_ST_ID_OK   : out std_logic;
    La_Ena_Skal_In : out std_logic;
    La_Ena_Port_In : out std_logic;
    K3_ID          : out std_logic_vector(15 downto 0);
    A_nK3_ID_En    : out std_logic;
    K2_ID          : out std_logic_vector(15 downto 0);
    A_nK2_ID_En    : out std_logic;
    A_nGR1_ID_Sel  : out std_logic;
    K1_ID          : out std_logic_vector(15 downto 0);
    A_nK1_ID_En    : out std_logic;
    K0_ID          : out std_logic_vector(15 downto 0);
    A_nK0_ID_En    : out std_logic;
    A_nGR0_ID_Sel  : out std_logic
  );
end APK_Stecker_ID_Cntrl;



architecture Arch_APK_Stecker_ID_Cntrl of APK_Stecker_ID_Cntrl is

  function How_many_Bits (int: integer) return integer is

    variable i, tmp : integer;
    begin
      tmp := int;
      i   := 0;
      while tmp > 0 loop
	tmp := tmp / 2;
	i := i + 1;
      end loop;
      return i;
  end How_many_bits;

  component lpm_counter
    generic (
      lpm_width     : natural;
      lpm_type      : string;
      lpm_direction : string;
      lpm_svalue    : string
    );
    port (
      clock  : in std_logic;
      cnt_en : in std_logic := '1';
      q      : out std_logic_vector (lpm_width-1 downto 0);
      sset   : in std_logic
    );
  end component;

  constant  Clk_in_ps  : integer := 1000000000 / (Clk_in_Hz / 1000);
  constant  C_Wait_Cnt : integer := Wait_in_ns * 1000 / Clk_in_ps;

  signal  S_Wait_Cnt      : std_logic_vector(How_many_Bits(C_Wait_Cnt) downto 0);
  signal  S_Ld_Wait       : std_logic;
  signal  S_Wait_Fin      : std_logic_vector (1 downto 0);
  signal  S_Sel_Wait_Cnt  : std_logic;

  signal  S_Start_Edge    : std_logic;
  signal  S_Start_ID_Sync : std_logic_vector(1 downto 0);
  
  signal  S_K3_ID         : std_logic_vector (15 downto 0);
  signal  S_K2_ID         : std_logic_vector (15 downto 0);
  signal  S_K1_ID         : std_logic_vector (15 downto 0);
  signal  S_K0_ID         : std_logic_vector (15 downto 0);

  signal  S_La_Ena_ID     : std_logic;

  signal  S_ID_Cntrl_Done : std_logic;
  
  
  type	T_ID_SM	is (
    ID_Idle,
    GR_ID_Sel,
    ID_En,
    GR_ID_Dis,
    ID_Done
  );

  signal  S_ID_SM : T_ID_SM;


begin 

P_Edge: process (clk, Reset)
  begin
    if Reset = '1' then
      S_Start_Edge <= '0';
      S_Start_ID_Sync <= "00";
    elsif rising_edge(clk) then
	S_Start_ID_Sync <= (S_Start_ID_Sync(0) & Start_ID_Cntrl);
	if ((S_Start_ID_Sync(1) = '0') and (S_Start_ID_Sync(0) = '1')) or Update_Apk_ID = '1' then
          S_Start_Edge <= '1';
	else
          S_Start_Edge <= '0';
	end if;
    end if;
end process P_Edge;



wait_with_lpm: if Use_LPM = 1 generate -----------------------------------------------

begin
    
wait_cnt : lpm_counter
  generic map (
    lpm_width     => S_Wait_Cnt'LENGTH,
    lpm_type      => "LPM_COUNTER",
    lpm_direction => "DOWN",
    lpm_svalue    => integer'image(C_Wait_Cnt)
  )
  port map (
    clock	=> clk,
    sset	=> S_Ld_Wait,
    cnt_en	=> S_Sel_Wait_Cnt,
    q		=> S_Wait_Cnt
  );
			
end generate wait_with_lpm; ----------------------------------------------------------
			

wait_without_lpm: if Use_LPM = 0 generate --------------------------------------------

begin
P_Wait:	process (clk, Reset)
  begin
    if Reset = '1' then
      S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'length);
    elsif rising_edge(clk) then
      if S_Ld_Wait = '1' then
        S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'length);
      elsif S_Sel_Wait_Cnt = '1' then
        S_Wait_Cnt <= S_Wait_Cnt - 1;
      else
        S_Wait_Cnt <= S_Wait_Cnt;
      end if;
    end if;			
  end process;
	
end generate wait_without_lpm; -------------------------------------------------------


P_ID_SM: process (clk, Reset)
  begin
    if Reset = '1' then
      S_ID_SM         <= ID_Idle;
      S_ID_Cntrl_Done <= '0';
    elsif rising_edge(clk) then
      S_Wait_Fin     <= (S_Wait_Fin(0) & S_Wait_Cnt(S_Wait_Cnt'left)); -- Schiebe-Reg. wird zur Flankenerkennung genutzt.
      S_Ld_Wait      <= '0';
      S_La_Ena_ID    <= '0';
      S_Sel_Wait_Cnt <= '0';
			
      case S_ID_SM is

        when ID_Idle =>
          S_Ld_Wait <= '1';
          if S_Start_Edge = '1' then
            S_ID_SM <= GR_ID_Sel;
          end if;
				
        when GR_ID_Sel =>
          S_ID_Cntrl_Done <= '0';
          S_Sel_Wait_Cnt  <= '1';
          if S_Wait_Fin = "01" then
            S_Ld_Wait <= '1';
          elsif S_Wait_Fin = "10" then
            s_id_sM <= ID_En;
          end if;
					
        when ID_En =>
          S_Sel_Wait_Cnt <= '1';
          if S_Wait_Fin = "01" then
            S_Ld_Wait   <= '1';
            S_La_Ena_ID <= '1';
          elsif S_Wait_Fin = "10" then
            S_ID_SM <= GR_ID_Dis;
          end if;
					
        when GR_ID_Dis =>
          S_Sel_Wait_Cnt <= '1';
          if S_Wait_Fin = "01" then
            S_Ld_Wait <= '1';
          elsif S_Wait_Fin = "10" then
            S_ID_SM <= ID_Done;
          end if;
						
        when ID_Done =>
          S_Sel_Wait_Cnt <= '1';
          if S_Wait_Fin = "01" then
            S_ID_Cntrl_Done <= '1';
            S_ID_SM <= ID_Idle;
          end if;
			
      end case;
                      
    end if;
end process P_ID_SM;


P_ID_La: process (clk)
  begin
    if rising_edge(clk) then
      if S_La_Ena_ID = '1' then
        if DB_GR1_APK_ID = '1' then
          S_K3_ID <= A_K3_D;
          S_K2_ID <= A_K2_D;
        else
          S_K3_ID <= (X"00" & K3_K2_Skal);
          S_K2_ID <= (X"00" & K3_K2_Skal);
        end if;
        if DB_GR0_APK_ID = '1' then
          S_K1_ID <= A_K1_D;
          S_K0_ID <= A_K0_D;
        else
          S_K1_ID <= (X"00" & K1_K0_Skal);
          S_K0_ID <= (X"00" & K1_K0_Skal);
        end if;
      end if;
    end if;
end process P_ID_La;

K3_ID <= S_K3_ID;
K2_ID <= S_K2_ID;
K1_ID <= S_K1_ID;
K0_ID <= S_K0_ID;


P_ID_Compare: process (clk)
  begin
    if rising_edge(clk) then
      if St_160_Pol = 0 then
        if (S_K1_ID = conv_std_logic_vector(K1_APK_ST_ID, 16)) and (S_K0_ID = conv_std_logic_vector(K0_APK_ST_ID, 16)) then
          APK_ST_ID_OK <= S_ID_Cntrl_Done;
        else
          APK_ST_ID_OK <= '0';
        end if;
      else
        if      (S_K3_ID = conv_std_logic_vector(K3_APK_ST_ID, 16))
            and (S_K2_ID = conv_std_logic_vector(K2_APK_ST_ID, 16))
            and (S_K1_ID = conv_std_logic_vector(K1_APK_ST_ID, 16))
            and (S_K0_ID = conv_std_logic_vector(K0_APK_ST_ID, 16)) then
          APK_ST_ID_OK <= S_ID_Cntrl_Done;
        else
          APK_ST_ID_OK <= '0';
        end if;
      end if;
    end if;
end process P_ID_Compare;

La_Ena_Skal_In <= '1' when (S_ID_SM /= ID_Idle) else '0';
La_Ena_Port_In <= '1' when (S_ID_SM /= ID_Idle) else '0';

A_nGR1_ID_Sel <= '0' when (S_ID_SM = ID_En) else '1';	-- ID-Strobe fuer K3+K2
A_nGR0_ID_Sel <= '0' when (S_ID_SM = ID_En) else '1';	-- ID-Strobe fuer K1+K0
A_nK3_ID_En <= DB_K3_inP when not ((S_ID_SM = ID_Idle) or (S_ID_SM = ID_Done)) else not DB_K3_inP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK2_ID_En <= DB_K2_inP when not ((S_ID_SM = ID_Idle) or (S_ID_SM = ID_Done)) else not DB_K2_inP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK1_ID_En <= DB_K1_inP when not ((S_ID_SM = ID_Idle) or (S_ID_SM = ID_Done)) else not DB_K1_inP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!
A_nK0_ID_En <= DB_K0_inP when not ((S_ID_SM = ID_Idle) or (S_ID_SM = ID_Done)) else not DB_K0_inP;	-- ID_EN ist '0'-aktiv bei Output-APKs und '1'-aktiv bei Input-APKs!

ID_Cntrl_Done <= '1' when S_ID_Cntrl_Done = '1' and S_ID_SM = ID_Idle else '0';

end; 
