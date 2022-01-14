----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:    16.01.2020
-- Design Name:
-- Module Name:     Clock_Timing
-- Project Name:
-- Target Devices:
-- Tool versions:
-- Description:
-- Generate, provide and tune required global Clock(s) and timing signals
-- Sources depend an
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:

--inputs

--
--outputs


-- user_dipsw   -- IFK Adresse

----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

LIBRARY altera_mf;
USE altera_mf.all;

entity Clock_Timing is

Port (

   PLL_reset            : in  STD_LOGIC;
   clk_24MHz_in         : in  STD_LOGIC;
   clk_50MHz_in         : in  STD_LOGIC;
   clk_ext_in           : in  STD_LOGIC;

   sys_clk              : out  STD_LOGIC; --120MHz

   clk_24MHz_out        : out  STD_LOGIC;
   clk_12MHZ_out        : out  STD_LOGIC;
   clk_50MHz_out        : out  STD_LOGIC;
   clk_25MHZ_out        : out  STD_LOGIC;
   clk_6MHz_out         : out  STD_LOGIC; --no PLL
   ext_clk_out          : out  STD_LOGIC; --no PLL

   locked_clk           : out  STD_LOGIC; --1 PLL locked, 0- PLL-rising

   Ena_every_100ns      : out  STD_LOGIC;
   Ena_every_166ns      : out  STD_LOGIC;
   Ena_every_250ns      : out  STD_LOGIC;
   Ena_every_500ns      : out  STD_LOGIC;
   Ena_every_us         : out  STD_LOGIC;
   Ena_every_ms         : out  STD_LOGIC;
   Ena_every_20ms       : out  STD_LOGIC;
   Ena_every_1sec       : out  STD_LOGIC;
   Ena_every_2_5s       : out  STD_LOGIC

);

end Clock_Timing;

architecture Behavioral of Clock_Timing is


Component PLL_1
   PORT
   (
      areset: IN STD_LOGIC  := '0';
      inclk0: IN STD_LOGIC  := '0';
      c0    : OUT STD_LOGIC ;
      c1    : OUT STD_LOGIC ;
      c2    : OUT STD_LOGIC ;
      c3    : OUT STD_LOGIC ;
      c4    : OUT STD_LOGIC ;
      locked: OUT STD_LOGIC
   );
end component;


COMPONENT Timebase
   GENERIC ( Test_Ein : INTEGER :=0 );
   PORT
   (
      clk                  :   IN STD_LOGIC;
      Ena_every_100ns      :   OUT STD_LOGIC;
      Ena_every_166ns      :   OUT STD_LOGIC;
      Ena_every_250ns      :   OUT STD_LOGIC;
      Ena_every_500ns      :   OUT STD_LOGIC;
      Ena_every_us         :   OUT STD_LOGIC;
      Ena_every_ms         :   OUT STD_LOGIC;
      Every_20ms           :   OUT STD_LOGIC
   );
END COMPONENT;


signal sys_clk_local    : std_logic := '0';
signal sys_rst_local    : std_logic := '0';
signal pll_locked_local : std_logic := '0';
signal clk_12MHz_local  : std_logic := '0';
signal clk_6MHz_local   : std_logic := '0';
signal Ena_every_20ms_local : std_logic := '0';
signal timer1sec        : std_logic_vector(7 downto 0) := ( others =>'0');
signal timer2_5sec      : std_logic_vector(7 downto 0) := ( others =>'0');


-----------------------------------------------------
-----------------------------------------------------

begin
Ena_every_20ms<=Ena_every_20ms_local;

sys_clk       <= sys_clk_local;
clk_12MHz_out <= clk_12MHz_local;
clk_6MHz_out  <= clk_6MHz_local;

locked_clk <= pll_locked_local;

ext_clk_out<=clk_ext_in; --add clk puffer??

PLL_1_inst : PLL_1 PORT MAP (
      areset   => '0',
      inclk0   => clk_24MHz_in,

      c0       => sys_clk_local,
      c1       => clk_24MHz_out,
      c2       => clk_12MHz_local,
      c3       => clk_50MHz_out,
      c4       => clk_25MHz_out,

      locked   => pll_locked_local
   );


Timebase_inst : Timebase
PORT MAP (
      clk    => sys_clk_local,
      Ena_every_100ns => Ena_every_100ns,
      Ena_every_166ns => Ena_every_166ns,
      Ena_every_250ns => Ena_every_250ns,
      Ena_every_500ns => Ena_every_500ns,
      Ena_every_us    => Ena_every_us,
      Ena_every_ms    => Ena_every_ms,
      Every_20ms      => Ena_every_20ms_local
   );

--generate 6MHz clock
--used only in output VG 14B pin
clk6mhz:process(clk_12MHz_local,pll_locked_local,clk_6MHz_local)

 begin
   if pll_locked_local='0'then -- keine 6MHz clk solange PLL instabil
      clk_6MHz_local <='0';
    elsif rising_edge(clk_12MHz_local) then
      clk_6MHz_local<= not clk_6MHz_local;
   end if;
 end process;


 Timer1s: process (sys_clk_local,sys_rst_local,timer1sec,Ena_every_20ms_local) -- shiftet
  begin

   if sys_rst_local = '1' then
      timer1sec <= ( others =>'0');
      Ena_every_1sec <= '0';

   elsif rising_edge(sys_clk_local) then
        Ena_every_1sec    <='0';
        if Ena_every_20ms_local = '1' then
          if timer1sec >= x"30" then
            timer1sec <= ( others =>'0');
            Ena_every_1sec <='1';
          else
            timer1sec <= timer1sec+1;
          end if; --counter ready?
        end if; --20ms
    end if; --elsif rising_edge(sys_clk)
end process;


--später zu Timer/Clock
Timer2_5s: process (sys_clk_local,sys_rst_local,timer2_5sec,Ena_every_20ms_local) -- shiftet
  begin

   if sys_rst_local='1' then
      timer2_5sec <= ( others =>'0');
      Ena_every_2_5s <= '0';

   elsif rising_edge(sys_clk_local) then
        Ena_every_2_5s  <='0';
        if Ena_every_20ms_local = '1' then
          if timer2_5sec >= x"7F" then
            timer2_5sec <= ( others =>'0');
            Ena_every_2_5s <= '1'; -- kurzer enable Puls
          else
            timer2_5sec <= timer2_5sec+1;
          end if; --counter ready?
        end if; --20ms
    end if; --elsif rising_edge(sys_clk)
end process;



end Behavioral;
