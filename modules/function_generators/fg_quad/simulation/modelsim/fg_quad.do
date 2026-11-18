# regression test for fg_quad_datapath, run with: vsim -c -do fg_quad.do
onerror {quit -code 1}
vlib work
vcom -2008 ../../fg_quad_pkg.vhd ../../fg_quad_datapath.vhd fg_quad_datapath_tb.vhd

vsim -gACU=false work.fg_quad_datapath_tb
run -all

vsim -gACU=true work.fg_quad_datapath_tb
run -all

quit
