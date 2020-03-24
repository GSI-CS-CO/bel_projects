# PCI Clock Settings
##############################
create_clock -period 30 -name pmc_pci_clk_in [get_ports {pmc_clk_i}]
set_false_path -from [get_ports pmc_rst_i] -to *

# Assigments for node pmc_devsel_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_devsel_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_devsel_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_devsel_io]

# Assigments for node pmc_frame_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_frame_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_frame_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_frame_io]

# Assigments for node pmc_irdy_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_irdy_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_irdy_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_irdy_io]

# Assigments for node pmc_par_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_par_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_par_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_par_io]

# Assigments for node pmc_perr_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_perr_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_perr_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_perr_io]

# Assigments for node pmc_stop_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_stop_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_stop_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_stop_io]

# Assigments for node pmc_trdy_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_trdy_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_trdy_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_trdy_io]

# Assigments for node pmc_ad_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_ad_io[*]]

# Assigments for node pmc_c_be_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_c_be_io[*]]

# Assigments for node pmc_idsel_i 
###############################
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_idsel_i]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_idsel_i]

# Assigments for node pmc_gnt_i 
###############################
set_input_delay -clock pmc_pci_clk_in -max 20.0 [get_ports pmc_gnt_i]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_gnt_i]

# Assigments for node pmc_intX_o 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_inta_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intb_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intc_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intd_o]

# Assigments for node pmc_serr_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_serr_io]

# Assigments for node pmc_req_o 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_req_o]

# Assigments for node pmc_busmode_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_busmode_io[*]]

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
