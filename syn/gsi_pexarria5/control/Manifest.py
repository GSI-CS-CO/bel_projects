target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "pci_control"
syn_project = "pci_control"

quartus_preflow = "pci_control.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_pexarria5/control", 
  ]
}
