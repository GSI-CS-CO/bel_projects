target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "34"
syn_device = "10ax027h2f"
syn_top = "pci_control"
syn_project = "pci_control"
syn_family = "Arria 10"

quartus_preflow = "pci_control.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexarria10/control",
  ]
}
