target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "40"
syn_device = "10as066n3f"
syn_top = "pci_control"
syn_project = "pci_control"
syn_family = "Arria 10"

quartus_preflow = "pci_control.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexarria10_soc/control",
  ]
}
