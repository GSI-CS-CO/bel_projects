onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {Test Bench Stimulation}
add wave -noupdate -radix unsigned /ssd1325_serial_driver_testbench/s_tb_cyc_counter
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_clk_system
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_rst_n_system
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/s_tb_slave_addr_out
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/s_tb_slave_data_out
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_we_out
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_cyc_out
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_stb_out
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/s_tb_slave_data_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_stall_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_ack_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_err_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_rty_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_tb_slave_int_in
add wave -noupdate /ssd1325_serial_driver_testbench/s_ssd_rst
add wave -noupdate /ssd1325_serial_driver_testbench/s_ssd_dc
add wave -noupdate -color Magenta /ssd1325_serial_driver_testbench/s_ssd_ss
add wave -noupdate -color {Lime Green} /ssd1325_serial_driver_testbench/s_ssd_sclk
add wave -noupdate -color Cyan /ssd1325_serial_driver_testbench/s_ssd_sd
add wave -noupdate -color Gold /ssd1325_serial_driver_testbench/s_ssd_irq
TreeUpdate [SetDefaultTree]
quietly WaveActivateNextPane
add wave -noupdate -divider ssd1325_serial_driver
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/g_address_size
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/g_data_size
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/g_spi_fifo_size
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/g_spi_data_size
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/clk_sys_i
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/rst_n_i
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/slave_i
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/slave_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_rst_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_dc_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_ss_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_sclk_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_data_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/ssd_irq_o
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_ack
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_stall
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_ss
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_irq
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_irq_en
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_irq_clear
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_done
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_data_in
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_data_out
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_write_en
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_read_en
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_empty
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_full
add wave -noupdate -radix unsigned /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_fill_level
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_rst_config
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_dc_config
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_ss_ctrl_config
add wave -noupdate /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_ss_state
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_data_reg
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_status_reg
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_fifo_fill_level_reg
add wave -noupdate -radix hexadecimal /ssd1325_serial_driver_testbench/wb_ssd1325_serial_driver_test/s_tx_control_reg
TreeUpdate [SetDefaultTree]
quietly WaveActivateNextPane
add wave -noupdate -divider {Test Bench Control}
add wave -noupdate /ssd1325_serial_driver_testbench/s_perform_legacy_tests
add wave -noupdate /ssd1325_serial_driver_testbench/s_perform_wb_to_spi_tests
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {50433810 ps} 0}
configure wave -namecolwidth 198
configure wave -valuecolwidth 86
configure wave -justifyvalue left
configure wave -signalnamewidth 1
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
WaveRestoreZoom {0 ps} {210 us}
