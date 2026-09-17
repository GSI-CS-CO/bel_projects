target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "29"
syn_device = "10ax048E3F"
syn_top = "ftm5dp"
syn_project = "ftm5dp"
syn_family = "Arria 10"

quartus_preflow = "ftm5dp.tcl"

modules = {
  "local" : [
    "../../../top/gsi_scu/ftm5dp",
  ]
}
