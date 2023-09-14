target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "i3sg"
syn_package = "34"
syn_device = "10ax027h4f"
syn_top = "idrogen"
syn_project = "idrogen"
syn_family = "Arria 10"

quartus_preflow = "idrogen.tcl"

modules = {
  "local" : [
    "../../../top/in2p3_idrogen/control",
  ]
}
