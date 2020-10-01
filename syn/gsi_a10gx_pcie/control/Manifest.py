target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "i1sg"
syn_package = "45"
syn_device = "10ax115s2f"
syn_top = "pci_control"
syn_project = "pci_control"
syn_family = "Arria 10"

quartus_preflow = "pci_control.tcl"

modules = {
  "local" : [
    "../../../top/gsi_a10gx_pcie/control",
  ]
}
