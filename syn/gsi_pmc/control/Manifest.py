target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "pci_pmc"
syn_project = "pci_pmc"

quartus_preflow = "pci_pmc.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_pmc/control", 
  ]
}
