target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "pci_control"
syn_project = "pci_control"

quartus_preflow = "pci_control.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexarria5/control",
  ]
}

