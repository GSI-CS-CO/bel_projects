target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "pci_control_sdr"
syn_project = "pci_control_sdr"

quartus_preflow = "pci_control_sdr.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexarria5/sdr",
  ]
}
