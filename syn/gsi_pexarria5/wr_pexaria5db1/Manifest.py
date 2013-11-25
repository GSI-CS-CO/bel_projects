target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "wr_pexaria5db1"
syn_project = "wr_pexaria5db1"

quartus_preflow = "wr_pexaria5db1.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_pexarria5/wr_pexaria5db1", 
  ]
}
