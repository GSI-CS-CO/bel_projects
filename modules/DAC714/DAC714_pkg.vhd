LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;

library work;

package DAC714_pkg is

component DAC714 is
  generic (
    Base_addr:        unsigned(15 downto 0) := X"0300";
    CLK_in_Hz:        integer := 100_000_000;
    SPI_CLK_in_Hz:    integer := 10_000_000;
    Default_is_FG_mode: integer range 0 to 1 := 0
    );
  port
    (
    Adr_from_SCUB_LA:   in   		std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    	std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in  	  std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in	    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    	std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in	    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    	std_logic := '1';
    nExt_Trig_DAC:      in 		  std_logic;                      -- external trigger input over optocoupler,
																																-- led on -> nExt_Trig_DAC is low
    FG_Data:            in    	std_logic_vector(15 downto 0) := (others => '0');  -- parallel dac data during FG-Mode
    FG_Strobe:          in    	std_logic := '0';               -- strobe to start SPI transfer (if possible) during FG-Mode
    DAC_SI:             out   	std_logic;                      -- connect to DAC-SDI
    nDAC_CLK:           out   	std_logic;                      -- spi-clock of DAC
    nCS_DAC:            out  	 	std_logic;                      -- '0' enable shift of internal shift register of DAC
    nLD_DAC:            out   	std_logic;                      -- '0' copy shift register to output latch of DAC
    nCLR_DAC:           buffer	std_logic;                      -- '0' resets the DAC, Clear Pulsewidth min 200ns
																																-- resets both the input latch and the D/A latch to 0000H (midscale).
    Rd_Port:            out   	std_logic_vector(15 downto 0);	-- output for all read sources of this macro
    Rd_Activ:           out   	std_logic;											-- this acro has read data available at the Rd_Port.
    Dtack:              out   	std_logic
    );
end component DAC714;

end package DAC714_pkg;
