library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.ghrd_5astfd5k3_pkg.all;

entity av_rocket_board is
  port(
    -- Monster ports
    sfp4_tx_disable_o      : out   std_logic;
    sfp4_tx_fault          : in    std_logic;
    sfp4_los               : in    std_logic;
    sfp4_txp_o             : out   std_logic;
    sfp4_rxp_i             : in    std_logic;
    sfp4_mod0              : in    std_logic; -- grounded by module
    sfp4_mod1              : inout std_logic; -- SCL
    sfp4_mod2              : inout std_logic; -- SDA
    --sfp4_rate              : out   std_logic;      
    
    sfp3_tx_disable_o      : out   std_logic := '1';
    sfp3_tx_fault          : in    std_logic;
    sfp3_los               : in    std_logic;
    --sfp3_txp_o             : out   std_logic;
    --sfp3_rxp_i             : in    std_logic;
    sfp3_mod0              : in    std_logic; -- grounded by module
    sfp3_mod1              : inout std_logic; -- SCL
    sfp3_mod2              : inout std_logic; -- SDA
    --sfp3_rate              : out   std_logic;
    
    sfp2_tx_disable_o      : out   std_logic := '1';
    sfp2_tx_fault          : in    std_logic;
    sfp2_los               : in    std_logic;
    --sfp2_txp_o             : out   std_logic;
    --sfp2_rxp_i             : in    std_logic;
    sfp2_mod0              : in    std_logic; -- grounded by module
    sfp2_mod1              : inout std_logic; -- SCL
    sfp2_mod2              : inout std_logic; -- SDA
    --sfp2_rate              : out   std_logic;
    
    sfp1_tx_disable_o      : out   std_logic := '1';
    sfp1_tx_fault          : in    std_logic;
    sfp1_los               : in    std_logic;
    --sfp1_txp_o             : out   std_logic;
    --sfp1_rxp_i             : in    std_logic;
    sfp1_mod0              : in    std_logic; -- grounded by module
    sfp1_mod1              : inout std_logic; -- SCL
    sfp1_mod2              : inout std_logic; -- SDA
    --sfp1_rate              : out   std_logic;
    
    --------------------------------------------    
    sfp4_red_o             : out   std_logic := 'Z';
    sfp4_green_o           : out   std_logic := 'Z';
    sfp3_red_o             : out   std_logic := 'Z';
    sfp3_green_o           : out   std_logic := 'Z';
    sfp2_red_o             : out   std_logic := 'Z';
    sfp2_green_o           : out   std_logic := 'Z';
    sfp1_red_o             : out   std_logic := 'Z';
    sfp1_green_o           : out   std_logic := 'Z';

    --------------------------------------------
    pe_smdat          : inout std_logic; -- unused (needed for CvP)
    pe_smclk          : out   std_logic := 'Z';
    pe_waken          : out   std_logic := 'Z';
    --------------------------------------------
    rom_data               : inout std_logic; -- OneWire
    dac_sclk               : out   std_logic; -- WR
    dac_din                : out   std_logic; -- WR
    ndac_cs                : out   std_logic_vector(2 downto 1); -- WR
    pcie_rx_i              : in    std_logic_vector(3 downto 0); -- PCIe
    pcie_tx_o              : out   std_logic_vector(3 downto 0); -- PCIe
    pcie_resetn_i          : in    std_logic; -- PCIe
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd                  : out   std_logic;
    slwr                  : out   std_logic;
    fd                    : inout std_logic_vector(7 downto 0); 
    pa                    : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl                   : in    std_logic_vector(2 downto 0); 
    uclk                  : in    std_logic;
    ures                  : out   std_logic;
    uclkin                : in    std_logic;
    
    -----------------------------------------------------------------------
    button_i             : in    std_logic_vector(4 downto 1);
    -----------------------------------------------------------------------

    lemo_p_i          : in    std_logic_vector(8 downto 1);
    lemo_n_i          : in    std_logic_vector(8 downto 1);
    lemo_p_o          : out   std_logic_vector(8 downto 1);
    lemo_n_o          : out   std_logic_vector(8 downto 1);
    lemo_oen_o        : out   std_logic_vector(8 downto 1);
    lemo_term_o       : out   std_logic_vector(8 downto 1);
    lemo_oe_ledn_o    : out   std_logic_vector(8 downto 1);
    lemo_act_ledn_o   : out   std_logic_vector(8 downto 1);
    led_o             : out   std_logic_vector(8 downto 1) := (others => '1');

    lvds_p_i          : in    std_logic_vector(3 downto 1);
    lvds_n_i          : in    std_logic_vector(3 downto 1);
    lvds_i_ledn_o     : out   std_logic_vector(3 downto 1);
    lvds_clk_p_i      : in    std_logic;
    --lvds_clk_n_i      : in    std_logic;

    lvds_p_o          : out   std_logic_vector(3 downto 1);
    lvds_n_o          : out   std_logic_vector(3 downto 1);
    lvds_o_ledn_o     : out   std_logic_vector(3 downto 1);
    lvds_clk_p_o      : out   std_logic;
    --lvds_clk_n_o      : out   std_logic;

    ------------------------------------------------------------------------
    -- Display
    ------------------------------------------------------------------------

    dsp_csn_o         : out   std_logic;
    dsp_resn_o        : out   std_logic;
    dsp_dcn_o         : out   std_logic;
    dsp_d1_o          : out   std_logic;
    dsp_d0_o          : out   std_logic;

    ------------------------------------------------------------------------
    -- Additional WREX/WR1 pins
    ------------------------------------------------------------------------
    wr1_nc1           : inout std_logic;
    wr1_nc4           : inout std_logic;
    wr1_nc5           : inout std_logic;
    wr1_nc6           : inout std_logic;
    --uart_i                 : in std_logic;
    --uart_o                 : out std_logic;
    
    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    fpga_rest_i     : in std_logic; -- reset from CPLD
    nres_i          : in std_logic; -- from chip
    con_io          : inout std_logic_vector( 5 downto 1) := (others => 'Z');
--  pbs2            : in std_logic; -- switches
--  hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer

    -- FPGA peripherals ports
    fpga_dipsw_pio         : in    std_logic_vector(3 downto 0); -- NAND_DQ3,RGMII1_RX_CTL,USB1_D4,HPS_GPIO22    -- PIN_C15, 7B (0)
                                                                 -- NAND_DQ5,RGMII1_RX_CLK,USB1_D6,HPS_GPIO24    -- PIN_C16, 7B
                                                                 -- NAND_DQ7,RGMII1_RXD2,HPS_GPIO26              -- PIN_C18, 7B
                                                                 -- NAND_WP,RGMII1_RXD3,QSPI_SS2,HPS_GPIO27      -- PIN_K12, 7B
    fpga_led_pio           : out   std_logic_vector(3 downto 0); -- SDMMC_D6 USB0_D6,HPS_GPIO42                  -- PIN_F13, 7C (0)
                                                                 -- SDMMC_D7 USB0_D7,HPS_GPIO43                  -- PIN_P14, 7C
                                                                 -- SDMMC_FB_CLK_IN,USB0_CLK,HPS_GPIO44          -- PIN_G13, 7C
                                                                 -- RGMII0_TX_CLK,RGMII0_TX_CLK,HPS_GPIO0        -- PIN_L15, 7C
    --fpga_button_pio        : in    std_logic_vector(3 downto 0); -- !!! Do we really need this?
    -- HPS peripherals ports
    hps_dipsw_pio         : in    std_logic_vector(3 downto 0); -- NAND_RB,RGMII1_TXD3,USB1_D3,HPS_GPIO18        -- PIN_A24, 7B (0)
                                                                -- NAND_DQ0,RGMII1_RXD0,HPS_GPIO19               -- PIN_C12, 7B
                                                                -- NAND_DQ4,RGMII1_TX_CTL,USB1_D5,HPS_GPIO23     -- PIN_C14, 7B
                                                                -- NAND_DQ6,RGMII1_RXD1,USB1_D7,HPS_GPIO25       -- PIN_C13, 7B
    hps_led_pio           : out   std_logic_vector(3 downto 0); -- QSPI_SS1,HPS_GPIO35                           -- PIN_B25, 7B (0)
                                                                -- SDMMC_PWREN,HPS_GPIO37                        -- PIN_K13, 7C
                                                                -- SDMMC_D4,HPS_GPIO40                           -- PIN_E13, 7C
                                                                -- SDMMC_D5,HPS_GPIO41                           -- PIN_N13, 7C
    --hps_button_pio        : in    std_logic_vector(3 downto 0); -- !!! Do we really need this?
    -- HPS memory controller ports
    hps_memory_mem_a       : out   std_logic_vector(14 downto 0); -- HPS_DDR -> HPS_A_0     -> F5,  6A (0)
                                                                  -- HPS_DDR -> HPS_A_1     -> E4,  6A (1)
                                                                  -- HPS_DDR -> HPS_A_2     -> G5,  6A (2)
                                                                  -- HPS_DDR -> HPS_A_3     -> H6,  6A (3)
                                                                  -- HPS_DDR -> HPS_A_4     -> B7,  6A (4)
                                                                  -- HPS_DDR -> HPS_A_5     -> A6,  6A (5)
                                                                  -- HPS_DDR -> HPS_A_6     -> A8,  6A (6)
                                                                  -- HPS_DDR -> HPS_A_7     -> A7,  6A (7)
                                                                  -- HPS_DDR -> HPS_A_8     -> D6,  6A (8)
                                                                  -- HPS_DDR -> HPS_A_9     -> C6,  6A (9)
                                                                  -- HPS_DDR -> HPS_A_10    -> J6,  6A (10)
                                                                  -- HPS_DDR -> HPS_A_11    -> J7,  6A (11)
                                                                  -- HPS_DDR -> HPS_A_12    -> D7,  6A (12)
                                                                  -- HPS_DDR -> HPS_A_13    -> C7,  6A (13)
                                                                  -- HPS_DDR -> HPS_A_14    -> D9,  6A (14)
                                                                  -- HPS_DDR -> HPS_A_15    -> D8,  6a (15) -- !!! Unused at the moment...
    hps_memory_mem_ba      : out   std_logic_vector(2 downto 0);  -- HPS_DDR -> HPS_BA_0    -> H7,  6A (0)
                                                                  -- HPS_DDR -> HPS_BA_1    -> A10, 6A (1)
                                                                  -- HPS_DDR -> HPS_BA_2    -> A9,  6A (2)
    hps_memory_mem_ck      : out   std_logic;                     -- HPS_DDR -> HPS_CK      -> G6,  6A
    hps_memory_mem_ck_n    : out   std_logic;                     -- HPS_DDR -> HPS_CK      -> G7,  6A
    hps_memory_mem_cke     : out   std_logic;                     -- HPS_DDR -> HPS_CKE_0   -> L7,  6A (Why CKE_0?)
    hps_memory_mem_cs_n    : out   std_logic;                     -- HPS_DDR -> HPS_CS_N    -> C9,  6A (Why CSN_0?)
    hps_memory_mem_ras_n   : out   std_logic;                     -- HPS_DDR -> HPS_RAS_N   -> D5,  6A
    hps_memory_mem_cas_n   : out   std_logic;                     -- HPS_DDR -> HPS_CAS_N   -> E6,  6A
    hps_memory_mem_we_n    : out   std_logic;                     -- HPS_DDR -> HPS_WE_N    -> B9,  6A
    hps_memory_mem_reset_n : out   std_logic;                     -- HPS_DDR -> HPS_RESET_N -> J2,  6B
    hps_memory_mem_dq      : inout std_logic_vector(39 downto 0); -- HPS_DDR -> HPS_DQ_0    -> D4,  6A (0)
                                                                  -- HPS_DDR -> HPS_DQ_1    -> C4,  6A (1)
                                                                  -- HPS_DDR -> HPS_DQ_2    -> E7,  6A (2)
                                                                  -- HPS_DDR -> HPS_DQ_3    -> G4,  6A (3)
                                                                  -- HPS_DDR -> HPS_DQ_4    -> C3,  6A (4)
                                                                  -- HPS_DDR -> HPS_DQ_5    -> D3,  6A (5)
                                                                  -- HPS_DDR -> HPS_DQ_6    -> A4,  6A (6)
                                                                  -- HPS_DDR -> HPS_DQ_7    -> K6,  6A (7)
                                                                  -- HPS_DDR -> HPS_DQ_8    -> B3,  6A (8)
                                                                  -- HPS_DDR -> HPS_DQ_9    -> A3,  6A (9)
                                                                  -- HPS_DDR -> HPS_DQ_10   -> G2,  6A (10)
                                                                  -- HPS_DDR -> HPS_DQ_11   -> B1,  6A (11)
                                                                  -- HPS_DDR -> HPS_DQ_12   -> G1,  6A (12)
                                                                  -- HPS_DDR -> HPS_DQ_13   -> F1,  6A (13)
                                                                  -- HPS_DDR -> HPS_DQ_14   -> M7,  6A (14)
                                                                  -- HPS_DDR -> HPS_DQ_15   -> F3,  6A (15)
                                                                  -- HPS_DDR -> HPS_DQ_16   -> J3,  6B (16)
                                                                  -- HPS_DDR -> HPS_DQ_17   -> J4,  6B (17)
                                                                  -- HPS_DDR -> HPS_DQ_18   -> R9,  6B (18)
                                                                  -- HPS_DDR -> HPS_DQ_19   -> K2,  6B (19)
                                                                  -- HPS_DDR -> HPS_DQ_20   -> M5,  6B (20)
                                                                  -- HPS_DDR -> HPS_DQ_21   -> N4,  6B (21)
                                                                  -- HPS_DDR -> HPS_DQ_22   -> J8,  6B (22)
                                                                  -- HPS_DDR -> HPS_DQ_23   -> K3,  6B (23)
                                                                  -- HPS_DDR -> HPS_DQ_24   -> K4,  6B (24)
                                                                  -- HPS_DDR -> HPS_DQ_25   -> L4,  6B (25)
                                                                  -- HPS_DDR -> HPS_DQ_26   -> U9,  6B (26)
                                                                  -- HPS_DDR -> HPS_DQ_27   -> M1,  6B (27)
                                                                  -- HPS_DDR -> HPS_DQ_28   -> P1,  6B (28)
                                                                  -- HPS_DDR -> HPS_DQ_29   -> N2,  6B (29)
                                                                  -- HPS_DDR -> HPS_DQ_30   -> R6,  6B (30)
                                                                  -- HPS_DDR -> HPS_DQ_31   -> N5,  6B (31)
                                                                  -- HPS_DDR -> HPS_DQ_32   -> T1,  6B (32)
                                                                  -- HPS_DDR -> HPS_DQ_33   -> R1,  6B (33)
                                                                  -- HPS_DDR -> HPS_DQ_34   -> M6,  6B (34)
                                                                  -- HPS_DDR -> HPS_DQ_35   -> T8,  6B (35)
                                                                  -- HPS_DDR -> HPS_DQ_36   -> R7,  6B (36)
                                                                  -- HPS_DDR -> HPS_DQ_37   -> P7,  6B (37)
                                                                  -- HPS_DDR -> HPS_DQ_38   -> N7,  6B (38)
                                                                  -- HPS_DDR -> HPS_DQ_39   -> R5,  6B (39)
    hps_memory_mem_dqs     : inout std_logic_vector(4 downto 0);  -- HPS_DDR -> HPS_DQS_0   -> A5,  6A (0)
                                                                  -- HPS_DDR -> HPS_DQS_1   -> D2,  6A (1)
                                                                  -- HPS_DDR -> HPS_DQS_2   -> G8,  6B (2)
                                                                  -- HPS_DDR -> HPS_DQS_3   -> M2,  6B (3)
                                                                  -- HPS_DDR -> HPS_DQS_4   -> R3,  6B (4)
    hps_memory_mem_dqs_n   : inout std_logic_vector(4 downto 0);  -- HPS_DDR -> HPS_DQS_0_N -> B6,  6A (0)
                                                                  -- HPS_DDR -> HPS_DQS_1_N -> E1,  6A (1)
                                                                  -- HPS_DDR -> HPS_DQS_2_N -> F8,  6B (2)
                                                                  -- HPS_DDR -> HPS_DQS_3_N -> M3,  6B (3)
                                                                  -- HPS_DDR -> HPS_DQS_4_N -> R2,  6B (4)
    hps_memory_mem_odt     : out   std_logic;                     -- HPS_DDR -> HPS_ODT_0   -> G3,  6A (Why ODT_0?)
    hps_memory_mem_dm      : out   std_logic_vector(4 downto 0);  -- HPS_DDR -> HPS_DM_0    -> K5,  6A
                                                                  -- HPS_DDR -> HPS_DM_1    -> E3,  6A
                                                                  -- HPS_DDR -> HPS_DM_2    -> L3,  6B
                                                                  -- HPS_DDR -> HPS_DM_3    -> P4,  6B
                                                                  -- HPS_DDR -> HPS_DM_4    -> R4,  6B
    hps_memory_oct_rzqin   : in    std_logic;                     -- HPS_DDR -> HPS_RZQ     -> B10, 6A
    -- HPS peripherals
    hps_emac1_INTn         : in    std_logic; -- Management bus interrupt;     RGMII0_TX_CTL,HPS_GPIO9;                        -- PIN_M14, 7D
    hps_emac1_MDC          : out   std_logic; -- Management bus clock;         RGMII1_MDC,SPIM0_MOSI,SPIS0_MOSI,HPS_GPIO55;    -- PIN_J16, 7E
    hps_emac1_MDIO         : inout std_logic; -- Management bus data;          RGMII1_MDIO,SPIM0_CLK,SPIS0_CLK,HPS_GPIO54;     -- PIN_F16, 7E
    hps_emac1_TX_CLK       : out   std_logic; -- 125-MHz RGMII transmit clock; RGMII1_TX_CLK,HPS_GPIO48;                       -- PIN_E16, 7D
    hps_emac1_TX_CTL       : out   std_logic; -- RGMII transmit enable;        RGMII1_TX_CTL,HPS_GPIO51;                       -- PIN_H15, 7D
    hps_emac1_TXD0         : out   std_logic; -- RGMII transmit data bus;      RGMII1_TXD0,HPS_GPIO49;                         -- PIN_D18, 7D
    hps_emac1_TXD1         : out   std_logic; -- RGMII transmit data bus;      RGMII1_TXD1,HPS_GPIO50;                         -- PIN_E19, 7D
    hps_emac1_TXD2         : out   std_logic; -- RGMII transmit data bus;      RGMII1_TXD2,SPIM0_MISO,SPIS0_MISO,HPS_GPIO56;   -- PIN_F17, 7E
    hps_emac1_TXD3         : out   std_logic; -- RGMII transmit data bus;      RGMII1_TXD3,SPIM0_SS0,SPIS0_SS0,HPS_GPIO57;     -- PIN_H16, 7E
    hps_emac1_RX_CLK       : in    std_logic; -- RGMII receive clock;          RGMII1_RX_CLK,SPIS1_CLK,SPIM1_CLK,HPS_GPIO58;   -- PIN_L16, 7E
    hps_emac1_RX_CTL       : in    std_logic; -- RGMII receive data valid;     RGMII1_RX_CTL,SPIS1_MOSI,SPIM1_MOSI,HPS_GPIO59; -- PIN_R16, 7E
    hps_emac1_RXD0         : in    std_logic; -- RGMII receive data bus;       RGMII1_RXD0,HPS_GPIO52;                         -- PIN_D20, 7D
    hps_emac1_RXD1         : in    std_logic; -- RGMII receive data bus;       RGMII1_RXD1,HPS_GPIO53;                         -- PIN_H14, 7D
    hps_emac1_RXD2         : in    std_logic; -- RGMII receive data bus;       RGMII1_RXD2,SPIS1_MISO,SPIM1_MISO,HPS_GPIO60;   -- PIN_K16, 7E
    hps_emac1_RXD3         : in    std_logic; -- RGMII receive data bus;       RGMII1_RXD3,SPIS1_SS0,SPIM1_SS0,HPS_GPIO61;     -- PIN_P16, 7E
    -- HPS SQI/FLASH -> N25Q512A83GSF40F (This is NOT the flash where the bit stream is stored!!!)
    hps_qspi_IO0           : inout std_logic; -- QSPI_IO0,USB1_CLK,HPS_GPIO29;                                                 -- PIN_J12, 7B
    hps_qspi_IO1           : inout std_logic; -- QSPI_IO1,USB1_STP,HPS_GPIO30;                                                 -- PIN_B21, 7B
    hps_qspi_IO2           : inout std_logic; -- QSPI_IO2,USB1_DIR,HPS_GPIO31;                                                 -- PIN_C20, 7B
    hps_qspi_IO3           : inout std_logic; -- QSPI_IO3,USB1_NXT,HPS_GPIO32;                                                 -- PIN_C21, 7B
    hps_qspi_SS0           : out   std_logic; -- QSPI_SS0,BOOTSEL1,HPS_GPIO33;                                                 -- PIN_C19, 7B
    hps_qspi_CLK           : out   std_logic; -- QSPI_CLK,HPS_GPIO34;                                                          -- PIN_A25, 7B
    -- HPS CD CARD
    hps_sdio_CLK           : out   std_logic; -- SD Card Clock;                SDMMC_CCLK_OUT,USB0_STP,HPS_GPIO45;             -- PIN_J13, 7C
    hps_sdio_CMD           : inout std_logic; -- SD Card Command;              SDMMC_CMD,USB0_D0,HPS_GPIO36;                   -- PIN_D13, 7C
    hps_sdio_D0            : inout std_logic; -- SD Card Data0;                SDMMC_D0,USB0_D2,HPS_GPIO38;                    -- PIN_D14, 7C
    hps_sdio_D1            : inout std_logic; -- SD Card Data1;                SDMMC_D1,USB0_D3,HPS_GPIO39;                    -- PIN_L13, 7C
    hps_sdio_D2            : inout std_logic; -- SD Card Data2;                SDMMC_D2,USB0_DIR,HPS_GPIO46;                   -- PIN_H13, 7C
    hps_sdio_D3            : inout std_logic; -- SD Card Data3 and Control;    SDMMC_D3,USB0_NXT,HPS_GPIO47;                   -- PIN_H12, 7C
    -- HPS USB TO GO
    hps_usb1_D0            : inout std_logic; -- RGMII0_TXD0,USB1_D0,HPS_GPIO1;                                                -- PIN_N15, 7D
    hps_usb1_D1            : inout std_logic; -- RGMII0_TXD1,USB1_D1,HPS_GPIO2;                                                -- PIN_K15, 7D
    hps_usb1_D2            : inout std_logic; -- RGMII0_TXD2,USB1_D2,HPS_GPIO3;                                                -- PIN_P15, 7D
    hps_usb1_D3            : inout std_logic; -- RGMII0_TXD3,USB1_D3,HPS_GPIO4;                                                -- PIN_D15, 7D
    hps_usb1_D4            : inout std_logic; -- RGMII0_RXD0,USB1_D4,HPS_GPIO5;                                                -- PIN_M15, 7D
    hps_usb1_D5            : inout std_logic; -- RGMII0_MDIO,USB1_D5,I2C2_SDA,HPS_GPIO6;                                       -- PIN_E15, 7D
    hps_usb1_D6            : inout std_logic; -- RGMII0_MDC ,USB1_D6,I2C2_SCL,HPS_GPIO7;                                       -- PIN_N16, 7D
    hps_usb1_D7            : inout std_logic; -- RGMII0_RX_CTL,USB1_D7,HPS_GPIO8;                                              -- PIN_D17, 7D
    hps_usb1_CLK           : in    std_logic; -- RGMII0_RX_CLK,USB1_CLK,HPS_GPIO10;                                            -- PIN_D16, 7D
    hps_usb1_STP           : out   std_logic; -- RGMII0_RXD1,USB1_STP,HPS_GPIO11;                                              -- PIN_L14, 7D
    hps_usb1_DIR           : in    std_logic; -- RGMII0_RXD2,USB1_DIR,HPS_GPIO12;                                              -- PIN_F15, 7D
    hps_usb1_NXT           : in    std_logic; -- RGMII0_RXD3,USB1_NXT,HPS_GPIO13;                                              -- PIN_D19, 7D
    -- HPS UART
    hps_uart0_RX           : in    std_logic; -- UART A RX;                    UART0_RX*,UART0_RX,SPIM1_MISO,HPS_GPIO65;       -- PIN_C11, 7A
    hps_uart0_TX           : out   std_logic; -- UART A TX;                    UART0_TX*,CLKSEL0,UART0_TX,SPIM1_SS0,HPS_GPIO66;-- PIN_B19, 7A
    hps_uart1_RX           : in    std_logic; -- UART B RX;                    UART1_RX,SPIM1_SS1,HPS_GPIO62;                  -- PIN_J9,  7A
    hps_uart1_TX           : out   std_logic; -- UART B TX;                    UART1_TX,SPIM0_CLK,HPS_GPIO63;                  -- PIN_A31, 7A
    -- HPS I2C
    hps_i2c0_SDA           : inout std_logic; -- Management serial data;       I2C0_SCL,UART1_TX,SPIM1_MOSI,HPS_GPIO64;        -- PIN_N12, 7A
    hps_i2c0_SCL           : inout std_logic; -- Management serial clock;      I2C0_SDA,UART1_RX,SPIM1_CLK,HPS_GPIO63;         -- PIN_B18, 7A
    -- GPIO
    hps_gpio_GPIO00        : inout std_logic;
    hps_gpio_GPIO17        : inout std_logic;
    hps_gpio_GPIO18        : inout std_logic;
    hps_gpio_GPIO22        : inout std_logic;
    hps_gpio_GPIO24        : inout std_logic;
    hps_gpio_GPIO26        : inout std_logic;
    hps_gpio_GPIO27        : inout std_logic;
    hps_gpio_GPIO35        : inout std_logic;
    hps_gpio_GPIO40        : inout std_logic;
    hps_gpio_GPIO41        : inout std_logic;
    hps_gpio_GPIO42        : inout std_logic;
    hps_gpio_GPIO43        : inout std_logic;
    -- TRACE
    hps_trace_CLK          : inout std_logic;  -- PIN_A15
    hps_trace_D0           : inout std_logic;  -- PIN_K10
    hps_trace_D1           : inout std_logic;  -- PIN_A16
    hps_trace_D2           : inout std_logic;  -- PIN_L10 -- I2C1_SDA @ HPS_GPIO51
    hps_trace_D3           : inout std_logic;  -- PIN_A18 -- I2C1_SCL @ HPS_GPIO52
    hps_trace_D4           : inout std_logic;  -- PIN_L11
    hps_trace_D5           : inout std_logic;  -- PIN_A17
    hps_trace_D6           : inout std_logic;  -- PIN_M11
    hps_trace_D7           : inout std_logic;  -- PIN_B15
    -- FPGA clock and reset
    sfp234_ref_clk_i       : in    std_logic;
    clk_20m_vcxo_i         : in    std_logic;
    clk_125m_pllref_i      : in    std_logic;
    clk_125m_local_i       : in    std_logic;
    pcie_refclk_i          : in    std_logic;
    fpga_clk_50            : in    std_logic
  );
end av_rocket_board;

architecture rtl of av_rocket_board is

  -- PLL stub signals
  signal clk_20m_vcxo    : std_logic;
  signal clk_125m_pllref : std_logic;
  signal clk_sfp         : std_logic;
  signal clk_125m_sfpref : std_logic;
  signal clk_125m_local  : std_logic;
  --signal fpga_clk_50     : std_logic;
  signal s_locked        : std_logic;
  
  -- Monster signals
  constant c_family       : string := "Arria V"; 
  constant c_project      : string := "av_rocket_board";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & ".mif";
  constant c_profile_name : string := "medium_icache_debug";

  signal s_gpio_o       : std_logic_vector( 4 downto 1);
  signal s_gpio_i       : std_logic_vector( 4 downto 1);
  signal s_lvds_p_i     : std_logic_vector(11 downto 1);
  signal s_lvds_n_i     : std_logic_vector(11 downto 1);
  signal s_lvds_i_led   : std_logic_vector(11 downto 1);
  signal s_lvds_p_o     : std_logic_vector(11 downto 1);
  signal s_lvds_n_o     : std_logic_vector(11 downto 1);
  signal s_lvds_o_led   : std_logic_vector(11 downto 1);
  signal s_lvds_oen     : std_logic_vector( 8 downto 1);
  signal s_lvds_term    : std_logic_vector(11 downto 1);

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 24) := 
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,      Channel,  OutputEnable, Termination, Logic Level
    ("LED1       ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED2       ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED3       ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED4       ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("BUT1       ",  IO_NONE,         false,   false,  0,     IO_INPUT,       IO_GPIO,  false,        false,       IO_TTL),
    ("BUT2       ",  IO_NONE,         false,   false,  1,     IO_INPUT,       IO_GPIO,  false,        false,       IO_TTL),
    ("BUT3       ",  IO_NONE,         false,   false,  2,     IO_INPUT,       IO_GPIO,  false,        false,       IO_TTL),
    ("BUT4       ",  IO_NONE,         false,   false,  3,     IO_INPUT,       IO_GPIO,  false,        false,       IO_TTL),
    ("IO1        ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO2        ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO3        ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO4        ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO5        ",  IO_NONE,         false,   false,  4,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO6        ",  IO_NONE,         false,   false,  5,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO7        ",  IO_NONE,         false,   false,  6,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("IO8        ",  IO_NONE,         false,   false,  7,     IO_INOUTPUT,    IO_LVDS,  true,         true,        IO_LVTTL),
    ("LVDSo1     ",  IO_NONE,         false,   false,  8,     IO_OUTPUT,      IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSo2     ",  IO_NONE,         false,   false,  9,     IO_OUTPUT,      IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSo3     ",  IO_NONE,         false,   false, 10,     IO_OUTPUT,      IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi1     ",  IO_NONE,         false,   false,  8,     IO_INPUT,       IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi2     ",  IO_NONE,         false,   false,  9,     IO_INPUT,       IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSi3     ",  IO_NONE,         false,   false, 10,     IO_INPUT,       IO_LVDS,  false,        false,       IO_LVDS),
    ("LVDSoClk   ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,      IO_FIXED, false,        false,       IO_LVDS),
    ("WR200MHz   ",  IO_NONE,         false,   false,  0,     IO_OUTPUT,      IO_FIXED, false,        false,       IO_LVDS),
    ("LVDSiClk   ",  IO_NONE,         false,   false,  0,     IO_INPUT,       IO_FIXED, false,        false,       IO_LVDS)
  );

  -- HPS signals
  signal stm_hw_events          : std_logic_vector(27 downto 0);
  signal fpga_led_internal_o    : std_logic_vector(3 downto 0);
  signal fpga_led_internal_i    : std_logic_vector(3 downto 0);
  signal fpga_debounced_buttons : std_logic_vector(3 downto 0);
  signal hps_fpga_reset_n       : std_logic;
  signal hps_cold_reset         : std_logic;
  signal hps_warm_reset         : std_logic;
  signal hps_debug_reset        : std_logic;
  signal hps_reset_req          : std_logic_vector(2 downto 0);

  signal fpga_button_pio_dummy  : std_logic_vector(3 downto 0);
  
  -- Misc. debug stuff
  signal s_led_blinking_1       : std_logic := '0';
  signal s_led_blinking_2       : std_logic := '0';
  signal s_led_blinking_3       : std_logic := '0';
  signal s_led_blinking_4       : std_logic := '0';
  signal led_counter_1          : std_logic_vector(31 downto 0) := (others => '0');
  signal led_counter_2          : std_logic_vector(31 downto 0) := (others => '0');
  signal led_counter_3          : std_logic_vector(31 downto 0) := (others => '0');
  signal led_counter_4          : std_logic_vector(31 downto 0) := (others => '0');
  
begin

  -- PLL stub
--  pll_inst : stub_pll
--    port map(
--      refclk   => clk_125m_local_i,
--      rst      => '0',
--      locked   => s_locked,
--      outclk_0 => clk_125m_pllref,
--      outclk_1 => clk_125m_sfpref,
--      outclk_2 => fpga_clk_50,
--      outclk_3 => clk_20m_vcxo
--    );

  -- Monster ...
  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25, 
      g_gpio_in         => 4,
      g_gpio_out        => 4,
      g_lvds_in         => 3,
      g_lvds_out        => 3,
      g_lvds_inout      => 8,
      g_fixed           => 3,
      g_en_pcie         => true,
      g_en_usb          => true,
      g_en_ssd1325      => true,
      g_io_table        => io_mapping_table,
      g_lm32_cores      => c_cores,
      g_lm32_ramsizes   => c_lm32_ramsizes/4,
      g_lm32_init_files => f_string_list_repeat(c_initf_name & ".mif", c_cores),
      g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
    )
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_clk_butis_o       => lvds_clk_p_o,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
      wr_dac_sclk_o          => dac_sclk,
      wr_dac_din_o           => dac_din,
      wr_ndac_cs_o           => ndac_cs,
      led_link_up_o          => s_led_link_up,
      led_link_act_o         => s_led_link_act,
      led_track_o            => s_led_track,
      led_pps_o              => s_led_pps,
      --wr_uart_o              => uart_o,
      --wr_uart_i              => uart_i,
      usb_rstn_o              => ures,
      usb_ebcyc_i             => pa(3),
      usb_speed_i             => pa(0),
      usb_shift_i             => pa(1),
      usb_readyn_io           => pa(7),
      usb_fifoadr_o           => pa(5 downto 4), 
      usb_sloen_o             => pa(2),
      usb_fulln_i             => ctl(1),
      usb_emptyn_i            => ctl(2),
      usb_slrdn_o             => slrd,
      usb_slwrn_o             => slwr,
      usb_pktendn_o           => pa(6),
      usb_fd_io               => fd,
      wr_ext_clk_i           => lvds_clk_p_i,
      gpio_o                 => s_gpio_o,
      gpio_i                 => s_gpio_i,
      lvds_p_i               => s_lvds_p_i,
      lvds_n_i               => s_lvds_n_i,
      lvds_i_led_o           => s_lvds_i_led,
      lvds_p_o               => s_lvds_p_o,
      lvds_n_o               => s_lvds_n_o,
      lvds_o_led_o           => s_lvds_o_led,
      lvds_oen_o(7 downto 0) => s_lvds_oen,
      lvds_term_o            => s_lvds_term,
      ssd1325_rst_o           => dsp_resn_o,
      ssd1325_dc_o            => dsp_dcn_o,
      ssd1325_ss_o            => dsp_csn_o,
      ssd1325_sclk_o          => dsp_d0_o,
      ssd1325_data_o          => dsp_d1_o,
      pcie_refclk_i          => pcie_refclk_i,
      pcie_rstn_i            => pcie_resetn_i,
      pcie_rx_i              => pcie_rx_i,
      pcie_tx_o              => pcie_tx_o
    );
  
  -- Monster connections
  sfp4_tx_disable_o <= '0';
  sfp3_tx_disable_o <= '1';
  sfp2_tx_disable_o <= '1';
  sfp1_tx_disable_o <= '1';
  --sfp4_rate <= '1';
  --sfp3_rate <= '1';
  --sfp2_rate <= '1';
  --sfp1_rate <= '1';
  
  -- SoC sub-system module
  soc_inst : ghrd_5astfd5k3
    port map(
      hps_0_f2h_stm_hw_events_stm_hwevents  => stm_hw_events,
      memory_mem_a                          => hps_memory_mem_a,
      memory_mem_ba                         => hps_memory_mem_ba,
      memory_mem_ck                         => hps_memory_mem_ck,
      memory_mem_ck_n                       => hps_memory_mem_ck_n,
      memory_mem_cke                        => hps_memory_mem_cke,
      memory_mem_cs_n                       => hps_memory_mem_cs_n,
      memory_mem_ras_n                      => hps_memory_mem_ras_n,
      memory_mem_cas_n                      => hps_memory_mem_cas_n,
      memory_mem_we_n                       => hps_memory_mem_we_n,
      memory_mem_reset_n                    => hps_memory_mem_reset_n,
      memory_mem_dq                         => hps_memory_mem_dq,
      memory_mem_dqs                        => hps_memory_mem_dqs,
      memory_mem_dqs_n                      => hps_memory_mem_dqs_n,
      memory_mem_odt                        => hps_memory_mem_odt,
      memory_mem_dm                         => hps_memory_mem_dm,
      memory_oct_rzqin                      => hps_memory_oct_rzqin,
      dipsw_pio_external_connection_export  => fpga_button_pio_dummy,
      led_pio_external_connection_in_port   => fpga_led_internal_i,
      led_pio_external_connection_out_port  => fpga_led_internal_o,
      button_pio_external_connection_export => fpga_debounced_buttons,
      hps_io_hps_io_emac1_inst_TX_CLK       => hps_emac1_TX_CLK, 
      hps_io_hps_io_emac1_inst_TXD0         => hps_emac1_TXD0,
      hps_io_hps_io_emac1_inst_TXD1         => hps_emac1_TXD1,
      hps_io_hps_io_emac1_inst_TXD2         => hps_emac1_TXD2,
      hps_io_hps_io_emac1_inst_TXD3         => hps_emac1_TXD3,
      hps_io_hps_io_emac1_inst_RXD0         => hps_emac1_RXD0,
      hps_io_hps_io_emac1_inst_MDIO         => hps_emac1_MDIO,
      hps_io_hps_io_emac1_inst_MDC          => hps_emac1_MDC,
      hps_io_hps_io_emac1_inst_RX_CTL       => hps_emac1_RX_CTL,
      hps_io_hps_io_emac1_inst_TX_CTL       => hps_emac1_TX_CTL,
      hps_io_hps_io_emac1_inst_RX_CLK       => hps_emac1_RX_CLK,
      hps_io_hps_io_emac1_inst_RXD1         => hps_emac1_RXD1,
      hps_io_hps_io_emac1_inst_RXD2         => hps_emac1_RXD2,
      hps_io_hps_io_emac1_inst_RXD3         => hps_emac1_RXD3,
      hps_io_hps_io_qspi_inst_IO0           => hps_qspi_IO0,
      hps_io_hps_io_qspi_inst_IO1           => hps_qspi_IO1,
      hps_io_hps_io_qspi_inst_IO2           => hps_qspi_IO2,
      hps_io_hps_io_qspi_inst_IO3           => hps_qspi_IO3,
      hps_io_hps_io_qspi_inst_SS0           => hps_qspi_SS0,
      hps_io_hps_io_qspi_inst_CLK           => hps_qspi_CLK,
      hps_io_hps_io_sdio_inst_CMD           => hps_sdio_CMD,
      hps_io_hps_io_sdio_inst_D0            => hps_sdio_D0,
      hps_io_hps_io_sdio_inst_D1            => hps_sdio_D1,
      hps_io_hps_io_sdio_inst_CLK           => hps_sdio_CLK,
      hps_io_hps_io_sdio_inst_D2            => hps_sdio_D2,
      hps_io_hps_io_sdio_inst_D3            => hps_sdio_D3,
      hps_io_hps_io_usb1_inst_D0            => hps_usb1_D0,
      hps_io_hps_io_usb1_inst_D1            => hps_usb1_D1,
      hps_io_hps_io_usb1_inst_D2            => hps_usb1_D2,
      hps_io_hps_io_usb1_inst_D3            => hps_usb1_D3,
      hps_io_hps_io_usb1_inst_D4            => hps_usb1_D4,
      hps_io_hps_io_usb1_inst_D5            => hps_usb1_D5,
      hps_io_hps_io_usb1_inst_D6            => hps_usb1_D6,
      hps_io_hps_io_usb1_inst_D7            => hps_usb1_D7,
      hps_io_hps_io_usb1_inst_CLK           => hps_usb1_CLK,
      hps_io_hps_io_usb1_inst_STP           => hps_usb1_STP,
      hps_io_hps_io_usb1_inst_DIR           => hps_usb1_DIR,
      hps_io_hps_io_usb1_inst_NXT           => hps_usb1_NXT,
      hps_io_hps_io_uart0_inst_RX           => hps_uart0_RX,
      hps_io_hps_io_uart0_inst_TX           => hps_uart0_TX,
      hps_io_hps_io_uart1_inst_RX           => hps_uart1_RX,
      hps_io_hps_io_uart1_inst_TX           => hps_uart1_TX,
      hps_io_hps_io_i2c0_inst_SDA           => hps_i2c0_SDA,
      hps_io_hps_io_i2c0_inst_SCL           => hps_i2c0_SCL,
      hps_io_hps_io_trace_inst_CLK          => hps_trace_CLK,
      hps_io_hps_io_trace_inst_D0           => hps_trace_D0,
      hps_io_hps_io_trace_inst_D1           => hps_trace_D1,
      hps_io_hps_io_trace_inst_D2           => hps_trace_D2,
      hps_io_hps_io_trace_inst_D3           => hps_trace_D3,
      hps_io_hps_io_trace_inst_D4           => hps_trace_D4,
      hps_io_hps_io_trace_inst_D5           => hps_trace_D5,
      hps_io_hps_io_trace_inst_D6           => hps_trace_D6,
      hps_io_hps_io_trace_inst_D7           => hps_trace_D7,
      --
      hps_io_hps_io_gpio_inst_GPIO00        => hps_gpio_GPIO00,
      hps_io_hps_io_gpio_inst_GPIO17        => hps_gpio_GPIO17,
      hps_io_hps_io_gpio_inst_GPIO18        => hps_gpio_GPIO18,
      hps_io_hps_io_gpio_inst_GPIO22        => hps_gpio_GPIO22,
      hps_io_hps_io_gpio_inst_GPIO24        => hps_gpio_GPIO24,
      hps_io_hps_io_gpio_inst_GPIO26        => hps_gpio_GPIO26,
      hps_io_hps_io_gpio_inst_GPIO27        => hps_gpio_GPIO27,
      hps_io_hps_io_gpio_inst_GPIO35        => hps_gpio_GPIO35,
      hps_io_hps_io_gpio_inst_GPIO40        => hps_gpio_GPIO40,
      hps_io_hps_io_gpio_inst_GPIO41        => hps_gpio_GPIO41,
      hps_io_hps_io_gpio_inst_GPIO42        => hps_gpio_GPIO42,
      hps_io_hps_io_gpio_inst_GPIO43        => hps_gpio_GPIO43,
      
      
      
      
      
      
      
      
      
      --
      clk_clk                               => fpga_clk_50,
      hps_0_h2f_reset_reset_n               => hps_fpga_reset_n,
      reset_reset_n                         => hps_fpga_reset_n,
      hps_0_f2h_cold_reset_req_reset_n      => not(hps_cold_reset),
      hps_0_f2h_warm_reset_req_reset_n      => not(hps_warm_reset),
      hps_0_f2h_debug_reset_req_reset_n     => not(hps_debug_reset)
    );
  
  -- SoC sub-system module, debounce logic to clean out glitches within 1ms
  debounce_inst : altera_wrapper_debounce
    port map(
      clk      => fpga_clk_50,
      reset_n  => hps_fpga_reset_n,
      data_in  => fpga_button_pio_dummy,
      data_out => fpga_debounced_buttons
    );
  
  -- SoC sub-system module, source/probe megawizard instance
  hps_reset_inst : hps_reset
    port map(
      source_clk => fpga_clk_50,
      source     => hps_reset_req
    );
  
  -- SoC sub-system reset handling (cold reset)
  pulse_cold_reset_inst : altera_wrapper_pcr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(0),
      pulse_out => hps_cold_reset
    );
  
  -- SoC sub-system reset handling (warm reset)
  pulse_warm_reset_inst : altera_wrapper_pwr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(1),
      pulse_out => hps_warm_reset
    );
  
  -- SoC sub-system reset handling (debug reset)
  pulse_debug_reset_inst : altera_wrapper_pdr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(2),
      pulse_out => hps_debug_reset
    );

--  proc_led_1: process( clk_125m_pllref )
--  begin
--    if rising_edge(clk_125m_pllref) then
--     if unsigned(led_counter_1) = 0 then
--       led_counter_1 <= std_logic_vector(to_unsigned(125000000, 32));
--       s_led_blinking_1 <= not s_led_blinking_1;
--    else
--      led_counter_1 <= std_logic_vector(unsigned(led_counter_1) - 1); 
--    end if;
--		end if;
--	end process;
--
--  proc_led_2: process( fpga_clk_100 )
--  begin
--    if rising_edge(fpga_clk_100) then
--     if unsigned(led_counter_2) = 0 then
--       led_counter_2 <= std_logic_vector(to_unsigned(125000000, 32));
--       s_led_blinking_2 <= not s_led_blinking_2;
--    else
--      led_counter_2 <= std_logic_vector(unsigned(led_counter_2) - 1); 
--    end if; 
--		end if;
--	end process;
--  
--	proc_led_3: process( clk_20m_vcxo )
--  begin
--	if rising_edge(clk_20m_vcxo) then
--	 if unsigned(led_counter_3) = 0 then
--		 led_counter_3 <= std_logic_vector(to_unsigned(125000000, 32));
--		 s_led_blinking_3 <= not s_led_blinking_3;
--	else
--		led_counter_3 <= std_logic_vector(unsigned(led_counter_3) - 1); 
--	end if;
--	end if; 
--	end process;
--  
--  --proc_led_4: process( clk_200 )
--  --Begin
--	--If rising_edge(clk_200) then
--	-- if unsigned(led_counter_4) = 0 then
--	--	 led_counter_4 <= std_logic_vector(to_unsigned(200000000, 32));
--	--	 s_led_blinking_4 <= not s_led_blinking_4;
--	--Else
----		led_counter_4 <= std_logic_vector(unsigned(led_counter_4) - 1); 
----	End if;
----	End if; 
----	End process;
--
--  -- SoC connections
--  --fpga_led_pio(0)                <= not fpga_led_internal_o; -- !!! Inverted to verify the correct bitstream
--  fpga_led_pio(0)                <= not s_led_blinking_3; -- !!! Inverted to verify the correct bitstream
--  fpga_led_pio(1)                <= not s_led_blinking_2 ; -- !!! Inverted to verify the correct bitstream
--  fpga_led_pio(2)                <= not s_locked; -- !!! Inverted to verify the correct bitstream
--  fpga_led_pio(3)                <= not sfp4_tx_fault; -- !!! Inverted to verify the correct bitstream
--  stm_hw_events(3  downto  0) <= fpga_debounced_buttons;
--  stm_hw_events(7  downto  4) <= fpga_led_internal_o;
--  stm_hw_events(11 downto  8) <= fpga_dipsw_pio;
--  stm_hw_events(27 downto 12) <= (others => '0');

  -- Base board LEDs (2.5V outputs, with 2.5V pull-up)
  led_o(1) <= '0' when (s_led_link_act and s_led_link_up)='1' else 'Z'; -- red   = traffic/no-link
  led_o(2) <= '0' when s_led_link_up                   ='1' else 'Z'; -- blue  = link
  led_o(3) <= '0' when s_led_track                     ='1' else 'Z'; -- green = timing valid
  led_o(4) <= '0' when s_led_pps                       ='1' else 'Z'; -- white = PPS

-- Link LEDs (2.5V outputs, with 2.5V pull-up)
  sfp4_red_o   <= '0' when s_led_link_act='1' else 'Z';
  sfp4_green_o <= '0' when s_led_link_up ='1' else 'Z';

  -- GPIO LED outputs
  led_o(5)         <= '0' when s_gpio_o(1)='1' else 'Z';  
  led_o(6)         <= '0' when s_gpio_o(2)='1' else 'Z';
  led_o(7)         <= '0' when s_gpio_o(3)='1' else 'Z';
  led_o(8)         <= '0' when s_gpio_o(4)='1' else 'Z';

--  -- GPIO inputs
  s_gpio_i <= not button_i;
--
--  -- Bidirectional LEMOs
  lemos : for i in 1 to 8 generate
    s_lvds_p_i(i) <= lemo_p_i(i);
    s_lvds_n_i(i) <= lemo_n_i(i);
    lemo_p_o(i) <= s_lvds_p_o(i);
    lemo_n_o(i) <= s_lvds_n_o(i);

    lemo_oen_o(i)      <= '0' when s_lvds_oen(i)  ='1' else '1'; -- has pull-up to 3.3V, output is 3.3V
    lemo_term_o(i)     <= '1' when s_lvds_term(i) ='1' else '0'; -- has pull-down,       output is 3.3V
    lemo_oe_ledn_o(i)  <= '0' when s_lvds_oen(i)  ='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
    lemo_act_ledn_o(i) <= '0' when s_lvds_i_led(i)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
  end generate;

  lvds : for i in 1 to 3 generate
    s_lvds_p_i(i+8) <= lvds_p_i(i);
    s_lvds_n_i(i+8) <= lvds_n_i(i);
    lvds_p_o(i) <= s_lvds_p_o(i+8);
    lvds_n_o(i) <= s_lvds_n_o(i+8);

    lvds_i_ledn_o(i) <= '0' when s_lvds_i_led(i+8)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
    lvds_o_ledn_o(i) <= '0' when s_lvds_o_led(i+8)='1' else 'Z'; -- has pull-up to 3.3V, output is 2.5V
  end generate;
      
  -- Wires to CPLD, currently unused
  con_io <= (others => 'Z');

end rtl;
