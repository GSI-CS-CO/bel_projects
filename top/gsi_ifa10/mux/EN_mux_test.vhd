LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

--Zusammenfassung der enable-wires auf array


ENTITY en_mux_test IS

  PORT
  (
   sys_reset   : in  std_logic;     -- syn. Clear
   sys_clk     : in  std_logic;     -- System-Clock

   G_nSwitch_Ena  : in STD_LOGIC;  --switch enable bidir puffer array

   VG_A_Out       : in STD_LOGIC;
   VG_A_Ena       : in STD_LOGIC;
   VG_B_Out       : in STD_LOGIC;
   VG_B_Ena       : in STD_LOGIC;
   VG_C_Out       : in STD_LOGIC;
   VG_C_Ena       : in STD_LOGIC;
   VG_D_Out       : in STD_LOGIC;
   VG_D_Ena       : in STD_LOGIC;

   VG_E_Out       : in STD_LOGIC;
   VG_E_Ena       : in STD_LOGIC;

   VG_F_Out       : in STD_LOGIC;
   VG_F_Ena       : in STD_LOGIC;

   VG_G_Out       : in STD_LOGIC;
   VG_G_Ena       : in STD_LOGIC;

   VG_I_Out       : in  STD_LOGIC;
   VG_I_Ena       : in  STD_LOGIC;

   VG_K_Out       : in  STD_LOGIC;
   VG_K_Ena       : in  STD_LOGIC;

   VG_L_Out       : in  STD_LOGIC;
   VG_L_Ena       : in  STD_LOGIC;

   VG_M_Out       : in  STD_LOGIC;
   VG_M_Ena       : in  STD_LOGIC;

   VG_N_Out       : in  STD_LOGIC;
   VG_N_Ena       : in  STD_LOGIC;

   X_SelDirO      : in STD_LOGIC_VECTOR(12 downto 1);
   X_EnIO         : in STD_LOGIC_VECTOR(12 downto 1);

   A_IO           : out STD_LOGIC_VECTOR(32 DOWNTO 1);
   B_IO           : out STD_LOGIC_VECTOR(32 DOWNTO 1);
   C_IO           : out STD_LOGIC_VECTOR(32 DOWNTO 1)

);


END en_mux_test;

architecture Behavioral of en_mux_test is

begin

process(sys_clk,sys_reset)

 begin
  if rising_edge(sys_clk) then

      A_IO(2 downto 1)      <= (others => '0');  --default in
      A_IO(10 downto 3)     <= (others => '0'); --inputs
      A_IO(11) <= X_SelDirO(1);
      A_IO(12) <= VG_K_Out;
      A_IO(13) <= VG_I_Out;
      A_IO(14) <= VG_K_Out;
      A_IO(15) <= VG_I_Out;
      A_IO(16) <= X_SelDirO(2);
      A_IO(17) <= X_SelDirO(3);
      A_IO(21 downto 18) <= (others => VG_E_Out);
      A_IO(25 downto 22) <= (others => VG_C_Out);
      A_IO(27 downto 26) <= (others => VG_B_Out);
      A_IO(31 downto 28) <= (others => VG_A_Out);
      A_IO(32)           <= '0'; --default in

      B_IO(1)  <= X_SelDirO(4);
      B_IO(2)  <= VG_N_Out;
      B_IO(3) <= X_SelDirO(12);
      B_IO(4)  <= VG_N_Out;
      B_IO(7 downto 5) <= (others => '0');
      B_IO(8)  <= X_SelDirO(5);
      B_IO(9) <= X_SelDirO(10);
      B_IO(10) <= '0';
      B_IO(11) <= VG_L_Out;
      B_IO(12) <= VG_N_Out;
      B_IO(13) <= X_SelDirO(6);
      B_IO(14) <= VG_M_Out;
      B_IO(15) <= X_SelDirO(7);
      B_IO(16) <= X_SelDirO(8);
      B_IO(17) <= X_SelDirO(9);
      B_IO(24 downto 18) <= (others => VG_D_Out);
      B_IO(25) <= VG_M_Out;
      B_IO(26) <= VG_L_Out;
      B_IO(27) <= '0';
      B_IO(28) <= VG_L_Out;
      B_IO(29) <= '0';
      B_IO(30) <= VG_L_Out;
      B_IO(31) <= '0';
      B_IO(32) <= VG_L_Out;

      C_IO(1)  <= '0'; --inputs
      C_IO(2)  <= '0';  --default in
      C_IO(10 downto 3) <= (others => '0'); --inputs

      C_IO(11) <= VG_L_Out;
      C_IO(12) <= VG_K_Out;
      C_IO(13) <= VG_I_Out;
      C_IO(14) <= VG_K_Out;
      C_IO(15) <= VG_K_Out;
      C_IO(16) <= X_SelDirO(11);
      C_IO(17) <= '0'; --inputs
      C_IO(18) <= VG_G_Out;
      C_IO(19) <= VG_F_Out;
      C_IO(20) <= VG_G_Out;
      C_IO(21) <= VG_G_Out;

      C_IO(25 downto 22) <= (others => VG_C_Out);
      C_IO(27 downto 26) <= (others => VG_B_Out);

      C_IO(31 downto 28) <= (others => VG_A_Out);
      C_IO(32) <= '0'; --default in

   end if;
end process;


  end Behavioral;

