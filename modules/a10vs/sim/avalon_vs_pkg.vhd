library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package avalon_vs_pkg is
    component avalon_vs is
        generic (
            g_addr_width:             natural := 4 -- bits
        );
        port (
            avs_clk:                  in  std_logic;
            avs_rst:                  in  std_logic;
            avs_ctrl_csr_addr:        in  std_logic;
            avs_ctrl_csr_rd:          in  std_logic;
            avs_ctrl_csr_readdata:    out std_logic_vector(31 downto 0);
            avs_ctrl_csr_wr:          in  std_logic;
            avs_ctrl_csr_writedata:   in std_logic_vector(31 downto 0);

            avs_sample_csr_addr:      in  std_logic_vector(g_addr_width - 1 downto 0);
            avs_sample_csr_rd:        in  std_logic;
            avs_sample_csr_readdata:  out std_logic_vector(31 downto 0);
            avs_sample_csr_wr:        in  std_logic;
            avs_sample_csr_writedata: in std_logic_vector(31 downto 0)
        );
    end component;
end avalon_vs_pkg;