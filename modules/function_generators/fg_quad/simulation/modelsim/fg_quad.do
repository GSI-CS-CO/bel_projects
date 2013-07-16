onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /fg_quad_datapath_tb/quad_fg/CLK_in_Hz
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/data_a
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/data_b
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/clk
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/rst
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/a_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/b_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/load_start
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/status_reg_changed
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/step_sel
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/b_shift
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/freq_sel
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/sw_out
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/set_out
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_a_shifted
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_b_shifted
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_a_reg
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_b_reg
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_Q_reg
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_X_reg
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_start_reg
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_load_start
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_inc_quad
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_inc_lin
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_add_lin_quad
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/control_state
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_cnt_out
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_reached
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_set
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_cnt_set
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_cnt
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_freq_cnt
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_freq_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_freq_sel
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_freq_cnt_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_add_cnt
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_add_cnt_en
add wave -noupdate -radix hexadecimal /fg_quad_datapath_tb/quad_fg/s_stp_reached
add wave -noupdate /fg_quad_datapath_tb/quad_fg/c_freq_cnt
add wave -noupdate /fg_quad_datapath_tb/quad_fg/c_add_cnt
add wave -noupdate /fg_quad_datapath_tb/quad_fg/c_freq_cnt_width
add wave -noupdate /fg_quad_datapath_tb/quad_fg/c_add_cnt_width
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {8900025429 ps} 0}
configure wave -namecolwidth 400
configure wave -valuecolwidth 89
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {7911944450 ps} {10532933450 ps}
