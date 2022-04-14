target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "29"
syn_device = "10ax048E3F"
syn_top = "ftm4"
syn_project = "ftm4"
syn_family = "Arria 10"

quartus_preflow = "ftm4.tcl"

modules = {
  "local" : [
    "../../../top/gsi_scu/ftm4",
  ]
}
