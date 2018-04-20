create_clock -name {clk_lvtio_i}        -period 10.000 [get_ports {clk_lvtio_i}       ]


# PCI Clock Settings
##############################
create_clock -period 30 -name PCI_CLOCK [get_ports {pmc_clk_i}]
set_false_path -from [get_ports pmc_rst_i] -to *



derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                          } \
 -group { clk_lvtio_i                                  } \
 -group { PCI_CLOCK                                    } \
 -group { clk_20m_vcxo_i    main|\dmtd_a5:dmtd_inst|*  } \
 -group { clk_125m_local_i  main|\sys_a5:sys_inst|*    } \
 -group { clk_sfp_ref_i     main|\ref_a5:ref_inst|*      \
          main|\phy_a5:phy|*.cdr_refclk*                 \
          main|\phy_a5:phy|*.cmu_pll.*                   \
          main|\phy_a5:phy|*|av_tx_pma|*                 \
          main|\phy_a5:phy|*|inst_av_pcs|*|tx*         } \
 -group { main|\phy_a5:phy|*|clk90bdes                   \
          main|\phy_a5:phy|*|clk90b                      \
          main|\phy_a5:phy|*|rcvdclkpma                } 

# cut: wb sys <=> pci (different frequencies and using xwb_clock_crossing)
set_false_path -from PCI_CLOCK -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to PCI_CLOCK

# cut: wb sys <=> wb flash   (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[4].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[4].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
# cut: wb sys <=> wb display (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[1].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[1].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
# cut: wr-ref <=> butis
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}]


# Assigments for node pmc_devsel_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_devsel_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_devsel_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_devsel_io]

# Assigments for node pmc_frame_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_frame_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_frame_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_frame_io]

# Assigments for node pmc_irdy_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_irdy_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_irdy_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_irdy_io]

# Assigments for node pmc_par_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_par_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_par_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_par_io]

# Assigments for node pmc_perr_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_perr_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_perr_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_perr_io]

# Assigments for node pmc_stop_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_stop_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_stop_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_stop_io]

# Assigments for node pmc_trdy_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_trdy_io]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_trdy_io]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_trdy_io]

# Assigments for node pmc_ad_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_ad_io[*]]

# Assigments for node pmc_c_be_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_c_be_io[*]]

# Assigments for node pmc_idsel_i 
###############################
set_input_delay -clock PCI_CLOCK -max 23.0 [get_ports pmc_idsel_i]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_idsel_i]

# Assigments for node pmc_gnt_i 
###############################
set_input_delay -clock PCI_CLOCK -max 20.0 [get_ports pmc_gnt_i]
set_input_delay -clock PCI_CLOCK -min 0.0 [get_ports pmc_gnt_i]

# Assigments for node pmc_intX_o 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_inta_o]
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_intb_o]
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_intc_o]
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_intd_o]

# Assigments for node pmc_serr_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_serr_io]

# Assigments for node pmc_req_o 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_req_o]

# Assigments for node pmc_busmode_io 
###############################
set_output_delay -clock PCI_CLOCK 19.0 [get_ports pmc_busmode_io[*]]


####################################################
# Assignments to meet PCI "Float to Active" Delay
####################################################
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*ad_iob*en_out    ] -to [get_ports pmc_ad_io[*]  ]

set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*par_iob*en_out   ] -to [get_ports pmc_par_io    ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*devsel_iob*en_out] -to [get_ports pmc_devsel_io ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*stop_iob*en_out  ] -to [get_ports pmc_stop_io   ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*trdy_iob*en_out  ] -to [get_ports pmc_trdy_io   ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*perr_iob*en_out  ] -to [get_ports pmc_perr_io   ]

set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*cbe_iob*en_out   ] -to [get_ports pmc_c_be_io[*]]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*frame_iob*en_out ] -to [get_ports pmc_frame_io  ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*irdy_iob*en_out  ] -to [get_ports pmc_irdy_io   ]




