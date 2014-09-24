target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "exploder5_csco_tr"
syn_project = "exploder5_csco_tr"

quartus_preflow = "exploder5_csco_tr.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_exploder5/exploder5_csco_tr", 
  ]
}
