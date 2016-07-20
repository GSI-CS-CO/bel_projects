----------------------------------------------------------------------------------
-- Author:          Jonny Doin, jdoin@opencores.org
-- 
-- Create Date:     15:36:20 05/15/2011
-- Module Name:     SPI_SLAVE - RTL
-- Project Name:    SPI INTERFACE
-- Target Devices:  Spartan-6
-- Tool versions:   ISE 13.1
-- Description: 
--
--      This block is the SPI slave interface, implemented in one single entity.
--      All internal core operations are synchronous to the external SPI clock, and follows the general SPI de-facto standard.
--      The parallel read/write interface is synchronous to a supplied system master clock, 'clk_i'.
--      Synchronization for the parallel ports is provided by input data request and write enable lines, and output data valid line.
--      Fully pipelined cross-clock circuitry guarantees that no setup artifacts occur on the buffers that are accessed by the two 
--      clock domains.
--


library ieee;
use ieee.std_logic_1164.all;

package spi_slave_pkg is
  component spi_slave is
      Generic (   
          N 		: positive := 8;                                        -- 32bit serial word length is default
          CPOL 	: std_logic := '0';                                     -- SPI mode selection (mode 0 default)
          CPHA 	: std_logic := '0';                                     -- CPOL = clock polarity, CPHA = clock phase.
          PREFETCH : positive := 3);                                      -- prefetch lookahead cycles
      Port (  
          clk_i 			  : in std_logic;                          -- internal interface clock (clocks di/do registers)
          spi_ssel_i 		: in std_logic;                               -- spi bus slave select line
          spi_sck_i 		: in std_logic;                                -- spi bus sck clock (clocks the shift register core)
          spi_mosi_i 		: in std_logic;                               -- spi bus mosi input
          spi_miso_o 		: out std_logic;                              -- spi bus spi_miso_o output
          di_req_o 		  : out std_logic;                                       -- preload lookahead data request line
          di_i 			    : in std_logic_vector (N-1 downto 0);  -- parallel load data in (clocked in on rising edge of clk_i)
          wren_i 			  : in std_logic;                                   -- user data write enable
          wr_ack_o 		  : out std_logic;                                       -- write acknowledge
          do_valid_o 		: out std_logic;                                     -- do_o data valid strobe, valid during one clk_i rising edge.
          do_o 			    : out  std_logic_vector (N-1 downto 0);          -- parallel output (clocked out on falling clk_i)
         	--- debug ports: can be removed for the application circuit
          do_transfer_o 	: out std_logic;                                -- debug: internal transfer driver
          wren_o 			    : out std_logic;                                -- debug: internal state of the wren_i pulse stretcher
          rx_bit_next_o 	: out std_logic;                                -- debug: internal rx bit
          state_dbg_o 	  : out std_logic_vector (3 downto 0);            -- debug: internal state register
          sh_reg_dbg_o 	  : out std_logic_vector (N-1 downto 0)           -- debug: internal shift register
      );                      
  end component;

end package;

