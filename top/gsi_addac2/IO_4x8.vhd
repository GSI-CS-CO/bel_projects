
--+-------------------------------------------------------------------------------------------------------------------------------------+
--| "IO_4x8" controls and access four 8-bit io ports.                                                                                   |
--|                                                                                                                                     |
--| Version 1, W.Panschow, d. 15.02.2013                                                                                                |
--|                                                                                                                                     |
--| This macro works together with four external 8-bit bidir-buffers, so you can select and set direction only in 8-bit partitions.     |
--|                                                                                                                                     |
--| Register of "IO_4x8":                                                                                                               |
--|                                                                                                                                     |
--| address       |   function                    | remark                                                                              |
--| --------------+-------------------------------+--------------------------------------------------------------------------------     |
--| Base_Addr     | control register "cntrl_reg"  | configure direction and enable of the four external 8-bit bidir buffer              |
--| base_addr + 1 | read/write* io[15..0]         | both corrospondent external buffers must be enabled and set to the same direction   |
--| base_addr + 2 | read/write* io[31..16]        | both corrospondent external buffers must be enabled and set to the same direction   |
--| base_addr + 3 | read/write* io[7..0]          | read data: highbyte is always "00", lowbyte = io[7..0]                              |
--|               |                               | write data: highbyte set to don't care, lowbyte to io[7..0]                         |
--|               |                               | enable and direction of the corrospondent ext. buffer must be set with "cntrl_reg"  |
--| base_addr + 4 | read/write* io[15..8]         | read data: highbyte is always "00", lowbyte = io[15..8]                             |
--|               |                               | write data: highbyte set to don't care, lowbyte to io[15..8]                        |
--|               |                               | enable and direction of the corrospondent ext. buffer must be set with "cntrl_reg"  |
--| base_addr + 5 | read/write* io[23..16]        | read data: highbyte is always "00", lowbyte = io[23..16]                            |
--|               |                               | write data: highbyte set don't care, lowbyte to io[23..16]                          |
--|               |                               | enable and direction of the corrospondent ext. buffer must be set with "cntrl_reg"  |
--| base_addr + 6 | read/write* io[31..24]        | read data: highbyte is always "00", lowbyte = io[31..24]                            |
--|               |                               | write data: highbyte set to don't care, lowbyte to io[31..24]                       |
--|               |                               | enable and direction of the corrospondent ext. buffer must be set with "cntrl_reg"  |
--|                                                                                                                                     |
--|    *read or write depends on the "cntrl_reg"-setting.                                                                               |
--|     If a port is set to output, you can only write to this port. If set to input, you can only read it.                             |
--|     If the direction of external buffers don't match with the read or write access you will get no dtack.                           |
--|                                                                                                                                     |
--| Layout of control register "cntrl_reg":                                                                                             |
--|                                                                                                                                     |
--|   cntrl_reg[3..0] => select ext. buffer                                                                                             |
--|                                                                                                                                     |
--|   cntrl_reg(0)  = '1' ext. buff. for io[7..0] is disabled;    = '0' ext. buff. for io[7..0] is enabled                              |
--|   cntrl_reg(1)  = '1' ext. buff. for io[15..8] is disabled;   = '0' ext. buff. for io[15..8] is enabled                             |
--|   cntrl_reg(2)  = '1' ext. buff. for io[23..16] is disabled;  = '0' ext. buff. for io[23..16] is enabled                            |
--|   cntrl_reg(3)  = '1' ext. buff. for io[31..24] is disabled;  = '0' ext. buff. for io[31..24] is enabled                            |
--|                                                                                                                                     |
--|   cntrl_reg[7..4] => select direction of ext. buffer                                                                                |
--|                                                                                                                                     |
--|   cntrl_reg(4)  = '1' ext. buff. for io[7..0] is output (wr only);   = '0' ext. buff. for io[7..0] is input (rd only)               |
--|   cntrl_reg(5)  = '1' ext. buff. for io[15..8] is output (wr only);  = '0' ext. buff. for io[15..8] is input (rd only)              |
--|   cntrl_reg(6)  = '1' ext. buff. for io[23..16] is output (wr only); = '0' ext. buff. for io[23..16] is input (rd only)             |
--|   cntrl_reg(7)  = '1' ext. buff. for io[31..24] is output (wr only); = '0' ext. buff. for io[31..24] is input (rd only)             |
--+-------------------------------------------------------------------------------------------------------------------------------------+

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

entity IO_4x8 is
  generic
    (
    Base_addr:  unsigned(15 downto 0) := X"0200"
    );
    
  port
    (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';
    io:                 inout std_logic_vector(31 downto 0);
    io_7_0_tx:          out   std_logic;                      -- '1' = external IO-Data(7..0)-buffer set to output.
    ext_io_7_0_dis:     out   std_logic;                      -- '1' = disable external IO-Data(7..0)-buffer.
    io_15_8_tx:         out   std_logic;                      -- '1' = external IO-Data(15..8)-buffer set to output
    ext_io_15_8_dis:    out   std_logic;                      -- '1' = disable external IO-Data(15..8)-buffer.
    io_23_16_tx:        out   std_logic;                      -- '1' = external IO-Data(23..16)-buffer set to output.
    ext_io_23_16_dis:   out   std_logic;                      -- '1' = disable external IO-Data(23..16)-buffer.
    io_31_24_tx:        out   std_logic;                      -- '1' = external IO-Data(31..24)-buffer set to output
    ext_io_31_24_dis:   out   std_logic;                      -- '1' = disable external IO-Data(31..24)-buffer.
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
    );  
  end IO_4x8;


architecture Arch_IO_4x8 of IO_4x8 is


constant  rw_cntrl_reg_addr_offset: unsigned(15 downto 0) := X"0000";
constant  rw_io_15_0_addr_offset:   unsigned(15 downto 0) := X"0001"; -- addr offset for read/write 16 bit IO(15 downto 0)
constant  rw_io_31_16_addr_offset:  unsigned(15 downto 0) := X"0002"; -- addr offset for read/write 16 bit IO(31 downto 16)
constant  rw_io_7_0_addr_offset:    unsigned(15 downto 0) := X"0003"; -- addr offset for read/write 8 bit IO(7 downto 0)
constant  rw_io_15_8_addr_offset:   unsigned(15 downto 0) := X"0004"; -- addr offset for read/write 8 bit IO(15 downto 8)
constant  rw_io_23_16_addr_offset:  unsigned(15 downto 0) := X"0005"; -- addr offset for read/write 8 bit IO(23 downto 16)
constant  rw_io_31_24_addr_offset:  unsigned(15 downto 0) := X"0006"; -- addr offset for read/write 8 bit IO(31 downto 24)

-- address to read/write control register
constant  rw_cntrl_reg_addr:  unsigned(15 downto 0) := Base_addr + rw_cntrl_reg_addr_offset;

-- address to read/write IO(15 downto 0)
constant  rw_io_15_0_addr:    unsigned(15 downto 0) := Base_addr + rw_io_15_0_addr_offset;

-- address to read/write IO(31 downto 16)
constant  rw_io_31_16_addr:   unsigned(15 downto 0) := base_addr + rw_io_31_16_addr_offset;

-- address to read/write IO(7 downto 0)
constant  rw_io_7_0_addr:     unsigned(15 downto 0) := base_addr + rw_io_7_0_addr_offset;

-- address to read/write IO(15 downto 8)
constant  rw_io_15_8_addr:    unsigned(15 downto 0) := base_addr + rw_io_15_8_addr_offset;

-- address to read/write IO(23 downto 16)
constant  rw_io_23_16_addr:   unsigned(15 downto 0) := base_addr + rw_io_23_16_addr_offset;

-- address to read/write IO(31 downto 24)
constant  rw_io_31_24_addr:   unsigned(15 downto 0) := base_addr + rw_io_31_24_addr_offset;

signal    rd_io_15_0:       std_logic;                      -- select for 16-bit read: io(15 downto 0)
signal    wr_io_15_0:       std_logic;                      -- select for 16-bit write: io(15 downto 0)

signal    rd_io_31_16:      std_logic;                      -- select for 16-bit read: io(31 downto 16)
signal    wr_io_31_16:      std_logic;                      -- select for 16-bit write: io(31 downto 16)

signal    rd_io_7_0:        std_logic;                      -- select for 8-bit read: io(7 downto 0)
signal    wr_io_7_0:        std_logic;                      -- select for 8-bit write: io(7 downto 0)

signal    rd_io_15_8:       std_logic;                      -- select for 8-bit read: io(15 downto 8)
signal    wr_io_15_8:       std_logic;                      -- select for 8-bit write: io(15 downto 8)

signal    rd_io_23_16:      std_logic;                      -- select for 8-bit read: io(23 downto 16)
signal    wr_io_23_16:      std_logic;                      -- select for 8-bit write: io(23 downto 16)

signal    rd_io_31_24:      std_logic;                      -- select for 8-bit read: io(31 downto 24)
signal    wr_io_31_24:      std_logic;                      -- select for 8-bit write: io(31 downto 24)

signal    s_dtack:          std_logic;

signal    in_15_0_sync1:    std_logic_vector(15 downto 0);  -- first synchronise of asynchron input io[15..0] 
signal    in_15_0_sync2:    std_logic_vector(15 downto 0);  -- second synchronise of asynchron input io[15..0]

signal    in_31_16_sync1:   std_logic_vector(15 downto 0);  -- first synchronise of asynchron input io[31..16] 
signal    in_31_16_sync2:   std_logic_vector(15 downto 0);  -- second synchronise of asynchron input io[31..16]

signal    rd_cntrl_reg:       std_logic;
signal    wr_cntrl_reg:       std_logic;
signal    cntrl_reg:          std_logic_vector(7 downto 0);

alias     s_ext_io_7_0_dis:   std_logic is  cntrl_reg(0); -- '0' = ext. buff. for io[7..0] is enabled
alias     s_ext_io_15_8_dis:  std_logic is  cntrl_reg(1); -- '0' = ext. buff. for io[15..8] is enabled
alias     s_ext_io_23_16_dis: std_logic is  cntrl_reg(2); -- '0' = ext. buff. for io[23..16] is enabled
alias     s_ext_io_31_24_dis: std_logic is  cntrl_reg(3); -- '0' = ext. buff. for io[31..24] is enabled
alias     s_io_7_0_tx:        std_logic is  cntrl_reg(4); -- '1' = ext. buff. for io[7..0] is output
alias     s_io_15_8_tx:       std_logic is  cntrl_reg(5); -- '1' = ext. buff. for io[15..8] is output
alias     s_io_23_16_tx:      std_logic is  cntrl_reg(6); -- '1' = ext. buff. for io[23..16] is output
alias     s_io_31_24_tx:      std_logic is  cntrl_reg(7); -- '1' = ext. buff. for io[31..24] is output

signal    out_byte1:          std_logic_vector(7 downto 0); -- output latch for io[7..0]
signal    out_byte2:          std_logic_vector(7 downto 0); -- output latch for io[15..0]
signal    out_byte3:          std_logic_vector(7 downto 0); -- output latch for io[23..16]
signal    out_byte4:          std_logic_vector(7 downto 0); -- output latch for io[31..24]

begin

P_Adr_Deco: process (nReset, clk)
  begin
    if nReset = '0' then
      rd_cntrl_reg  <= '0';
      wr_cntrl_reg  <= '0';
      rd_io_15_0    <= '0';
      wr_io_15_0    <= '0';
      rd_io_31_16   <= '0';
      wr_io_31_16   <= '0';
      rd_io_7_0     <= '0';  
      wr_io_7_0     <= '0';  
      rd_io_15_8    <= '0'; 
      wr_io_15_8    <= '0'; 
      rd_io_23_16   <= '0';
      wr_io_23_16   <= '0';
      rd_io_31_24   <= '0';
      wr_io_31_24   <= '0';
      S_Dtack       <= '0';
      user_rd_active <= '0';
    
    elsif rising_edge(clk) then
      rd_cntrl_reg  <= '0';
      wr_cntrl_reg  <= '0';
      rd_io_15_0    <= '0';
      wr_io_15_0    <= '0';
      rd_io_31_16   <= '0';
      wr_io_31_16   <= '0';
      rd_io_7_0     <= '0';  
      wr_io_7_0     <= '0';  
      rd_io_15_8    <= '0'; 
      wr_io_15_8    <= '0'; 
      rd_io_23_16   <= '0';
      wr_io_23_16   <= '0';
      rd_io_31_24   <= '0';
      wr_io_31_24   <= '0';
      S_Dtack       <= '0';
      user_rd_active <= '0';
      
      if Ext_Adr_Val = '1' then

        case unsigned(ADR_from_SCUB_LA) IS
        
          when rw_cntrl_reg_addr =>
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              rd_cntrl_reg <= '1';
              user_rd_active <= '1';
            end if;
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              wr_cntrl_reg <= '1';
              user_rd_active <= '0';
            end if;


          when rw_io_15_0_addr =>                                   -- read-write io(15..0)
            if Ext_Rd_active = '1' then
              if (s_io_15_8_tx = '0') and (s_io_7_0_tx = '0')       -- direction of both data buffer must be set to input
                and (s_ext_io_15_8_dis = '0') and (s_ext_io_7_0_dis = '0')  -- both buffer must be enabled
              then
                S_Dtack     <= '1';
                rd_io_15_0  <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_15_8_tx = '1') and (s_io_7_0_tx = '1')       -- direction of both data buffer must be set to output
                and (s_ext_io_15_8_dis = '0') and (s_ext_io_7_0_dis = '0')  -- both buffer must be enabled
              then
                S_Dtack     <= '1';
                wr_io_15_0  <= '1';
                user_rd_active <= '0';
             end if;
            end if;


          when rw_io_31_16_addr =>                                    -- read-write io(31..16)
            if Ext_Rd_active = '1' then
              if (s_io_31_24_tx = '0') and (s_io_23_16_tx = '0')      -- direction of both data buffer must be set to input
                and (s_ext_io_31_24_dis = '0') and (s_ext_io_23_16_dis = '0')  -- both buffer must be enabled
              then
                S_Dtack     <= '1';
                rd_io_31_16 <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_31_24_tx = '1') and (s_io_23_16_tx = '1')      -- direction of both data buffer must be set to input
                and (s_ext_io_31_24_dis = '0') and (s_ext_io_23_16_dis = '0')  -- both buffer must be enabled
              then 
                S_Dtack     <= '1';
                wr_io_31_16  <= '1';
                user_rd_active <= '0';
              end if;
            end if;

          when rw_io_7_0_addr =>                                       -- read-write io(7..0)
            if Ext_Rd_active = '1' then
              if (s_io_7_0_tx = '0') and (s_ext_io_7_0_dis = '0') then -- direction of data buffer must be enabled and set to input
                S_Dtack   <= '1';
                rd_io_7_0 <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_7_0_tx = '1') and (s_ext_io_7_0_dis = '0') then -- direction of data buffer must be enabled and set to output
                S_Dtack   <= '1';
                wr_io_7_0 <= '1';
                user_rd_active <= '0';
              end if;
            end if;

          when rw_io_15_8_addr =>                                      -- read-write io(15..8)
            if Ext_Rd_active = '1' then
              if (s_io_15_8_tx = '0') and (s_ext_io_15_8_dis = '0') then -- direction of data buffer must be enabled and set to input
                S_Dtack   <= '1';
                rd_io_15_8 <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_15_8_tx = '1') and (s_ext_io_15_8_dis = '0') then -- direction of data buffer must be enabled and set to output
                S_Dtack   <= '1';
                wr_io_15_8 <= '1';
                user_rd_active <= '0';
              end if;
            end if;

          when rw_io_23_16_addr =>                                     -- read-write io(23..16)
            if Ext_Rd_active = '1' then
              if (s_io_23_16_tx = '0') and (s_ext_io_23_16_dis = '0') then -- direction of data buffer must be enabled and set to input
                S_Dtack   <= '1';
                rd_io_23_16 <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_23_16_tx = '1') and (s_ext_io_23_16_dis = '0') then -- direction of data buffer must be enabled and set to output
                S_Dtack   <= '1';
                wr_io_23_16 <= '1';
                user_rd_active <= '0';
              end if;
            end if;

          when rw_io_31_24_addr =>                                     -- read-write io(31..24)
            if Ext_Rd_active = '1' then
              if (s_io_31_24_tx = '0') and (s_ext_io_31_24_dis = '0') then -- direction of data buffer must be enabled and set to input
                S_Dtack   <= '1';
                rd_io_31_24 <= '1';
                user_rd_active <= '1';
              end if;
            end if;
            if Ext_Wr_active = '1' then
              if (s_io_31_24_tx = '1') and (s_ext_io_31_24_dis = '0') then -- direction of data buffer must be enabled and set to output
                S_Dtack   <= '1';
                wr_io_31_24 <= '1';
                user_rd_active <= '0';
              end if;
            end if;

            when others => 
              rd_cntrl_reg  <= '0';
              wr_cntrl_reg  <= '0';
              rd_io_15_0    <= '0';
              wr_io_15_0    <= '0';
              rd_io_31_16   <= '0';
              wr_io_31_16   <= '0';
              rd_io_7_0     <= '0';  
              wr_io_7_0     <= '0';  
              rd_io_15_8    <= '0'; 
              wr_io_15_8    <= '0'; 
              rd_io_23_16   <= '0';
              wr_io_23_16   <= '0';
              rd_io_31_24   <= '0';
              wr_io_31_24   <= '0';
              S_Dtack       <= '0';
              user_rd_active <= '0';

        end case;
      end if;
    end if;
  end process P_Adr_Deco;
  

p_cntrl:  process (nReset, clk)
  begin
    if nReset = '0' then
      s_ext_io_7_0_dis    <= '1';
      s_ext_io_15_8_dis   <= '1';
      s_ext_io_23_16_dis  <= '1';
      s_ext_io_31_24_dis  <= '1';
      s_io_7_0_tx         <= '0';
      s_io_15_8_tx        <= '0';
      s_io_23_16_tx       <= '0';
      s_io_31_24_tx       <= '0';
    elsif rising_edge(clk) then
      if wr_cntrl_reg = '1' then
        s_ext_io_7_0_dis    <= Data_from_SCUB_LA(0);--Data_from_SCUB_LA(s_ext_io_7_0_dis'pos(s_ext_io_7_0_dis));
        s_ext_io_15_8_dis   <= Data_from_SCUB_LA(1);--Data_from_SCUB_LA(s_ext_io_15_8_dis'pos(s_ext_io_15_8_dis));
        s_ext_io_23_16_dis  <= Data_from_SCUB_LA(2);--Data_from_SCUB_LA(s_ext_io_23_16_dis'pos(s_ext_io_23_16_dis));
        s_ext_io_31_24_dis  <= Data_from_SCUB_LA(3);--Data_from_SCUB_LA(s_ext_io_31_24_dis'pos(s_ext_io_31_24_dis));
        s_io_7_0_tx         <= Data_from_SCUB_LA(4);--Data_from_SCUB_LA(s_io_7_0_tx'pos(s_io_7_0_tx));
        s_io_15_8_tx        <= Data_from_SCUB_LA(5);--Data_from_SCUB_LA(s_io_15_8_tx'pos(s_io_15_8_tx));
        s_io_23_16_tx       <= Data_from_SCUB_LA(6);--Data_from_SCUB_LA(s_io_23_16_tx'pos(s_io_23_16_tx));
        s_io_31_24_tx       <= Data_from_SCUB_LA(7);--Data_from_SCUB_LA(s_io_31_24_tx'pos(s_io_31_24_tx));
      end if;
    end if; 
  end process p_cntrl;

  
p_sync_in_15_0: process (nReset, clk)           -- synchronise asynchron input io[15..0]
  begin
    if nReset = '0' then
      in_15_0_sync1 <= (others => '0');
      in_15_0_sync2 <= (others => '0');
    elsif rising_edge(clk) then
      in_15_0_sync1 <= io(15 downto 0);    -- first synchronise of asynchron input io[15..0]
      in_15_0_sync2 <= in_15_0_sync1;           -- second synchronise of asynchron input io[15..0]
    end if;    
  end process p_sync_in_15_0;

  
p_sync_in_31_16: process (nReset, clk)          -- synchronise asynchron input io[31..16]
  begin
    if nReset = '0' then
      in_31_16_sync1 <= (others => '0');
      in_31_16_sync2 <= (others => '0');
    elsif rising_edge(clk) then
      in_31_16_sync1 <= io(31 downto 16);  -- first synchronise of asynchron input io[31..16]
      in_31_16_sync2 <= in_31_16_sync1;         -- second synchronise of asynchron input io[31..16]
    end if;    
  end process p_sync_in_31_16;


p_output_reg: process (nReset, clk)
  begin
    if nReset = '0' then
      out_byte1 <= (others => '0');
      out_byte2 <= (others => '0');
      out_byte3 <= (others => '0');
      out_byte4 <= (others => '0');
    elsif rising_edge(clk) then
      if wr_io_15_0 = '1' then
        out_byte1 <= Data_from_SCUB_LA(7 downto 0);
        out_byte2 <= Data_from_SCUB_LA(15 downto 8);
      end if;
      if wr_io_31_16 = '1' then
        out_byte3 <= Data_from_SCUB_LA(7 downto 0);
        out_byte4 <= Data_from_SCUB_LA(15 downto 8);
      end if;
      if wr_io_7_0 = '1' then
        out_byte1 <= Data_from_SCUB_LA(7 downto 0);
      end if;
      if wr_io_15_8 = '1' then
        out_byte2 <= Data_from_SCUB_LA(7 downto 0);
      end if;
      if wr_io_23_16 = '1' then
        out_byte3 <= Data_from_SCUB_LA(7 downto 0);
      end if;
      if wr_io_31_24 = '1' then
        out_byte4 <= Data_from_SCUB_LA(7 downto 0);
      end if;
    end if;
   end process p_output_reg;

 
p_rd_mux: process (
                  in_15_0_sync2, in_31_16_sync2, cntrl_reg, rd_io_15_0, rd_io_31_16,
                  rd_cntrl_reg, rd_io_7_0, rd_io_15_8, rd_io_23_16, rd_io_31_24
                  )
  begin
    if rd_io_15_0 = '1' then
      Data_to_SCUB <= in_15_0_sync2;
    elsif rd_io_31_16 = '1' then
      Data_to_SCUB <= in_31_16_sync2;
    elsif rd_cntrl_reg = '1' then
      Data_to_SCUB <= X"00" & cntrl_reg;
    elsif rd_io_7_0 = '1' then
      Data_to_SCUB <= X"00" & in_15_0_sync2(7 downto 0);
    elsif rd_io_15_8 = '1' then
      Data_to_SCUB <= X"00" & in_15_0_sync2(15 downto 8);
    elsif rd_io_23_16 = '1' then
      Data_to_SCUB <= X"00" & in_31_16_sync2(7 downto 0);
    elsif rd_io_31_24 = '1' then
      Data_to_SCUB <= X"00" & in_31_16_sync2(15 downto 8);
    else
      Data_to_SCUB <= in_15_0_sync2;
    end if;
  end process p_rd_mux;

p_tristate: process (
                    s_io_7_0_tx, s_io_15_8_tx, s_io_23_16_tx, s_io_31_24_tx,
                    out_byte1, out_byte2, out_byte3, out_byte4
                    )
  begin
    if s_io_7_0_tx = '1' then
      io(7 downto 0) <= out_byte1;
    else
      io(7 downto 0) <= (others => 'Z');
    end if;
    if s_io_15_8_tx = '1' then
      io(15 downto 8) <= out_byte2;
    else
      io(15 downto 8) <= (others => 'Z');
    end if;
    if s_io_23_16_tx = '1' then
      io(23 downto 16) <= out_byte3;
    else
      io(23 downto 16) <= (others => 'Z');
    end if;
    if s_io_31_24_tx = '1' then
      io(31 downto 24) <= out_byte4;
    else
      io(31 downto 24) <= (others => 'Z');
    end if;
  end process p_tristate;


Dtack_to_SCUB <= S_Dtack;

ext_io_7_0_dis    <= s_ext_io_7_0_dis;
ext_io_15_8_dis   <= s_ext_io_15_8_dis;
ext_io_23_16_dis  <= s_ext_io_23_16_dis;
ext_io_31_24_dis  <= s_ext_io_31_24_dis;

io_7_0_tx   <= s_io_7_0_tx;
io_15_8_tx  <= s_io_15_8_tx;
io_23_16_tx <= s_io_23_16_tx;
io_31_24_tx <= s_io_31_24_tx;


end Arch_IO_4x8;
