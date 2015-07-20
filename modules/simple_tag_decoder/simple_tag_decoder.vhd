library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

  -- DDS_start_tag_decoder_inst : entity work.DDS_start_tag_decoder
    -- generic map (
      -- start_tag           => x"feedbabe"
      -- )
    -- port map (
      -- clk_i             =>   clk_sys,              
      -- nrst_i            =>   nPowerup_Res_sig,

      -- -- SCU_bus_slave interface
      -- Timing_Pattern_LA_i   => Timing_Pattern_LA,	
      -- Timing_Pattern_RCV_i  => Timing_Pattern_RCV,
      -- -- fg_quad_scu_bus interface
      -- brdcst_i              => fg_brdcst,               
      -- brdcst_o              => DDS_start_tag_decoder_brdcst

      -- );
      

entity simple_tag_decoder is
  generic (
    start_tag:            std_logic_vector(31 downto 0)

    );
  port (
    clk_i:              in    std_logic;                     
    nrst_i:             in    std_logic;

    -- SCU_bus_slave interface
    Timing_Pattern_LA_i:   in    std_logic_vector(31 downto 0);  -- connect to SCU_bus_slave
    Timing_Pattern_RCV_i:  in    std_logic;                      -- connect to SCU_bus_slave
    -- fg_quad_scu_bus interface
    start_o:            out   std_logic                          -- connect to fg_quad_scu_bus

    );
end entity;


architecture simple_tag_decoder_arch of simple_tag_decoder is

--	States
	type DDS_start_tag_decoder_states is(IDLE, TAG_RECEIVED);
	signal DDS_start_tag_decoder_state	:	DDS_start_tag_decoder_states;
 
begin

DDS_start_tag_decoder_SM: process (clk_i, nrst_i) 
  begin
    if nrst_i = '0' then
      start_o                     <= '0';
      DDS_start_tag_decoder_state <= IDLE;
      
    elsif rising_edge(clk_i) then    
      case DDS_start_tag_decoder_state is
        
        when IDLE =>
          start_o <= '0';
          if Timing_Pattern_RCV_i = '1' then
            DDS_start_tag_decoder_state <= TAG_RECEIVED;
          end if;
          
        when TAG_RECEIVED => -- WAIT_FOR_START_TAG
          start_o       <= '0';          
          case Timing_Pattern_LA_i is
            when start_tag =>
              start_o       <= '1';               
            when others =>
          end case;
          DDS_start_tag_decoder_state <= IDLE;
            
       
        when others	=>	null;
      end case;
            
    end if;
  end process DDS_start_tag_decoder_SM;

end architecture;