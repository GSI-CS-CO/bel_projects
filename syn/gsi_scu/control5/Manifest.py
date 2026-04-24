target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "E2SG"
syn_package = "29"
syn_device = "10AX027E3F"
syn_top = "scu_control"
syn_project = "scu_control"
syn_family = "Arria 10"

quartus_preflow = "scu_control.tcl"

modules = {
  "local" : [
    "../../../top/gsi_scu/control5",
  ]
}
