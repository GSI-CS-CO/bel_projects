target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_grade = "e2sg"
syn_package = "34"
syn_device = "10ax066h2f"
syn_top = "ftm10"
syn_project = "ftm10"
syn_family = "Arria 10"

quartus_preflow = "ftm10.tcl"

modules = {
  "local" : [
    "../../../top/gsi_pexarria10/ftm10",
  ]
}
