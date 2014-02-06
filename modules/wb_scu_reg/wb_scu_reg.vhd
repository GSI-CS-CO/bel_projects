library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

entity wb_scu_reg is
  generic (
    Base_addr:      unsigned(15 downto 0);
    register_cnt: integer
  );
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out;
    
    -- SCU bus
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
  );
end entity;

architecture wb_scu_reg_arch of wb_scu_reg is
  type channel_reg_type is array(0 to register_cnt-1) of std_logic_vector(15 downto 0);    -- array for register_count registers
  signal s_shadow_regs:   channel_reg_type;
  signal s_ext_regs:      channel_reg_type;
  
  signal rd_pulse1: std_logic;
  signal rd_pulse2: std_logic;
  signal copy_shadow: std_logic;
  signal rd_active: std_logic;
  
  signal dtack:         std_logic;
begin

  wb_regs: process (clk_sys_i, rst_n_i, s_shadow_regs, slave_i)
  begin
    if rising_edge(clk_sys_i) then
      -- It is vitally important that for each occurance of
      --   (cyc and stb and not stall) there is (ack or rty or err)
      --   sometime later on the bus.
      --
      -- This is an easy solution for a device that never stalls:
      slave_o.ack <= slave_i.cyc and slave_i.stb;
      slave_o.dat <= (others => '0');
      
      if rst_n_i = '0' then
        for I in 0 to register_cnt-1 loop
          s_shadow_regs(I) <= (others => '0');
        end loop;
      else
        -- Detect a write to the register byte
        if slave_i.cyc = '1' and slave_i.stb = '1' and
          slave_i.we = '1' then
            if slave_i.sel = x"3" then
              s_shadow_regs(to_integer(unsigned(slave_i.adr(4 downto 1)))) <= slave_i.dat(15 downto 0);
            elsif slave_i.sel = x"c" then
              s_shadow_regs(to_integer(unsigned(slave_i.adr(4 downto 1)))) <= slave_i.dat(31 downto 16);
            end if;
        end if;
        
        slave_o.dat <= s_shadow_regs(to_integer(unsigned(slave_i.adr(4 downto 1)))) & s_shadow_regs(to_integer(unsigned(slave_i.adr(4 downto 1))));
      end if;
    end if;
   
  end process;
  
  slave_o.int <= '0';
  slave_o.err <= '0';
  slave_o.rty <= '0';
  slave_o.stall <= '0'; -- always ready
  
  rd_pulse: process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      rd_pulse1 <= Ext_Rd_active;
      rd_pulse2 <= rd_pulse1;
    end if;
  end process;

  copy_shadow <= '1' when rd_pulse1 = '1' and rd_pulse2 = '0' else '0';

  shadow_2_ext: process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      if copy_shadow = '1' then
        for I in 0 to register_cnt-1 loop
          s_ext_regs(I) <= s_shadow_regs(I);
        end loop;
      end if;
    end if;
  end process;
  
  adr_decoder: process (clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      dtack         <= '0';
      rd_active     <= '0';
      
      if Ext_Adr_Val = '1' then
        if unsigned(Adr_from_SCUB_LA) >= Base_addr and unsigned(Adr_from_SCUB_LA) < Base_addr + register_cnt then
          if Ext_Rd_active = '1' then
            dtack        <= '1';
            rd_active    <= '1';
          end if;
        end if;
      end if;
      
    end if;
  end process adr_decoder;
  
  user_rd_active <= rd_active;

  Data_to_SCUB <= s_ext_regs(to_integer(unsigned(Adr_from_SCUB_LA))) when rd_active = '1' else
                  x"0000";
                  
  Dtack_to_SCUB <= dtack;

end architecture;