# ATTENTION: Manifest.py.template file is the one to edit. Manifest.py is automatically generated!
# See https://docs.makotemplates.org/en/latest/syntax.html for template syntax.

files = [
       # Blackbox
   "io_blackbox.sv",
       # Flex Bus
   "flex_bus/flex_filterplus.sv",
   "flex_bus/flex_superhub.sv",
   "flex_bus/flex_in.sv",
   "flex_bus/flex_out.sv",
   "flex_bus/flex_hub.sv",
   "flex_bus/flex_buffer.sv",
   "flex_bus/flex_filter.sv",
       # Frontend plugins
   "frontend_plugins/frontend_unknown.sv",  
   "frontend_plugins/frontend_ocio.sv",  
   "frontend_plugins/interbackplane/frontend_interbackplane.sv",  
       # Proc plugins
   "proc_plugins/proc_disable.sv",  
   "proc_plugins/proc_pass.sv",  
   "proc_plugins/proc_in_debounce.sv",  
   "proc_plugins/proc_in_debounce.sv",  
   "proc_plugins/proc_in_long_debounce.sv",  
       # User plugins
   "user_plugins/user_gpio.sv",  
       # Interbackplane stuff
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_empty.sv",  
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_5in1out.sv",  
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_in.sv",  
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_in.sv",  
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_lwlin.sv",  
   "frontend_plugins/interbackplane/cardlet_plugins/ibpl_out.sv",  

       # Environment
   "scu_diob.vhd",
   "scu_diob_pkg.vhd",
   "scu_diob.sdc",
   "blackbox_config_pkg.vhd"
]

modules = {
  "local" : [
    "../..",
  ]
}
