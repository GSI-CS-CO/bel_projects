library ieee;
use ieee.std_logic_1164.all;

entity monster_phy_reg is
  generic(
    g_enable : boolean := false);
  port(
    tx_clk_i            : in  std_logic;
    rx_clk_i            : in  std_logic;
    -- WR core side
    wrc_tx_data_i       : in  std_logic_vector;
    wrc_tx_k_i          : in  std_logic_vector;
    wrc_tx_disparity_o  : out std_logic;
    wrc_tx_enc_err_o    : out std_logic;
    wrc_rx_data_o       : out std_logic_vector;
    wrc_rx_k_o          : out std_logic_vector;
    wrc_rx_enc_err_o    : out std_logic;
    wrc_rx_bitslide_o   : out std_logic_vector;
    -- PHY side
    phy_tx_data_o       : out std_logic_vector;
    phy_tx_k_o          : out std_logic_vector;
    phy_tx_disparity_i  : in  std_logic;
    phy_tx_enc_err_i    : in  std_logic;
    phy_rx_data_i       : in  std_logic_vector;
    phy_rx_k_i          : in  std_logic_vector;
    phy_rx_enc_err_i    : in  std_logic;
    phy_rx_bitslide_i   : in  std_logic_vector);
end monster_phy_reg;

architecture rtl of monster_phy_reg is
begin

  reg_n : if not g_enable generate
    phy_tx_data_o      <= wrc_tx_data_i;
    phy_tx_k_o         <= wrc_tx_k_i;
    wrc_tx_disparity_o <= phy_tx_disparity_i;
    wrc_tx_enc_err_o   <= phy_tx_enc_err_i;
    wrc_rx_data_o      <= phy_rx_data_i;
    wrc_rx_k_o         <= phy_rx_k_i;
    wrc_rx_enc_err_o   <= phy_rx_enc_err_i;
    wrc_rx_bitslide_o  <= phy_rx_bitslide_i;
  end generate;

  reg_y : if g_enable generate
    tx : process(tx_clk_i)
    begin
      if rising_edge(tx_clk_i) then
        phy_tx_data_o      <= wrc_tx_data_i;
        phy_tx_k_o         <= wrc_tx_k_i;
        wrc_tx_disparity_o <= phy_tx_disparity_i;
        wrc_tx_enc_err_o   <= phy_tx_enc_err_i;
      end if;
    end process;

    rx : process(rx_clk_i)
    begin
      if rising_edge(rx_clk_i) then
        wrc_rx_data_o      <= phy_rx_data_i;
        wrc_rx_k_o         <= phy_rx_k_i;
        wrc_rx_enc_err_o   <= phy_rx_enc_err_i;
        wrc_rx_bitslide_o  <= phy_rx_bitslide_i;
      end if;
    end process;
  end generate;

end rtl;
