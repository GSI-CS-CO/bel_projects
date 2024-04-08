--Read write block auf registers

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

USE work.BLM_counter_pkg.all;

-- do do
-- check if Data_to_SCUB is != Z for one cycle is sufficient

entity ETB_Register is

  generic
    (
      Base_addr : Integer := 0;
      Reg_Length: Integer := 16  --range
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

    regs_out:           out   t_arr_Data_Word;                -- Register Values

    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic
    );
  end ETB_Register;



architecture  ETB_Register of ETB_Register is


constant addr_width     : INTEGER := Adr_from_SCUB_LA'length;
constant C_REG_START    : unsigned(addr_width-1 downto 0) := to_unsigned(Base_addr, addr_width);   -- Startaddress of Register Block
constant C_REG_END      : unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr+Reg_Length-1), addr_width);-- Last address of Register Block

signal allregisters     : t_arr_Data_Word := (others =>(others => '0')); --its a array (0 to 15) of std_logic_vector(15 downto 0);
signal RegIdx           : unsigned(addr_width-1 downto 0) := (others => '0'); --Register offset


---------------------------------------
Begin

regs_out <= allregisters;

RegIdx <= unsigned(Adr_from_SCUB_LA) - unsigned(C_REG_START); --generate register offset


--read write block auf registers
ETB_Reg: process (nReset, clk,Ext_Wr_active,Ext_Rd_active,RegIdx,Adr_from_SCUB_LA,Data_from_SCUB_LA,Ext_Adr_Val,allregisters)
  begin
    if nReset = '0' then
      allregisters   <= (others =>(others =>'0'));
      Dtack_to_SCUB  <= '0';
      user_rd_active <= '0';
      Data_to_SCUB   <= (others => '0');
      
    elsif rising_edge(clk) then
      Dtack_to_SCUB  <= '0';
      user_rd_active <= '0';
      Data_to_SCUB   <= (others => '0');
      if Ext_Adr_Val = '1' then
      
         --addr in register range?            
         if (unsigned(Adr_from_SCUB_LA) >= unsigned(C_REG_START)) and (unsigned(Adr_from_SCUB_LA) <= unsigned(C_REG_END)) then 
            --read from reg address
            if Ext_Rd_active = '1' then
               Dtack_to_SCUB  <= '1';
               user_rd_active <= '1';
               Data_to_SCUB   <= allregisters(to_integer(RegIdx));
            end if;

            --write to reg address
            if Ext_Wr_active = '1' then
               Dtack_to_SCUB <= '1';
               allregisters(to_integer(RegIdx)) <= Data_from_SCUB_LA;
            end if;
        end if; --if (Adr_from_SCUB_LA <= C_REG_START)
      end if; -- if Ext_Adr_Val = '1' then
   end if; --if rising_edge(clk) then
end process;


end ETB_Register;
