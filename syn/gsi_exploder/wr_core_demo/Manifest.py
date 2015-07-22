target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "ep2agx125df"
syn_grade = "c6"
syn_package = "25"
syn_top = "exploder_top"
syn_project = "exploder_top"

quartus_preflow = "exploder_top.tcl"

modules = {"local" : [ "../../../top/gsi_exploder/wr_core_demo"]}

				 
syn_tool = "quartus"
